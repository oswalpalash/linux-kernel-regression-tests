// https://syzkaller.appspot.com/bug?id=30b36ad692dfa0eb09e5826bf2783a3a36f4c094
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

unsigned long long procid;

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
  int i;
  for (i = 0; i < 100; i++) {
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
  int iter;
  for (iter = 0;; iter++) {
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
      if (current_time_ms() - start < 5 * 1000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

void execute_one(void)
{
  *(uint32_t*)0x20000440 = 2;
  *(uint32_t*)0x20000444 = 6;
  *(uint64_t*)0x20000448 = 0x20000000;
  *(uint8_t*)0x20000000 = 0xb4;
  STORE_BY_BITMASK(uint8_t, , 0x20000001, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000001, 0, 4, 4);
  *(uint16_t*)0x20000002 = 0;
  *(uint32_t*)0x20000004 = 0x4800;
  *(uint8_t*)0x20000008 = 0x61;
  *(uint8_t*)0x20000009 = 0x11;
  *(uint16_t*)0x2000000a = 4;
  *(uint32_t*)0x2000000c = 0;
  *(uint8_t*)0x20000010 = 0x85;
  STORE_BY_BITMASK(uint8_t, , 0x20000011, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000011, 1, 4, 4);
  *(uint16_t*)0x20000012 = 0;
  *(uint32_t*)0x20000014 = 2;
  *(uint8_t*)0x20000018 = 0x85;
  *(uint8_t*)0x20000019 = 0;
  *(uint16_t*)0x2000001a = 0;
  *(uint32_t*)0x2000001c = 5;
  *(uint8_t*)0x20000020 = 0x95;
  *(uint8_t*)0x20000021 = 0;
  *(uint16_t*)0x20000022 = 0;
  *(uint32_t*)0x20000024 = 0;
  *(uint8_t*)0x20000028 = 0x95;
  *(uint8_t*)0x20000029 = 0;
  *(uint16_t*)0x2000002a = 0;
  *(uint32_t*)0x2000002c = 0;
  *(uint64_t*)0x20000450 = 0x20000080;
  memcpy((void*)0x20000080, "GPL\000", 4);
  *(uint32_t*)0x20000458 = 5;
  *(uint32_t*)0x2000045c = 0xc3;
  *(uint64_t*)0x20000460 = 0x200000c0;
  *(uint32_t*)0x20000468 = 0;
  *(uint32_t*)0x2000046c = 0;
  *(uint8_t*)0x20000470 = 0;
  *(uint8_t*)0x20000471 = 0;
  *(uint8_t*)0x20000472 = 0;
  *(uint8_t*)0x20000473 = 0;
  *(uint8_t*)0x20000474 = 0;
  *(uint8_t*)0x20000475 = 0;
  *(uint8_t*)0x20000476 = 0;
  *(uint8_t*)0x20000477 = 0;
  *(uint8_t*)0x20000478 = 0;
  *(uint8_t*)0x20000479 = 0;
  *(uint8_t*)0x2000047a = 0;
  *(uint8_t*)0x2000047b = 0;
  *(uint8_t*)0x2000047c = 0;
  *(uint8_t*)0x2000047d = 0;
  *(uint8_t*)0x2000047e = 0;
  *(uint8_t*)0x2000047f = 0;
  *(uint32_t*)0x20000480 = 0;
  *(uint32_t*)0x20000484 = 0;
  *(uint32_t*)0x20000488 = -1;
  *(uint32_t*)0x2000048c = 8;
  *(uint64_t*)0x20000490 = 0;
  *(uint32_t*)0x20000498 = 0;
  *(uint32_t*)0x2000049c = 0x10;
  *(uint64_t*)0x200004a0 = 0;
  *(uint32_t*)0x200004a8 = 0;
  syscall(__NR_bpf, 5, 0x20000440, 0x70);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
