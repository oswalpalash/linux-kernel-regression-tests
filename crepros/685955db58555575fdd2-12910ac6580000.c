// https://syzkaller.appspot.com/bug?id=6a24b7af770ce25b89df7e7c570da88bf42912ab
#define _GNU_SOURCE
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>
#include <errno.h>

static volatile int stop = 0;

static void *read_pipe(void *arg)
{
    int fd = open("/sys/kernel/tracing/trace_pipe", O_RDONLY);
    if (fd < 0) {
        printf("[-] Failed to open trace_pipe in thread: %s\n", strerror(errno));
        return NULL;
    }
    char buf[4096];
    while (!stop) {
        read(fd, buf, sizeof(buf));
    }
    close(fd);
    return NULL;
}

static void *write_size(void *arg)
{
    int fd = open("/sys/kernel/tracing/buffer_subbuf_size_kb", O_WRONLY);
    if (fd < 0) {
        printf("[-] Failed to open buffer_subbuf_size_kb in thread: %s\n", strerror(errno));
        return NULL;
    }
    while (!stop) {
        if (write(fd, "4\n", 2) < 0) {
            // Ignore errors, might be busy
        }
        if (write(fd, "8\n", 2) < 0) {
            // Ignore errors
        }
    }
    close(fd);
    return NULL;
}

static void *do_syscalls(void *arg)
{
    while (!stop) {
        syscall(SYS_getpid);
    }
    return NULL;
}

int main(void)
{
    int fd;

    printf("[*] Setting up tracing...\n");

    /* Enable all events to ensure the ring buffer is flooded with data */
    fd = open("/sys/kernel/tracing/events/enable", O_WRONLY);
    if (fd >= 0) {
        if (write(fd, "1", 1) < 0) {
            printf("[-] Failed to write to events/enable: %s\n", strerror(errno));
        } else {
            printf("[+] Enabled all tracing events.\n");
        }
        close(fd);
    } else {
        printf("[-] Failed to open events/enable: %s\n", strerror(errno));
        exit(1);
    }

    printf("[*] Starting threads...\n");

    pthread_t t1, t2, t3, t4, t5;
    
    if (pthread_create(&t1, NULL, read_pipe, NULL) != 0) {
        printf("[-] Failed to create thread 1\n");
        exit(1);
    }
    if (pthread_create(&t2, NULL, write_size, NULL) != 0) {
        printf("[-] Failed to create thread 2\n");
        exit(1);
    }
    if (pthread_create(&t3, NULL, do_syscalls, NULL) != 0) {
        printf("[-] Failed to create thread 3\n");
        exit(1);
    }
    if (pthread_create(&t4, NULL, read_pipe, NULL) != 0) {
        printf("[-] Failed to create thread 4\n");
        exit(1);
    }
    if (pthread_create(&t5, NULL, write_size, NULL) != 0) {
        printf("[-] Failed to create thread 5\n");
        exit(1);
    }

    printf("[+] Threads running. Waiting for 5 seconds to trigger race...\n");
    sleep(5);
    
    stop = 1;
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);
    pthread_join(t5, NULL);

    printf("[+] Done. If no crash occurred, the race was not triggered.\n");

    /* Disable events */
    fd = open("/sys/kernel/tracing/events/enable", O_WRONLY);
    if (fd >= 0) {
        if (write(fd, "0", 1) < 0) {
            printf("[-] Failed to write to events/enable to disable: %s\n", strerror(errno));
        }
        close(fd);
    }

    return 0;
}
