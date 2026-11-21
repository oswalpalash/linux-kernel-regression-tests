// https://syzkaller.appspot.com/bug?id=cb1fcc879c9fcbf4ceaa70d2dc7392aea152dd53
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <stdint.h>
#include <stdbool.h>
#include <dirent.h>
#include <errno.h>
#include <sched.h>

void write_file(const char *path, const char *val) {
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        // We don't exit here because failslab might not be available in all kernels,
        // but we print the error.
        printf("[-] Failed to open %s: %s\n", path, strerror(errno));
        return;
    }
    if (write(fd, val, strlen(val)) < 0) {
        printf("[-] Failed to write to %s: %s\n", path, strerror(errno));
    }
    close(fd);
}

struct usb_config_descriptor {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t wTotalLength;
    uint8_t  bNumInterfaces;
    uint8_t  bConfigurationValue;
    uint8_t  iConfiguration;
    uint8_t  bmAttributes;
    uint8_t  bMaxPower;
} __attribute__ ((packed));

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
} __attribute__ ((packed));

struct config_data {
    uint32_t tag;
    struct usb_config_descriptor config;
    struct usb_device_descriptor device;
} __attribute__ ((packed));

void trigger_bug(int nth) {
    int res;

    res = unshare(CLONE_NEWNS);
    if (res < 0) {
        printf("[-] Failed to unshare: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] unshare successful.\n");

    res = mount(NULL, "/", NULL, MS_PRIVATE | MS_REC, NULL);
    if (res < 0) {
        printf("[-] Failed to mount / private: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] mount / private successful.\n");

    res = mkdir("/tmp/gadgetfs", 0777);
    if (res < 0 && errno != EEXIST) {
        printf("[-] Failed to mkdir /tmp/gadgetfs: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] mkdir /tmp/gadgetfs successful.\n");

    res = mount("gadgetfs", "/tmp/gadgetfs", "gadgetfs", 0, NULL);
    if (res < 0) {
        printf("[-] Failed to mount gadgetfs: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] mount gadgetfs successful.\n");

    char ep0_name[256] = {0};
    DIR *dir = opendir("/tmp/gadgetfs");
    if (!dir) {
        printf("[-] Failed to opendir /tmp/gadgetfs: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] opendir /tmp/gadgetfs successful.\n");

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_name[0] != '.') {
            strncpy(ep0_name, ent->d_name, sizeof(ep0_name) - 1);
            break;
        }
    }
    closedir(dir);

    if (ep0_name[0] == '\0') {
        printf("[-] Failed to find ep0 file\n");
        umount("/tmp/gadgetfs");
        exit(1);
    }
    printf("[+] Found ep0 file: %s\n", ep0_name);

    char ep0_path[512];
    snprintf(ep0_path, sizeof(ep0_path), "/tmp/gadgetfs/%s", ep0_name);

    int fd = open(ep0_path, O_RDWR);
    if (fd < 0) {
        printf("[-] Failed to open %s: %s\n", ep0_path, strerror(errno));
        umount("/tmp/gadgetfs");
        exit(1);
    }
    printf("[+] open %s successful.\n", ep0_path);

    struct config_data data;
    memset(&data, 0, sizeof(data));
    data.tag = 0;

    data.config.bLength = 9;
    data.config.bDescriptorType = 2; // USB_DT_CONFIG
    data.config.wTotalLength = 9;
    data.config.bConfigurationValue = 1;
    data.config.bmAttributes = 0x80; // USB_CONFIG_ATT_ONE

    data.device.bLength = 18;
    data.device.bDescriptorType = 1; // USB_DT_DEVICE
    data.device.bNumConfigurations = 1;

    write_file("/sys/kernel/debug/failslab/ignore-gfp-wait", "N");
    write_file("/sys/kernel/debug/failslab/task-filter", "Y");
    write_file("/sys/kernel/debug/failslab/probability", "100");
    write_file("/sys/kernel/debug/failslab/times", "1");
    write_file("/proc/self/make-it-fail", "1");
    
    char nth_str[32];
    snprintf(nth_str, sizeof(nth_str), "%d", nth);
    char path[256];
    snprintf(path, sizeof(path), "/proc/self/task/%ld/fail-nth", syscall(SYS_gettid));
    write_file(path, nth_str);

    printf("[*] Triggering bug with fail-nth=%d...\n", nth);
    // Expected to fail due to fault injection
    if (write(fd, &data, sizeof(data)) < 0) {
        printf("[+] write to ep0 failed as expected (fault injection).\n");
    } else {
        printf("[-] write to ep0 succeeded (fault injection missed).\n");
    }

    write_file("/sys/kernel/debug/failslab/probability", "0");
    write_file("/sys/kernel/debug/failslab/times", "0");
    write_file("/proc/self/make-it-fail", "0");
    write_file(path, "0");

    res = close(fd);
    if (res < 0) {
        printf("[-] Failed to close fd: %s\n", strerror(errno));
    } else {
        printf("[+] close fd successful.\n");
    }
    
    res = umount("/tmp/gadgetfs");
    if (res < 0) {
        printf("[-] Failed to umount gadgetfs: %s\n", strerror(errno));
    } else {
        printf("[+] umount gadgetfs successful.\n");
    }
}

int main() {
    printf("[*] Starting bug reproduction...\n");
    
    int res = mount("debugfs", "/sys/kernel/debug", "debugfs", 0, NULL);
    if (res < 0 && errno != EBUSY) {
        printf("[-] Failed to mount debugfs: %s\n", strerror(errno));
    } else {
        printf("[+] debugfs mounted or already present.\n");
    }

    for (int i = 1; i <= 50; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            printf("[-] Failed to fork: %s\n", strerror(errno));
            exit(1);
        }
        if (pid == 0) {
            trigger_bug(i);
            exit(0);
        }
        wait(NULL);
    }
    
    printf("[+] Bug reproduction finished.\n");
    return 0;
}
