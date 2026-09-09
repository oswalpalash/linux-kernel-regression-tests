// https://syzkaller.appspot.com/bug?id=33dc6af458554c37677c3b42a0728376c930fb3c
#define _GNU_SOURCE
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdatomic.h>
#include <errno.h>
#include <sys/syscall.h>
#include <linux/falloc.h>

#define THP_SIZE (2 * 1024 * 1024)
#define PAGE_SIZE 4096

atomic_int state = 0; // 0: wait, 1: run, 2: exit
char *fault_addr = NULL;
int tmp_fd;

void *fault_thread(void *arg) {
    while (1) {
        while (atomic_load(&state) == 0) {}
        if (atomic_load(&state) == 2) break;
        
        char *p = fault_addr;
        while (atomic_load(&state) == 1) {
            // Trigger do_fault_around -> filemap_map_pages
            // Use pwrite to safely read from p. If p is unmapped, it returns -EFAULT instead of SIGSEGV.
            pwrite(tmp_fd, p, 1, 0);
            
            // Clear the PTE to force a new fault on the next iteration
            madvise((void*)p, 16 * PAGE_SIZE, MADV_DONTNEED);
        }
    }
    return NULL;
}

int main() {
    tmp_fd = syscall(SYS_memfd_create, "tmp", 0);
    if (tmp_fd < 0) {
        printf("[-] Failed to memfd_create tmp: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] memfd_create tmp successful.\n");

    // Allocate a 4MB region to find a 2MB aligned virtual address
    void *dummy = mmap(NULL, 4 * 1024 * 1024, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (dummy == MAP_FAILED) {
        printf("[-] Failed to mmap dummy: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] mmap dummy successful.\n");
    
    // addr has PTE index 0
    unsigned long addr = ((unsigned long)dummy + 2 * 1024 * 1024 - 1) & ~(2 * 1024 * 1024 - 1);

    pthread_t t[4];
    for (int i = 0; i < 4; i++) {
        int res = pthread_create(&t[i], NULL, fault_thread, NULL);
        if (res != 0) {
            printf("[-] Failed to pthread_create: %s\n", strerror(res));
            exit(1);
        }
    }
    printf("[+] pthread_create successful.\n");

    for (int i = 0; i < 50000; i++) {
        int fd = syscall(SYS_memfd_create, "test", 0);
        if (fd < 0) {
            printf("[-] Failed to memfd_create: %s\n", strerror(errno));
            exit(1);
        }
        
        if (ftruncate(fd, THP_SIZE) < 0) {
            printf("[-] Failed to ftruncate: %s\n", strerror(errno));
            exit(1);
        }
        
        // Allocate a THP at index 0
        char *p_huge = mmap(NULL, THP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (p_huge != MAP_FAILED) {
            madvise(p_huge, THP_SIZE, MADV_HUGEPAGE);
            memset(p_huge, 1, THP_SIZE); // Fault it in
        } else {
            printf("[-] Failed to mmap p_huge: %s\n", strerror(errno));
            exit(1);
        }
        
        // Map a 64KB region at offset 256 pages (1MB) to the 2MB aligned address
        // This forces start_pgoff = 256, and vmf->pte to be at index 0 of the page table
        char *p = mmap((void*)addr, 16 * PAGE_SIZE, PROT_READ, MAP_SHARED | MAP_FIXED, fd, 256 * PAGE_SIZE);
        if (p != MAP_FAILED) {
            fault_addr = p;
            atomic_store(&state, 1); // Start faulting
            
            // Variable delay to sweep the race window
            for (volatile int j = 0; j < (i % 100) * 10; j++) {}
            
            // Split the THP concurrently
            // Punching a hole at page 0 forces the THP to be split.
            // If the fault thread is in next_uptodate_folio(), xas->xa_index will jump backwards to 64.
            int res = fallocate(fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, 0, PAGE_SIZE);
            if (res < 0) {
                printf("[-] Failed to fallocate: %s\n", strerror(errno));
                exit(1);
            }
            
            atomic_store(&state, 0); // Stop faulting
            munmap(p, 16 * PAGE_SIZE);
        } else {
            printf("[-] Failed to mmap p: %s\n", strerror(errno));
            exit(1);
        }
        
        if (p_huge != MAP_FAILED) {
            munmap(p_huge, THP_SIZE);
        }
        close(fd);
        
        if (i % 1000 == 0) {
            printf("[+] Iteration %d\n", i);
        }
    }
    
    atomic_store(&state, 2);
    for (int i = 0; i < 4; i++) {
        pthread_join(t[i], NULL);
    }
    printf("[+] Reproducer finished.\n");
    return 0;
}