// https://syzkaller.appspot.com/bug?id=a36e516d95e32f53a6ad5f1793039e4768711dea
#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

static void write_file(const char *path, const char *val)
{
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        printf("[-] Failed to open %s: %s\n", path, strerror(errno));
        exit(1);
    }
    ssize_t len = strlen(val);
    ssize_t res = write(fd, val, len);
    if (res < 0) {
        printf("[-] Failed to write to %s: %s\n", path, strerror(errno));
        close(fd);
        exit(1);
    }
    close(fd);
    printf("[+] Successfully wrote %s to %s\n", val, path);
}

int main(void)
{
    int fd, i;
    char buf[256];

    printf("[*] Setting up fault injection...\n");
    write_file("/sys/kernel/debug/failslab/probability", "5\n");
    write_file("/sys/kernel/debug/failslab/times", "-1\n");
    write_file("/sys/kernel/debug/failslab/task-filter", "1\n");
    write_file("/sys/kernel/debug/failslab/ignore-gfp-wait", "0\n");
    write_file("/proc/self/make-it-fail", "1\n");
    printf("[+] Fault injection setup successful.\n");

    fd = open("/sys/class/fcloop/ctl/add_target_port", O_WRONLY);
    if (fd < 0) {
        printf("[-] Failed to open add_target_port: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] add_target_port opened successfully.\n");

    printf("[*] Triggering bug...\n");
    for (i = 0; i < 10000; i++) {
        snprintf(buf, sizeof(buf), "wwnn=0x%016llx,wwpn=0x%016llx",
                 (unsigned long long)i, (unsigned long long)i);
        ssize_t res = write(fd, buf, strlen(buf));
        if (res < 0) {
            /* Expected to fail due to fault injection, do not exit */
        }
    }

    close(fd);
    printf("[+] Trigger loop finished.\n");
    
    /* Wait for any asynchronous cleanup */
    sleep(2);
    
    return 0;
}
