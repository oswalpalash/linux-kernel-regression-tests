// https://syzkaller.appspot.com/bug?id=08ab0b3d81c97da9356e26547a679d4a63a73275
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/mount.h>
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


int fd_power;
int fd_zoned;
int start_flag = 0;
int delay_iters = 0;

void *thread_power(void *arg)
{
    PIN_TO_CPU(0);
    WAIT_ON(&start_flag, 1);
    
    // Write '1' to power to initiate null_add_dev()
    int res = write(fd_power, "1\n", 2);
    (void)res;
    return NULL;
}

void *thread_zoned(void *arg)
{
    PIN_TO_CPU(1);
    WAIT_ON(&start_flag, 1);
    
    // Small busy-wait delay to let the power thread pass the first `dev->zoned` check.
    // The race window is during `blk_mq_alloc_disk`, which takes some time.
    for (volatile int i = 0; i < delay_iters; i++);
    
    // Write '1' to zoned to flip the value before the second check
    int res = write(fd_zoned, "1\n", 2);
    (void)res;
    return NULL;
}

int main(void)
{
    SETUP_UNBUFFERED_IO();
    
    int res;
    struct stat st;

    printf("[*] Starting reproducer...\n");

    res = stat("/sys/kernel/config", &st);
    if (res < 0) {
        printf("[-] Failed to stat /sys/kernel/config: %s\n", strerror(errno));
    } else {
        printf("[+] stat /sys/kernel/config successful.\n");
    }

    res = mount("none", "/sys/kernel/config", "configfs", 0, NULL);
    if (res < 0 && errno != EBUSY) {
        printf("[-] Failed to mount configfs: %s\n", strerror(errno));
    } else {
        printf("[+] mount configfs successful.\n");
    }

    res = stat("/sys/kernel/config/nullb", &st);
    if (res < 0) {
        printf("[-] Failed to stat /sys/kernel/config/nullb: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] stat /sys/kernel/config/nullb successful.\n");

    TIMER_START(start);
    int dev_idx = 0;
    
    // Run for up to 10 seconds to hit the race window
    while (TIMER_NOT_EXPIRED(start, 10.0)) {
        char path[256];
        char power_path[256];
        char zoned_path[256];
        
        snprintf(path, sizeof(path), "/sys/kernel/config/nullb/nullb%d", dev_idx++);
        res = mkdir(path, 0755);
        if (res < 0) {
            if (errno == EEXIST) {
                rmdir(path);
                res = mkdir(path, 0755);
            }
            if (res < 0) {
                continue;
            }
        }

        snprintf(power_path, sizeof(power_path), "%s/power", path);
        snprintf(zoned_path, sizeof(zoned_path), "%s/zoned", path);

        fd_power = open(power_path, O_WRONLY);
        fd_zoned = open(zoned_path, O_WRONLY);

        if (fd_power < 0 || fd_zoned < 0) {
            if (fd_power >= 0) close(fd_power);
            if (fd_zoned >= 0) close(fd_zoned);
            rmdir(path);
            continue;
        }

        start_flag = 0;
        delay_iters = rand() % 2000;

        pthread_t t1, t2;
        pthread_create(&t1, NULL, thread_power, NULL);
        pthread_create(&t2, NULL, thread_zoned, NULL);

        // Release both threads simultaneously
        SIGNAL(&start_flag, 1);

        pthread_join(t1, NULL);
        pthread_join(t2, NULL);

        close(fd_power);
        close(fd_zoned);

        // Cleanup: rmdir on configfs automatically powers off and drops the device
        rmdir(path);
    }
    
    printf("[+] Reproducer finished.\n");
    // Sleep a bit to allow asynchronous warnings to print
    sleep(2);
    return 0;
}
