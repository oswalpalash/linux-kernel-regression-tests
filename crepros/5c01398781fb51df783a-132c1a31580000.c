// https://syzkaller.appspot.com/bug?id=3d86ea603e5b33ea46fb8d8b41688f87d54546ba
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/mman.h>
#include <linux/loop.h>
#include <linux/fs.h>

#ifndef MS_STRICTATIME
#define MS_STRICTATIME (1 << 24)
#endif

#ifndef BLKBSZSET
#define BLKBSZSET _IOW(0x12, 113, size_t)
#endif

#define BTRFS_MAGIC 0x4D5F53665248425FULL
#define BTRFS_SUPER_INFO_OFFSET 0x10000

#define BTRFS_ROOT_TREE_OBJECTID 1ULL
#define BTRFS_EXTENT_TREE_OBJECTID 2ULL
#define BTRFS_CHUNK_TREE_OBJECTID 3ULL
#define BTRFS_DEV_TREE_OBJECTID 4ULL
#define BTRFS_FS_TREE_OBJECTID 5ULL
#define BTRFS_CSUM_TREE_OBJECTID 7ULL
#define BTRFS_UUID_TREE_OBJECTID 9ULL
#define BTRFS_DATA_RELOC_TREE_OBJECTID ((uint64_t)-9ULL)

#define BTRFS_INODE_ITEM_KEY 1
#define BTRFS_INODE_REF_KEY 12
#define BTRFS_ROOT_ITEM_KEY 132
#define BTRFS_BLOCK_GROUP_ITEM_KEY 192
#define BTRFS_DEV_EXTENT_KEY 204
#define BTRFS_DEV_ITEM_KEY 216
#define BTRFS_CHUNK_ITEM_KEY 228

#define BTRFS_BLOCK_GROUP_DATA (1ULL << 0)
#define BTRFS_BLOCK_GROUP_SYSTEM (1ULL << 1)
#define BTRFS_BLOCK_GROUP_METADATA (1ULL << 2)

#define BTRFS_FEATURE_INCOMPAT_MIXED_BACKREF (1ULL << 0)
#define BTRFS_FEATURE_INCOMPAT_MIXED_GROUPS (1ULL << 2)

#define BTRFS_IOCTL_MAGIC 0x94
#define BTRFS_PATH_NAME_MAX 4087

struct btrfs_ioctl_vol_args {
    int64_t fd;
    char name[BTRFS_PATH_NAME_MAX + 1];
};
#define BTRFS_IOC_ADD_DEV _IOW(BTRFS_IOCTL_MAGIC, 10, struct btrfs_ioctl_vol_args)

struct btrfs_disk_key {
    uint64_t objectid;
    uint8_t type;
    uint64_t offset;
} __attribute__((__packed__));

struct btrfs_header {
    uint8_t csum[32];
    uint8_t fsid[16];
    uint64_t bytenr;
    uint64_t flags;
    uint8_t chunk_tree_uuid[16];
    uint64_t generation;
    uint64_t owner;
    uint32_t nritems;
    uint8_t level;
} __attribute__((__packed__));

struct btrfs_item {
    struct btrfs_disk_key key;
    uint32_t offset;
    uint32_t size;
} __attribute__((__packed__));

struct btrfs_stripe {
    uint64_t devid;
    uint64_t offset;
    uint8_t dev_uuid[16];
} __attribute__((__packed__));

struct btrfs_chunk {
    uint64_t length;
    uint64_t owner;
    uint64_t stripe_len;
    uint64_t type;
    uint32_t io_align;
    uint32_t io_width;
    uint32_t sector_size;
    uint16_t num_stripes;
    uint16_t sub_stripes;
    struct btrfs_stripe stripe;
} __attribute__((__packed__));

struct btrfs_dev_item {
    uint64_t devid;
    uint64_t total_bytes;
    uint64_t bytes_used;
    uint32_t io_align;
    uint32_t io_width;
    uint32_t sector_size;
    uint64_t type;
    uint64_t generation;
    uint64_t start_offset;
    uint32_t dev_group;
    uint8_t seek_speed;
    uint8_t bandwidth;
    uint8_t uuid[16];
    uint8_t fsid[16];
} __attribute__((__packed__));

struct btrfs_block_group_item {
    uint64_t used;
    uint64_t chunk_objectid;
    uint64_t flags;
} __attribute__((__packed__));

struct btrfs_dev_extent {
    uint64_t chunk_tree;
    uint64_t chunk_objectid;
    uint64_t chunk_offset;
    uint64_t length;
    uint8_t chunk_tree_uuid[16];
} __attribute__((__packed__));

struct btrfs_timespec {
    uint64_t sec;
    uint32_t nsec;
} __attribute__((__packed__));

struct btrfs_inode_item {
    uint64_t generation;
    uint64_t transid;
    uint64_t size;
    uint64_t nbytes;
    uint64_t block_group;
    uint32_t nlink;
    uint32_t uid;
    uint32_t gid;
    uint32_t mode;
    uint64_t rdev;
    uint64_t flags;
    uint64_t sequence;
    uint64_t reserved[4];
    struct btrfs_timespec atime;
    struct btrfs_timespec ctime;
    struct btrfs_timespec mtime;
    struct btrfs_timespec otime;
} __attribute__((__packed__));

struct btrfs_inode_ref {
    uint64_t index;
    uint16_t name_len;
} __attribute__((__packed__));

struct btrfs_root_item {
    struct btrfs_inode_item inode;
    uint64_t generation;
    uint64_t root_dirid;
    uint64_t bytenr;
    uint64_t byte_limit;
    uint64_t bytes_used;
    uint64_t last_snapshot;
    uint64_t flags;
    uint32_t refs;
    struct btrfs_disk_key drop_progress;
    uint8_t drop_level;
    uint8_t level;
    uint64_t generation_v2;
    uint8_t uuid[16];
    uint8_t parent_uuid[16];
    uint8_t received_uuid[16];
    uint64_t ctransid;
    uint64_t otransid;
    uint64_t stransid;
    uint64_t rtransid;
    struct btrfs_timespec ctime;
    struct btrfs_timespec otime;
    struct btrfs_timespec stime;
    struct btrfs_timespec rtime;
    uint64_t reserved[8];
} __attribute__((__packed__));

struct btrfs_super_block {
    uint8_t csum[32];
    uint8_t fsid[16];
    uint64_t bytenr;
    uint64_t flags;
    uint64_t magic;
    uint64_t generation;
    uint64_t root;
    uint64_t chunk_root;
    uint64_t log_root;
    uint64_t __unused_log_root_transid;
    uint64_t total_bytes;
    uint64_t bytes_used;
    uint64_t root_dir_objectid;
    uint64_t num_devices;
    uint32_t sectorsize;
    uint32_t nodesize;
    uint32_t __unused_leafsize;
    uint32_t stripesize;
    uint32_t sys_chunk_array_size;
    uint64_t chunk_root_generation;
    uint64_t compat_flags;
    uint64_t compat_ro_flags;
    uint64_t incompat_flags;
    uint16_t csum_type;
    uint8_t root_level;
    uint8_t chunk_root_level;
    uint8_t log_root_level;
    struct btrfs_dev_item dev_item;
    char label[256];
    uint64_t cache_generation;
    uint64_t uuid_tree_generation;
    uint8_t metadata_uuid[16];
    uint64_t nr_global_roots;
    uint64_t remap_root;
    uint64_t remap_root_generation;
    uint8_t remap_root_level;
    uint8_t reserved[199];
    uint8_t sys_chunk_array[2048];
    uint8_t super_roots[672];
    uint8_t padding[565];
} __attribute__((__packed__));

static uint32_t crc32c(uint32_t crc, const void *buf, size_t len) {
    const uint8_t *p = (const uint8_t *)buf;
    while (len--) {
        crc ^= *p++;
        for (int i = 0; i < 8; i++)
            crc = (crc >> 1) ^ ((crc & 1) ? 0x82F63B78 : 0);
    }
    return crc;
}

struct leaf_item_desc {
    struct btrfs_disk_key key;
    const void *data;
    uint32_t size;
};

static void make_leaf(uint8_t *leaf, uint64_t bytenr, uint64_t owner,
                      const struct leaf_item_desc *items, uint32_t nritems,
                      const uint8_t *fsid) {
    memset(leaf, 0, 4096);
    struct btrfs_header *hdr = (struct btrfs_header *)leaf;
    memcpy(hdr->fsid, fsid, 16);
    hdr->bytenr = bytenr;
    hdr->flags = 1; /* BTRFS_HEADER_FLAG_WRITTEN */
    hdr->generation = 1;
    hdr->owner = owner;
    hdr->nritems = nritems;
    hdr->level = 0;

    uint32_t cur_offset = 4096 - sizeof(struct btrfs_header);
    struct btrfs_item *it = (struct btrfs_item *)(leaf + sizeof(struct btrfs_header));

    for (uint32_t i = 0; i < nritems; i++) {
        cur_offset -= items[i].size;
        it[i].key = items[i].key;
        it[i].offset = cur_offset;
        it[i].size = items[i].size;
        memcpy(leaf + sizeof(struct btrfs_header) + cur_offset,
               items[i].data, items[i].size);
    }

    uint32_t csum = crc32c(~0U, leaf + 32, 4096 - 32) ^ ~0U;
    memcpy(hdr->csum, &csum, 4);
}

static void init_root_item(struct btrfs_root_item *ri, uint64_t bytenr, uint64_t dirid) {
    memset(ri, 0, sizeof(*ri));
    ri->inode.generation = 1;
    ri->inode.transid = 1;
    ri->inode.nlink = 1;
    ri->inode.mode = 040755;
    ri->generation = 1;
    ri->root_dirid = dirid;
    ri->bytenr = bytenr;
    ri->bytes_used = 4096;
    ri->refs = 1;
    ri->level = 0;
    ri->generation_v2 = 1;
}

static int setup_loop_dev(const char *img_path, char *loop_path, size_t loop_path_sz, int bsz) {
    int ctl_fd = open("/dev/loop-control", O_RDWR);
    if (ctl_fd < 0) {
        printf("[-] Failed to open /dev/loop-control: %s\n", strerror(errno));
        return -1;
    }
    int dev_nr = ioctl(ctl_fd, LOOP_CTL_GET_FREE);
    close(ctl_fd);
    if (dev_nr < 0) {
        printf("[-] Failed ioctl(LOOP_CTL_GET_FREE): %s\n", strerror(errno));
        return -1;
    }

    snprintf(loop_path, loop_path_sz, "/dev/loop%d", dev_nr);
    if (access(loop_path, F_OK) != 0) {
        mknod(loop_path, S_IFBLK | 0660, makedev(7, dev_nr));
    }

    int img_fd = open(img_path, O_RDWR);
    if (img_fd < 0) {
        printf("[-] Failed to open %s: %s\n", img_path, strerror(errno));
        return -1;
    }

    int loop_fd = open(loop_path, O_RDWR);
    if (loop_fd < 0) {
        printf("[-] Failed to open %s: %s\n", loop_path, strerror(errno));
        close(img_fd);
        return -1;
    }

    if (ioctl(loop_fd, LOOP_SET_FD, img_fd) < 0) {
        printf("[-] Failed ioctl(LOOP_SET_FD) on %s: %s\n", loop_path, strerror(errno));
        close(loop_fd);
        close(img_fd);
        return -1;
    }

    if (bsz > 0) {
        if (ioctl(loop_fd, BLKBSZSET, &bsz) < 0) {
            printf("[-] Failed ioctl(BLKBSZSET) on %s: %s\n", loop_path, strerror(errno));
            close(loop_fd);
            close(img_fd);
            return -1;
        }
        printf("[+] Set block size of %s to %d.\n", loop_path, bsz);
    }

    /* Close descriptors so subsequent exclusive block layer opens (BLK_OPEN_EXCL) succeed */
    close(loop_fd);
    close(img_fd);
    return 0;
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("[*] Starting BTRFS deadlock reproducer...\n");

    const size_t img_size = 16 * 1024 * 1024; /* 16 MiB */
    uint8_t *img = calloc(1, img_size);
    if (!img) {
        printf("[-] Failed to allocate image buffer: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] Allocated 16 MiB image buffer.\n");

    uint8_t fsid[16] = {
        0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42,
        0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42
    };
    uint8_t dev_uuid[16] = {
        0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24,
        0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24
    };

    /* 1. Device item */
    struct btrfs_dev_item dev_item;
    memset(&dev_item, 0, sizeof(dev_item));
    dev_item.devid = 1;
    dev_item.total_bytes = img_size;
    dev_item.bytes_used = 32768;
    dev_item.io_align = 4096;
    dev_item.io_width = 4096;
    dev_item.sector_size = 4096;
    dev_item.generation = 1;
    memcpy(dev_item.uuid, dev_uuid, 16);
    memcpy(dev_item.fsid, fsid, 16);

    /* 2. System Chunk */
    struct btrfs_chunk sys_chunk;
    memset(&sys_chunk, 0, sizeof(sys_chunk));
    sys_chunk.length = 0x400000; /* 4 MiB */
    sys_chunk.owner = 2;
    sys_chunk.stripe_len = 65536;
    sys_chunk.type = BTRFS_BLOCK_GROUP_SYSTEM;
    sys_chunk.io_align = 4096;
    sys_chunk.io_width = 4096;
    sys_chunk.sector_size = 4096;
    sys_chunk.num_stripes = 1;
    sys_chunk.sub_stripes = 0;
    sys_chunk.stripe.devid = 1;
    sys_chunk.stripe.offset = 0x100000;
    memcpy(sys_chunk.stripe.dev_uuid, dev_uuid, 16);

    /* 3. Mixed (Data + Metadata) Chunk */
    struct btrfs_chunk mixed_chunk;
    memset(&mixed_chunk, 0, sizeof(mixed_chunk));
    mixed_chunk.length = 0x800000; /* 8 MiB */
    mixed_chunk.owner = 2;
    mixed_chunk.stripe_len = 65536;
    mixed_chunk.type = BTRFS_BLOCK_GROUP_DATA | BTRFS_BLOCK_GROUP_METADATA;
    mixed_chunk.io_align = 4096;
    mixed_chunk.io_width = 4096;
    mixed_chunk.sector_size = 4096;
    mixed_chunk.num_stripes = 1;
    mixed_chunk.sub_stripes = 0;
    mixed_chunk.stripe.devid = 1;
    mixed_chunk.stripe.offset = 0x500000;
    memcpy(mixed_chunk.stripe.dev_uuid, dev_uuid, 16);

    /* 4. Chunk Leaf (bytenr 0x100000, owner 3) */
    struct leaf_item_desc chunk_items[3] = {
        { { 1, BTRFS_DEV_ITEM_KEY, 1 }, &dev_item, sizeof(dev_item) },
        { { 256, BTRFS_CHUNK_ITEM_KEY, 0x100000 }, &sys_chunk, sizeof(sys_chunk) },
        { { 256, BTRFS_CHUNK_ITEM_KEY, 0x500000 }, &mixed_chunk, sizeof(mixed_chunk) }
    };
    make_leaf(img + 0x100000, 0x100000, BTRFS_CHUNK_TREE_OBJECTID, chunk_items, 3, fsid);

    /* 5. Tree Root Leaf (bytenr 0x500000, owner 1) */
    struct btrfs_root_item ri_extent, ri_dev, ri_fs, ri_csum, ri_uuid, ri_reloc;
    init_root_item(&ri_extent, 0x501000, 6);
    init_root_item(&ri_dev, 0x502000, 6);
    init_root_item(&ri_fs, 0x503000, 256);
    init_root_item(&ri_csum, 0x504000, 6);
    init_root_item(&ri_uuid, 0x506000, 6);
    init_root_item(&ri_reloc, 0x505000, 256);

    struct leaf_item_desc root_items[6] = {
        { { BTRFS_EXTENT_TREE_OBJECTID, BTRFS_ROOT_ITEM_KEY, 0 }, &ri_extent, sizeof(ri_extent) },
        { { BTRFS_DEV_TREE_OBJECTID, BTRFS_ROOT_ITEM_KEY, 0 }, &ri_dev, sizeof(ri_dev) },
        { { BTRFS_FS_TREE_OBJECTID, BTRFS_ROOT_ITEM_KEY, 0 }, &ri_fs, sizeof(ri_fs) },
        { { BTRFS_CSUM_TREE_OBJECTID, BTRFS_ROOT_ITEM_KEY, 0 }, &ri_csum, sizeof(ri_csum) },
        { { BTRFS_UUID_TREE_OBJECTID, BTRFS_ROOT_ITEM_KEY, 0 }, &ri_uuid, sizeof(ri_uuid) },
        { { BTRFS_DATA_RELOC_TREE_OBJECTID, BTRFS_ROOT_ITEM_KEY, 0 }, &ri_reloc, sizeof(ri_reloc) }
    };
    make_leaf(img + 0x500000, 0x500000, BTRFS_ROOT_TREE_OBJECTID, root_items, 6, fsid);

    /* 6. Extent Leaf (bytenr 0x501000, owner 2) */
    struct btrfs_block_group_item bg_sys = {
        .used = 4096,
        .chunk_objectid = 256,
        .flags = BTRFS_BLOCK_GROUP_SYSTEM
    };
    struct btrfs_block_group_item bg_mixed = {
        .used = 28672,
        .chunk_objectid = 256,
        .flags = BTRFS_BLOCK_GROUP_DATA | BTRFS_BLOCK_GROUP_METADATA
    };
    struct leaf_item_desc extent_items[2] = {
        { { 0x100000, BTRFS_BLOCK_GROUP_ITEM_KEY, 0x400000 }, &bg_sys, sizeof(bg_sys) },
        { { 0x500000, BTRFS_BLOCK_GROUP_ITEM_KEY, 0x800000 }, &bg_mixed, sizeof(bg_mixed) }
    };
    make_leaf(img + 0x501000, 0x501000, BTRFS_EXTENT_TREE_OBJECTID, extent_items, 2, fsid);

    /* 7. Dev Leaf (bytenr 0x502000, owner 4) */
    struct btrfs_dev_extent de_sys = {
        .chunk_tree = BTRFS_CHUNK_TREE_OBJECTID,
        .chunk_objectid = 256,
        .chunk_offset = 0x100000,
        .length = 0x400000
    };
    struct btrfs_dev_extent de_mixed = {
        .chunk_tree = BTRFS_CHUNK_TREE_OBJECTID,
        .chunk_objectid = 256,
        .chunk_offset = 0x500000,
        .length = 0x800000
    };
    struct leaf_item_desc dev_items[2] = {
        { { 1, BTRFS_DEV_EXTENT_KEY, 0x100000 }, &de_sys, sizeof(de_sys) },
        { { 1, BTRFS_DEV_EXTENT_KEY, 0x500000 }, &de_mixed, sizeof(de_mixed) }
    };
    make_leaf(img + 0x502000, 0x502000, BTRFS_DEV_TREE_OBJECTID, dev_items, 2, fsid);

    /* 8. FS Leaf (bytenr 0x503000, owner 5) */
    struct btrfs_inode_item root_dir_inode;
    memset(&root_dir_inode, 0, sizeof(root_dir_inode));
    root_dir_inode.generation = 1;
    root_dir_inode.transid = 1;
    root_dir_inode.nlink = 1;
    root_dir_inode.mode = 040755;
    root_dir_inode.atime.sec = 1;
    root_dir_inode.ctime.sec = 1;
    root_dir_inode.mtime.sec = 1;
    root_dir_inode.otime.sec = 1;

    uint8_t iref_buf[12];
    memset(iref_buf, 0, sizeof(iref_buf));
    struct btrfs_inode_ref *iref = (struct btrfs_inode_ref *)iref_buf;
    iref->index = 0;
    iref->name_len = 2;
    memcpy(iref_buf + sizeof(struct btrfs_inode_ref), "..", 2);

    struct leaf_item_desc fs_items[2] = {
        { { 256, BTRFS_INODE_ITEM_KEY, 0 }, &root_dir_inode, sizeof(root_dir_inode) },
        { { 256, BTRFS_INODE_REF_KEY, 256 }, iref_buf, sizeof(iref_buf) }
    };
    make_leaf(img + 0x503000, 0x503000, BTRFS_FS_TREE_OBJECTID, fs_items, 2, fsid);

    /* 9. CSUM Leaf (bytenr 0x504000, owner 7) */
    make_leaf(img + 0x504000, 0x504000, BTRFS_CSUM_TREE_OBJECTID, NULL, 0, fsid);

    /* 10. Data Reloc Leaf (bytenr 0x505000, owner -9ULL) */
    make_leaf(img + 0x505000, 0x505000, BTRFS_DATA_RELOC_TREE_OBJECTID, fs_items, 2, fsid);

    /* 11. UUID Leaf (bytenr 0x506000, owner 9) */
    make_leaf(img + 0x506000, 0x506000, BTRFS_UUID_TREE_OBJECTID, NULL, 0, fsid);

    /* 12. Superblock (bytenr 0x10000) */
    struct btrfs_super_block *sb = (struct btrfs_super_block *)(img + 0x10000);
    memcpy(sb->fsid, fsid, 16);
    sb->bytenr = 0x10000;
    sb->magic = BTRFS_MAGIC;
    sb->generation = 1;
    sb->root = 0x500000;
    sb->chunk_root = 0x100000;
    sb->total_bytes = img_size;
    sb->bytes_used = 32768;
    sb->root_dir_objectid = 6;
    sb->num_devices = 1;
    sb->sectorsize = 4096;
    sb->nodesize = 4096;
    sb->__unused_leafsize = 4096;
    sb->stripesize = 4096;
    sb->chunk_root_generation = 1;
    sb->incompat_flags = BTRFS_FEATURE_INCOMPAT_MIXED_BACKREF | BTRFS_FEATURE_INCOMPAT_MIXED_GROUPS;
    sb->csum_type = 0; /* CRC32C */
    sb->root_level = 0;
    sb->chunk_root_level = 0;
    sb->dev_item = dev_item;
    sb->uuid_tree_generation = 1;
    memcpy(sb->metadata_uuid, fsid, 16);
    sb->nr_global_roots = 1;

    struct btrfs_disk_key sys_key = { .objectid = 256, .type = BTRFS_CHUNK_ITEM_KEY, .offset = 0x100000 };
    memcpy(sb->sys_chunk_array, &sys_key, sizeof(sys_key));
    memcpy(sb->sys_chunk_array + sizeof(sys_key), &sys_chunk, sizeof(sys_chunk));
    sb->sys_chunk_array_size = sizeof(sys_key) + sizeof(sys_chunk);

    uint32_t sb_csum = crc32c(~0U, (uint8_t *)sb + 32, 4096 - 32) ^ ~0U;
    memcpy(sb->csum, &sb_csum, 4);
    printf("[+] Constructed valid BTRFS image in memory.\n");

    /* Write btrfs.img */
    const char *btrfs_img_path = "/tmp/btrfs.img";
    int fd = open(btrfs_img_path, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        printf("[-] Failed to create %s: %s\n", btrfs_img_path, strerror(errno));
        exit(1);
    }
    ssize_t written = write(fd, img, img_size);
    if (written != (ssize_t)img_size) {
        printf("[-] Failed to write complete image to %s: %s\n", btrfs_img_path, strerror(errno));
        close(fd);
        exit(1);
    }
    close(fd);
    free(img);
    printf("[+] Written 16 MiB image to %s.\n", btrfs_img_path);

    /* Write target dummy device image */
    const char *target_img_path = "/tmp/target.img";
    const size_t target_size = (32 * 1024 * 1024) + 1024; /* Non-multiple of 4096 forces i_blkbits to 10 */
    fd = open(target_img_path, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        printf("[-] Failed to create %s: %s\n", target_img_path, strerror(errno));
        exit(1);
    }
    if (ftruncate(fd, target_size) < 0) {
        printf("[-] Failed to ftruncate %s: %s\n", target_img_path, strerror(errno));
        close(fd);
        exit(1);
    }
    close(fd);
    printf("[+] Created target image at %s (size: %zu).\n", target_img_path, target_size);

    /* Setup loop devices with descriptors closed */
    char loop0_path[64], loop1_path[64];
    if (setup_loop_dev(btrfs_img_path, loop0_path, sizeof(loop0_path), 0) < 0) {
        printf("[-] Failed to setup loop device for %s\n", btrfs_img_path);
        exit(1);
    }
    printf("[+] Bound %s to %s and closed fds.\n", btrfs_img_path, loop0_path);

    if (setup_loop_dev(target_img_path, loop1_path, sizeof(loop1_path), 1024) < 0) {
        printf("[-] Failed to setup loop device for %s\n", target_img_path);
        exit(1);
    }
    printf("[+] Bound %s to %s (bsz=1024) and closed fds.\n", target_img_path, loop1_path);

    /* Trigger Dependency 1: bd_inode->i_rwsem -> mm->mmap_lock via bdev read */
    int bdev_fd = open(loop0_path, O_RDONLY);
    if (bdev_fd < 0) {
        printf("[-] Failed to open %s for read: %s\n", loop0_path, strerror(errno));
        exit(1);
    }
    char bdev_read_buf[4096];
    ssize_t nread = read(bdev_fd, bdev_read_buf, sizeof(bdev_read_buf));
    if (nread < 0) {
        printf("[-] Failed to read %s: %s\n", loop0_path, strerror(errno));
        close(bdev_fd);
        exit(1);
    }
    close(bdev_fd);
    printf("[+] Step 1: Read %zd bytes from %s (registered bd_inode -> mmap_lock).\n",
           nread, loop0_path);

    /* Also read target loop device to register its inode lock class */
    bdev_fd = open(loop1_path, O_RDONLY);
    if (bdev_fd >= 0) {
        read(bdev_fd, bdev_read_buf, sizeof(bdev_read_buf));
        close(bdev_fd);
        printf("[+] Step 1b: Read from %s and closed.\n", loop1_path);
    }

    /* Mount BTRFS filesystem */
    const char *mnt_dir = "/tmp/btrfs_mnt";
    mkdir(mnt_dir, 0755);
    int res = mount(loop0_path, mnt_dir, "btrfs", MS_STRICTATIME, "nospace_cache");
    if (res < 0) {
        printf("[-] Failed to mount %s at %s: %s\n", loop0_path, mnt_dir, strerror(errno));
        exit(1);
    }
    printf("[+] Mounted %s at %s successfully.\n", loop0_path, mnt_dir);

    /* Trigger Dependency 2: mm->mmap_lock -> btrfs_trans_num_writers via mmap with atime touch */
    char testfile[256];
    snprintf(testfile, sizeof(testfile), "%s/testfile", mnt_dir);
    int file_fd = open(testfile, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (file_fd < 0) {
        printf("[-] Failed to create %s: %s\n", testfile, strerror(errno));
        exit(1);
    }
    char file_data[4096];
    memset(file_data, 'A', sizeof(file_data));
    ssize_t nw = write(file_fd, file_data, sizeof(file_data));
    if (nw != (ssize_t)sizeof(file_data)) {
        printf("[-] Failed to write to %s: %s\n", testfile, strerror(errno));
        close(file_fd);
        exit(1);
    }
    /* Set atime back to epoch 0 so touch_atime in mmap triggers transaction */
    struct timespec ts[2] = { { 0, 0 }, { 0, UTIME_OMIT } };
    res = futimens(file_fd, ts);
    if (res < 0) {
        printf("[-] Failed futimens on %s: %s\n", testfile, strerror(errno));
        close(file_fd);
        exit(1);
    }
    void *addr = mmap(NULL, 4096, PROT_READ, MAP_SHARED, file_fd, 0);
    if (addr == MAP_FAILED) {
        printf("[-] Failed to mmap %s: %s\n", testfile, strerror(errno));
        close(file_fd);
        exit(1);
    }
    printf("[+] Step 2: Mmapped %s successfully (registered mmap_lock -> writers).\n", testfile);
    munmap(addr, 4096);
    close(file_fd);

    /* Trigger Dependency 3: btrfs_trans_num_writers -> btrfs_trans_num_extwriters via mkdir */
    char testdir[256];
    snprintf(testdir, sizeof(testdir), "%s/testdir", mnt_dir);
    res = mkdir(testdir, 0755);
    if (res < 0) {
        printf("[-] Failed to mkdir %s: %s\n", testdir, strerror(errno));
        exit(1);
    }
    printf("[+] Step 3: Created directory %s (registered writers -> extwriters).\n", testdir);

    /* Trigger Dependency 0: extwriters -> bd_inode->i_rwsem via BTRFS_IOC_ADD_DEV */
    int mnt_fd = open(mnt_dir, O_RDONLY | O_DIRECTORY);
    if (mnt_fd < 0) {
        printf("[-] Failed to open mount directory %s: %s\n", mnt_dir, strerror(errno));
        exit(1);
    }
    struct btrfs_ioctl_vol_args add_args;
    memset(&add_args, 0, sizeof(add_args));
    add_args.fd = 0;
    strncpy(add_args.name, loop1_path, sizeof(add_args.name) - 1);
    printf("[*] Step 4: Executing ioctl(BTRFS_IOC_ADD_DEV, %s) to complete cycle...\n", loop1_path);
    res = ioctl(mnt_fd, BTRFS_IOC_ADD_DEV, &add_args);
    printf("[+] ioctl(BTRFS_IOC_ADD_DEV) returned %d (errno: %d, %s).\n",
           res, errno, strerror(errno));
    close(mnt_fd);

    /* Delay to allow kernel dmesg and lockdep report to flush */
    printf("[*] Sleeping 2 seconds to allow lockdep report to complete...\n");
    sleep(2);

    printf("[+] Test execution complete.\n");
    return 0;
}
