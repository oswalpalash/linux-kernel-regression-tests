// https://syzkaller.appspot.com/bug?id=4cf5ee79b52a4797c5bd40a58bd6ab243d40de48
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

unsigned long long procid;

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

uint64_t r[1] = {0xffffffffffffffff};

void execute_one(void)
{
  long res = 0;
  memcpy((void*)0x20000140, "/dev/swradio#", 14);
  res = syz_open_dev(0x20000140, 0, 2);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000000 = 0x800000;
  *(uint32_t*)0x20000004 = 0xa;
  *(uint32_t*)0x20000008 = 1;
  *(uint32_t*)0x20000010 = 0xb;
  memcpy((void*)0x20000018,
         "\xdb\xf7\x69\xcc\x6f\x48\x89\x6c\x3a\x01\xe7\x71\xda\x50\xbb\xf1\xdc"
         "\xb4\xa7\x22\x1d\x91\xef\x5e\x85\xc8\x0f\xa8\xdf\x6e\xea\xdb\x67\x14"
         "\x0c\xfe\xb6\x96\x0f\x41\xce\xb8\x14\xf2\xb6\x79\x6d\x56\xaa\xfd\x17"
         "\x4a\xd9\x16\x85\x9b\xb2\xb6\x50\x14\x09\x79\xac\xbc\xc4\xc1\x6e\x98"
         "\x5f\x68\xeb\x68\x69\x59\xe4\x78\x20\x49\xbd\x07\x70\x6c\xc4\x34\x1c"
         "\xe2\xc6\x20\xc9\x5c\x55\x4f\x48\x30\xf1\x42\xa3\xbf\xd1\x93\xc1\x36"
         "\x00\x01\xd2\xfc\x13\xa4\xae\x67\x52\x17\x49\xfa\x77\xe7\xa5\x83\x3b"
         "\x23\xd6\x3e\xa8\x18\x89\x3c\x97\x80\xa8\xcd\x92\xd3\xfc\x14\xec\xa1"
         "\xa6\xbd\xdd\x1a\x91\xa4\x9b\x8c\x77\x50\x4f\xd2\x68\x3e\x23\x89\x65"
         "\x3b\x12\xc6\x0e\xe3\x95\x9c\xc5\x73\x64\x85\x33\x3d\xbc\xc5\x3e\x20"
         "\x7a\x1a\x63\x8d\x79\x15\x1e\x7e\xda\x35\xb7\x84\xd6\x18\x3f\x0f\xa8"
         "\x5d\x1c\x03\xff\x76\x1d\xc0\x2f\x19\x57\x15\x73\x34",
         200);
  *(uint32_t*)0x200000e0 = 0;
  *(uint32_t*)0x200000e4 = 0;
  *(uint32_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000ec = 0;
  *(uint32_t*)0x200000f0 = 0;
  *(uint32_t*)0x200000f4 = 0;
  *(uint32_t*)0x200000f8 = 0;
  *(uint32_t*)0x200000fc = 0;
  syscall(__NR_ioctl, r[0], 0xc100565c, 0x20000000);
  syscall(__NR_mmap, 0x20a00000, 0x600000, -1, 0x11, r[0], 0x20a00000);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
