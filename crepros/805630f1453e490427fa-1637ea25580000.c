// https://syzkaller.appspot.com/bug?id=bf0167e2eeef9627bac03cc1f1d885a0d4751924
#define _GNU_SOURCE
#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define USB_DT_DEVICE 1
#define USB_DT_CONFIG 2
#define USB_DT_INTERFACE 4
#define USB_DT_ENDPOINT 5

#define USB_CLASS_VENDOR_SPEC 0xff
#define USB_DIR_IN 0x80
#define USB_ENDPOINT_XFER_BULK 2
#define USB_CONFIG_ATT_ONE (1 << 7)

struct usb_config_descriptor {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint16_t wTotalLength;
	uint8_t  bNumInterfaces;
	uint8_t  bConfigurationValue;
	uint8_t  iConfiguration;
	uint8_t  bmAttributes;
	uint8_t  bMaxPower;
} __attribute__((packed));

struct usb_interface_descriptor {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint8_t  bInterfaceNumber;
	uint8_t  bAlternateSetting;
	uint8_t  bNumEndpoints;
	uint8_t  bInterfaceClass;
	uint8_t  bInterfaceSubClass;
	uint8_t  bInterfaceProtocol;
	uint8_t  iInterface;
} __attribute__((packed));

struct usb_endpoint_descriptor {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint8_t  bEndpointAddress;
	uint8_t  bmAttributes;
	uint16_t wMaxPacketSize;
	uint8_t  bInterval;
} __attribute__((packed));

struct usb_device_descriptor {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint16_t bcdUSB;
	uint8_t  bDeviceClass;
	uint8_t  bDeviceSubClass;
	uint8_t  bDeviceProtocol;
	uint8_t  bMaxPacketSize0;
	uint16_t idVendor;
	uint16_t idProduct;
	uint16_t bcdDevice;
	uint8_t  iManufacturer;
	uint8_t  iProduct;
	uint8_t  iSerialNumber;
	uint8_t  bNumConfigurations;
} __attribute__((packed));

int main(void)
{
	printf("[*] Starting reproducer...\n");

	if (mkdir("/tmp/gadgetfs", 0777) < 0 && errno != EEXIST) {
		printf("[-] Failed to mkdir /tmp/gadgetfs: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] mkdir /tmp/gadgetfs successful.\n");

	if (mount("gadgetfs", "/tmp/gadgetfs", "gadgetfs", 0, NULL) != 0) {
		printf("[-] Failed to mount gadgetfs: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] mount gadgetfs successful.\n");

	DIR *dir = opendir("/tmp/gadgetfs");
	if (!dir) {
		printf("[-] Failed to opendir /tmp/gadgetfs: %s\n", strerror(errno));
		exit(1);
	}

	char ep0_path[512] = {0};
	struct dirent *ent;
	while ((ent = readdir(dir)) != NULL) {
		if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
			snprintf(ep0_path, sizeof(ep0_path), "/tmp/gadgetfs/%s", ent->d_name);
			break;
		}
	}
	closedir(dir);

	if (ep0_path[0] == '\0') {
		printf("[-] ep0 file not found\n");
		exit(1);
	}
	printf("[+] Found ep0 file: %s\n", ep0_path);

	int ep0 = open(ep0_path, O_RDWR);
	if (ep0 < 0) {
		printf("[-] Failed to open ep0: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] open ep0 successful.\n");

	struct usb_config_descriptor config = {
		.bLength = sizeof(config),
		.bDescriptorType = USB_DT_CONFIG,
		.wTotalLength = htole16(sizeof(config) + sizeof(struct usb_interface_descriptor) + sizeof(struct usb_endpoint_descriptor)),
		.bNumInterfaces = 1,
		.bConfigurationValue = 1,
		.iConfiguration = 0,
		.bmAttributes = USB_CONFIG_ATT_ONE,
		.bMaxPower = 0,
	};

	struct usb_interface_descriptor interface = {
		.bLength = sizeof(interface),
		.bDescriptorType = USB_DT_INTERFACE,
		.bInterfaceNumber = 0,
		.bAlternateSetting = 0,
		.bNumEndpoints = 1,
		.bInterfaceClass = USB_CLASS_VENDOR_SPEC,
		.bInterfaceSubClass = 0,
		.bInterfaceProtocol = 0,
		.iInterface = 0,
	};

	struct usb_endpoint_descriptor endpoint = {
		.bLength = sizeof(endpoint),
		.bDescriptorType = USB_DT_ENDPOINT,
		.bEndpointAddress = USB_DIR_IN | 1,
		.bmAttributes = USB_ENDPOINT_XFER_BULK,
		.wMaxPacketSize = htole16(512),
		.bInterval = 0,
	};

	struct usb_device_descriptor device = {
		.bLength = sizeof(device),
		.bDescriptorType = USB_DT_DEVICE,
		.bcdUSB = htole16(0x0200),
		.bDeviceClass = USB_CLASS_VENDOR_SPEC,
		.bDeviceSubClass = 0,
		.bDeviceProtocol = 0,
		.bMaxPacketSize0 = 64,
		.idVendor = htole16(0x1234),
		.idProduct = htole16(0x5678),
		.bcdDevice = htole16(0x0100),
		.iManufacturer = 0,
		.iProduct = 0,
		.iSerialNumber = 0,
		.bNumConfigurations = 1,
	};

	char buf[1024];
	int len = 0;
	uint32_t tag = 0;
	memcpy(buf + len, &tag, 4); len += 4;
	memcpy(buf + len, &config, sizeof(config)); len += sizeof(config);
	memcpy(buf + len, &interface, sizeof(interface)); len += sizeof(interface);
	memcpy(buf + len, &endpoint, sizeof(endpoint)); len += sizeof(endpoint);
	memcpy(buf + len, &device, sizeof(device)); len += sizeof(device);

	if (write(ep0, buf, len) != len) {
		printf("[-] Failed to write config to ep0: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] write config to ep0 successful.\n");

	dir = opendir("/tmp/gadgetfs");
	if (!dir) {
		printf("[-] Failed to opendir /tmp/gadgetfs: %s\n", strerror(errno));
		exit(1);
	}

	char ep1_path[512] = {0};
	while ((ent = readdir(dir)) != NULL) {
		if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
			char path[512];
			snprintf(path, sizeof(path), "/tmp/gadgetfs/%s", ent->d_name);
			if (strcmp(path, ep0_path) != 0) {
				strcpy(ep1_path, path);
				break;
			}
		}
	}
	closedir(dir);

	if (ep1_path[0] == '\0') {
		printf("[-] endpoint file not found\n");
		exit(1);
	}
	printf("[+] Found endpoint file: %s\n", ep1_path);

	int ep1 = open(ep1_path, O_RDWR);
	if (ep1 < 0) {
		printf("[-] Failed to open ep1: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] open ep1 successful.\n");

	printf("[*] Triggering large kmalloc via write...\n");
	write(ep1, NULL, 0x7fffffff);
	printf("[+] write to ep1 finished.\n");

	return 0;
}
