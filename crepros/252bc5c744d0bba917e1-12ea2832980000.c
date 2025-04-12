// https://syzkaller.appspot.com/bug?id=86615e769ae98523f2a5f89f41be5a6b1f14fc8a
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

static unsigned long long procid;

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
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  *(uint32_t*)0x20000a40 = 3;
  *(uint32_t*)0x20000a44 = 0xc;
  *(uint64_t*)0x20000a48 = 0x20000440;
  memcpy((void*)0x20000440,
         "\x18\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x18"
         "\x11\x00\x00",
         20);
  *(uint32_t*)0x20000454 = -1;
  memcpy((void*)0x20000458,
         "\x00\x00\x00\x00\x00\x00\x00\x00\xb7\x08\x00\x00\x00\x00\x00\x00\x7b"
         "\x8a\xf8\xff\x00\x00\x00\x00\xbf\xa2\x00\x00\x00\x00\x00\x00\x07\x02"
         "\x00\x00\xf8\xff\xff\xff\xb7\x03\x00\x00\x00\x00\x00\x00\xb7\x04\x00"
         "\x00\x00\x00\x00\x00\x85\x00\x00\x00\x57\x00\x00\x00\x95",
         65);
  *(uint64_t*)0x20000a50 = 0;
  *(uint32_t*)0x20000a58 = 0;
  *(uint32_t*)0x20000a5c = 0;
  *(uint64_t*)0x20000a60 = 0;
  *(uint32_t*)0x20000a68 = 0;
  *(uint32_t*)0x20000a6c = 0;
  memset((void*)0x20000a70, 0, 16);
  *(uint32_t*)0x20000a80 = 0;
  *(uint32_t*)0x20000a84 = 0x25;
  *(uint32_t*)0x20000a88 = -1;
  *(uint32_t*)0x20000a8c = 8;
  *(uint64_t*)0x20000a90 = 0;
  *(uint32_t*)0x20000a98 = 0;
  *(uint32_t*)0x20000a9c = 0x10;
  *(uint64_t*)0x20000aa0 = 0;
  *(uint32_t*)0x20000aa8 = 0;
  *(uint32_t*)0x20000aac = 0;
  *(uint32_t*)0x20000ab0 = 0;
  *(uint32_t*)0x20000ab4 = 0;
  *(uint64_t*)0x20000ab8 = 0;
  *(uint64_t*)0x20000ac0 = 0;
  *(uint32_t*)0x20000ac8 = 0x10;
  *(uint32_t*)0x20000acc = 0;
  syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x20000a40ul, /*size=*/0x90ul);
  *(uint32_t*)0x20000640 = 0x16;
  *(uint32_t*)0x20000644 = 0;
  *(uint32_t*)0x20000648 = 4;
  *(uint32_t*)0x2000064c = 0xff;
  *(uint32_t*)0x20000650 = 0;
  *(uint32_t*)0x20000654 = 1;
  *(uint32_t*)0x20000658 = 0;
  memset((void*)0x2000065c, 0, 16);
  *(uint32_t*)0x2000066c = 0;
  *(uint32_t*)0x20000670 = -1;
  *(uint32_t*)0x20000674 = 0;
  *(uint32_t*)0x20000678 = 0;
  *(uint32_t*)0x2000067c = 0;
  *(uint64_t*)0x20000680 = 0;
  res = syscall(__NR_bpf, /*cmd=*/0ul, /*arg=*/0x20000640ul, /*size=*/0x48ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000a40 = 3;
  *(uint32_t*)0x20000a44 = 0xc;
  *(uint64_t*)0x20000a48 = 0x20000440;
  memcpy((void*)0x20000440,
         "\x18\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x18"
         "\x11\x00\x00",
         20);
  *(uint32_t*)0x20000454 = r[0];
  *(uint64_t*)0x20000a50 = 0;
  *(uint32_t*)0x20000a58 = 0;
  *(uint32_t*)0x20000a5c = 0;
  *(uint64_t*)0x20000a60 = 0;
  *(uint32_t*)0x20000a68 = 0;
  *(uint32_t*)0x20000a6c = 0;
  memset((void*)0x20000a70, 0, 16);
  *(uint32_t*)0x20000a80 = 0;
  *(uint32_t*)0x20000a84 = 0x25;
  *(uint32_t*)0x20000a88 = -1;
  *(uint32_t*)0x20000a8c = 8;
  *(uint64_t*)0x20000a90 = 0;
  *(uint32_t*)0x20000a98 = 0;
  *(uint32_t*)0x20000a9c = 0x10;
  *(uint64_t*)0x20000aa0 = 0;
  *(uint32_t*)0x20000aa8 = 0;
  *(uint32_t*)0x20000aac = 0;
  *(uint32_t*)0x20000ab0 = 0;
  *(uint32_t*)0x20000ab4 = 0;
  *(uint64_t*)0x20000ab8 = 0;
  *(uint64_t*)0x20000ac0 = 0;
  *(uint32_t*)0x20000ac8 = 0x10;
  *(uint32_t*)0x20000acc = 0;
  syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x20000a40ul, /*size=*/0x90ul);
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
  const char* reason;
  (void)reason;
  for (procid = 0; procid < 5; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
