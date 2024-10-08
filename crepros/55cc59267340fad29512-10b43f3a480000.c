// https://syzkaller.appspot.com/bug?id=598548102af902f0c5bec39164f1b7b58b8b81c0
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#ifndef __NR_io_uring_enter
#define __NR_io_uring_enter 426
#endif
#ifndef __NR_io_uring_setup
#define __NR_io_uring_setup 425
#endif

static void sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
}

static uint64_t current_time_ms(void)
{
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    exit(1);
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static bool write_file(const char* file, const char* what, ...)
{
  char buf[1024];
  va_list args;
  va_start(args, what);
  vsnprintf(buf, sizeof(buf), what, args);
  va_end(args);
  buf[sizeof(buf) - 1] = 0;
  int len = strlen(buf);
  int fd = open(file, O_WRONLY | O_CLOEXEC);
  if (fd == -1)
    return false;
  if (write(fd, buf, len) != len) {
    int err = errno;
    close(fd);
    errno = err;
    return false;
  }
  close(fd);
  return true;
}

#define SIZEOF_IO_URING_SQE 64
#define SIZEOF_IO_URING_CQE 16
#define SQ_HEAD_OFFSET 0
#define SQ_TAIL_OFFSET 64
#define SQ_RING_MASK_OFFSET 256
#define SQ_RING_ENTRIES_OFFSET 264
#define SQ_FLAGS_OFFSET 276
#define SQ_DROPPED_OFFSET 272
#define CQ_HEAD_OFFSET 128
#define CQ_TAIL_OFFSET 192
#define CQ_RING_MASK_OFFSET 260
#define CQ_RING_ENTRIES_OFFSET 268
#define CQ_RING_OVERFLOW_OFFSET 284
#define CQ_FLAGS_OFFSET 280
#define CQ_CQES_OFFSET 320

struct io_sqring_offsets {
  uint32_t head;
  uint32_t tail;
  uint32_t ring_mask;
  uint32_t ring_entries;
  uint32_t flags;
  uint32_t dropped;
  uint32_t array;
  uint32_t resv1;
  uint64_t resv2;
};

struct io_cqring_offsets {
  uint32_t head;
  uint32_t tail;
  uint32_t ring_mask;
  uint32_t ring_entries;
  uint32_t overflow;
  uint32_t cqes;
  uint64_t resv[2];
};

struct io_uring_params {
  uint32_t sq_entries;
  uint32_t cq_entries;
  uint32_t flags;
  uint32_t sq_thread_cpu;
  uint32_t sq_thread_idle;
  uint32_t features;
  uint32_t resv[4];
  struct io_sqring_offsets sq_off;
  struct io_cqring_offsets cq_off;
};

#define IORING_OFF_SQ_RING 0
#define IORING_OFF_SQES 0x10000000ULL

static long syz_io_uring_setup(volatile long a0, volatile long a1,
                               volatile long a2, volatile long a3,
                               volatile long a4, volatile long a5)
{
  uint32_t entries = (uint32_t)a0;
  struct io_uring_params* setup_params = (struct io_uring_params*)a1;
  void* vma1 = (void*)a2;
  void* vma2 = (void*)a3;
  void** ring_ptr_out = (void**)a4;
  void** sqes_ptr_out = (void**)a5;
  uint32_t fd_io_uring = syscall(__NR_io_uring_setup, entries, setup_params);
  uint32_t sq_ring_sz =
      setup_params->sq_off.array + setup_params->sq_entries * sizeof(uint32_t);
  uint32_t cq_ring_sz = setup_params->cq_off.cqes +
                        setup_params->cq_entries * SIZEOF_IO_URING_CQE;
  uint32_t ring_sz = sq_ring_sz > cq_ring_sz ? sq_ring_sz : cq_ring_sz;
  *ring_ptr_out = mmap(vma1, ring_sz, PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_POPULATE | MAP_FIXED, fd_io_uring,
                       IORING_OFF_SQ_RING);
  uint32_t sqes_sz = setup_params->sq_entries * SIZEOF_IO_URING_SQE;
  *sqes_ptr_out =
      mmap(vma2, sqes_sz, PROT_READ | PROT_WRITE,
           MAP_SHARED | MAP_POPULATE | MAP_FIXED, fd_io_uring, IORING_OFF_SQES);
  return fd_io_uring;
}

static long syz_io_uring_submit(volatile long a0, volatile long a1,
                                volatile long a2, volatile long a3)
{
  char* ring_ptr = (char*)a0;
  char* sqes_ptr = (char*)a1;
  char* sqe = (char*)a2;
  uint32_t sqes_index = (uint32_t)a3;
  uint32_t sq_ring_entries = *(uint32_t*)(ring_ptr + SQ_RING_ENTRIES_OFFSET);
  uint32_t cq_ring_entries = *(uint32_t*)(ring_ptr + CQ_RING_ENTRIES_OFFSET);
  uint32_t sq_array_off =
      (CQ_CQES_OFFSET + cq_ring_entries * SIZEOF_IO_URING_CQE + 63) & ~63;
  if (sq_ring_entries)
    sqes_index %= sq_ring_entries;
  char* sqe_dest = sqes_ptr + sqes_index * SIZEOF_IO_URING_SQE;
  memcpy(sqe_dest, sqe, SIZEOF_IO_URING_SQE);
  uint32_t sq_ring_mask = *(uint32_t*)(ring_ptr + SQ_RING_MASK_OFFSET);
  uint32_t* sq_tail_ptr = (uint32_t*)(ring_ptr + SQ_TAIL_OFFSET);
  uint32_t sq_tail = *sq_tail_ptr & sq_ring_mask;
  uint32_t sq_tail_next = *sq_tail_ptr + 1;
  uint32_t* sq_array = (uint32_t*)(ring_ptr + sq_array_off);
  *(sq_array + sq_tail) = sqes_index;
  __atomic_store_n(sq_tail_ptr, sq_tail_next, __ATOMIC_RELEASE);
  return 0;
}

static void kill_and_wait(int pid, int* status)
{
  kill(-pid, SIGKILL);
  kill(pid, SIGKILL);
  for (int i = 0; i < 100; i++) {
    if (waitpid(-1, status, WNOHANG | __WALL) == pid)
      return;
    usleep(1000);
  }
  DIR* dir = opendir("/sys/fs/fuse/connections");
  if (dir) {
    for (;;) {
      struct dirent* ent = readdir(dir);
      if (!ent)
        break;
      if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        continue;
      char abort[300];
      snprintf(abort, sizeof(abort), "/sys/fs/fuse/connections/%s/abort",
               ent->d_name);
      int fd = open(abort, O_WRONLY);
      if (fd == -1) {
        continue;
      }
      if (write(fd, abort, 1) < 0) {
      }
      close(fd);
    }
    closedir(dir);
  } else {
  }
  while (waitpid(-1, status, __WALL) != pid) {
  }
}

static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  write_file("/proc/self/oom_score_adj", "1000");
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter = 0;
  for (;; iter++) {
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      setup_test();
      execute_one();
      exit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      if (waitpid(-1, &status, WNOHANG | WAIT_FLAGS) == pid)
        break;
      sleep_ms(1);
      if (current_time_ms() - start < 5000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[5] = {0xffffffffffffffff, 0x0, 0x0, 0xffffffffffffffff,
                 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  *(uint32_t*)0x20000244 = 0;
  *(uint32_t*)0x20000248 = 0;
  *(uint32_t*)0x2000024c = 0;
  *(uint32_t*)0x20000250 = 0;
  *(uint32_t*)0x20000258 = -1;
  memset((void*)0x2000025c, 0, 12);
  res = -1;
  res = syz_io_uring_setup(0x3ede, 0x20000240, 0x20002000, 0x20ffe000,
                           0x20000100, 0x20000140);
  if (res != -1) {
    r[0] = res;
    r[1] = *(uint64_t*)0x20000100;
    r[2] = *(uint64_t*)0x20000140;
  }
  res = syscall(__NR_socket, 0x26ul, 5ul, 0);
  if (res != -1)
    r[3] = res;
  *(uint16_t*)0x20000180 = 0x26;
  memcpy((void*)0x20000182, "skcipher\000\000\000\000\000\000", 14);
  *(uint32_t*)0x20000190 = 0;
  *(uint32_t*)0x20000194 = 0;
  memcpy((void*)0x20000198,
         "cbc(blowfish)"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000",
         64);
  syscall(__NR_bind, r[3], 0x20000180ul, 0x58ul);
  memcpy((void*)0x20c18000, "\xad\x56\xb6\xc5", 4);
  syscall(__NR_setsockopt, r[3], 0x117, 1, 0x20c18000ul, 4ul);
  res = syscall(__NR_accept, r[3], 0ul, 0ul);
  if (res != -1)
    r[4] = res;
  *(uint64_t*)0x20002600 = 0;
  *(uint32_t*)0x20002608 = 0;
  *(uint64_t*)0x20002610 = 0x20002540;
  *(uint64_t*)0x20002540 = 0x20001540;
  memcpy(
      (void*)0x20001540,
      "\x2d\xa6\x4c\x9a\x8a\xa9\x94\x16\x46\x5d\x01\xdc\x86\x35\x31\xc6\x5e\x48"
      "\x4b\xe6\x6c\x0f\x27\xfb\x9b\x59\x61\x8b\xe3\xf6\xbe\xc8\x75\xc6\x0e\x72"
      "\xa5\x89\x59\x69\x36\x79\xf5\x4c\xfc\x4c\x7e\xb2\x97\x02\x8b\x49\x18\x93"
      "\xa9\x7a\xce\xf6\xd4\x8c\x8d\xe8\xd5\x7c\x7e\xc3\xc4\x17\x10\xeb\xfa\x75"
      "\x8f\x7b\xe7\x15\x4b\x83\x49\x47\x56\x20\x46\x96\x2e\x8c\xb6\x49\x93\xe6"
      "\x5e\x6e\xe4\xb8\xc9\x82\xa9\x28\x3b\xc2\xa2\x55\x02\xc7\xc4\x09\x61\x9f"
      "\xb8\xdd\xa6\x75\xb4\x0e\x23\x13\x48\x56\x9b\x0d\x16\x45\x87\x90\x89\x9a"
      "\xc5\xc1\xa9\xbb\x4f\x98\xc9\x1c\x45\xa6\x5c\x5d\x60\x8d\x42\x49\x4e\x0d"
      "\xf0\x4d\x37\x67\x54\x05\xe2\x8e\xb7\x70\x88\xf3\x68\x68\x78\x2f\xac\xc8"
      "\xb2\x00\xbd\x30\xf9\xd0\xd3\xf5\x94\x7b\xc0\x8b\x34\x2e\x80\x89\xd2\xcc"
      "\x54\xa3\xc0\x2e\x11\x57\xe3\x6b\x92\xdf\x28\x68\xa8\x1a\x7d\x9d\x3e\x55"
      "\x37\x56\x1c\x26\x33\xa0\xc0\x14\x19\x63\x96\x59\x35\x31\xe9\xe1\xa3\x7f"
      "\xba\x0d\x66\xc0\xd3\x73\x7f\x4c\x26\x8e\xc7\xf5\x12\x79\x5f\x17\x16\x70"
      "\xe2\x34\x3c\xb0\x9b\xf6\x38\x71\x09\x72\xd4\x2f\xc5\xc3\x20\x90\x3f\x46"
      "\x05\x79\x42\xb8\xad\xf1\x98\x5f\x80\x5c\xe9\x79\x97\xbc\x56\xf4\xea\x7e"
      "\x84\xb7\x01\x70\x46\xfb\x8b\xf5\xef\xb8\x9e\x1d\xb8\x0e\xa0\xa8\x64\x44"
      "\x0e\xb6\x90\xce\x32\xb1\x71\x7e\x4f\xac\xa7\x9e\x33\xe6\x11\x8a\xad\xb7"
      "\x72\x73\x11\x4e\xc6\x50\x79\x34\x20\x60\xc0\xd2\x39\x14\x10\x11\xb3\x04"
      "\x46\x3f\x7c\x84\x75\xe0\xbd\x84\x0c\x06\xb0\xbc\x40\xda\x9f\x04\x90\x71"
      "\xeb\x28\x38\x2f\xad\xf8\x02\x21\x99\x25\x06\xb0\x78\xa5\x0c\x4d\x62\x30"
      "\x52\x2e\xda\x0c\x0e\x01\x3c\x0e\xd8\x9b\x00\xc6\x25\x4f\xc6\x38\x04\xef"
      "\x6f\x1b\xe1\xbb\xf1\x8a\xcc\x02\xd1\x16\x7a\x9a\x46\xdd\x89\x2e\x24\x90"
      "\x76\xc8\xc7\x32\x7b\x6a\xda\xe9\x7b\xe1\x2b\xe3\x4c\xb4\x59\x26\x01\x8f"
      "\x54\x53\x40\x51\xac\x3c\x38\xf6\x2d\xcb\xf7\x62\x89\x73\x8a\x15\x65\x24"
      "\x94\xdb\xd7\x15\x68\xff\xae\xfd\xd6\x29\x34\xaf\x1f\x03\x29\x6f\x83\x48"
      "\xa1\xfb\xff\x90\x7f\x56\xaf\xb0\x32\x73\xc1\xdc\x3d\x2f\x8d\x9d\x07\x55"
      "\x48\x8f\x75\xe6\x70\x2e\xa5\xce\x19\x67\x70\xfa\x47\x99\x7b\xa5\xbf\xdd"
      "\xf5\xea\x4d\x36\x7e\x54\xd0\x49\x8b\x4f\xe1\x1b\xef\xa6\xe7\xcf\x37\x89"
      "\x09\xe7\x77\x5a\xdd\x5a\xf9\xcd\x91\x13\xf0\xcd\xf1\x7d\xff\x7e\xe7\x76"
      "\x2b\xc2\x56\x31\xaa\x9e\x56\xcb\x0a\xe8\x8f\xe0\x39\xa4\x1b\x35\x82\x1a"
      "\xa4\x2b\x2a\xdd\x81\x14\x70\x5a\x03\x11\x0d\x68\xa8\xed\x29\x94\xf3\x21"
      "\xa9\xac\x26\x75\x58\x5b\x73\x95\xea\x39\xee\xf9\xb8\x11\xc6\x74\xb2\x4c"
      "\x4d\x56\x3d\xcf\x12\xfb\xf9\xbd\x9e\x0e\xa8\x1e\xc8\x69\x30\xac\xdb\x59"
      "\x38\x6d\x18\xe8\x5e\x30\x77\x9d\x37\xda\xac\x98\xf7\xde\x63\x8c\x75\x2a"
      "\xad\x25\x88\x32\x86\x3a\x0b\x34\x2d\x34\xe7\xf7\x6e\x51\x96\xe5\xb4\x34"
      "\x49\xfe\x8a\xb3\xba\xbd\x03\xcc\xc9\xb1\xd4\xe6\x0d\xa6\x4f\x7e\xb8\xfd"
      "\xb2\xf9\x3c\x6b\x67\x5f\x6e\x88\x8d\x58\x4c\xea\xa9\x23\x14\x15\xd4\x5e"
      "\xba\xb2\x78\x4b\x45\x45\xf5\x40\x81\xeb\xd4\x75\x57\xba\x5e\x34\xca\xa5"
      "\xa9\x4b\xd6\xab\x74\x86\xe8\xdb\xc9\xef\x40\xc1\xb2\x55\xd0\x6e\x2b\x53"
      "\x97\x0e\x4c\xcf\x9e\x79\x6a\x6b\xc7\x73\x07\x37\xc9\xec\x8c\xc6\x79\xde"
      "\x7d\xa6\x5c\xea\x51\x9b\x9d\x95\x9b\x36\x40\xbc\x0d\x86\x9d\xe7\xeb\xdc"
      "\x36\xd1\x71\x8e\x35\xd1\x43\xd1\xbc\xe8\x17\x67\x75\x60\x48\x7e\x00\xd0"
      "\xaa\xa1\x6b\xad\x5a\xd5\x2a\xe7\xbd\x49\x78\x37\x18\x94\x1b\xac\xd7\xe0"
      "\x9f\x0b\x7f\x9e\x63\x37\xe5\x11\x88\x79\x56\x63\xdf\x44\x71\xb1\xab\x10"
      "\xd6\xca\xe1\xbd\xa1\x7e\x9a\x53\xb9\xd4\x68\x49\x3e\xc7\x36\x55\xcb\x4f"
      "\xd6\x43\x2f\xbd\x8e\xc1\x37\xb7\x46\x5a\x3b\xa7\x90\xc7\x75\x0f\xae\xfd"
      "\x07\x72\x6e\xcf\x3b\xee\x6b\x6b\xd5\xd2\xac\x01\xa9\xd1\x2d\xf5\xc2\x62"
      "\x6a\xea\xbe\x56\xc9\x12\xfe\xd8\xbe\x19\x4e\xdd\x54\xad\x98\x0b\x22\xdd"
      "\x9e\x74\xb8\xb9\xc9\x01\x61\xf5\xec\xce\x91\x22\x58\x31\x50\xf8\x4e\xa9"
      "\x87\x01\x1f\x9c\xde\x27\x4f\x3e\x49\xac\xdd\x91\x78\xf5\xae\xff\x20\xc9"
      "\x97\x26\xb2\x99\xaa\x31\x51\xed\xa4\x14\x57\x77\x68\x47\x5a\x1f\xf4\x15"
      "\xce\x32\x32\xd2\x5a\xfc\x3d\xa1\x74\x7f\x99\x40\x5c\x92\x29\x25\xa7\x8a"
      "\xf6\x91\xe2\xc7\xb8\x29\x07\x85\x20\x95\x45\x11\x5d\xc0\x9e\x8f\xea\x0c"
      "\x2c\xcd\xc6\x42\x71\xdd\x5b\xec\x2c\xf5\x29\x2b\x1f\x92\xf0\x8d\x62\xde"
      "\x6f\x67\x62\x70\xaa\x4d\x96\xc3\xc8\x31\xa3\xcb\x62\x76\x53\xaa\xa8\x7d"
      "\x51\x45\xe1\x4f\xbc\xd6\x84\x98\x79\x26\xc4\xc0\xc7\xb6\x84\xd5\xb9\x19"
      "\x51\xc5\xc5\xb2\xb3\x9c\x72\x82\x86\x9c\xf7\xbe\x77\x6c\xbb\xb5\xb2\xf5"
      "\xbb\x38\xc7\xd8\x6d\x63\x0f\xe4\x48\xb4\xfc\x22\x6e\x0d\x13\xef\xa9\x84"
      "\xac\x85\xdd\x76\x34\xb3\x4d\x83\x3b\x0f\x80\x0b\xc2\xf7\x5f\xdf\xd4\xf0"
      "\x08\x5b\xc0\x08\x3e\xa2\x99\xde\xae\x27\xf1\xc0\x29\xb1\xd9\xb3\xad\xa7"
      "\x84\x70\x1e\xef\xab\x9f\xb8\xd5\x5a\xb2\x5c\x05\x38\x31\x91\xdd\xfe\xf4"
      "\x63\x7f\x46\x93\xc2\x4f\xeb\x2a\x95\x52\x3a\x0d\x5c\xb6\xe9\x20\x2b\x0b"
      "\x4f\xbb\x30\xfc\xfd\xaf\x23\xba\x90\xfe\x57\xa6\x90\x79\x9f\xaa\xdf\x43"
      "\xaa\x78\xa8\xa9\x5c\x48\x99\x03\xa6\x7d\x2a\x6f\xe9\x36\x4d\x83\x0f\xd1"
      "\x0d\xbf\x04\x8a\x12\x2c\x05\x86\x14\x79\xd2\x3c\xce\x86\x3e\x9f\x6b\x07"
      "\x5b\x93\x5f\x82\x5f\xa0\x92\x63\x43\x18\x62\x56\x61\xa4\xce\xa6\x16\x6c"
      "\x1a\xee\xd3\x6d\x40\xda\xf1\x23\x66\x6d\x4e\x4f\xd8\xd1\xad\x75\x7b\x87"
      "\x56\x1f\x27\x2d\x6c\x8f\x5f\xb0\x07\x6b\x12\x96\xbf\xf2\x37\xe6\xf2\x25"
      "\xca\xf6\x40\x5a\x01\xad\x3a\xe0\x2f\xf2\xd2\xe0\x29\x73\x2a\xaa\x46\xaa"
      "\x4a\x65\xab\x39\x18\x37\x56\xb7\xf4\xe6\x41\x81\x2e\x55\xc5\x9f\xe1\xba"
      "\x0a\x16\x06\xde\x9d\x2d\x39\x65\xfa\x99\xae\x4d\x84\x7e\x72\x40\x02\xca"
      "\x1a\xae\x5a\xd2\xb1\x9a\x28\x0a\x57\xd0\xb7\x50\x42\xa0\x0a\x4e\x17\x2f"
      "\x3c\x9a\xd1\x84\x28\x07\x99\x1f\xb9\xdf\x52\x74\x08\x94\x31\xba\xdd\xb1"
      "\x69\x14\xef\x73\xf8\xe4\xad\xfd\xa0\xd9\x49\xb4\xee\xfe\xb1\x1e\xe2\xb0"
      "\x7a\x72\x86\xa4\x92\x01\xc5\x4c\x29\x36\xc6\x99\x04\x28\x91\x9b\x40\x0f"
      "\x85\xf0\x78\x40\x59\x36\x56\x20\x00\x01\x54\x5b\x83\x07\xa6\xa4\xde\xeb"
      "\xb3\x28\xdc\x20\xc8\xbe\xcb\xda\xe4\x07\xd7\x6c\x33\xda\x18\xdb\x54\x4c"
      "\x57\x11\x1c\xd7\x35\x86\x5f\xdc\xf3\x94\x6c\x3c\x0e\x2d\x71\x72\x36\x41"
      "\x29\x79\x96\x63\x4c\x11\xf3\x9b\x10\xd0\x59\x72\xcd\x00\xb2\x18\xf4\xdc"
      "\x44\x46\x88\x53\x1a\x3f\x31\xa3\xf4\x7b\x96\xc3\x76\xda\xaf\x3c\x56\x7a"
      "\x8e\xd7\x72\x30\x65\x0c\x84\x08\x71\xbf\x58\x05\x7b\xcc\x40\x20\xfa\x97"
      "\xb9\x15\x45\xd9\x08\x21\x4b\x94\x88\x89\x2e\x88\x4b\xed\xc4\x91\x8a\x76"
      "\x77\xdd\x7c\x42\xed\xb9\x50\xc4\xc0\xaf\xa9\x56\x15\x99\x97\xdf\x8f\x15"
      "\x0d\x41\x63\x13\xa2\x1e\xec\x7a\xa3\xe8\xe0\x4f\xbc\x6a\x07\xdc\x0c\xd4"
      "\xdb\x98\x97\xee\x7a\x02\xb1\x81\x7e\xb1\xd6\xe9\xfc\x10\xb3\x60\x28\x1f"
      "\x9c\x1e\x5c\x7e\x95\x9f\x1f\xfa\xc0\x63\x8b\xa1\x23\xa4\xdf\x67\x45\x2b"
      "\x69\x0a\x4f\x3a\x44\x9f\xe4\x0e\x5a\xc7\x9a\x42\xdd\xe4\x1d\x08\xe3\x7b"
      "\x2b\x73\x1c\x68\x90\xfa\xb0\x48\x9a\x89\xac\x1c\xbe\xc1\xdb\x6a\x04\xdb"
      "\x08\x73\x75\x32\x46\x93\x3e\x5b\x5a\x00\xa8\x48\x57\x9b\xf9\xcf\x4e\x6d"
      "\xf5\x66\x49\xf3\x99\x4a\x64\x59\xb8\x4a\x21\xe2\x50\x17\x5f\xca\xda\x45"
      "\x38\xb2\x91\x0b\x8d\x70\xff\xd0\xa2\x12\xd6\xb2\xf7\x0f\x94\x4a\xdf\x13"
      "\x07\xed\xcf\xfb\xdd\xd8\x42\x36\xcc\xde\xc9\x8d\x8b\x83\xf2\x11\x50\x4d"
      "\x8e\xa1\xcf\x92\x21\x9d\xe2\xf7\x7e\xbe\xbb\xc8\x84\x75\x97\xcf\x36\xaf"
      "\x15\xd0\x20\xd8\xa1\x88\x24\x75\xc2\xcc\x4e\xbd\x78\xdd\xe3\xeb\xf0\x5e"
      "\xb7\xad\xb2\x9f\xf9\xe5\x59\x52\x70\xc3\x80\xf3\x6f\xf9\x42\xbc\x6d\x71"
      "\x8c\x42\xa5\x23\x2a\xac\x3d\xc1\xc2\x6d\x29\x9d\xca\xa8\xc1\xab\xc8\x3d"
      "\xcf\x1d\x52\x49\x13\x9c\x7f\x98\x9b\xeb\x3f\x68\xcd\xd9\x01\xf4\x96\x21"
      "\xd9\x23\x79\x1e\x5b\xa7\xb2\xd6\xf0\x9e\x48\xd0\x81\xb6\x15\x19\x24\xaa"
      "\x0c\x94\x9b\x22\xbb\x36\x50\x70\xa4\x2b\x19\x0f\xef\x3b\xe2\x1b\xec\x66"
      "\x7d\x9e\x37\x1e\xb0\xf9\x45\xdb\xa6\x7f\x28\x95\x1d\xf1\xd8\xcf\x7e\xf4"
      "\x3b\x19\x42\x02\x8c\xaa\xce\x6d\x71\xcc\x6f\x3b\xea\xe1\xc0\x4b\xd2\x4b"
      "\xf7\xa5\xdb\x71\x75\x61\xd1\x04\x92\x1b\xf4\x4c\xec\x64\x38\x1e\x3e\x1f"
      "\xb5\x1d\x5c\x5a\x66\xef\x48\xba\x6d\xda\xf2\xe1\x33\xf7\x81\x69\xfb\xf4"
      "\xa1\x5b\x92\xfc\x59\xb7\xb3\x74\xe9\x03\xfb\x3c\xbb\xb2\x5f\x5d\x79\x97"
      "\xfd\x88\xd2\xef\xff\xf8\x41\x1e\xc9\xd7\x9a\xba\x14\x3b\x91\xaa\xea\x10"
      "\x64\x03\x4f\x90\x82\x49\x5c\xcf\x10\x09\x1f\xba\x42\x12\x4d\xc8\x94\xbc"
      "\x52\x79\x33\xc1\xcd\x21\x70\xe5\x6c\x73\xf1\xa9\x90\x32\xa2\x7b\xf7\xeb"
      "\xf6\xfe\x4a\x2e\x7b\x19\x46\x0f\x9b\x94\x16\x60\x6f\x35\x3a\x0b\x19\x2a"
      "\xd0\xfb\xfb\x3e\x07\xe6\x72\xa6\xb0\xb6\x11\x02\xdb\xd3\xae\xf1\x1e\x3c"
      "\x21\x47\x7f\xb8\xaf\x07\xfa\x7c\xec\xff\xf6\x4f\xae\x33\x1b\x4b\x1a\x0b"
      "\x7b\x38\x4b\x70\x49\x7c\x90\x5d\x86\x56\x9d\x17\xb9\x80\xaf\xe7\x80\xf6"
      "\x32\x28\x1a\xc2\xb2\xbf\x85\x22\xca\xf1\x0a\xd1\x2c\x68\xe2\x64\x3c\x4e"
      "\x05\x8a\xbc\xea\xf4\xd2\x90\x28\xa8\x64\xee\xe4\x7b\x8d\x16\x09\x0a\x2f"
      "\x59\x56\x4e\x00\x0f\x31\xe5\x31\x10\xff\xad\xfb\xa6\xa7\xe6\x62\x6c\x48"
      "\x28\x70\x6f\xef\x5e\x90\x24\x2c\xc6\x85\xfb\xa0\x17\xe1\xab\x63\xca\xe0"
      "\xf1\xc9\x88\xc4\xa3\x71\x8b\xd8\x98\x92\xfe\xfc\x43\xe7\x19\x67\x23\xae"
      "\x04\x22\x10\xb8\x55\x84\xc7\x8a\xfb\x68\xe3\x21\x30\x02\x8e\x4f\x78\xda"
      "\x3c\x2b\x99\x59\x06\x3d\xaa\x23\x59\x50\x96\x7a\xaf\xcd\x79\xcb\xe6\xfc"
      "\x6b\xac\x76\x20\x0c\x69\xe6\x56\xc7\x67\xd5\x11\x23\x97\x5b\x17\x31\xf5"
      "\xfd\x98\x91\xbd\xeb\x5d\x90\x23\x9c\x20\x86\x0f\x02\x03\xc5\x63\x76\xd6"
      "\x49\xcf\x1f\xbf\xcb\xe9\xd2\x60\x2d\x7a\xd7\xf3\x85\x4a\x2e\x5e\x7b\xde"
      "\xc3\x45\x04\xed\x02\xe0\xdd\x8b\xe3\x4b\x4f\x70\x4f\xf9\x1c\x05\x65\xe8"
      "\x0e\x35\x47\x50\xfe\xf3\x77\xd3\xb4\x40\xdf\xea\x01\x0b\xc7\xb9\x29\xd3"
      "\x55\x7d\x84\x88\x46\x1c\x21\x57\xfd\xd1\x70\x8b\xfb\x49\xb4\x6d\xdf\x3c"
      "\x61\x1d\x0d\xc4\x59\x55\x6a\xa6\x17\x8e\xc7\xcb\x13\xfe\x3c\x7e\x2f\xf8"
      "\x24\x5d\x41\x47\x17\x15\xfe\xd5\x00\x83\x6e\xd6\xa3\x2b\xb9\x89\x12\xd0"
      "\x84\x63\xee\xb1\xe7\xd5\x8b\x37\x34\x1d\xf1\x00\xa0\x20\xa2\x4c\x41\x83"
      "\xec\x60\xc4\xc7\x56\x0a\xa7\x6c\xb0\x74\x13\x98\xc3\xa6\x92\x2d\x2c\x0f"
      "\xef\x63\x81\xca\x19\xce\xf3\x7f\x2b\x28\xee\x7e\x70\x39\x3d\x4b\xcb\x10"
      "\x0e\xaa\x0a\xea\xde\x98\x8c\x29\x38\xe5\xd7\x8f\xc3\xcb\xbe\x6a\x2b\xbe"
      "\xff\x4a\x79\x8d\xb4\xc7\x2c\x70\x4a\x8c\xcc\x2e\xb3\xd8\x20\xbc\x05\x74"
      "\x0e\xd5\x84\xf4\xc0\x58\x88\xd6\x3a\xdc\xa4\x20\x13\xef\xb6\xcc\x24\x4a"
      "\x86\xc8\x4b\xe1\x57\xb3\x68\xf5\x78\x8c\x3e\x45\xb6\x13\x19\xe6\xec\x2a"
      "\xc4\x2b\xd7\x12\x2c\x9e\xb5\x29\x5a\xbe\x26\x46\x5b\x11\x2b\x66\x9a\xe8"
      "\xc2\xff\x62\x6f\x65\x01\xf9\xd6\x3e\x51\x6e\x6f\x6b\x7d\xb4\xdd\x75\x3b"
      "\xc5\x47\xda\xdb\xea\x14\x0c\xb7\x4e\x0a\x7e\x6c\x99\x8d\x11\xe1\xa5\x95"
      "\x4d\xc4\x98\x30\x1f\xf6\xce\x36\xd7\x5f\xf5\xde\x40\xf0\xcc\xe1\xf6\x76"
      "\xbc\x18\x5d\xf8\x39\x42\x8d\xba\xa6\xd6\x31\x2e\xd7\xdd\x72\xd4\xfa\xc7"
      "\xb1\x6a\xf2\x1e\x54\x1e\x43\x5a\x58\xb3\x01\xe7\xe2\xd4\x8f\x4a\xd9\x37"
      "\xb6\x3b\x0b\x71\x26\x2c\x7f\x9b\x68\xb8\x6f\xab\x4a\x19\x44\x47\xbf\x09"
      "\xa0\x9f\xde\x91\xd1\x32\xf0\xa7\x48\xa1\x50\xf7\x6e\x64\xd4\xee\x5a\x34"
      "\xa3\x58\xe0\x20\xf7\x04\x11\x57\x0b\xc5\x42\x4e\x48\xc1\xfc\x9c\x5e\xa9"
      "\x32\xb3\x29\x9d\x16\xd5\xca\x31\xde\x56\x5d\xce\xb1\x35\xdf\x42\x5e\x41"
      "\x34\x27\x8c\xdb\x63\x78\x19\xc8\x02\x0f\xcb\x46\x33\x69\xb6\xbc\xaa\x86"
      "\xc7\x5b\x6d\x95\x00\xa7\x01\x97\x18\x3b\x11\xb9\xdc\x0a\xb8\xde\x8e\x70"
      "\x29\xb1\xe0\xca\x3b\x84\x5f\xe1\x30\x0e\xd5\x87\xdb\xd1\xb8\x6f\xc2\x14"
      "\x8f\x96\x9c\xb6\x3a\x72\xd9\x3b\xe5\x9f\xe0\x44\x0c\xfd\x76\x61\xf0\xc5"
      "\xfe\x0a\x7b\x96\xc8\xf9\xdc\xfd\x5a\x4f\x58\xe5\xd5\x66\xb6\x26\x02\x77"
      "\x76\x11\x56\x8b\xb1\xd3\xb3\x43\x80\x60\x8c\x24\x9e\x17\x0f\xe4\xd5\xce"
      "\xd8\x25\xc0\x5a\x92\x61\x6b\xa8\xe9\x12\x3d\x62\xff\x72\x92\x7c\x35\x0a"
      "\x62\x3c\xc5\xc0\xb0\xbe\xfc\x1b\x16\x9b\x07\xc6\xe5\xbb\x98\x32\xe7\x17"
      "\x1e\x4f\xb8\xa6\x14\x2e\xab\xce\xc4\x47\x01\xbe\x5b\x61\x8b\xb7\x10\xd8"
      "\x4b\xdc\xcc\x84\x80\x33\x09\x5d\xd9\xe8\xd2\xd7\xcb\x2e\xc7\x68\x9b\x5e"
      "\x32\x8b\xea\x4e\x8a\x78\x2d\x57\x22\x09\xbe\xce\x89\x4a\xb7\x02\x94\x5a"
      "\x34\xab\x97\xe4\xb5\xe3\x65\xab\x76\xad\xfa\xe3\x96\x32\xf0\x4f\x06\x47"
      "\x27\x9d\x95\x84\xeb\x2d\x02\x20\x2c\x0d\xad\xad\x89\x5f\x2c\xb5\xb5\xd3"
      "\xe1\xd1\x08\x57\x98\xc0\x0b\xf8\x10\xd9\x11",
      2819);
  *(uint64_t*)0x20002548 = 0xb03;
  *(uint64_t*)0x20002618 = 1;
  *(uint64_t*)0x20002620 = 0;
  *(uint64_t*)0x20002628 = 0;
  *(uint32_t*)0x20002630 = 0;
  *(uint32_t*)0x20002638 = 0;
  syscall(__NR_sendmmsg, r[4], 0x20002600ul, 1ul, 0ul);
  *(uint8_t*)0x20000000 = 0x16;
  *(uint8_t*)0x20000001 = 0;
  *(uint16_t*)0x20000002 = 0;
  *(uint32_t*)0x20000004 = 5;
  *(uint64_t*)0x20000008 = 0;
  *(uint64_t*)0x20000010 = 0x20000040;
  *(uint32_t*)0x20000018 = 0x12;
  *(uint32_t*)0x2000001c = 0;
  *(uint64_t*)0x20000020 = 0;
  *(uint16_t*)0x20000028 = 0;
  *(uint16_t*)0x2000002a = 0;
  memset((void*)0x2000002c, 0, 20);
  syz_io_uring_submit(r[1], r[2], 0x20000000, 0);
  syscall(__NR_io_uring_enter, r[0], 0x2ff, 0, 0ul, 0ul, 0ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  loop();
  return 0;
}
