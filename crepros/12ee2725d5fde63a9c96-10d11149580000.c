// https://syzkaller.appspot.com/bug?id=5c0a2aab45ce84b942f3374e81cff9b62f9680a6
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>

char cg_path[512];

int write_file(const char *path, const char *val) {
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        return -1;
    }
    ssize_t len = strlen(val);
    ssize_t ret = write(fd, val, len);
    close(fd);
    if (ret != len) {
        return -1;
    }
    return 0;
}

void protect_main_process() {
    int res = write_file("/proc/self/oom_score_adj", "-1000");
    if (res < 0) {
        printf("[-] Failed to write_file /proc/self/oom_score_adj: %s\n", strerror(errno));
    } else {
        printf("[+] write_file /proc/self/oom_score_adj successful.\n");
    }
}

int setup_cgroup() {
    cg_path[0] = '\0';
    
    FILE *f = fopen("/proc/mounts", "r");
    if (f) {
        char line[1024];
        char dev[256], mnt[256], type[256], opts[512];
        while (fgets(line, sizeof(line), f)) {
            if (sscanf(line, "%255s %255s %255s %511s", dev, mnt, type, opts) == 4) {
                if (strcmp(type, "cgroup") == 0 && strstr(opts, "memory")) {
                    snprintf(cg_path, sizeof(cg_path), "%s/test_memcg_%d", mnt, getpid());
                    break;
                }
            }
        }
        fclose(f);
    }

    if (cg_path[0] == '\0') {
        int res = mkdir("/tmp/memcg", 0777);
        if (res < 0 && errno != EEXIST) {
            printf("[-] Failed to mkdir /tmp/memcg: %s\n", strerror(errno));
        }
        res = mount("cgroup", "/tmp/memcg", "cgroup", 0, "memory");
        if (res == 0) {
            snprintf(cg_path, sizeof(cg_path), "/tmp/memcg/test_memcg_%d", getpid());
        } else {
            snprintf(cg_path, sizeof(cg_path), "/sys/fs/cgroup/memory/test_memcg_%d", getpid());
        }
    }

    int res = mkdir(cg_path, 0777);
    if (res < 0 && errno != EEXIST) {
        printf("[-] Failed to mkdir %s: %s\n", cg_path, strerror(errno));
        return -1;
    }
    printf("[+] mkdir %s successful.\n", cg_path);
    
    char path[768];
    snprintf(path, sizeof(path), "%s/memory.soft_limit_in_bytes", cg_path);
    res = write_file(path, "1048576");
    if (res < 0) {
        printf("[-] Failed to write_file soft limit: %s\n", strerror(errno));
        return -1;
    }
    printf("[+] write_file soft limit successful.\n");
    
    char pid_str[32];
    snprintf(pid_str, sizeof(pid_str), "%d", getpid());
    snprintf(path, sizeof(path), "%s/cgroup.procs", cg_path);
    res = write_file(path, pid_str);
    if (res < 0) {
        snprintf(path, sizeof(path), "%s/tasks", cg_path);
        res = write_file(path, pid_str);
        if (res < 0) {
            printf("[-] Failed to write_file cgroup.procs/tasks: %s\n", strerror(errno));
            return -1;
        }
    }
    printf("[+] write_file cgroup.procs/tasks successful.\n");
    return 0;
}

void disable_mglru() {
    int res = write_file("/sys/kernel/mm/lru_gen/enabled", "0");
    if (res < 0) {
        printf("[-] Failed to write_file disable MGLRU: %s\n", strerror(errno));
    } else {
        printf("[+] write_file disable MGLRU successful.\n");
    }
}

void *toggle_mglru(void *arg) {
    int fd = open("/sys/kernel/mm/lru_gen/enabled", O_WRONLY);
    if (fd < 0) {
        printf("[-] Failed to open lru_gen/enabled in thread: %s\n", strerror(errno));
        return NULL;
    }
    printf("[+] open lru_gen/enabled in thread successful.\n");
    
    while (1) {
        lseek(fd, 0, SEEK_SET);
        write(fd, "0x0007\n", 7);
        lseek(fd, 0, SEEK_SET);
        write(fd, "0\n", 2);
    }
    return NULL;
}

void *memory_pressure_thread(void *arg) {
    while (1) {
        pid_t pid = fork();
        if (pid < 0) {
            sleep(1);
            continue;
        }
        if (pid == 0) {
            write_file("/proc/self/oom_score_adj", "1000");
            
            char path[768];
            snprintf(path, sizeof(path), "%s/../cgroup.procs", cg_path);
            char pid_str[32];
            snprintf(pid_str, sizeof(pid_str), "%d", getpid());
            if (write_file(path, pid_str) < 0) {
                snprintf(path, sizeof(path), "%s/../tasks", cg_path);
                write_file(path, pid_str);
            }
            
            while (1) {
                size_t chunk = 64 * 1024 * 1024;
                char *ptr = mmap(NULL, chunk, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
                if (ptr != MAP_FAILED) {
                    for (size_t i = 0; i < chunk; i += 4096) ptr[i] = 1;
                } else {
                    chunk = 4 * 1024 * 1024;
                    ptr = mmap(NULL, chunk, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
                    if (ptr != MAP_FAILED) {
                        for (size_t i = 0; i < chunk; i += 4096) ptr[i] = 1;
                    } else {
                        sleep(1);
                    }
                }
            }
            exit(0);
        }
        
        int status;
        waitpid(pid, &status, 0);
    }
    return NULL;
}

int main() {
    protect_main_process();
    
    int res = setup_cgroup();
    if (res < 0) {
        printf("[-] Failed to setup_cgroup\n");
        exit(1);
    }
    printf("[+] setup_cgroup successful.\n");
    
    disable_mglru();
    
    size_t alloc_size = 64 * 1024 * 1024;
    volatile char *p = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) {
        printf("[-] Failed to mmap 64MB: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] mmap 64MB successful.\n");
    
    for (size_t i = 0; i < alloc_size; i += 4096) {
        p[i] = 0x42;
    }
    printf("[+] Faulted 64MB successful.\n");
    
    pthread_t tid1, tid2;
    res = pthread_create(&tid1, NULL, toggle_mglru, NULL);
    if (res != 0) {
        printf("[-] Failed to pthread_create toggle_mglru: %s\n", strerror(res));
        exit(1);
    }
    printf("[+] pthread_create toggle_mglru successful.\n");
    
    res = pthread_create(&tid2, NULL, memory_pressure_thread, NULL);
    if (res != 0) {
        printf("[-] Failed to pthread_create memory_pressure: %s\n", strerror(res));
        exit(1);
    }
    printf("[+] pthread_create memory_pressure successful.\n");
    
    time_t start = time(NULL);
    while (time(NULL) - start < 60) {
        for (size_t i = 0; i < alloc_size; i += 4096) {
            p[i] = 0x42;
        }
        usleep(100000);
    }
    
    printf("[+] Finished successfully.\n");
    return 0;
}
