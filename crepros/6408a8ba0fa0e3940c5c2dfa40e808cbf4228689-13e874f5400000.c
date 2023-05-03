// https://syzkaller.appspot.com/bug?id=6408a8ba0fa0e3940c5c2dfa40e808cbf4228689
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
#ifndef __NR_userfaultfd
#define __NR_userfaultfd 323
#endif

uint64_t r[3] = {0x0, 0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  long res = 0;
  res = syscall(__NR_io_setup, 0x400, 0x20000300);
  if (res != -1)
    r[0] = *(uint64_t*)0x20000300;
  memcpy((void*)0x200000c0, "/dev/md0", 9);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x200000c0, 0, 0);
  if (res != -1)
    r[1] = res;
  syscall(__NR_close, r[1]);
  syscall(__NR_userfaultfd, 0);
  *(uint64_t*)0x20000b00 = 0x200002c0;
  *(uint64_t*)0x200002c0 = 0;
  *(uint32_t*)0x200002c8 = 0;
  *(uint32_t*)0x200002cc = 0;
  *(uint16_t*)0x200002d0 = 5;
  *(uint16_t*)0x200002d2 = 0;
  *(uint32_t*)0x200002d4 = r[1];
  *(uint64_t*)0x200002d8 = 0;
  *(uint64_t*)0x200002e0 = 0;
  *(uint64_t*)0x200002e8 = 0;
  *(uint64_t*)0x200002f0 = 0;
  *(uint32_t*)0x200002f8 = 0;
  *(uint32_t*)0x200002fc = -1;
  syscall(__NR_io_submit, r[0], 1, 0x20000b00);
  res = syscall(__NR_userfaultfd, 0);
  if (res != -1)
    r[2] = res;
  *(uint64_t*)0x20000040 = 0xaa;
  *(uint64_t*)0x20000048 = 0x72;
  *(uint64_t*)0x20000050 = 0;
  syscall(__NR_ioctl, r[2], 0xc018aa3f, 0x20000040);
  syscall(__NR_read, r[2], 0x20009f9c, 0x64);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
