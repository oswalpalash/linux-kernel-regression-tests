// https://syzkaller.appspot.com/bug?id=4586f90db02397e76d91542615ed3ec3c10672f9
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <errno.h>

void write_file(const char *path, const char *val) {
    int fd = open(path, O_WRONLY);
    if (fd >= 0) {
        if (write(fd, val, strlen(val)) < 0) {
            // Ignore write errors as some files might not exist or accept the value
        }
        close(fd);
    }
}

int main() {
    printf("[+] Setting up failslab...\n");
    write_file("/sys/kernel/debug/failslab/probability", "0");
    write_file("/sys/kernel/debug/failslab/times", "-1");
    write_file("/sys/kernel/debug/failslab/verbose", "0");
    write_file("/sys/kernel/debug/failslab/ignore-gfp-wait", "N");
    write_file("/sys/kernel/debug/failslab/task-filter", "N");

    printf("[+] Binding serial8250 initially...\n");
    int fd = open("/sys/bus/platform/drivers/serial8250/bind", O_WRONLY);
    if (fd >= 0) {
        if (write(fd, "serial8250", 10) < 0) {
            // It might already be bound, which is fine.
        }
        close(fd);
    }

    printf("[+] Starting fault injection loop...\n");
    for (int n = 1; n < 200; n++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            continue;
        }
        if (pid == 0) {
            // Child process
            char path[256];
            char buf[32];
            sprintf(path, "/proc/self/task/%ld/fail-nth", syscall(SYS_gettid));
            sprintf(buf, "%d", n);
            write_file(path, buf);
            
            int fd_unbind = open("/sys/bus/platform/drivers/serial8250/unbind", O_WRONLY);
            if (fd_unbind >= 0) {
                if (write(fd_unbind, "serial8250", 10) < 0) {
                    // Expected to fail when fail-nth hits an allocation during write
                }
                close(fd_unbind);
            }
            exit(0);
        }
        int status;
        waitpid(pid, &status, 0);
        
        // Bind it back for the next iteration
        fd = open("/sys/bus/platform/drivers/serial8250/bind", O_WRONLY);
        if (fd >= 0) {
            if (write(fd, "serial8250", 10) < 0) {
                // Ignore
            }
            close(fd);
        }
    }
    
    printf("[+] Finished without crashing.\n");
    return 0;
}
