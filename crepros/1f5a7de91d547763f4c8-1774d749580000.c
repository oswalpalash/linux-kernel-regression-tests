// https://syzkaller.appspot.com/bug?id=451f9aa7dc3fd510720ec3973118494b6e308d5f
#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/loop.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <errno.h>

int setup_loop(int index) {
    char path[256];
    sprintf(path, "/tmp/disk%d", index);
    int fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        printf("[-] Failed to open backing file %s: %s\n", path, strerror(errno));
        exit(1);
    }
    if (ftruncate(fd, 10 * 1024 * 1024) < 0) {
        printf("[-] Failed to ftruncate: %s\n", strerror(errno));
        exit(1);
    }
    
    int cfd = open("/dev/loop-control", O_RDWR);
    if (cfd < 0) {
        printf("[-] Failed to open loop-control: %s\n", strerror(errno));
        exit(1);
    }
    int loop_num = ioctl(cfd, LOOP_CTL_GET_FREE);
    close(cfd);
    if (loop_num < 0) {
        printf("[-] LOOP_CTL_GET_FREE failed: %s\n", strerror(errno));
        exit(1);
    }
    
    sprintf(path, "/dev/loop%d", loop_num);
    int loop_fd = open(path, O_RDWR);
    if (loop_fd < 0) {
        if (mknod(path, S_IFBLK | 0644, makedev(7, loop_num)) < 0 && errno != EEXIST) {
            printf("[-] Failed to mknod %s: %s\n", path, strerror(errno));
            exit(1);
        }
        loop_fd = open(path, O_RDWR);
        if (loop_fd < 0) {
            printf("[-] Failed to open loop device %s: %s\n", path, strerror(errno));
            exit(1);
        }
    }
    if (ioctl(loop_fd, LOOP_SET_FD, fd) < 0) {
        printf("[-] LOOP_SET_FD failed: %s\n", strerror(errno));
        exit(1);
    }
    close(loop_fd);
    close(fd);
    
    printf("[+] Setup loop%d\n", loop_num);
    return loop_num;
}

void write_file(const char *path, const char *val) {
    int fd;
    int retries = 50;
    while (retries--) {
        fd = open(path, O_WRONLY);
        if (fd < 0) {
            printf("[-] Failed to open %s: %s\n", path, strerror(errno));
            exit(1);
        }
        if (write(fd, val, strlen(val)) >= 0) {
            close(fd);
            printf("[+] Wrote '%s' to %s\n", val, path);
            return;
        }
        if (errno != EBUSY) {
            printf("[-] Failed to write '%s' to %s: %s\n", val, path, strerror(errno));
            exit(1);
        }
        close(fd);
        usleep(100000); // 100ms
    }
    printf("[-] Failed to write '%s' to %s after retries (EBUSY)\n", val, path);
    exit(1);
}

int main() {
    printf("[*] Setting up loop devices...\n");
    int l1 = setup_loop(1);
    int l2 = setup_loop(2);
    int l3 = setup_loop(3);
    int l4 = setup_loop(4);
    
    printf("[*] Creating md array...\n");
    write_file("/sys/module/md_mod/parameters/new_array", "md123");
    write_file("/sys/block/md123/md/level", "raid5");
    write_file("/sys/block/md123/md/chunk_size", "4096");
    write_file("/sys/block/md123/md/raid_disks", "3");
    
    char buf[256];
    char path[256];
    
    printf("[*] Adding devices to array...\n");
    sprintf(buf, "7:%d", l1);
    write_file("/sys/block/md123/md/new_dev", buf);
    sprintf(path, "/sys/block/md123/md/dev-loop%d/slot", l1);
    write_file(path, "0");
    
    sprintf(buf, "7:%d", l2);
    write_file("/sys/block/md123/md/new_dev", buf);
    sprintf(path, "/sys/block/md123/md/dev-loop%d/slot", l2);
    write_file(path, "1");
    
    sprintf(buf, "7:%d", l3);
    write_file("/sys/block/md123/md/new_dev", buf);
    sprintf(path, "/sys/block/md123/md/dev-loop%d/slot", l3);
    write_file(path, "2");
    
    // Add l4 as a spare (no slot assigned)
    sprintf(buf, "7:%d", l4);
    write_file("/sys/block/md123/md/new_dev", buf);
    
    printf("[*] Skipping initial sync...\n");
    write_file("/sys/block/md123/md/resync_start", "none");
    
    printf("[*] Starting array...\n");
    write_file("/sys/block/md123/md/array_state", "active");
    
    // Wait a bit to ensure array is fully active and any background threads have settled
    sleep(1);
    
    printf("[*] Freezing sync thread...\n");
    write_file("/sys/block/md123/md/sync_action", "frozen");
    
    printf("[*] Changing raid_disks to 4...\n");
    write_file("/sys/block/md123/md/raid_disks", "4");
    
    printf("[*] Stopping array...\n");
    write_file("/sys/block/md123/md/array_state", "inactive");
    
    printf("[*] Restarting array to trigger BUG...\n");
    write_file("/sys/block/md123/md/array_state", "active");
    
    // Wait for asynchronous BUG to trigger if any
    sleep(3);
    
    printf("[+] Done. If the kernel didn't crash, the bug was not triggered.\n");
    return 0;
}
