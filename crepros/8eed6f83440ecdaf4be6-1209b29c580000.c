// https://syzkaller.appspot.com/bug?id=963f1092456516a85e453ae63e61c132411c8881
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>

void write_file(const char *path, const char *val) {
    int fd = open(path, O_WRONLY);
    if (fd >= 0) {
        if (write(fd, val, strlen(val)) < 0) {
            printf("[-] Failed to write to %s: %s\n", path, strerror(errno));
        }
        close(fd);
    } else {
        printf("[-] Failed to open %s for writing: %s\n", path, strerror(errno));
    }
}

void write_file_quiet(const char *path, const char *val) {
    int fd = open(path, O_WRONLY);
    if (fd >= 0) {
        if (write(fd, val, strlen(val)) < 0) {}
        close(fd);
    }
}

char* find_safe_pci_device(void) {
    static char pci_addr[256];
    DIR *dir = opendir("/sys/bus/pci/devices");
    if (!dir) {
        printf("[-] Failed to opendir /sys/bus/pci/devices: %s\n", strerror(errno));
        return NULL;
    }

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_name[0] == '.') continue;

        char path[512];
        snprintf(path, sizeof(path), "/sys/bus/pci/devices/%s/class", ent->d_name);
        int fd = open(path, O_RDONLY);
        if (fd < 0) continue;

        char class_buf[32] = {0};
        if (read(fd, class_buf, sizeof(class_buf) - 1) > 0) {
            unsigned int class_val = 0;
            if (sscanf(class_buf, "0x%x", &class_val) == 1) {
                unsigned int base_class = (class_val >> 16) & 0xff;
                // Skip Storage (0x01), Network (0x02), Bridge (0x06)
                if (base_class != 0x01 && base_class != 0x02 && base_class != 0x06) {
                    strcpy(pci_addr, ent->d_name);
                    close(fd);
                    closedir(dir);
                    return pci_addr;
                }
            }
        }
        close(fd);
    }
    closedir(dir);
    
    // Fallback: just find any device that is not a bridge
    dir = opendir("/sys/bus/pci/devices");
    if (!dir) return NULL;
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_name[0] == '.') continue;
        char path[512];
        snprintf(path, sizeof(path), "/sys/bus/pci/devices/%s/class", ent->d_name);
        int fd = open(path, O_RDONLY);
        if (fd < 0) continue;
        char class_buf[32] = {0};
        if (read(fd, class_buf, sizeof(class_buf) - 1) > 0) {
            unsigned int class_val = 0;
            if (sscanf(class_buf, "0x%x", &class_val) == 1) {
                unsigned int base_class = (class_val >> 16) & 0xff;
                if (base_class != 0x06) {
                    strcpy(pci_addr, ent->d_name);
                    close(fd);
                    closedir(dir);
                    return pci_addr;
                }
            }
        }
        close(fd);
    }
    closedir(dir);
    return NULL;
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("[*] Setting up fault injection...\n");
    write_file("/sys/kernel/debug/fail_page_alloc/ignore-gfp-wait", "N");
    write_file("/sys/kernel/debug/fail_page_alloc/probability", "100");
    write_file("/sys/kernel/debug/fail_page_alloc/times", "-1");
    write_file("/sys/kernel/debug/fail_page_alloc/task-filter", "Y");
    write_file("/sys/kernel/debug/fail_page_alloc/min-order", "4");
    write_file("/proc/self/make-it-fail", "1");
    printf("[+] Fault injection configured.\n");

    char *pci_addr = find_safe_pci_device();
    if (!pci_addr) {
        printf("[-] No safe PCI devices found\n");
        return 1;
    }
    printf("[+] Found safe PCI device: %s\n", pci_addr);

    char path[512];
    char vendor[32] = {0}, device[32] = {0};
    
    snprintf(path, sizeof(path), "/sys/bus/pci/devices/%s/vendor", pci_addr);
    int fd = open(path, O_RDONLY);
    if (fd >= 0) {
        if (read(fd, vendor, sizeof(vendor)-1) < 0) {}
        close(fd);
    } else {
        printf("[-] Failed to read vendor: %s\n", strerror(errno));
        return 1;
    }
    
    snprintf(path, sizeof(path), "/sys/bus/pci/devices/%s/device", pci_addr);
    fd = open(path, O_RDONLY);
    if (fd >= 0) {
        if (read(fd, device, sizeof(device)-1) < 0) {}
        close(fd);
    } else {
        printf("[-] Failed to read device: %s\n", strerror(errno));
        return 1;
    }

    vendor[strcspn(vendor, "\n")] = 0;
    device[strcspn(device, "\n")] = 0;
    printf("[*] Device ID: %s %s\n", vendor, device);

    printf("[*] Unbinding device from current driver...\n");
    snprintf(path, sizeof(path), "/sys/bus/pci/devices/%s/driver/unbind", pci_addr);
    write_file_quiet(path, pci_addr);

    printf("[*] Adding ID to vmxnet3...\n");
    char id_str[128];
    snprintf(id_str, sizeof(id_str), "%s %s", vendor, device);
    write_file("/sys/bus/pci/drivers/vmxnet3/new_id", id_str);

    printf("[*] Binding device to vmxnet3...\n");
    write_file("/sys/bus/pci/drivers/vmxnet3/bind", pci_addr);

    printf("[+] Done.\n");
    return 0;
}
