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
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_sendmsg
#define __NR_sendmsg 370
#endif
#ifndef __NR_sendto
#define __NR_sendto 369
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  long res = 0;
  res = syscall(__NR_socket, 0x10, 3, 0xc);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20d65000 = 0x20000040;
  *(uint16_t*)0x20000040 = 0x10;
  *(uint16_t*)0x20000042 = 0;
  *(uint32_t*)0x20000044 = 0;
  *(uint32_t*)0x20000048 = 0;
  *(uint32_t*)0x20d65004 = 0xc;
  *(uint32_t*)0x20d65008 = 0x20000100;
  *(uint32_t*)0x20000100 = 0x20000140;
  *(uint32_t*)0x20000140 = 0x1c;
  *(uint8_t*)0x20000144 = 2;
  *(uint8_t*)0x20000145 = 1;
  *(uint16_t*)0x20000146 = -1;
  *(uint32_t*)0x20000148 = 0;
  *(uint32_t*)0x2000014c = 0;
  *(uint8_t*)0x20000150 = 0;
  *(uint8_t*)0x20000151 = 0;
  *(uint16_t*)0x20000152 = htobe16(0);
  *(uint16_t*)0x20000154 = 8;
  *(uint16_t*)0x20000156 = 8;
  *(uint16_t*)0x20000158 = 4;
  *(uint16_t*)0x2000015a = 0;
  *(uint32_t*)0x20000104 = 0x1c;
  *(uint32_t*)0x20d6500c = 1;
  *(uint32_t*)0x20d65010 = 0;
  *(uint32_t*)0x20d65014 = 0;
  *(uint32_t*)0x20d65018 = 0;
  syscall(__NR_sendmsg, (long)r[0], 0x20d65000, 0);
  res = syscall(__NR_socket, 0xa, 0x802, 0x88);
  if (res != -1)
    r[1] = res;
  *(uint16_t*)0x20000340 = 0xa;
  *(uint16_t*)0x20000342 = htobe16(0x4e23);
  *(uint32_t*)0x20000344 = 0;
  *(uint8_t*)0x20000348 = 0xfe;
  *(uint8_t*)0x20000349 = 0x80;
  *(uint8_t*)0x2000034a = 0;
  *(uint8_t*)0x2000034b = 0;
  *(uint8_t*)0x2000034c = 0;
  *(uint8_t*)0x2000034d = 0;
  *(uint8_t*)0x2000034e = 0;
  *(uint8_t*)0x2000034f = 0;
  *(uint8_t*)0x20000350 = 0;
  *(uint8_t*)0x20000351 = 0;
  *(uint8_t*)0x20000352 = 0;
  *(uint8_t*)0x20000353 = 0;
  *(uint8_t*)0x20000354 = 0;
  *(uint8_t*)0x20000355 = 0;
  *(uint8_t*)0x20000356 = 0;
  *(uint8_t*)0x20000357 = 0x15;
  *(uint32_t*)0x20000358 = 0;
  syscall(__NR_sendto, (long)r[1], 0x20000080, 0, 0, 0x20000340, 0x1c);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
