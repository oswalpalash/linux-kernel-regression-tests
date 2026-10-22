// https://syzkaller.appspot.com/bug?id=65bca57b3568594bd977e17d89dd2040efbf570d
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <linux/loop.h>
#include <linux/fs.h>
#include <errno.h>

/* NILFS2 Constants */
#define NILFS_SUPER_MAGIC           0x3434
#define NILFS_SEGSUM_MAGIC          0x1eaffa11
#define NILFS_CURRENT_REV           2
#define NILFS_MINOR_REV             0
#define NILFS_ROOT_INO              2
#define NILFS_DAT_INO               3
#define NILFS_CPFILE_INO            4
#define NILFS_SUFILE_INO            5
#define NILFS_IFILE_INO             6
#define NILFS_USER_INO              11
#define NILFS_SB_OFFSET_BYTES       1024
#define NILFS_VALID_FS              0x0001
#define NILFS_SS_LOGBGN             0x0001
#define NILFS_SS_LOGEND             0x0002
#define NILFS_SS_SR                 0x0004
#define NILFS_SEGMENT_USAGE_ACTIVE  0
#define NILFS_SEGMENT_USAGE_DIRTY   1

#pragma pack(push, 1)
struct nilfs_inode {
    uint64_t i_blocks;
    uint64_t i_size;
    uint64_t i_ctime;
    uint64_t i_mtime;
    uint32_t i_ctime_nsec;
    uint32_t i_mtime_nsec;
    uint32_t i_uid;
    uint32_t i_gid;
    uint16_t i_mode;
    uint16_t i_links_count;
    uint32_t i_flags;
    uint64_t i_bmap[7];
    uint64_t i_xattr;
    uint32_t i_generation;
    uint32_t i_pad;
};

struct nilfs_super_root {
    uint32_t sr_sum;
    uint16_t sr_bytes;
    uint16_t sr_flags;
    uint64_t sr_nongc_ctime;
    struct nilfs_inode sr_dat;
    struct nilfs_inode sr_cpfile;
    struct nilfs_inode sr_sufile;
};

struct nilfs_super_block {
    uint32_t s_rev_level;
    uint16_t s_minor_rev_level;
    uint16_t s_magic;
    uint16_t s_bytes;
    uint16_t s_flags;
    uint32_t s_crc_seed;
    uint32_t s_sum;
    uint32_t s_log_block_size;
    uint64_t s_nsegments;
    uint64_t s_dev_size;
    uint64_t s_first_data_block;
    uint32_t s_blocks_per_segment;
    uint32_t s_r_segments_percentage;
    uint64_t s_last_cno;
    uint64_t s_last_pseg;
    uint64_t s_last_seq;
    uint64_t s_free_blocks_count;
    uint64_t s_ctime;
    uint64_t s_mtime;
    uint64_t s_wtime;
    uint16_t s_mnt_count;
    uint16_t s_max_mnt_count;
    uint16_t s_state;
    uint16_t s_errors;
    uint64_t s_lastcheck;
    uint32_t s_checkinterval;
    uint32_t s_creator_os;
    uint16_t s_def_resuid;
    uint16_t s_def_resgid;
    uint32_t s_first_ino;
    uint16_t s_inode_size;
    uint16_t s_dat_entry_size;
    uint16_t s_checkpoint_size;
    uint16_t s_segment_usage_size;
    uint8_t  s_uuid[16];
    char     s_volume_name[80];
    uint32_t s_c_interval;
    uint32_t s_c_block_max;
    uint64_t s_feature_compat;
    uint64_t s_feature_compat_ro;
    uint64_t s_feature_incompat;
    uint32_t s_reserved[186];
};

struct nilfs_segment_summary {
    uint32_t ss_datasum;
    uint32_t ss_sumsum;
    uint32_t ss_magic;
    uint16_t ss_bytes;
    uint16_t ss_flags;
    uint64_t ss_seq;
    uint64_t ss_create;
    uint64_t ss_next;
    uint32_t ss_nblocks;
    uint32_t ss_nfinfo;
    uint32_t ss_sumbytes;
    uint32_t ss_pad;
    uint64_t ss_cno;
};

struct nilfs_snapshot_list {
    uint64_t ssl_next;
    uint64_t ssl_prev;
};

struct nilfs_checkpoint {
    uint32_t cp_flags;
    uint32_t cp_checkpoints_count;
    struct nilfs_snapshot_list cp_snapshot_list;
    uint64_t cp_cno;
    uint64_t cp_create;
    uint64_t cp_nblk_inc;
    uint64_t cp_inodes_count;
    uint64_t cp_blocks_count;
    struct nilfs_inode cp_ifile_inode;
};

struct nilfs_cpfile_header {
    uint64_t ch_ncheckpoints;
    uint64_t ch_nsnapshots;
    struct nilfs_snapshot_list ch_snapshot_list;
};

struct nilfs_segment_usage {
    uint64_t su_lastmod;
    uint32_t su_nblocks;
    uint32_t su_flags;
};

struct nilfs_sufile_header {
    uint64_t sh_ncleansegs;
    uint64_t sh_ndirtysegs;
    uint64_t sh_last_alloc;
};

struct nilfs_dat_entry {
    uint64_t de_blocknr;
    uint64_t de_start;
    uint64_t de_end;
    uint64_t de_rsv;
};

struct nilfs_dir_entry {
    uint64_t inode;
    uint16_t rec_len;
    uint8_t  name_len;
    uint8_t  file_type;
    char     name[255];
    char     pad;
};
#pragma pack(pop)

uint32_t crc32_le(uint32_t crc, const unsigned char *p, size_t len) {
    while (len--) {
        crc ^= *p++;
        for (int i = 0; i < 8; i++)
            crc = (crc >> 1) ^ ((crc & 1) ? 0xedb88320U : 0);
    }
    return crc;
}

#define BLOCK_SIZE 4096
#define BLOCKS_PER_SEGMENT 64
#define TOTAL_SEGMENTS 16
#define DEVICE_SIZE ((uint64_t)TOTAL_SEGMENTS * BLOCKS_PER_SEGMENT * BLOCK_SIZE)

void generate_nilfs2_img(int fd) {
    uint8_t *img = (uint8_t *)calloc(TOTAL_SEGMENTS * BLOCKS_PER_SEGMENT, BLOCK_SIZE);
    if (!img) {
        printf("[-] Failed to allocate memory for image\n");
        exit(1);
    }

    uint32_t crc_seed = 0x1e38a05b;
    uint64_t ctime_val = 1700000000ULL;
    uint64_t pseg_start = 1;
    uint32_t pseg_nblocks = 9;

    // 1. Superblock
    struct nilfs_super_block *sb = (struct nilfs_super_block *)(img + NILFS_SB_OFFSET_BYTES);
    sb->s_rev_level = NILFS_CURRENT_REV;
    sb->s_magic = NILFS_SUPER_MAGIC;
    sb->s_bytes = offsetof(struct nilfs_super_block, s_reserved);
    sb->s_crc_seed = crc_seed;
    sb->s_log_block_size = 2;
    sb->s_nsegments = TOTAL_SEGMENTS;
    sb->s_dev_size = DEVICE_SIZE;
    sb->s_blocks_per_segment = BLOCKS_PER_SEGMENT;
    sb->s_r_segments_percentage = 5;
    sb->s_last_cno = 1;
    sb->s_last_pseg = pseg_start;
    sb->s_last_seq = 1;
    sb->s_free_blocks_count = (TOTAL_SEGMENTS * BLOCKS_PER_SEGMENT) - 10;
    sb->s_ctime = ctime_val;
    sb->s_wtime = ctime_val;
    sb->s_state = NILFS_VALID_FS;
    sb->s_first_ino = NILFS_USER_INO;
    sb->s_inode_size = sizeof(struct nilfs_inode);
    sb->s_dat_entry_size = sizeof(struct nilfs_dat_entry);
    sb->s_checkpoint_size = sizeof(struct nilfs_checkpoint);
    sb->s_segment_usage_size = sizeof(struct nilfs_segment_usage);

    uint32_t sumoff = offsetof(struct nilfs_super_block, s_sum);
    uint8_t zero_sum[4] = {0};
    uint32_t crc = crc32_le(crc_seed, (const unsigned char *)sb, sumoff);
    crc = crc32_le(crc, zero_sum, 4);
    crc = crc32_le(crc, (const unsigned char *)sb + sumoff + 4, sb->s_bytes - sumoff - 4);
    sb->s_sum = crc;

    // 2. Segment Summary (Block 1)
    struct nilfs_segment_summary *ss = (struct nilfs_segment_summary *)(img + pseg_start * BLOCK_SIZE);
    ss->ss_magic = NILFS_SEGSUM_MAGIC;
    ss->ss_bytes = sizeof(struct nilfs_segment_summary);
    ss->ss_flags = NILFS_SS_LOGBGN | NILFS_SS_LOGEND | NILFS_SS_SR;
    ss->ss_seq = 1;
    ss->ss_create = ctime_val;
    ss->ss_next = BLOCKS_PER_SEGMENT;
    ss->ss_nblocks = pseg_nblocks;
    ss->ss_sumbytes = BLOCK_SIZE;
    ss->ss_cno = 1;

    // 3. DAT Entry Block (Block 2)
    uint8_t *dat_block = img + 2 * BLOCK_SIZE;
    ((struct nilfs_dat_entry *)(dat_block + 1 * 32))->de_blocknr = 3; // cpfile data
    ((struct nilfs_dat_entry *)(dat_block + 2 * 32))->de_blocknr = 4; // ifile entry
    ((struct nilfs_dat_entry *)(dat_block + 3 * 32))->de_blocknr = 5; // root dir data
    ((struct nilfs_dat_entry *)(dat_block + 4 * 32))->de_blocknr = 6; // test dir data
    ((struct nilfs_dat_entry *)(dat_block + 5 * 32))->de_blocknr = 7; // sufile header
    ((struct nilfs_dat_entry *)(dat_block + 6 * 32))->de_blocknr = 8; // sufile entry

    // 4. cpfile Data Block (Block 3)
    uint8_t *cp_block = img + 3 * BLOCK_SIZE;
    ((struct nilfs_cpfile_header *)cp_block)->ch_ncheckpoints = 1;
    struct nilfs_checkpoint *cp = (struct nilfs_checkpoint *)(cp_block + 192);
    cp->cp_checkpoints_count = 1;
    cp->cp_cno = 1;
    cp->cp_inodes_count = 2;
    cp->cp_blocks_count = pseg_nblocks;
    struct nilfs_inode *ii = &cp->cp_ifile_inode;
    ii->i_blocks = 1;
    ii->i_size = BLOCK_SIZE * 3;
    ii->i_mode = 0100644;
    ii->i_links_count = 1;
    ii->i_bmap[3] = 2; // vblocknr 2 -> physical block 4

    // 5. ifile Entry Block (Block 4)
    uint8_t *ifile_block = img + 4 * BLOCK_SIZE;
    struct nilfs_inode *root_ino = (struct nilfs_inode *)(ifile_block + NILFS_ROOT_INO * 128);
    root_ino->i_blocks = 1;
    root_ino->i_size = BLOCK_SIZE;
    root_ino->i_mode = 0040755;
    root_ino->i_links_count = 3;
    root_ino->i_bmap[1] = 3; // vblocknr 3 -> physical block 5

    struct nilfs_inode *testdir_ino = (struct nilfs_inode *)(ifile_block + NILFS_USER_INO * 128);
    testdir_ino->i_blocks = 1;
    testdir_ino->i_size = BLOCK_SIZE;
    testdir_ino->i_mode = 0040755;
    testdir_ino->i_links_count = 1; // CORRUPTED LINK COUNT: 1 instead of 2
    testdir_ino->i_bmap[1] = 4; // vblocknr 4 -> physical block 6

    // 6. Root Dir Data (Block 5)
    uint8_t *dir_block = img + 5 * BLOCK_SIZE;
    struct nilfs_dir_entry *de_dot = (struct nilfs_dir_entry *)dir_block;
    de_dot->inode = NILFS_ROOT_INO; de_dot->rec_len = 16; de_dot->name_len = 1; de_dot->file_type = 2; memcpy(de_dot->name, ".", 1);
    
    struct nilfs_dir_entry *de_dotdot = (struct nilfs_dir_entry *)(dir_block + 16);
    de_dotdot->inode = NILFS_ROOT_INO; de_dotdot->rec_len = 16; de_dotdot->name_len = 2; de_dotdot->file_type = 2; memcpy(de_dotdot->name, "..", 2);
    
    struct nilfs_dir_entry *de_testdir = (struct nilfs_dir_entry *)(dir_block + 32);
    de_testdir->inode = NILFS_USER_INO; de_testdir->rec_len = BLOCK_SIZE - 32; de_testdir->name_len = 7; de_testdir->file_type = 2; memcpy(de_testdir->name, "testdir", 7);

    // 7. Test Dir Data (Block 6)
    uint8_t *testdir_block = img + 6 * BLOCK_SIZE;
    struct nilfs_dir_entry *td_dot = (struct nilfs_dir_entry *)testdir_block;
    td_dot->inode = NILFS_USER_INO; td_dot->rec_len = 16; td_dot->name_len = 1; td_dot->file_type = 2; memcpy(td_dot->name, ".", 1);
    
    struct nilfs_dir_entry *td_dotdot = (struct nilfs_dir_entry *)(testdir_block + 16);
    td_dotdot->inode = NILFS_ROOT_INO; td_dotdot->rec_len = BLOCK_SIZE - 16; td_dotdot->name_len = 2; td_dotdot->file_type = 2; memcpy(td_dotdot->name, "..", 2);

    // 8. sufile Header (Block 7)
    uint8_t *suh_block = img + 7 * BLOCK_SIZE;
    struct nilfs_sufile_header *suh = (struct nilfs_sufile_header *)suh_block;
    suh->sh_ncleansegs = TOTAL_SEGMENTS - 1;
    suh->sh_ndirtysegs = 1;

    // 9. sufile Entry (Block 8)
    uint8_t *sue_block = img + 8 * BLOCK_SIZE;
    struct nilfs_segment_usage *su = (struct nilfs_segment_usage *)(sue_block + 32);
    su->su_nblocks = pseg_nblocks + 1;
    su->su_flags = (1 << NILFS_SEGMENT_USAGE_DIRTY) | (1 << NILFS_SEGMENT_USAGE_ACTIVE);

    // 10. Super Root (Block 9)
    struct nilfs_super_root *sr = (struct nilfs_super_root *)(img + 9 * BLOCK_SIZE);
    sr->sr_bytes = sizeof(struct nilfs_super_root);
    sr->sr_dat.i_blocks = 1; sr->sr_dat.i_size = BLOCK_SIZE * 3; sr->sr_dat.i_mode = 0100644; sr->sr_dat.i_links_count = 1; sr->sr_dat.i_bmap[3] = 2;
    sr->sr_cpfile.i_blocks = 1; sr->sr_cpfile.i_size = BLOCK_SIZE; sr->sr_cpfile.i_mode = 0100644; sr->sr_cpfile.i_links_count = 1; sr->sr_cpfile.i_bmap[1] = 1;
    sr->sr_sufile.i_blocks = 2; sr->sr_sufile.i_size = BLOCK_SIZE * 3; sr->sr_sufile.i_mode = 0100644; sr->sr_sufile.i_links_count = 1; sr->sr_sufile.i_bmap[1] = 5; sr->sr_sufile.i_bmap[3] = 6;

    sr->sr_sum = crc32_le(crc_seed, (const unsigned char *)sr + 4, sr->sr_bytes - 4);
    ss->ss_datasum = crc32_le(crc_seed, img + pseg_start * BLOCK_SIZE + 4, pseg_nblocks * BLOCK_SIZE - 4);

    ssize_t written = 0;
    while (written < DEVICE_SIZE) {
        ssize_t res = write(fd, img + written, DEVICE_SIZE - written);
        if (res < 0) {
            if (errno == EINTR) continue;
            printf("[-] Failed to write img: %s\n", strerror(errno));
            exit(1);
        }
        written += res;
    }
    free(img);
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);

    int fd = open("nilfs2.img", O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
        printf("[-] Failed to open nilfs2.img: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] open nilfs2.img successful.\n");

    int res_ftruncate = ftruncate(fd, DEVICE_SIZE);
    if (res_ftruncate < 0) {
        printf("[-] Failed to ftruncate: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] ftruncate successful.\n");

    int loop_ctl = open("/dev/loop-control", O_RDWR);
    if (loop_ctl < 0) {
        printf("[-] Failed to open /dev/loop-control: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] open /dev/loop-control successful.\n");

    int loop_num = ioctl(loop_ctl, LOOP_CTL_GET_FREE);
    close(loop_ctl);
    if (loop_num < 0) {
        printf("[-] Failed to LOOP_CTL_GET_FREE: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] LOOP_CTL_GET_FREE successful.\n");

    char loop_name[64];
    sprintf(loop_name, "/dev/loop%d", loop_num);
    int loop_fd = open(loop_name, O_RDWR);
    if (loop_fd < 0) {
        printf("[-] Failed to open loop device: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] open loop device successful.\n");

    int res_loop_set_fd = ioctl(loop_fd, LOOP_SET_FD, fd);
    if (res_loop_set_fd < 0) {
        printf("[-] Failed to LOOP_SET_FD: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] LOOP_SET_FD successful.\n");

    close(fd);
    close(loop_fd);

    // Wait for udevd to probe the zero-filled device and exit immediately
    usleep(250000); // 250ms

    // Now write the actual nilfs2 image data directly to the loop device
    loop_fd = open(loop_name, O_RDWR);
    if (loop_fd < 0) {
        printf("[-] Failed to open loop device for writing: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] open loop device for writing successful.\n");

    generate_nilfs2_img(loop_fd);
    
    int res_fsync = fsync(loop_fd);
    if (res_fsync < 0) {
        printf("[-] Failed to fsync: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] fsync successful.\n");

    close(loop_fd);

    int res_mkdir = mkdir("/mnt/nilfs", 0777);
    if (res_mkdir < 0 && errno != EEXIST) {
        printf("[-] Failed to mkdir /mnt/nilfs: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] mkdir successful.\n");
    
    int res_mount = mount(loop_name, "/mnt/nilfs", "nilfs2", 0, NULL);
    if (res_mount < 0) {
        printf("[-] Failed to mount: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] mount successful.\n");

    // A single rmdir on the corrupted directory (i_links_count == 1) will trigger the bug.
    // nilfs_unlink drops it to 0, then nilfs_rmdir drops it to -1, triggering the warning.
    int res_rmdir = rmdir("/mnt/nilfs/testdir");
    if (res_rmdir < 0) {
        printf("[+] rmdir returned %d (errno: %d - %s)\n", res_rmdir, errno, strerror(errno));
    } else {
        printf("[+] rmdir succeeded unexpectedly.\n");
    }

    sleep(2);

    int res_umount = umount("/mnt/nilfs");
    if (res_umount < 0) {
        printf("[-] Failed to umount: %s\n", strerror(errno));
    } else {
        printf("[+] umount successful.\n");
    }

    loop_fd = open(loop_name, O_RDWR);
    if (loop_fd >= 0) {
        int res_clr = ioctl(loop_fd, LOOP_CLR_FD, 0);
        if (res_clr < 0) {
            printf("[-] Failed to LOOP_CLR_FD: %s\n", strerror(errno));
        } else {
            printf("[+] LOOP_CLR_FD successful.\n");
        }
        close(loop_fd);
    }

    return 0;
}