// https://syzkaller.appspot.com/bug?id=da5f1d8e7668178313e2bc496b5ba2d0892e0cd4
#define _GNU_SOURCE
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#define NUM_VMAS     64
#define PAGE_SZ      4096
#define BASE_ADDR    ((uintptr_t)0x20000000)
#define UNMAP_ADDR   ((uintptr_t)(BASE_ADDR + (NUM_VMAS * 2 + 8) * PAGE_SZ))

static void *addrs[NUM_VMAS];
static volatile int start_work = 0;
static volatile int work_done = 0;
static volatile int stop = 0;

static void pin_to_cpu(int cpu)
{
	cpu_set_t set;
	CPU_ZERO(&set);
	CPU_SET(cpu, &set);
	if (sched_setaffinity(0, sizeof(set), &set) < 0) {
		printf("[-] Failed to sched_setaffinity for CPU %d: %s\n",
		       cpu, strerror(errno));
		exit(1);
	}
}

static inline double get_time_sec(void)
{
	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0) {
		printf("[-] Failed to clock_gettime: %s\n", strerror(errno));
		exit(1);
	}
	return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

static void *worker_thread(void *arg)
{
	long cpu_id = (long)arg;
	pin_to_cpu((int)cpu_id);

	int iter = 1;
	while (!stop) {
		/* 1. Spin-wait until main thread has reset all VMAs to anon_vma == NULL */
		while (__atomic_load_n(&start_work, __ATOMIC_ACQUIRE) < iter) {
			if (stop)
				return NULL;
			asm volatile("pause" ::: "memory");
		}

		if (stop)
			return NULL;

		/* Fine-grained phase sweep to shift timing across the validate_mm window */
		int delay_cycles = ((iter >> 2) % 64) * 5;
		for (int d = 0; d < delay_cycles; d++) {
			asm volatile("pause" ::: "memory");
		}

		/* 2. Fault VMAs concurrently while validate_mm() is validating the mm */
		int pattern = iter % 4;
		if (pattern == 0) {
			/* Reverse order: traverses from highest to lowest VMA, crossing
			 * validate_mm's ascending traversal */
			for (int i = NUM_VMAS - 1; i >= 0; i--) {
				*(volatile char *)addrs[i] = 1;
			}
		} else if (pattern == 1) {
			/* Forward order: traverses from lowest to highest VMA */
			for (int i = 0; i < NUM_VMAS; i++) {
				*(volatile char *)addrs[i] = 1;
			}
		} else if (pattern == 2) {
			/* Reverse order from rotating offset */
			int offset = iter % NUM_VMAS;
			for (int j = 0; j < NUM_VMAS; j++) {
				int i = (offset - j + NUM_VMAS) % NUM_VMAS;
				*(volatile char *)addrs[i] = 1;
			}
		} else {
			/* Forward order from rotating offset */
			int offset = iter % NUM_VMAS;
			for (int j = 0; j < NUM_VMAS; j++) {
				int i = (offset + j) % NUM_VMAS;
				*(volatile char *)addrs[i] = 1;
			}
		}

		/* 3. Signal completion of faulting phase for this iteration */
		__atomic_store_n(&work_done, iter, __ATOMIC_RELEASE);
		iter++;
	}

	return NULL;
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	printf("[*] Starting reproducer for validate_mm GPF / null-ptr-deref...\n");

	long num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
	if (num_cpus < 1) {
		printf("[-] Failed to query online CPUs: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] Detected %ld online CPUs.\n", num_cpus);

	pin_to_cpu(0);
	printf("[+] Main thread pinned to CPU 0.\n");

	for (int i = 0; i < NUM_VMAS; i++) {
		addrs[i] = (void *)(BASE_ADDR + i * 2 * PAGE_SZ);
		void *p = mmap(addrs[i], PAGE_SZ, PROT_READ | PROT_WRITE,
		               MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
		if (p == MAP_FAILED) {
			printf("[-] Initial mmap failed for VMA %d: %s\n", i, strerror(errno));
			exit(1);
		}
	}
	printf("[+] Successfully initialized %d isolated VMAs.\n", NUM_VMAS);

	void *uaddr = (void *)UNMAP_ADDR;
	void *up = mmap(uaddr, PAGE_SZ, PROT_READ | PROT_WRITE,
	                MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
	if (up == MAP_FAILED) {
		printf("[-] Initial mmap failed for scratch unmap buffer: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] Initialized scratch unmap buffer at %p.\n", uaddr);

	int worker_cpu = (num_cpus > 1) ? 1 : 0;
	pthread_t worker_th;
	int pt_res = pthread_create(&worker_th, NULL, worker_thread, (void *)(long)worker_cpu);
	if (pt_res != 0) {
		printf("[-] Failed to create worker thread: %s\n", strerror(pt_res));
		exit(1);
	}
	printf("[+] Spawned worker thread pinned to CPU %d.\n", worker_cpu);

	printf("[*] Starting synchronized race loop between munmap(validate_mm) and page faults...\n");
	double t0 = get_time_sec();
	int iter = 1;

	while ((get_time_sec() - t0) < 15.0) {
		/* 1. Reset all isolated VMAs to unfaulted state (anon_vma == NULL).
		 * Worker is spin-waiting on start_work and touches nothing during reset. */
		for (int i = 0; i < NUM_VMAS; i++) {
			void *p = mmap(addrs[i], PAGE_SZ, PROT_READ | PROT_WRITE,
			               MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
			if (p == MAP_FAILED) {
				printf("[-] mmap failed for VMA %d at iter %d: %s\n",
				       i, iter, strerror(errno));
				exit(1);
			}
		}

		/* Re-map scratch buffer for unmapping */
		void *p_unmap = mmap(uaddr, PAGE_SZ, PROT_READ | PROT_WRITE,
		                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
		if (p_unmap == MAP_FAILED) {
			printf("[-] mmap failed for scratch buffer at iter %d: %s\n",
			       iter, strerror(errno));
			exit(1);
		}

		/* 2. Release worker right before calling munmap */
		__atomic_store_n(&start_work, iter, __ATOMIC_RELEASE);

		/* 3. Immediately unmap scratch buffer -> downgrades mmap_lock to READ
		 * and invokes validate_mm() while worker faults VMAs concurrently */
		int unmap_res = munmap(uaddr, PAGE_SZ);
		if (unmap_res < 0) {
			printf("[-] munmap failed at iter %d: %s\n", iter, strerror(errno));
			exit(1);
		}

		/* 4. Wait for worker to finish touching VMAs before resetting next iteration */
		while (__atomic_load_n(&work_done, __ATOMIC_ACQUIRE) < iter) {
			asm volatile("pause" ::: "memory");
		}

		if ((iter % 5000) == 0) {
			printf("[+] Completed %d iterations (elapsed: %.2fs)...\n",
			       iter, get_time_sec() - t0);
		}

		iter++;
	}

	printf("[+] Race loop finished: completed %d iterations without crash.\n", iter - 1);
	stop = 1;
	__atomic_store_n(&start_work, iter, __ATOMIC_RELEASE);

	int join_res = pthread_join(worker_th, NULL);
	if (join_res != 0) {
		printf("[-] Failed to join worker thread: %s\n", strerror(join_res));
		exit(1);
	}
	printf("[+] Worker thread successfully joined.\n");

	return 0;
}
