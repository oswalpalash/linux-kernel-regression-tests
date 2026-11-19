// https://syzkaller.appspot.com/bug?id=42aaad9f6810d96493e09b7367d2b066ff886e99
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <poll.h>
#include <pthread.h>
#include <endian.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>

// Copyright 2026 syzkaller project authors. All rights reserved.
// Use of this source code is governed by Apache 2 LICENSE that can be found in the LICENSE file.

// IMPORTANT: Do not copy the macros or definitions below directly into your reproducer.
// Instead, add the following line to your reproducer:
// #include "race_toolkit.h"

// --- Race Condition Toolkit ---
// Macros and snippets for CPU pinning, memory barriers, and userfaultfd.

#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <linux/futex.h>
#include <linux/userfaultfd.h>
#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

// Unbuffered I/O: Ensure logs are written immediately.
#define SETUP_UNBUFFERED_IO() setvbuf(stdout, NULL, _IONBF, 0)

// CPU Pinning: Pin the current thread to a specific CPU core.
#define PIN_TO_CPU(cpu)                                                \
	do {                                                           \
		cpu_set_t mask;                                        \
		CPU_ZERO(&mask);                                       \
		CPU_SET(cpu, &mask);                                   \
		if (sched_setaffinity(0, sizeof(mask), &mask) == -1) { \
			perror("sched_setaffinity");                   \
		}                                                      \
	} while (0)

// Memory Barrier: Ensure memory ordering.
#define MB() __atomic_thread_fence(__ATOMIC_SEQ_CST)

// Spin-wait Barrier: Wait until a memory location has a specific value.
// Best for tight race windows (low latency, no context switches).
#define WAIT_ON(addr, val)                                               \
	do {                                                             \
		while (__atomic_load_n(addr, __ATOMIC_ACQUIRE) != (val)) \
			;                                                \
	} while (0)

// Signal: Set a memory location to a specific value to release a WAIT_ON.
#define SIGNAL(addr, val) __atomic_store_n(addr, val, __ATOMIC_RELEASE)

// --- Timing Primitives ---
// Robust timing loops in VM environments (using CLOCK_MONOTONIC to avoid time(NULL) jumps).

static inline double timer_elapsed_sec(struct timespec* start)
{
	struct timespec now;
	if (clock_gettime(CLOCK_MONOTONIC, &now) == -1) {
		perror("clock_gettime(CLOCK_MONOTONIC) elapsed");
		exit(1);
	}
	return (double)(now.tv_sec - start->tv_sec) + (double)(now.tv_nsec - start->tv_nsec) / 1e9;
}

// Initialize a monotonic timer variable.
#define TIMER_START(t)                                          \
	struct timespec t;                                      \
	if (clock_gettime(CLOCK_MONOTONIC, &t) == -1) {         \
		perror("clock_gettime(CLOCK_MONOTONIC) start"); \
		exit(1);                                        \
	}

// Check if the elapsed time since 't' is less than 'sec' seconds.
#define TIMER_NOT_EXPIRED(t, sec) (timer_elapsed_sec(&(t)) < (double)(sec))

// Futex-based Event: Shared with syzkaller executor.
// Best for general synchronization or longer waits to save CPU.
typedef struct {
	int state;
} event_t;

static void event_init(event_t* ev)
{
	ev->state = 0;
}
static void event_reset(event_t* ev)
{
	ev->state = 0;
}

static void event_set(event_t* ev)
{
	if (__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE)) {
		fprintf(stderr, "event already set\n");
		exit(1);
	}
	__atomic_store_n(&ev->state, 1, __ATOMIC_RELEASE);
	syscall(SYS_futex, &ev->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1000000);
}

static void event_wait(event_t* ev)
{
	while (!__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
		syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, 0);
}

// userfaultfd setup: Register a memory range for page fault handling.
static int setup_uffd(void* addr, size_t len)
{
	int uffd = syscall(__NR_userfaultfd, O_CLOEXEC | O_NONBLOCK);
	if (uffd == -1)
		return -1;
	struct uffdio_api api = {.api = UFFD_API, .features = 0};
	if (ioctl(uffd, UFFDIO_API, &api) == -1) {
		close(uffd);
		return -1;
	}
	struct uffdio_register reg = {
	    .range = {.start = (uintptr_t)addr, .len = len},
	    .mode = UFFDIO_REGISTER_MODE_MISSING};
	if (ioctl(uffd, UFFDIO_REGISTER, &reg) == -1) {
		close(uffd);
		return -1;
	}
	return uffd;
}

// --- Guidance on Usage ---
// 1. Use WAIT_ON/SIGNAL for tight race conditions to avoid scheduling overhead.
// 2. Use event_t (futexes) for general coordination or when waiting for longer periods.
// 3. Always use PIN_TO_CPU to increase race probability on multi-core systems.
// 4. Use setup_uffd to register a memory range for page fault handling. This allows you to
//    pause a thread accessing that memory until you handle the fault, creating a reliable
//    and controllable race window.
// 5. Call SETUP_UNBUFFERED_IO() at the start of main() to ensure that logs are printed
//    immediately. This is essential for understanding the exact interleaving of events
//    when debugging race conditions.
// 6. For timing-based loops (e.g., running a race for 10 seconds), do NOT use time(NULL)
//    or loops relying on real-time clocks, as VM clocks are highly unreliable and can fail or drift.
//    Instead, use the robust monotonic timing primitives TIMER_START and TIMER_NOT_EXPIRED:
//        TIMER_START(start);
//        while (TIMER_NOT_EXPIRED(start, 10.0)) {
//            // Your race logic here
//        }


#ifndef AF_BLUETOOTH
#define AF_BLUETOOTH 31
#endif

#ifndef BTPROTO_L2CAP
#define BTPROTO_L2CAP 0
#endif

#ifndef BTPROTO_HCI
#define BTPROTO_HCI 1
#endif

#ifndef SOL_BLUETOOTH
#define SOL_BLUETOOTH 274
#endif

#ifndef BT_MODE
#define BT_MODE 15
#endif

#ifndef BT_MODE_LE_FLOWCTL
#define BT_MODE_LE_FLOWCTL 0x03
#endif

#ifndef BDADDR_LE_PUBLIC
#define BDADDR_LE_PUBLIC 0x01
#endif

#ifndef HCIDEVUP
#define HCIDEVUP _IOW('H', 201, int)
#endif

typedef struct {
    uint8_t b[6];
} __attribute__((packed)) bdaddr_t;

struct sockaddr_l2 {
    sa_family_t l2_family;
    uint16_t    l2_psm;
    bdaddr_t    l2_bdaddr;
    uint16_t    l2_cid;
    uint8_t     l2_bdaddr_type;
};

static int g_vhci_fd = -1;
static int g_stop = 0;
static int g_connected = 0;
static uint16_t g_remote_scid = 0x0040;
static uint8_t g_latest_ident = 1;

static void *reader_thread(void *arg)
{
    (void)arg;
    struct pollfd pfd = { .fd = g_vhci_fd, .events = POLLIN };

    while (!__atomic_load_n(&g_stop, __ATOMIC_RELAXED)) {
        int ret = poll(&pfd, 1, 50);
        if (ret <= 0)
            continue;

        uint8_t buf[1024];
        ssize_t n = read(g_vhci_fd, buf, sizeof(buf));
        if (n <= 0)
            continue;

        if (buf[0] == 0x01 && n >= 3) {
            /* HCI Command Packet */
            uint16_t opcode = buf[1] | (buf[2] << 8);

            if (opcode == 0x200c) {
                /* HCI_OP_LE_SET_SCAN_ENABLE */
                uint8_t cc[] = { 0x04, 0x0e, 4, 0x01, 0x0c, 0x20, 0x00 };
                ssize_t w = write(g_vhci_fd, cc, sizeof(cc));
                (void)w;

                /* If scan enable is set and not yet connected, inject advertising report */
                if ((buf[4] == 0x01 || buf[3] == 0x01) && !__atomic_load_n(&g_connected, __ATOMIC_RELAXED)) {
                    uint8_t adv_report[] = {
                        0x04, 0x3e, 12, 0x02, 0x01, 0x00, 0x00,
                        0x11, 0x22, 0x33, 0x44, 0x55, 0x66,
                        0x00, 0xce
                    };
                    w = write(g_vhci_fd, adv_report, sizeof(adv_report));
                    (void)w;
                }
            } else if (opcode == 0x200d || opcode == 0x2043) {
                /* HCI_OP_LE_CREATE_CONN or HCI_OP_LE_EXT_CREATE_CONN */
                uint8_t cmd_status[] = { 0x04, 0x0f, 0x04, 0x00, 0x01, buf[1], buf[2] };
                ssize_t w = write(g_vhci_fd, cmd_status, sizeof(cmd_status));
                (void)w;

                /* Connection Complete with bdaddr_type = 0x00 (ADDR_LE_DEV_PUBLIC) */
                uint8_t conn_comp[] = {
                    0x04, 0x3e, 19, 0x01, 0x00,
                    0x40, 0x00, /* handle 0x0040 */
                    0x00,       /* role = master */
                    0x00,       /* bdaddr_type = 0x00 (ADDR_LE_DEV_PUBLIC) */
                    0x11, 0x22, 0x33, 0x44, 0x55, 0x66,
                    0x18, 0x00, 0x00, 0x00, 0x2a, 0x00, 0x00
                };
                w = write(g_vhci_fd, conn_comp, sizeof(conn_comp));
                (void)w;
            } else if (opcode == 0x2016) {
                /* HCI_OP_LE_READ_REMOTE_FEATURES */
                uint8_t cmd_status[] = { 0x04, 0x0f, 0x04, 0x00, 0x01, 0x16, 0x20 };
                ssize_t w = write(g_vhci_fd, cmd_status, sizeof(cmd_status));
                (void)w;

                /* Subevent 0x04: HCI_EV_LE_REMOTE_FEAT_COMPLETE */
                uint8_t feat_comp[] = {
                    0x04, 0x3e, 12, 0x04, 0x00, 0x40, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                };
                w = write(g_vhci_fd, feat_comp, sizeof(feat_comp));
                (void)w;
            } else if (opcode == 0x0406) {
                /* HCI_OP_DISCONNECT */
                uint8_t cmd_status[] = { 0x04, 0x0f, 0x04, 0x00, 0x01, 0x06, 0x04 };
                ssize_t w = write(g_vhci_fd, cmd_status, sizeof(cmd_status));
                (void)w;

                uint8_t disconn_comp[] = { 0x04, 0x05, 0x04, 0x00, 0x40, 0x00, 0x16 };
                w = write(g_vhci_fd, disconn_comp, sizeof(disconn_comp));
                (void)w;
            } else {
                switch (opcode) {
                case 0x1002: { /* HCI_OP_READ_LOCAL_COMMANDS */
                    uint8_t ev[71];
                    memset(ev, 0, sizeof(ev));
                    ev[0] = 0x04; /* HCI_EVENT_PKT */
                    ev[1] = 0x0e; /* HCI_EV_CMD_COMPLETE */
                    ev[2] = 68;   /* plen: 3 hdr + 1 status + 64 commands */
                    ev[3] = 0x01; /* ncmd */
                    ev[4] = 0x02; /* opcode lo */
                    ev[5] = 0x10; /* opcode hi */
                    ev[6] = 0x00; /* status */
                    ssize_t w = write(g_vhci_fd, ev, sizeof(ev));
                    (void)w;
                    break;
                }
                case 0x0c0d: { /* HCI_OP_READ_STORED_LINK_KEY */
                    uint8_t ev[] = { 0x04, 0x0e, 8, 0x01, 0x0d, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00 };
                    ssize_t w = write(g_vhci_fd, ev, sizeof(ev));
                    (void)w;
                    break;
                }
                case 0x0c12: { /* HCI_OP_DELETE_STORED_LINK_KEY */
                    uint8_t ev[] = { 0x04, 0x0e, 6, 0x01, 0x12, 0x0c, 0x00, 0x00, 0x00 };
                    ssize_t w = write(g_vhci_fd, ev, sizeof(ev));
                    (void)w;
                    break;
                }
                case 0x1009: { /* HCI_OP_READ_BD_ADDR */
                    uint8_t ev[] = { 0x04, 0x0e, 10, 0x01, 0x09, 0x10, 0x00,
                                     0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0x01 };
                    ssize_t w = write(g_vhci_fd, ev, sizeof(ev));
                    (void)w;
                    break;
                }
                case 0x1005: { /* HCI_OP_READ_BUFFER_SIZE */
                    uint8_t ev[] = { 0x04, 0x0e, 11, 0x01, 0x05, 0x10, 0x00,
                                     0x00, 0x04, 0x00, 0x0a, 0x00, 0x00, 0x00 };
                    ssize_t w = write(g_vhci_fd, ev, sizeof(ev));
                    (void)w;
                    break;
                }
                case 0x1003: { /* HCI_OP_READ_LOCAL_FEATURES */
                    uint8_t ev[15] = { 0x04, 0x0e, 12, 0x01, 0x03, 0x10, 0x00 };
                    ev[7 + 4] = 0x60; /* LMP_LE | LMP_NO_BREDR */
                    ssize_t w = write(g_vhci_fd, ev, sizeof(ev));
                    (void)w;
                    break;
                }
                case 0x1001: { /* HCI_OP_READ_LOCAL_VERSION */
                    uint8_t ev[] = { 0x04, 0x0e, 12, 0x01, 0x01, 0x10, 0x00,
                                     0x0a, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00 };
                    ssize_t w = write(g_vhci_fd, ev, sizeof(ev));
                    (void)w;
                    break;
                }
                case 0x2002: { /* HCI_OP_LE_READ_BUFFER_SIZE */
                    uint8_t ev[] = { 0x04, 0x0e, 7, 0x01, 0x02, 0x20, 0x00,
                                     0x00, 0x04, 0x0a };
                    ssize_t w = write(g_vhci_fd, ev, sizeof(ev));
                    (void)w;
                    break;
                }
                case 0x2003: { /* HCI_OP_LE_READ_LOCAL_FEATURES */
                    uint8_t ev[15] = { 0x04, 0x0e, 12, 0x01, 0x03, 0x20, 0x00 };
                    ssize_t w = write(g_vhci_fd, ev, sizeof(ev));
                    (void)w;
                    break;
                }
                case 0x201c: { /* HCI_OP_LE_READ_SUPPORTED_STATES */
                    uint8_t ev[15] = { 0x04, 0x0e, 12, 0x01, 0x1c, 0x20, 0x00 };
                    memset(&ev[7], 0xff, 8);
                    ssize_t w = write(g_vhci_fd, ev, sizeof(ev));
                    (void)w;
                    break;
                }
                default: {
                    uint8_t ev[] = { 0x04, 0x0e, 4, 0x01, buf[1], buf[2], 0x00 };
                    ssize_t w = write(g_vhci_fd, ev, sizeof(ev));
                    (void)w;
                    break;
                }
                }
            }
        } else if (buf[0] == 0x02 && n >= 9) {
            /* HCI ACL Data Packet */
            uint8_t ncp[] = { 0x04, 0x13, 0x05, 0x01, 0x40, 0x00, 0x01, 0x00 };
            ssize_t w = write(g_vhci_fd, ncp, sizeof(ncp));
            (void)w;

            if (n >= 11 && buf[7] == 0x05 && buf[8] == 0x00) {
                /* LE Signaling Channel (CID 0x0005) */
                uint8_t code = buf[9];
                uint8_t ident = buf[10];

                if (code == 0x14 && n >= 17) {
                    /* L2CAP_LE_CONN_REQ */
                    uint16_t scid = buf[15] | (buf[16] << 8);
                    __atomic_store_n(&g_remote_scid, scid, __ATOMIC_RELEASE);

                    uint8_t conn_rsp[] = {
                        0x02, 0x40, 0x20, 18, 0x00, 14, 0x00, 0x05, 0x00,
                        0x15, ident, 10, 0x00,
                        0x40, 0x00, /* dcid = 0x0040 */
                        0xa0, 0x02, /* mtu = 672 */
                        0xa0, 0x02, /* mps = 672 */
                        0x0a, 0x00, /* credits = 10 */
                        0x00, 0x00  /* result = Success */
                    };
                    w = write(g_vhci_fd, conn_rsp, sizeof(conn_rsp));
                    (void)w;
                    __atomic_store_n(&g_connected, 1, __ATOMIC_RELEASE);
                } else if (code == 0x16) {
                    /* L2CAP_LE_CREDITS */
                    __atomic_store_n(&g_latest_ident, ident, __ATOMIC_RELEASE);

                    /* Immediately write back L2CAP_COMMAND_REJ for ident */
                    uint8_t rej[] = {
                        0x02, 0x40, 0x20, 10, 0x00, 6, 0x00, 0x05, 0x00,
                        0x01, ident, 2, 0x00, 0x00, 0x00
                    };
                    w = write(g_vhci_fd, rej, sizeof(rej));
                    (void)w;
                } else if (code == 0x06 && n >= 15) {
                    /* L2CAP_DISCONN_REQ */
                    uint8_t disconn_rsp[] = {
                        0x02, 0x40, 0x20, 8, 0x00, 4, 0x00, 0x05, 0x00,
                        0x07, ident, 4, 0x00,
                        buf[11], buf[12], buf[13], buf[14]
                    };
                    w = write(g_vhci_fd, disconn_rsp, sizeof(disconn_rsp));
                    (void)w;
                }
            }
        }
    }
    return NULL;
}

static void *sprayer_thread(void *arg)
{
    (void)arg;
    PIN_TO_CPU(1);

    uint8_t rej[15] = {
        0x02, 0x40, 0x20, 10, 0x00, 6, 0x00, 0x05, 0x00,
        0x01, 0x02, 2, 0x00, 0x00, 0x00
    };

    while (!__atomic_load_n(&g_stop, __ATOMIC_RELAXED)) {
        if (!__atomic_load_n(&g_connected, __ATOMIC_ACQUIRE)) {
            usleep(1000);
            continue;
        }

        uint8_t curr = __atomic_load_n(&g_latest_ident, __ATOMIC_RELAXED);
        for (int i = 0; i <= 8; i++) {
            uint8_t id = (curr + i) & 0xff;
            if (id == 0)
                continue;
            rej[10] = id;
            ssize_t w = write(g_vhci_fd, rej, sizeof(rej));
            (void)w;
        }

        for (uint8_t id = 1; id <= 16; id++) {
            rej[10] = id;
            ssize_t w = write(g_vhci_fd, rej, sizeof(rej));
            (void)w;
        }

        usleep(30);
    }
    return NULL;
}

static void *sprayer_thread2(void *arg)
{
    (void)arg;
    PIN_TO_CPU(2);

    uint8_t rej[15] = {
        0x02, 0x40, 0x20, 10, 0x00, 6, 0x00, 0x05, 0x00,
        0x01, 0x02, 2, 0x00, 0x00, 0x00
    };

    while (!__atomic_load_n(&g_stop, __ATOMIC_RELAXED)) {
        if (!__atomic_load_n(&g_connected, __ATOMIC_ACQUIRE)) {
            usleep(1000);
            continue;
        }

        uint8_t curr = __atomic_load_n(&g_latest_ident, __ATOMIC_RELAXED);
        for (int i = 0; i <= 16; i++) {
            uint8_t id = (curr + i) & 0xff;
            if (id == 0)
                continue;
            rej[10] = id;
            ssize_t w = write(g_vhci_fd, rej, sizeof(rej));
            (void)w;
        }

        usleep(30);
    }
    return NULL;
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("[*] Starting reproducer for WARNING in l2cap_chan_le_send_credits...\n");

    g_vhci_fd = open("/dev/vhci", O_RDWR | O_NONBLOCK);
    if (g_vhci_fd < 0) {
        printf("[-] Failed to open /dev/vhci: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] open /dev/vhci successful.\n");

    /* Vendor packet: type 0xff (HCI_VENDOR_PKT), opcode 0x00 (HCI_PRIMARY) */
    uint8_t create_cmd[2] = { 0xff, 0x00 };
    ssize_t written = write(g_vhci_fd, create_cmd, sizeof(create_cmd));
    if (written < 0) {
        printf("[-] Failed to write create_cmd to /dev/vhci: %s\n", strerror(errno));
        close(g_vhci_fd);
        exit(1);
    }
    printf("[+] write create_cmd to /dev/vhci successful.\n");

    uint8_t resp[4];
    ssize_t read_bytes = -1;
    for (int attempts = 0; attempts < 50; attempts++) {
        read_bytes = read(g_vhci_fd, resp, sizeof(resp));
        if (read_bytes >= 4)
            break;
        usleep(10000);
    }
    if (read_bytes < 4) {
        printf("[-] Failed to read controller creation response from /dev/vhci: %s\n", strerror(errno));
        close(g_vhci_fd);
        exit(1);
    }
    uint16_t dev_id = resp[2] | (resp[3] << 8);
    printf("[+] Controller created: dev_id = %u\n", dev_id);

    pthread_t reader_th;
    int th_res = pthread_create(&reader_th, NULL, reader_thread, NULL);
    if (th_res != 0) {
        printf("[-] Failed to create reader_thread: %s\n", strerror(th_res));
        close(g_vhci_fd);
        exit(1);
    }
    printf("[+] Controller reader thread started.\n");

    int hci_sock = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
    if (hci_sock < 0) {
        printf("[-] Failed to create HCI raw socket: %s\n", strerror(errno));
        close(g_vhci_fd);
        exit(1);
    }
    printf("[+] socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI) successful.\n");

    int ioctl_res = ioctl(hci_sock, HCIDEVUP, dev_id);
    if (ioctl_res < 0 && errno != EALREADY) {
        printf("[-] Failed to bring up HCI device via HCIDEVUP: %s\n", strerror(errno));
        close(hci_sock);
        close(g_vhci_fd);
        exit(1);
    }
    printf("[+] Bring up HCI device (HCIDEVUP %u) successful (res=%d, errno=%d).\n",
           dev_id, ioctl_res, errno);

    int l2cap_sock = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
    if (l2cap_sock < 0) {
        printf("[-] Failed to create L2CAP socket: %s\n", strerror(errno));
        close(hci_sock);
        close(g_vhci_fd);
        exit(1);
    }
    printf("[+] socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP) successful.\n");

    struct sockaddr_l2 src_addr;
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.l2_family = AF_BLUETOOTH;
    src_addr.l2_bdaddr_type = BDADDR_LE_PUBLIC;

    int bind_res = bind(l2cap_sock, (struct sockaddr *)&src_addr, sizeof(src_addr));
    if (bind_res < 0) {
        printf("[-] Failed to bind L2CAP socket: %s\n", strerror(errno));
        close(l2cap_sock);
        close(hci_sock);
        close(g_vhci_fd);
        exit(1);
    }
    printf("[+] bind L2CAP socket successful.\n");

    int mode = BT_MODE_LE_FLOWCTL;
    int opt_res = setsockopt(l2cap_sock, SOL_BLUETOOTH, BT_MODE, &mode, sizeof(mode));
    if (opt_res < 0) {
        printf("[-] Failed to set BT_MODE to BT_MODE_LE_FLOWCTL: %s\n", strerror(errno));
        close(l2cap_sock);
        close(hci_sock);
        close(g_vhci_fd);
        exit(1);
    }
    printf("[+] setsockopt(l2cap_sock, SOL_BLUETOOTH, BT_MODE, BT_MODE_LE_FLOWCTL) successful.\n");

    struct timeval tv = { .tv_sec = 5, .tv_usec = 0 };
    setsockopt(l2cap_sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    setsockopt(l2cap_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    struct sockaddr_l2 dst_addr;
    memset(&dst_addr, 0, sizeof(dst_addr));
    dst_addr.l2_family = AF_BLUETOOTH;
    dst_addr.l2_psm = htole16(0x0027);
    dst_addr.l2_bdaddr_type = BDADDR_LE_PUBLIC;
    dst_addr.l2_bdaddr.b[0] = 0x11;
    dst_addr.l2_bdaddr.b[1] = 0x22;
    dst_addr.l2_bdaddr.b[2] = 0x33;
    dst_addr.l2_bdaddr.b[3] = 0x44;
    dst_addr.l2_bdaddr.b[4] = 0x55;
    dst_addr.l2_bdaddr.b[5] = 0x66;

    printf("[*] Connecting L2CAP socket to remote LE device...\n");
    int conn_res = connect(l2cap_sock, (struct sockaddr *)&dst_addr, sizeof(dst_addr));
    if (conn_res < 0) {
        printf("[-] Failed to connect L2CAP socket: %s\n", strerror(errno));
        close(l2cap_sock);
        close(hci_sock);
        close(g_vhci_fd);
        exit(1);
    }
    printf("[+] L2CAP socket connected successfully (BT_CONNECTED).\n");

    while (!__atomic_load_n(&g_connected, __ATOMIC_ACQUIRE))
        usleep(1000);

    pthread_t sprayer_th;
    th_res = pthread_create(&sprayer_th, NULL, sprayer_thread, NULL);
    if (th_res != 0) {
        printf("[-] Failed to create sprayer_thread: %s\n", strerror(th_res));
        close(l2cap_sock);
        close(hci_sock);
        close(g_vhci_fd);
        exit(1);
    }
    printf("[+] Rejection sprayer thread 1 started on CPU 1.\n");

    pthread_t sprayer_th2;
    int has_th2 = 0;
    long nprocs = sysconf(_SC_NPROCESSORS_ONLN);
    if (nprocs > 2) {
        th_res = pthread_create(&sprayer_th2, NULL, sprayer_thread2, NULL);
        if (th_res == 0) {
            has_th2 = 1;
            printf("[+] Rejection sprayer thread 2 started on CPU 2.\n");
        }
    }

    PIN_TO_CPU(0);
    printf("[*] Starting credit return race loop on CPU 0...\n");
    TIMER_START(timer);

    int iter = 0;
    while (TIMER_NOT_EXPIRED(timer, 12.0)) {
        uint16_t scid = __atomic_load_n(&g_remote_scid, __ATOMIC_ACQUIRE);
        if (scid == 0)
            scid = 0x0040;

        uint8_t sdu_pkt[] = {
            0x02, 0x40, 0x20, 10, 0x00, 6, 0x00,
            (uint8_t)(scid & 0xff), (uint8_t)((scid >> 8) & 0xff),
            0x04, 0x00, 'D', 'A', 'T', 'A'
        };
        ssize_t w = write(g_vhci_fd, sdu_pkt, sizeof(sdu_pkt));
        if (w < 0) {
            printf("[-] Failed to write sdu_pkt: %s\n", strerror(errno));
            break;
        }

        struct pollfd pfd = { .fd = l2cap_sock, .events = POLLIN };
        int p = poll(&pfd, 1, 10);
        char recv_buf[128];
        if (p > 0 && (pfd.revents & POLLIN)) {
            recv(l2cap_sock, recv_buf, sizeof(recv_buf), 0);
        } else {
            recv(l2cap_sock, recv_buf, sizeof(recv_buf), MSG_DONTWAIT);
        }

        iter++;
        if (iter % 1000 == 0) {
            printf("[+] Race loop running: iteration %d (elapsed: %.1fs)...\n",
                   iter, timer_elapsed_sec(&timer));
        }
        usleep(20);
    }

    printf("[+] Completed %d race iterations.\n", iter);
    __atomic_store_n(&g_stop, 1, __ATOMIC_RELEASE);

    pthread_join(sprayer_th, NULL);
    if (has_th2)
        pthread_join(sprayer_th2, NULL);

    int close_res = close(l2cap_sock);
    if (close_res < 0)
        printf("[-] Failed to close l2cap_sock: %s\n", strerror(errno));
    else
        printf("[+] close(l2cap_sock) successful.\n");

    close_res = close(hci_sock);
    if (close_res < 0)
        printf("[-] Failed to close hci_sock: %s\n", strerror(errno));
    else
        printf("[+] close(hci_sock) successful.\n");

    close_res = close(g_vhci_fd);
    if (close_res < 0)
        printf("[-] Failed to close g_vhci_fd: %s\n", strerror(errno));
    else
        printf("[+] close(g_vhci_fd) successful.\n");

    pthread_join(reader_th, NULL);

    printf("[*] Waiting for asynchronous kernel execution and trace output...\n");
    sleep(2);

    printf("[+] Reproducer finished.\n");
    return 0;
}
