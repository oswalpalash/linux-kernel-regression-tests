// https://syzkaller.appspot.com/bug?id=379463b5468cdd27a76a6a24746da67bbfb21f14
#define _GNU_SOURCE
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <sched.h>
#include <sys/syscall.h>
#include <linux/aio_abi.h>
#include <time.h>
#include <errno.h>

#define NUM_WRITERS 8
#define NUM_READERS 8

volatile int stop = 0;

void *writer_thread(void *arg) {
    PIN_TO_CPU(0);
    long id = (long)arg;
    
    char filename[32];
    sprintf(filename, "./testfile_fsync_%ld", id);
    int fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
        printf("[-] Failed to open %s: %s\n", filename, strerror(errno));
        exit(1);
    }
    
    char *buf = malloc(4096);
    if (!buf) {
        printf("[-] Failed to malloc: %s\n", strerror(errno));
        exit(1);
    }
    memset(buf, 1, 4096);
    if (pwrite(fd, buf, 4096, 0) < 0) {
        printf("[-] Failed to pwrite: %s\n", strerror(errno));
        exit(1);
    }
    if (fsync(fd) < 0) {
        printf("[-] Failed to fsync: %s\n", strerror(errno));
        exit(1);
    }
    
    aio_context_t ctx = 0;
    if (syscall(__NR_io_setup, 128, &ctx) < 0) {
        printf("[-] Failed to io_setup: %s\n", strerror(errno));
        exit(1);
    }
    
    struct iocb cb;
    struct iocb *cbs[1] = {&cb};
    struct io_event events[1];
    
    while (!__atomic_load_n(&stop, __ATOMIC_RELAXED)) {
        // Dirty the file to force fsync to sleep waiting for disk I/O
        if (pwrite(fd, buf, 4096, 0) < 0) {
            // Ignore transient errors
        }
        
        memset(&cb, 0, sizeof(cb));
        cb.aio_fildes = fd;
        cb.aio_lio_opcode = IOCB_CMD_FSYNC;
        
        // Submit AIO fsync. This queues aio_fsync_work on system_percpu_wq (CPU 0's bound pool).
        if (syscall(__NR_io_submit, ctx, 1, cbs) == 1) {
            // Wait for the fsync worker to finish and wake up.
            // This triggers wq_worker_running() on CPU 0's bound pool.
            syscall(__NR_io_getevents, ctx, 1, 1, events, NULL);
        }
    }
    
    syscall(__NR_io_destroy, ctx);
    close(fd);
    unlink(filename);
    free(buf);
    return NULL;
}

void *reader_thread(void *arg) {
    PIN_TO_CPU(1);
    
    int fd_stat = open("/proc/sys/vm/stat_refresh", O_RDONLY);
    int fd_overcommit = -1;
    if (fd_stat < 0) {
        fd_overcommit = open("/proc/sys/vm/overcommit_memory", O_RDWR);
        if (fd_overcommit < 0) {
            printf("[-] Failed to open sysctls: %s\n", strerror(errno));
            exit(1);
        }
    }
    
    char buf[16];
    while (!__atomic_load_n(&stop, __ATOMIC_RELAXED)) {
        if (fd_stat >= 0) {
            // Reading from stat_refresh triggers schedule_on_each_cpu(refresh_vm_stats).
            // This queues work on CPU 0's system_percpu_wq from CPU 1,
            // triggering kick_pool_pick() on CPU 0's pool.
            if (pread(fd_stat, buf, sizeof(buf), 0) < 0) {
                // Ignore transient errors
            }
        } else {
            // Fallback: changing overcommit_memory to NEVER also triggers schedule_on_each_cpu
            if (pwrite(fd_overcommit, "2\n", 2, 0) < 0) {}
            if (pwrite(fd_overcommit, "0\n", 2, 0) < 0) {}
        }
    }
    
    if (fd_stat >= 0) close(fd_stat);
    if (fd_overcommit >= 0) close(fd_overcommit);
    return NULL;
}

void *timer_thread(void *arg) {
    // Run for 4.5 seconds to exit cleanly before the fuzzer sends SIGKILL (at 5.0s).
    // This prevents KCSAN from catching unrelated bugs during process exit.
    usleep(4500000);
    __atomic_store_n(&stop, 1, __ATOMIC_RELAXED);
    return NULL;
}

int main() {
    SETUP_UNBUFFERED_IO();
    
    pthread_t writers[NUM_WRITERS];
    pthread_t readers[NUM_READERS];
    pthread_t timer;
    
    printf("[+] Starting reproducer...\n");
    
    for (long i = 0; i < NUM_WRITERS; i++) {
        if (pthread_create(&writers[i], NULL, writer_thread, (void *)i) != 0) {
            printf("[-] Failed to create writer thread: %s\n", strerror(errno));
            exit(1);
        }
    }
    printf("[+] Writer threads created successfully.\n");
    
    for (long i = 0; i < NUM_READERS; i++) {
        if (pthread_create(&readers[i], NULL, reader_thread, (void *)i) != 0) {
            printf("[-] Failed to create reader thread: %s\n", strerror(errno));
            exit(1);
        }
    }
    printf("[+] Reader threads created successfully.\n");
    
    if (pthread_create(&timer, NULL, timer_thread, NULL) != 0) {
        printf("[-] Failed to create timer thread: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] Timer thread created successfully.\n");
    
    for (int i = 0; i < NUM_WRITERS; i++) {
        pthread_join(writers[i], NULL);
    }
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }
    pthread_join(timer, NULL);
    
    printf("[+] Reproducer finished successfully.\n");
    return 0;
}