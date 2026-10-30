// https://syzkaller.appspot.com/bug?id=e782399bd8b4cb43e2a975d9cb153908012284ce
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sched.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#define HUGETLB_ADDR   ((void *)0x200000000000ULL)
#define HUGETLB_SIZE   (12ULL * 1024 * 1024) /* 12 MB: 6 x 2 MB pages */
#define HPAGE_SIZE     (2ULL * 1024 * 1024)
#define NUM_FAULTERS   4
#define NUM_ZAPPERS    4

static volatile bool keep_running = true;

static void write_sysctl(const char *path, const char *val)
{
	int fd = open(path, O_WRONLY);
	if (fd < 0) {
		printf("[-] Failed to open %s: %s\n", path, strerror(errno));
		exit(1);
	}
	printf("[+] open %s successful.\n", path);

	size_t len = strlen(val);
	ssize_t n = write(fd, val, len);
	if (n < 0 || (size_t)n != len) {
		printf("[-] Failed to write to %s: %s\n", path, strerror(errno));
		close(fd);
		exit(1);
	}
	printf("[+] write to %s successful.\n", path);

	int res = close(fd);
	if (res < 0) {
		printf("[-] Failed to close %s: %s\n", path, strerror(errno));
		exit(1);
	}
	printf("[+] close %s successful.\n", path);
}

static void *faulter_thread(void *arg)
{
	int thread_id = (int)(intptr_t)arg;
	int num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
	if (num_cpus < 1)
		num_cpus = 1;
	int cpus = (num_cpus >= 2) ? 2 : 1;
	int target_cpu = thread_id % cpus;

	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(target_cpu, &cpuset);
	int res = sched_setaffinity(0, sizeof(cpuset), &cpuset);
	if (res < 0) {
		printf("[-] Failed to sched_setaffinity in faulter %d: %s\n", thread_id, strerror(errno));
		exit(1);
	}

	char *addr = (char *)HUGETLB_ADDR;
	size_t start_off = (thread_id * HPAGE_SIZE) % HUGETLB_SIZE;

	while (__atomic_load_n(&keep_running, __ATOMIC_RELAXED)) {
		for (size_t i = 0; i < HUGETLB_SIZE; i += HPAGE_SIZE) {
			size_t off = (start_off + i) % HUGETLB_SIZE;
			*(volatile char *)(addr + off) = (char)thread_id;
		}
	}
	return NULL;
}

static void *zapper_thread(void *arg)
{
	int thread_id = (int)(intptr_t)arg;
	int num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
	if (num_cpus < 1)
		num_cpus = 1;
	int cpus = (num_cpus >= 2) ? 2 : 1;
	int target_cpu = thread_id % cpus;

	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(target_cpu, &cpuset);
	int res = sched_setaffinity(0, sizeof(cpuset), &cpuset);
	if (res < 0) {
		printf("[-] Failed to sched_setaffinity in zapper %d: %s\n", thread_id, strerror(errno));
		exit(1);
	}

	while (__atomic_load_n(&keep_running, __ATOMIC_RELAXED)) {
		int ret = madvise(HUGETLB_ADDR, HUGETLB_SIZE, MADV_DONTNEED);
		if (ret < 0) {
			printf("[-] Failed to madvise: %s\n", strerror(errno));
			exit(1);
		}
	}
	return NULL;
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	printf("[+] Starting hugetlb bad page state reproducer...\n");

	write_sysctl("/proc/sys/vm/nr_overcommit_hugepages", "1000\n");
	write_sysctl("/proc/sys/vm/nr_hugepages", "0\n");

	void *addr = mmap(HUGETLB_ADDR, HUGETLB_SIZE,
			  PROT_READ | PROT_WRITE,
			  MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB | MAP_NORESERVE | MAP_FIXED,
			  -1, 0);
	if (addr == MAP_FAILED) {
		printf("[-] Failed to mmap hugetlb: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] mmap hugetlb at %p successful.\n", addr);

	pthread_t th_faulters[NUM_FAULTERS];
	pthread_t th_zappers[NUM_ZAPPERS];

	for (int i = 0; i < NUM_FAULTERS; i++) {
		int res = pthread_create(&th_faulters[i], NULL, faulter_thread, (void *)(intptr_t)i);
		if (res != 0) {
			printf("[-] Failed to pthread_create faulter %d: %s\n", i, strerror(res));
			exit(1);
		}
		printf("[+] pthread_create faulter %d successful.\n", i);
	}

	for (int i = 0; i < NUM_ZAPPERS; i++) {
		int res = pthread_create(&th_zappers[i], NULL, zapper_thread, (void *)(intptr_t)i);
		if (res != 0) {
			printf("[-] Failed to pthread_create zapper %d: %s\n", i, strerror(res));
			exit(1);
		}
		printf("[+] pthread_create zapper %d successful.\n", i);
	}

	struct timespec start;
	int res = clock_gettime(CLOCK_MONOTONIC, &start);
	if (res < 0) {
		printf("[-] Failed to clock_gettime: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] Racing faulter and zapper threads for 25 seconds...\n");

	while (1) {
		struct timespec now;
		if (clock_gettime(CLOCK_MONOTONIC, &now) < 0) {
			printf("[-] Failed to clock_gettime: %s\n", strerror(errno));
			exit(1);
		}
		double elapsed = (now.tv_sec - start.tv_sec) +
				 (now.tv_nsec - start.tv_nsec) / 1e9;
		if (elapsed >= 25.0)
			break;
		usleep(100000);
	}

	printf("[+] Time elapsed, stopping threads...\n");
	__atomic_store_n(&keep_running, false, __ATOMIC_RELAXED);

	for (int i = 0; i < NUM_FAULTERS; i++) {
		int s = pthread_join(th_faulters[i], NULL);
		if (s != 0) {
			printf("[-] Failed to pthread_join faulter %d: %s\n", i, strerror(s));
			exit(1);
		}
		printf("[+] pthread_join faulter %d successful.\n", i);
	}

	for (int i = 0; i < NUM_ZAPPERS; i++) {
		int s = pthread_join(th_zappers[i], NULL);
		if (s != 0) {
			printf("[-] Failed to pthread_join zapper %d: %s\n", i, strerror(s));
			exit(1);
		}
		printf("[+] pthread_join zapper %d successful.\n", i);
	}

	res = munmap(addr, HUGETLB_SIZE);
	if (res < 0) {
		printf("[-] Failed to munmap hugetlb: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] munmap hugetlb successful.\n");

	printf("[+] Test completed cleanly.\n");
	return 0;
}
