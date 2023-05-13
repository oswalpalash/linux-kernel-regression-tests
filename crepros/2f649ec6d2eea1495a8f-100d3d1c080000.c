// https://syzkaller.appspot.com/bug?id=ade76788e9b55f1c31241bc879c8ce1177f4b7ca
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

static int inject_fault(int nth)
{
  int fd;
  fd = open("/proc/thread-self/fail-nth", O_RDWR);
  if (fd == -1)
    exit(1);
  char buf[16];
  sprintf(buf, "%d", nth);
  if (write(fd, buf, strlen(buf)) != (ssize_t)strlen(buf))
    exit(1);
  return fd;
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

static void setup_fault()
{
  static struct {
    const char* file;
    const char* val;
    bool fatal;
  } files[] = {
      {"/sys/kernel/debug/failslab/ignore-gfp-wait", "N", true},
      {"/sys/kernel/debug/fail_futex/ignore-private", "N", false},
      {"/sys/kernel/debug/fail_page_alloc/ignore-gfp-highmem", "N", false},
      {"/sys/kernel/debug/fail_page_alloc/ignore-gfp-wait", "N", false},
      {"/sys/kernel/debug/fail_page_alloc/min-order", "0", false},
  };
  unsigned i;
  for (i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
    if (!write_file(files[i].file, files[i].val)) {
      if (files[i].fatal)
        exit(1);
    }
  }
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

uint64_t r[1] = {0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  *(uint32_t*)0x20000140 = 0xa;
  *(uint32_t*)0x20000144 = 0x16;
  *(uint32_t*)0x20000148 = 8;
  *(uint32_t*)0x2000014c = 0x7f;
  *(uint32_t*)0x20000150 = 0;
  *(uint32_t*)0x20000154 = -1;
  *(uint32_t*)0x20000158 = 0;
  memset((void*)0x2000015c, 0, 16);
  *(uint32_t*)0x2000016c = 0;
  *(uint32_t*)0x20000170 = -1;
  *(uint32_t*)0x20000174 = 0;
  *(uint32_t*)0x20000178 = 0;
  *(uint32_t*)0x2000017c = 0;
  *(uint64_t*)0x20000180 = 0;
  res = syscall(__NR_bpf, 0x100000000000000ul, 0x20000140ul, 0x48ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000440 = 0x11;
  *(uint32_t*)0x20000444 = 0xa;
  *(uint64_t*)0x20000448 = 0x20000680;
  *(uint8_t*)0x20000680 = 0x18;
  STORE_BY_BITMASK(uint8_t, , 0x20000681, 8, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000681, 0, 4, 4);
  *(uint16_t*)0x20000682 = 0;
  *(uint32_t*)0x20000684 = 0;
  *(uint8_t*)0x20000688 = 0;
  *(uint8_t*)0x20000689 = 0;
  *(uint16_t*)0x2000068a = 0;
  *(uint32_t*)0x2000068c = 0;
  *(uint8_t*)0x20000690 = 0x85;
  STORE_BY_BITMASK(uint8_t, , 0x20000691, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000691, 1, 4, 4);
  *(uint16_t*)0x20000692 = 0;
  *(uint32_t*)0x20000694 = 6;
  *(uint8_t*)0x20000698 = 0x18;
  STORE_BY_BITMASK(uint8_t, , 0x20000699, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000699, 1, 4, 4);
  *(uint16_t*)0x2000069a = 0;
  *(uint32_t*)0x2000069c = r[0];
  *(uint8_t*)0x200006a0 = 0;
  *(uint8_t*)0x200006a1 = 0;
  *(uint16_t*)0x200006a2 = 0;
  *(uint32_t*)0x200006a4 = 0;
  *(uint8_t*)0x200006a8 = 0x15;
  STORE_BY_BITMASK(uint8_t, , 0x200006a9, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x200006a9, 0, 4, 4);
  *(uint16_t*)0x200006aa = 0;
  *(uint32_t*)0x200006ac = 0;
  *(uint8_t*)0x200006b0 = 0x18;
  STORE_BY_BITMASK(uint8_t, , 0x200006b1, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x200006b1, 0, 4, 4);
  *(uint16_t*)0x200006b2 = 0;
  *(uint32_t*)0x200006b4 = 0;
  *(uint8_t*)0x200006b8 = 0;
  *(uint8_t*)0x200006b9 = 0;
  *(uint16_t*)0x200006ba = 0;
  *(uint32_t*)0x200006bc = 0;
  *(uint8_t*)0x200006c0 = 0x95;
  *(uint8_t*)0x200006c1 = 0;
  *(uint16_t*)0x200006c2 = 0;
  *(uint32_t*)0x200006c4 = 0;
  *(uint8_t*)0x200006c8 = 0x95;
  *(uint8_t*)0x200006c9 = 0;
  *(uint16_t*)0x200006ca = 0;
  *(uint32_t*)0x200006cc = 0;
  *(uint64_t*)0x20000450 = 0x20000000;
  memcpy((void*)0x20000000, "GPL\000", 4);
  *(uint32_t*)0x20000458 = 4;
  *(uint32_t*)0x2000045c = 0xee;
  *(uint64_t*)0x20000460 = 0x20000340;
  *(uint32_t*)0x20000468 = 0;
  *(uint32_t*)0x2000046c = 0;
  memset((void*)0x20000470, 0, 16);
  *(uint32_t*)0x20000480 = 0;
  *(uint32_t*)0x20000484 = 0;
  *(uint32_t*)0x20000488 = -1;
  *(uint32_t*)0x2000048c = 8;
  *(uint64_t*)0x20000490 = 0;
  *(uint32_t*)0x20000498 = 0;
  *(uint32_t*)0x2000049c = 0x10;
  *(uint64_t*)0x200004a0 = 0;
  *(uint32_t*)0x200004a8 = 0;
  *(uint32_t*)0x200004ac = 0;
  *(uint32_t*)0x200004b0 = 0;
  *(uint32_t*)0x200004b4 = 0;
  *(uint64_t*)0x200004b8 = 0;
  inject_fault(49);
  syscall(__NR_bpf, 5ul, 0x20000440ul, 0x80ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  setup_fault();
  loop();
  return 0;
}
