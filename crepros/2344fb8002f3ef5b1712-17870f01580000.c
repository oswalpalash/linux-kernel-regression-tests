// https://syzkaller.appspot.com/bug?id=a8e04b0abf03c6e410669271375924bfa0c3fa8e
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#ifndef AF_BLUETOOTH
#define AF_BLUETOOTH 31
#endif

#ifndef BTPROTO_HCI
#define BTPROTO_HCI 1
#endif

#ifndef HCI_CHANNEL_USER
#define HCI_CHANNEL_USER 1
#endif

#ifndef HCIDEVUP
#define HCIDEVUP _IOW('H', 201, int)
#endif

#ifndef HCIDEVDOWN
#define HCIDEVDOWN _IOW('H', 202, int)
#endif

#ifndef HCIDEVRESET
#define HCIDEVRESET _IOW('H', 203, int)
#endif

struct sockaddr_hci {
    sa_family_t    hci_family;
    unsigned short hci_dev;
    unsigned short hci_channel;
};

static atomic_int stop = 0;

/* Controller A: Raw device quirk (0x80) for instant zero-command user-channel toggling */
static int vhci_fd_a = -1;
static int dev_id_a = -1;

/* Controller B: Standard controller (0x00) driven via ioctls with instant command responder */
static int vhci_fd_b = -1;
static int dev_id_b = -1;

/* 7-byte HCI packets matching crash trace */
static const uint8_t acl_pkt[7] = { 0x02, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00 };
static const uint8_t ev_pkt[7]  = { 0x04, 0x0e, 0x04, 0x01, 0x03, 0x0c, 0x00 };
static const uint8_t hw_err[4]  = { 0x04, 0x10, 0x01, 0x00 };

static void pin_to_cpu(int cpu) {
    long ncpus = sysconf(_SC_NPROCESSORS_ONLN);
    if (ncpus < 1)
        ncpus = 1;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu % (int)ncpus, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
}

/* Responds immediately to kernel HCI commands with appropriate Command Complete events */
static void handle_vhci_command(int fd, uint16_t opcode) {
    uint8_t resp[512];
    memset(resp, 0, sizeof(resp));
    resp[0] = 0x04; // HCI_EVENT_PKT
    resp[1] = 0x0e; // HCI_EV_CMD_COMPLETE
    resp[3] = 0x01; // ncmd = 1
    resp[4] = opcode & 0xff;
    resp[5] = (opcode >> 8) & 0xff;
    resp[6] = 0x00; // status = 0 (success)

    size_t plen = 4;
    switch (opcode) {
    case 0x0c03: // HCI_OP_RESET
        plen = 4;
        break;
    case 0x1003: // HCI_OP_READ_LOCAL_FEATURES
        plen = 4 + 8;
        resp[11] = 0x20; // LMP_NO_BREDR (features[4] = 0x20)
        break;
    case 0x1001: // HCI_OP_READ_LOCAL_VERSION
        plen = 4 + 8;
        resp[7] = 6;     // HCI Version 4.0
        resp[10] = 6;    // LMP Version 4.0
        break;
    case 0x1009: // HCI_OP_READ_BD_ADDR
        plen = 4 + 6;
        resp[7] = 0x11; resp[8] = 0x22; resp[9] = 0x33;
        resp[10] = 0x44; resp[11] = 0x55; resp[12] = 0x66;
        break;
    case 0x1005: // HCI_OP_READ_BUFFER_SIZE
        plen = 4 + 7;
        resp[7] = 0x00; resp[8] = 0x04; // ACL MTU 1024
        resp[9] = 0x40;                 // SCO MTU 64
        resp[10] = 0x0a; resp[11] = 0x00; // ACL max pkts 10
        resp[12] = 0x0a; resp[13] = 0x00; // SCO max pkts 10
        break;
    case 0x1002: // HCI_OP_READ_LOCAL_COMMANDS
        plen = 4 + 64;
        break;
    case 0x2002: // HCI_OP_LE_READ_BUFFER_SIZE
        plen = 4 + 3;
        resp[7] = 0x00; resp[8] = 0x04;
        resp[9] = 0x0a;
        break;
    case 0x2003: // HCI_OP_LE_READ_LOCAL_FEATURES
        plen = 4 + 8;
        break;
    case 0x203b: // HCI_OP_LE_READ_BUFFER_SIZE_V2
        plen = 4 + 3;
        resp[7] = 0x00; resp[8] = 0x04;
        resp[9] = 0x0a;
        break;
    default:
        plen = 4;
        break;
    }
    resp[2] = plen;
    ssize_t ret = write(fd, resp, plen + 3);
    (void)ret;
}

static int create_vhci_device(uint8_t opcode, int *out_dev_id) {
    int fd = open("/dev/vhci", O_RDWR | O_NONBLOCK);
    if (fd < 0)
        return -1;

    uint8_t create_pkt[2] = { 0xff, opcode };
    if (write(fd, create_pkt, sizeof(create_pkt)) != (ssize_t)sizeof(create_pkt)) {
        close(fd);
        return -1;
    }

    int dev_id = -1;
    struct pollfd pfd = { .fd = fd, .events = POLLIN };
    if (poll(&pfd, 1, 100) > 0) {
        uint8_t resp[4];
        ssize_t n = read(fd, resp, sizeof(resp));
        if (n >= 4 && resp[0] == 0xff) {
            dev_id = resp[2] | (resp[3] << 8);
        }
    }
    if (out_dev_id)
        *out_dev_id = dev_id;

    return fd;
}

/* Worker 0 (CPU 0): Instant command responder for both controllers */
static void *responder_thread(void *arg) {
    int cpu = (intptr_t)arg;
    pin_to_cpu(cpu);

    uint8_t buf[512];
    while (!atomic_load(&stop)) {
        struct pollfd pfds[2] = {
            { .fd = vhci_fd_a, .events = POLLIN },
            { .fd = vhci_fd_b, .events = POLLIN }
        };
        int pr = poll(pfds, 2, 5);
        if (pr > 0) {
            for (int i = 0; i < 2; i++) {
                if (pfds[i].fd >= 0 && (pfds[i].revents & POLLIN)) {
                    while (!atomic_load(&stop)) {
                        ssize_t n = read(pfds[i].fd, buf, sizeof(buf));
                        if (n <= 0)
                            break;
                        if (buf[0] == 0x01 && n >= 3) {
                            uint16_t opcode = buf[1] | (buf[2] << 8);
                            handle_vhci_command(pfds[i].fd, opcode);
                        }
                    }
                }
            }
        }
    }
    return NULL;
}

/* Worker 1 & 2: High-frequency HCI_CHANNEL_USER cycling on Controller A */
static void *user_channel_cycle_thread(void *arg) {
    int cpu = (intptr_t)arg;
    pin_to_cpu(cpu);

    struct sockaddr_hci haddr = {
        .hci_family = AF_BLUETOOTH,
        .hci_dev = (uint16_t)dev_id_a,
        .hci_channel = HCI_CHANNEL_USER,
    };

    while (!atomic_load(&stop)) {
        int s = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
        if (s < 0) {
            sched_yield();
            continue;
        }

        if (bind(s, (struct sockaddr *)&haddr, sizeof(haddr)) == 0) {
            /* Controller A is now HCI_UP without any initialization commands.
             * Closing immediately executes hci_dev_do_close() and drain_workqueue(). */
            close(s);
        } else {
            close(s);
            sched_yield();
        }
    }
    return NULL;
}

/* Worker 3 & 4: Packet blasters targeting Controller A */
static void *writer_thread_a(void *arg) {
    int cpu = (intptr_t)arg;
    pin_to_cpu(cpu);

    int count = 0;
    while (!atomic_load(&stop)) {
        ssize_t w1 = write(vhci_fd_a, acl_pkt, sizeof(acl_pkt));
        (void)w1;
        ssize_t w2 = write(vhci_fd_a, ev_pkt, sizeof(ev_pkt));
        (void)w2;
        if ((++count & 0x3f) == 0)
            sched_yield();
    }
    return NULL;
}

/* Worker 5: Standard ioctl toggler for Controller B */
static void *ioctl_cycle_thread(void *arg) {
    int cpu = (intptr_t)arg;
    pin_to_cpu(cpu);

    int s = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
    while (!atomic_load(&stop)) {
        if (s < 0) {
            s = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
            if (s < 0) {
                sched_yield();
                continue;
            }
        }

        ioctl(s, HCIDEVRESET, (unsigned long)dev_id_b);
        ioctl(s, HCIDEVDOWN,  (unsigned long)dev_id_b);
        ioctl(s, HCIDEVUP,    (unsigned long)dev_id_b);
        sched_yield();
    }
    if (s >= 0)
        close(s);
    return NULL;
}

/* Worker 6 & 7: Packet and hardware error blasters targeting Controller B */
static void *writer_thread_b(void *arg) {
    int cpu = (intptr_t)arg;
    pin_to_cpu(cpu);

    int count = 0;
    while (!atomic_load(&stop)) {
        ssize_t w1 = write(vhci_fd_b, acl_pkt, sizeof(acl_pkt));
        (void)w1;
        ssize_t w2 = write(vhci_fd_b, ev_pkt, sizeof(ev_pkt));
        (void)w2;
        ssize_t w3 = write(vhci_fd_b, hw_err, sizeof(hw_err));
        (void)w3;
        if ((++count & 0x3f) == 0)
            sched_yield();
    }
    return NULL;
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("[*] Starting reproducer for WARNING in hci_recv_frame...\n");

    /* Step 1: Create Controller A with raw device quirk (0x80) */
    vhci_fd_a = create_vhci_device(0x80, &dev_id_a);
    if (vhci_fd_a < 0) {
        printf("[-] Failed to create Controller A: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] Controller A created successfully (fd=%d, dev_id=hci%d).\n",
           vhci_fd_a, dev_id_a);

    /* Step 2: Poll until Controller A setup completes and accepts HCI_CHANNEL_USER */
    int ready = 0;
    for (int retry = 0; retry < 100; retry++) {
        int ts = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
        if (ts >= 0) {
            struct sockaddr_hci haddr = {
                .hci_family = AF_BLUETOOTH,
                .hci_dev = (uint16_t)dev_id_a,
                .hci_channel = HCI_CHANNEL_USER,
            };
            if (bind(ts, (struct sockaddr *)&haddr, sizeof(haddr)) == 0) {
                close(ts);
                ready = 1;
                printf("[+] Controller A ready for HCI_CHANNEL_USER (attempt %d).\n", retry + 1);
                break;
            }
            close(ts);
        }
        usleep(5000);
    }
    if (!ready) {
        printf("[*] Controller A not bound yet, proceeding with reproduction...\n");
    }

    /* Step 3: Create Controller B with standard controller opcode (0x00) */
    vhci_fd_b = create_vhci_device(0x00, &dev_id_b);
    if (vhci_fd_b < 0) {
        printf("[-] Failed to create Controller B: %s\n", strerror(errno));
        close(vhci_fd_a);
        exit(1);
    }
    printf("[+] Controller B created successfully (fd=%d, dev_id=hci%d).\n",
           vhci_fd_b, dev_id_b);

    /* Step 4: Verify basic Bluetooth socket capability */
    int test_s = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
    if (test_s < 0) {
        printf("[-] Failed to open test AF_BLUETOOTH socket: %s\n", strerror(errno));
        close(vhci_fd_a);
        close(vhci_fd_b);
        exit(1);
    }
    printf("[+] socket AF_BLUETOOTH SOCK_RAW successful.\n");

    int res = close(test_s);
    if (res < 0) {
        printf("[-] Failed to close test AF_BLUETOOTH socket: %s\n", strerror(errno));
        close(vhci_fd_a);
        close(vhci_fd_b);
        exit(1);
    }
    printf("[+] close test AF_BLUETOOTH socket successful.\n");

    /* Step 5: Spawn worker threads distributed across CPU 0 and CPU 1 */
    pthread_t th[8];
    res = pthread_create(&th[0], NULL, responder_thread,          (void *)(intptr_t)0);
    if (res != 0) {
        printf("[-] Failed to create responder thread: %s\n", strerror(res));
        exit(1);
    }
    res = pthread_create(&th[1], NULL, user_channel_cycle_thread, (void *)(intptr_t)1);
    if (res != 0) {
        printf("[-] Failed to create user channel cycler 1: %s\n", strerror(res));
        exit(1);
    }
    res = pthread_create(&th[2], NULL, user_channel_cycle_thread, (void *)(intptr_t)0);
    if (res != 0) {
        printf("[-] Failed to create user channel cycler 2: %s\n", strerror(res));
        exit(1);
    }
    res = pthread_create(&th[3], NULL, writer_thread_a,           (void *)(intptr_t)0);
    if (res != 0) {
        printf("[-] Failed to create writer A 1: %s\n", strerror(res));
        exit(1);
    }
    res = pthread_create(&th[4], NULL, writer_thread_a,           (void *)(intptr_t)1);
    if (res != 0) {
        printf("[-] Failed to create writer A 2: %s\n", strerror(res));
        exit(1);
    }
    res = pthread_create(&th[5], NULL, ioctl_cycle_thread,        (void *)(intptr_t)1);
    if (res != 0) {
        printf("[-] Failed to create ioctl cycler: %s\n", strerror(res));
        exit(1);
    }
    res = pthread_create(&th[6], NULL, writer_thread_b,           (void *)(intptr_t)0);
    if (res != 0) {
        printf("[-] Failed to create writer B 1: %s\n", strerror(res));
        exit(1);
    }
    res = pthread_create(&th[7], NULL, writer_thread_b,           (void *)(intptr_t)1);
    if (res != 0) {
        printf("[-] Failed to create writer B 2: %s\n", strerror(res));
        exit(1);
    }
    printf("[+] Successfully launched all 8 worker threads.\n");

    /* Step 6: Run race condition loop for 18 seconds */
    printf("[*] Running race condition loop for 18 seconds...\n");
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    while (1) {
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        double elapsed = (now.tv_sec - start.tv_sec) + (now.tv_nsec - start.tv_nsec) / 1e9;
        if (elapsed > 18.0)
            break;
        usleep(50000);
    }

    printf("[*] Stopping worker threads...\n");
    atomic_store(&stop, 1);

    for (int i = 0; i < 8; i++) {
        pthread_join(th[i], NULL);
    }
    printf("[+] All worker threads joined successfully.\n");

    /* Step 7: Clean up VHCI controllers */
    res = close(vhci_fd_a);
    if (res < 0) {
        printf("[-] Failed to close Controller A: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] close Controller A successful.\n");

    res = close(vhci_fd_b);
    if (res < 0) {
        printf("[-] Failed to close Controller B: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] close Controller B successful.\n");

    /* Allow asynchronous kernel workqueue teardown to settle */
    sleep(1);
    printf("[+] Reproduction run completed successfully.\n");
    return 0;
}
