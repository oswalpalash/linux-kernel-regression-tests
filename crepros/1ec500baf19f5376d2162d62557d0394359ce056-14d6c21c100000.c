// https://syzkaller.appspot.com/bug?id=1ec500baf19f5376d2162d62557d0394359ce056
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

uint64_t r[1] = {0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  memcpy((void*)0x20000400, "/dev/uhid\000", 10);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000400ul, 2ul, 0ul);
  if (res != -1)
    r[0] = res;
  memcpy(
      (void*)0x200000c0,
      "\x0b\x00\x00\x00\x73\x79\x7a\x31\x19\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00\x00\x00\x04"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00"
      "\x00\x00\x00\x00\x01\x00\x73\x8d\x7a\x31\x00\x00\x00\x00\x00\xff\x07\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x73\x79"
      "\x7a\x31\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\xcf\x0a\x00\x00\x00\x00\x00\x00\x00\x00\x0c\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x16\x00\x00\x00\x00\x40\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x3b\x38\xe9\x67\xac\x82\x06\xea"
      "\xae\x86\xb9\x7e\xec\x0b\x2b\xed\x1e\xe2\x33\x64\xb1\x0d\x6a\xad\x51\x02"
      "\x00\x00\x00\xe2\xa1\xdb\x3c\x6a\x0d\xee\x4a\xfc\x66\xd2\x44\x28\x05\x20"
      "\x1c\x39\x38\x9a\x80\x4c\x41\xc2\x99\x3f\xc6\x7e\x8a\x14\x60\x45\xe1\x4a"
      "\x8a\x08\x00\x55\x0e\x6a\x25\xc0\xef\x65\xf6\xec\x71\xf0\x08\x42\x54\xd1"
      "\x40\x18\x7f\xaf\xa4\xa1\xee\x6e\xce\x53\xc6\x73\x85\xb8\x83\xa3\x6a\xd2"
      "\x4a\x04\x00\x00\x00\x00\x00\x00\x00\x6a\x8a\xb1\x1b\x0a\x0b\x00\xe7\x7e"
      "\x6c\x16\x18\x9c\xfa\x16\xcb\xe0\x1a\x4c\xe4\x11\x37\x8e\xaa\xb7\x37\x2d"
      "\xab\x5e\xef\x84\xc3\x1b\x2d\xad\x86\x8a\x53\xe6\xf5\xe6\x97\x46\xa7\x1e"
      "\xc9\x2d\xca\xa9\xa7\xdf\xab\x39\x42\x86\xe5\xc8\x1e\xae\x45\xe3\xa2\x5b"
      "\x94\x2b\x8d\xa1\x1e\xdb\x57\x8b\x45\x3a\xca\xc0\x3a\x9d\x34\x48\x00\x00"
      "\x00\x00\x83\xd6\xd5\xfe\x4f\x83\x3d\x4d\x4c\xfb\xee\xf0\xe0\xe6\x2b\xe2"
      "\x05\x00\x00\x00\x3c\x32\x98\x4c\x6c\x4b\x2b\x9c\x33\xd8\xa6\x24\xce\xa9"
      "\x5c\x3b\x3c\x6d\xd8\x73\x56\x9c\xf4\x78\x6f\xc5\x16\x6b\x03\x00\x00\x00"
      "\x00\x00\x1f\xf2\x8d\x3c\xe3\xe3\xb8\xf8\x1e\x34\xcf\x97\xc9\xc8\x41\xcb"
      "\x2e\xf0\x81\x07\xa9\xa9\x65\x49\xe3\xd2\x59\xdf\x17\xe2\x9e\xd6\x4b\xd6"
      "\x12\x08\x13\xf9\xf0\x34\x4e\x13\x95\x06\x70\x1e\x8f\xde\xdb\x06\x00\x9b"
      "\x5e\x4d\x0c\x67\xbd\xa0\xb9\x28\xb7\x32\xcf\xf7\x82\xb0\x68\x40\x75\xf2"
      "\xcb\x78\x51\xef\xdd\x77\x97\xee\x95\xd2\xac\x28\xa8\xca\xbd\x26\xc1\x56"
      "\x82\xaa\x58\xd3\x1a\xec\x95\x6b\xd7\xc2\x78\x06\x40\x34\x34\xb3\xc3\x0b"
      "\x07\x0b\xcc\x82\x66\xe1\x2f\xa6\x66\x02\x05\x62\x56\xf7\x46\x75\xb7\xcb"
      "\x6a\x2c\xd9\x33\x59\xde\x4d\x87\xb2\xef\xa3\x70\xc8\xf3\xdf\x53\xca\xf8"
      "\xfe\x18\x0c\x4d\xea\x3f\x5b\x7a\x87\x1b\x30\xc7\xa5\x75\x3b\x48\xf7\xf0"
      "\x91\x92\xa3\x4b\x1e\xfa\xab\x02\xdc\xa0\x51\x7e\xee\x10\xff\x30\x20\x6f"
      "\x78\xec\x82\xc7\x2f\x48\x9f\xc3\x26\x0b\xd7\xd9\x24\xe7\xe2\x68\xc4\xfb"
      "\x5b\x80\x05\x75\x06\x7b\xbb\x58\x5b\xbd\x2e\x51\x63\xa2\xda\x00\x29\x02"
      "\x72\xce\xc9\xc5\x27\xe7\x7f\xb7\xf1\x53\x33\x5a\xbd\x27\xa3\xf6\xa0\x7f"
      "\x64\x02\xf9\xe4\x44\x7b\x74\x06\x73\x23\xf9\x92\xb4\xa8\xc9\x8c\xcf\x7c"
      "\x0e\xb6\x9e\x2f\xa6\x20\xb7\x1f\x48\xa6\xd1",
      785);
  syscall(__NR_write, r[0], 0x200000c0ul, 0x12eul);
  *(uint32_t*)0x20000040 = 0xc;
  *(uint16_t*)0x20000044 = 0x13;
  memcpy((void*)0x20000046, "\x80\xf5\x37\x17\x67\xfb\x3e\x5f\xbb\xb2\xf9\xdb"
                            "\x88\x1e\x00\x41\x85\x68\xce",
         19);
  syscall(__NR_write, r[0], 0x20000040ul, 0x19ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  loop();
  return 0;
}
