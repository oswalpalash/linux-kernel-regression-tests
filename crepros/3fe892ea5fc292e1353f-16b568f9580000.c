// https://syzkaller.appspot.com/bug?id=8edbcfdca7bad371d02a53b2e6ed15c6e3afa7e0
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <linux/major.h>
#include <linux/loop.h>
#include <linux/raid/md_u.h>
#include <linux/raid/md_p.h>
#include <errno.h>
#include <string.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <sched.h>

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


int loop_ctrl_fd;
int loop_fd;
int md_fd;
int mem_fd;
char loop_name[32];
char md_name[32];
void *buf;
volatile int stop = 0;
int loop_num = -1;

mdu_array_info_t array_info = {0};
mdu_disk_info_t disk_info = {0};

void setup_loop() {
    mem_fd = syscall(__NR_memfd_create, "disk", 0);
    if (mem_fd < 0) {
        perror("memfd_create");
        exit(1);
    }
    if (ftruncate(mem_fd, 1024 * 1024 * 10) < 0) { // 10MB
        perror("ftruncate");
        exit(1);
    }

    loop_ctrl_fd = open("/dev/loop-control", O_RDWR);
    if (loop_ctrl_fd < 0) {
        perror("open /dev/loop-control");
        exit(1);
    }

    loop_num = ioctl(loop_ctrl_fd, LOOP_CTL_GET_FREE);
    if (loop_num < 0) {
        perror("LOOP_CTL_GET_FREE");
        exit(1);
    }

    snprintf(loop_name, sizeof(loop_name), "/dev/loop%d", loop_num);
    loop_fd = open(loop_name, O_RDWR);
    if (loop_fd < 0) {
        if (mknod(loop_name, S_IFBLK | 0600, makedev(7, loop_num)) < 0) {
            perror("mknod loop");
            exit(1);
        }
        loop_fd = open(loop_name, O_RDWR);
        if (loop_fd < 0) {
            perror("open loop");
            exit(1);
        }
    }

    if (ioctl(loop_fd, LOOP_SET_FD, mem_fd) < 0) {
        perror("LOOP_SET_FD");
        exit(1);
    }
}

void setup_md(int md_num) {
    snprintf(md_name, sizeof(md_name), "/dev/md%d", md_num);
    if (mknod(md_name, S_IFBLK | 0600, makedev(9, md_num)) < 0 && errno != EEXIST) {
        perror("mknod md");
        exit(1);
    }

    md_fd = open(md_name, O_RDWR | O_DIRECT);
    if (md_fd < 0) {
        perror("open md");
        exit(1);
    }

    array_info.level = 1; // RAID1
    array_info.size = 1024; // 1MB
    array_info.nr_disks = 1;
    array_info.raid_disks = 1;
    array_info.md_minor = md_num;
    array_info.not_persistent = 1;
    array_info.state = 0;
    array_info.active_disks = 1;
    array_info.working_disks = 1;

    disk_info.number = 0;
    disk_info.major = 7;
    disk_info.minor = loop_num;
    disk_info.raid_disk = 0;
    disk_info.state = (1 << MD_DISK_ACTIVE) | (1 << MD_DISK_SYNC);

    ioctl(md_fd, SET_ARRAY_INFO, &array_info);
    ioctl(md_fd, ADD_NEW_DISK, &disk_info);
    ioctl(md_fd, RUN_ARRAY, NULL);
}

void *thread_write(void *arg) {
    PIN_TO_CPU(0);
    while (!stop) {
        pwrite64(md_fd, buf, 4096, 0);
    }
    return NULL;
}

void *thread_stop(void *arg) {
    PIN_TO_CPU(1);
    while (!stop) {
        ioctl(md_fd, STOP_ARRAY, NULL);
        ioctl(md_fd, SET_ARRAY_INFO, &array_info);
        ioctl(md_fd, ADD_NEW_DISK, &disk_info);
        ioctl(md_fd, RUN_ARRAY, NULL);
    }
    return NULL;
}

int main() {
    SETUP_UNBUFFERED_IO();
    
    if (posix_memalign(&buf, 4096, 4096) != 0) {
        perror("posix_memalign");
        exit(1);
    }
    memset(buf, 0x41, 4096);

    setup_loop();
    setup_md(100);

    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread_write, NULL);
    pthread_create(&t2, NULL, thread_stop, NULL);

    TIMER_START(start);
    while (TIMER_NOT_EXPIRED(start, 5.0)) {
        usleep(100000);
    }

    stop = 1;
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("[+] Done.\n");
    exit(0);
}