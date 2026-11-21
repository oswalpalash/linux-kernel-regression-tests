// https://syzkaller.appspot.com/bug?id=0ea6b39d3e6ea961cdf3df179d73f36321e2da7a
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mount.h>

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	printf("[*] Starting reproducer for circular locking dependency in uvcg_extension_ba_source_id_show...\n");

	/* Step 1: Ensure /sys/kernel/config exists and configfs is mounted */
	int res = mkdir("/sys/kernel/config", 0755);
	if (res < 0 && errno != EEXIST) {
		printf("[-] Failed to mkdir /sys/kernel/config: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] /sys/kernel/config directory checked.\n");

	res = mount("none", "/sys/kernel/config", "configfs", 0, NULL);
	if (res < 0 && errno != EBUSY) {
		printf("[-] Failed to mount configfs: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] configfs mounted or already present.\n");

	/* Step 2: Establish dependency: inode_lock -> &frag->frag_sem
	 * CRITICAL: Do NOT create or remove dummy directories inside
	 * /sys/kernel/config/usb_gadget/ or /sys/kernel/config/target/
	 * to avoid acquiring gadget_subsys.su_mutex or target_core_fabrics.su_mutex,
	 * which causes a premature 3-lock circular dependency.
	 * Instead, discover and use an independent third subsystem (e.g. netconsole or nullb).
	 */
	int established = 0;
	DIR *d = opendir("/sys/kernel/config");
	if (d) {
		struct dirent *de;
		while ((de = readdir(d)) != NULL) {
			if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
				continue;
			if (!strcmp(de->d_name, "usb_gadget") || !strcmp(de->d_name, "target"))
				continue;

			char path[256];
			snprintf(path, sizeof(path), "/sys/kernel/config/%s/test_tgt", de->d_name);
			printf("[*] Probing third configfs subsystem '%s' at %s...\n", de->d_name, path);
			if (mkdir(path, 0755) == 0) {
				printf("[+] Created %s\n", path);
				if (rmdir(path) == 0) {
					printf("[+] Removed %s (established inode_lock -> frag_sem via %s).\n", path, de->d_name);
					established = 1;
					break;
				} else {
					printf("[-] Failed to rmdir %s: %s\n", path, strerror(errno));
				}
			} else {
				printf("[-] mkdir %s failed: %s\n", path, strerror(errno));
			}
		}
		closedir(d);
	}

	if (!established) {
		printf("[*] Trying fallback third subsystems netconsole and nullb...\n");
		if (mkdir("/sys/kernel/config/netconsole/test_tgt", 0755) == 0) {
			if (rmdir("/sys/kernel/config/netconsole/test_tgt") == 0) {
				printf("[+] Established inode_lock -> frag_sem via netconsole.\n");
				established = 1;
			}
		} else if (mkdir("/sys/kernel/config/nullb/test_tgt", 0755) == 0) {
			if (rmdir("/sys/kernel/config/nullb/test_tgt") == 0) {
				printf("[+] Established inode_lock -> frag_sem via nullb.\n");
				established = 1;
			}
		}
	}

	if (!established) {
		printf("[-] Failed to establish inode_lock -> frag_sem via a third subsystem.\n");
		exit(1);
	}

	/* Step 3: Establish dependency: gadget_subsys.su_mutex -> &opts->dep_lock
	 * Creating /sys/kernel/config/usb_gadget/g1/functions/tcm.usb0 calls function_make()
	 * holding gadget_subsys.su_mutex, which invokes tcm_set_name() acquiring &opts->dep_lock.
	 */
	res = mkdir("/sys/kernel/config/usb_gadget/g1", 0755);
	if (res < 0 && errno != EEXIST) {
		printf("[-] Failed to mkdir g1: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] Created gadget g1.\n");

	res = mkdir("/sys/kernel/config/usb_gadget/g1/functions/tcm.usb0", 0755);
	if (res < 0 && errno != EEXIST) {
		printf("[-] Failed to mkdir tcm.usb0: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] Created function tcm.usb0 (established gadget_subsys.su_mutex -> opts->dep_lock dependency).\n");

	/* Step 4: Establish dependency: &opts->dep_lock -> inode_lock
	 * Register target/usb_gadget fabric and create WWN and TPG.
	 * usbg_make_tpg() acquires &opts->dep_lock and calls configfs_depend_item_unlocked(),
	 * which acquires inode_lock on the configfs root dentry.
	 * Since Step 2 used a neutral third subsystem and did not touch target_core_fabrics.su_mutex,
	 * no premature collision occurs here.
	 */
	res = mkdir("/sys/kernel/config/target/usb_gadget", 0755);
	if (res < 0 && errno != EEXIST) {
		printf("[-] Failed to mkdir /sys/kernel/config/target/usb_gadget: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] Target usb_gadget fabric registered.\n");

	res = mkdir("/sys/kernel/config/target/usb_gadget/naa.1234567890123456", 0755);
	if (res < 0 && errno != EEXIST) {
		printf("[-] Failed to mkdir target WWN naa.1234567890123456: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] Created target WWN naa.1234567890123456.\n");

	res = mkdir("/sys/kernel/config/target/usb_gadget/naa.1234567890123456/tpgt_1", 0755);
	if (res < 0 && errno != EEXIST) {
		printf("[-] Failed to mkdir target tpgt_1: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] Created target portal group tpgt_1 (established opts->dep_lock -> inode_lock dependency).\n");

	/* Step 5: Create UVC function and Extension Unit */
	res = mkdir("/sys/kernel/config/usb_gadget/g1/functions/uvc.usb0", 0755);
	if (res < 0 && errno != EEXIST) {
		printf("[-] Failed to mkdir uvc.usb0: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] Created function uvc.usb0.\n");

	res = mkdir("/sys/kernel/config/usb_gadget/g1/functions/uvc.usb0/control/extensions/xu0", 0755);
	if (res < 0 && errno != EEXIST) {
		printf("[-] Failed to mkdir xu0: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] Created extension unit xu0.\n");

	/* Step 6: Trigger reverse dependency: &frag->frag_sem -> gadget_subsys.su_mutex
	 * Reading baSourceID calls configfs_read_iter() holding down_read(&frag->frag_sem),
	 * which calls uvcg_extension_ba_source_id_show() where mutex_lock(su_mutex) is invoked,
	 * completing the 4-lock circular cycle and cleanly triggering:
	 * possible deadlock in uvcg_extension_ba_source_id_show
	 */
	const char *attr_path = "/sys/kernel/config/usb_gadget/g1/functions/uvc.usb0/control/extensions/xu0/baSourceID";
	int fd = open(attr_path, O_RDONLY);
	if (fd < 0) {
		printf("[-] Failed to open %s: %s\n", attr_path, strerror(errno));
		exit(1);
	}
	printf("[+] Opened %s successfully.\n", attr_path);

	char buf[128];
	ssize_t n = read(fd, buf, sizeof(buf));
	if (n < 0) {
		printf("[-] Failed to read %s: %s\n", attr_path, strerror(errno));
		close(fd);
		exit(1);
	}
	printf("[+] Read %zd bytes from %s.\n", n, attr_path);
	close(fd);

	/* Allow any asynchronous console/lockdep logs to flush */
	sleep(1);
	printf("[+] Reproducer finished.\n");

	return 0;
}
