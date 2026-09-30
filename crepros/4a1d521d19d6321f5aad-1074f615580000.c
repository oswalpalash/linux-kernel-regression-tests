// https://syzkaller.appspot.com/bug?id=901fad1ef89b918e650e5ecccacb1ec279dc28c3
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
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


#define NUM_PRE_CTRLS 32

int next_id = 0;

void my_mkdir(const char *path) {
    if (mkdir(path, 0777) < 0) {
        if (errno != EEXIST) {
            printf("[-] Failed to mkdir %s: %s\n", path, strerror(errno));
        }
    }
}

void my_symlink(const char *target, const char *linkpath) {
    if (symlink(target, linkpath) < 0) {
        if (errno != EEXIST) {
            printf("[-] Failed to symlink %s to %s: %s\n", target, linkpath, strerror(errno));
        }
    }
}

void write_file(const char *path, const char *val) {
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        printf("[-] Failed to open %s: %s\n", path, strerror(errno));
        return;
    }
    if (write(fd, val, strlen(val)) < 0) {
        printf("[-] Failed to write to %s: %s\n", path, strerror(errno));
    }
    close(fd);
}

void setup_nvmet() {
    int fd = open("/dev/nvme-fabrics", O_RDWR);
    if (fd >= 0) {
        // Dummy write to trigger request_module("nvme-loop")
        if (write(fd, "transport=loop,nqn=dummy", 24) < 0) {
            // Expected to fail, but module should be loaded
        }
        close(fd);
    }
    
    my_mkdir("/sys/kernel/config/nvmet");
    my_mkdir("/sys/kernel/config/nvmet/subsystems");
    my_mkdir("/sys/kernel/config/nvmet/subsystems/test");
    write_file("/sys/kernel/config/nvmet/subsystems/test/attr_allow_any_host", "1");
    my_mkdir("/sys/kernel/config/nvmet/ports");
    my_mkdir("/sys/kernel/config/nvmet/ports/1");
    write_file("/sys/kernel/config/nvmet/ports/1/addr_trtype", "loop");
    my_symlink("/sys/kernel/config/nvmet/subsystems/test", "/sys/kernel/config/nvmet/ports/1/subsystems/test");
    printf("[+] nvmet setup successful.\n");
}

void set_wq_max_active() {
    const char *paths[] = {
        "/sys/bus/workqueue/devices/nvme-reset-wq/max_active",
        "/sys/devices/virtual/workqueue/nvme-reset-wq/max_active",
        "/sys/class/workqueue/nvme-reset-wq/max_active"
    };
    for (int i = 0; i < 3; i++) {
        int fd = open(paths[i], O_WRONLY);
        if (fd >= 0) {
            if (write(fd, "1", 1) > 0) {
                printf("[+] Set max_active to 1 via %s\n", paths[i]);
            }
            close(fd);
            return;
        }
    }
    printf("[-] Could not find nvme-reset-wq max_active, proceeding anyway\n");
}

int get_next_nvme_id() {
    DIR *d = opendir("/sys/class/nvme");
    if (!d) return 0;
    int max_id = -1;
    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        if (strncmp(dir->d_name, "nvme", 4) == 0) {
            int id = atoi(dir->d_name + 4);
            if (id > max_id) max_id = id;
        }
    }
    closedir(d);
    return max_id + 1;
}

void *reset_existing(void *arg) {
    int start_id = next_id - NUM_PRE_CTRLS;
    if (start_id < 0) start_id = 0;
    
    while (1) {
        for (int i = start_id; i < next_id; i++) {
            char path[256];
            snprintf(path, sizeof(path), "/sys/class/nvme/nvme%d/reset_controller", i);
            int fd = open(path, O_WRONLY);
            if (fd >= 0) {
                if (write(fd, "1", 1) < 0) {
                    // Ignore errors
                }
                close(fd);
            }
        }
    }
    return NULL;
}

void *create_ctrls(void *arg) {
    const char *cmd = "transport=loop,nqn=test";
    while (1) {
        int fd = open("/dev/nvme-fabrics", O_RDWR);
        if (fd >= 0) {
            if (write(fd, cmd, strlen(cmd)) < 0) {
                // Ignore errors
            }
            close(fd);
        }
        usleep(50000); // 50ms delay between creations
    }
    return NULL;
}

void *hit_window(void *arg) {
    int current_id = next_id;
    while (1) {
        char path[256];
        snprintf(path, sizeof(path), "/sys/class/nvme/nvme%d/reset_controller", current_id);
        int fd = open(path, O_WRONLY);
        if (fd >= 0) {
            int res = write(fd, "1", 1);
            if (res > 0) {
                printf("[+] Hit the window for nvme%d!\n", current_id);
            }
            close(fd);
            current_id++;
        } else if (errno != ENOENT) {
            usleep(1000);
        }
    }
    return NULL;
}

int main() {
    SETUP_UNBUFFERED_IO();
    setup_nvmet();
    set_wq_max_active();

    printf("[+] Pre-creating %d controllers...\n", NUM_PRE_CTRLS);
    const char *cmd = "transport=loop,nqn=test";
    for (int i = 0; i < NUM_PRE_CTRLS; i++) {
        int fd = open("/dev/nvme-fabrics", O_RDWR);
        if (fd >= 0) {
            if (write(fd, cmd, strlen(cmd)) < 0) {
                printf("[-] Failed to create pre-controller: %s\n", strerror(errno));
            }
            close(fd);
        } else {
            printf("[-] Failed to open /dev/nvme-fabrics: %s\n", strerror(errno));
            exit(1);
        }
    }
    
    next_id = get_next_nvme_id();
    printf("[+] Next NVMe ID to expect: %d\n", next_id);

    pthread_t t_reset[4], t_create, t_hit;
    
    for (int i = 0; i < 4; i++) {
        if (pthread_create(&t_reset[i], NULL, reset_existing, NULL) != 0) {
            printf("[-] Failed to create reset thread\n");
            exit(1);
        }
    }
    
    if (pthread_create(&t_hit, NULL, hit_window, NULL) != 0) {
        printf("[-] Failed to create hit_window thread\n");
        exit(1);
    }
    
    if (pthread_create(&t_create, NULL, create_ctrls, NULL) != 0) {
        printf("[-] Failed to create create_ctrls thread\n");
        exit(1);
    }
    
    printf("[+] Threads created successfully. Running for 15 seconds...\n");

    TIMER_START(start);
    while (TIMER_NOT_EXPIRED(start, 15.0)) {
        sleep(1);
    }

    printf("[+] Done. Waiting for delayed reset_works to execute...\n");
    sleep(3);
    
    return 0;
}
