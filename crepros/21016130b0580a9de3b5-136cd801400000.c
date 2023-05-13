// https://syzkaller.appspot.com/bug?id=9c57d3996565d35691fa44b2daf987259838ccce
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
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

static long syz_open_dev(long a0, long a1, long a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
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

#define SYZ_HAVE_SETUP_TEST 1
static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
}

#define SYZ_HAVE_RESET_TEST 1
static void reset_test()
{
  int fd;
  for (fd = 3; fd < 30; fd++)
    close(fd);
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
      reset_test();
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

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  long res = 0;
  res = syscall(__NR_socket, 0x10, 3, 0xc);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20d65000 = 0x20de2ff4;
  *(uint16_t*)0x20de2ff4 = 0x10;
  *(uint16_t*)0x20de2ff6 = 0;
  *(uint32_t*)0x20de2ff8 = 0;
  *(uint32_t*)0x20de2ffc = 0;
  *(uint32_t*)0x20d65008 = 0xc;
  *(uint64_t*)0x20d65010 = 0x20000000;
  *(uint64_t*)0x20000000 = 0x20000140;
  *(uint32_t*)0x20000140 = 0x24;
  *(uint8_t*)0x20000144 = 2;
  *(uint8_t*)0x20000145 = 1;
  *(uint16_t*)0x20000146 = -1;
  *(uint32_t*)0x20000148 = 0;
  *(uint32_t*)0x2000014c = 0;
  *(uint8_t*)0x20000150 = 0;
  *(uint8_t*)0x20000151 = 0;
  *(uint16_t*)0x20000152 = htobe16(0);
  *(uint16_t*)0x20000154 = 8;
  *(uint16_t*)0x20000156 = 0x15;
  *(uint32_t*)0x20000158 = 0;
  *(uint16_t*)0x2000015c = 8;
  *(uint16_t*)0x2000015e = 8;
  *(uint32_t*)0x20000160 = 0;
  *(uint64_t*)0x20000008 = 0x24;
  *(uint64_t*)0x20d65018 = 1;
  *(uint64_t*)0x20d65020 = 0;
  *(uint64_t*)0x20d65028 = 0;
  *(uint32_t*)0x20d65030 = 0;
  syscall(__NR_sendmsg, r[0], 0x20d65000, 0);
  res = syscall(__NR_socket, 0xa, 6, 0);
  if (res != -1)
    r[1] = res;
  *(uint16_t*)0x20000000 = 0xa;
  *(uint16_t*)0x20000002 = htobe16(0x4e20);
  *(uint32_t*)0x20000004 = 0;
  *(uint8_t*)0x20000008 = 0;
  *(uint8_t*)0x20000009 = 0;
  *(uint8_t*)0x2000000a = 0;
  *(uint8_t*)0x2000000b = 0;
  *(uint8_t*)0x2000000c = 0;
  *(uint8_t*)0x2000000d = 0;
  *(uint8_t*)0x2000000e = 0;
  *(uint8_t*)0x2000000f = 0;
  *(uint8_t*)0x20000010 = 0;
  *(uint8_t*)0x20000011 = 0;
  *(uint8_t*)0x20000012 = 0;
  *(uint8_t*)0x20000013 = 0;
  *(uint8_t*)0x20000014 = 0;
  *(uint8_t*)0x20000015 = 0;
  *(uint8_t*)0x20000016 = 0;
  *(uint8_t*)0x20000017 = 0;
  *(uint32_t*)0x20000018 = 0;
  syscall(__NR_bind, r[1], 0x20000000, 0x1c);
  res = syscall(__NR_socket, 2, 6, 0);
  if (res != -1)
    r[2] = res;
  syscall(__NR_listen, r[1], 6);
  *(uint16_t*)0x20e5c000 = 2;
  *(uint16_t*)0x20e5c002 = htobe16(0x4e20);
  *(uint32_t*)0x20e5c004 = htobe32(0x7f000001);
  *(uint8_t*)0x20e5c008 = 0;
  *(uint8_t*)0x20e5c009 = 0;
  *(uint8_t*)0x20e5c00a = 0;
  *(uint8_t*)0x20e5c00b = 0;
  *(uint8_t*)0x20e5c00c = 0;
  *(uint8_t*)0x20e5c00d = 0;
  *(uint8_t*)0x20e5c00e = 0;
  *(uint8_t*)0x20e5c00f = 0;
  syscall(__NR_connect, r[2], 0x20e5c000, 0x10);
  memcpy((void*)0x20000040, "/dev/amidi#", 12);
  syz_open_dev(0x20000040, 0, 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
