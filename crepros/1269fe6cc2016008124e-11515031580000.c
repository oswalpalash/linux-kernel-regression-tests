// https://syzkaller.appspot.com/bug?id=e576484501f89c134e721120ed94ed7b99f67104
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("[*] Starting reproducer for netlink_has_listeners null-ptr-deref...\n");

    /* Step 1: Ensure debugfs is mounted */
    int res = mkdir("/sys/kernel/debug", 0755);
    if (res < 0 && errno != EEXIST) {
        printf("[-] Failed to mkdir /sys/kernel/debug: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] mkdir /sys/kernel/debug check passed.\n");

    res = mount("none", "/sys/kernel/debug", "debugfs", 0, NULL);
    if (res < 0 && errno != EBUSY) {
        printf("[-] Failed to mount debugfs: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] debugfs mounted or already present.\n");

    /* Step 2: Clean up any pre-existing netdevsim10 device */
    if (access("/sys/bus/netdevsim/devices/netdevsim10", F_OK) == 0) {
        printf("[*] Pre-existing netdevsim10 found, removing...\n");
        int fd_clean = open("/sys/bus/netdevsim/del_device", O_WRONLY);
        if (fd_clean < 0) {
            printf("[-] Failed to open /sys/bus/netdevsim/del_device for cleanup: %s\n", strerror(errno));
            exit(1);
        }
        ssize_t w_clean = write(fd_clean, "10\n", 3);
        if (w_clean < 0) {
            printf("[-] Failed to write to del_device for cleanup: %s\n", strerror(errno));
            exit(1);
        }
        res = close(fd_clean);
        if (res < 0) {
            printf("[-] Failed to close /sys/bus/netdevsim/del_device after cleanup: %s\n", strerror(errno));
            exit(1);
        }
        printf("[+] Cleaned up pre-existing netdevsim10.\n");
    }

    /* Step 3: Fork a child process to create device inside an isolated network namespace */
    pid_t pid = fork();
    if (pid < 0) {
        printf("[-] Failed to fork child process: %s\n", strerror(errno));
        exit(1);
    }

    if (pid == 0) {
        /* In child process: unshare network namespace */
        res = unshare(CLONE_NEWNET);
        if (res < 0) {
            printf("[-] Failed to unshare CLONE_NEWNET in child: %s\n", strerror(errno));
            exit(1);
        }
        printf("[+] Child unshared CLONE_NEWNET.\n");

        /* Create netdevsim device with ID 10 and 1 port inside child netns */
        int fd_new = open("/sys/bus/netdevsim/new_device", O_WRONLY);
        if (fd_new < 0) {
            printf("[-] Child failed to open /sys/bus/netdevsim/new_device: %s\n", strerror(errno));
            exit(1);
        }
        ssize_t w_new = write(fd_new, "10 1\n", 5);
        if (w_new < 0) {
            printf("[-] Child failed to write to /sys/bus/netdevsim/new_device: %s\n", strerror(errno));
            exit(1);
        }
        res = close(fd_new);
        if (res < 0) {
            printf("[-] Child failed to close /sys/bus/netdevsim/new_device: %s\n", strerror(errno));
            exit(1);
        }
        printf("[+] Netdevsim device 10 created in child netns.\n");

        /* Inhibit devlink reload so devlink instance remains bound to child netns during exit */
        int fd_reload = open("/sys/kernel/debug/netdevsim/netdevsim10/dont_allow_reload", O_WRONLY);
        if (fd_reload < 0) {
            printf("[-] Child failed to open dont_allow_reload debugfs file: %s\n", strerror(errno));
            exit(1);
        }
        ssize_t w_reload = write(fd_reload, "1\n", 2);
        if (w_reload < 0) {
            printf("[-] Child failed to write to dont_allow_reload debugfs file: %s\n", strerror(errno));
            exit(1);
        }
        res = close(fd_reload);
        if (res < 0) {
            printf("[-] Child failed to close dont_allow_reload debugfs file: %s\n", strerror(errno));
            exit(1);
        }
        printf("[+] Configured dont_allow_reload=1 for netdevsim10.\n");

        /* Child exits to trigger netns cleanup */
        printf("[+] Child exiting now to initiate netns destruction.\n");
        exit(0);
    }

    /* In parent process: wait for child to exit */
    int status = 0;
    res = waitpid(pid, &status, 0);
    if (res < 0) {
        printf("[-] Failed to waitpid for child: %s\n", strerror(errno));
        exit(1);
    }
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        printf("[-] Child process terminated abnormally with status %d\n", status);
        exit(1);
    }
    printf("[+] Child process exited cleanly.\n");

    /* Step 4: Allow cleanup_net workqueue to dismantle child netns (clearing net->genl_sock) */
    printf("[*] Sleeping 1 second to allow cleanup_net workqueue to process child netns...\n");
    sleep(1);
    printf("[+] Sleep completed.\n");

    /* Step 5: Delete netdevsim10 from parent; this invokes unregister_netdevice on the port,
     * triggering devlink_port_notify -> genl_has_listeners() on the dead netns where
     * net->genl_sock is NULL, causing the KASAN null-ptr-deref in netlink_has_listeners.
     */
    printf("[*] Deleting netdevsim10 to trigger devlink port notification into dead netns...\n");
    int fd_del = open("/sys/bus/netdevsim/del_device", O_WRONLY);
    if (fd_del < 0) {
        printf("[-] Failed to open /sys/bus/netdevsim/del_device: %s\n", strerror(errno));
        exit(1);
    }
    ssize_t w_del = write(fd_del, "10\n", 3);
    if (w_del < 0) {
        printf("[-] Failed to write to /sys/bus/netdevsim/del_device: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] Device deleted.\n");

    res = close(fd_del);
    if (res < 0) {
        printf("[-] Failed to close /sys/bus/netdevsim/del_device: %s\n", strerror(errno));
        exit(1);
    }

    sleep(1);
    printf("[+] Reproducer finished without crashing (bug might be fixed).\n");
    return 0;
}