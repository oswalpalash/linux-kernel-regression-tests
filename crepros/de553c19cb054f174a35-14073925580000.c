// https://syzkaller.appspot.com/bug?id=9c30bc03df6d6eb87a653b966f4a0b31565d39b4
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdint.h>

#define UDC_NAME_LENGTH_MAX 128

struct usb_raw_init {
	uint8_t	driver_name[UDC_NAME_LENGTH_MAX];
	uint8_t	device_name[UDC_NAME_LENGTH_MAX];
	uint8_t	speed;
};

#define USB_RAW_IO_BASE		'U'
#define USB_RAW_IOCTL_INIT	_IOW(USB_RAW_IO_BASE, 0, struct usb_raw_init)
#define USB_RAW_IOCTL_RUN	_IO(USB_RAW_IO_BASE, 1)

#define USB_SPEED_HIGH 3

void enable_tracepoint(void)
{
	int fd = open("/sys/kernel/tracing/events/gadget/usb_ep_alloc_request/enable", O_WRONLY);
	if (fd < 0)
		fd = open("/sys/kernel/debug/tracing/events/gadget/usb_ep_alloc_request/enable", O_WRONLY);
	
	if (fd >= 0) {
		if (write(fd, "1\n", 2) < 0) {
			printf("[-] Failed to write to tracepoint enable: %s\n", strerror(errno));
			exit(1);
		}
		close(fd);
		printf("[+] Tracepoint enabled.\n");
	} else {
		printf("[-] Failed to open tracepoint enable file: %s\n", strerror(errno));
		exit(1);
	}
}

void set_fail_nth(int n)
{
	int fd = open("/proc/self/fail-nth", O_WRONLY);
	if (fd >= 0) {
		char buf[32];
		int len = snprintf(buf, sizeof(buf), "%d\n", n);
		if (write(fd, buf, len) < 0) {
			// Ignore errors here, as we might just be resetting it
		}
		close(fd);
	} else {
		printf("[-] Failed to open /proc/self/fail-nth: %s\n", strerror(errno));
		exit(1);
	}
}

int main(void)
{
	int fd;
	struct usb_raw_init init;
	int i;

	enable_tracepoint();

	for (i = 1; i <= 100; i++) {
		fd = open("/dev/raw-gadget", O_RDWR);
		if (fd < 0) {
			printf("[-] Failed to open /dev/raw-gadget: %s\n", strerror(errno));
			exit(1);
		}

		memset(&init, 0, sizeof(init));
		strcpy((char *)init.driver_name, "dummy_udc");
		strcpy((char *)init.device_name, "dummy_udc.0");
		init.speed = USB_SPEED_HIGH;

		if (ioctl(fd, USB_RAW_IOCTL_INIT, &init) == 0) {
			set_fail_nth(i);
			ioctl(fd, USB_RAW_IOCTL_RUN, 0);
			set_fail_nth(0);
		} else {
			printf("[-] USB_RAW_IOCTL_INIT failed: %s\n", strerror(errno));
		}
		close(fd);
	}

	printf("[+] Finished loop.\n");
	return 0;
}
