// https://syzkaller.appspot.com/bug?id=2f0c664ab14fb7dfaff18ee81a1b21548c03ce3b
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

uint64_t r[1] = {0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  res = syscall(__NR_socket, /*domain=*/0x10ul, /*type=*/3ul, /*proto=*/0);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000300 = 0;
  *(uint32_t*)0x20000308 = 0;
  *(uint64_t*)0x20000310 = 0x200002c0;
  *(uint64_t*)0x200002c0 = 0x20001c40;
  *(uint32_t*)0x20001c40 = 0xf0;
  *(uint16_t*)0x20001c44 = 0x30;
  *(uint16_t*)0x20001c46 = 1;
  *(uint32_t*)0x20001c48 = 0;
  *(uint32_t*)0x20001c4c = 0;
  *(uint8_t*)0x20001c50 = 0;
  *(uint8_t*)0x20001c51 = 0;
  *(uint16_t*)0x20001c52 = 0;
  *(uint16_t*)0x20001c54 = 0xdc;
  *(uint16_t*)0x20001c56 = 1;
  *(uint16_t*)0x20001c58 = 0x6c;
  STORE_BY_BITMASK(uint16_t, , 0x20001c5a, 1, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20001c5b, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20001c5b, 0, 7, 1);
  *(uint16_t*)0x20001c5c = 0xb;
  *(uint16_t*)0x20001c5e = 1;
  memcpy((void*)0x20001c60, "police\000", 7);
  *(uint16_t*)0x20001c68 = 0x40;
  STORE_BY_BITMASK(uint16_t, , 0x20001c6a, 2, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20001c6b, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20001c6b, 1, 7, 1);
  *(uint16_t*)0x20001c6c = 0x3c;
  *(uint16_t*)0x20001c6e = 1;
  *(uint32_t*)0x20001c70 = 0;
  *(uint32_t*)0x20001c74 = 0;
  *(uint32_t*)0x20001c78 = 0;
  *(uint32_t*)0x20001c7c = 0;
  *(uint32_t*)0x20001c80 = 0;
  *(uint8_t*)0x20001c84 = 0;
  *(uint8_t*)0x20001c85 = 0;
  *(uint16_t*)0x20001c86 = 0;
  *(uint16_t*)0x20001c88 = 0;
  *(uint16_t*)0x20001c8a = 0;
  *(uint32_t*)0x20001c8c = 0;
  *(uint8_t*)0x20001c90 = 0;
  *(uint8_t*)0x20001c91 = 0;
  *(uint16_t*)0x20001c92 = 0;
  *(uint16_t*)0x20001c94 = 0;
  *(uint16_t*)0x20001c96 = 0;
  *(uint32_t*)0x20001c98 = 0;
  *(uint32_t*)0x20001c9c = 0;
  *(uint32_t*)0x20001ca0 = 0;
  *(uint32_t*)0x20001ca4 = 0;
  *(uint16_t*)0x20001ca8 = 4;
  *(uint16_t*)0x20001caa = 6;
  *(uint16_t*)0x20001cac = 0xc;
  *(uint16_t*)0x20001cae = 7;
  *(uint32_t*)0x20001cb0 = 0;
  *(uint32_t*)0x20001cb4 = 0;
  *(uint16_t*)0x20001cb8 = 0xc;
  *(uint16_t*)0x20001cba = 8;
  *(uint32_t*)0x20001cbc = 0;
  *(uint32_t*)0x20001cc0 = 0;
  *(uint16_t*)0x20001cc4 = 0x6c;
  STORE_BY_BITMASK(uint16_t, , 0x20001cc6, 2, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20001cc7, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20001cc7, 0, 7, 1);
  *(uint16_t*)0x20001cc8 = 0xb;
  *(uint16_t*)0x20001cca = 1;
  memcpy((void*)0x20001ccc, "police\000", 7);
  *(uint16_t*)0x20001cd4 = 0x40;
  STORE_BY_BITMASK(uint16_t, , 0x20001cd6, 2, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20001cd7, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20001cd7, 1, 7, 1);
  *(uint16_t*)0x20001cd8 = 0x3c;
  *(uint16_t*)0x20001cda = 1;
  *(uint32_t*)0x20001cdc = 1;
  *(uint32_t*)0x20001ce0 = 0;
  *(uint32_t*)0x20001ce4 = 0;
  *(uint32_t*)0x20001ce8 = 0;
  *(uint32_t*)0x20001cec = 0;
  *(uint8_t*)0x20001cf0 = 0;
  *(uint8_t*)0x20001cf1 = 0;
  *(uint16_t*)0x20001cf2 = 0;
  *(uint16_t*)0x20001cf4 = 0;
  *(uint16_t*)0x20001cf6 = 0;
  *(uint32_t*)0x20001cf8 = 0;
  *(uint8_t*)0x20001cfc = 0;
  *(uint8_t*)0x20001cfd = 0;
  *(uint16_t*)0x20001cfe = 0;
  *(uint16_t*)0x20001d00 = 0;
  *(uint16_t*)0x20001d02 = 0;
  *(uint32_t*)0x20001d04 = 0;
  *(uint32_t*)0x20001d08 = 0;
  *(uint32_t*)0x20001d0c = 0;
  *(uint32_t*)0x20001d10 = 0;
  *(uint16_t*)0x20001d14 = 4;
  *(uint16_t*)0x20001d16 = 6;
  *(uint16_t*)0x20001d18 = 0xc;
  *(uint16_t*)0x20001d1a = 7;
  *(uint32_t*)0x20001d1c = 0;
  *(uint32_t*)0x20001d20 = 0;
  *(uint16_t*)0x20001d24 = 0xc;
  *(uint16_t*)0x20001d26 = 8;
  *(uint32_t*)0x20001d28 = 0;
  *(uint32_t*)0x20001d2c = 0;
  *(uint64_t*)0x200002c8 = 0xf0;
  *(uint64_t*)0x20000318 = 1;
  *(uint64_t*)0x20000320 = 0;
  *(uint64_t*)0x20000328 = 0;
  *(uint32_t*)0x20000330 = 0;
  syscall(__NR_sendmsg, /*fd=*/r[0], /*msg=*/0x20000300ul, /*f=*/0ul);
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
  loop();
  return 0;
}
