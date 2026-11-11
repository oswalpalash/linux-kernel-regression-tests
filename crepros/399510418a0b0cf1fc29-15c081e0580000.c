// https://syzkaller.appspot.com/bug?id=786268e7098940dbadcc515cdfe5c87eed3ec7c7
#define _GNU_SOURCE
#include <fcntl.h>
#include <pthread.h>
#include <sys/sendfile.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <string.h>
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


int memfd;
int nullfd;
int stop = 0;

void *thread_func(void *arg) {
    int cpu = (int)(long)arg;
    PIN_TO_CPU(cpu);
    
    while (!__atomic_load_n(&stop, __ATOMIC_RELAXED)) {
        // Reset the shared file offset so sendfile can keep reading data
        if (lseek(memfd, 0, SEEK_SET) < 0) {
            printf("[-] Failed to lseek: %s\n", strerror(errno));
            exit(1);
        }
        
        // Call sendfile with a NULL offset. This will read from and update
        // the shared f_pos without taking f_pos_lock, leading to a concurrent
        // update of in->f_ra.prev_pos in shmem_file_splice_read.
        for (int i = 0; i < 16; i++) {
            if (sendfile(nullfd, memfd, NULL, 4096) < 0) {
                printf("[-] Failed to sendfile: %s\n", strerror(errno));
                exit(1);
            }
        }
    }
    return NULL;
}

int main() {
    SETUP_UNBUFFERED_IO();
    
    // Use syscall directly to avoid compilation issues on older glibc versions
    memfd = syscall(SYS_memfd_create, "test", 0);
    if (memfd < 0) {
        printf("[-] Failed to memfd_create: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] memfd_create successful.\n");
    
    // Populate the memfd with enough data to allow multiple sendfile calls
    void *buf = calloc(1, 65536);
    if (!buf) {
        printf("[-] Failed to calloc: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] calloc successful.\n");

    if (write(memfd, buf, 65536) != 65536) {
        printf("[-] Failed to write: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] write successful.\n");
    free(buf);

    nullfd = open("/dev/null", O_WRONLY);
    if (nullfd < 0) {
        printf("[-] Failed to open /dev/null: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] open /dev/null successful.\n");

    pthread_t t1, t2;
    if (pthread_create(&t1, NULL, thread_func, (void *)0) != 0) {
        printf("[-] Failed to create thread 1: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] pthread_create 1 successful.\n");

    if (pthread_create(&t2, NULL, thread_func, (void *)1) != 0) {
        printf("[-] Failed to create thread 2: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] pthread_create 2 successful.\n");

    // Run the race for 5 seconds
    TIMER_START(start);
    while (TIMER_NOT_EXPIRED(start, 5.0)) {
        usleep(100000);
    }
    
    __atomic_store_n(&stop, 1, __ATOMIC_RELAXED);
    
    if (pthread_join(t1, NULL) != 0) {
        printf("[-] Failed to join thread 1: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] pthread_join 1 successful.\n");

    if (pthread_join(t2, NULL) != 0) {
        printf("[-] Failed to join thread 2: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] pthread_join 2 successful.\n");

    if (close(memfd) < 0) {
        printf("[-] Failed to close memfd: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] close memfd successful.\n");

    if (close(nullfd) < 0) {
        printf("[-] Failed to close nullfd: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] close nullfd successful.\n");

    printf("[+] All checks passed.\n");
    return 0;
}