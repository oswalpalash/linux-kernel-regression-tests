// https://syzkaller.appspot.com/bug?id=f2866fbf0de3dbad4d562cffb405940ad3a3f461
#define _GNU_SOURCE
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/dvb/dmx.h>
#include <dirent.h>
#include <errno.h>

void *unbind_thread(void *arg)
{
    // Wait for the main thread to set up the filter
    usleep(200000);
    
    DIR *dir = opendir("/sys/bus/platform/drivers/vidtv");
    if (!dir) {
        printf("[-] Failed to open vidtv driver dir: %s\n", strerror(errno));
        return NULL;
    }
    
    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        // Check if it's a device directory (not . or .. or module, etc.)
        if (ent->d_name[0] == '.' || strcmp(ent->d_name, "bind") == 0 || 
            strcmp(ent->d_name, "unbind") == 0 || strcmp(ent->d_name, "uevent") == 0 ||
            strcmp(ent->d_name, "module") == 0)
            continue;
            
        printf("[+] Found device to unbind: %s\n", ent->d_name);
        int fd = open("/sys/bus/platform/drivers/vidtv/unbind", O_WRONLY);
        if (fd >= 0) {
            if (write(fd, ent->d_name, strlen(ent->d_name)) < 0) {
                printf("[-] Failed to unbind %s: %s\n", ent->d_name, strerror(errno));
            } else {
                printf("[+] Successfully unbound %s\n", ent->d_name);
            }
            close(fd);
        }
    }
    closedir(dir);
    return NULL;
}

int main(void)
{
    int fds[16];
    int num_fds = 0;
    char path[256];

    // Open all available demux devices and start a feed on them
    for (int i = 0; i < 16; i++) {
        snprintf(path, sizeof(path), "/dev/dvb/adapter%d/demux0", i);
        int fd = open(path, O_RDWR);
        if (fd >= 0) {
            fds[num_fds++] = fd;
            
            struct dmx_pes_filter_params params;
            memset(&params, 0, sizeof(params));
            params.pid = 0x100 + i;
            params.input = DMX_IN_FRONTEND;
            params.output = DMX_OUT_TAP;
            params.pes_type = DMX_PES_OTHER;
            params.flags = DMX_IMMEDIATE_START;
            
            if (ioctl(fd, DMX_SET_PES_FILTER, &params) < 0) {
                printf("[-] ioctl DMX_SET_PES_FILTER failed on %s: %s\n", path, strerror(errno));
            } else {
                printf("[+] Started feed on %s\n", path);
            }
        }
    }

    if (num_fds == 0) {
        printf("[-] No demux devices found\n");
        exit(1);
    }

    pthread_t th;
    if (pthread_create(&th, NULL, unbind_thread, NULL) != 0) {
        printf("[-] Failed to create thread: %s\n", strerror(errno));
        exit(1);
    }

    // Wait for the unbind thread to destroy the mutex
    usleep(500000);

    // Closing the file descriptor will trigger dvb_demux_release() -> ... -> vidtv_stop_feed()
    // which will attempt to lock the destroyed mutex, triggering the warning.
    for (int i = 0; i < num_fds; i++) {
        if (close(fds[i]) < 0) {
            printf("[-] Failed to close fd: %s\n", strerror(errno));
        } else {
            printf("[+] Closed fd\n");
        }
    }

    pthread_join(th, NULL);
    
    // Wait for any asynchronous tasks to complete
    usleep(500000);

    printf("[+] Done.\n");
    return 0;
}