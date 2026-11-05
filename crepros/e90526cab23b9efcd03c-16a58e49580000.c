// https://syzkaller.appspot.com/bug?id=bbcc9b024a051e0058a5ac6bd85ba90aaeb9e6d2
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#define NBD_SET_SOCK _IO(0xab, 0)
#define NBD_DO_IT _IO(0xab, 3)
#define NBD_CLEAR_SOCK _IO(0xab, 4)
#define NBD_DISCONNECT _IO(0xab, 8)
#define NBD_SET_FLAGS _IO(0xab, 10)
#define NBD_FLAG_CAN_MULTI_CONN (1 << 8)

int fail_nth_fd = -1;

void sigalrm_handler(int sig) {
    /* Just interrupt the blocking wait_event_interruptible in NBD_DO_IT */
}

int set_scheduler(int nbd_idx, const char *sched) {
    char path[256];
    snprintf(path, sizeof(path), "/sys/block/nbd%d/queue/scheduler", nbd_idx);
    int fd = open(path, O_WRONLY);
    if (fd >= 0) {
        ssize_t ret = write(fd, sched, strlen(sched));
        close(fd);
        if (ret > 0) {
            printf("[+] Successfully set scheduler %s for nbd%d\n", sched, nbd_idx);
            return 1;
        }
    }
    return 0;
}

void setup_scheduler(int nbd_idx) {
    /* The bug requires an active IO scheduler to be attached before the update */
    if (!set_scheduler(nbd_idx, "mq-deadline")) {
        if (!set_scheduler(nbd_idx, "bfq")) {
            set_scheduler(nbd_idx, "kyber");
        }
    }
}

void set_fail_nth(int n) {
    if (fail_nth_fd >= 0) {
        char buf[32];
        int len = snprintf(buf, sizeof(buf), "%d", n);
        if (write(fail_nth_fd, buf, len) < 0) {
            /* Ignore write errors */
        }
    }
}

int main() {
    int sv[2];
    struct itimerval it;

    printf("[*] Starting reproducer...\n");

    /* Pre-open fail-nth so we can clear it later without triggering memory allocations */
    fail_nth_fd = open("/proc/self/fail-nth", O_WRONLY);
    if (fail_nth_fd < 0) {
        printf("[-] Failed to open /proc/self/fail-nth: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] open /proc/self/fail-nth successful.\n");

    if (signal(SIGALRM, sigalrm_handler) == SIG_ERR) {
        printf("[-] Failed to signal: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] signal successful.\n");

    /* Iterate fail_nth to precisely hit the allocation in blk_mq_prealloc_tag_set_tags */
    for (int fail_nth = 1; fail_nth <= 250; fail_nth++) {
        printf("[*] Iteration %d, fail_nth=%d\n", fail_nth, fail_nth);
        
        /* Cycle through NBD devices to ensure a fresh config for each iteration */
        int nbd_idx = fail_nth % 16;
        char dev_name[32];
        snprintf(dev_name, sizeof(dev_name), "/dev/nbd%d", nbd_idx);

        int nbd_fd = open(dev_name, O_RDWR);
        if (nbd_fd < 0) {
            printf("[-] Failed to open %s: %s\n", dev_name, strerror(errno));
            exit(1);
        }
        printf("[+] open %s successful.\n", dev_name);

        if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0) {
            printf("[-] Failed to socketpair: %s\n", strerror(errno));
            exit(1);
        }
        printf("[+] socketpair successful.\n");
        
        /* Attach 2 sockets to increase num_connections to 2 */
        if (ioctl(nbd_fd, NBD_SET_SOCK, sv[0]) < 0) {
            printf("[-] Failed to ioctl NBD_SET_SOCK (0): %s\n", strerror(errno));
            exit(1);
        }
        printf("[+] ioctl NBD_SET_SOCK (0) successful.\n");

        if (ioctl(nbd_fd, NBD_SET_SOCK, sv[1]) < 0) {
            printf("[-] Failed to ioctl NBD_SET_SOCK (1): %s\n", strerror(errno));
            exit(1);
        }
        printf("[+] ioctl NBD_SET_SOCK (1) successful.\n");

        if (ioctl(nbd_fd, NBD_SET_FLAGS, NBD_FLAG_CAN_MULTI_CONN) < 0) {
            printf("[-] Failed to ioctl NBD_SET_FLAGS: %s\n", strerror(errno));
            exit(1);
        }
        printf("[+] ioctl NBD_SET_FLAGS successful.\n");

        setup_scheduler(nbd_idx);

        /* Set a timer to interrupt NBD_DO_IT since it will block regardless of the error */
        it.it_value.tv_sec = 0;
        it.it_value.tv_usec = 100000; /* 100ms */
        it.it_interval.tv_sec = 0;
        it.it_interval.tv_usec = 0;
        if (setitimer(ITIMER_REAL, &it, NULL) < 0) {
            printf("[-] Failed to setitimer: %s\n", strerror(errno));
            exit(1);
        }
        printf("[+] setitimer successful.\n");

        /* Inject fault */
        set_fail_nth(fail_nth);

        /* Trigger the buggy queue update */
        int ioctl_res = ioctl(nbd_fd, NBD_DO_IT, 0);
        if (ioctl_res < 0) {
            printf("[-] ioctl NBD_DO_IT returned error (expected if interrupted or failed): %s\n", strerror(errno));
        } else {
            printf("[+] ioctl NBD_DO_IT successful.\n");
        }
        
        /* Disable fault injection safely using the pre-opened fd */
        set_fail_nth(0);

        /* Disable timer */
        it.it_value.tv_usec = 0;
        if (setitimer(ITIMER_REAL, &it, NULL) < 0) {
            printf("[-] Failed to disable setitimer: %s\n", strerror(errno));
            exit(1);
        }
        printf("[+] disable setitimer successful.\n");

        /* Trigger the KASAN out-of-bounds read by freeing the mismatched elevator */
        set_scheduler(nbd_idx, "none");

        /* Cleanup and prepare for the next iteration */
        ioctl(nbd_fd, NBD_DISCONNECT, 0);
        ioctl(nbd_fd, NBD_CLEAR_SOCK, 0);

        close(sv[0]);
        close(sv[1]);
        close(nbd_fd);
        
        /* Allow a brief moment for async recv threads to exit and drop config refs */
        usleep(10000); 
    }

    printf("[+] Reproducer finished.\n");
    return 0;
}
