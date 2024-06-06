// https://syzkaller.appspot.com/bug?id=45d4976c97a271ac8afb8df81696e63002df98cb
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
  *(uint32_t*)0x200001c0 = 0x12;
  *(uint32_t*)0x200001c4 = 6;
  *(uint32_t*)0x200001c8 = 8;
  *(uint32_t*)0x200001cc = 2;
  *(uint32_t*)0x200001d0 = 0;
  *(uint32_t*)0x200001d4 = -1;
  *(uint32_t*)0x200001d8 = 0;
  memset((void*)0x200001dc, 0, 16);
  *(uint32_t*)0x200001ec = 0;
  *(uint32_t*)0x200001f0 = -1;
  *(uint32_t*)0x200001f4 = 0;
  *(uint32_t*)0x200001f8 = 0;
  *(uint32_t*)0x200001fc = 0;
  *(uint64_t*)0x20000200 = 0;
  res = syscall(__NR_bpf, /*cmd=*/0ul, /*arg=*/0x200001c0ul, /*size=*/0x48ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x200007c0 = 0x11;
  *(uint32_t*)0x200007c4 = 0xc;
  *(uint64_t*)0x200007c8 = 0x20000080;
  memcpy((void*)0x20000080,
         "\x18\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xb0\x77\x00\x18"
         "\x11\x00\x00",
         20);
  *(uint32_t*)0x20000094 = r[0];
  memcpy((void*)0x20000098,
         "\x00\x00\x00\x00\x00\x00\x00\x00\xb7\x08\x00\x00\x00\x00\x00\x00\x7b"
         "\x8a\xf8\xff\x00\x00\x00\x00\xbf\xa2\x00\x00\x00\x00\x00\x00\x07\x02"
         "\x00\x00\xf8\xff\xff\xff\xb7\x03\x00\x00\x08\x00\x00\x00\xb7\x04\x00"
         "\x00\x00\x00\x00\x00\x85\x00\x00\x00\x03\x00\x00\x00\x95",
         65);
  *(uint64_t*)0x200007d0 = 0x20000240;
  memcpy((void*)0x20000240, "syzkaller\000", 10);
  *(uint32_t*)0x200007d8 = 0;
  *(uint32_t*)0x200007dc = 0;
  *(uint64_t*)0x200007e0 = 0;
  *(uint32_t*)0x200007e8 = 0;
  *(uint32_t*)0x200007ec = 0;
  memset((void*)0x200007f0, 0, 16);
  *(uint32_t*)0x20000800 = 0;
  *(uint32_t*)0x20000804 = 0;
  *(uint32_t*)0x20000808 = -1;
  *(uint32_t*)0x2000080c = 0;
  *(uint64_t*)0x20000810 = 0;
  *(uint32_t*)0x20000818 = 0;
  *(uint32_t*)0x2000081c = 0;
  *(uint64_t*)0x20000820 = 0;
  *(uint32_t*)0x20000828 = 0;
  *(uint32_t*)0x2000082c = 0;
  *(uint32_t*)0x20000830 = 0;
  *(uint32_t*)0x20000834 = 0;
  *(uint64_t*)0x20000838 = 0;
  *(uint64_t*)0x20000840 = 0;
  *(uint32_t*)0x20000848 = 0;
  *(uint32_t*)0x2000084c = 0;
  res = syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x200007c0ul, /*size=*/0x90ul);
  if (res != -1)
    r[1] = res;
  *(uint64_t*)0x20000b40 = 0x200002c0;
  memcpy((void*)0x200002c0, "hrtimer_start\000", 14);
  *(uint32_t*)0x20000b48 = r[1];
  syscall(__NR_bpf, /*cmd=*/0x11ul, /*arg=*/0x20000b40ul, /*size=*/0x10ul);
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
