// https://syzkaller.appspot.com/bug?id=1073a31aac1412da71a28463403c5ec71832ac6c
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <dirent.h>
#include <linux/uhid.h>
#include <linux/input.h>

#ifndef BUS_USB
#define BUS_USB 0x03
#endif

char rdesc[] = {
    0x06, 0x00, 0xff,  // Usage Page (Vendor Defined 0xFF00)
    0x09, 0x01,        // Usage (0x01)
    0xa1, 0x01,        // Collection (Application)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xff, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x40,        //   Report Count (64)
    0x09, 0x01,        //   Usage (0x01)
    0x81, 0x02,        //   Input (Data,Var,Abs)
    0x95, 0x40,        //   Report Count (64)
    0x09, 0x01,        //   Usage (0x01)
    0x91, 0x02,        //   Output (Data,Var,Abs)
    0xc0               // End Collection
};

char udc_name[256] = "dummy_udc.0";

void find_udc() {
    DIR *dir = opendir("/sys/class/udc");
    if (dir) {
        struct dirent *ent;
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_name[0] != '.') {
                strcpy(udc_name, ent->d_name);
                if (strstr(ent->d_name, "dummy") != NULL)
                    break;
            }
        }
        closedir(dir);
    }
}

void write_file(const char *path, const char *val) {
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        printf("[-] Failed to open %s: %s\n", path, strerror(errno));
        exit(1);
    }
    if (write(fd, val, strlen(val)) < 0) {
        if (strcmp(path, "/sys/kernel/config/usb_gadget/g1/UDC") == 0 && errno == EBUSY) {
            // Ignore EBUSY for UDC
        } else {
            printf("[-] Failed to write to %s: %s\n", path, strerror(errno));
            exit(1);
        }
    }
    close(fd);
}

void my_mkdir(const char *path) {
    if (mkdir(path, 0777) < 0 && errno != EEXIST) {
        printf("[-] Failed to mkdir %s: %s\n", path, strerror(errno));
        exit(1);
    }
}

void setup_gadget() {
    find_udc();
    if (mount("none", "/sys/kernel/config", "configfs", 0, NULL) < 0 && errno != EBUSY) {
        printf("[-] Failed to mount configfs: %s\n", strerror(errno));
        exit(1);
    }
    my_mkdir("/sys/kernel/config/usb_gadget/g1");
    write_file("/sys/kernel/config/usb_gadget/g1/idVendor", "0x1b1c");
    write_file("/sys/kernel/config/usb_gadget/g1/idProduct", "0x0c10");
    my_mkdir("/sys/kernel/config/usb_gadget/g1/configs/c.1");
    my_mkdir("/sys/kernel/config/usb_gadget/g1/functions/hid.usb0");
    write_file("/sys/kernel/config/usb_gadget/g1/functions/hid.usb0/protocol", "0");
    write_file("/sys/kernel/config/usb_gadget/g1/functions/hid.usb0/subclass", "0");
    write_file("/sys/kernel/config/usb_gadget/g1/functions/hid.usb0/report_length", "64");
    
    int fd = open("/sys/kernel/config/usb_gadget/g1/functions/hid.usb0/report_desc", O_WRONLY);
    if (fd < 0) {
        printf("[-] Failed to open report_desc: %s\n", strerror(errno));
        exit(1);
    }
    if (write(fd, rdesc, sizeof(rdesc)) < 0) {
        printf("[-] Failed to write report_desc: %s\n", strerror(errno));
        exit(1);
    }
    close(fd);
    
    if (symlink("/sys/kernel/config/usb_gadget/g1/functions/hid.usb0", "/sys/kernel/config/usb_gadget/g1/configs/c.1/hid.usb0") < 0 && errno != EEXIST) {
        printf("[-] Failed to symlink: %s\n", strerror(errno));
        exit(1);
    }
    write_file("/sys/kernel/config/usb_gadget/g1/UDC", udc_name);
    printf("[+] Gadget setup successful.\n");
}

void *uhid_thread(void *arg) {
    int fd = open("/dev/uhid", O_RDWR);
    if (fd < 0) {
        printf("[-] Failed to open /dev/uhid: %s\n", strerror(errno));
        exit(1);
    }
    
    struct uhid_event ev = {0};
    ev.type = UHID_CREATE2;
    strcpy((char*)ev.u.create2.name, "test-uhid");
    memcpy(ev.u.create2.rd_data, rdesc, sizeof(rdesc));
    ev.u.create2.rd_size = sizeof(rdesc);
    ev.u.create2.bus = BUS_USB;
    ev.u.create2.vendor = 0x1b1c;
    ev.u.create2.product = 0x0c10;
    if (write(fd, &ev, sizeof(ev)) < 0) {
        printf("[-] Failed to write UHID_CREATE2: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] UHID device created.\n");
    
    while (1) {
        int ret = read(fd, &ev, sizeof(ev));
        if (ret < 0) {
            printf("[-] uhid read failed: %s\n", strerror(errno));
            exit(1);
        }
        if (ret > 0 && ev.type == UHID_OUTPUT) {
            struct uhid_event reply = {0};
            reply.type = UHID_INPUT2;
            reply.u.input2.size = 64;
            if (write(fd, &reply, sizeof(reply)) < 0) {
                printf("[-] uhid write failed: %s\n", strerror(errno));
                exit(1);
            }
            printf("[+] uhid replied to output report.\n");
        }
    }
    return NULL;
}

void *hidg_thread(void *arg) {
    int fd = -1;
    while (fd < 0) {
        for (int i = 0; i < 8; i++) {
            char path[32];
            sprintf(path, "/dev/hidg%d", i);
            fd = open(path, O_RDWR);
            if (fd >= 0) {
                printf("[+] Opened %s\n", path);
                break;
            }
        }
        if (fd < 0) usleep(100000);
    }
    
    while (1) {
        char buf[64];
        int ret = read(fd, buf, sizeof(buf));
        if (ret < 0) {
            printf("[-] hidg read failed: %s\n", strerror(errno));
            exit(1);
        }
        if (ret > 0) {
            char reply[64] = {0};
            if (write(fd, reply, sizeof(reply)) < 0) {
                printf("[-] hidg write failed: %s\n", strerror(errno));
                exit(1);
            }
            printf("[+] hidg replied to output report.\n");
        }
    }
    return NULL;
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    pthread_t t1, t2;
    
    printf("[*] Starting UHID thread...\n");
    pthread_create(&t1, NULL, uhid_thread, NULL);
    
    sleep(2);
    
    printf("[*] Starting Gadget thread...\n");
    setup_gadget();
    pthread_create(&t2, NULL, hidg_thread, NULL);
    
    sleep(5);
    printf("[+] Done.\n");
    return 0;
}
