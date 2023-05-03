// https://syzkaller.appspot.com/bug?id=c203af981f0f7cc27debda75f35d852546cc6013
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

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0x0};

void execute_one(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x11ul, 2ul, 0x300);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_socket, 0x10ul, 0x803ul, 0);
  if (res != -1)
    r[2] = res;
  *(uint64_t*)0x20000300 = 0;
  *(uint32_t*)0x20000308 = 0;
  *(uint64_t*)0x20000310 = 0x200001c0;
  *(uint64_t*)0x200001c0 = 0;
  *(uint64_t*)0x200001c8 = 0x14;
  *(uint64_t*)0x20000318 = 1;
  *(uint64_t*)0x20000320 = 0;
  *(uint64_t*)0x20000328 = 0;
  *(uint32_t*)0x20000330 = 0;
  syscall(__NR_sendmsg, r[2], 0x20000300ul, 0ul);
  *(uint32_t*)0x200000c0 = 0x14;
  res = syscall(__NR_getsockname, r[2], 0x20000180ul, 0x200000c0ul);
  if (res != -1)
    r[3] = *(uint32_t*)0x20000184;
  *(uint64_t*)0x20000380 = 0;
  *(uint32_t*)0x20000388 = 0;
  *(uint64_t*)0x20000390 = 0x200002c0;
  *(uint64_t*)0x200002c0 = 0x200003c0;
  *(uint32_t*)0x200003c0 = 0x3c;
  *(uint16_t*)0x200003c4 = 0x10;
  *(uint16_t*)0x200003c6 = 0x437;
  *(uint32_t*)0x200003c8 = 0;
  *(uint32_t*)0x200003cc = 0;
  *(uint8_t*)0x200003d0 = 0;
  *(uint8_t*)0x200003d1 = 0;
  *(uint16_t*)0x200003d2 = 0;
  *(uint32_t*)0x200003d4 = r[3];
  *(uint32_t*)0x200003d8 = 0x50483;
  *(uint32_t*)0x200003dc = 0;
  *(uint16_t*)0x200003e0 = 0x1c;
  STORE_BY_BITMASK(uint16_t, , 0x200003e2, 0x12, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200003e3, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200003e3, 1, 7, 1);
  *(uint16_t*)0x200003e4 = 0xe;
  *(uint16_t*)0x200003e6 = 1;
  memcpy((void*)0x200003e8, "ip6erspan\000", 10);
  *(uint16_t*)0x200003f4 = 8;
  STORE_BY_BITMASK(uint16_t, , 0x200003f6, 2, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200003f7, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200003f7, 1, 7, 1);
  *(uint16_t*)0x200003f8 = 4;
  *(uint16_t*)0x200003fa = 0x12;
  *(uint64_t*)0x200002c8 = 0x3c;
  *(uint64_t*)0x20000398 = 1;
  *(uint64_t*)0x200003a0 = 0;
  *(uint64_t*)0x200003a8 = 0;
  *(uint32_t*)0x200003b0 = 0;
  syscall(__NR_sendmsg, r[1], 0x20000380ul, 0ul);
  *(uint32_t*)0x20000000 = 0x400;
  syscall(__NR_setsockopt, r[0], 0x107, 0x14, 0x20000000ul, 4ul);
  *(uint16_t*)0x20000040 = 0x11;
  *(uint16_t*)0x20000042 = htobe16(0);
  *(uint32_t*)0x20000044 = r[3];
  *(uint16_t*)0x20000048 = 1;
  *(uint8_t*)0x2000004a = 0;
  *(uint8_t*)0x2000004b = 6;
  memset((void*)0x2000004c, 255, 6);
  memset((void*)0x20000052, 0, 2);
  syscall(__NR_sendto, r[0], 0ul, 0ul, 0ul, 0x20000040ul, 0x14ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  loop();
  return 0;
}
