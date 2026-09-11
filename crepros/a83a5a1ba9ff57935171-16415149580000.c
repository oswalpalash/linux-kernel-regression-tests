// https://syzkaller.appspot.com/bug?id=d167e1e5b87854e67296f9fc84f10ae5386be63f
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/usbdevice_fs.h>
#include <linux/usb/ch9.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>
#include <time.h>
#include <stdint.h>
#include <errno.h>
#include <sys/mount.h>
#include <sys/stat.h>

static int write_file(const char *path, const char *str) {
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        printf("[-] Failed to open %s: %s\n", path, strerror(errno));
        return -1;
    }
    int len = strlen(str);
    if (write(fd, str, len) != len) {
        printf("[-] Failed to write to %s: %s\n", path, strerror(errno));
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

static void cleanup_gadget() {
    write_file("/sys/kernel/config/usb_gadget/g1/UDC", "\n");
    unlink("/sys/kernel/config/usb_gadget/g1/configs/c.1/mass_storage.0");
    rmdir("/sys/kernel/config/usb_gadget/g1/configs/c.1");
    rmdir("/sys/kernel/config/usb_gadget/g1/functions/mass_storage.0");
    rmdir("/sys/kernel/config/usb_gadget/g1/strings/0x409");
    rmdir("/sys/kernel/config/usb_gadget/g1");
}

static void setup_gadget() {
    int res;
    
    res = mount("none", "/sys/kernel/config", "configfs", 0, NULL);
    if (res < 0 && errno != EBUSY) {
        printf("[-] Failed to mount configfs: %s\n", strerror(errno));
    } else {
        printf("[+] configfs mounted.\n");
    }

    cleanup_gadget(); // Ensure clean state

    res = mkdir("/sys/kernel/config/usb_gadget/g1", 0777);
    if (res < 0 && errno != EEXIST) {
        printf("[-] Failed to create gadget dir: %s\n", strerror(errno));
        exit(1);
    }

    write_file("/sys/kernel/config/usb_gadget/g1/idVendor", "0x1d6b");
    write_file("/sys/kernel/config/usb_gadget/g1/idProduct", "0x0104");

    mkdir("/sys/kernel/config/usb_gadget/g1/strings/0x409", 0777);
    mkdir("/sys/kernel/config/usb_gadget/g1/configs/c.1", 0777);
    mkdir("/sys/kernel/config/usb_gadget/g1/functions/mass_storage.0", 0777);

    int fd = open("./dummy_lun", O_CREAT | O_RDWR, 0666);
    if (fd >= 0) {
        if (ftruncate(fd, 1024 * 1024) < 0) {
            printf("[-] ftruncate failed: %s\n", strerror(errno));
        }
        close(fd);
        char cwd[256];
        if (getcwd(cwd, sizeof(cwd))) {
            char path[512];
            snprintf(path, sizeof(path), "%s/dummy_lun", cwd);
            write_file("/sys/kernel/config/usb_gadget/g1/functions/mass_storage.0/lun.0/file", path);
        }
    }

    res = symlink("/sys/kernel/config/usb_gadget/g1/functions/mass_storage.0",
                  "/sys/kernel/config/usb_gadget/g1/configs/c.1/mass_storage.0");
    if (res < 0 && errno != EEXIST) {
        printf("[-] Failed to symlink function: %s\n", strerror(errno));
        exit(1);
    }

    DIR *dir = opendir("/sys/class/udc");
    if (!dir) {
        printf("[-] Failed to open /sys/class/udc: %s\n", strerror(errno));
        exit(1);
    }
    struct dirent *ent;
    char udc_name[256] = {0};
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_name[0] != '.') {
            strncpy(udc_name, ent->d_name, sizeof(udc_name) - 1);
            break;
        }
    }
    closedir(dir);

    if (udc_name[0] == 0) {
        printf("[-] No UDC found\n");
        exit(1);
    }

    printf("[+] Found UDC: %s\n", udc_name);
    res = write_file("/sys/kernel/config/usb_gadget/g1/UDC", udc_name);
    if (res < 0) {
        printf("[-] Failed to bind UDC\n");
        exit(1);
    }
    printf("[+] Gadget bound to UDC.\n");
}

static int find_usb_device(char *dev_path, size_t max_len) {
    DIR *dir = opendir("/sys/bus/usb/devices");
    if (!dir) {
        printf("[-] opendir /sys/bus/usb/devices failed: %s\n", strerror(errno));
        return -1;
    }
    
    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_name[0] == '.') continue;
        char path[256], vid[16] = {0}, pid[16] = {0};
        
        snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/idVendor", ent->d_name);
        int fd = open(path, O_RDONLY);
        if (fd >= 0) {
            int n = read(fd, vid, sizeof(vid)-1);
            if (n > 0) vid[n] = 0;
            close(fd);
        }
        
        snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/idProduct", ent->d_name);
        fd = open(path, O_RDONLY);
        if (fd >= 0) {
            int n = read(fd, pid, sizeof(pid)-1);
            if (n > 0) pid[n] = 0;
            close(fd);
        }
        
        if (strncmp(vid, "1d6b", 4) == 0 && strncmp(pid, "0104", 4) == 0) {
            int busnum = 0, devnum = 0;
            char buf[32];
            
            snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/busnum", ent->d_name);
            fd = open(path, O_RDONLY);
            if (fd >= 0) {
                int n = read(fd, buf, sizeof(buf)-1);
                if (n > 0) { buf[n] = 0; busnum = atoi(buf); }
                close(fd);
            }
            
            snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/devnum", ent->d_name);
            fd = open(path, O_RDONLY);
            if (fd >= 0) {
                int n = read(fd, buf, sizeof(buf)-1);
                if (n > 0) { buf[n] = 0; devnum = atoi(buf); }
                close(fd);
            }
            
            snprintf(dev_path, max_len, "/dev/bus/usb/%03d/%03d", busnum, devnum);
            closedir(dir);
            return 0;
        }
    }
    closedir(dir);
    return -1;
}

volatile int stop = 0;
int dev_fd = -1;

void* race_thread(void* arg) {
    struct usb_ctrlrequest ctrl_set_config = {
        .bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE,
        .bRequest = USB_REQ_SET_CONFIGURATION,
        .wValue = 1,
        .wIndex = 0,
        .wLength = 0,
    };

    struct usb_ctrlrequest ctrl_vendor = {
        .bRequestType = USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_INTERFACE,
        .bRequest = 0,
        .wValue = 0,
        .wIndex = 0,
        .wLength = 0,
    };

    struct usbdevfs_urb urb_set_config = {
        .type = USBDEVFS_URB_TYPE_CONTROL,
        .endpoint = 0,
        .buffer = &ctrl_set_config,
        .buffer_length = sizeof(ctrl_set_config),
        .usercontext = (void*)1,
    };

    struct usbdevfs_urb urb_vendor = {
        .type = USBDEVFS_URB_TYPE_CONTROL,
        .endpoint = 0,
        .buffer = &ctrl_vendor,
        .buffer_length = sizeof(ctrl_vendor),
        .usercontext = (void*)2,
    };

    unsigned int seed = (unsigned int)pthread_self();
    int config_val = 0;
    
    while (!stop) {
        // Alternate between configuration 1 and 0 to repeatedly trigger do_set_interface()
        config_val = 1 - config_val;
        ctrl_set_config.wValue = config_val;

        int ret1 = ioctl(dev_fd, USBDEVFS_SUBMITURB, &urb_set_config);
        int ret2 = ioctl(dev_fd, USBDEVFS_SUBMITURB, &urb_vendor);
        
        if (ret1 == 0) {
            // Random busy-wait delay to allow dummy_timer to process the SET_CONFIGURATION URB
            // before we discard it. This clears delayed_status and forces the VENDOR URB to be processed.
            seed = seed * 1103515245 + 12345;
            int delay = seed % 100000;
            for (volatile int i = 0; i < delay; i++);
            
            ioctl(dev_fd, USBDEVFS_DISCARDURB, &urb_set_config);
        }

        struct usbdevfs_urb *reaped_urb;
        if (ret1 == 0) ioctl(dev_fd, USBDEVFS_REAPURB, &reaped_urb);
        if (ret2 == 0) ioctl(dev_fd, USBDEVFS_REAPURB, &reaped_urb);
    }
    return NULL;
}

int main() {
    setup_gadget();

    char dev_path[256] = {0};
    int retries = 5;
    while (retries-- > 0) {
        if (find_usb_device(dev_path, sizeof(dev_path)) == 0) break;
        sleep(1);
    }
    if (dev_path[0] == 0) {
        printf("[-] Target dummy device not found.\n");
        return 1;
    }
    printf("[+] Found device at %s\n", dev_path);

    dev_fd = open(dev_path, O_RDWR);
    if (dev_fd < 0) {
        printf("[-] Failed to open %s: %s\n", dev_path, strerror(errno));
        return 1;
    }

    #define NUM_THREADS 4
    pthread_t t[NUM_THREADS];

    printf("[+] Starting race threads...\n");
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&t[i], NULL, race_thread, NULL);
    }

    sleep(10);
    stop = 1;

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(t[i], NULL);
    }

    close(dev_fd);
    printf("[+] Race loop finished.\n");
    
    cleanup_gadget();
    sleep(2);
    
    return 0;
}
