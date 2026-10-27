// https://syzkaller.appspot.com/bug?id=68c0b6bdd9696979e46cf78b5b4da8d64902667f
#define _GNU_SOURCE
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/major.h>
#include <linux/raid/md_u.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main(void) {
    // 1. Open or create the MD device
    int fd = open("/dev/md0", O_RDWR);
    if (fd < 0) {
        if (mknod("/dev/md0", S_IFBLK | 0600, makedev(9, 0)) < 0 && errno != EEXIST) {
            printf("[-] Failed to mknod /dev/md0: %s\n", strerror(errno));
            exit(1);
        }
        fd = open("/dev/md0", O_RDWR);
        if (fd < 0) {
            printf("[-] Failed to open /dev/md0: %s\n", strerror(errno));
            exit(1);
        }
    }
    printf("[+] /dev/md0 opened successfully.\n");

    // 2. Stop the array in case it's already running
    ioctl(fd, STOP_ARRAY, NULL);
    printf("[+] STOP_ARRAY executed.\n");

    // 3. Set array info with a massive raid_disks count
    mdu_array_info_t array_info;
    memset(&array_info, 0, sizeof(array_info));
    array_info.level = 0; // RAID0
    array_info.size = 0;
    array_info.not_persistent = 1;
    array_info.raid_disks = 0x40000000; // Large value to trigger allocation > INT_MAX
    array_info.chunk_size = 65536;      // 64K to avoid PAGE_SECTORS warning in blk_validate_limits

    if (ioctl(fd, SET_ARRAY_INFO, &array_info) < 0) {
        printf("[-] Failed to SET_ARRAY_INFO: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] SET_ARRAY_INFO successful.\n");

    // 4. Open or create a backing device (ramdisk)
    int ram_fd = open("/dev/ram1", O_RDWR);
    if (ram_fd < 0) {
        if (mknod("/dev/ram1", S_IFBLK | 0600, makedev(1, 1)) < 0 && errno != EEXIST) {
            printf("[-] Failed to mknod /dev/ram1: %s\n", strerror(errno));
            exit(1);
        }
        ram_fd = open("/dev/ram1", O_RDWR);
        if (ram_fd < 0) {
            printf("[-] Failed to open /dev/ram1: %s\n", strerror(errno));
            exit(1);
        }
    }
    printf("[+] /dev/ram1 opened successfully.\n");

    struct stat st;
    if (fstat(ram_fd, &st) < 0) {
        printf("[-] Failed to fstat /dev/ram1: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] fstat /dev/ram1 successful.\n");

    // 5. Add the backing device to the array
    mdu_disk_info_t disk_info;
    memset(&disk_info, 0, sizeof(disk_info));
    disk_info.major = major(st.st_rdev);
    disk_info.minor = minor(st.st_rdev);
    disk_info.raid_disk = 0;
    disk_info.state = 0;

    if (ioctl(fd, ADD_NEW_DISK, &disk_info) < 0) {
        printf("[-] Failed to ADD_NEW_DISK: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] ADD_NEW_DISK successful.\n");

    // 6. Run the array to trigger the warning in create_strip_zones
    if (ioctl(fd, RUN_ARRAY, NULL) < 0) {
        printf("[-] RUN_ARRAY failed (expected if bug triggers or due to invalid config): %s\n", strerror(errno));
    } else {
        printf("[+] RUN_ARRAY successful.\n");
    }

    close(ram_fd);
    close(fd);
    return 0;
}