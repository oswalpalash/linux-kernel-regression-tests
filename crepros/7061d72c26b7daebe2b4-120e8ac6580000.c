// https://syzkaller.appspot.com/bug?id=43d38a27e40a402515b25a46a837bad3aec09895
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
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

int fds[32];
int num_fds = 0;

char target_device[256] = {0};
char target_driver[256] = {0};

void find_device() {
    DIR *dir = opendir("/sys/bus/platform/devices");
    if (!dir) {
        printf("[-] Failed to opendir /sys/bus/platform/devices: %s\n", strerror(errno));
        exit(1);
    }
    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (strncmp(ent->d_name, "snd_aloop", 9) == 0) {
            strcpy(target_device, ent->d_name);
            strcpy(target_driver, "snd_aloop");
            break;
        }
        if (strncmp(ent->d_name, "snd_dummy", 9) == 0) {
            strcpy(target_device, ent->d_name);
            strcpy(target_driver, "snd_dummy");
            break;
        }
    }
    closedir(dir);
    printf("[+] find_device successful.\n");
}

void *unbind_thread(void *arg) {
    PIN_TO_CPU(0);
    struct sched_param param;
    param.sched_priority = 99;
    if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
        printf("[-] Failed to sched_setscheduler FIFO 99: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] sched_setscheduler FIFO 99 successful.\n");

    char path[256];
    snprintf(path, sizeof(path), "/sys/bus/platform/drivers/%s/unbind", target_driver);
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        printf("[-] Failed to open unbind: %s\n", strerror(errno));
        exit(1);
    }
    int res = write(fd, target_device, strlen(target_device));
    if (res < 0) {
        printf("[-] Failed to write unbind: %s\n", strerror(errno));
        exit(1);
    }
    close(fd);
    printf("[+] unbind successful.\n");
    return NULL;
}

void *close_thread(void *arg) {
    PIN_TO_CPU(0);
    struct sched_param param;
    param.sched_priority = 98;
    if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
        printf("[-] Failed to sched_setscheduler FIFO 98: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] sched_setscheduler FIFO 98 successful.\n");

    for (int i = 0; i < num_fds; i++) {
        if (fds[i] >= 0) {
            int res = close(fds[i]);
            if (res < 0) {
                printf("[-] Failed to close fd: %s\n", strerror(errno));
                exit(1);
            }
            fds[i] = -1;
        }
    }
    printf("[+] close_thread successful.\n");
    return NULL;
}

int main() {
    SETUP_UNBUFFERED_IO();
    
    find_device();
    if (target_device[0] == '\0') {
        printf("[-] No snd_aloop or snd_dummy device found.\n");
        exit(1);
    }
    printf("[+] Target device: %s, driver: %s\n", target_device, target_driver);
    
    // Initial unbind to ensure clean state
    char unbind_path[256];
    snprintf(unbind_path, sizeof(unbind_path), "/sys/bus/platform/drivers/%s/unbind", target_driver);
    int ufd = open(unbind_path, O_WRONLY);
    if (ufd >= 0) {
        write(ufd, target_device, strlen(target_device));
        close(ufd);
    }
    
    TIMER_START(start);
    while (TIMER_NOT_EXPIRED(start, 10.0)) {
        char path[256];
        snprintf(path, sizeof(path), "/sys/bus/platform/drivers/%s/bind", target_driver);
        int fd = open(path, O_WRONLY);
        if (fd < 0) {
            printf("[-] Failed to open bind: %s\n", strerror(errno));
            exit(1);
        }
        int res = write(fd, target_device, strlen(target_device));
        if (res < 0) {
            printf("[-] Failed to write bind: %s\n", strerror(errno));
            exit(1);
        }
        close(fd);
        printf("[+] bind successful.\n");
        
        num_fds = 0;
        int retries = 10;
        while (retries-- > 0 && num_fds == 0) {
            for (int i = 0; i < 8; i++) {
                char devpath[64];
                sprintf(devpath, "/dev/snd/controlC%d", i);
                int cfd = open(devpath, O_RDONLY);
                if (cfd >= 0) fds[num_fds++] = cfd;
                
                sprintf(devpath, "/dev/snd/pcmC%dD0p", i);
                cfd = open(devpath, O_RDONLY);
                if (cfd >= 0) fds[num_fds++] = cfd;
                
                sprintf(devpath, "/dev/snd/pcmC%dD0c", i);
                cfd = open(devpath, O_RDONLY);
                if (cfd >= 0) fds[num_fds++] = cfd;
            }
            if (num_fds == 0) usleep(50000);
        }
        if (num_fds == 0) {
            printf("[-] Failed to open any sound devices after retries.\n");
            exit(1);
        }
        printf("[+] Opened %d sound devices.\n", num_fds);
        
        pthread_t t1, t2;
        res = pthread_create(&t1, NULL, unbind_thread, NULL);
        if (res != 0) {
            printf("[-] Failed to pthread_create t1: %s\n", strerror(res));
            exit(1);
        }
        printf("[+] pthread_create t1 successful.\n");
        
        // Give the unbind thread plenty of time to run and block in wait_for_completion
        usleep(50000); 
        
        res = pthread_create(&t2, NULL, close_thread, NULL);
        if (res != 0) {
            printf("[-] Failed to pthread_create t2: %s\n", strerror(res));
            exit(1);
        }
        printf("[+] pthread_create t2 successful.\n");
        
        res = pthread_join(t1, NULL);
        if (res != 0) {
            printf("[-] Failed to pthread_join t1: %s\n", strerror(res));
            exit(1);
        }
        res = pthread_join(t2, NULL);
        if (res != 0) {
            printf("[-] Failed to pthread_join t2: %s\n", strerror(res));
            exit(1);
        }
        printf("[+] pthread_join successful.\n");
    }
    
    return 0;
}
