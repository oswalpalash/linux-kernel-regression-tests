// https://syzkaller.appspot.com/bug?id=8edbcfdca7bad371d02a53b2e6ed15c6e3afa7e0
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <linux/major.h>
#include <linux/raid/md_u.h>
#include <linux/raid/md_p.h>
#include <linux/loop.h>
#include <pthread.h>
#include <errno.h>
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


int md_fd;
volatile int stop = 0;
char *buf;

void *write_thread(void *arg) {
    PIN_TO_CPU(1);
    while (!__atomic_load_n(&stop, __ATOMIC_ACQUIRE)) {
        /* Submit bios concurrently with STOP_ARRAY */
        pwrite(md_fd, buf, 4096, 0);
    }
    return NULL;
}

int main(void) {
    SETUP_UNBUFFERED_IO();
    PIN_TO_CPU(0);

    /* O_DIRECT requires aligned memory buffers */
    if (posix_memalign((void **)&buf, 4096, 4096) != 0) {
        printf("[-] Failed to posix_memalign: %s\n", strerror(errno));
        exit(1);
    }
    memset(buf, 0, 4096);
    printf("[+] posix_memalign successful.\n");

    char temp_path[] = "/tmp/loop_backing_XXXXXX";
    int file_fd = mkstemp(temp_path);
    if (file_fd < 0) {
        printf("[-] Failed to mkstemp: %s\n", strerror(errno));
        exit(1);
    }
    unlink(temp_path);
    printf("[+] mkstemp successful.\n");

    if (ftruncate(file_fd, 1024 * 1024) < 0) {
        printf("[-] Failed to ftruncate: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] ftruncate successful.\n");

    int control_fd = open("/dev/loop-control", O_RDWR);
    if (control_fd < 0) {
        printf("[-] Failed to open /dev/loop-control: %s\n", strerror(errno));
        exit(1);
    }
    int dev_num = ioctl(control_fd, LOOP_CTL_GET_FREE, 0);
    close(control_fd);
    if (dev_num < 0) {
        printf("[-] Failed to ioctl LOOP_CTL_GET_FREE: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] ioctl LOOP_CTL_GET_FREE successful, dev_num=%d.\n", dev_num);

    char loop_path[64];
    snprintf(loop_path, sizeof(loop_path), "/dev/loop%d", dev_num);
    int loop_fd = open(loop_path, O_RDWR);
    if (loop_fd < 0) {
        printf("[-] Failed to open %s: %s\n", loop_path, strerror(errno));
        exit(1);
    }
    if (ioctl(loop_fd, LOOP_SET_FD, file_fd) < 0) {
        printf("[-] Failed to ioctl LOOP_SET_FD: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] loop device setup successful.\n");

    /* Create and open the MD device node with O_DIRECT to bypass page cache */
    unlink("/dev/md142");
    if (mknod("/dev/md142", S_IFBLK | 0600, makedev(9, 142)) < 0) {
        if (errno != EEXIST) {
            printf("[-] Failed to mknod /dev/md142: %s\n", strerror(errno));
            exit(1);
        }
    }
    printf("[+] mknod /dev/md142 successful.\n");

    md_fd = open("/dev/md142", O_RDWR | O_DIRECT);
    if (md_fd < 0) {
        printf("[-] Failed to open /dev/md142: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] open /dev/md142 successful.\n");

    struct stat st;
    if (stat(loop_path, &st) < 0) {
        printf("[-] Failed to stat %s: %s\n", loop_path, strerror(errno));
        exit(1);
    }
    printf("[+] stat successful.\n");

    pthread_t th;
    if (pthread_create(&th, NULL, write_thread, NULL) != 0) {
        printf("[-] Failed to pthread_create: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] pthread_create successful.\n");

    printf("Starting race loop. The kernel should crash shortly...\n");
    TIMER_START(start);
    while (TIMER_NOT_EXPIRED(start, 10.0)) {
        mdu_array_info_t array_info;
        memset(&array_info, 0, sizeof(array_info));
        array_info.level = 1; /* RAID1 */
        array_info.size = 1024;
        array_info.nr_disks = 1;
        array_info.raid_disks = 1;
        array_info.md_minor = 142;
        array_info.not_persistent = 1;

        if (ioctl(md_fd, SET_ARRAY_INFO, &array_info) < 0) {
            ioctl(md_fd, STOP_ARRAY, 0);
            continue;
        }

        mdu_disk_info_t disk_info;
        memset(&disk_info, 0, sizeof(disk_info));
        disk_info.number = 0;
        disk_info.major = major(st.st_rdev);
        disk_info.minor = minor(st.st_rdev);
        disk_info.raid_disk = 0;
        disk_info.state = (1 << MD_DISK_ACTIVE) | (1 << MD_DISK_SYNC);

        if (ioctl(md_fd, ADD_NEW_DISK, &disk_info) < 0) {
            ioctl(md_fd, STOP_ARRAY, 0);
            continue;
        }
        
        mdu_param_t param;
        memset(&param, 0, sizeof(param));
        if (ioctl(md_fd, RUN_ARRAY, &param) < 0) {
            ioctl(md_fd, STOP_ARRAY, 0);
            continue;
        }

        /* Trigger the teardown race */
        ioctl(md_fd, STOP_ARRAY, 0);
    }

    __atomic_store_n(&stop, 1, __ATOMIC_RELEASE);
    pthread_join(th, NULL);

    close(md_fd);
    ioctl(loop_fd, LOOP_CLR_FD, 0);
    close(loop_fd);
    close(file_fd);

    printf("[+] Race loop finished without crashing.\n");
    return 0;
}