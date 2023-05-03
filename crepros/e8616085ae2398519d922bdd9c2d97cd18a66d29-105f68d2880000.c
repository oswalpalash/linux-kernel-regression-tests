// https://syzkaller.appspot.com/bug?id=e8616085ae2398519d922bdd9c2d97cd18a66d29
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

uint64_t r[5] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff, 0x0};

void execute_one(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x200000000000011ul, 0x4000000000080002ul, 0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0);
  if (res != -1)
    r[2] = res;
  res = syscall(__NR_socket, 0x10ul, 0x803ul, 0);
  if (res != -1)
    r[3] = res;
  *(uint64_t*)0x20000580 = 0;
  *(uint32_t*)0x20000588 = 0;
  *(uint64_t*)0x20000590 = 0x20000540;
  *(uint64_t*)0x20000540 = 0;
  *(uint64_t*)0x20000548 = 0x1c;
  *(uint64_t*)0x20000598 = 1;
  *(uint64_t*)0x200005a0 = 0;
  *(uint64_t*)0x200005a8 = 0;
  *(uint32_t*)0x200005b0 = 0;
  syscall(__NR_sendmsg, r[3], 0x20000580ul, 0ul);
  *(uint32_t*)0x20000080 = 0x14;
  res = syscall(__NR_getsockname, r[3], 0x20000600ul, 0x20000080ul);
  if (res != -1)
    r[4] = *(uint32_t*)0x20000604;
  *(uint64_t*)0x20000040 = 0;
  *(uint32_t*)0x20000048 = 0;
  *(uint64_t*)0x20000050 = 0x20000000;
  *(uint64_t*)0x20000000 = 0x20000340;
  memcpy((void*)0x20000340,
         "\x3c\x00\x00\x00\x10\x00\x85\x06\x00\x00\x20\x00\xfe\x61\x22\x33\xca"
         "\x00\x08\x00",
         20);
  *(uint32_t*)0x20000354 = r[4];
  memcpy(
      (void*)0x20000358,
      "\x23\x77\xf2\x9e\x25\x21\x55\xb2\x1c\x00\x12\x00\x0c\x00\x01\x00\x62\x6f"
      "\x6e\x64\x00\x00\x00\x00\x0c\x00\x02\x00\x08\x00\x01\x00\x01\x34\xe7\x30"
      "\x70\x75\xa7\xcc\x6d\x2d\xba\x6e\x4d\xce\x25\xf1\x89\x68\xdd\x3d\x6f\x77"
      "\x19\x9c\xd0\x6d\x7a\x4c\xfc\xdc\x99\xdc\xfd\x5e\xc3\xf3\xe3\xd9\x8b\xe8"
      "\xa8\xba\xc2\xdc\xc4\x14\xb5\x8d\xda\x48\xb3\xea\x35\x41\x1d\x5b\x11\x2c"
      "\x26\xf3\x1b\x35\x29\x82\xf5\x5b\xe4\x46\xb3\xdd\x47\xe4\x35\x95\x42\x52"
      "\x21\x38\x28\xba\x98\xa1\xbc\x36\x32\x78\xf8\xbd\x13\xad\x74\x6b\xb8\xed"
      "\xad\x61\x91\x62\xf5\xd1\x89\x2e\x9f\xa4\x2e\x4f\xe2\xb6\x0f\x5f\xe2\xbb"
      "\x96\x3f\x08\xd6\x69\x68\x20\xad\xe9\xcf\xf2\xb2\xde\xb9\x1c\xe5\x65\x71"
      "\x68\xa9\x0d\xc5\x23\x0e\x33\xb8\xc2\x6c\xd9\x25\xc3\x13\x66\xa2\xae\x33"
      "\x9f\x12\xba\x89\x66\xbe\x14\x39\xce\xc6\x35\xb0\x8c\x0a\x97\x49\x0b\x13"
      "\x3a\x5b\x73\x60\xb5\x93\x47\x83\x3f\xc9\x5a\x7b\xf3\xdc\x9b\xc6\x47\x41"
      "\xde\x1a\x6e\x83\xc9\xbd\xfd\xfd\x0b\xaa\xbe\xc9\x81\x09\x9b\xb3\xdb\xd6"
      "\x4a\x7e\x79\x79\xcf\xb7\x93\x5a\xff\xbc\xda\x49\x19\x0b\x7e\xc9\xbc\x1e"
      "\x89\xd6\xcc\xed\xec\x20\xf9\x1b\x57\x1e\x6f\xc0\x49\xba\x82\x82\x1b\x26"
      "\xca\x4f\x85\xf4\xb0\x3f\x70\xb1\x76\xb4\x3d\xe9\x15\xbe\xc7\x6e\x40\x5b"
      "\xce\x49\xa4\xb4\x6e\xc7\x45\xb5\x1f\x36\x28\x29\x16\xb7\x7d\x7f\x91\x3a"
      "\x6a\xfd\x68\x13\xdf\x2c",
      312);
  *(uint64_t*)0x20000008 = 0x3c;
  *(uint64_t*)0x20000058 = 1;
  *(uint64_t*)0x20000060 = 0;
  *(uint64_t*)0x20000068 = 0;
  *(uint32_t*)0x20000070 = 0;
  syscall(__NR_sendmsg, r[2], 0x20000040ul, 0ul);
  *(uint64_t*)0x20000240 = 0;
  *(uint32_t*)0x20000248 = 0;
  *(uint64_t*)0x20000250 = 0x20000180;
  *(uint64_t*)0x20000180 = 0x20000780;
  *(uint32_t*)0x20000780 = 0x58;
  *(uint16_t*)0x20000784 = 0x10;
  *(uint16_t*)0x20000786 = 0xff1f;
  *(uint32_t*)0x20000788 = 0;
  *(uint32_t*)0x2000078c = 0;
  *(uint8_t*)0x20000790 = 0;
  *(uint8_t*)0x20000791 = 0;
  *(uint16_t*)0x20000792 = 0;
  *(uint32_t*)0x20000794 = 0;
  *(uint32_t*)0x20000798 = 0x800;
  *(uint32_t*)0x2000079c = 0;
  *(uint16_t*)0x200007a0 = 0x28;
  STORE_BY_BITMASK(uint16_t, , 0x200007a2, 0x12, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200007a3, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200007a3, 1, 7, 1);
  *(uint16_t*)0x200007a4 = 8;
  *(uint16_t*)0x200007a6 = 1;
  memcpy((void*)0x200007a8, "gre\000", 4);
  *(uint16_t*)0x200007ac = 0x1c;
  STORE_BY_BITMASK(uint16_t, , 0x200007ae, 2, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200007af, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200007af, 1, 7, 1);
  *(uint16_t*)0x200007b0 = 8;
  *(uint16_t*)0x200007b2 = 6;
  *(uint32_t*)0x200007b4 = htobe32(-1);
  *(uint16_t*)0x200007b8 = 5;
  *(uint16_t*)0x200007ba = 9;
  *(uint8_t*)0x200007bc = 8;
  *(uint16_t*)0x200007c0 = 8;
  *(uint16_t*)0x200007c2 = 5;
  *(uint32_t*)0x200007c4 = 8;
  *(uint16_t*)0x200007c8 = 8;
  *(uint16_t*)0x200007ca = 0xa;
  *(uint32_t*)0x200007cc = r[4];
  *(uint16_t*)0x200007d0 = 8;
  *(uint16_t*)0x200007d2 = 0x1b;
  *(uint32_t*)0x200007d4 = 0x8000;
  *(uint64_t*)0x20000188 = 0x58;
  *(uint64_t*)0x20000258 = 1;
  *(uint64_t*)0x20000260 = 0;
  *(uint64_t*)0x20000268 = 0;
  *(uint32_t*)0x20000270 = 0;
  syscall(__NR_sendmsg, r[1], 0x20000240ul, 0x4004000ul);
  *(uint16_t*)0x200000c0 = 0x11;
  *(uint16_t*)0x200000c2 = htobe16(0);
  *(uint32_t*)0x200000c4 = r[4];
  *(uint16_t*)0x200000c8 = 1;
  *(uint8_t*)0x200000ca = 0;
  *(uint8_t*)0x200000cb = 6;
  memset((void*)0x200000cc, 170, 5);
  *(uint8_t*)0x200000d1 = 0xbb;
  memset((void*)0x200000d2, 0, 2);
  syscall(__NR_bind, r[0], 0x200000c0ul, 0x14ul);
  *(uint64_t*)0x20000300 = 0;
  *(uint32_t*)0x20000308 = 0;
  *(uint64_t*)0x20000310 = 0x200002c0;
  *(uint64_t*)0x200002c0 = 0;
  *(uint64_t*)0x200002c8 = 0;
  *(uint64_t*)0x20000318 = 1;
  *(uint64_t*)0x20000320 = 0;
  *(uint64_t*)0x20000328 = 0;
  *(uint32_t*)0x20000330 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000300ul, 0ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  loop();
  return 0;
}
