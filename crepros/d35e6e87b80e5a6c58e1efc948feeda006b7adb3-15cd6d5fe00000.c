// https://syzkaller.appspot.com/bug?id=d35e6e87b80e5a6c58e1efc948feeda006b7adb3
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

#ifndef __NR_bind
#define __NR_bind 361
#endif
#ifndef __NR_getsockname
#define __NR_getsockname 367
#endif
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

uint64_t r[3] = {0xffffffffffffffff, 0x0, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x11, 3, 0);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20000080 = 0x11;
  memcpy((void*)0x20000082,
         "\x00\x00\x01\x00\x00\x00\x00\x00\x08\x00\x44\x94\x4e\xeb\xa7\x1a\x49"
         "\x76\xe2\x52\x92\x2c\xb1\x8f\x6e\x2e\x2a\xba\x00\x00\x00\x01\x2e\x0b"
         "\x38\x36\x00\x54\x04\xb0\xe0\x30\x1a\x4c\xe8\x75\xf2\xe3\xff\x5f\x16"
         "\x3e\xe3\x40\xb7\x67\x95\x00\x80\x00\x00\x00\x00\x00\x00\x01\x01\x01"
         "\x3c\x58\x11\x03\x9e\x15\x77\x50\x27\xec\xce\x66\xfd\x79\x2b\xbf\x0e"
         "\x5b\xf5\xff\x1b\x08\x16\xf3\xf6\xdb\x1c\x00\x01\x00\x00\x00\x00\x00"
         "\x00\x00\x49\x74\x00\x00\x00\x00\x00\x00\x00\x06\xad\x8e\x5e\xcc\x32"
         "\x6d\x3a\x09\xff\xc2\xc6\x54",
         126);
  syscall(__NR_bind, (intptr_t)r[0], 0x20000080, 0x80);
  *(uint32_t*)0x20000000 = 0x14;
  res = syscall(__NR_getsockname, (intptr_t)r[0], 0x200003c0, 0x20000000);
  if (res != -1)
    r[1] = *(uint32_t*)0x200003c4;
  res = syscall(__NR_socket, 0x10, 3, 0);
  if (res != -1)
    r[2] = res;
  *(uint32_t*)0x200007c0 = 0;
  *(uint32_t*)0x200007c4 = 0;
  *(uint32_t*)0x200007c8 = 0x20000040;
  *(uint32_t*)0x20000040 = 0x20000200;
  *(uint32_t*)0x20000200 = 0x170;
  *(uint16_t*)0x20000204 = 0x24;
  *(uint16_t*)0x20000206 = 0xf0b;
  *(uint32_t*)0x20000208 = 0;
  *(uint32_t*)0x2000020c = 0;
  *(uint8_t*)0x20000210 = 0;
  *(uint8_t*)0x20000211 = 0;
  *(uint16_t*)0x20000212 = 0;
  *(uint32_t*)0x20000214 = r[1];
  *(uint16_t*)0x20000218 = 0;
  *(uint16_t*)0x2000021a = 0;
  *(uint16_t*)0x2000021c = -1;
  *(uint16_t*)0x2000021e = -1;
  *(uint16_t*)0x20000220 = 0;
  *(uint16_t*)0x20000222 = 0;
  *(uint16_t*)0x20000224 = 0xa;
  *(uint16_t*)0x20000226 = 1;
  memcpy((void*)0x20000228, "choke\000", 6);
  *(uint16_t*)0x20000230 = 0x140;
  *(uint16_t*)0x20000232 = 2;
  *(uint16_t*)0x20000234 = 0x14;
  *(uint16_t*)0x20000236 = 1;
  *(uint32_t*)0x20000238 = 0x23474499;
  *(uint32_t*)0x2000023c = 0xffffffe0;
  *(uint32_t*)0x20000240 = 0xede5;
  *(uint8_t*)0x20000244 = 0xf;
  *(uint8_t*)0x20000245 = 1;
  *(uint8_t*)0x20000246 = 0x17;
  *(uint8_t*)0x20000247 = 3;
  *(uint16_t*)0x20000248 = 0x104;
  *(uint16_t*)0x2000024a = 2;
  memcpy((void*)0x2000024c,
         "\xc3\x0c\x74\xc0\xf5\x19\x93\xe0\x95\x6d\x3b\xb2\xe1\x92\xbc\x52\xab"
         "\xcb\x6c\x7d\xf2\x3f\xaf\x7f\x5d\xf5\xea\xcc\x05\x54\x14\x81\x40\xea"
         "\xb0\xd8\x5c\x4a\x2d\x3c\x1b\x14\x5c\xdb\xcc\xd8\xb9\x7b\xd9\xe1\x67"
         "\xd1\x00\xfe\xff\xa9\x0d\xa5\xaa\x6a\x29\xb7\x5e\xc5\xba\x64\x47\xfb"
         "\x4d\x9e\xc6\x53\xe0\xd7\xc5\x01\x3c\xb0\xdf\x42\xf9\x5e\x0d\x43\x2c"
         "\xe1\x28\x4f\x76\x51\x18\x9e\xa4\x96\x88\x94\xa1\x21\x01\xf4\x61\x37"
         "\x44\xcf\xf5\x33\xa9\x64\xa2\x3f\xff\x84\x00\x1d\x49\x08\x9f\x2d\xfc"
         "\x07\x50\x71\xbb\x4b\x72\x78\xbc\x41\x19\x13\x66\xc2\x6c\x9d\xb2\xd3"
         "\x2f\x70\xa7\x5a\x8a\xb1\x4f\x22\x77\x8b\x14\x50\x02\x2c\x93\xed\x34"
         "\x2e\xb2\xe6\xc3\x37\x4c\x6b\xf6\x53\x1c\xb7\x86\xe6\x8a\xf3\x32\x2b"
         "\x11\x12\xc9\x82\x2c\x4e\x26\xee\xa3\x0b\x9f\x56\x3a\xa0\x90\x3e\xd1"
         "\xdf\x21\xb7\x8b\x9c\xe7\x60\x91\x07\xff\xc0\x23\x14\xfe\xb7\x2e\x57"
         "\x2c\x3f\xf6\x06\xc3\xa8\x0b\xea\x48\xed\x72\x90\xc9\x40\x92\x24\xcb"
         "\x10\xa0\x79\x02\xbb\xc8\x17\x32\x39\x20\xc0\xe5\xac\xa0\xd2\x3c\xcb"
         "\x44\xc1\x64\xfd\x71\xa2\xf9\xd7\x85\x57\xc8\x03\x80\x54\xdf\xa2\xc2"
         "\x35",
         256);
  *(uint16_t*)0x2000034c = 8;
  *(uint16_t*)0x2000034e = 3;
  *(uint32_t*)0x20000350 = 0xfff;
  *(uint16_t*)0x20000354 = 8;
  *(uint16_t*)0x20000356 = 3;
  *(uint32_t*)0x20000358 = 0x81;
  *(uint16_t*)0x2000035c = 0x14;
  *(uint16_t*)0x2000035e = 1;
  *(uint32_t*)0x20000360 = 0;
  *(uint32_t*)0x20000364 = 0;
  *(uint32_t*)0x20000368 = 0x81;
  *(uint8_t*)0x2000036c = 5;
  *(uint8_t*)0x2000036d = 0xc;
  *(uint8_t*)0x2000036e = 0x1c;
  *(uint8_t*)0x2000036f = 7;
  *(uint32_t*)0x20000044 = 0x170;
  *(uint32_t*)0x200007cc = 1;
  *(uint32_t*)0x200007d0 = 0;
  *(uint32_t*)0x200007d4 = 0;
  *(uint32_t*)0x200007d8 = 0;
  syscall(__NR_sendmsg, (intptr_t)r[2], 0x200007c0, 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000, 0x1000, 0, 0x32, -1, 0);
  syscall(__NR_mmap, 0x20000000, 0x1000000, 7, 0x32, -1, 0);
  syscall(__NR_mmap, 0x21000000, 0x1000, 0, 0x32, -1, 0);
  loop();
  return 0;
}
