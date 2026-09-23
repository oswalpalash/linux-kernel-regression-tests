// https://syzkaller.appspot.com/bug?id=9c60838fb4d13addeb5748e0b2afacf1f482e1be
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
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <linux/usb/ch9.h>
#include <signal.h>
#include <errno.h>
#include <dirent.h>
#include <time.h>

#define UDC_NAME_LENGTH_MAX 128

struct usb_raw_init {
	__u8	driver_name[UDC_NAME_LENGTH_MAX];
	__u8	device_name[UDC_NAME_LENGTH_MAX];
	__u8	speed;
};

enum usb_raw_event_type {
	USB_RAW_EVENT_INVALID = 0,
	USB_RAW_EVENT_CONNECT = 1,
	USB_RAW_EVENT_CONTROL = 2,
	USB_RAW_EVENT_SUSPEND = 3,
	USB_RAW_EVENT_RESUME = 4,
	USB_RAW_EVENT_RESET = 5,
	USB_RAW_EVENT_DISCONNECT = 6,
};

struct usb_raw_event {
	__u32		type;
	__u32		length;
	__u8		data[];
};

struct usb_raw_ep_io {
	__u16		ep;
	__u16		flags;
	__u32		length;
	__u8		data[];
};

#define USB_RAW_IOCTL_INIT		_IOW('U', 0, struct usb_raw_init)
#define USB_RAW_IOCTL_RUN		_IO('U', 1)
#define USB_RAW_IOCTL_EVENT_FETCH	_IOR('U', 2, struct usb_raw_event)
#define USB_RAW_IOCTL_EP0_WRITE		_IOW('U', 3, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_EP0_READ		_IOWR('U', 4, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_EP0_STALL		_IO('U', 12)

#define USB_DIR_IN 0x80
#define USB_REQ_GET_DESCRIPTOR 0x06
#define USB_REQ_SET_CONFIGURATION 0x09
#define USB_DT_DEVICE 1
#define USB_DT_CONFIG 2
#define USB_DT_INTERFACE 4
#define USB_DT_ENDPOINT 5

/* Standard 7-byte endpoint descriptor to avoid audio-extension parsing issues */
struct usb_endpoint_descriptor_7 {
	__u8  bLength;
	__u8  bDescriptorType;
	__u8  bEndpointAddress;
	__u8  bmAttributes;
	__u16 wMaxPacketSize;
	__u8  bInterval;
} __attribute__ ((packed));

struct usb_device_descriptor dev_desc = {
	.bLength = sizeof(struct usb_device_descriptor),
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = 64,
	.idVendor = 0x1b71,
	.idProduct = 0x3002,
	.bcdDevice = 0x0100,
	.iManufacturer = 0,
	.iProduct = 0,
	.iSerialNumber = 0,
	.bNumConfigurations = 1,
};

struct config_pack {
	struct usb_config_descriptor conf;
	struct usb_interface_descriptor intf0;
	struct usb_interface_descriptor intf1;
	struct usb_endpoint_descriptor_7 ep1;
	struct usb_endpoint_descriptor_7 ep2;
	struct usb_endpoint_descriptor_7 ep3;
	struct usb_endpoint_descriptor_7 ep4;
} __attribute__((packed));

struct config_pack conf_desc = {
	.conf = {
		.bLength = sizeof(struct usb_config_descriptor),
		.bDescriptorType = USB_DT_CONFIG,
		.wTotalLength = sizeof(struct config_pack),
		.bNumInterfaces = 1,
		.bConfigurationValue = 1,
		.iConfiguration = 0,
		.bmAttributes = 0x80,
		.bMaxPower = 50,
	},
	.intf0 = {
		.bLength = sizeof(struct usb_interface_descriptor),
		.bDescriptorType = USB_DT_INTERFACE,
		.bInterfaceNumber = 0,
		.bAlternateSetting = 0,
		.bNumEndpoints = 0,
		.bInterfaceClass = 255,
		.bInterfaceSubClass = 0,
		.bInterfaceProtocol = 0,
		.iInterface = 0,
	},
	.intf1 = {
		.bLength = sizeof(struct usb_interface_descriptor),
		.bDescriptorType = USB_DT_INTERFACE,
		.bInterfaceNumber = 0,
		.bAlternateSetting = 1,
		.bNumEndpoints = 4,
		.bInterfaceClass = 255,
		.bInterfaceSubClass = 0,
		.bInterfaceProtocol = 0,
		.iInterface = 0,
	},
	.ep1 = {
		.bLength = sizeof(struct usb_endpoint_descriptor_7),
		.bDescriptorType = USB_DT_ENDPOINT,
		.bEndpointAddress = 0x81,
		.bmAttributes = 1, // ISOC
		.wMaxPacketSize = 512,
		.bInterval = 1,
	},
	.ep2 = {
		.bLength = sizeof(struct usb_endpoint_descriptor_7),
		.bDescriptorType = USB_DT_ENDPOINT,
		.bEndpointAddress = 0x82,
		.bmAttributes = 2, // BULK
		.wMaxPacketSize = 512,
		.bInterval = 0,
	},
	.ep3 = {
		.bLength = sizeof(struct usb_endpoint_descriptor_7),
		.bDescriptorType = USB_DT_ENDPOINT,
		.bEndpointAddress = 0x83,
		.bmAttributes = 2, // BULK
		.wMaxPacketSize = 512,
		.bInterval = 0,
	},
	.ep4 = {
		.bLength = sizeof(struct usb_endpoint_descriptor_7),
		.bDescriptorType = USB_DT_ENDPOINT,
		.bEndpointAddress = 0x84,
		.bmAttributes = 2, // BULK
		.wMaxPacketSize = 512,
		.bInterval = 0,
	}
};

volatile int disconnect_now = 0;
volatile int device_configured = 0;

void find_udc(char *udc_name, size_t max_len) {
	DIR *dir = opendir("/sys/class/udc");
	if (!dir) {
		printf("[-] Failed to open /sys/class/udc: %s\n", strerror(errno));
		exit(1);
	}
	struct dirent *ent;
	while ((ent = readdir(dir)) != NULL) {
		if (ent->d_name[0] != '.') {
			strncpy(udc_name, ent->d_name, max_len);
			printf("[+] Found UDC: %s\n", udc_name);
			break;
		}
	}
	closedir(dir);
}

void *usb_emulation_thread(void *arg) {
	int num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
	if (num_cpus > 0) PIN_TO_CPU(0 % num_cpus);
	
	int fd = open("/dev/raw-gadget", O_RDWR);
	if (fd < 0) {
		printf("[-] Failed to open /dev/raw-gadget: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] open /dev/raw-gadget successful.\n");
	
	struct usb_raw_init init = {
		.driver_name = "dummy_udc",
		.device_name = "dummy_udc.0",
		.speed = 3, // USB_SPEED_HIGH
	};
	find_udc((char*)init.device_name, sizeof(init.device_name));

	if (ioctl(fd, USB_RAW_IOCTL_INIT, &init) < 0) {
		printf("[-] Failed to USB_RAW_IOCTL_INIT: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] USB_RAW_IOCTL_INIT successful.\n");

	if (ioctl(fd, USB_RAW_IOCTL_RUN, 0) < 0) {
		printf("[-] Failed to USB_RAW_IOCTL_RUN: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] USB_RAW_IOCTL_RUN successful.\n");

	while (!__atomic_load_n(&disconnect_now, __ATOMIC_ACQUIRE)) {
		char buf[sizeof(struct usb_raw_event) + sizeof(struct usb_ctrlrequest)] = {0};
		struct usb_raw_event *ev = (struct usb_raw_event *)buf;
		ev->length = sizeof(struct usb_ctrlrequest);
		
		int ret = ioctl(fd, USB_RAW_IOCTL_EVENT_FETCH, ev);
		if (ret < 0) {
			usleep(1000);
			continue;
		}

		if (ev->type == USB_RAW_EVENT_CONTROL) {
			struct usb_ctrlrequest *req = (struct usb_ctrlrequest *)ev->data;
			
			/* raw-gadget treats (IN && wLength > 0) as IN transfers, everything else as OUT */
			if ((req->bRequestType & USB_DIR_IN) && req->wLength) {
				if (req->bRequest == USB_REQ_GET_DESCRIPTOR) {
					int desc_type = req->wValue >> 8;
					if (desc_type == USB_DT_DEVICE) {
						struct {
							struct usb_raw_ep_io io;
							char data[sizeof(dev_desc)];
						} __attribute__((packed)) io_req;
						memset(&io_req, 0, sizeof(io_req));
						io_req.io.ep = 0;
						io_req.io.length = sizeof(dev_desc);
						if (io_req.io.length > req->wLength) io_req.io.length = req->wLength;
						memcpy(io_req.data, &dev_desc, io_req.io.length);
						if (ioctl(fd, USB_RAW_IOCTL_EP0_WRITE, &io_req) < 0) {
							// Ignore errors
						}
					} else if (desc_type == USB_DT_CONFIG) {
						struct {
							struct usb_raw_ep_io io;
							char data[sizeof(conf_desc)];
						} __attribute__((packed)) io_req;
						memset(&io_req, 0, sizeof(io_req));
						io_req.io.ep = 0;
						io_req.io.length = sizeof(conf_desc);
						if (io_req.io.length > req->wLength) io_req.io.length = req->wLength;
						memcpy(io_req.data, &conf_desc, io_req.io.length);
						if (ioctl(fd, USB_RAW_IOCTL_EP0_WRITE, &io_req) < 0) {
							// Ignore errors
						}
					} else {
						if (ioctl(fd, USB_RAW_IOCTL_EP0_STALL, 0) < 0) {
							// Ignore stall errors
						}
					}
				} else {
					if (ioctl(fd, USB_RAW_IOCTL_EP0_STALL, 0) < 0) {
						// Ignore stall errors
					}
				}
			} else {
				struct {
					struct usb_raw_ep_io io;
					char data[4096];
				} io_req;
				memset(&io_req, 0, sizeof(io_req));
				io_req.io.ep = 0;
				io_req.io.length = req->wLength;
				if (io_req.io.length > 4096) io_req.io.length = 4096;
				
				/* Acknowledge OUT transfer (or zero-length IN transfer) */
				if (ioctl(fd, USB_RAW_IOCTL_EP0_READ, &io_req) < 0) {
					// Ignore errors
				}
				
				if (req->bRequest == USB_REQ_SET_CONFIGURATION) {
					__atomic_store_n(&device_configured, 1, __ATOMIC_RELEASE);
				}
			}
		}
	}
	
	close(fd);
	return NULL;
}

int find_video_node() {
	DIR *dir = opendir("/sys/class/video4linux");
	if (!dir) return -1;
	struct dirent *ent;
	int found = -1;
	while ((ent = readdir(dir)) != NULL) {
		if (strncmp(ent->d_name, "video", 5) == 0) {
			char path[256];
			snprintf(path, sizeof(path), "/sys/class/video4linux/%s/name", ent->d_name);
			int fd = open(path, O_RDONLY);
			if (fd >= 0) {
				char name[256] = {0};
				ssize_t n = read(fd, name, sizeof(name) - 1);
				close(fd);
				if (n > 0 && strncmp(name, "usbtv", 5) == 0) {
					found = atoi(ent->d_name + 5);
					break;
				}
			}
		}
	}
	closedir(dir);
	return found;
}

char usb_dev_name[256] = {0};

int find_usb_device() {
	DIR *dir = opendir("/sys/bus/usb/drivers/usbtv");
	if (!dir) return -1;
	struct dirent *ent;
	int found = 0;
	while ((ent = readdir(dir)) != NULL) {
		if (strchr(ent->d_name, ':')) {
			strncpy(usb_dev_name, ent->d_name, sizeof(usb_dev_name)-1);
			found = 1;
			break;
		}
	}
	closedir(dir);
	return found ? 0 : -1;
}

#define NUM_THREADS 4

volatile int stop_threads = 0;
volatile int video_fd = -1;

void *querycap_thread(void *arg) {
	int id = (int)(long)arg;
	int num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
	if (num_cpus > 0) PIN_TO_CPU((id % num_cpus));
	
	struct v4l2_capability cap;
	while (!__atomic_load_n(&stop_threads, __ATOMIC_ACQUIRE)) {
		int fd = __atomic_load_n(&video_fd, __ATOMIC_ACQUIRE);
		if (fd >= 0) {
			ioctl(fd, VIDIOC_QUERYCAP, &cap);
		} else {
			// Small backoff when fd is not available
			for (volatile int i = 0; i < 100; i++) {}
		}
	}
	return NULL;
}

int main() {
	SETUP_UNBUFFERED_IO();
	srand(time(NULL) ^ getpid());
	int num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
	if (num_cpus > 0) PIN_TO_CPU(NUM_THREADS % num_cpus);

	pthread_t usb_th;
	if (pthread_create(&usb_th, NULL, usb_emulation_thread, NULL) != 0) {
		printf("[-] Failed to create usb thread\n");
		exit(1);
	}
	printf("[+] usb thread created successfully.\n");

	int wait_time = 0;
	while (!__atomic_load_n(&device_configured, __ATOMIC_ACQUIRE) && wait_time < 500) {
		usleep(10000);
		wait_time++;
	}
	if (!__atomic_load_n(&device_configured, __ATOMIC_ACQUIRE)) {
		printf("[-] Device not configured\n");
		__atomic_store_n(&disconnect_now, 1, __ATOMIC_RELEASE);
		pthread_join(usb_th, NULL);
		exit(1);
	}
	printf("[+] Device configured successfully.\n");

	int usb_found = 0;
	for (int i = 0; i < 50; i++) {
		if (find_usb_device() >= 0) {
			usb_found = 1;
			break;
		}
		usleep(10000);
	}
	if (!usb_found) {
		printf("[-] USB device not found in sysfs\n");
		__atomic_store_n(&disconnect_now, 1, __ATOMIC_RELEASE);
		pthread_join(usb_th, NULL);
		exit(1);
	}
	printf("[+] USB device found successfully: %s\n", usb_dev_name);

	int vnode = -1;
	for (int i = 0; i < 50; i++) {
		vnode = find_video_node();
		if (vnode >= 0) break;
		usleep(10000);
	}
	if (vnode < 0) {
		printf("[-] Video node not found\n");
		__atomic_store_n(&disconnect_now, 1, __ATOMIC_RELEASE);
		pthread_join(usb_th, NULL);
		exit(1);
	}
	printf("[+] Video node found successfully: /dev/video%d\n", vnode);

	int unbind_fd = open("/sys/bus/usb/drivers/usbtv/unbind", O_WRONLY);
	if (unbind_fd < 0) {
		printf("[-] Failed to open unbind: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] open unbind successful.\n");

	int bind_fd = open("/sys/bus/usb/drivers/usbtv/bind", O_WRONLY);
	if (bind_fd < 0) {
		printf("[-] Failed to open bind: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] open bind successful.\n");

	pthread_t qc_th[NUM_THREADS];
	for (int i = 0; i < NUM_THREADS; i++) {
		if (pthread_create(&qc_th[i], NULL, querycap_thread, (void*)(long)i) != 0) {
			printf("[-] Failed to create querycap thread\n");
			exit(1);
		}
	}
	printf("[+] querycap threads created successfully.\n");

	printf("[+] Starting race loop...\n");
	TIMER_START(start);
	while (TIMER_NOT_EXPIRED(start, 10.0)) {
		vnode = find_video_node();
		if (vnode >= 0) {
			char devpath[256];
			snprintf(devpath, sizeof(devpath), "/dev/video%d", vnode);
			int fd = open(devpath, O_RDWR);
			if (fd >= 0) {
				// Share fd with ioctl threads
				__atomic_store_n(&video_fd, fd, __ATOMIC_RELEASE);
				
				// Let threads hammer the ioctl
				usleep(1000 + (rand() % 2000));
				
				// Synchronously trigger disconnect
				if (pwrite(unbind_fd, usb_dev_name, strlen(usb_dev_name), 0) < 0) {
					// Ignore write errors
				}
				
				// Stop ioctl threads from using this fd
				__atomic_store_n(&video_fd, -1, __ATOMIC_RELEASE);
				close(fd);
				
				// Ensure minor is freed before rebinding
				usleep(10000);
				
			} else {
				usleep(1000);
			}
		} else {
			usleep(1000);
		}
		
		// Re-bind the driver
		if (pwrite(bind_fd, usb_dev_name, strlen(usb_dev_name), 0) < 0) {
			// Ignore write errors
		}
		usleep(1000);
	}

	__atomic_store_n(&stop_threads, 1, __ATOMIC_RELEASE);
	for (int i = 0; i < NUM_THREADS; i++) {
		pthread_join(qc_th[i], NULL);
	}

	__atomic_store_n(&disconnect_now, 1, __ATOMIC_RELEASE);
	pthread_join(usb_th, NULL);

	close(unbind_fd);
	close(bind_fd);

	printf("[+] Race loop finished successfully.\n");
	return 0;
}