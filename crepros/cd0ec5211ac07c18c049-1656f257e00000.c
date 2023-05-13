// https://syzkaller.appspot.com/bug?id=3d97ba93fb3566000c1c59691ea427370d33ea1b
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

uint64_t r[1] = {0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000080 = 0;
  *(uint32_t*)0x20000088 = 0xd8ff0500;
  *(uint64_t*)0x20000090 = 0x20000100;
  *(uint64_t*)0x20000100 = 0x200001c0;
  *(uint32_t*)0x200001c0 = 0x3c;
  *(uint16_t*)0x200001c4 = 0x10;
  *(uint16_t*)0x200001c6 = 0x401;
  *(uint32_t*)0x200001c8 = 0;
  *(uint32_t*)0x200001cc = 0;
  *(uint8_t*)0x200001d0 = 0;
  *(uint8_t*)0x200001d1 = 0;
  *(uint16_t*)0x200001d2 = 0;
  *(uint32_t*)0x200001d4 = 0;
  *(uint32_t*)0x200001d8 = 0x3d2;
  *(uint32_t*)0x200001dc = 0;
  *(uint16_t*)0x200001e0 = 0x14;
  STORE_BY_BITMASK(uint16_t, , 0x200001e2, 0x12, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200001e3, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200001e3, 1, 7, 1);
  *(uint16_t*)0x200001e4 = 0xb;
  *(uint16_t*)0x200001e6 = 1;
  memcpy((void*)0x200001e8, "bridge\000", 7);
  *(uint16_t*)0x200001f0 = 4;
  STORE_BY_BITMASK(uint16_t, , 0x200001f2, 2, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200001f3, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200001f3, 1, 7, 1);
  *(uint16_t*)0x200001f4 = 8;
  *(uint16_t*)0x200001f6 = 0xa;
  *(uint32_t*)0x200001f8 = 0;
  *(uint64_t*)0x20000108 = 0x3c;
  *(uint64_t*)0x20000098 = 1;
  *(uint64_t*)0x200000a0 = 0;
  *(uint64_t*)0x200000a8 = 0;
  *(uint32_t*)0x200000b0 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000080ul, 0ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0ul);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
