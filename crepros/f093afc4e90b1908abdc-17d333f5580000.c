// https://syzkaller.appspot.com/bug?id=3fa0f25642d2dd1a84b51be08705a43c50c0cfa3
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <errno.h>

void write_file(const char *path, const char *val) {
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        printf("[-] Failed to open %s: %s\n", path, strerror(errno));
        exit(1);
    }
    if (write(fd, val, strlen(val)) != strlen(val)) {
        printf("[-] Failed to write to %s: %s\n", path, strerror(errno));
        exit(1);
    }
    close(fd);
}

int main() {
    int res;

    // 0. Mount debugfs
    res = mount("debugfs", "/sys/kernel/debug", "debugfs", 0, NULL);
    if (res < 0 && errno != EBUSY) {
        printf("[-] Failed to mount debugfs: %s\n", strerror(errno));
    } else {
        printf("[+] debugfs mounted or already mounted.\n");
    }

    // 1. Mount configfs
    res = mount("none", "/sys/kernel/config", "configfs", 0, NULL);
    if (res < 0 && errno != EBUSY) {
        printf("[-] Failed to mount configfs: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] configfs mounted or already mounted.\n");

    // 2. Create gadget
    res = mkdir("/sys/kernel/config/usb_gadget/g1", 0777);
    if (res < 0 && errno != EEXIST) {
        printf("[-] Failed to create usb_gadget/g1: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] usb_gadget/g1 created.\n");

    // 3. Create uvc function
    res = mkdir("/sys/kernel/config/usb_gadget/g1/functions/uvc.0", 0777);
    if (res < 0 && errno != EEXIST) {
        printf("[-] Failed to create uvc.0 function: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] uvc.0 function created.\n");

    // 4. Configure failslab
    write_file("/sys/kernel/debug/failslab/task-filter", "Y\n");
    write_file("/sys/kernel/debug/failslab/ignore-gfp-wait", "N\n");
    write_file("/sys/kernel/debug/failslab/times", "-1\n");
    write_file("/proc/self/make-it-fail", "1\n");
    write_file("/sys/kernel/debug/failslab/probability", "5\n");
    printf("[+] failslab configured.\n");

    // 5. Trigger Loop
    const char *ext_path = "/sys/kernel/config/usb_gadget/g1/functions/uvc.0/control/extensions/ext1";
    
    printf("[+] Starting trigger loop...\n");
    for (int i = 0; i < 10000; i++) {
        res = mkdir(ext_path, 0777);
        if (res == 0) {
            rmdir(ext_path);
        }
    }
    printf("[+] Loop finished.\n");

    return 0;
}
