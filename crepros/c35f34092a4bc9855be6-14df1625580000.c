// https://syzkaller.appspot.com/bug?id=017fcaefb318d50250802887137c879246165ccf
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>

int write_file(const char *path, const char *val) {
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        printf("[-] Failed to open %s: %s\n", path, strerror(errno));
        return -1;
    }
    ssize_t ret = write(fd, val, strlen(val));
    if (ret < 0) {
        printf("[-] Failed to write to %s: %s\n", path, strerror(errno));
        close(fd);
        return -1;
    }
    close(fd);
    printf("[+] Wrote '%s' to %s\n", val, path);
    return 0;
}

int make_dir(const char *path) {
    if (mkdir(path, 0777) < 0) {
        if (errno != EEXIST) {
            printf("[-] Failed to mkdir %s: %s\n", path, strerror(errno));
            return -1;
        }
    }
    printf("[+] Created directory %s\n", path);
    return 0;
}

int main(void) {
    char gadget_path[256];
    snprintf(gadget_path, sizeof(gadget_path), "/sys/kernel/config/usb_gadget/g_midi2_%d", getpid());
    
    if (make_dir(gadget_path) < 0) exit(1);
    
    char path[512];
    
    snprintf(path, sizeof(path), "%s/idVendor", gadget_path);
    if (write_file(path, "0x1d6b") < 0) exit(1);
    
    snprintf(path, sizeof(path), "%s/idProduct", gadget_path);
    if (write_file(path, "0x0104") < 0) exit(1);

    snprintf(path, sizeof(path), "%s/strings", gadget_path);
    if (make_dir(path) < 0) exit(1);
    
    snprintf(path, sizeof(path), "%s/strings/0x409", gadget_path);
    if (make_dir(path) < 0) exit(1);
    
    snprintf(path, sizeof(path), "%s/strings/0x409/serialnumber", gadget_path);
    if (write_file(path, "123456789") < 0) exit(1);
    
    snprintf(path, sizeof(path), "%s/strings/0x409/manufacturer", gadget_path);
    if (write_file(path, "Syz") < 0) exit(1);
    
    snprintf(path, sizeof(path), "%s/strings/0x409/product", gadget_path);
    if (write_file(path, "Kaller") < 0) exit(1);

    snprintf(path, sizeof(path), "%s/configs", gadget_path);
    if (make_dir(path) < 0) exit(1);
    
    snprintf(path, sizeof(path), "%s/configs/c.1", gadget_path);
    if (make_dir(path) < 0) exit(1);
    
    snprintf(path, sizeof(path), "%s/configs/c.1/strings", gadget_path);
    if (make_dir(path) < 0) exit(1);
    
    snprintf(path, sizeof(path), "%s/configs/c.1/strings/0x409", gadget_path);
    if (make_dir(path) < 0) exit(1);
    
    snprintf(path, sizeof(path), "%s/configs/c.1/strings/0x409/configuration", gadget_path);
    if (write_file(path, "Config 1") < 0) exit(1);

    snprintf(path, sizeof(path), "%s/functions", gadget_path);
    if (make_dir(path) < 0) exit(1);
    
    snprintf(path, sizeof(path), "%s/functions/midi2.usb0", gadget_path);
    if (make_dir(path) < 0) exit(1);

    snprintf(path, sizeof(path), "%s/functions/midi2.usb0/ep.0/block.0/num_groups", gadget_path);
    if (write_file(path, "16") < 0) exit(1);
    
    snprintf(path, sizeof(path), "%s/functions/midi2.usb0/ep.0/block.0/midi1_num_groups", gadget_path);
    if (write_file(path, "16") < 0) exit(1);
    
    snprintf(path, sizeof(path), "%s/functions/midi2.usb0/ep.0/block.0/direction", gadget_path);
    if (write_file(path, "3") < 0) exit(1);

    char target[512];
    char linkpath[512];
    snprintf(target, sizeof(target), "%s/functions/midi2.usb0", gadget_path);
    snprintf(linkpath, sizeof(linkpath), "%s/configs/c.1/midi2.usb0", gadget_path);
    
    if (symlink(target, linkpath) < 0) {
        if (errno != EEXIST) {
            printf("[-] Failed to symlink: %s\n", strerror(errno));
            exit(1);
        }
    }
    printf("[+] Symlinked function to config\n");

    char udc_name[256] = {0};
    for (int i = 0; i < 20; i++) {
        DIR *dir = opendir("/sys/class/udc");
        if (dir) {
            struct dirent *ent;
            while ((ent = readdir(dir)) != NULL) {
                if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
                    strncpy(udc_name, ent->d_name, sizeof(udc_name) - 1);
                    break;
                }
            }
            closedir(dir);
        }
        if (udc_name[0] != '\0') {
            break;
        }
        usleep(100000);
    }

    if (udc_name[0] == '\0') {
        printf("[-] No UDC found\n");
        exit(1);
    }

    printf("[+] Found UDC: %s\n", udc_name);

    snprintf(path, sizeof(path), "%s/UDC", gadget_path);
    if (write_file(path, udc_name) < 0) exit(1);

    printf("[+] Bound gadget to UDC\n");

    sleep(1);

    return 0;
}