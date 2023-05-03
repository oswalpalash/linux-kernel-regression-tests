// https://syzkaller.appspot.com/bug?id=948d78a059fd4a4e6f1a5fc16a56261ed2c49f30
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

void execute_one(void)
{
  *(uint32_t*)0x20000640 = 1;
  *(uint32_t*)0x20000644 = 0x80;
  *(uint8_t*)0x20000648 = 0;
  *(uint8_t*)0x20000649 = 0;
  *(uint8_t*)0x2000064a = 0;
  *(uint8_t*)0x2000064b = 0;
  *(uint32_t*)0x2000064c = 0;
  *(uint64_t*)0x20000650 = 2;
  *(uint64_t*)0x20000658 = 4;
  *(uint64_t*)0x20000660 = 0;
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 0, 0, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 1, 1, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 0, 2, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 0, 3, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 0, 4, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 1, 5, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 0, 6, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 0, 7, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 1, 8, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 0, 9, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 0, 10, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 0, 11, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 0, 12, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 1, 13, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 0, 14, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 0, 15, 2);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 0, 17, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 0, 18, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 0, 19, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 0, 20, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 0, 21, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 0, 22, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 0, 23, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 0, 24, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 1, 25, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 1, 26, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 0, 27, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 0, 28, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 1, 29, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 0, 30, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 0, 31, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 1, 32, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 0, 33, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 0, 34, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 0, 35, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 1, 36, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 1, 37, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 0, 38, 26);
  *(uint32_t*)0x20000670 = 0;
  *(uint32_t*)0x20000674 = 0;
  *(uint64_t*)0x20000678 = 0;
  *(uint64_t*)0x20000680 = 0;
  *(uint64_t*)0x20000688 = 0;
  *(uint64_t*)0x20000690 = 0;
  *(uint32_t*)0x20000698 = 0;
  *(uint32_t*)0x2000069c = 7;
  *(uint64_t*)0x200006a0 = 0;
  *(uint32_t*)0x200006a8 = 0;
  *(uint16_t*)0x200006ac = 9;
  *(uint16_t*)0x200006ae = 0;
  *(uint32_t*)0x200006b0 = 0;
  *(uint32_t*)0x200006b4 = 0;
  *(uint64_t*)0x200006b8 = 0xffffffffffffff4f;
  syscall(__NR_perf_event_open, 0x20000640ul, 0, 0ul, -1, 8ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  loop();
  return 0;
}
