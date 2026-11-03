// https://syzkaller.appspot.com/bug?id=10fa6f6cc05ffc45265e436ce9352bc44cf0dfe5
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#ifndef __NR_fsopen
#define __NR_fsopen 430
#endif
#ifndef __NR_fsconfig
#define __NR_fsconfig 431
#endif
#ifndef __NR_fsmount
#define __NR_fsmount 432
#endif
#ifndef FSCONFIG_CMD_CREATE
#define FSCONFIG_CMD_CREATE 6
#endif

#define BATCH_A 32
#define BATCH_B 32

struct thread_data {
	int id;
	int cpu_a;
	int cpu_b;
	int mode;
};

static atomic_bool running = true;

static void pin_cpu(int cpu)
{
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(cpu, &cpuset);
	int res = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
	if (res != 0) {
		printf("[-] pthread_setaffinity_np(cpu=%d) failed: %s\n", cpu, strerror(res));
	}
}

static void enable_trace(const char *path)
{
	int fd = open(path, O_WRONLY);
	if (fd < 0) {
		printf("[-] Failed to open %s: %s\n", path, strerror(errno));
		exit(1);
	}
	const char *val = "1\n";
	ssize_t len = strlen(val);
	ssize_t res = write(fd, val, len);
	if (res < 0) {
		printf("[-] Failed to write to %s: %s\n", path, strerror(errno));
		close(fd);
		exit(1);
	}
	int cres = close(fd);
	if (cres < 0) {
		printf("[-] Failed to close %s: %s\n", path, strerror(errno));
		exit(1);
	}
	printf("[+] Successfully enabled %s\n", path);
}

static void setup_tracing(void)
{
	struct stat st;
	const char *trace_base = NULL;

	if (stat("/sys/kernel/tracing/events/hugetlbfs/hugetlbfs_free_inode/enable", &st) == 0) {
		trace_base = "/sys/kernel/tracing";
		printf("[+] Found tracefs at /sys/kernel/tracing\n");
	} else if (stat("/sys/kernel/debug/tracing/events/hugetlbfs/hugetlbfs_free_inode/enable", &st) == 0) {
		trace_base = "/sys/kernel/debug/tracing";
		printf("[+] Found tracefs at /sys/kernel/debug/tracing\n");
	} else {
		printf("[*] Mounting tracefs at /tmp/tracefs\n");
		int res = mkdir("/tmp/tracefs", 0755);
		if (res < 0 && errno != EEXIST) {
			printf("[-] Failed to mkdir /tmp/tracefs: %s\n", strerror(errno));
			exit(1);
		}
		res = mount("tracefs", "/tmp/tracefs", "tracefs", 0, NULL);
		if (res < 0) {
			printf("[-] Failed to mount tracefs: %s\n", strerror(errno));
			exit(1);
		}
		trace_base = "/tmp/tracefs";
		printf("[+] Successfully mounted tracefs at /tmp/tracefs\n");
	}

	char path[256];
	snprintf(path, sizeof(path), "%s/events/hugetlbfs/hugetlbfs_free_inode/enable", trace_base);
	enable_trace(path);

	snprintf(path, sizeof(path), "%s/events/hugetlbfs/enable", trace_base);
	int efd = open(path, O_WRONLY);
	if (efd >= 0) {
		write(efd, "1\n", 2);
		close(efd);
		printf("[+] Successfully enabled %s\n", path);
	}

	snprintf(path, sizeof(path), "%s/tracing_on", trace_base);
	enable_trace(path);
}

static void *thread_path_a(void *arg)
{
	struct thread_data *td = (struct thread_data *)arg;
	pin_cpu(td->cpu_a);

	int fsfds[BATCH_A];
	int mfds[BATCH_A];

	while (atomic_load(&running)) {
		int count = 0;
		for (int i = 0; i < BATCH_A; i++) {
			int fsfd = syscall(__NR_fsopen, "hugetlbfs", 0);
			if (fsfd < 0) {
				printf("[-] Failed to fsopen hugetlbfs: %s\n", strerror(errno));
				exit(1);
			}
			int res = syscall(__NR_fsconfig, fsfd, FSCONFIG_CMD_CREATE, NULL, NULL, 0);
			if (res < 0) {
				printf("[-] Failed to fsconfig(FSCONFIG_CMD_CREATE): %s\n", strerror(errno));
				close(fsfd);
				exit(1);
			}
			int mfd = syscall(__NR_fsmount, fsfd, 0, 0);
			if (mfd < 0) {
				printf("[-] Failed to fsmount: %s\n", strerror(errno));
				close(fsfd);
				exit(1);
			}
			fsfds[count] = fsfd;
			mfds[count] = mfd;
			count++;
		}

		if (td->mode == 0) {
			for (int i = 0; i < count; i++) {
				int res = close(fsfds[i]);
				if (res < 0) {
					printf("[-] Failed to close fsfd: %s\n", strerror(errno));
					exit(1);
				}
			}
			for (int i = 0; i < count; i++) {
				int res = close(mfds[i]);
				if (res < 0) {
					printf("[-] Failed to close mfd: %s\n", strerror(errno));
					exit(1);
				}
			}
		} else {
			for (int i = 0; i < count; i++) {
				int res = close(mfds[i]);
				if (res < 0) {
					printf("[-] Failed to close mfd: %s\n", strerror(errno));
					exit(1);
				}
			}
			for (int i = 0; i < count; i++) {
				int res = close(fsfds[i]);
				if (res < 0) {
					printf("[-] Failed to close fsfd: %s\n", strerror(errno));
					exit(1);
				}
			}
		}

		sched_yield();
	}

	return NULL;
}

static void *thread_path_b(void *arg)
{
	struct thread_data *td = (struct thread_data *)arg;
	pin_cpu(td->cpu_a);

	int fds[BATCH_B];
	int iter = 0;

	while (atomic_load(&running)) {
		char dir[64];
		snprintf(dir, sizeof(dir), "/tmp/hugetlb_%d_%d", td->id, iter % 8);
		iter++;

		int res = mount("none", dir, "hugetlbfs", 0, NULL);
		if (res < 0) {
			printf("[-] Failed to mount hugetlbfs on %s: %s\n", dir, strerror(errno));
			exit(1);
		}

		for (int i = 0; i < BATCH_B; i++) {
			char filepath[128];
			snprintf(filepath, sizeof(filepath), "%s/f_%d", dir, i);
			fds[i] = open(filepath, O_CREAT | O_RDWR, 0666);
			if (fds[i] < 0) {
				printf("[-] Failed to open %s: %s\n", filepath, strerror(errno));
				exit(1);
			}
			int ures = unlink(filepath);
			if (ures < 0) {
				printf("[-] Failed to unlink %s: %s\n", filepath, strerror(errno));
				exit(1);
			}
		}

		res = umount2(dir, MNT_DETACH);
		if (res < 0) {
			printf("[-] Failed to umount2 %s: %s\n", dir, strerror(errno));
			exit(1);
		}

		for (int i = 0; i < BATCH_B / 2; i++) {
			int cres = close(fds[i]);
			if (cres < 0) {
				printf("[-] Failed to close fds[%d]: %s\n", i, strerror(errno));
				exit(1);
			}
		}

		pin_cpu(td->cpu_b);

		for (int i = BATCH_B / 2; i < BATCH_B; i++) {
			int cres = close(fds[i]);
			if (cres < 0) {
				printf("[-] Failed to close fds[%d]: %s\n", i, strerror(errno));
				exit(1);
			}
		}

		pin_cpu(td->cpu_a);
		sched_yield();
	}

	return NULL;
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	printf("[*] Starting hugetlbfs tracepoint UAF reproducer...\n");

	struct rlimit rlim;
	rlim.rlim_cur = 4096;
	rlim.rlim_max = 4096;
	if (setrlimit(RLIMIT_NOFILE, &rlim) < 0) {
		printf("[-] setrlimit(RLIMIT_NOFILE) failed: %s\n", strerror(errno));
	} else {
		printf("[+] setrlimit(RLIMIT_NOFILE, 4096) successful.\n");
	}

	setup_tracing();
	printf("[+] Tracing configured successfully.\n");

	int test_fsfd = syscall(__NR_fsopen, "hugetlbfs", 0);
	if (test_fsfd < 0) {
		printf("[-] Failed test fsopen hugetlbfs: %s\n", strerror(errno));
		exit(1);
	}
	int res = syscall(__NR_fsconfig, test_fsfd, FSCONFIG_CMD_CREATE, NULL, NULL, 0);
	if (res < 0) {
		printf("[-] Failed test fsconfig(FSCONFIG_CMD_CREATE): %s\n", strerror(errno));
		close(test_fsfd);
		exit(1);
	}
	int test_mfd = syscall(__NR_fsmount, test_fsfd, 0, 0);
	if (test_mfd < 0) {
		printf("[-] Failed test fsmount: %s\n", strerror(errno));
		close(test_fsfd);
		exit(1);
	}
	res = close(test_fsfd);
	if (res < 0) {
		printf("[-] Failed test close(test_fsfd): %s\n", strerror(errno));
		exit(1);
	}
	res = close(test_mfd);
	if (res < 0) {
		printf("[-] Failed test close(test_mfd): %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] Path A test iteration successful.\n");

	char probe_dir[] = "/tmp/hugetlb_probe_dir";
	res = mkdir(probe_dir, 0755);
	if (res < 0 && errno != EEXIST) {
		printf("[-] Failed to mkdir %s: %s\n", probe_dir, strerror(errno));
		exit(1);
	}
	res = mount("none", probe_dir, "hugetlbfs", 0, NULL);
	if (res < 0) {
		printf("[-] Failed to mount %s: %s\n", probe_dir, strerror(errno));
		exit(1);
	}
	char probe_file[128];
	snprintf(probe_file, sizeof(probe_file), "%s/test_file", probe_dir);
	int probe_fd = open(probe_file, O_CREAT | O_RDWR, 0666);
	if (probe_fd < 0) {
		printf("[-] Failed to open %s: %s\n", probe_file, strerror(errno));
		exit(1);
	}
	res = unlink(probe_file);
	if (res < 0) {
		printf("[-] Failed to unlink %s: %s\n", probe_file, strerror(errno));
		exit(1);
	}
	res = umount2(probe_dir, MNT_DETACH);
	if (res < 0) {
		printf("[-] Failed to umount2 %s: %s\n", probe_dir, strerror(errno));
		exit(1);
	}
	res = close(probe_fd);
	if (res < 0) {
		printf("[-] Failed to close probe_fd: %s\n", strerror(errno));
		exit(1);
	}
	rmdir(probe_dir);
	printf("[+] Path B test iteration successful.\n");

	for (int t = 2; t <= 3; t++) {
		for (int i = 0; i < 8; i++) {
			char dir[64];
			snprintf(dir, sizeof(dir), "/tmp/hugetlb_%d_%d", t, i);
			res = mkdir(dir, 0755);
			if (res < 0 && errno != EEXIST) {
				printf("[-] Failed to mkdir %s: %s\n", dir, strerror(errno));
				exit(1);
			}
		}
	}
	printf("[+] Scratch mount directories created successfully.\n");

	int num_cpus = (int)sysconf(_SC_NPROCESSORS_ONLN);
	if (num_cpus < 1)
		num_cpus = 1;
	printf("[+] Detected %d online CPU(s).\n", num_cpus);

	pthread_t threads[4];
	struct thread_data td[4];

	td[0].id = 0;
	td[0].cpu_a = 0 % num_cpus;
	td[0].cpu_b = 1 % num_cpus;
	td[0].mode = 0;

	td[1].id = 1;
	td[1].cpu_a = 1 % num_cpus;
	td[1].cpu_b = 0 % num_cpus;
	td[1].mode = 1;

	td[2].id = 2;
	td[2].cpu_a = 2 % num_cpus;
	td[2].cpu_b = (2 + 1) % num_cpus;
	td[2].mode = 0;

	td[3].id = 3;
	td[3].cpu_a = 3 % num_cpus;
	td[3].cpu_b = (3 + 1) % num_cpus;
	td[3].mode = 0;

	res = pthread_create(&threads[0], NULL, thread_path_a, &td[0]);
	if (res != 0) {
		printf("[-] Failed to create thread 0: %s\n", strerror(res));
		exit(1);
	}
	res = pthread_create(&threads[1], NULL, thread_path_a, &td[1]);
	if (res != 0) {
		printf("[-] Failed to create thread 1: %s\n", strerror(res));
		exit(1);
	}
	res = pthread_create(&threads[2], NULL, thread_path_b, &td[2]);
	if (res != 0) {
		printf("[-] Failed to create thread 2: %s\n", strerror(res));
		exit(1);
	}
	res = pthread_create(&threads[3], NULL, thread_path_b, &td[3]);
	if (res != 0) {
		printf("[-] Failed to create thread 3: %s\n", strerror(res));
		exit(1);
	}
	printf("[+] All 4 worker threads started successfully.\n");

	printf("[+] Running race loop for 10 seconds...\n");
	sleep(10);

	printf("[+] Stopping worker threads...\n");
	atomic_store(&running, false);

	for (int i = 0; i < 4; i++) {
		res = pthread_join(threads[i], NULL);
		if (res != 0) {
			printf("[-] Failed to join thread %d: %s\n", i, strerror(res));
		}
	}
	printf("[+] All worker threads stopped.\n");

	printf("[+] Waiting 2 seconds for RCU and workqueue items to settle...\n");
	sleep(2);

	for (int t = 2; t <= 3; t++) {
		for (int i = 0; i < 8; i++) {
			char dir[64];
			snprintf(dir, sizeof(dir), "/tmp/hugetlb_%d_%d", t, i);
			rmdir(dir);
		}
	}

	printf("[+] Reproducer finished execution successfully.\n");
	return 0;
}
