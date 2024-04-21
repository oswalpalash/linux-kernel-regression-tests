// https://syzkaller.appspot.com/bug?id=bffd6506151659e22d4cb0e92de6388554762492
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
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#ifndef __NR_bpf
#define __NR_bpf 321
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

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  *(uint32_t*)0x200009c0 = 0x12;
  *(uint32_t*)0x200009c4 = 6;
  *(uint32_t*)0x200009c8 = 8;
  *(uint32_t*)0x200009cc = 1;
  *(uint32_t*)0x200009d0 = 0;
  *(uint32_t*)0x200009d4 = -1;
  *(uint32_t*)0x200009d8 = 0;
  memset((void*)0x200009dc, 0, 16);
  *(uint32_t*)0x200009ec = 0;
  *(uint32_t*)0x200009f0 = -1;
  *(uint32_t*)0x200009f4 = 0;
  *(uint32_t*)0x200009f8 = 0;
  *(uint32_t*)0x200009fc = 0;
  *(uint64_t*)0x20000a00 = 0;
  res = syscall(__NR_bpf, /*cmd=*/0ul, /*arg=*/0x200009c0ul, /*size=*/0x48ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x200000c0 = 0;
  *(uint32_t*)0x200000c4 = 0xc;
  *(uint64_t*)0x200000c8 = 0x20000440;
  *(uint8_t*)0x20000440 = 0x18;
  STORE_BY_BITMASK(uint8_t, , 0x20000441, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000441, 0, 4, 4);
  *(uint16_t*)0x20000442 = 0;
  *(uint32_t*)0x20000444 = 0;
  *(uint8_t*)0x20000448 = 0;
  *(uint8_t*)0x20000449 = 0;
  *(uint16_t*)0x2000044a = 0;
  *(uint32_t*)0x2000044c = 0;
  *(uint8_t*)0x20000450 = 0x18;
  STORE_BY_BITMASK(uint8_t, , 0x20000451, 1, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000451, 1, 4, 4);
  *(uint16_t*)0x20000452 = 0;
  *(uint32_t*)0x20000454 = r[0];
  *(uint8_t*)0x20000458 = 0;
  *(uint8_t*)0x20000459 = 0;
  *(uint16_t*)0x2000045a = 0;
  *(uint32_t*)0x2000045c = 0;
  STORE_BY_BITMASK(uint8_t, , 0x20000460, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000460, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000460, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000461, 8, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000461, 0, 4, 4);
  *(uint16_t*)0x20000462 = 0;
  *(uint32_t*)0x20000464 = 0;
  STORE_BY_BITMASK(uint8_t, , 0x20000468, 3, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000468, 3, 3, 2);
  STORE_BY_BITMASK(uint8_t, , 0x20000468, 3, 5, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000469, 0xa, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000469, 8, 4, 4);
  *(uint16_t*)0x2000046a = 0xfff8;
  *(uint32_t*)0x2000046c = 0;
  STORE_BY_BITMASK(uint8_t, , 0x20000470, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000470, 1, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000470, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000471, 2, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000471, 0xa, 4, 4);
  *(uint16_t*)0x20000472 = 0;
  *(uint32_t*)0x20000474 = 0;
  STORE_BY_BITMASK(uint8_t, , 0x20000478, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000478, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000478, 0, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000479, 2, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000479, 0, 4, 4);
  *(uint16_t*)0x2000047a = 0;
  *(uint32_t*)0x2000047c = 0xfffffff8;
  STORE_BY_BITMASK(uint8_t, , 0x20000480, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000480, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000480, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000481, 3, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000481, 0, 4, 4);
  *(uint16_t*)0x20000482 = 0;
  *(uint32_t*)0x20000484 = 8;
  STORE_BY_BITMASK(uint8_t, , 0x20000488, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000488, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000488, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000489, 4, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000489, 0, 4, 4);
  *(uint16_t*)0x2000048a = 0;
  *(uint32_t*)0x2000048c = 0;
  *(uint8_t*)0x20000490 = 0x85;
  *(uint8_t*)0x20000491 = 0;
  *(uint16_t*)0x20000492 = 0;
  *(uint32_t*)0x20000494 = 3;
  *(uint8_t*)0x20000498 = 0x95;
  *(uint8_t*)0x20000499 = 0;
  *(uint16_t*)0x2000049a = 0;
  *(uint32_t*)0x2000049c = 0;
  *(uint64_t*)0x200000d0 = 0;
  *(uint32_t*)0x200000d8 = 0;
  *(uint32_t*)0x200000dc = 0;
  *(uint64_t*)0x200000e0 = 0;
  *(uint32_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000ec = 0;
  memset((void*)0x200000f0, 0, 16);
  *(uint32_t*)0x20000100 = 0;
  *(uint32_t*)0x20000104 = 0;
  *(uint32_t*)0x20000108 = -1;
  *(uint32_t*)0x2000010c = 0;
  *(uint64_t*)0x20000110 = 0;
  *(uint32_t*)0x20000118 = 0;
  *(uint32_t*)0x2000011c = 0;
  *(uint64_t*)0x20000120 = 0;
  *(uint32_t*)0x20000128 = 0;
  *(uint32_t*)0x2000012c = 0;
  *(uint32_t*)0x20000130 = 0;
  *(uint32_t*)0x20000134 = 0;
  *(uint64_t*)0x20000138 = 0;
  *(uint64_t*)0x20000140 = 0;
  *(uint32_t*)0x20000148 = 0;
  *(uint32_t*)0x2000014c = 0;
  syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x200000c0ul, /*size=*/0x90ul);
  *(uint32_t*)0x200000c0 = 0x11;
  *(uint32_t*)0x200000c4 = 0xc;
  *(uint64_t*)0x200000c8 = 0x20000440;
  *(uint64_t*)0x200000d0 = 0x20000240;
  memcpy((void*)0x20000240, "GPL\000", 4);
  *(uint32_t*)0x200000d8 = 0;
  *(uint32_t*)0x200000dc = 0;
  *(uint64_t*)0x200000e0 = 0;
  *(uint32_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000ec = 0;
  memset((void*)0x200000f0, 0, 16);
  *(uint32_t*)0x20000100 = 0;
  *(uint32_t*)0x20000104 = 0;
  *(uint32_t*)0x20000108 = -1;
  *(uint32_t*)0x2000010c = 0;
  *(uint64_t*)0x20000110 = 0;
  *(uint32_t*)0x20000118 = 0;
  *(uint32_t*)0x2000011c = 0;
  *(uint64_t*)0x20000120 = 0;
  *(uint32_t*)0x20000128 = 0;
  *(uint32_t*)0x2000012c = 0;
  *(uint32_t*)0x20000130 = 0;
  *(uint32_t*)0x20000134 = 0;
  *(uint64_t*)0x20000138 = 0;
  *(uint64_t*)0x20000140 = 0;
  *(uint32_t*)0x20000148 = 0;
  *(uint32_t*)0x2000014c = 0;
  res = syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x200000c0ul, /*size=*/0x90ul);
  if (res != -1)
    r[1] = res;
  *(uint64_t*)0x20000180 = 0x20000080;
  memcpy((void*)0x20000080, "contention_end\000", 15);
  *(uint32_t*)0x20000188 = r[1];
  syscall(__NR_bpf, /*cmd=*/0x11ul, /*arg=*/0x20000180ul, /*size=*/0x10ul);
}
int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  loop();
  return 0;
}
