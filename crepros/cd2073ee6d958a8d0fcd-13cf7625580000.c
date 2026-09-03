// https://syzkaller.appspot.com/bug?id=2f03051571c4a7136b1304007cb0f833c31202dc
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

#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

volatile int step = 0;
void *addr;

void *thread0(void *arg) {
    PIN_TO_CPU(0);
    
    WAIT_ON(&step, 1);
    
    // Fault in the page on CPU 0.
    // It gets added to CPU 0's lru_add batch, but not yet on the LRU list.
    *(volatile char *)addr = 1;
    printf("[+] thread0: faulted in page on CPU 0\n");
    
    SIGNAL(&step, 2);
    
    // Spin to prevent CPU 0 from draining its lru_add batch.
    WAIT_ON(&step, 5);
    
    return NULL;
}

void *thread1(void *arg) {
    PIN_TO_CPU(1);
    
    WAIT_ON(&step, 2);
    
    // mlock the page on CPU 1.
    // It adds the page to CPU 1's mlock_fbatch with LRU_FOLIO, then drains it.
    // __mlock_folio sees the page is not on LRU (still in CPU 0's batch) and does nothing.
    int res = mlock(addr, 4096);
    if (res < 0) {
        printf("[-] Failed to mlock: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] mlock successful.\n");
    
    // munmap the page on CPU 1.
    // Since the VMA is locked, it calls munlock_vma_folio, which adds the page
    // to CPU 1's mlock_fbatch with NO flags. munmap does not drain the batch.
    res = munmap(addr, 4096);
    if (res < 0) {
        printf("[-] Failed to munmap: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] munmap successful.\n");
    
    SIGNAL(&step, 3);
    
    // Sleep until the test finishes.
    WAIT_ON(&step, 5);
    
    return NULL;
}

int main() {
    SETUP_UNBUFFERED_IO();
    
    // Pin main thread to CPU 2+ to avoid interfering with CPU 0 and 1.
    // If CPU 2 is not available, it will just fail and run anywhere.
    cpu_set_t set;
    CPU_ZERO(&set);
    for (int i = 2; i < 8; i++) {
        CPU_SET(i, &set);
    }
    int res = sched_setaffinity(0, sizeof(set), &set);
    if (res < 0) {
        printf("[-] Failed to sched_setaffinity (ignoring): %s\n", strerror(errno));
    } else {
        printf("[+] sched_setaffinity successful.\n");
    }
    
    // Ensure CPU 1 is online initially.
    int fd = open("/sys/devices/system/cpu/cpu1/online", O_WRONLY);
    if (fd < 0) {
        printf("[-] Failed to open /sys/devices/system/cpu/cpu1/online: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] open /sys/devices/system/cpu/cpu1/online successful.\n");
    
    res = write(fd, "1\n", 2);
    if (res < 0) {
        printf("[-] Failed to write to /sys/devices/system/cpu/cpu1/online: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] write to /sys/devices/system/cpu/cpu1/online successful (CPU 1 is online).\n");
    
    res = close(fd);
    if (res < 0) {
        printf("[-] Failed to close fd: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] close fd successful.\n");
    
    addr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED) {
        printf("[-] Failed to mmap: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] mmap successful.\n");
    
    pthread_t t0, t1;
    res = pthread_create(&t0, NULL, thread0, NULL);
    if (res != 0) {
        printf("[-] Failed to pthread_create t0: %s\n", strerror(res));
        exit(1);
    }
    printf("[+] pthread_create t0 successful.\n");
    
    res = pthread_create(&t1, NULL, thread1, NULL);
    if (res != 0) {
        printf("[-] Failed to pthread_create t1: %s\n", strerror(res));
        exit(1);
    }
    printf("[+] pthread_create t1 successful.\n");
    
    SIGNAL(&step, 1);
    
    WAIT_ON(&step, 3);
    
    // Offline CPU 1.
    // This triggers page_alloc_cpu_dead -> mlock_drain_remote on a surviving CPU.
    // mlock_drain_remote calls __munlock_folio, which sees the page is not on LRU,
    // skips taking the LRU lock, and calls __zone_stat_mod_folio with preemption enabled,
    // triggering the warning.
    fd = open("/sys/devices/system/cpu/cpu1/online", O_WRONLY);
    if (fd < 0) {
        printf("[-] Failed to open /sys/devices/system/cpu/cpu1/online: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] open /sys/devices/system/cpu/cpu1/online successful.\n");
    
    res = write(fd, "0\n", 2);
    if (res < 0) {
        printf("[-] Failed to write to /sys/devices/system/cpu/cpu1/online: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] write to /sys/devices/system/cpu/cpu1/online successful (CPU 1 offlined).\n");
    
    res = close(fd);
    if (res < 0) {
        printf("[-] Failed to close fd: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] close fd successful.\n");
    
    // Wait a bit to ensure the asynchronous warning is triggered and printed.
    sleep(1);
    
    SIGNAL(&step, 5);
    
    res = pthread_join(t0, NULL);
    if (res != 0) {
        printf("[-] Failed to pthread_join t0: %s\n", strerror(res));
        exit(1);
    }
    printf("[+] pthread_join t0 successful.\n");
    
    res = pthread_join(t1, NULL);
    if (res != 0) {
        printf("[-] Failed to pthread_join t1: %s\n", strerror(res));
        exit(1);
    }
    printf("[+] pthread_join t1 successful.\n");
    
    // Restore CPU 1.
    fd = open("/sys/devices/system/cpu/cpu1/online", O_WRONLY);
    if (fd < 0) {
        printf("[-] Failed to open /sys/devices/system/cpu/cpu1/online: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] open /sys/devices/system/cpu/cpu1/online successful.\n");
    
    res = write(fd, "1\n", 2);
    if (res < 0) {
        printf("[-] Failed to write to /sys/devices/system/cpu/cpu1/online: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] write to /sys/devices/system/cpu/cpu1/online successful (CPU 1 restored).\n");
    
    res = close(fd);
    if (res < 0) {
        printf("[-] Failed to close fd: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] close fd successful.\n");
    
    printf("[+] Done\n");
    return 0;
}