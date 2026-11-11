// https://syzkaller.appspot.com/bug?id=d9d0a1ed37a0a74593582f1e516646c59e6dddba
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <pthread.h>
#include <stdatomic.h>

static atomic_int stop_flag = 0;

static void *thread_create_unbind(void *arg)
{
	(void)arg;
	while (!atomic_load(&stop_flag)) {
		int fd_new = open("/sys/bus/netdevsim/new_device", O_WRONLY);
		if (fd_new >= 0) {
			write(fd_new, "0 1\n", 4);
			close(fd_new);
		}
		int fd_unbind = open("/sys/bus/netdevsim/drivers/netdevsim/unbind", O_WRONLY);
		if (fd_unbind >= 0) {
			write(fd_unbind, "netdevsim0\n", 11);
			close(fd_unbind);
		}
		int fd_del = open("/sys/bus/netdevsim/del_device", O_WRONLY);
		if (fd_del >= 0) {
			write(fd_del, "0\n", 2);
			close(fd_del);
		}
	}
	return NULL;
}

static void *thread_write_port(void *arg)
{
	(void)arg;
	while (!atomic_load(&stop_flag)) {
		int fd_port = open("/sys/bus/netdevsim/devices/netdevsim0/new_port", O_WRONLY);
		if (fd_port >= 0) {
			write(fd_port, "1\n", 2);
			close(fd_port);
		}
	}
	return NULL;
}

int main(void)
{
	int res;
	int fd_new;
	int fd_port;
	int fd_unbind;
	struct stat st;

	setvbuf(stdout, NULL, _IONBF, 0);
	printf("[*] Starting netdevsim port add reproducer...\n");

	/* Check /sys existence */
	res = stat("/sys", &st);
	if (res < 0) {
		printf("[-] Failed to stat /sys: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] stat /sys successful.\n");

	/* Mount sysfs if not mounted */
	if (access("/sys/bus", F_OK) < 0) {
		res = mount("none", "/sys", "sysfs", 0, NULL);
		if (res < 0) {
			printf("[-] Failed to mount /sys: %s\n", strerror(errno));
			exit(1);
		}
		printf("[+] mount sysfs successful.\n");
	} else {
		printf("[+] sysfs is already mounted.\n");
	}

	/* Clean up existing netdevsim0 if present */
	if (access("/sys/bus/netdevsim/devices/netdevsim0", F_OK) == 0) {
		printf("[*] netdevsim0 exists from previous run, cleaning up...\n");
		int fd_del = open("/sys/bus/netdevsim/del_device", O_WRONLY);
		if (fd_del < 0) {
			printf("[-] Failed to open /sys/bus/netdevsim/del_device: %s\n", strerror(errno));
			exit(1);
		}
		printf("[+] open /sys/bus/netdevsim/del_device successful.\n");

		res = write(fd_del, "0\n", 2);
		if (res < 0) {
			printf("[-] Failed to write to /sys/bus/netdevsim/del_device: %s\n", strerror(errno));
			exit(1);
		}
		printf("[+] write to /sys/bus/netdevsim/del_device successful.\n");

		res = close(fd_del);
		if (res < 0) {
			printf("[-] Failed to close /sys/bus/netdevsim/del_device: %s\n", strerror(errno));
			exit(1);
		}
		printf("[+] close /sys/bus/netdevsim/del_device successful.\n");
	} else {
		printf("[+] netdevsim0 does not exist yet, no initial cleanup needed.\n");
	}

	/* 1. Create netdevsim0 with 1 port */
	fd_new = open("/sys/bus/netdevsim/new_device", O_WRONLY);
	if (fd_new < 0) {
		printf("[-] Failed to open /sys/bus/netdevsim/new_device: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] open /sys/bus/netdevsim/new_device successful.\n");

	res = write(fd_new, "0 1\n", 4);
	if (res < 0) {
		printf("[-] Failed to write to /sys/bus/netdevsim/new_device: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] write to /sys/bus/netdevsim/new_device successful.\n");

	res = close(fd_new);
	if (res < 0) {
		printf("[-] Failed to close /sys/bus/netdevsim/new_device: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] close /sys/bus/netdevsim/new_device successful.\n");

	/* 2. Open /sys/bus/netdevsim/devices/netdevsim0/new_port */
	fd_port = open("/sys/bus/netdevsim/devices/netdevsim0/new_port", O_WRONLY);
	if (fd_port < 0) {
		printf("[-] Failed to open /sys/bus/netdevsim/devices/netdevsim0/new_port: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] open /sys/bus/netdevsim/devices/netdevsim0/new_port successful.\n");

	/* 3. Unbind the driver from netdevsim0 */
	fd_unbind = open("/sys/bus/netdevsim/drivers/netdevsim/unbind", O_WRONLY);
	if (fd_unbind < 0) {
		printf("[-] Failed to open /sys/bus/netdevsim/drivers/netdevsim/unbind: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] open /sys/bus/netdevsim/drivers/netdevsim/unbind successful.\n");

	res = write(fd_unbind, "netdevsim0\n", 11);
	if (res < 0) {
		printf("[-] Failed to write to /sys/bus/netdevsim/drivers/netdevsim/unbind: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] write to /sys/bus/netdevsim/drivers/netdevsim/unbind successful (netdevsim0 unbound).\n");

	res = close(fd_unbind);
	if (res < 0) {
		printf("[-] Failed to close /sys/bus/netdevsim/drivers/netdevsim/unbind: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] close /sys/bus/netdevsim/drivers/netdevsim/unbind successful.\n");

	/* 4. Trigger the crash by writing port index to new_port on the unbound device */
	printf("[*] Writing to new_port on unbound netdevsim0 to trigger kernel crash...\n");
	res = write(fd_port, "1\n", 2);
	if (res < 0) {
		printf("[-] write to new_port failed: %s\n", strerror(errno));
	} else {
		printf("[+] write to new_port returned: %d bytes\n", res);
	}

	res = close(fd_port);
	if (res < 0) {
		printf("[-] Failed to close fd_port: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] close fd_port successful.\n");

	/* 5. Fallback concurrent race loop if sequential write did not trigger crash */
	printf("[*] Sequential attempt did not crash; starting concurrent race loop...\n");
	pthread_t th1, th2;
	res = pthread_create(&th1, NULL, thread_create_unbind, NULL);
	if (res != 0) {
		printf("[-] Failed to create thread 1: %s\n", strerror(res));
		exit(1);
	}
	res = pthread_create(&th2, NULL, thread_write_port, NULL);
	if (res != 0) {
		printf("[-] Failed to create thread 2: %s\n", strerror(res));
		exit(1);
	}
	printf("[+] Concurrent threads running, waiting 3 seconds...\n");
	sleep(3);
	atomic_store(&stop_flag, 1);
	pthread_join(th1, NULL);
	pthread_join(th2, NULL);
	printf("[+] Concurrent race loop completed without crash.\n");

	return 0;
}
