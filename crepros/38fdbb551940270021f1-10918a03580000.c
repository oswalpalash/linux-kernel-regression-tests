// https://syzkaller.appspot.com/bug?id=a8a475c0432367a3cb82801f862b767db03eb67d
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <endian.h>
#include <arpa/inet.h>
#include <linux/loop.h>

#define OCFS2_SUPER_BLOCK_SIGNATURE "OCFSV2"
#define OCFS2_INODE_SIGNATURE       "INODE01"
#define OCFS2_GROUP_DESC_SIGNATURE  "GROUP01"

#define OCFS2_FEATURE_INCOMPAT_LOCAL_MOUNT  0x0008
#define OCFS2_FEATURE_INCOMPAT_INLINE_DATA  0x0040

#define OCFS2_VALID_FL        0x00000001
#define OCFS2_SYSTEM_FL       0x00000010
#define OCFS2_SUPER_BLOCK_FL  0x00000020
#define OCFS2_LOCAL_ALLOC_FL  0x00000040
#define OCFS2_BITMAP_FL       0x00000080
#define OCFS2_JOURNAL_FL      0x00000100
#define OCFS2_HEARTBEAT_FL    0x00000200
#define OCFS2_CHAIN_FL        0x00000400
#define OCFS2_DEALLOC_FL      0x00000800

#define OCFS2_INLINE_DATA_FL  0x0001

#define BLOCK_SIZE  4096
#define NUM_BLOCKS  2048
#define TOTAL_SIZE  (NUM_BLOCKS * BLOCK_SIZE)

struct ocfs2_dinode {
    uint8_t   i_signature[8];
    uint32_t  i_generation;
    uint16_t  i_suballoc_slot;
    uint16_t  i_suballoc_bit;
    uint16_t  i_links_count_hi;
    uint16_t  i_xattr_inline_size;
    uint32_t  i_clusters;
    uint32_t  i_uid;
    uint32_t  i_gid;
    uint64_t  i_size;
    uint16_t  i_mode;
    uint16_t  i_links_count;
    uint32_t  i_flags;
    uint64_t  i_atime;
    uint64_t  i_ctime;
    uint64_t  i_mtime;
    uint64_t  i_dtime;
    uint64_t  i_blkno;
    uint64_t  i_last_eb_blk;
    uint32_t  i_fs_generation;
    uint32_t  i_atime_nsec;
    uint32_t  i_ctime_nsec;
    uint32_t  i_mtime_nsec;
    uint32_t  i_attr;
    uint16_t  i_orphaned_slot;
    uint16_t  i_dyn_features;
    uint64_t  i_xattr_loc;
    uint8_t   i_check[8];
    uint64_t  i_dx_root;
    uint64_t  i_refcount_loc;
    uint64_t  i_suballoc_loc;
    uint16_t  i_dio_orphaned_slot;
    uint16_t  i_reserved1[3];
    uint64_t  i_reserved2[2];
    union {
        uint64_t i_pad1;
        struct {
            uint64_t i_rdev;
        } dev1;
        struct {
            uint32_t i_used;
            uint32_t i_total;
        } bitmap1;
        struct {
            uint32_t ij_flags;
            uint32_t ij_recovery_generation;
        } journal1;
    } id1;
    uint8_t   id2_data[4096 - 192];
} __attribute__((packed));

struct ocfs2_super_block {
    uint16_t s_major_rev_level;
    uint16_t s_minor_rev_level;
    uint16_t s_mnt_count;
    uint16_t s_max_mnt_count;
    uint16_t s_state;
    uint16_t s_errors;
    uint32_t s_checkinterval;
    uint64_t s_lastcheck;
    uint32_t s_creator_os;
    uint32_t s_feature_compat;
    uint32_t s_feature_incompat;
    uint32_t s_feature_ro_compat;
    uint64_t s_root_blkno;
    uint64_t s_system_dir_blkno;
    uint32_t s_blocksize_bits;
    uint32_t s_clustersize_bits;
    uint16_t s_max_slots;
    uint16_t s_tunefs_flag;
    uint32_t s_uuid_hash;
    uint64_t s_first_cluster_group;
    uint8_t  s_label[64];
    uint8_t  s_uuid[16];
    uint8_t  s_cluster_info[24];
    uint16_t s_xattr_inline_size;
    uint16_t s_reserved0;
    uint32_t s_dx_seed[3];
    uint64_t s_reserved2[15];
} __attribute__((packed));

struct ocfs2_inline_data {
    uint16_t id_count;
    uint16_t id_reserved0;
    uint32_t id_reserved1;
    uint8_t  id_data[];
} __attribute__((packed));

struct ocfs2_chain_rec {
    uint32_t c_free;
    uint32_t c_total;
    uint64_t c_blkno;
} __attribute__((packed));

struct ocfs2_chain_list {
    uint16_t cl_cpg;
    uint16_t cl_bpc;
    uint16_t cl_count;
    uint16_t cl_next_free_rec;
    uint64_t cl_reserved1;
    struct ocfs2_chain_rec cl_recs[];
} __attribute__((packed));

struct ocfs2_extent_rec {
    uint32_t e_cpos;
    uint16_t e_leaf_clusters;
    uint8_t  e_reserved1;
    uint8_t  e_flags;
    uint64_t e_blkno;
} __attribute__((packed));

struct ocfs2_extent_list {
    uint16_t l_tree_depth;
    uint16_t l_count;
    uint16_t l_next_free_rec;
    uint16_t l_reserved1;
    uint64_t l_reserved2;
    struct ocfs2_extent_rec l_recs[];
} __attribute__((packed));

struct ocfs2_local_alloc {
    uint32_t la_bm_off;
    uint16_t la_size;
    uint16_t la_reserved1;
    uint64_t la_reserved2;
    uint8_t  la_bitmap[];
} __attribute__((packed));

struct ocfs2_truncate_rec {
    uint32_t t_start;
    uint32_t t_clusters;
} __attribute__((packed));

struct ocfs2_truncate_log {
    uint16_t tl_count;
    uint16_t tl_used;
    uint32_t tl_reserved1;
    struct ocfs2_truncate_rec tl_recs[];
} __attribute__((packed));

struct ocfs2_dir_entry {
    uint64_t inode;
    uint16_t rec_len;
    uint8_t  name_len;
    uint8_t  file_type;
    char     name[];
} __attribute__((packed));

struct ocfs2_group_desc {
    uint8_t  bg_signature[8];
    uint16_t bg_size;
    uint16_t bg_bits;
    uint16_t bg_free_bits_count;
    uint16_t bg_chain;
    uint32_t bg_generation;
    uint16_t bg_contig_free_bits;
    uint16_t bg_reserved1;
    uint64_t bg_next_group;
    uint64_t bg_parent_dinode;
    uint64_t bg_blkno;
    uint8_t  bg_check[8];
    uint64_t bg_reserved2;
    uint8_t  bg_bitmap[];
} __attribute__((packed));

struct jbd2_journal_superblock {
    uint32_t h_magic;
    uint32_t h_blocktype;
    uint32_t h_sequence;
    uint32_t s_blocksize;
    uint32_t s_maxlen;
    uint32_t s_first;
    uint32_t s_sequence;
    uint32_t s_start;
    uint32_t s_errno;
    uint32_t s_feature_compat;
    uint32_t s_feature_incompat;
    uint32_t s_feature_ro_compat;
    uint8_t  s_uuid[16];
    uint32_t s_nr_users;
    uint32_t s_dynsuper;
    uint32_t s_max_transaction;
    uint32_t s_max_trans_data;
    uint8_t  s_checksum_type;
    uint8_t  s_padding2[3];
    uint32_t s_num_fc_blks;
    uint32_t s_head;
    uint32_t s_padding[40];
    uint32_t s_checksum;
    uint8_t  s_users[16 * 48];
} __attribute__((packed));

static void add_dir_entry(uint8_t *data_buf, int *offset, uint64_t inode,
                          const char *name, uint8_t file_type, int is_last, int max_size)
{
    struct ocfs2_dir_entry *de = (struct ocfs2_dir_entry *)(data_buf + *offset);
    uint8_t nlen = (uint8_t)strlen(name);
    uint16_t rlen;
    if (is_last) {
        rlen = (uint16_t)(max_size - *offset);
    } else {
        rlen = (uint16_t)((nlen + 12 + 3) & ~3);
    }
    de->inode = htole64(inode);
    de->rec_len = htole16(rlen);
    de->name_len = nlen;
    de->file_type = file_type;
    memcpy(de->name, name, nlen);
    *offset += rlen;
}

static void create_ocfs2_image(const char *image_path)
{
    uint8_t *img = calloc(NUM_BLOCKS, BLOCK_SIZE);
    if (!img) {
        printf("[-] Failed to calloc image memory: %s\n", strerror(errno));
        exit(1);
    }

    /* Block 2: Superblock */
    struct ocfs2_dinode *sb_di = (struct ocfs2_dinode *)(img + 2 * BLOCK_SIZE);
    memcpy(sb_di->i_signature, OCFS2_SUPER_BLOCK_SIGNATURE, 8);
    sb_di->i_generation = htole32(1);
    sb_di->i_suballoc_slot = htole16(0);
    sb_di->i_suballoc_bit = htole16(0);
    sb_di->i_links_count = htole16(1);
    sb_di->i_clusters = htole32(NUM_BLOCKS);
    sb_di->i_size = htole64((uint64_t)NUM_BLOCKS * BLOCK_SIZE);
    sb_di->i_mode = htole16(S_IFREG | 0644);
    sb_di->i_flags = htole32(OCFS2_VALID_FL | OCFS2_SYSTEM_FL | OCFS2_SUPER_BLOCK_FL);
    sb_di->i_blkno = htole64(2);
    sb_di->i_fs_generation = htole32(1);

    struct ocfs2_super_block *sb = (struct ocfs2_super_block *)sb_di->id2_data;
    sb->s_major_rev_level = htole16(0);
    sb->s_minor_rev_level = htole16(90);
    sb->s_blocksize_bits = htole32(12);
    sb->s_clustersize_bits = htole32(12);
    sb->s_max_slots = htole16(1);
    sb->s_feature_compat = htole32(0);
    sb->s_feature_incompat = htole32(OCFS2_FEATURE_INCOMPAT_LOCAL_MOUNT |
                                    OCFS2_FEATURE_INCOMPAT_INLINE_DATA);
    sb->s_feature_ro_compat = htole32(0);
    sb->s_root_blkno = htole64(3);
    sb->s_system_dir_blkno = htole64(4);
    sb->s_first_cluster_group = htole64(64);
    sb->s_uuid_hash = htole32(0x12345678);
    for (int i = 0; i < 16; i++) {
        sb->s_uuid[i] = (uint8_t)(i + 1);
    }

    /* Block 3: Root Directory (with 200 regular file entries f0..f199)
     * Must have OCFS2_SYSTEM_FL set because ocfs2_init_global_system_inodes()
     * loads root_blkno with OCFS2_FI_FLAG_SYSFILE.
     */
    struct ocfs2_dinode *root_di = (struct ocfs2_dinode *)(img + 3 * BLOCK_SIZE);
    memcpy(root_di->i_signature, OCFS2_INODE_SIGNATURE, 8);
    root_di->i_generation = htole32(1);
    root_di->i_suballoc_slot = htole16(0);
    root_di->i_suballoc_bit = htole16(0);
    root_di->i_links_count = htole16(2);
    root_di->i_mode = htole16(S_IFDIR | 0755);
    root_di->i_flags = htole32(OCFS2_VALID_FL | OCFS2_SYSTEM_FL);
    root_di->i_dyn_features = htole16(OCFS2_INLINE_DATA_FL);
    root_di->i_blkno = htole64(3);
    root_di->i_fs_generation = htole32(1);
    root_di->i_size = htole64(3896);

    struct ocfs2_inline_data *root_data = (struct ocfs2_inline_data *)root_di->id2_data;
    root_data->id_count = htole16(3896);
    int root_off = 0;
    add_dir_entry(root_data->id_data, &root_off, 3, ".", 2, 0, 3896);
    add_dir_entry(root_data->id_data, &root_off, 3, "..", 2, 0, 3896);
    for (int i = 0; i < 200; i++) {
        char fname[16];
        snprintf(fname, sizeof(fname), "f%d", i);
        int is_last = (i == 199);
        add_dir_entry(root_data->id_data, &root_off, 100 + i, fname, 1, is_last, 3896);
    }

    /* Block 4: System Directory */
    struct ocfs2_dinode *sys_di = (struct ocfs2_dinode *)(img + 4 * BLOCK_SIZE);
    memcpy(sys_di->i_signature, OCFS2_INODE_SIGNATURE, 8);
    sys_di->i_generation = htole32(1);
    sys_di->i_suballoc_slot = htole16(0);
    sys_di->i_suballoc_bit = htole16(0);
    sys_di->i_links_count = htole16(2);
    sys_di->i_mode = htole16(S_IFDIR | 0755);
    sys_di->i_flags = htole32(OCFS2_VALID_FL | OCFS2_SYSTEM_FL);
    sys_di->i_dyn_features = htole16(OCFS2_INLINE_DATA_FL);
    sys_di->i_blkno = htole64(4);
    sys_di->i_fs_generation = htole32(1);
    sys_di->i_size = htole64(3896);

    struct ocfs2_inline_data *sys_data = (struct ocfs2_inline_data *)sys_di->id2_data;
    sys_data->id_count = htole16(3896);
    int sys_off = 0;
    add_dir_entry(sys_data->id_data, &sys_off, 4, ".", 2, 0, 3896);
    add_dir_entry(sys_data->id_data, &sys_off, 3, "..", 2, 0, 3896);
    add_dir_entry(sys_data->id_data, &sys_off, 5, "global_inode_alloc", 1, 0, 3896);
    add_dir_entry(sys_data->id_data, &sys_off, 6, "slot_map", 1, 0, 3896);
    add_dir_entry(sys_data->id_data, &sys_off, 7, "heartbeat", 1, 0, 3896);
    add_dir_entry(sys_data->id_data, &sys_off, 8, "global_bitmap", 1, 0, 3896);
    add_dir_entry(sys_data->id_data, &sys_off, 9, "orphan_dir:0000", 2, 0, 3896);
    add_dir_entry(sys_data->id_data, &sys_off, 10, "extent_alloc:0000", 1, 0, 3896);
    add_dir_entry(sys_data->id_data, &sys_off, 11, "inode_alloc:0000", 1, 0, 3896);
    add_dir_entry(sys_data->id_data, &sys_off, 12, "journal:0000", 1, 0, 3896);
    add_dir_entry(sys_data->id_data, &sys_off, 13, "local_alloc:0000", 1, 0, 3896);
    add_dir_entry(sys_data->id_data, &sys_off, 14, "truncate_log:0000", 1, 1, 3896);

    /* Blocks 5, 10, 11: Chain allocator system inodes */
    int chain_inodes[3] = {5, 10, 11};
    for (int idx = 0; idx < 3; idx++) {
        int blk = chain_inodes[idx];
        struct ocfs2_dinode *di = (struct ocfs2_dinode *)(img + blk * BLOCK_SIZE);
        memcpy(di->i_signature, OCFS2_INODE_SIGNATURE, 8);
        di->i_generation = htole32(1);
        di->i_suballoc_slot = htole16(0);
        di->i_suballoc_bit = htole16(0);
        di->i_links_count = htole16(1);
        di->i_mode = htole16(S_IFREG | 0644);
        di->i_flags = htole32(OCFS2_VALID_FL | OCFS2_SYSTEM_FL | OCFS2_BITMAP_FL | OCFS2_CHAIN_FL);
        di->i_blkno = htole64(blk);
        di->i_fs_generation = htole32(1);
        struct ocfs2_chain_list *cl = (struct ocfs2_chain_list *)di->id2_data;
        cl->cl_cpg = htole16(2048);
        cl->cl_bpc = htole16(1);
        cl->cl_count = htole16(243);
        cl->cl_next_free_rec = htole16(0);
    }

    /* Block 6: slot_map system inode */
    struct ocfs2_dinode *sm_di = (struct ocfs2_dinode *)(img + 6 * BLOCK_SIZE);
    memcpy(sm_di->i_signature, OCFS2_INODE_SIGNATURE, 8);
    sm_di->i_generation = htole32(1);
    sm_di->i_suballoc_slot = htole16(0);
    sm_di->i_suballoc_bit = htole16(0);
    sm_di->i_links_count = htole16(1);
    sm_di->i_mode = htole16(S_IFREG | 0644);
    sm_di->i_flags = htole32(OCFS2_VALID_FL | OCFS2_SYSTEM_FL);
    sm_di->i_blkno = htole64(6);
    sm_di->i_fs_generation = htole32(1);
    sm_di->i_clusters = htole32(1);
    sm_di->i_size = htole64(BLOCK_SIZE);
    struct ocfs2_extent_list *sm_el = (struct ocfs2_extent_list *)sm_di->id2_data;
    sm_el->l_tree_depth = htole16(0);
    sm_el->l_count = htole16(243);
    sm_el->l_next_free_rec = htole16(1);
    sm_el->l_recs[0].e_cpos = htole32(0);
    sm_el->l_recs[0].e_leaf_clusters = htole16(1);
    sm_el->l_recs[0].e_blkno = htole64(20);

    /* Block 20: slot_map data block (0xFFFF indicates empty slot) */
    memset(img + 20 * BLOCK_SIZE, 0xFF, BLOCK_SIZE);

    /* Block 7: heartbeat system inode */
    struct ocfs2_dinode *hb_di = (struct ocfs2_dinode *)(img + 7 * BLOCK_SIZE);
    memcpy(hb_di->i_signature, OCFS2_INODE_SIGNATURE, 8);
    hb_di->i_generation = htole32(1);
    hb_di->i_suballoc_slot = htole16(0);
    hb_di->i_suballoc_bit = htole16(0);
    hb_di->i_links_count = htole16(1);
    hb_di->i_mode = htole16(S_IFREG | 0644);
    hb_di->i_flags = htole32(OCFS2_VALID_FL | OCFS2_SYSTEM_FL | OCFS2_HEARTBEAT_FL);
    hb_di->i_blkno = htole64(7);
    hb_di->i_fs_generation = htole32(1);
    struct ocfs2_extent_list *hb_el = (struct ocfs2_extent_list *)hb_di->id2_data;
    hb_el->l_tree_depth = htole16(0);
    hb_el->l_count = htole16(243);
    hb_el->l_next_free_rec = htole16(0);

    /* Block 8: global_bitmap system inode */
    struct ocfs2_dinode *gb_di = (struct ocfs2_dinode *)(img + 8 * BLOCK_SIZE);
    memcpy(gb_di->i_signature, OCFS2_INODE_SIGNATURE, 8);
    gb_di->i_generation = htole32(1);
    gb_di->i_suballoc_slot = htole16(0);
    gb_di->i_suballoc_bit = htole16(0);
    gb_di->i_links_count = htole16(1);
    gb_di->i_mode = htole16(S_IFREG | 0644);
    gb_di->i_flags = htole32(OCFS2_VALID_FL | OCFS2_SYSTEM_FL | OCFS2_BITMAP_FL | OCFS2_CHAIN_FL);
    gb_di->i_blkno = htole64(8);
    gb_di->i_fs_generation = htole32(1);
    gb_di->i_clusters = htole32(NUM_BLOCKS);
    gb_di->i_size = htole64((uint64_t)NUM_BLOCKS * BLOCK_SIZE);
    struct ocfs2_chain_list *gb_cl = (struct ocfs2_chain_list *)gb_di->id2_data;
    gb_cl->cl_cpg = htole16(2048);
    gb_cl->cl_bpc = htole16(1);
    gb_cl->cl_count = htole16(243);
    gb_cl->cl_next_free_rec = htole16(1);
    gb_cl->cl_recs[0].c_free = htole32(1000);
    gb_cl->cl_recs[0].c_total = htole32(2048);
    gb_cl->cl_recs[0].c_blkno = htole64(64);

    /* Block 64: global_bitmap group descriptor */
    struct ocfs2_group_desc *gd = (struct ocfs2_group_desc *)(img + 64 * BLOCK_SIZE);
    memcpy(gd->bg_signature, OCFS2_GROUP_DESC_SIGNATURE, 8);
    gd->bg_size = htole16(256);
    gd->bg_bits = htole16(2048);
    gd->bg_free_bits_count = htole16(1000);
    gd->bg_chain = htole16(0);
    gd->bg_generation = htole32(1);
    gd->bg_parent_dinode = htole64(8);
    gd->bg_blkno = htole64(64);
    memset(gd->bg_bitmap, 0xFF, 131);

    /* Block 9: orphan_dir:0000 */
    struct ocfs2_dinode *orph_di = (struct ocfs2_dinode *)(img + 9 * BLOCK_SIZE);
    memcpy(orph_di->i_signature, OCFS2_INODE_SIGNATURE, 8);
    orph_di->i_generation = htole32(1);
    orph_di->i_suballoc_slot = htole16(0);
    orph_di->i_suballoc_bit = htole16(0);
    orph_di->i_links_count = htole16(2);
    orph_di->i_mode = htole16(S_IFDIR | 0755);
    orph_di->i_flags = htole32(OCFS2_VALID_FL | OCFS2_SYSTEM_FL);
    orph_di->i_dyn_features = htole16(OCFS2_INLINE_DATA_FL);
    orph_di->i_blkno = htole64(9);
    orph_di->i_fs_generation = htole32(1);
    orph_di->i_size = htole64(3896);
    struct ocfs2_inline_data *orph_data = (struct ocfs2_inline_data *)orph_di->id2_data;
    orph_data->id_count = htole16(3896);
    int orph_off = 0;
    add_dir_entry(orph_data->id_data, &orph_off, 9, ".", 2, 0, 3896);
    add_dir_entry(orph_data->id_data, &orph_off, 4, "..", 2, 1, 3896);

    /* Block 12: journal:0000 system inode */
    struct ocfs2_dinode *j_di = (struct ocfs2_dinode *)(img + 12 * BLOCK_SIZE);
    memcpy(j_di->i_signature, OCFS2_INODE_SIGNATURE, 8);
    j_di->i_generation = htole32(1);
    j_di->i_suballoc_slot = htole16(0);
    j_di->i_suballoc_bit = htole16(0);
    j_di->i_links_count = htole16(1);
    j_di->i_mode = htole16(S_IFREG | 0644);
    j_di->i_flags = htole32(OCFS2_VALID_FL | OCFS2_SYSTEM_FL | OCFS2_JOURNAL_FL);
    j_di->i_blkno = htole64(12);
    j_di->i_fs_generation = htole32(1);
    j_di->i_size = htole64(4 * 1024 * 1024);
    j_di->i_clusters = htole32(1024);
    struct ocfs2_extent_list *j_el = (struct ocfs2_extent_list *)j_di->id2_data;
    j_el->l_tree_depth = htole16(0);
    j_el->l_count = htole16(243);
    j_el->l_next_free_rec = htole16(1);
    j_el->l_recs[0].e_cpos = htole32(0);
    j_el->l_recs[0].e_leaf_clusters = htole16(1024);
    j_el->l_recs[0].e_blkno = htole64(1024);

    /* Block 1024: JBD2 journal superblock */
    struct jbd2_journal_superblock *jsb =
        (struct jbd2_journal_superblock *)(img + 1024 * BLOCK_SIZE);
    jsb->h_magic = htonl(0xc03b3998);
    jsb->h_blocktype = htonl(4);
    jsb->h_sequence = htonl(1);
    jsb->s_blocksize = htonl(4096);
    jsb->s_maxlen = htonl(1024);
    jsb->s_first = htonl(1);
    jsb->s_sequence = htonl(1);
    jsb->s_start = 0;

    /* Block 13: local_alloc:0000 */
    struct ocfs2_dinode *la_di = (struct ocfs2_dinode *)(img + 13 * BLOCK_SIZE);
    memcpy(la_di->i_signature, OCFS2_INODE_SIGNATURE, 8);
    la_di->i_generation = htole32(1);
    la_di->i_suballoc_slot = htole16(0);
    la_di->i_suballoc_bit = htole16(0);
    la_di->i_links_count = htole16(1);
    la_di->i_mode = htole16(S_IFREG | 0644);
    la_di->i_flags = htole32(OCFS2_VALID_FL | OCFS2_SYSTEM_FL |
                            OCFS2_BITMAP_FL | OCFS2_LOCAL_ALLOC_FL);
    la_di->i_blkno = htole64(13);
    la_di->i_fs_generation = htole32(1);
    struct ocfs2_local_alloc *la = (struct ocfs2_local_alloc *)la_di->id2_data;
    la->la_size = htole16(3888);
    la->la_bm_off = htole32(0);
    la_di->id1.bitmap1.i_used = htole32(0);
    la_di->id1.bitmap1.i_total = htole32(0);

    /* Block 14: truncate_log:0000 */
    struct ocfs2_dinode *tl_di = (struct ocfs2_dinode *)(img + 14 * BLOCK_SIZE);
    memcpy(tl_di->i_signature, OCFS2_INODE_SIGNATURE, 8);
    tl_di->i_generation = htole32(1);
    tl_di->i_suballoc_slot = htole16(0);
    tl_di->i_suballoc_bit = htole16(0);
    tl_di->i_links_count = htole16(1);
    tl_di->i_mode = htole16(S_IFREG | 0644);
    tl_di->i_flags = htole32(OCFS2_VALID_FL | OCFS2_SYSTEM_FL | OCFS2_DEALLOC_FL);
    tl_di->i_blkno = htole64(14);
    tl_di->i_fs_generation = htole32(1);
    struct ocfs2_truncate_log *tl = (struct ocfs2_truncate_log *)tl_di->id2_data;
    tl->tl_count = htole16(487);
    tl->tl_used = htole16(0);

    /* Blocks 100..299: Regular file inodes f0..f199
     * Must NOT have OCFS2_SYSTEM_FL because ocfs2_lookup() reads them with flags = 0.
     * Must have valid extent list (l_count = 243) and valid suballoc slot/bit (0).
     */
    for (int i = 0; i < 200; i++) {
        int blk = 100 + i;
        struct ocfs2_dinode *f_di = (struct ocfs2_dinode *)(img + blk * BLOCK_SIZE);
        memcpy(f_di->i_signature, OCFS2_INODE_SIGNATURE, 8);
        f_di->i_generation = htole32(1);
        f_di->i_suballoc_slot = htole16(0);
        f_di->i_suballoc_bit = htole16(0);
        f_di->i_links_count = htole16(1);
        f_di->i_clusters = htole32(0);
        f_di->i_size = htole64(0);
        f_di->i_mode = htole16(S_IFREG | 0644);
        f_di->i_flags = htole32(OCFS2_VALID_FL);
        f_di->i_dyn_features = 0;
        f_di->i_blkno = htole64(blk);
        f_di->i_fs_generation = htole32(1);
        struct ocfs2_extent_list *f_el = (struct ocfs2_extent_list *)f_di->id2_data;
        f_el->l_tree_depth = htole16(0);
        f_el->l_count = htole16(243);
        f_el->l_next_free_rec = htole16(0);
    }

    unlink(image_path);
    int img_fd = open(image_path, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (img_fd < 0) {
        printf("[-] Failed to open %s: %s\n", image_path, strerror(errno));
        free(img);
        exit(1);
    }

    ssize_t written = write(img_fd, img, TOTAL_SIZE);
    if (written != TOTAL_SIZE) {
        printf("[-] Incomplete write of image: %zd vs %d (%s)\n",
               written, TOTAL_SIZE, strerror(errno));
        close(img_fd);
        free(img);
        exit(1);
    }
    close(img_fd);
    free(img);
    printf("[+] Created ocfs2 disk image %s (%d bytes).\n", image_path, TOTAL_SIZE);
}

static int verify_and_attach_v1(const char *cg_path)
{
    char lim_path[512];
    snprintf(lim_path, sizeof(lim_path), "%s/memory.limit_in_bytes", cg_path);
    if (access(lim_path, F_OK) != 0) {
        snprintf(lim_path, sizeof(lim_path), "%s/memory.usage_in_bytes", cg_path);
        if (access(lim_path, F_OK) != 0)
            return 0;
    }

    char tasks_path[512];
    snprintf(tasks_path, sizeof(tasks_path), "%s/tasks", cg_path);
    int tfd = open(tasks_path, O_WRONLY);
    if (tfd >= 0) {
        char pid_str[32];
        snprintf(pid_str, sizeof(pid_str), "%d\n", getpid());
        if (write(tfd, pid_str, strlen(pid_str)) > 0) {
            printf("[+] Attached pid %d to %s\n", getpid(), tasks_path);
        }
        close(tfd);
    }
    return 1;
}

static int verify_and_attach_v2(const char *cg_path)
{
    char max_path[512];
    snprintf(max_path, sizeof(max_path), "%s/memory.max", cg_path);
    if (access(max_path, F_OK) != 0) {
        snprintf(max_path, sizeof(max_path), "%s/memory.current", cg_path);
        if (access(max_path, F_OK) != 0)
            return 0;
    }

    char procs_path[512];
    snprintf(procs_path, sizeof(procs_path), "%s/cgroup.procs", cg_path);
    int pfd = open(procs_path, O_WRONLY);
    if (pfd >= 0) {
        char pid_str[32];
        snprintf(pid_str, sizeof(pid_str), "%d\n", getpid());
        if (write(pfd, pid_str, strlen(pid_str)) > 0) {
            printf("[+] Attached pid %d to %s\n", getpid(), procs_path);
        }
        close(pfd);
    }
    return 1;
}

/* Dynamically locate or mount memory cgroup hierarchy to record
 * shrinker_mutex -> fs_reclaim dependency via alloc_shrinker_info().
 */
static int setup_memcg(char *cg_test_path, size_t max_len)
{
    FILE *fp = fopen("/proc/mounts", "r");
    if (!fp) {
        printf("[-] Failed to open /proc/mounts: %s\n", strerror(errno));
        return -1;
    }

    char line[512];
    char cgroup2_mnt[256] = {0};
    char cgroup1_mem_mnt[256] = {0};

    while (fgets(line, sizeof(line), fp)) {
        char spec[256], file[256], vfstype[64], mntopts[256];
        if (sscanf(line, "%255s %255s %63s %255s", spec, file, vfstype, mntopts) == 4) {
            if (strcmp(vfstype, "cgroup2") == 0 && !cgroup2_mnt[0]) {
                strncpy(cgroup2_mnt, file, sizeof(cgroup2_mnt) - 1);
            } else if (strcmp(vfstype, "cgroup") == 0 && strstr(mntopts, "memory") && !cgroup1_mem_mnt[0]) {
                strncpy(cgroup1_mem_mnt, file, sizeof(cgroup1_mem_mnt) - 1);
            }
        }
    }
    fclose(fp);

    printf("[+] /proc/mounts scan: cgroup1_mem='%s', cgroup2='%s'\n",
           cgroup1_mem_mnt, cgroup2_mnt);

    /* Strategy 1: Use existing cgroup v1 memory mount */
    if (cgroup1_mem_mnt[0]) {
        snprintf(cg_test_path, max_len, "%s/cg_test", cgroup1_mem_mnt);
        rmdir(cg_test_path);
        if (mkdir(cg_test_path, 0755) == 0 || errno == EEXIST) {
            if (verify_and_attach_v1(cg_test_path)) {
                printf("[+] Verified cgroup v1 memcg at %s\n", cg_test_path);
                return 0;
            }
            printf("[-] cgroup v1 at %s missing memory controller files\n", cg_test_path);
            rmdir(cg_test_path);
        } else {
            printf("[-] mkdir %s failed: %s\n", cg_test_path, strerror(errno));
        }
    }

    /* Strategy 2: Use existing cgroup2 mount */
    if (cgroup2_mnt[0]) {
        char ctrl_path[512];
        snprintf(ctrl_path, sizeof(ctrl_path), "%s/cgroup.subtree_control", cgroup2_mnt);
        int cfd = open(ctrl_path, O_WRONLY);
        if (cfd >= 0) {
            ssize_t w = write(cfd, "+memory\n", 8);
            if (w < 0) {
                printf("[-] Failed to write +memory to %s: %s\n", ctrl_path, strerror(errno));
            } else {
                printf("[+] Wrote +memory to %s\n", ctrl_path);
            }
            close(cfd);
        } else {
            printf("[-] Failed to open %s: %s\n", ctrl_path, strerror(errno));
        }

        snprintf(cg_test_path, max_len, "%s/cg_test", cgroup2_mnt);
        rmdir(cg_test_path);
        if (mkdir(cg_test_path, 0755) == 0 || errno == EEXIST) {
            if (verify_and_attach_v2(cg_test_path)) {
                printf("[+] Verified cgroup v2 memcg at %s\n", cg_test_path);
                return 0;
            }
            printf("[-] cgroup v2 at %s missing memory controller files\n", cg_test_path);
            rmdir(cg_test_path);
        } else {
            printf("[-] mkdir %s failed: %s\n", cg_test_path, strerror(errno));
        }
    }

    /* Strategy 3: Mount cgroup2 at /tmp/cg2 */
    mkdir("/tmp/cg2", 0755);
    if (mount("none", "/tmp/cg2", "cgroup2", 0, NULL) == 0) {
        printf("[+] Mounted cgroup2 at /tmp/cg2\n");
        int cfd = open("/tmp/cg2/cgroup.subtree_control", O_WRONLY);
        if (cfd >= 0) {
            write(cfd, "+memory\n", 8);
            close(cfd);
            printf("[+] Wrote +memory to /tmp/cg2/cgroup.subtree_control\n");
        }
        snprintf(cg_test_path, max_len, "/tmp/cg2/cg_test");
        rmdir(cg_test_path);
        if (mkdir(cg_test_path, 0755) == 0 || errno == EEXIST) {
            if (verify_and_attach_v2(cg_test_path)) {
                printf("[+] Verified mounted cgroup v2 memcg at %s\n", cg_test_path);
                return 0;
            }
            printf("[-] /tmp/cg2/cg_test missing memory controller files\n");
            rmdir(cg_test_path);
        }
    } else {
        printf("[-] Failed to mount cgroup2 at /tmp/cg2: %s\n", strerror(errno));
    }

    /* Strategy 4: Mount cgroup v1 memory at /tmp/cg1 */
    mkdir("/tmp/cg1", 0755);
    if (mount("none", "/tmp/cg1", "cgroup", 0, "memory") == 0) {
        printf("[+] Mounted cgroup v1 at /tmp/cg1\n");
        snprintf(cg_test_path, max_len, "/tmp/cg1/cg_test");
        rmdir(cg_test_path);
        if (mkdir(cg_test_path, 0755) == 0 || errno == EEXIST) {
            if (verify_and_attach_v1(cg_test_path)) {
                printf("[+] Verified mounted cgroup v1 memcg at %s\n", cg_test_path);
                return 0;
            }
            printf("[-] /tmp/cg1/cg_test missing memory controller files\n");
            rmdir(cg_test_path);
        }
    } else {
        printf("[-] Failed to mount cgroup v1 at /tmp/cg1: %s\n", strerror(errno));
    }

    return -1;
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("[+] Starting reproducer for deadlock in shrinker_free...\n");

    const char *img_path = "/tmp/ocfs2.img";
    const char *mnt_path = "/tmp/ocfs2_mnt";

    /* Step 1: Create raw ocfs2 filesystem image with valid dinodes */
    create_ocfs2_image(img_path);

    /* Step 2: Acquire loop device and attach image */
    int loop_ctrl_fd = open("/dev/loop-control", O_RDWR);
    if (loop_ctrl_fd < 0) {
        printf("[-] Failed to open /dev/loop-control: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] Opened /dev/loop-control.\n");

    int loop_num = ioctl(loop_ctrl_fd, LOOP_CTL_GET_FREE);
    if (loop_num < 0 || loop_num >= 32) {
        printf("[-] LOOP_CTL_GET_FREE returned %d: %s, using fallback loop0\n",
               loop_num, strerror(errno));
        loop_num = 0;
    }
    close(loop_ctrl_fd);
    printf("[+] Using loop device /dev/loop%d.\n", loop_num);

    char loop_dev[64];
    snprintf(loop_dev, sizeof(loop_dev), "/dev/loop%d", loop_num);

    /* Preventive clear in case device was previously used */
    int pfd = open(loop_dev, O_RDWR);
    if (pfd >= 0) {
        ioctl(pfd, LOOP_CLR_FD, 0);
        close(pfd);
    }

    int loop_fd = open(loop_dev, O_RDWR);
    if (loop_fd < 0) {
        printf("[-] Failed to open %s: %s\n", loop_dev, strerror(errno));
        exit(1);
    }
    printf("[+] Opened %s.\n", loop_dev);

    int img_fd = open(img_path, O_RDWR);
    if (img_fd < 0) {
        printf("[-] Failed to open %s for loop attach: %s\n", img_path, strerror(errno));
        close(loop_fd);
        exit(1);
    }
    printf("[+] Opened %s for loop attach.\n", img_path);

    int sfd_res = ioctl(loop_fd, LOOP_SET_FD, img_fd);
    if (sfd_res < 0) {
        printf("[-] Failed LOOP_SET_FD on %s: %s\n", loop_dev, strerror(errno));
        close(img_fd);
        close(loop_fd);
        exit(1);
    }
    printf("[+] LOOP_SET_FD succeeded on %s.\n", loop_dev);

    /* Close both img_fd and loop_fd prior to mount.
     * setup_bdev_super() opens block device with BLK_OPEN_RESTRICT_WRITES,
     * which fails with -EBUSY if bdev->bd_writers > 0.
     */
    close(img_fd);
    close(loop_fd);
    printf("[+] Closed img_fd and loop_fd prior to mount.\n");

    /* Step 3: Establish shrinker_mutex -> fs_reclaim dependency via memcg */
    printf("[+] Establishing shrinker_mutex -> fs_reclaim dependency via memcg...\n");
    char cg_path[512] = {0};
    int cg_ret = setup_memcg(cg_path, sizeof(cg_path));
    if (cg_ret < 0) {
        printf("[-] Failed to setup memcg, continuing with reproduction attempts...\n");
    } else {
        printf("[+] Memory cgroup ready at %s\n", cg_path);
    }

    /* Step 4: Mount ocfs2 filesystem with heartbeat=none */
    mkdir(mnt_path, 0755);
    int mres = mount(loop_dev, mnt_path, "ocfs2", 0, "heartbeat=none");
    if (mres < 0) {
        printf("[-] Failed to mount %s on %s: %s\n", loop_dev, mnt_path, strerror(errno));
        int clr_fd = open(loop_dev, O_RDWR);
        if (clr_fd >= 0) {
            ioctl(clr_fd, LOOP_CLR_FD, 0);
            close(clr_fd);
        }
        unlink(img_path);
        rmdir(mnt_path);
        exit(1);
    }
    printf("[+] Successfully mounted %s on %s with options 'heartbeat=none'.\n",
           loop_dev, mnt_path);

    /* Step 5: Stat all 200 files to load dentries/inodes into dcache LRU */
    printf("[+] Populating dcache LRU with ocfs2 dentries...\n");
    int stat_success = 0;
    for (int i = 0; i < 200; i++) {
        char fpath[128];
        snprintf(fpath, sizeof(fpath), "%s/f%d", mnt_path, i);
        struct stat st;
        int sres = stat(fpath, &st);
        if (sres == 0) {
            stat_success++;
        } else if (i == 0) {
            printf("[-] stat(%s) failed: %s\n", fpath, strerror(errno));
        }
    }
    printf("[+] Completed accessing 200 files (successful: %d).\n", stat_success);

    /* Step 6: Trigger memory reclaim under fs_reclaim to establish
     *         fs_reclaim -> &osb->journal_shutdown_sem dependency.
     * Note: We avoid writing to /proc/sys/vm/drop_caches because drop_slab()
     * does not acquire fs_reclaim and would prematurely evict dentries.
     */
    printf("[+] Triggering memory reclaim under fs_reclaim...\n");

    /* 6a. Proactive reclaim via sysfs node interface (__node_reclaim -> fs_reclaim) */
    for (int nid = 0; nid < 8; nid++) {
        char npath[64];
        snprintf(npath, sizeof(npath), "/sys/devices/system/node/node%d/reclaim", nid);
        int rfd = open(npath, O_WRONLY);
        if (rfd >= 0) {
            const char *val = "67108864\n";
            ssize_t w = write(rfd, val, strlen(val));
            if (w < 0) {
                printf("[-] Write to %s returned: %s (expected if reclaim target not met)\n",
                       npath, strerror(errno));
            } else {
                printf("[+] Wrote 64MB reclaim target to %s.\n", npath);
            }
            close(rfd);
        }
    }

    /* 6b. Proactive reclaim via memcg interface if available */
    if (cg_path[0]) {
        char rec_path[512];
        snprintf(rec_path, sizeof(rec_path), "%s/memory.reclaim", cg_path);
        int rcfd = open(rec_path, O_WRONLY);
        if (rcfd >= 0) {
            const char *val = "67108864\n";
            ssize_t w = write(rcfd, val, strlen(val));
            if (w > 0) {
                printf("[+] Triggered proactive reclaim via %s\n", rec_path);
            }
            close(rcfd);
        }

        snprintf(rec_path, sizeof(rec_path), "%s/memory.force_empty", cg_path);
        rcfd = open(rec_path, O_WRONLY);
        if (rcfd >= 0) {
            const char *val = "0\n";
            ssize_t w = write(rcfd, val, strlen(val));
            if (w > 0) {
                printf("[+] Triggered force_empty via %s\n", rec_path);
            }
            close(rcfd);
        }
    }

    /* 6c. Memory pressure helper process to invoke kswapd & direct reclaim */
    unsigned long mem_avail_kb = 0;
    FILE *mfp = fopen("/proc/meminfo", "r");
    if (mfp) {
        char line[128];
        while (fgets(line, sizeof(line), mfp)) {
            if (sscanf(line, "MemAvailable: %lu kB", &mem_avail_kb) == 1)
                break;
        }
        fclose(mfp);
        printf("[+] Available memory: %lu kB.\n", mem_avail_kb);
    }

    size_t target_bytes = ((size_t)mem_avail_kb * 1024 * 85) / 100;
    if (target_bytes == 0)
        target_bytes = 512 * 1024 * 1024;
    printf("[+] Target memory allocation pressure: %zu MB.\n", target_bytes / (1024 * 1024));

    pid_t pid = fork();
    if (pid < 0) {
        printf("[-] Failed to fork memory pressure child: %s\n", strerror(errno));
    } else if (pid == 0) {
        int oom_fd = open("/proc/self/oom_score_adj", O_WRONLY);
        if (oom_fd >= 0) {
            write(oom_fd, "1000\n", 5);
            close(oom_fd);
        }
        size_t allocated = 0;
        size_t chunk = 32 * 1024 * 1024;
        while (allocated < target_bytes) {
            if (allocated + chunk > target_bytes)
                chunk = target_bytes - allocated;
            char *p = mmap(NULL, chunk, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (p == MAP_FAILED)
                break;
            for (size_t off = 0; off < chunk; off += 4096)
                p[off] = 1;
            allocated += chunk;
        }
        usleep(200000);
        _exit(0);
    } else {
        int status = 0;
        waitpid(pid, &status, 0);
        printf("[+] Memory pressure child finished (status: %d).\n", status);
    }

    /* Step 7: Unmount ocfs2 to trigger circular locking dependency warning */
    printf("[+] Unmounting %s to trigger shrinker_free deadlock warning...\n", mnt_path);
    int ures = umount(mnt_path);
    if (ures < 0) {
        printf("[-] Failed to umount %s: %s\n", mnt_path, strerror(errno));
    } else {
        printf("[+] Unmounted %s successfully.\n", mnt_path);
    }

    /* Step 8: Cleanup loop device and temporary files */
    int clr_fd = open(loop_dev, O_RDWR);
    if (clr_fd >= 0) {
        int clr_res = ioctl(clr_fd, LOOP_CLR_FD, 0);
        if (clr_res < 0) {
            printf("[-] Failed LOOP_CLR_FD: %s\n", strerror(errno));
        } else {
            printf("[+] Detached loop device %s.\n", loop_dev);
        }
        close(clr_fd);
    } else {
        printf("[-] Failed to open %s for LOOP_CLR_FD: %s\n", loop_dev, strerror(errno));
    }

    unlink(img_path);
    rmdir(mnt_path);
    if (cg_path[0])
        rmdir(cg_path);
    umount("/tmp/cg2");
    rmdir("/tmp/cg2");
    umount("/tmp/cg1");
    rmdir("/tmp/cg1");

    /* Allow asynchronous kernel logs to flush */
    sleep(1);
    printf("[+] Reproducer sequence finished.\n");
    return 0;
}
