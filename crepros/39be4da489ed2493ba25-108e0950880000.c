// https://syzkaller.appspot.com/bug?id=80537585ebe4c0de086bc787e9f0cf0e53169f3f
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  memcpy((void*)0x20000740, "/dev/vmci\000", 10);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000740ul, 2ul, 0ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000080 = 0xb0000;
  syscall(__NR_ioctl, r[0], 0x7a7, 0x20000080ul);
  *(uint32_t*)0x20000000 = 0x4d3;
  *(uint32_t*)0x20000004 = 0;
  syscall(__NR_ioctl, r[0], 0x7a0, 0x20000000ul);
  *(uint32_t*)0x20000640 = 0x4d3;
  *(uint32_t*)0x20000644 = 0;
  *(uint32_t*)0x20000648 = 0;
  *(uint32_t*)0x2000064c = 0;
  *(uint64_t*)0x20000650 = 0;
  *(uint64_t*)0x20000658 = 3;
  *(uint64_t*)0x20000660 = 0x20000000;
  *(uint64_t*)0x20000668 = 4;
  *(uint32_t*)0x20000670 = 0;
  *(uint32_t*)0x20000674 = 0;
  syscall(__NR_ioctl, r[0], 0x7a8, 0x20000640ul);
  *(uint32_t*)0x200000c0 = 0x4d3;
  *(uint32_t*)0x200000c4 = 0;
  *(uint32_t*)0x200000c8 = 0;
  *(uint32_t*)0x200000cc = 0;
  *(uint64_t*)0x200000d0 = 0;
  *(uint64_t*)0x200000d8 = 0;
  *(uint64_t*)0x200000e0 = 0;
  *(uint64_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000f0 = 0;
  *(uint32_t*)0x200000f4 = 0;
  syscall(__NR_ioctl, r[0], 0x7a4, 0x200000c0ul);
  memcpy((void*)0x20000740, "/dev/vmci\000", 10);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000740ul, 2ul, 0ul);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x20000080 = 0xb0000;
  syscall(__NR_ioctl, r[1], 0x7a7, 0x20000080ul);
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  syscall(__NR_ioctl, r[1], 0x7a0, 0x20000000ul);
  *(uint32_t*)0x20000640 = 0x4d3;
  *(uint32_t*)0x20000644 = 0;
  *(uint32_t*)0x20000648 = 0;
  *(uint32_t*)0x2000064c = 0;
  *(uint64_t*)0x20000650 = 0;
  *(uint64_t*)0x20000658 = 3;
  *(uint64_t*)0x20000660 = 0x20000000;
  *(uint64_t*)0x20000668 = 4;
  *(uint32_t*)0x20000670 = 0;
  *(uint32_t*)0x20000674 = 0;
  syscall(__NR_ioctl, r[1], 0x7a8, 0x20000640ul);
  *(uint64_t*)0x20000040 = 0x20000100;
  *(uint32_t*)0x20000100 = 2;
  *(uint32_t*)0x20000104 = 0xb00;
  *(uint32_t*)0x20000108 = 1;
  *(uint32_t*)0x2000010c = 0x81;
  *(uint64_t*)0x20000110 = 0x400;
  memcpy(
      (void*)0x20000118,
      "\xce\x56\xa1\xbe\xb4\xe8\xf9\xb8\x87\x00\xc8\x82\xdf\x7c\x1a\x70\x8d\x9e"
      "\xeb\xdb\x6c\x18\x96\xf4\xeb\xf6\x97\xe8\x59\xac\x08\xcb\x6f\x2c\xca\x50"
      "\x89\x82\x7d\xbb\x70\xa7\x0a\x02\x23\x7a\xa6\xa6\x3a\x04\x25\x12\x70\x3c"
      "\xd6\x8b\xe2\x4e\x7d\xa2\x5b\x74\x4c\x06\x9a\xcc\x3a\x57\xd4\x18\x62\xd7"
      "\x2c\xee\x2f\xc6\x68\xfe\x6b\xb3\x6d\xb2\xb4\xdb\xce\xab\x9a\x5f\x17\x89"
      "\xf0\x06\x37\xb5\xc6\xfc\xe1\x0e\x36\xfa\x13\x93\xed\x2d\x5b\x6f\xf1\x41"
      "\xec\xdc\x6a\x47\xef\x05\x1b\xf3\xce\x74\xb0\x12\xed\x5e\x9b\x3a\x07\xab"
      "\x97\x28\x21\xf1\xc9\x11\x98\xc2\x1d\x7e\x60\x25\x35\x8d\x49\xe2\x33\x1e"
      "\x51\xdb\xa2\xc6\x2f\x11\xf2\xb8\x66\xac\x72\x9a\x5c\x95\x26\xee\x59\xc5"
      "\xf1\x35\xb2\x5c\x0c\xc5\xda\xb7\x22\x16\x09\x6c\xb6\x5c\x01\xa6\xc5\x39"
      "\xa7\x0c\xf0\x48\x13\x0e\x8a\x83\xb9\x11\xf7\xb6\xd3\x83\xb4\x44\x2b\x07"
      "\xeb\x47\xac\x6a\xf2\x0f\xad\x0d\x83\xa8\x1e\xbb\x71\xeb\x14\xbe\x5b\x5c"
      "\x48\xeb\x21\x65\x07\xc3\x14\x24\x07\x07\x4c\xb3\xf3\x8d\xe0\x7e\x0f\x57"
      "\x61\xab\x32\xfb\xfe\x9b\x03\xad\x33\x98\x47\xce\xbd\x48\x75\x52\xe3\x43"
      "\x0b\xfa\xc0\x81\x81\x40\xd1\xc9\xb8\x30\x2a\x8f\xab\x88\x3a\x6a\xa1\x2c"
      "\x76\xe2\x8f\xfb\xe0\x10\x97\xda\x0a\x8d\xc6\xde\x55\x87\xb6\xd8\x73\x5e"
      "\xc0\xb3\xdd\xb5\xc7\x63\x51\xb3\x77\x1b\x00\x4e\xd0\xb5\xd2\xe3\x91\x2a"
      "\xe9\x9b\xd4\x5a\xb3\x10\x9b\x3e\x80\x17\x7f\xb0\xd8\x88\x33\xa6\xa6\x14"
      "\x04\x6f\x68\x79\x21\xa4\x66\x7e\x34\x50\x92\x47\x03\x1e\x4d\x95\x72\x02"
      "\xe8\x48\xfa\x8c\xf4\x55\x81\xf9\x91\xc7\xeb\xd4\x72\x45\x1d\x51\xd1\xd3"
      "\xfb\x1d\xfc\x5e\x6e\x93\x0d\x2f\xb2\xdb\xb2\xdf\x80\xb7\x67\x01\x76\x04"
      "\x25\xd1\x35\x36\x37\x78\x7f\xef\xec\xa6\x4e\x7d\xe7\xfd\x76\x73\x82\x99"
      "\xae\x3d\x07\x9f\xa1\x82\x4c\x92\x5a\x14\xfc\xbf\x4d\x64\x1f\x1d\xf3\x6e"
      "\xb1\x4c\x84\x74\x23\x53\x8a\x2b\x3f\xc3\x09\x28\xaa\x4f\xe3\x8c\xb6\xf3"
      "\x29\xa2\xe6\xbb\x41\xa9\x48\xba\x93\x23\xe0\x49\xbb\x90\x4b\xae\xf3\x57"
      "\xb1\xc8\x97\x96\x28\x5d\xeb\x68\xb9\x4c\xdb\x12\x38\x01\x18\x73\x20\x03"
      "\x63\x38\xf5\x3d\x18\x4f\x5a\xcc\x24\x7a\x96\xd7\x04\xfb\x08\x53\x01\x6d"
      "\x72\x6f\x4e\x8a\xa6\x50\x69\xe7\xf1\x45\x5a\x6b\x55\xfe\x49\xed\x48\x2f"
      "\xda\x49\xf4\x33\x8d\x60\x75\xe6\xe3\xc1\xb8\x23\xaa\xf4\xaf\x89\xfb\x72"
      "\x07\xc7\x21\x47\x6a\x31\xd3\x26\x74\xa0\x77\x0f\x30\xc0\xa2\x66\x5e\x52"
      "\x77\x4a\xe7\xb8\x1d\xb9\xd6\x4a\x58\x3b\xb5\x89\x9c\xa5\x6f\x03\x05\x60"
      "\xa6\x03\x89\xde\x14\x33\xaf\xe3\x40\x2a\x5a\xa6\xdf\x8f\xbd\x5e\x93\x58"
      "\x42\x08\xd7\xcc\x0f\xd0\xda\xed\x0c\x1f\x56\x42\x34\xcc\x46\x09\x81\xbf"
      "\x92\xe3\x16\xb5\x58\xf4\x87\xe5\xeb\x81\x1a\x30\xef\x60\x9e\xa4\xdd\x01"
      "\x7e\x6c\xbb\x6f\x48\xc3\x0e\xd4\xd1\x9c\x69\x1e\x66\xd7\xb0\xdf\x5d\xb4"
      "\x46\xbd\xc9\x92\x06\x55\x0b\x76\x5e\x7d\x5c\x7c\x2c\x3c\x61\xfe\x73\xd7"
      "\xa9\x80\x06\x49\x4c\xdb\xdd\x71\x7c\xa5\x02\x5c\xe2\x25\x09\xca\x4c\xf4"
      "\x15\xde\x0d\xfa\x44\xa1\x6c\xdc\xd9\x20\xab\xbc\xeb\x28\xad\x7c\x2b\x88"
      "\x1c\x5e\x22\x1e\xfe\x38\x50\xf2\xfe\x24\x8a\x56\x4d\xfa\x2e\xe1\x05\xaa"
      "\x88\x98\x01\x5f\x9c\xf9\x4f\x2e\xd6\xa4\x29\x85\x65\xcc\x25\x26\xd7\x3b"
      "\xb3\x0e\x4a\x80\x3c\x48\xa8\x15\xbc\x86\xa3\xc1\xf8\x73\x3b\x59\xf2\xe1"
      "\x84\xb9\xc6\x81\xb6\x55\x63\x4e\xdd\xa4\xae\x58\x98\xe7\xb3\xa6\xc2\x37"
      "\x65\xf4\x26\xd8\x1b\xc1\x24\x03\xe5\x90\xd6\x4a\x0a\x4c\xa0\x07\x5a\x56"
      "\xb3\x22\xe7\xfc\x5f\xfe\x9f\x44\xbc\x76\x8c\xa8\xfa\x5f\x2f\x9a\x3c\xc3"
      "\x97\x86\xf6\xc0\xd9\x18\xfd\x3d\xb5\x87\x6b\xa0\xa7\x62\x56\xdc\x2a\x94"
      "\x76\x0c\x5f\xfa\xf3\x06\x74\x8e\xa5\xea\x08\xb4\x98\xfe\x74\xbd\x0b\x76"
      "\xc6\x19\x85\xad\x0b\xd6\x92\x08\xc0\x8f\x62\x50\x81\xbc\x51\x11\x8a\xbc"
      "\xfa\x96\x06\x75\xa1\x1f\x9a\xcd\x18\x4f\x3f\xef\x63\x62\x3d\xf5\x4e\xc8"
      "\x49\x52\x88\x4a\x14\x40\x44\x03\x7d\xc1\x50\x8c\xae\x91\x13\x7e\x5b\x7f"
      "\xb5\x75\xdb\x89\xfa\x82\x4d\x9e\x4a\xc5\xac\x45\xfb\x29\xd2\x43\x8c\x77"
      "\x42\xaf\x67\xbe\xd8\xca\xe1\xd6\x0a\xdd\x5a\xef\xa8\x45\x17\x00\x96\x14"
      "\x0e\x0e\x16\x29\x98\x85\xf9\x25\x64\x10\x18\x23\xb9\x06\xcc\x4a\x9e\x2a"
      "\x6e\xb2\x73\x4a\x2f\x9f\x42\x47\xc0\xb9\x72\x4c\x23\x93\x05\xe6\xb5\xa1"
      "\xd2\xac\x81\xad\x25\x0d\xf3\x71\x7e\x6d\x44\xd0\xf7\x85\x74\x49\x59\x9d"
      "\x75\x13\xd9\x3a\x73\xb4\x3f\x1d\xb1\xb6\x30\x4d\xa9\x4f\x57\x9a\x03\x5d"
      "\x9e\x57\x2a\xa0\xb4\x99\xfe\x24\x45\x1f\x89\x3e\x20\x51\xfe\x00\x13\xdd"
      "\xdc\x3e\x74\xdf\xcd\xc2\x40\x03\xba\x82\x6b\xd0\x12\x5f\xa4\x1d",
      1024);
  *(uint32_t*)0x20000048 = 0x418;
  *(uint32_t*)0x2000004c = 9;
  syscall(__NR_ioctl, r[0], 0x7ac, 0x20000040ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  loop();
  return 0;
}
