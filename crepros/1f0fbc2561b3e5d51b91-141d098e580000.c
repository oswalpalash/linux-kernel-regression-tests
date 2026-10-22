// https://syzkaller.appspot.com/bug?id=fc0bb3cb0c2024065682ab02ec9cde73867433fe
#define _GNU_SOURCE
#include <errno.h>
#include <linux/reboot.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <unistd.h>

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("[*] Starting reproducer for reboot self-deadlock...\n");

    int res = prctl(PR_SET_NAME, "syz.3.5755");
    if (res < 0) {
        printf("[-] Failed to prctl(PR_SET_NAME): %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] Successfully set process name to syz.3.5755\n");

    printf("[*] Invoking reboot syscall with LINUX_REBOOT_CMD_SW_SUSPEND...\n");
    /*
     * Invoking reboot with LINUX_REBOOT_CMD_SW_SUSPEND causes kernel/reboot.c
     * to acquire system_transition_mutex and invoke hibernate().
     * hibernate() calls lock_system_sleep(), which sets PF_NOFREEZE on current
     * and attempts to acquire system_transition_mutex again.
     * Because struct mutex is non-recursive, the calling task immediately
     * self-deadlocks in TASK_UNINTERRUPTIBLE (state:D), triggering the hung
     * task detector.
     */
    res = syscall(SYS_reboot, LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2,
                  LINUX_REBOOT_CMD_SW_SUSPEND, NULL);
    if (res < 0) {
        printf("[-] Failed to execute reboot syscall: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] reboot syscall returned unexpectedly with code %d\n", res);

    return 0;
}
