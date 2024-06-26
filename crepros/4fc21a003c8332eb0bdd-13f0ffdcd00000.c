// https://syzkaller.appspot.com/bug?id=9b10491371879700d6a21c15684c2232ff015084
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

static long syz_open_dev(volatile long a0, volatile long a1, volatile long a2)
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
      if (current_time_ms() - start < 5000) {
        continue;
      }
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  memcpy((void*)0x20000000, "/dev/dri/card#\000", 15);
  syz_open_dev(0x20000000, 1, 0);
  memcpy((void*)0x20000040, "/dev/kvm\000", 9);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000040ul, 0ul, 0ul);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_ioctl, r[0], 0xae01, 0ul);
  if (res != -1)
    r[1] = res;
  syscall(__NR_ioctl, r[1], 0xae60, 0);
  res = syscall(__NR_ioctl, r[1], 0xae41, 0ul);
  if (res != -1)
    r[2] = res;
  memcpy(
      (void*)0x20000580,
      "\x6c\xdd\x42\x37\xdd\x24\x5c\x84\x04\x72\x1e\xfd\xc9\xc8\xdc\x19\x64\x12"
      "\x5f\xa9\x6f\xa4\x2b\x75\xfb\x34\x88\xfd\x80\x15\xbb\xa4\xc8\x10\x36\xc9"
      "\x3a\x40\xc8\xa4\xd4\x41\x2a\x76\x3b\x00\x04\x00\x00\x00\x00\x00\x00\x3c"
      "\x5c\xa2\x06\xc0\x47\xec\xee\x37\x7a\xba\x09\xe7\xb8\x83\x78\xe3\xd6\x3a"
      "\x98\xfc\x19\x1f\x36\x1d\x26\x4f\xfa\x8b\x46\x48\x5f\x02\xba\xee\x1a\xb6"
      "\xb8\x15\x42\x52\x06\x61\x78\x86\x8d\x1e\xf4\xb5\x36\x5c\x5d\xc2\x6c\xa0"
      "\x97\xdd\xda\x7c\x21\xa9\x84\xc2\xb9\xca\x4b\xbb\x7a\x87\x16\x5c\x0c\x1d"
      "\xbc\x75\xd7\xea\x4d\xf1\x00\x00\x17\x4a\x3a\xc8\x69\x45\x25\x95\x2f\x44"
      "\x50\x0a\x1f\x0d\xb5\x09\xc3\x2c\xc7\xac\xe8\x42\xc2\x8f\x37\xf0\x6e\x4e"
      "\xa9\xf1\xe5\xf0\xc6\xc3\x79\xf9\xcc\x58\xbf\x69\xfc\xde\x31\x8e\xad\x48"
      "\x25\xaa\x1b\x6a\x83\x2d\x4e\x48\xcc\x41\xbb\x5a\x6b\xaa\x41\xd6\x14\xf6"
      "\xc8\x94\x1b\xee\x80\x59\x54\xa6\x2d\x19\x6a\x4e\x8d\x41\xf6\xb2\x12\x24"
      "\xb5\x7f\x53\x0d\x00\x00\xc1\xff\x53\xbf\x79\xa1\xf5\xc5\xdc\x34\xb2\x26"
      "\x2d\x66\xae\x79\x3b\x63\x04\xa3\x0b\x97\x07\x7f\x1c\x13\x10\x45\xcb\xc1"
      "\x1c\x45\x62\xd2\x2d\xb8\x8d\x0e\xdc\x5d\xae\xe1\x71\xcc\x04\xd9\x6d\x9e"
      "\xc2\xdb\x07\x47\x8f\x34\x7e\xdb\xd6\x40\x49\x23\xad\x4a\x56\x72\xb1\xb2"
      "\x85\xc7\x98\x8c\x4e\xc0\x92\x2c\x65\x5f\xf6\x00\x00\x00\x00\xc0\x0d\xc2"
      "\x90\xd9\x36\xd9\x32\x36\x05\x1f\xad\xfb\x4b\x95\xd0\x2c\x0b\xda\x7c\xe3"
      "\x8d\xab\xb7\xcd\x10\x3f\xe4\xd0\xc9\xc9\x63\xcd\x71\x7a\x77\xf8\xdf\x8d"
      "\x46\x09\x9b\x1f\x58\xe0\x68\xaf\x6a\xfb\xbc\x19\xdb\x16\x1c\x6d\xf3\xe7"
      "\xc9\xc7\x1b\xc0\x8a\x28\x2f\xc2\xc1\x42\x85\x6b\x5e\x4c\xaf\xf4\xc0\xa4"
      "\xf7\x24\x45\xef\x10\xdc\xd2\xc5\x69\x31\x9d\x6e\x9b\xb2\x05\x8d\x02\x3f"
      "\x66\x9a\x64\xfc\x7d\x96\x84\xb4\x5b\x00\x00\x00\x00\x36\x46\x73\xdc\xfa"
      "\x92\x35\xea\x5a\x2f\xf2\x3c\x4b\xb5\xc5\xac\xb2\x90\xe8\x97\x6d\xca\xc7"
      "\x79\xff\x00\x00\x00\x00\x00\x00\x00\x3d\x4e\x18\x5a\xfe\x28\xb7\x74\xb9"
      "\x9d\x38\x90\xbd\x37\x42\x86\x17\xde\x4c\xdd\x6f\x53\xc4\x19\xce\x31\x05"
      "\x41\x82\xfd\x09\x8a\xf7\xb7\xf1\xb1\x15\x2c\x69\x16\x11\xf8\x97\x55\x8d"
      "\x4b\x75\x5c\xb7\x83\x97\x8d\x98\x59\xb0\x53\x7b\x05\xb6\x23\xdc\xb5\xc4"
      "\xca\x93\x17\x47\x1a\x40\xfa\x49\x98\xcc\xa8\x0e\x96\x1e\xff\xfb\x4e\x1a"
      "\xa2\x5d\x8a\x17\xde\xef\x0c\x86\x94\xc4\x39\x5f\xc9\x9b\xe3\xc3\xfe\x7a"
      "\xeb\x8a\xf4\x92\x9c\xe7\xd3\x46\xca\x62\xb2\x5d\x48\xfd\xa5\xd1\x01\x46"
      "\x70\x2f\x78\xb2\x33\xb5\x20\x87\x52\x72\x6e\xd9\xf0\xc3\x40\xd4\x94\xb9"
      "\x2d\x19\xcc\x93\x0b\xb8\xa5\xf8\xb4\xda\x8f\x46\x03\xac\x0c\x3b\x69\x83"
      "\x84\xe1\x7a\x57\x0d\xc8\x52\x48\x23\xed\x15\xaf\x4e\xcf\xab\xb4\xb2\x54"
      "\x1d\x3c\x11\x4b\x7b\xba\x1c\x21\xa8\x45\xc9\xcf\x0d\x1c\xc2\x4a\xba\x47"
      "\xe3\x0f\x55\x8b\x22\x46\xad\x95\xcc\xf7\xd2\xf8\x0c\xc0\xab\x26\xf0\x83"
      "\x36\xea\x1a\x33\xb7\x9c\xf3\x5b\x89\x88\x37\x01\x6e\xb2\x11\xa1\x73\x4c"
      "\x7a\xf0\x76\xe1\x54\x51\xe3\x35\x19\xfc\x97\x8f\x66\xdf\x7d\xf4\x55\x7c"
      "\x91\x02\x4a\x8d\xc1\x30\xa2\x8e\xf5\xf6\x3a\xd0\x7b\x39\xc8\xd2\x3b\x85"
      "\xcf\x43\x4e\x06\x5e\x8a\x29\xa8\x00\x65\x5d\x12\x7d\xe6\xf6\x34\x7b\x49"
      "\x51\xf9\x7b\x57\x03\xdc\x78\xb1\xca\x9d\x74\xea\x6a\x9a\xe1\x2a\xb3\x67"
      "\xc0\xde\x26\x59\xcc\x38\xd2\xf3\x3d\xdd\x86\xe0\x59\x7d\x33\x36\x1e\xad"
      "\xa1\x19\xb5\x13\x21\x45\xfa\x45\x25\xc4\x88\xc7\xff\xfd\x6c\xed\xa6\xe9"
      "\xa0\x2e\xbd\x97\xce\xd6\xb0\x16\x1f\x2c\xc8\x46\x15\xce\xb8\xb1\x88\x83"
      "\x29\x9c\x63\x6e\x9e\x46\x72\x4a\x9a\x06\x00\xa8\xbb\x02\xf3\xe4\x89\x63"
      "\x1d\x52\x20\x19\xa3\x5f\xe1\x2a\x33\xca\xf9\xdd\x87\x68\xdd\xbc\x02\xa4"
      "\x84\xc3\x45\xc3\xef\xf2\x54\x29\x7b\x1d\xbb\x04\x98\x9c\x3f\x9f\x3c\x7b"
      "\x3c\x98\x5c\x39\xb1\xd3\x13\x01\x80\x68\xd3\x80\x9b\xac\x8c\x65\x7e\x39"
      "\xf4\xf6\x92\x61\x3e\x28\x38\x7e\x95\x57\x22\x90\x8d\xd8\x8b\x56\x16\x3b"
      "\xe8\x31\x2f\xf4\x7c\x5b\x6f\x28\x04\x72\x93\x5a\xf7\x4e\x97\xa5\xa8\x11"
      "\x0a\x4d\x74\x49\x6f\x4c\x8e\xc8\x2d\xdb\x56\xd9\xb9\x62\xd2\xfc\x43\xfa"
      "\x01\xa0\x47\x52\x68\x65\xc8\x4f\x7c\xff\x36\x05\x6c\xc4\xac\x25\x80\x21"
      "\xe1\x58\x1d\x43\xba\xda\xae\xc6\xcc\x5a\x2e\xf9\x89\xde\x98\x01\xfe\xd6"
      "\xd4\xbe\x2b\xfc\xfe\x07\xa6\x9c\x46\xbf\xfb\xe9\xdd\x03\x97\x08\x00\x00"
      "\x00\x00\x00\x00\x00\xd3\x72\xbd\xd6\xd8\x9d\xc1\xec\xf6\x3c\x23\xd5\x06"
      "\x11\x4d\x0f\xba\x2b\xd1\xc6\x9e\x8f\x7e\x3f\xcc\xdc\xda\x85\xce\x97\x5e"
      "\xc1\x38\x1b\x1c\xec\x6d\xda\xa7\x6e\x18\x67\x19\xd8\x19\x16\x43",
      1024);
  syscall(__NR_ioctl, r[2], 0x4400ae8f, 0x20000580ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  loop();
  return 0;
}
