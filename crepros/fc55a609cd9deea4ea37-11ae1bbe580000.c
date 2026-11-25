// https://syzkaller.appspot.com/bug?id=8a66225cfe208d71c8d722c5676724fa51bcded0
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef AF_BLUETOOTH
#define AF_BLUETOOTH 31
#endif

#ifndef BTPROTO_HCI
#define BTPROTO_HCI 1
#endif

#define HCI_DEV_NONE 0xffff
#define HCI_CHANNEL_CONTROL 3

/* HCI Packet Types */
#define HCI_COMMAND_PKT 0x01
#define HCI_EVENT_PKT   0x04

/* HCI Events */
#define HCI_EV_CMD_COMPLETE 0x0e

/* HCI Command Opcodes */
#define HCI_OP_READ_LOCAL_VERSION             0x1001
#define HCI_OP_READ_LOCAL_COMMANDS            0x1002
#define HCI_OP_READ_LOCAL_FEATURES            0x1003
#define HCI_OP_READ_BD_ADDR                   0x1009
#define HCI_OP_LE_READ_BUFFER_SIZE            0x2002
#define HCI_OP_LE_READ_LOCAL_FEATURES         0x2003
#define HCI_OP_LE_READ_SUPPORTED_STATES       0x201c
#define HCI_OP_LE_SET_EXT_ADV_PARAMS          0x2036
#define HCI_OP_LE_READ_NUM_SUPPORTED_ADV_SETS 0x203b

/* MGMT Opcodes and Events */
#define MGMT_OP_SET_POWERED         0x0005
#define MGMT_OP_ADD_EXT_ADV_PARAMS  0x0054
#define MGMT_EV_CMD_COMPLETE        0x0001
#define MGMT_EV_INDEX_ADDED         0x0004

struct sockaddr_hci {
    sa_family_t hci_family;
    uint16_t hci_dev;
    uint16_t hci_channel;
};

struct mgmt_hdr {
    uint16_t opcode;
    uint16_t index;
    uint16_t len;
} __attribute__((packed));

struct mgmt_ev_cmd_complete {
    uint16_t opcode;
    uint8_t status;
    uint8_t data[];
} __attribute__((packed));

struct mgmt_cp_add_ext_adv_params {
    uint8_t  instance;
    uint32_t flags;
    uint16_t duration;
    uint16_t timeout;
    uint32_t min_interval;
    uint32_t max_interval;
    int8_t   tx_power;
} __attribute__((packed));

static int vhci_fd = -1;

static void *vhci_responder_thread(void *arg)
{
    (void)arg;
    uint8_t rx_buf[1024];

    printf("[+] Responder thread started. Listening for HCI commands on /dev/vhci...\n");

    while (1) {
        ssize_t n = read(vhci_fd, rx_buf, sizeof(rx_buf));
        if (n <= 0) {
            if (n < 0 && (errno == EAGAIN || errno == EINTR))
                continue;
            printf("[*] Responder thread read returned %zd (errno: %s), exiting...\n",
                   n, strerror(errno));
            pthread_exit(NULL);
        }

        if (n < 4 || rx_buf[0] != HCI_COMMAND_PKT)
            continue;

        uint16_t opcode = rx_buf[1] | (rx_buf[2] << 8);
        printf("[*] Controller received HCI opcode: 0x%04x\n", opcode);

        /* Target stall condition: When HCI_OP_LE_SET_EXT_ADV_PARAMS arrives, do NOT reply!
         * This leaves the first add_ext_adv_params command blocked in __hci_cmd_sync().
         */
        if (opcode == HCI_OP_LE_SET_EXT_ADV_PARAMS) {
            printf("[+] Captured target opcode HCI_OP_LE_SET_EXT_ADV_PARAMS (0x%04x)!\n", opcode);
            printf("[+] Withholding response to stall cmd_sync_work. Exiting responder thread...\n");
            pthread_exit(NULL);
        }

        /* Build HCI_EV_CMD_COMPLETE packet */
        uint8_t resp_buf[256];
        uint8_t rp_buf[128];
        uint8_t rp_len = 0;

        memset(rp_buf, 0, sizeof(rp_buf));
        rp_buf[0] = 0x00; /* status: success */

        switch (opcode) {
        case HCI_OP_READ_LOCAL_FEATURES:
            /* LE-only controller: LMP_NO_BREDR (0x20) | LMP_LE (0x40) in byte 4 */
            rp_buf[1 + 4] = 0x60;
            rp_len = 9; /* status (1) + features (8) */
            break;

        case HCI_OP_READ_BD_ADDR:
            /* Return BD_ADDR: 11:22:33:44:55:66 */
            rp_buf[1] = 0x11;
            rp_buf[2] = 0x22;
            rp_buf[3] = 0x33;
            rp_buf[4] = 0x44;
            rp_buf[5] = 0x55;
            rp_buf[6] = 0x66;
            rp_len = 7; /* status (1) + bdaddr (6) */
            break;

        case HCI_OP_READ_LOCAL_VERSION:
            rp_buf[1] = 0x0a; /* hci_ver: BT 5.1 */
            rp_buf[4] = 0x0a; /* lmp_ver: BT 5.1 */
            rp_len = 9;
            break;

        case HCI_OP_READ_LOCAL_COMMANDS:
            /* Return 64 bytes of 0x00 (no optional commands supported, prevents extra init queries) */
            memset(&rp_buf[1], 0x00, 64);
            rp_len = 65;
            break;

        case HCI_OP_LE_READ_LOCAL_FEATURES:
            /* Enable only HCI_LE_EXT_ADV (bit 4 of byte 1, 0x10) */
            memset(&rp_buf[1], 0x00, 8);
            rp_buf[1 + 1] = 0x10;
            rp_len = 9;
            break;

        case HCI_OP_LE_READ_BUFFER_SIZE:
            rp_buf[1] = 0x1b; /* le_mtu: 27 */
            rp_buf[2] = 0x00;
            rp_buf[3] = 0x04; /* le_max_pkt: 4 */
            rp_len = 4;
            break;

        case HCI_OP_LE_READ_NUM_SUPPORTED_ADV_SETS:
            rp_buf[1] = 0x04; /* num_of_sets: 4 */
            rp_len = 2;
            break;

        case HCI_OP_LE_READ_SUPPORTED_STATES:
            memset(&rp_buf[1], 0xff, 8);
            rp_len = 9;
            break;

        default:
            /* Status-only response (rp_len = 1, status = 0x00) */
            rp_len = 1;
            break;
        }

        resp_buf[0] = HCI_EVENT_PKT;
        resp_buf[1] = HCI_EV_CMD_COMPLETE;
        resp_buf[2] = 3 + rp_len;           /* plen */
        resp_buf[3] = 0x01;                 /* ncmd */
        resp_buf[4] = opcode & 0xff;        /* opcode low */
        resp_buf[5] = (opcode >> 8) & 0xff; /* opcode high */
        memcpy(&resp_buf[6], rp_buf, rp_len);

        ssize_t w = write(vhci_fd, resp_buf, 3 + resp_buf[2]);
        if (w < 0) {
            printf("[-] Failed to write HCI event response: %s\n", strerror(errno));
            pthread_exit(NULL);
        }
    }

    return NULL;
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("[*] Reproducer for deadlock in add_ext_adv_params_complete started.\n");

    /* 1. Setup Bluetooth Management Socket */
    int mgmt_fd = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
    if (mgmt_fd < 0) {
        printf("[-] Failed to create AF_BLUETOOTH socket: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] Created AF_BLUETOOTH socket.\n");

    struct sockaddr_hci addr;
    memset(&addr, 0, sizeof(addr));
    addr.hci_family  = AF_BLUETOOTH;
    addr.hci_dev     = HCI_DEV_NONE;
    addr.hci_channel = HCI_CHANNEL_CONTROL;

    int res = bind(mgmt_fd, (struct sockaddr *)&addr, sizeof(addr));
    if (res < 0) {
        printf("[-] Failed to bind AF_BLUETOOTH socket to HCI_CHANNEL_CONTROL: %s\n",
               strerror(errno));
        close(mgmt_fd);
        exit(1);
    }
    printf("[+] Bound AF_BLUETOOTH socket to HCI_CHANNEL_CONTROL.\n");

    /* 2. Create Virtual Bluetooth Controller via /dev/vhci */
    vhci_fd = open("/dev/vhci", O_RDWR);
    if (vhci_fd < 0) {
        printf("[-] Failed to open /dev/vhci: %s\n", strerror(errno));
        close(mgmt_fd);
        exit(1);
    }
    printf("[+] Successfully opened /dev/vhci.\n");

    uint8_t create_req[2] = { 0xff, 0x00 };
    ssize_t written = write(vhci_fd, create_req, sizeof(create_req));
    if (written < 0) {
        printf("[-] Failed to write create request to /dev/vhci: %s\n", strerror(errno));
        close(vhci_fd);
        close(mgmt_fd);
        exit(1);
    }
    printf("[+] Wrote controller create command to /dev/vhci.\n");

    uint8_t vhci_resp[4];
    ssize_t n = read(vhci_fd, vhci_resp, sizeof(vhci_resp));
    if (n < 0) {
        printf("[-] Failed to read controller response from /dev/vhci: %s\n", strerror(errno));
        close(vhci_fd);
        close(mgmt_fd);
        exit(1);
    }
    if (n != 4 || vhci_resp[0] != 0xff || vhci_resp[1] != 0x00) {
        printf("[-] Unexpected controller response from /dev/vhci: read %zd bytes\n", n);
        close(vhci_fd);
        close(mgmt_fd);
        exit(1);
    }
    uint16_t hci_id = vhci_resp[2] | (vhci_resp[3] << 8);
    printf("[+] Created virtual controller hci%u.\n", hci_id);

    /* 3. Start Responder Thread */
    pthread_t thread;
    res = pthread_create(&thread, NULL, vhci_responder_thread, NULL);
    if (res != 0) {
        printf("[-] Failed to create responder thread: %s\n", strerror(res));
        close(vhci_fd);
        close(mgmt_fd);
        exit(1);
    }
    printf("[+] Created vhci responder thread.\n");

    /* 4. Wait for MGMT_EV_INDEX_ADDED for our hci_id */
    printf("[*] Waiting for MGMT_EV_INDEX_ADDED for hci%u...\n", hci_id);
    while (1) {
        uint8_t buf[512];
        ssize_t r = recv(mgmt_fd, buf, sizeof(buf), 0);
        if (r < (ssize_t)sizeof(struct mgmt_hdr)) {
            printf("[-] Failed to read mgmt event: %s\n", strerror(errno));
            close(vhci_fd);
            close(mgmt_fd);
            exit(1);
        }
        struct mgmt_hdr *hdr = (struct mgmt_hdr *)buf;
        if (hdr->opcode == MGMT_EV_INDEX_ADDED && hdr->index == hci_id) {
            printf("[+] Received MGMT_EV_INDEX_ADDED for hci%u.\n", hci_id);
            break;
        }
    }

    /* 5. Power On Controller via MGMT Socket */
    printf("[*] Sending MGMT_OP_SET_POWERED to hci%u...\n", hci_id);
    struct {
        struct mgmt_hdr hdr;
        uint8_t val;
    } __attribute__((packed)) pwr_req;
    pwr_req.hdr.opcode = MGMT_OP_SET_POWERED;
    pwr_req.hdr.index  = hci_id;
    pwr_req.hdr.len    = sizeof(pwr_req.val);
    pwr_req.val        = 1;

    ssize_t s = send(mgmt_fd, &pwr_req, sizeof(pwr_req), 0);
    if (s < 0) {
        printf("[-] Failed to send MGMT_OP_SET_POWERED: %s\n", strerror(errno));
        close(vhci_fd);
        close(mgmt_fd);
        exit(1);
    }

    /* Wait for MGMT_OP_SET_POWERED completion */
    printf("[*] Waiting for MGMT_OP_SET_POWERED completion...\n");
    while (1) {
        uint8_t buf[512];
        ssize_t r = recv(mgmt_fd, buf, sizeof(buf), 0);
        if (r < (ssize_t)sizeof(struct mgmt_hdr)) {
            printf("[-] Failed to read mgmt event: %s\n", strerror(errno));
            close(vhci_fd);
            close(mgmt_fd);
            exit(1);
        }
        struct mgmt_hdr *hdr = (struct mgmt_hdr *)buf;
        if (hdr->opcode == MGMT_EV_CMD_COMPLETE && hdr->index == hci_id) {
            struct mgmt_ev_cmd_complete *ev = (struct mgmt_ev_cmd_complete *)(buf + sizeof(*hdr));
            if (ev->opcode == MGMT_OP_SET_POWERED) {
                printf("[+] Controller hci%u powered on successfully (status: 0x%02x).\n",
                       hci_id, ev->status);
                break;
            }
        }
    }

    /* 6. Send First MGMT_OP_ADD_EXT_ADV_PARAMS (Instance 1)
     * This will enter add_ext_adv_params_sync() and send HCI_OP_LE_SET_EXT_ADV_PARAMS (0x2036).
     * The responder thread will intercept 0x2036, withhold the response, and terminate.
     */
    printf("[*] Sending MGMT_OP_ADD_EXT_ADV_PARAMS for instance 1 (stalled entry)...\n");
    struct {
        struct mgmt_hdr hdr;
        struct mgmt_cp_add_ext_adv_params cp;
    } __attribute__((packed)) adv_req1;
    memset(&adv_req1, 0, sizeof(adv_req1));
    adv_req1.hdr.opcode = MGMT_OP_ADD_EXT_ADV_PARAMS;
    adv_req1.hdr.index  = hci_id;
    adv_req1.hdr.len    = sizeof(adv_req1.cp);
    adv_req1.cp.instance = 1;

    s = send(mgmt_fd, &adv_req1, sizeof(adv_req1), 0);
    if (s < 0) {
        printf("[-] Failed to send MGMT_OP_ADD_EXT_ADV_PARAMS (inst 1): %s\n", strerror(errno));
        close(vhci_fd);
        close(mgmt_fd);
        exit(1);
    }
    printf("[+] Sent MGMT_OP_ADD_EXT_ADV_PARAMS (inst 1).\n");

    /* Wait for responder thread to exit upon intercepting 0x2036 */
    printf("[*] Waiting for responder thread to intercept 0x2036...\n");
    res = pthread_join(thread, NULL);
    if (res != 0) {
        printf("[-] pthread_join failed: %s\n", strerror(res));
        close(vhci_fd);
        close(mgmt_fd);
        exit(1);
    }
    printf("[+] Responder thread joined. Instance 1 is now stalled in kernel __hci_cmd_sync().\n");

    /* 7. Send Second MGMT_OP_ADD_EXT_ADV_PARAMS (Instance 2)
     * add_ext_adv_params() acquires hdev->lock -> cmd_sync_work_lock, appending instance 2
     * to hdev->cmd_sync_work_list. Because cmd_sync_work is blocked on instance 1,
     * instance 2 remains queued in cmd_sync_work_list.
     */
    printf("[*] Sending MGMT_OP_ADD_EXT_ADV_PARAMS for instance 2 (backlogged entry)...\n");
    struct {
        struct mgmt_hdr hdr;
        struct mgmt_cp_add_ext_adv_params cp;
    } __attribute__((packed)) adv_req2;
    memset(&adv_req2, 0, sizeof(adv_req2));
    adv_req2.hdr.opcode = MGMT_OP_ADD_EXT_ADV_PARAMS;
    adv_req2.hdr.index  = hci_id;
    adv_req2.hdr.len    = sizeof(adv_req2.cp);
    adv_req2.cp.instance = 2;

    s = send(mgmt_fd, &adv_req2, sizeof(adv_req2), 0);
    if (s < 0) {
        printf("[-] Failed to send MGMT_OP_ADD_EXT_ADV_PARAMS (inst 2): %s\n", strerror(errno));
        close(vhci_fd);
        close(mgmt_fd);
        exit(1);
    }
    printf("[+] Sent MGMT_OP_ADD_EXT_ADV_PARAMS (inst 2).\n");

    /* Ensure instance 2 is queued into hdev->cmd_sync_work_list */
    usleep(50000);

    /* 8. Trigger Deadlock via Controller Teardown
     * Closing vhci_fd triggers vhci_release() -> hci_unregister_dev() -> hci_cmd_sync_clear().
     * hci_cmd_sync_clear() cancels the blocked instance 1, then holds cmd_sync_work_lock
     * while canceling instance 2. The destroy callback add_ext_adv_params_complete()
     * attempts to acquire hdev->lock while holding cmd_sync_work_lock, triggering
     * lockdep circular dependency detection.
     */
    printf("[*] Closing /dev/vhci to trigger unregistration and circular locking dependency...\n");
    int c = close(vhci_fd);
    if (c < 0) {
        printf("[-] Failed to close /dev/vhci: %s\n", strerror(errno));
        close(mgmt_fd);
        exit(1);
    }
    printf("[+] Closed /dev/vhci.\n");

    /* Sleep to allow kernel lockdep report to be fully processed and logged */
    sleep(1);

    c = close(mgmt_fd);
    if (c < 0) {
        printf("[-] Failed to close mgmt socket: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] Closed MGMT socket. Reproducer finished.\n");
    return 0;
}
