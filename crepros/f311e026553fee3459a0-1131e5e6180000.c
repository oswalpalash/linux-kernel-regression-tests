// https://syzkaller.appspot.com/bug?id=2fe9ea441886baefcee9f6bcef484f5de6eb9fb7
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
  *(uint32_t*)0x200009c4 = 4;
  *(uint32_t*)0x200009c8 = 8;
  *(uint32_t*)0x200009cc = 0x100c;
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
  *(uint32_t*)0x200000c0 = 0x11;
  *(uint32_t*)0x200000c4 = 0xc;
  *(uint64_t*)0x200000c8 = 0x20000280;
  *(uint8_t*)0x20000280 = 0x18;
  STORE_BY_BITMASK(uint8_t, , 0x20000281, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000281, 0, 4, 4);
  *(uint16_t*)0x20000282 = 0;
  *(uint32_t*)0x20000284 = 0;
  *(uint8_t*)0x20000288 = 0;
  *(uint8_t*)0x20000289 = 0;
  *(uint16_t*)0x2000028a = 0;
  *(uint32_t*)0x2000028c = 0;
  *(uint8_t*)0x20000290 = 0x18;
  STORE_BY_BITMASK(uint8_t, , 0x20000291, 1, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000291, 1, 4, 4);
  *(uint16_t*)0x20000292 = 0;
  *(uint32_t*)0x20000294 = r[0];
  *(uint8_t*)0x20000298 = 0;
  *(uint8_t*)0x20000299 = 0;
  *(uint16_t*)0x2000029a = 0;
  *(uint32_t*)0x2000029c = 0;
  STORE_BY_BITMASK(uint8_t, , 0x200002a0, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x200002a0, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x200002a0, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x200002a1, 8, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x200002a1, 0, 4, 4);
  *(uint16_t*)0x200002a2 = 0;
  *(uint32_t*)0x200002a4 = 0;
  STORE_BY_BITMASK(uint8_t, , 0x200002a8, 3, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x200002a8, 3, 3, 2);
  STORE_BY_BITMASK(uint8_t, , 0x200002a8, 3, 5, 3);
  STORE_BY_BITMASK(uint8_t, , 0x200002a9, 0xa, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x200002a9, 8, 4, 4);
  *(uint16_t*)0x200002aa = 0xfff8;
  *(uint32_t*)0x200002ac = 0;
  STORE_BY_BITMASK(uint8_t, , 0x200002b0, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x200002b0, 1, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x200002b0, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x200002b1, 2, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x200002b1, 0xa, 4, 4);
  *(uint16_t*)0x200002b2 = 0;
  *(uint32_t*)0x200002b4 = 0;
  STORE_BY_BITMASK(uint8_t, , 0x200002b8, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x200002b8, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x200002b8, 0, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x200002b9, 2, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x200002b9, 0, 4, 4);
  *(uint16_t*)0x200002ba = 0;
  *(uint32_t*)0x200002bc = 0xfffffff8;
  STORE_BY_BITMASK(uint8_t, , 0x200002c0, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x200002c0, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x200002c0, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x200002c1, 3, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x200002c1, 0, 4, 4);
  *(uint16_t*)0x200002c2 = 0;
  *(uint32_t*)0x200002c4 = 8;
  STORE_BY_BITMASK(uint8_t, , 0x200002c8, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x200002c8, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x200002c8, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x200002c9, 4, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x200002c9, 0, 4, 4);
  *(uint16_t*)0x200002ca = 0;
  *(uint32_t*)0x200002cc = 0;
  *(uint8_t*)0x200002d0 = 0x85;
  *(uint8_t*)0x200002d1 = 0;
  *(uint16_t*)0x200002d2 = 0;
  *(uint32_t*)0x200002d4 = 3;
  *(uint8_t*)0x200002d8 = 0x95;
  *(uint8_t*)0x200002d9 = 0;
  *(uint16_t*)0x200002da = 0;
  *(uint32_t*)0x200002dc = 0;
  *(uint64_t*)0x200000d0 = 0x20000040;
  memcpy((void*)0x20000040, "syzkaller\000", 10);
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
  *(uint64_t*)0x20000a40 = 0x20000980;
  memcpy((void*)0x20000980, "workqueue_activate_work\000", 24);
  *(uint32_t*)0x20000a48 = r[1];
  syscall(__NR_bpf, /*cmd=*/0x11ul, /*arg=*/0x20000a40ul, /*size=*/0x10ul);
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
