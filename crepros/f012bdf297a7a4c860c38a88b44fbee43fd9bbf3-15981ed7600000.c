// https://syzkaller.appspot.com/bug?id=f012bdf297a7a4c860c38a88b44fbee43fd9bbf3
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

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  memcpy((void*)0x20000040, "./file0\000", 8);
  syscall(__NR_mkdir, 0x20000040, 0);
  res = syscall(__NR_pipe2, 0x20000240, 0);
  if (res != -1) {
    r[0] = *(uint32_t*)0x20000240;
    r[1] = *(uint32_t*)0x20000244;
  }
  memcpy((void*)0x200008c0,
         "\x15\x00\x00\x00\x65\xff\xff\x01\x80\x00\x00\x08\x00\x39\x50\x32\x30"
         "\x30\x30\x2e\x4c\xc7\x45\x02\xf9\x87\xc2\xce\xc6\x50\x4d\xf6\xea\xd7"
         "\x4e\xd8\xa6\x0a\xb5\x63\xe9\x8b\x4b\x2a\x3d\x27\xa7\x08\x2d\xbb\x78"
         "\xab\xd5\x5f\xba\x3d\xc8\x2f\x01\xbf\xb4\xa6\x7c\xf6\x21\xd6\x23\x45"
         "\x55\xc0\x8d\xc5\x40\x47\x37\x53\xcd\x89\xe9\xb0\x8e\x3f\x59\x72\xfe"
         "\x9c\xa1\x62\xb1\x23\xe1\x92\xe8\xc8\x9c\x9d\xd8\x1c\x79\x6f\x27\xf5"
         "\x37\xcc\x5a\x3f\xb5\x4a\xff\x8e\xaf\xf4\xf6\xb5\x9c\x41\x70\x5b\x96"
         "\xa6\x71\x1d\x46\x79\x07\x9d\xaf\xc7\xe7\xa3\xb6\x19\x06\x4f\x2d\x03"
         "\xab\xad\x04\x6d\x25\x79\x47\x02\x6b\xf7\x18\x21\x75\x4a\xb2\xac\x08"
         "\x14\x10\xe7\x23\x03\xbc\x39",
         160);
  syscall(__NR_write, r[1], 0x200008c0, 0x15);
  res = syscall(__NR_dup, r[1]);
  if (res != -1)
    r[2] = res;
  *(uint32_t*)0x20000100 = 0x18;
  *(uint32_t*)0x20000104 = 0;
  *(uint64_t*)0x20000108 = 0;
  *(uint64_t*)0x20000110 = 0;
  syscall(__NR_write, r[2], 0x20000100, 0x18);
  memcpy((void*)0x20000000, "./file0\000", 8);
  memcpy((void*)0x20000140, "9p\000", 3);
  memcpy((void*)0x20000480, "trans=fd,", 9);
  memcpy((void*)0x20000489, "rfdno", 5);
  *(uint8_t*)0x2000048e = 0x3d;
  sprintf((char*)0x2000048f, "0x%016llx", (long long)r[0]);
  *(uint8_t*)0x200004a1 = 0x2c;
  memcpy((void*)0x200004a2, "wfdno", 5);
  *(uint8_t*)0x200004a7 = 0x3d;
  sprintf((char*)0x200004a8, "0x%016llx", (long long)r[1]);
  *(uint8_t*)0x200004ba = 0x2c;
  *(uint8_t*)0x200004bb = 0;
  syscall(__NR_mount, 0, 0x20000000, 0x20000140, 0, 0x20000480);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  setup_leak();
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
