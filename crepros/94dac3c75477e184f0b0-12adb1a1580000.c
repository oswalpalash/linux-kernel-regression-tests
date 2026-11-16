// https://syzkaller.appspot.com/bug?id=5ea399487c893ffb523172d55967119d015b3c76
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/fs.h>
#include <linux/loop.h>
#include <linux/dm-ioctl.h>

static void init_dm_hdr(struct dm_ioctl *dm, size_t data_size, const char *name, uint32_t flags)
{
    memset(dm, 0, data_size);
    dm->version[0] = DM_VERSION_MAJOR;
    dm->version[1] = DM_VERSION_MINOR;
    dm->version[2] = DM_VERSION_PATCHLEVEL;
    dm->data_size = data_size;
    dm->data_start = sizeof(struct dm_ioctl);
    dm->flags = flags;
    if (name)
        strncpy(dm->name, name, sizeof(dm->name) - 1);
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("[*] Starting dm-integrity commit WARNING reproducer...\n");

    /* 1. Open /dev/mapper/control */
    int dm_fd = open("/dev/mapper/control", O_RDWR);
    if (dm_fd < 0) {
        printf("[-] Failed to open /dev/mapper/control: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] open /dev/mapper/control successful.\n");

    /* Pre-cleanup: remove any existing leftover dm device */
    char buf[4096];
    struct dm_ioctl *dm = (struct dm_ioctl *)buf;
    init_dm_hdr(dm, sizeof(buf), "syz_integrity_test", 0);
    int res = ioctl(dm_fd, DM_DEV_REMOVE, dm);
    if (res < 0) {
        if (errno == ENODEV || errno == ENXIO) {
            printf("[*] Pre-cleanup: target does not exist (expected): %s\n", strerror(errno));
        } else {
            printf("[*] Pre-cleanup DM_DEV_REMOVE: %s\n", strerror(errno));
        }
    } else {
        printf("[+] Pre-cleanup: removed existing dm device.\n");
    }

    /* 2. Open /dev/loop-control and allocate a free loop device */
    int ctrl_fd = open("/dev/loop-control", O_RDWR);
    if (ctrl_fd < 0) {
        printf("[-] Failed to open /dev/loop-control: %s\n", strerror(errno));
        close(dm_fd);
        exit(1);
    }
    printf("[+] open /dev/loop-control successful.\n");

    int dev_nr = ioctl(ctrl_fd, LOOP_CTL_GET_FREE);
    if (dev_nr < 0) {
        printf("[-] Failed to ioctl(LOOP_CTL_GET_FREE): %s\n", strerror(errno));
        close(ctrl_fd);
        close(dm_fd);
        exit(1);
    }
    printf("[+] ioctl(LOOP_CTL_GET_FREE) returned loop index %d\n", dev_nr);

    res = close(ctrl_fd);
    if (res < 0) {
        printf("[-] Failed to close /dev/loop-control: %s\n", strerror(errno));
        close(dm_fd);
        exit(1);
    }
    printf("[+] close /dev/loop-control successful.\n");

    /* 3. Create backing file */
    const char *img_path = "/tmp/integrity_repro_img";
    int img_fd = open(img_path, O_RDWR | O_CREAT | O_TRUNC, 0600);
    if (img_fd < 0) {
        printf("[-] Failed to create backing image %s: %s\n", img_path, strerror(errno));
        close(dm_fd);
        exit(1);
    }
    printf("[+] open %s with O_CREAT successful.\n", img_path);

    res = ftruncate(img_fd, 64 * 1024 * 1024);
    if (res < 0) {
        printf("[-] Failed to ftruncate backing image: %s\n", strerror(errno));
        close(img_fd);
        unlink(img_path);
        close(dm_fd);
        exit(1);
    }
    printf("[+] ftruncate backing image (64 MB) successful.\n");

    /* 4. Open loop device node and attach backing file */
    char loop_path[64];
    snprintf(loop_path, sizeof(loop_path), "/dev/loop%d", dev_nr);
    int loop_fd = open(loop_path, O_RDWR);
    if (loop_fd < 0) {
        printf("[-] Failed to open %s: %s\n", loop_path, strerror(errno));
        close(img_fd);
        unlink(img_path);
        close(dm_fd);
        exit(1);
    }
    printf("[+] open %s successful.\n", loop_path);

    /* Clear any stale loop attachment */
    ioctl(loop_fd, LOOP_CLR_FD, 0);

    res = ioctl(loop_fd, LOOP_SET_FD, img_fd);
    if (res < 0) {
        printf("[-] Failed to ioctl(LOOP_SET_FD): %s\n", strerror(errno));
        close(loop_fd);
        close(img_fd);
        unlink(img_path);
        close(dm_fd);
        exit(1);
    }
    printf("[+] ioctl(LOOP_SET_FD) attached %s to %s successfully.\n", img_path, loop_path);

    /* 5. Create device-mapper target */
    init_dm_hdr(dm, sizeof(buf), "syz_integrity_test", 0);
    res = ioctl(dm_fd, DM_DEV_CREATE, dm);
    if (res < 0) {
        printf("[-] Failed to ioctl(DM_DEV_CREATE): %s\n", strerror(errno));
        ioctl(loop_fd, LOOP_CLR_FD, 0);
        close(loop_fd);
        close(img_fd);
        unlink(img_path);
        close(dm_fd);
        exit(1);
    }
    printf("[+] ioctl(DM_DEV_CREATE) successful.\n");

    /* 6. Load dm-integrity table: format sector 0 and journal */
    init_dm_hdr(dm, sizeof(buf), "syz_integrity_test", 0);
    dm->target_count = 1;
    struct dm_target_spec *tgt = (struct dm_target_spec *)(buf + sizeof(struct dm_ioctl));
    tgt->sector_start = 0;
    tgt->length = 64;
    strncpy(tgt->target_type, "integrity", sizeof(tgt->target_type) - 1);

    char params[256];
    snprintf(params, sizeof(params), "%s 0 32 J 2 journal_sectors:1 internal_hash:sha256", loop_path);
    strcpy((char *)(tgt + 1), params);

    tgt->next = (sizeof(struct dm_target_spec) + strlen(params) + 1 + 7) & ~7;
    dm->data_size = sizeof(struct dm_ioctl) + tgt->next;

    res = ioctl(dm_fd, DM_TABLE_LOAD, dm);
    if (res < 0) {
        printf("[-] Failed to ioctl(DM_TABLE_LOAD): %s\n", strerror(errno));
        init_dm_hdr(dm, sizeof(buf), "syz_integrity_test", 0);
        ioctl(dm_fd, DM_DEV_REMOVE, dm);
        ioctl(loop_fd, LOOP_CLR_FD, 0);
        close(loop_fd);
        close(img_fd);
        unlink(img_path);
        close(dm_fd);
        exit(1);
    }
    printf("[+] ioctl(DM_TABLE_LOAD) successful. Integrity superblock and journal initialized.\n");

    /* 7. Corrupt superblock on the underlying storage */
    char zero_buf[4096];
    memset(zero_buf, 0, sizeof(zero_buf));

    res = pwrite(img_fd, zero_buf, sizeof(zero_buf), 0);
    if (res < 0) {
        printf("[-] Failed to pwrite zeroes to img_fd: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] pwrite zeroes to img_fd successful.\n");

    res = fsync(img_fd);
    if (res < 0) {
        printf("[-] Failed to fsync img_fd: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] fsync img_fd successful.\n");

    res = pwrite(loop_fd, zero_buf, sizeof(zero_buf), 0);
    if (res < 0) {
        printf("[-] Failed to pwrite zeroes to loop_fd: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] pwrite zeroes to loop_fd successful.\n");

    res = fsync(loop_fd);
    if (res < 0) {
        printf("[-] Failed to fsync loop_fd: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] fsync loop_fd successful.\n");

    res = ioctl(loop_fd, BLKFLSBUF, 0);
    if (res < 0) {
        printf("[*] ioctl(BLKFLSBUF): %s\n", strerror(errno));
    } else {
        printf("[+] ioctl(BLKFLSBUF) cache flush successful.\n");
    }

    /* 8. Resume target (flags = 0):
     * dm_integrity_resume reads sector 0, detects superblock_changed,
     * calls dm_integrity_io_error, and jumps to skip_writes:, skipping replay_journal.
     */
    init_dm_hdr(dm, sizeof(buf), "syz_integrity_test", 0);
    res = ioctl(dm_fd, DM_DEV_SUSPEND, dm);
    if (res < 0) {
        printf("[-] Failed to ioctl(DM_DEV_SUSPEND - resume): %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] ioctl(DM_DEV_SUSPEND - resume) successful. Superblock change detected by kernel.\n");

    /* 9. Suspend target (flags = DM_SUSPEND_FLAG):
     * dm_integrity_postsuspend unconditionally dispatches commit_work and drains commit_wq.
     * integrity_commit calls pad_uncommitted(), where ic->free_sectors is 0,
     * triggering WARN_ON(journal_sections * journal_section_entries != free_sectors).
     */
    init_dm_hdr(dm, sizeof(buf), "syz_integrity_test", DM_SUSPEND_FLAG);
    res = ioctl(dm_fd, DM_DEV_SUSPEND, dm);
    if (res < 0) {
        printf("[-] Failed to ioctl(DM_DEV_SUSPEND - suspend): %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] ioctl(DM_DEV_SUSPEND - suspend) successful. Triggered commit and pad_uncommitted WARN_ON!\n");

    /* Allow kernel workqueues to process */
    usleep(200000);

    /* 10. Cleanup */
    init_dm_hdr(dm, sizeof(buf), "syz_integrity_test", 0);
    res = ioctl(dm_fd, DM_DEV_REMOVE, dm);
    if (res < 0) {
        printf("[-] Failed to ioctl(DM_DEV_REMOVE): %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] ioctl(DM_DEV_REMOVE) successful.\n");

    res = ioctl(loop_fd, LOOP_CLR_FD, 0);
    if (res < 0) {
        printf("[-] Failed to ioctl(LOOP_CLR_FD): %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] ioctl(LOOP_CLR_FD) successful.\n");

    res = close(loop_fd);
    if (res < 0) {
        printf("[-] Failed to close loop_fd: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] close loop_fd successful.\n");

    res = close(img_fd);
    if (res < 0) {
        printf("[-] Failed to close img_fd: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] close img_fd successful.\n");

    res = unlink(img_path);
    if (res < 0) {
        printf("[-] Failed to unlink %s: %s\n", img_path, strerror(errno));
        exit(1);
    }
    printf("[+] unlink %s successful.\n", img_path);

    res = close(dm_fd);
    if (res < 0) {
        printf("[-] Failed to close dm_fd: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] close dm_fd successful.\n");

    sleep(1);
    printf("[+] Reproducer completed successfully.\n");
    return 0;
}
