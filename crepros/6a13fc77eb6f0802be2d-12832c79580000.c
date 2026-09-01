// https://syzkaller.appspot.com/bug?id=7700bc790eb8f2a046687140f18b6e95601bf6f6
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <errno.h>

#ifndef MNT_DETACH
#define MNT_DETACH 2
#endif

// A minimal 9P2000.L server to satisfy mount and open requests
static void *ninep_server(void *arg) {
    int fd = (int)(intptr_t)arg;
    uint32_t size;
    uint8_t id;
    uint16_t tag;
    char buf[8192];
    uint8_t fid_type[1024] = {0}; // 1 = dir, 2 = file
    
    while (1) {
        if (recv(fd, &size, 4, MSG_WAITALL) != 4) break;
        if (recv(fd, &id, 1, MSG_WAITALL) != 1) break;
        if (recv(fd, &tag, 2, MSG_WAITALL) != 2) break;
        
        int payload_size = size - 7;
        if (payload_size > 0) {
            if (payload_size > sizeof(buf)) break;
            if (recv(fd, buf, payload_size, MSG_WAITALL) != payload_size) break;
        }
        
        uint8_t reply_id = id + 1;
        uint32_t reply_size = 7;
        char reply[256];
        memset(reply, 0, sizeof(reply));
        
        if (id == 100) { // TVERSION
            reply_size = 7 + 4 + 2 + 8;
            *(uint32_t*)(reply + 7) = 8192;
            *(uint16_t*)(reply + 11) = 8;
            memcpy(reply + 13, "9P2000.L", 8);
        } else if (id == 104) { // TATTACH
            uint32_t fid = *(uint32_t*)buf;
            if (fid < 1024) fid_type[fid] = 1; // dir
            reply_size = 7 + 13;
            reply[7] = 0x80; // QTDIR
            *(uint32_t*)(reply + 8) = 0;
            *(uint64_t*)(reply + 12) = 1;
        } else if (id == 110) { // TWALK
            uint32_t fid = *(uint32_t*)buf;
            uint32_t newfid = *(uint32_t*)(buf + 4);
            uint16_t nwname = *(uint16_t*)(buf + 8);
            if (nwname == 0) {
                if (newfid < 1024 && fid < 1024) fid_type[newfid] = fid_type[fid];
                reply_size = 7 + 2;
                *(uint16_t*)(reply + 7) = 0;
            } else {
                if (newfid < 1024) fid_type[newfid] = 2; // file
                reply_size = 7 + 2 + 13;
                *(uint16_t*)(reply + 7) = 1;
                reply[9] = 0; // QTFILE
                *(uint32_t*)(reply + 10) = 0;
                *(uint64_t*)(reply + 14) = 2;
            }
        } else if (id == 12) { // TLOPEN
            uint32_t fid = *(uint32_t*)buf;
            if (fid < 1024) fid_type[fid] = 2; // file
            reply_size = 7 + 13 + 4;
            reply[7] = 0; // QTFILE
            *(uint32_t*)(reply + 8) = 0;
            *(uint64_t*)(reply + 12) = 2;
            *(uint32_t*)(reply + 20) = 8192;
        } else if (id == 14) { // TLCREATE
            uint32_t fid = *(uint32_t*)buf;
            if (fid < 1024) fid_type[fid] = 2; // file
            reply_size = 7 + 13 + 4;
            reply[7] = 0; // QTFILE
            *(uint32_t*)(reply + 8) = 0;
            *(uint64_t*)(reply + 12) = 2;
            *(uint32_t*)(reply + 20) = 8192;
        } else if (id == 24) { // TGETATTR
            uint32_t fid = *(uint32_t*)buf;
            uint8_t type = (fid < 1024 && fid_type[fid] == 1) ? 1 : 2;
            reply_size = 7 + 153;
            *(uint64_t*)(reply + 7) = 0x3fffULL; // all fields valid
            if (type == 1) {
                reply[15] = 0x80; // QTDIR
                *(uint32_t*)(reply + 16) = 0;
                *(uint64_t*)(reply + 20) = 1;
                *(uint32_t*)(reply + 28) = 0777 | S_IFDIR; // st_mode
                *(uint64_t*)(reply + 40) = 2; // st_nlink
            } else {
                reply[15] = 0; // QTFILE
                *(uint32_t*)(reply + 16) = 0;
                *(uint64_t*)(reply + 20) = 2;
                *(uint32_t*)(reply + 28) = 0777 | S_IFREG; // st_mode
                *(uint64_t*)(reply + 40) = 1; // st_nlink
            }
            *(uint32_t*)(reply + 32) = 0; // st_uid
            *(uint32_t*)(reply + 36) = 0; // st_gid
            *(uint64_t*)(reply + 48) = 0; // st_rdev
            *(uint64_t*)(reply + 56) = 4096; // st_size
            *(uint64_t*)(reply + 64) = 4096; // st_blksize
            *(uint64_t*)(reply + 72) = 8; // st_blocks
        } else if (id == 70) { // TSTATFS
            reply_size = 7 + 60;
            *(uint32_t*)(reply + 7) = 0x01021997; // type
            *(uint32_t*)(reply + 11) = 4096; // bsize
            *(uint64_t*)(reply + 15) = 1000; // blocks
            *(uint64_t*)(reply + 23) = 1000; // bfree
            *(uint64_t*)(reply + 31) = 1000; // bavail
            *(uint64_t*)(reply + 39) = 1000; // files
            *(uint64_t*)(reply + 47) = 1000; // ffree
            *(uint64_t*)(reply + 55) = 0; // fsid
            *(uint32_t*)(reply + 63) = 255; // namelen
        } else if (id == 118) { // TWRITE
            reply_size = 7 + 4;
            *(uint32_t*)(reply + 7) = *(uint32_t*)(buf + 12); // count
        } else if (id == 116) { // TREAD
            reply_size = 7 + 4;
            *(uint32_t*)(reply + 7) = 0; // count = 0
        } else if (id == 120 || id == 108 || id == 50) { // TCLUNK, TFLUSH, TFSYNC
            reply_size = 7;
        } else {
            reply_id = 107; // RLERROR
            reply_size = 7 + 4;
            *(uint32_t*)(reply + 7) = 2; // ENOENT
        }
        
        *(uint32_t*)reply = reply_size;
        reply[4] = reply_id;
        *(uint16_t*)(reply + 5) = tag;
        
        if (send(fd, reply, reply_size, MSG_NOSIGNAL) != reply_size) break;
    }
    return NULL;
}

void set_ignore_gfp_wait(const char *name) {
    char path[256];
    snprintf(path, sizeof(path), "/sys/kernel/debug/%s/ignore-gfp-wait", name);
    int fd = open(path, O_WRONLY);
    if (fd >= 0) {
        if (write(fd, "N\n", 2) < 0) {
            printf("[-] Failed to write to %s: %s\n", path, strerror(errno));
        } else {
            printf("[+] Set %s to N\n", path);
        }
        close(fd);
    } else {
        printf("[-] Failed to open %s: %s\n", path, strerror(errno));
    }
}

int main() {
    int fds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) < 0) {
        printf("[-] socketpair failed: %s\n", strerror(errno));
        return 1;
    }
    printf("[+] socketpair successful.\n");

    pthread_t tid;
    if (pthread_create(&tid, NULL, ninep_server, (void*)(intptr_t)fds[1]) != 0) {
        printf("[-] pthread_create failed: %s\n", strerror(errno));
        return 1;
    }
    printf("[+] pthread_create successful.\n");

    char mount_dir[] = "/tmp/9p_XXXXXX";
    if (!mkdtemp(mount_dir)) {
        printf("[-] mkdtemp failed: %s\n", strerror(errno));
        return 1;
    }
    printf("[+] mkdtemp successful: %s\n", mount_dir);

    char opts[256];
    // cache=none forces unbuffered writes, which routes through netfs_unbuffered_write
    snprintf(opts, sizeof(opts), "trans=fd,rfdno=%d,wfdno=%d,version=9p2000.L,cache=none", fds[0], fds[0]);
    
    if (mount("9p", mount_dir, "9p", 0, opts) < 0) {
        printf("[-] mount failed: %s\n", strerror(errno));
        rmdir(mount_dir);
        return 1;
    }
    printf("[+] mount successful.\n");

    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s/test", mount_dir);
    int fd = open(file_path, O_WRONLY | O_CREAT, 0666);
    if (fd < 0) {
        printf("[-] open failed: %s\n", strerror(errno));
        umount2(mount_dir, MNT_DETACH);
        rmdir(mount_dir);
        return 1;
    }
    printf("[+] open successful.\n");

    // Enable fault injection for GFP_KERNEL allocations
    set_ignore_gfp_wait("failslab");
    set_ignore_gfp_wait("fail_page_alloc");

    int fail_fd = open("/proc/self/fail-nth", O_RDWR);
    if (fail_fd < 0) {
        printf("[-] Failed to open fail-nth: %s\n", strerror(errno));
    } else {
        printf("[+] Opened fail-nth successfully.\n");
    }

    // We iterate fail-nth to systematically fail the N-th allocation.
    // This avoids needing to know exactly how many allocations happen before netfs_alloc_subrequest.
    for (int i = 1; i <= 15; i++) {
        printf("[*] Trying fail-nth = %d\n", i);
        
        if (fail_fd >= 0) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%d", i);
            if (pwrite(fail_fd, buf, strlen(buf), 0) < 0) {
                printf("[-] Failed to write fail-nth: %s\n", strerror(errno));
            }
        }

        // This will trigger netfs_prepare_write(). When fail-nth hits the netfs_alloc_subrequest
        // allocation, it will return NULL and crash the kernel.
        ssize_t res = write(fd, "test", 4);
        if (res < 0) {
            printf("[-] write failed: %s\n", strerror(errno));
        } else {
            printf("[+] write successful: %zd bytes\n", res);
        }

        // Reset fail-nth to 0 just in case
        if (fail_fd >= 0) {
            if (pwrite(fail_fd, "0", 1, 0) < 0) {
                // ignore
            }
        }
    }

    if (fail_fd >= 0) close(fail_fd);
    close(fd);
    umount2(mount_dir, MNT_DETACH);
    rmdir(mount_dir);
    return 0;
}
