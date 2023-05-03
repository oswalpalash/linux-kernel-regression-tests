// https://syzkaller.appspot.com/bug?id=e16a5939447ee24dd21d2d26248fa3fabdc51105
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

static unsigned long long procid;

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
  memcpy((void*)0x20000080, "/dev/uhid\000", 10);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000080ul, 2ul, 0ul);
  if (res != -1)
    r[0] = res;
  memcpy(
      (void*)0x20000200,
      "\x0b\x00\x00\x00\x73\x79\x7a\x31\x19\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00\x00\x00\x04"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x01\x00\x73\x8d\x7a\x31\x00\x00\x00\x00\x00\xff\x07\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xe7\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x73\x79"
      "\x7a\x31\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\xcf\x0a\x00\x00\x00\x00\x00\x00\x00\x00\x0c\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x16\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x3b\x38\xe9\x67\xac\x82\x06\xea"
      "\xae\x86\xb9\x7e\xec\x0b\x2b\xed\x1e\xe2\x33\x64\xb1\x0d\x6a\xad\x51\x02"
      "\x00\x00\x00\xe2\xa1\xdb\x3c\x6a\x0d\xee\x4a\xfc\x66\xd2\x44\x28\x05\x20"
      "\x1c\x39\x38\x9a\x80\x4c\x41\xc2\x99\x3f\xc6\x7e\x8a\x14\x60\x45\xe1\x4a"
      "\x8a\x08\x00\x55\x0e\x6a\x25\xc0\xef\x65\xf6\xec\x71\xf0\x08\x42\x54\xd1"
      "\x40\x18\x7f\xaf\xa4\xa1\xee\x6e\xce\x53\xc6\x73\x85\xb8\x83\xa3\x6a\xd2"
      "\x4a\x04\x00\x00\x00\x00\x00\x00\x00\x6a\x8a\xb1\x1b\x0a\x0b\x00\xe7\x7e"
      "\x6c\x16\x18\x9c\xfa\x16\xcb\xe0\x1a\x4c\xe4\x11\x37\x8e\xaa\xb7\x37\x2d"
      "\xab\x5e\xef\x84\xc8\x81\x82\x0f\x86\x8a\x53\xe6\xf5\xe6\x97\x46\xa7\x1e"
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
      "\x6a\x2c\xd9\x33\x59\xde\x4d\x87\xb2\x70\x8d\x70\xc8\xf3\xdf\x53\xca\xf8"
      "\xfe\x18\x0c\x4d\xea\x3f\x5b\x7a\x87\x1b\x30\xc7\xa5\x75\x3b\x48\xf7\xf0"
      "\x91\x92\xa3\x4b\x0e\xfa\xab\x02\xdc\xa0\x51\x7e\xee\x10\xff\x30\x20\x6f"
      "\x78\xec\x82\xc7\x2f\xf1\x51\x20\x53\xdd\x79\x31\x03\x0c\xc7\x04\xdf\xac"
      "\xa2\x0f\x25\x6a\x4f\x2e\xc4\xa1\xdb\xa9\xc2\xa9\xb9\x88\xfa\x89\x76\x9b"
      "\x8b\x47\xf2\x19\x0c\x3d\x4d\x52\x6b\x82\xa2\x35\x90\x10\x0f\xda\x7c\x42"
      "\xc3\xc8\x6b\x99\x30\x7e\x76\xe0\xe1\xc3\x3b\x3d\xed\xe8\xa7\x8c\x64\x2c"
      "\x6b\x02\x0f\xd2\xbc\x95\x81\x6a\x29\xb1\xff\xea\xc0\x34\x60\xab\x29\x8b"
      "\x2b\xa2\xc2\xd4\x81\x18\x50\xc1\x4c\x58\xb7\x1f\x9c\x5b\x50\x7c\x67\x3a"
      "\xa8\x95\x9a\x80\x1e\xa8\x90\x51\xb3\xe7\x79\xb2\xe4\x73\x02\x7b\x8b\x05"
      "\x00\x5f\xf1\xa8\x15\x50\x52\x06\x99\xda\x8d\xb7\x37\xc7\x3e\x55\x5d\xba"
      "\x6f\x99\x2c\x48\x4d\xff\x02\x63\xb3\x90\x81\xd9\xd1\xb3\x72\xad\x46\xe3"
      "\x7f\xba\x93\x0e\x4a\x02\x11\x23\x96\xe3\x2d\xd5\xd0\x60\x1d\xb9\x45\x65"
      "\xd4\x69\xe2\x25\x7b\x22\xa5\x00\xec\x31\x75\x25\x74\x56\xe8\xca\x35\x43"
      "\xcf\xf8\x87\xd7\x09\x30\xf6\xf2\xac\x30\x38\x09",
      912);
  syscall(__NR_write, r[0], 0x20000200ul, 0x12eul);
  *(uint32_t*)0x200005c0 = 8;
  memcpy(
      (void*)0x200005c4,
      "\x23\x15\x45\x82\x9f\xa8\x35\x83\x77\x80\xc4\x64\x94\xf8\x48\xce\xba\xbd"
      "\x8c\x19\x1f\x7f\x65\xf2\x74\x8d\x24\x00\xb9\x24\xc4\xf6\x0b\x32\x27\x91"
      "\x9a\x6e\x3e\xca\x70\x47\x6b\x85\x91\xe9\x47\x9d\x95\x81\x1f\xf6\xdb\xbf"
      "\xf0\x9a\x61\x3d\x16\xd4\xdc\x07\xd8\xb4\x36\xbc\xe8\xa4\x53\x11\xf4\x22"
      "\x41\xe4\x3d\x0c\xcf\x12\xe3\xe1\x8f\x6f\x04\xd1\xe0\x90\x46\xa5\xd1\xa0"
      "\x21\xdc\x0e\x2a\x11\x23\xab\x42\x51\xf6\xbf\x49\xb3\xc7\xb6\x23\xd9\xc8"
      "\x2c\x17\x3c\xab\x74\x0e\x39\x4f\xca\x0b\x73\x74\x09\xec\x1d\x7e\xa7\x62"
      "\xe9\x6e\x87\xd4\xd0\xf0\xc2\xd9\xfc\x42\xb7\xaf\x11\xb1\xa1\x58\xd9\x73"
      "\x23\x52\x3d\xaa\x86\x40\x20\xac\xcf\xa0\x98\xff\x4c\x03\x47\x51\x50\x95"
      "\x56\x1e\x9b\x41\xdf\x91\x3d\x19\x2e\xa8\x25\xb5\x22\x07\x00\x3b\xd5\x24"
      "\x5b\x38\xa7\x40\x04\xe3\x4d\x30\x4d\x6c\x41\x7f\x8b\xc3\x40\xb1\x5f\x1b"
      "\xd2\x7b\x90\x6c\xca\x55\xf7\x86\xbe\xdc\x37\xbc\x6c\x7d\x7a\x27\x66\x13"
      "\x6f\xd0\xff\x4c\x8d\x46\x0a\x96\x98\x0f\x08\x9a\xa9\x75\x7c\x2a\x54\x98"
      "\xb8\x37\xe0\xa3\xa7\x0b\xd6\x82\xca\x39\x9c\xba\xd0\x08\x59\xaf\x48\x9e"
      "\x7f\x8a\xb9\x0d\x54\x8e\xb2\x12\x3a\x4f\xe6\xf1\x26\x08\x2b\x52\xf6\xb7"
      "\x44\x6c\x7d\x24\x83\xc4\xc9\x87\x47\xf6\xff\x6b\xdf\x12\x12\xd0\x7f\xfa"
      "\xb0\xf4\xdc\x00\xf9\x5a\xde\xb8\xaa\xd6\xcc\x44\xb6\x10\x8e\x03\x78\xa9"
      "\x8d\x23\xcd\xc5\x4f\x3c\x37\x4e\x57\x87\xa6\x4a\xf8\x5c\x35\xb8\xcf\x62"
      "\xc3\xfb\xc1\xbd\xf4\xcf\xcc\xf4\x41\xe3\xea\x68\xed\x55\xdc\xc2\xf6\x28"
      "\xa0\xd6\x2e\x82\xf4\x0c\xf3\xd4\x26\xba\xb8\x6d\x17\xfd\xa5\xd8\xd0\x4c"
      "\x0d\xaf\x3f\x19\x73\xe8\xd5\xa9\xe5\xf9\x05\x0e\x80\x75\x09\x6f\x06\x84"
      "\x75\xc1\xd0\xd0\x1a\x33\x08\xae\x6a\x6e\xa4\x0b\x58\x71\x14\x1e\xff\xa3"
      "\x5e\x10\x5d\x82\x52\x60\xb2\xeb\xf8\xa5\x96\x72\x23\x8f\x0b\x43\xe4\x3f"
      "\x49\x10\x76\xb8\x18\x6d\x8e\x5e\x13\x29\xc1\xb8\x1c\x97\x4d\xad\x84\x45"
      "\xe6\xad\x91\x07\xbd\xf4\x84\x2f\x1a\x9d\x46\xfe\xce\x7b\x8e\x45\xda\x2b"
      "\xbd\x11\xd7\x86\x60\x14\x80\xa7\x6d\xe4\xad\x3b\xcd\xb4\x15\x6d\x9a\x79"
      "\xe6\x53\xb5\xdc\xc7\x05\xfb\xab\x19\xb3\xc9\x22\x52\xfc\x3d\xb1\xaf\xe8"
      "\x09\x54\xed\x14\xbb\xaf\x26\x81\x1c\x7b\x0f\x4c\x1e\xaf\xad\xe0\xc4\xd4"
      "\x7e\xac\x0f\x14\xbe\xe0\x13\xa7\x83\x34\x02\xa9\x29\xd0\x5a\x8d\x09\x50"
      "\xa0\x6b\x9a\x9a\x77\x6d\x2a\x17\x79\xf5\xa9\x92\x4b\x46\x6b\xaa\x8f\x5e"
      "\x0f\x6d\x27\x14\xcf\xfa\xbf\x94\xf8\x74\xaf\x1f\xa3\xb7\xc5\x61\x12\x8f"
      "\xf3\xc3\xac\x33\x4c\x17\x8f\xae\x3c\x45\x8e\x01\x75\xb3\x15\x56\x3e\x8d"
      "\xd2\x84\x0f\xf4\x88\xea\xca\xd5\x9d\xf6\x00\xa2\x9f\xd2\xe1\x27\x84\x97"
      "\xdc\xe1\xf2\x80\x79\x54\x40\x04\xaa\xc5\xf3\xca\x81\x0e\x7e\x32\xd7\x7c"
      "\x6a\x2b\xac\x18\x56\xf2\xe6\x89\x05\x3c\xa3\xe1\x77\x45\x87\x37\x7d\x0a"
      "\xca\x45\xda\x31\xb0\xf9\x3e\x38\xde\xe0\xdc\x0d\x8b\xa0\x7c\xbf\x40\x79"
      "\xb7\x71\x09\x07\xaa\x17\x6e\xaf\xb5\x27\xa4\x46\x2e\x94\xa6\xba\xae\x2b"
      "\xf3\xdc\xaa\x36\xc4\x93\x3e\x3a\x05\x06\xfc\xec\xc7\xd8\x57\x61\x9b\xda"
      "\x55\xc8\x20\xd8\x80\x7b\x8a\xf8\xfd\xd4\x04\xe4\xc9\xe2\x95\x2a\xc2\x5f"
      "\xd1\x30\x88\x38\xa4\x51\x51\x71\xbb\x3b\x52\xf3\x65\x73\x16\x0a\x27\x03"
      "\x54\x14\x7b\x88\x1d\x4b\xfb\x69\x01\x9d\x25\x15\x46\x96\xaa\x54\x88\xe5"
      "\x5d\x5d\x53\xe3\x95\x6d\x8b\xd8\xb7\x9c\x04\xf3\x27\xed\x75\x8f\xa6\xc4"
      "\xf2\xd0\x94\xd8\xf4\x86\x13\x08\xb3\x6e\xd3\x6d\x10\x65\x6a\xb9\xf0\x8c"
      "\x71\x02\xed\xad\x55\x35\xca\xe0\xe7\x13\xe2\x59\x59\x5d\xe2\xd1\x11\x5f"
      "\x71\x56\x81\xee\xd7\xdb\x1a\x03\x30\x6b\xc9\xd8\x10\x7b\x0d\xdb\xb2\x35"
      "\x00\x86\xe4\xcd\x4a\x5f\xd0\x3b\x78\x39\xbb\x5e\x88\x1e\xf0\xa3\x03\x52"
      "\xa7\x1c\x26\x8b\xd3\xb4\xce\xf6\x8d\x0c\xd8\x90\xdd\x47\x39\x56\x56\xbd"
      "\x61\x14\x0f\x9c\xbd\xab\xaf\x58\xf3\x23\x85\xe6\x94\x80\x2d\x59\xdf\x36"
      "\xb6\x37\xaa\x17\x30\xd8\x72\xa0\x5c\x6c\x56\xa0\x7d\x37\x37\xff\x32\x01"
      "\xf7\xa0\x04\xb2\x22\xac\x83\x36\x05\xf9\x5a\x85\x37\x53\xd5\x53\x45\x80"
      "\x92\x5b\xc3\x75\x1b\xa6\x76\x66\x0a\xcc\x3b\x57\x8a\x6a\x30\x58\x65\xcb"
      "\xfb\x77\xc7\x0f\xf9\xb0\x51\xa2\x74\xa3\x67\x84\x8a\x68\x41\xf0\x51\x9a"
      "\x89\x7a\x1f\x34\xbf\x95\x9f\xb1\xe8\x1b\xfc\xfb\x2e\x9d\xc2\xf5\x5a\x4b"
      "\x6d\x8e\xaf\x7b\x82\xd8\x46\x97\x79\x2f\xd0\xce\x21\x11\x94\x09\xfb\x09"
      "\x12\x97\x09\xb6\x2a\x8d\x46\x25\xc6\x5a\xe1\xc1\xc0\xc8\xb4\x35\x43\x30"
      "\x3e\x30\xb1\xbf\x83\x1f\x94\x76\xbc\x10\x88\x00\x40\x83\x26\xa3\x18\xfc"
      "\xb5\x3b\xd1\x55\x02\xe9\x55\xb9\xfc\x97\x1c\x9b\xdc\x4e\x04\x9e\xa6\x8d"
      "\xfd\x34\x23\x05\xf9\x56\x91\x10\x1e\x61\xb5\x38\xb4\xba\x1a\xa2\x6c\x28"
      "\xad\x75\xbc\x8b\x9f\x33\xac\x23\xb1\x58\x26\xc2\x3f\xa0\x6b\x30\xe4\x42"
      "\xe6\x91\x06\xde\x3b\x2e\x29\xa4\x90\x2f\x70\xcb\xe5\xb0\x2b\xc6\x58\xd5"
      "\xe1\xad\xe1\x1a\x39\xae\xf0\x9c\x5c\x0e\xed\x27\x21\xc4\xb2\x75\x32\xac"
      "\x52\x2a\xd4\xc3\x98\x35\xd8\xf1\x63\x7b\xd6\x52\xe0\x57\x3d\xed\x5b\xf7"
      "\x37\x69\x64\xe5\x48\x11\x46\xe7\x7c\x0a\xe5\xfc\x74\x25\x51\x2f\x1a\xa5"
      "\xc6\x0a\x4e\x2f\x2c\x53\xa9\x7c\x1e\xdd\x4b\xeb\xca\x65\x0e\xa4\x4b\x06"
      "\xfa\xf2\x95\xef\x15\xb3\xd1\xc5\x64\x49\x97\x17\x89\x4d\x48\xe3\x87\x22"
      "\x6b\xd5\x2f\xce\xa1\x52\x77\x91\x59\x8c\xbd\x48\x59\x0e\xdb\x99\x5f\xb8"
      "\x43\x2f\xa1\x99\x6d\xc8\xb2\x99\x82\x33\xca\xf8\x3a\x44\x65\x7c\xe8\xbb"
      "\x44\x92\x9f\x34\x40\x12\xff\x40\xa3\x1d\xe4\xbf\x98\x66\x27\xdb\xb5\x19"
      "\xe8\x41\xd5\x73\xcb\x52\x5d\xfa\x1e\xcd\xcf\xf6\x3a\x9b\x14\xfb\x43\x44"
      "\x02\x39\x41\xf1\x78\xe5\xc3\xb6\x2e\xc3\xe4\x07\xd3\x98\x33\xca\xcf\x5f"
      "\x4c\xb2\xdf\xf3\x57\x56\x05\xb1\xb4\x3a\x30\x6a\x3b\x7b\xdc\xeb\xba\xea"
      "\x36\x57\x36\xba\x41\xcd\x42\xca\xc2\xad\x1a\x97\x72\x71\x63\xfc\x27\x84"
      "\x19\x30\x86\xfb\x09\x27\xea\xc3\x39\x26\x3b\xe1\xc1\x87\x68\x3c\xf2\xbf"
      "\xf3\xbd\x9a\x69\xb0\x0e\x7e\x05\xa7\xc2\x2e\xd9\x5c\x7b\xd7\xba\x29\x88"
      "\x7c\xdf\xac\xe8\x2e\x2c\xf4\x81\xc4\xd4\xf9\x19\x15\xce\x29\x38\x93\xc1"
      "\x33\xe4\xad\x98\x4a\xe6\x0d\x61\x9b\x66\x2b\x02\xd2\xdf\xf7\xb6\x9d\xa7"
      "\xfc\x45\xb4\x06\x99\xa7\x30\xa9\x9a\x43\xab\x54\x08\x47\x54\xcd\x05\x92"
      "\x22\x25\xd8\x1e\x7c\x47\x3e\xa2\x6f\x28\xb9\x8d\x69\x57\x52\xf5\x14\x29"
      "\x0b\x34\xa8\xf7\x75\x18\x07\x3d\xcf\xb6\x57\xd1\x13\x4c\x71\xae\x33\x58"
      "\x04\x1c\x42\x72\x2b\x51\x4e\x9d\xd1\xc1\x69\x68\x9c\x3e\x34\xaf\xd9\x1b"
      "\x98\x56\x19\xf9\xe1\x07\x5f\x7e\x99\xb0\xe8\xaf\x4a\x1f\x5c\x69\xaa\x99"
      "\x45\x0e\x11\x36\x53\x26\x80\x02\x48\xdf\xf2\x62\x69\x80\x2f\xe5\x5b\x96"
      "\x54\x5a\x5c\xa1\xfe\xf5\xd4\xbd\x4a\x37\x40\xd6\x1f\xdb\x60\x9c\x19\x49"
      "\x87\x39\x4f\xee\x3a\x2f\xf8\x02\xa6\x33\x4e\x63\x4d\x61\x52\xfd\x11\x2f"
      "\xa1\xba\xab\x0c\x06\x7d\x49\x02\x50\xf4\x0e\x93\x8e\xe0\x7f\x9b\xa7\x2c"
      "\x7a\x20\x02\xc1\x55\x47\x77\xe2\xbe\x47\xa1\x06\xd5\xba\xe4\x5b\xb0\x9c"
      "\x81\x5a\x9c\x1f\x73\x33\xb5\x70\x6b\x04\xc0\x1c\xf9\x88\x26\x21\xed\xe6"
      "\xd5\x6d\x1c\x92\x60\x57\xca\x2b\x38\x7e\xcc\x34\x2f\x4d\x78\xca\x3d\x93"
      "\xca\x39\xe4\xba\x18\x7b\x2a\x71\xd6\x33\x3b\xeb\xf2\x6c\x09\x0e\x3a\x5a"
      "\xda\xbd\xfe\x38\xef\x34\xe8\x6f\xb0\x1a\x25\xdd\xd0\xaf\x55\xe6\x23\x2c"
      "\x22\xe9\x7a\x2f\x7b\xc9\x34\x26\x34\x12\xff\x42\xf4\x29\x37\xf5\xe5\xf1"
      "\x0d\x2e\xb8\x1b\xa7\x6a\x3e\x4e\x61\x4b\x4b\x07\x6c\x58\xca\xc4\xd3\xf4"
      "\x01\xb1\xb5\xd4\x7e\xff\xe1\xbb\xe7\xaa\xd3\x53\x6b\xa0\x1e\x70\x35\x8d"
      "\x24\x3e\xce\x9b\x64\x19\xc3\x5b\xc8\x33\x03\x27\x8b\x39\x7e\xf5\x66\xab"
      "\x5d\x24\xf4\x60\x2b\x2f\x99\x4d\x16\x01\x13\x03\x6d\xbc\x93\x71\x02\xcd"
      "\xe4\xb2\x0d\xfc\xd1\xf0\x4b\xc2\x5f\x9c\x6b\xf4\xa8\x1f\x19\xae\x04\xa5"
      "\x7b\xf1\x81\xc1\x18\xf7\x32\xc8\x4c\xd1\x32\x63\x85\xb8\x6c\xb8\xaf\x5c"
      "\x38\x04\x12\x73\x38\xa7\x8d\xc9\x5b\xe3\x62\x96\x3e\x3a\xc8\xb4\xcb\x75"
      "\x4f\xea\x0f\xeb\x82\xe7\x06\xc3\x7d\x41\xe5\x2d\x55\xba\x39\x7c\x2f\xf3"
      "\x4f\x88\xc1\xfb\x46\x68\x54\x5f\x8b\x25\x9c\xaf\xad\x51\xc3\x30\x79\xed"
      "\x64\x59\xc5\xfd\x53\xcc\xb9\x90\x8b\xdc\x5e\xa7\x02\xe6\x99\x74\xee\x24"
      "\x6d\xb0\xdc\x24\x77\x80\xde\x0b\x9a\xdd\x4b\x20\x1e\xa9\x5d\xce\xfb\xf1"
      "\x74\x4a\xe0\xf3\x3a\xeb\x89\x9a\xf2\xde\x26\x7c\xc3\xcf\x6e\x71\x3e\xf1"
      "\xb0\x01\x49\xf8\xfc\xc9\xc2\xeb\x14\x5e\x3b\xa6\x92\x88\x9d\xd2\x5f\x61"
      "\x64\x40\x0b\x8f\xf7\x2f\xa2\x2b\xb0\xc5\x5a\x6b\x05\x5c\x35\x07\x69\x96"
      "\xb4\x65\xf7\xe4\x96\x94\xc7\xed\x3b\x98\x22\x5b\x96\x18\x72\x7a\x24\x41"
      "\x1c\xcf\xb4\x46\x55\xf3\xfc\x68\xf1\xa7\x8a\x54\x0b\x3f\xc5\x06\xc2\x7b"
      "\xd3\x52\xf9\xa8\x74\xe4\xad\xdb\x7a\xb8\x76\x11\x32\x4f\x33\x17\x88\x4d"
      "\x9c\x2a\x0f\xe8\x12\xc7\xb5\x7a\xe7\xec\x94\x5c\xf4\x37\x0b\x39\xd1\xc8"
      "\xdb\x74\x0c\xf8\x8a\x20\x63\xfd\x0f\x91\x03\xb3\xa9\x10\x49\x22\xc2\x2c"
      "\x18\x33\x76\x7e\xf5\xb8\x51\xda\x38\xa0\x8f\x65\x2e\x8e\x1a\x81\x3e\x95"
      "\xf6\x43\x47\xea\x19\x3a\x4f\x82\x4e\x84\xbd\x61\xfb\x3d\x98\x2d\xaf\xe2"
      "\xd0\xb7\x99\x08\xbb\x69\x44\x4c\xf3\xfe\x78\x8e\x3b\x67\xb3\x80\xad\xf4"
      "\x06\x19\x13\x21\xac\x78\x53\x1e\x26\x3b\x73\x57\x5d\x7d\x84\x6f\xb3\x1b"
      "\x84\x8f\x71\xe9\x39\xa9\xab\x90\x88\xb1\xc9\x36\xbe\xe1\xbd\xbe\x88\x16"
      "\x57\xfa\xef\xad\x39\xb7\x5f\xbd\x4d\xe9\x75\xe6\x73\xb3\x01\x5e\x87\x97"
      "\x69\xc2\xe2\xb8\xe8\x1d\xfa\x96\x5c\x3f\xa8\xb3\x26\xd3\x5a\x6e\x97\x98"
      "\xfd\xf4\xf3\x1a\x7d\x2e\x57\x88\x4c\xdb\x97\x03\x0e\xc8\x36\x6f\xdb\x75"
      "\xef\x88\xf4\x00\xf6\x58\x8c\x8b\xa9\x41\x87\xbb\x8a\xb5\xc6\x94\x49\xf2"
      "\x19\xd7\x43\x23\x87\xad\x95\xcc\xe9\x87\x86\x61\x62\xdb\xc5\xa1\x7b\xa8"
      "\x3b\x1f\x07\x20\x6a\xb7\x01\x8f\x1d\xed\x0f\x28\x7e\xf8\xc9\xb2\xab\xd2"
      "\xd1\xee\x6c\x55\x4e\x81\x7b\x84\x41\x75\x32\x2f\x3a\x55\x8e\xf0\xfa\xdd"
      "\x82\xed\xec\xaa\x85\x35\x94\x66\x50\xb6\xe9\xb5\x13\x50\xce\x31\x87\x23"
      "\x4c\x71\xbc\x4d\x44\x35\xc6\xe3\x76\x0a\x60\x0b\x19\xb6\x45\x6e\x66\x87"
      "\x03\x05\xef\x10\x86\x0b\x3b\x7e\x49\x9b\xf2\xf4\x83\x11\xe2\x61\x2b\xf0"
      "\x8d\x01\x7b\x7a\x7f\xc2\x86\x62\x11\x1b\x9f\x0b\x94\x91\x17\xe4\x62\x98"
      "\x80\x1f\x51\xde\xed\x1a\x3e\x85\xda\x6e\xf2\x84\x12\xe1\x9f\xc9\xd8\xfc"
      "\x47\x73\x3d\x98\xea\x9d\x30\x46\xa3\xc3\x41\x5a\xaa\xd5\xb3\x30\xc7\xde"
      "\xb0\x46\x41\x1c\x1b\x27\x25\x1a\x35\xe6\x8f\x28\x08\x57\x09\x6a\x7a\x55"
      "\x5f\xff\xdd\xe4\x2b\xdd\xde\xe1\xc8\xc4\x3f\x8d\x56\x44\x28\x3f\xa2\xd6"
      "\xb6\xba\xdf\xa4\xcc\x22\x10\xbe\x70\xcb\x75\xfd\x46\xae\xeb\x85\x27\x3a"
      "\x06\x14\x61\xc4\xa5\x45\x4e\x26\x58\xc3\x4b\x28\xb5\xf8\x9e\x36\xbe\xdc"
      "\x55\x9d\x5c\x7b\x36\xd9\x48\xd3\xe9\x91\x04\x7c\xce\xe7\x8c\xcf\xc3\x7d"
      "\xd8\x30\xd4\x7b\xad\x90\xaf\xa3\xaa\x4f\xbb\x23\x2e\xeb\x15\x50\xb3\x1a"
      "\x61\x03\x83\xa4\xb3\xa2\xca\x8a\x93\x2e\x41\xd7\xac\xf4\x11\x3f\x00\x3d"
      "\x22\x5a\xb4\xd5\xfc\x52\x40\x89\x60\x5f\x5d\x19\xdf\x83\x73\x5f\xec\x04"
      "\x9e\x35\x6d\xc4\xcd\x92\xd1\x11\x50\xea\xf6\x80\x66\xe2\x31\x83\x4d\x8a"
      "\xb9\xbc\xc9\x96\x4b\x27\x50\x18\xbd\x31\x38\xb0\x17\x4e\xf5\x1f\xdf\xc6"
      "\x60\x29\x4f\xfd\x37\x17\xb8\x49\x91\xfb\xea\xe0\xa0\x15\x70\x10\x9b\xdb"
      "\x03\x2d\x75\x3e\x9e\x12\x6c\x1d\x24\xad\xf9\x6e\xdc\x0f\xc4\x37\x57\x37"
      "\x55\x28\x58\x7e\xa8\x05\xcf\x8a\x01\x4b\x69\xd3\x96\x72\x5d\x8e\x3a\x8b"
      "\xc8\x0f\x73\xfa\xd8\xc7\xef\x20\x6a\x72\x9d\x62\xbb\x8f\xae\x21\xca\xb1"
      "\x91\x8d\x76\x2e\x5f\xd1\xf4\xde\xd4\x3c\x80\x74\x8c\x1d\xce\xb7\xce\x3d"
      "\x50\x2b\xdb\xbf\xd8\x45\xd0\xc4\x37\x6e\x3d\xf7\x13\xa8\x5c\x7e\xd6\xff"
      "\x23\xcb\x30\x97\x9f\x6d\x49\x5a\x6b\xf8\x63\x06\x5a\x36\xd0\x9e\x2f\x42"
      "\x7a\xf1\x58\x04\x3e\x9c\x05\xc7\x5f\xa8\xec\x20\xae\x45\xb4\xe0\xb6\xa6"
      "\xef\x6b\x86\xfa\x13\xd9\x53\xe1\xac\x01\xa7\x05\xb7\x51\x41\xc8\x19\x4a"
      "\xaf\x12\xfb\x7b\xd8\xcf\xd2\x93\x57\xcc\x42\x9c\xac\x89\x0b\x3f\xd0\xc5"
      "\x79\x6e\x0d\x60\xe1\x9d\xb1\x6c\x71\x6c\x62\x32\x66\x13\x1d\x1e\x98\x4f"
      "\x08\x12\xd7\xae\xec\xf8\x8b\x26\x55\xcc\xe9\x1b\xc6\xaa\x9a\x8e\xac\xfa"
      "\x5c\xfb\xd4\x7f\x99\x5c\x89\x23\x25\x9b\xf7\xf7\xf8\x5b\x0e\xd3\x9d\xf1"
      "\x8e\x43\x8c\xea\x10\x04\xc8\xdb\x5b\x08\x80\x2c\x5f\xe5\x08\x5b\xe2\x27"
      "\xdb\xd9\x15\x63\xfd\x50\x52\xd6\xdd\x17\xac\x06\x24\x52\x87\x5b\x72\x7c"
      "\xcc\x75\x93\x4a\xb1\xd5\x5e\x99\xf3\x7e\xeb\x0d\xa4\x25\x00\xa7\x35\xdb"
      "\xc8\x9d\x6a\x20\x1b\x84\x67\x1c\x0a\x35\xd0\xd0\x80\xcb\x5c\x48\x65\x12"
      "\xb8\xd2\xbc\x96\x9d\x8b\xae\xe3\x5a\x5d\x7b\xfc\x28\xeb\xc0\x83\x1d\xe3"
      "\xc3\xce\xf2\x19\xc6\x5d\xaf\xec\x18\xbe\x52\xf1\x96\xe9\xd3\x78\xdc\xf3"
      "\x56\x91\x94\x08\x98\xd7\x39\x79\x88\x3a\x4d\x3b\xc3\x83\x7a\x7b\x2e\x55"
      "\xc3\x09\xbc\x9a\xc6\xce\xb4\x5b\x56\xb3\x26\x63\x5f\xca\xf9\x8e\x23\xdb"
      "\x40\x5c\x61\x34\xe6\xcf\xa3\x34\xfe\x61\x3e\xce\x91\xe3\x26\xab\x14\xbe"
      "\xb9\x6e\xc7\x55\x5f\x4a\x89\x51\x7e\xa4\x90\x38\xa5\x33\x93\xeb\xde\x9f"
      "\x7d\xe6\xec\x92\x2e\xc8\x54\xbe\xaf\x06\x11\x74\xdf\x57\x4c\xf9\x6e\x5d"
      "\xf0\x3e\x65\xe4\xf9\x8b\xa4\xe0\x27\xdb\xfe\xd1\x22\xa1\x9c\x89\x6d\x37"
      "\x1d\xae\xa7\x41\x33\x4d\xed\x38\xe2\xda\xab\x14\x42\x18\xec\x45\xb7\xe9"
      "\x40\xa9\x0d\x79\xde\x4d\xab\x32\xa7\x76\xac\x7c\x06\xef\x2f\x28\x6f\x40"
      "\xd3\xfa\x20\xb5\xbe\x1f\xa7\x4e\x55\x27\x59\xe4\x78\x4d\x95\x54\xd6\x44"
      "\xac\xe5\xc4\x77\xd4\x91\x67\xde\xec\x54\xaa\x1f\x85\x58\x5c\x33\x60\xb5"
      "\xeb\x91\xcb\x85\x29\xd1\x87\x2c\x3c\x68\x26\x90\x31\x0e\x09\xa7\xf7\x3b"
      "\xfd\x3c\x06\x60\xa4\xd4\x22\x5d\xca\x70\xd8\xf3\xbb\x0d\x68\x59\xfb\xfc"
      "\xfa\x76\x64\x28\xfc\xc8\x3a\xfa\x07\xdb\x7b\xf6\x3a\x6e\x05\xed\x8f\x29"
      "\xdb\x37\x23\x08\xbc\xb3\x17\x04\x6e\xe0\x67\xa8\xe8\x34\xb8\xf2\x82\x0c"
      "\xaa\x6d\xfb\x81\x00\x2b\xc9\xde\xac\xa5\x0b\xf8\x84\xb2\x1b\xe5\xe7\x0b"
      "\x8f\x95\x3c\xe8\x8d\x7f\xe8\x39\xed\xaa\x8a\x24\x16\x50\x68\x38\xc2\x18"
      "\x8d\x9a\x72\xe8\xb7\xb6\x08\xa2\xde\x78\x93\xad\xeb\x7b\x8b\xb8\x48\xa2"
      "\x94\xf3\x66\x42\xa7\xcb\x08\x7d\x42\x01\xc3\x39\xec\x07\xdd\x04\xde\xff"
      "\x73\x03\xdf\x23\xe0\x64\xb7\x1b\xd9\x37\x72\x49\x56\x21\xc6\x1c\x62\xf3"
      "\xbd\xd3\x7e\x52\x23\x6d\x07\xba\xe6\x23\x76\x7d\x44\x62\x9f\xbb\x17\x25"
      "\x3e\x3e\x21\x28\xad\xa2\xdc\x0d\x34\x01\x5f\xfd\xdb\x22\x9c\x52\x39\x3c"
      "\xf6\xb7\x91\x89\xa7\x0a\x48\x8d\x84\xbe\xd6\x11\x1c\x26\x06\x9b\x1d\x2b"
      "\xe2\xbd\xb1\x9f\x51\xfa\x99\x8c\x92\x60\x12\x5d\x1c\x9b\xe8\x8d\xe9\x43"
      "\x01\xc3\x22\x7f\xfc\xc8\xba\x76\x71\xe9\x17\x6f\x72\x08\xf7\x73\xc9\x6e"
      "\x24\xa9\xd8\x03\xb2\x07\x14\x81\x1a\x73\x1e\xbd\x1a\x24\x8e\x93\xb9\x3e"
      "\xd1\x12\x21\x5d\xfe\x2d\xb8\x8f\xe5\xa5\x4a\xd4\xb7\x2f\xa3\x09\xbc\xf6"
      "\xa0\x83\x61\x53\x08\xef\x7b\x6a\x7f\xbd\xeb\x76\x6a\x34\xef\x8f\x8d\x01"
      "\xb6\x85\x1f\x20\x11\x5c\x20\xf4\x24\x48\xce\xa5\xcd\x0a\x3a\xc6\x59\x15"
      "\xf2\x58\x05\x36\x3f\xa4\x35\x1b\x98\x1f\x5f\x50\x07\x43\x82\xfe\x98\xb6"
      "\x42\xd5\xe2\x88\x85\xa8\x87\x24\x7d\x50\x39\x1e\x21\xd7\x6d\x18\xfd\xcc"
      "\xbb\x46\xa4\x83\x13\x28\xf5\x32\xa5\xe8\xe1\x23\x86\x52\xc1\x81\x71\x84"
      "\xbd\x2a\x35\x97\xd0\x43\xb8\x38\x81\x65\xeb\x43\x8a\x4c\x80\xa7\x12\x29"
      "\x43\xf7\x85\xef\x50\xe2\x3e\xd0\xb6\xc9\x00\x5f\xe4\x87\x08\xde\x20\xa3"
      "\x62\x8e\xec\x8b\xc1\x31\x15\x77\x4e\x8b\x93\xb5\x2a\xc8\xdb\xea\x29\x5e"
      "\xf4\x9c\x6c\x46\x0d\xcb\x8d\xe6\xf0\x81\x35\x14\x78\x39\x87\x01\xfc\x66"
      "\x8a\xcb\x80\x94\xf1\x47\x8b\x0c\x6c\xd5\x9c\x58\xdd\xa0\x5a\x3d\x57\x83"
      "\x22\xdc\x70\xf5\x18\xd0\x57\x38\xee\x6f\x39\x98\x7c\x8f\x7f\xe3\x7b\x66"
      "\xe5\xa7\x7c\x61\x61\x41\xd7\xf7\x2e\x2d\x78\x70\x00\x90\x44\x09\x2d\x16"
      "\x55\x3e\x02\xb1\xc9\x34\xf3\xdc\x1f\x06\x6f\xdf\x46\x07\xae\xd7\xed\xf1"
      "\xff\x6e\x4f\x4c\x84\xd7\xd4\xd4\xa9\xb0\x0a\x88\xaa\x3b\x6b\xc2\x66\xc1"
      "\xe1\xa6\x76\xb3\xec\xcd\x71\x41\x46\xa4\xd4\x9c\x8f\x8a\x7e\xc5\xa8\x81"
      "\x56\x68\x29\x47\xf9\x0e\xcf\x0f\xdf\xab\x92\xee\xfd\xbf\x71\x8b\xe9\xad"
      "\xaf\x34\x50\x0b\xac\x79\x05\x89\xaa\xb6\x49\xfa\x8c\x7a\x8a\x92\x81\x0f"
      "\x58\xc0\xf4\xde\xad\xb0\x56\xc4\xfd\x91\x4f\xf8\xd5\x4b\xe2\xd3\xc9\xab"
      "\xcf\x20\xe2\x7f\x3d\x09\x18\xb6\xb6\x91\xcb\x75\x8b\x36\x5b\x28\x38\x11"
      "\xed\x1f\x29\x85\xb6\x7c\x76\xcf\x36\x5d\x8b\xa0\xe5\xdb\x99\xea\x7b\xbe"
      "\x49\xab\x7e\xc4\x06\xb1\x8e\x0a\xb1\xbe\x25\x11\x8e\xc7\x7b\xae\x55\x3c"
      "\x5a\x29\x2d\xb1\xd7\xed\x97\x38\x17\xf7\xc2\x39\xd5\xab\xcd\xdf\x08\x08"
      "\x09\xe9\x81\x0e\x33\xf7\x83\x46\xe4\x30\x5c\x36\x96\x23\x4b\x67\xb1\x68"
      "\x69\x12\x97\xe1\x47\xf7\x6a\x23\x1c\xf6\x1f\xb3\x9a\xc3\xaf\xeb\x21\x3e"
      "\xb7\x05\x09\x68\x43\x14\x42\xdc\xc3\x17\xc5\xb5\x78\x30\x4b\x44\xe3\x02"
      "\x08\x5f\x00\x64\xdd\x34\x2e\x05\xd8\x34\x3d\xcd\x6f\xac\x9b\x5c\xb3\x60"
      "\x48\x33\x81\xbf\xde\xee\xaa\xb7\x63\x9d\x14\x91\x20\x35\x44\x7e\xe3\x67"
      "\x76\x35\xd1\x3c\x61\x78\x2e\x0e\x96\x93\x97\x37\x2b\xad\x1d\x68\x2f\x5b"
      "\x79\xc9\xfe\x34\x6f\xae\x49\x60\xf9\x71\x26\xe2\xec\x01\x40\x3c\xc4\x13"
      "\xcd\xaf\x4e\x65\x65\x4d\xc5\x08\xf3\x78\xdc\x64\xd0\xb6\x5a\x40\x2a\xd0"
      "\xbf\x42\x0e\xe8\xc9\x3c\xa5\x16\xa9\xb7\x9d\xc9\xee\x94\xb1\x20\x3e\x6d"
      "\x53\xa4\x23\x20\x1d\x68\x7a\x1f\x81\xfd\xfd\x73\x24\x6e\xa3\x65\x7e\x3c"
      "\x99\x95\x90\x73\x33\x13\x9b\xc7\xd4\xe4\x57\xb7\xd0\x21\x0a\xf7\xa8\xe1"
      "\x00\x81\x3a\xb5\xc5\x2c\x54\xf5\x38\xbc\xbf\x72\x88\x6c\xf8\xed\x89\x64"
      "\x18\xb3\x7c\x6c\xdc\x15\xfc\x21\xb1\x66\x5f\xc3\x5f\x41\x26\x04\xfc\xab"
      "\x47\x9c\xb3\x6d\xf4\x4d\x69\xd3\xbb\xc8\xd1\x0a\xd4\x18\x69\x94\xfe\xba"
      "\x52\x8a\x58\xf3\x49\x8d\x02\xdf\x30\x1a\x36\xaf\xad\x05\x0e\xb7\x33\x52"
      "\x87\x8f\x2d\x87\x39\xbb\x2d\x55\xcf\xf1\x5e\xe1\xfd\x17\x7c\x3b\xa6\x5e"
      "\x8f\xa6\x9a\x0f\xfb\x78\x05\xcd\x49\x2c\xa4\x8d\x7a\xdc\xdd\xcf\x5b\x7f"
      "\x2e\xda\x76\x38\x6a\xf4\xac\xce\x60\xc2\x84\xd5\x40\x08\xac\xfd\x20\x9b"
      "\x26\x21\xd2\xbe\x29\x00\xc3\xc2\xbb\x37\xe3\xf8\xd8\x9f\x5b\x5b\x38\x5d"
      "\x8d\xc8\xd7\x97\xbe\x15\x75\x09\xb7\x85\xa2\x8d\xfc\x08\x2d\x8d\x7b\x52"
      "\x4a\x70\x4e\x67\xb6\x07\x50\xd9\xb1\xb6\x0f\x93\xf9\x22\x02\xac\x42\x0f"
      "\xaf\xbb\x68\xe5\x9f\x9a\x8c\x4c\x4f\xa4\x9d\x50\xf4\x48\xbf\x5d\xf6\x59"
      "\x0d\x1c\x4b\xbb\x1e\x87\xfd\xe2\xdd\x45",
      4096);
  *(uint16_t*)0x200015c4 = 0x1000;
  syscall(__NR_write, r[0], 0x200005c0ul, 0x1006ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  for (procid = 0; procid < 4; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
