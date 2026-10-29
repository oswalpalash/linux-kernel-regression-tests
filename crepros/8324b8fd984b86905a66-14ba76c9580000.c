// https://syzkaller.appspot.com/bug?id=5684d6c411dbfeaea3adf2beb7820051fa25a337
#define _GNU_SOURCE
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <errno.h>
#include <sys/klog.h>

#define NUM_WRITERS 16
#define NUM_READERS 16
#define MSG_SIZE 1024
#define READ_BUF_SIZE 4096

long total_cpus = 1;

void pin_to_cpu(long index) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    if (sched_getaffinity(0, sizeof(cpu_set_t), &cpuset) < 0) {
        printf("[-] Failed to sched_getaffinity: %s\n", strerror(errno));
        exit(1);
    }
    
    long target_cpu = 0;
    long count = 0;
    long available = 0;
    
    for (long i = 0; i < CPU_SETSIZE; i++) {
        if (CPU_ISSET(i, &cpuset)) {
            available++;
        }
    }
    
    if (available == 0) available = 1;
    index = index % available;
    
    for (long i = 0; i < CPU_SETSIZE; i++) {
        if (CPU_ISSET(i, &cpuset)) {
            if (count == index) {
                target_cpu = i;
                break;
            }
            count++;
        }
    }
    
    CPU_ZERO(&cpuset);
    CPU_SET(target_cpu, &cpuset);
    if (sched_setaffinity(0, sizeof(cpu_set_t), &cpuset) < 0) {
        printf("[-] Failed to sched_setaffinity for CPU %ld: %s\n", target_cpu, strerror(errno));
        exit(1);
    }
}

void *writer_thread(void *arg) {
    long id = (long)arg;
    pin_to_cpu(id);

    int fd = open("/dev/kmsg", O_WRONLY);
    if (fd < 0) {
        printf("[-] Failed to open /dev/kmsg for writing: %s\n", strerror(errno));
        exit(1);
    }
    
    char *buf = malloc(MSG_SIZE);
    if (!buf) {
        printf("[-] Failed to malloc: %s\n", strerror(errno));
        exit(1);
    }
    
    memset(buf, 'A', MSG_SIZE - 1);
    buf[0] = '<';
    buf[1] = '7';
    buf[2] = '>';
    buf[MSG_SIZE - 1] = '\n';
    
    while (1) {
        if (write(fd, buf, MSG_SIZE) < 0) {
            if (errno == ENOMEM || errno == EAGAIN || errno == EINTR) {
                usleep(100);
            } else {
                printf("[-] Failed to write to /dev/kmsg: %s\n", strerror(errno));
                exit(1);
            }
        }
    }
    
    free(buf);
    close(fd);
    return NULL;
}

void *reader_thread(void *arg) {
    long id = (long)arg;
    pin_to_cpu(id + total_cpus / 2);

    char *buf = malloc(READ_BUF_SIZE);
    if (!buf) {
        printf("[-] Failed to malloc: %s\n", strerror(errno));
        exit(1);
    }
    
    while (1) {
        // SYSLOG_ACTION_READ_ALL = 3
        if (klogctl(3, buf, READ_BUF_SIZE) < 0) {
            if (errno == EINTR || errno == EAGAIN || errno == EINVAL) {
                continue;
            }
            // Ignore errors to keep reading
        }
    }
    
    free(buf);
    return NULL;
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("[*] Starting reproducer...\n");

    int fd = open("/proc/sys/kernel/printk_devkmsg", O_WRONLY);
    if (fd < 0) {
        printf("[-] Failed to open /proc/sys/kernel/printk_devkmsg: %s\n", strerror(errno));
    } else {
        if (write(fd, "on\n", 3) < 0) {
            printf("[-] Failed to write to printk_devkmsg: %s\n", strerror(errno));
        } else {
            printf("[+] write to printk_devkmsg successful.\n");
        }
        close(fd);
    }

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    if (sched_getaffinity(0, sizeof(cpu_set_t), &cpuset) == 0) {
        total_cpus = 0;
        for (long i = 0; i < CPU_SETSIZE; i++) {
            if (CPU_ISSET(i, &cpuset)) {
                total_cpus++;
            }
        }
    }
    if (total_cpus < 1) total_cpus = 1;

    pthread_t writers[NUM_WRITERS];
    pthread_t readers[NUM_READERS];
    
    for (long i = 0; i < NUM_WRITERS; i++) {
        int ret = pthread_create(&writers[i], NULL, writer_thread, (void *)i);
        if (ret != 0) {
            printf("[-] Failed to create writer thread %ld: %s\n", i, strerror(ret));
            exit(1);
        }
    }
    printf("[+] Writer threads created.\n");
    
    for (long i = 0; i < NUM_READERS; i++) {
        int ret = pthread_create(&readers[i], NULL, reader_thread, (void *)i);
        if (ret != 0) {
            printf("[-] Failed to create reader thread %ld: %s\n", i, strerror(ret));
            exit(1);
        }
    }
    printf("[+] Reader threads created.\n");
    
    printf("[*] Running for 60 seconds...\n");
    sleep(60);
    printf("[+] Done.\n");
    
    return 0;
}