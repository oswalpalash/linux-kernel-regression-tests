// https://syzkaller.appspot.com/bug?id=caed28f292ccd32eef950b27d68cf16852303b7f
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

#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_sendmsg
#define __NR_sendmsg 370
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10, 3, 0xc);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000280 = 0;
  *(uint32_t*)0x20000284 = 8;
  *(uint32_t*)0x20000288 = 0x20000240;
  *(uint32_t*)0x20000240 = 0x20000100;
  memcpy((void*)0x20000100,
         "\x14\x00\x00\x00\x10\x00\x00\x00\x1e\x6c\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x0a\x20\x00\x00\x00\x00\x0a\x01\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x09\x00\x01\x00\x73\x79\x7a\x30\x00\x00\x00"
         "\x00\x38\x00\x00\x00\x12\x0a\x01\x05\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x04\x00\x04\x80\x09\x00\x02\x00\x00\x39\x7d\x00\x00"
         "\x00\x00\x00\x09\x00\x01\x00\x73\x79\x7a\x30\x00\x00\x00\x00\x08\x00"
         "\x03\x40\x00\x00\x00\x01\x14\x00\x00\x00\x11\x00\xdf\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x0a",
         128);
  *(uint32_t*)0x20000244 = 0x80;
  *(uint32_t*)0x2000028c = 1;
  *(uint32_t*)0x20000290 = 0;
  *(uint32_t*)0x20000294 = 0;
  *(uint32_t*)0x20000298 = 0;
  syscall(__NR_sendmsg, (intptr_t)r[0], 0x20000280, 0);
  res = syscall(__NR_socket, 0x10, 3, 0xc);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x20000c40 = 0;
  *(uint32_t*)0x20000c44 = 0;
  *(uint32_t*)0x20000c48 = 0x20000000;
  *(uint32_t*)0x20000000 = 0x200000c0;
  memcpy((void*)0x200000c0,
         "\x14\x00\x00\x00\x10\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x0a\x4c\x00\x00\x00\x09\x0a\xc9\xdf\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x08\x00\x05\x40\x00\x00\x00\x28\x09\x00\x01"
         "\x00\x73\x79\x7a\x30\x00\x00\x00\x00\x08\x00\x0a\x40\x00\x00\x00\x00"
         "\x19\x00\x02\x00\x73\x79\x7a\x31\x00\x00\x00\x00\x08\x00\x03\x40\x00"
         "\x00\x00\x08\x08\x00\x06\x40\xff\xff\xff\x00\x14\x00\x00\x00\x11\x00"
         "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0a",
         116);
  *(uint32_t*)0x20000004 = 0x74;
  *(uint32_t*)0x20000c4c = 1;
  *(uint32_t*)0x20000c50 = 0;
  *(uint32_t*)0x20000c54 = 0;
  *(uint32_t*)0x20000c58 = 0;
  syscall(__NR_sendmsg, (intptr_t)r[1], 0x20000c40, 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
