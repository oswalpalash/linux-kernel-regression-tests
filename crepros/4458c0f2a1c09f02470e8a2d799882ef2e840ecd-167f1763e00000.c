// https://syzkaller.appspot.com/bug?id=4458c0f2a1c09f02470e8a2d799882ef2e840ecd
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

#define KMEMLEAK_FILE "/sys/kernel/debug/kmemleak"

static void setup_leak()
{
  if (!write_file(KMEMLEAK_FILE, "scan"))
    exit(1);
  sleep(5);
  if (!write_file(KMEMLEAK_FILE, "scan"))
    exit(1);
  if (!write_file(KMEMLEAK_FILE, "clear"))
    exit(1);
}

static void check_leaks(void)
{
  int fd = open(KMEMLEAK_FILE, O_RDWR);
  if (fd == -1)
    exit(1);
  uint64_t start = current_time_ms();
  if (write(fd, "scan", 4) != 4)
    exit(1);
  sleep(1);
  while (current_time_ms() - start < 4 * 1000)
    sleep(1);
  if (write(fd, "scan", 4) != 4)
    exit(1);
  static char buf[128 << 10];
  ssize_t n = read(fd, buf, sizeof(buf) - 1);
  if (n < 0)
    exit(1);
  int nleaks = 0;
  if (n != 0) {
    sleep(1);
    if (write(fd, "scan", 4) != 4)
      exit(1);
    if (lseek(fd, 0, SEEK_SET) < 0)
      exit(1);
    n = read(fd, buf, sizeof(buf) - 1);
    if (n < 0)
      exit(1);
    buf[n] = 0;
    char* pos = buf;
    char* end = buf + n;
    while (pos < end) {
      char* next = strstr(pos + 1, "unreferenced object");
      if (!next)
        next = end;
      char prev = *next;
      *next = 0;
      fprintf(stderr, "BUG: memory leak\n%s\n", pos);
      *next = prev;
      pos = next;
      nleaks++;
    }
  }
  if (write(fd, "clear", 5) != 5)
    exit(1);
  close(fd);
  if (nleaks)
    exit(1);
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
    check_leaks();
  }
}

void execute_one(void)
{
  memcpy((void*)0x200001c0, "./file0\000", 8);
  syscall(__NR_mkdir, 0x200001c0ul, 0ul);
  *(uint32_t*)0x20000000 = 5;
  *(uint32_t*)0x20000004 = 0x70;
  *(uint8_t*)0x20000008 = 0;
  *(uint8_t*)0x20000009 = 0;
  *(uint8_t*)0x2000000a = 0;
  *(uint8_t*)0x2000000b = 0;
  *(uint32_t*)0x2000000c = 0;
  *(uint64_t*)0x20000010 = 0;
  *(uint64_t*)0x20000018 = 0;
  *(uint64_t*)0x20000020 = 0;
  STORE_BY_BITMASK(uint64_t, , 0x20000028, 0, 0, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000028, 0, 1, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000028, 0, 2, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000028, 0, 3, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000028, 0, 4, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000028, 0, 5, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000028, 9, 6, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000028, 0, 7, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000028, 0, 8, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000028, 0, 9, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000028, 0, 10, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000028, 0, 11, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000028, 0, 12, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000028, 0, 13, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000028, 0, 14, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000028, 0, 15, 2);
  STORE_BY_BITMASK(uint64_t, , 0x20000028, 0, 17, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000028, 0, 18, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000028, 0, 19, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000028, 0, 20, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000028, 0, 21, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000028, 0, 22, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000028, 0, 23, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000028, 0, 24, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000028, 0, 25, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000028, 0, 26, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000028, 0, 27, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000028, 0, 28, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000028, 0, 29, 35);
  *(uint32_t*)0x20000030 = 0;
  *(uint32_t*)0x20000034 = 0;
  *(uint64_t*)0x20000038 = -1;
  *(uint64_t*)0x20000040 = 0;
  *(uint64_t*)0x20000048 = 0x200000000;
  *(uint64_t*)0x20000050 = 0x60;
  *(uint32_t*)0x20000058 = 0;
  *(uint32_t*)0x2000005c = 0;
  *(uint64_t*)0x20000060 = 0;
  *(uint32_t*)0x20000068 = 0;
  *(uint16_t*)0x2000006c = 0;
  *(uint16_t*)0x2000006e = 0;
  syscall(__NR_perf_event_open, 0x20000000ul, -1, 0ul, -1, 0ul);
  memcpy((void*)0x20000080,
         "\x8d\xa0\x25\x01\x92\xf0\x9c\x09\xc7\xe7\x28\xd4\x92\x30\x53\x8d\x39"
         "\x3e\x88\xd0\x03\x80\xc1\x2e\x07\x6f\x8f\x47\x69\x3b\x61\xda\xd5\x24"
         "\xe3\xed\xc4\x65\x22\xd1\xfd\x57\xc6\x52\x4c\x4f\x25\xa7\xd2\x2c\x25"
         "\xc7\x0c\x6c\xfe\xaf\x67\xf7\xec\x2a\xd0\x90\xf7\xe8\x82\xa0\x1c\x83"
         "\xa9\x62\xe1\xe3\xae\x8e\x29\xec\x71\xf5\x98\x11\xd4\xf8\xa4\x04\xdc"
         "\xeb\x9d\xaf\x59\x57\x79\x08\x38\x63\xcc\xa2\x6b\x2d\x81\x4f\x48\x61"
         "\x66\xbf\x58\x77\x70\x19\xc6\x41\x14\x91\xb9\x37\x25\x15\x16\xa5\x64"
         "\x85\x9e\x41\xcf\x82\x46\x3e\x89\xe4\x6a\x4e\xe4\x6a\x33\x2f\x83\x64"
         "\xb0\xf7\xac\x63\xba\x3a",
         142);
  memcpy((void*)0x200006c0, "./file0\000", 8);
  memcpy((void*)0x20000b00, "nfs\000", 4);
  syscall(__NR_mount, 0x20000080ul, 0x200006c0ul, 0x20000b00ul, 0ul,
          0x20000000ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  setup_leak();
  loop();
  return 0;
}
