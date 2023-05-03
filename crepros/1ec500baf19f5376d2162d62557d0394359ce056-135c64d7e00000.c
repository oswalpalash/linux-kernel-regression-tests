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
      (void*)0x20000b40,
      "\x0b\x00\x00\x00\x73\x79\x7a\x31\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x8a\x8e\x87\x4b\x73\xe9\x90\xb4"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x2b"
      "\x40\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x01\x00\x73\x8d\x7a\x31\x00\x00\x00\x00\x00\xff\x07\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x73\x79"
      "\x7a\x31\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\xcf\x0a\x00\x00\x00\x00\x00\x00\x00\x00\x0c\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x16\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x3b\x38\xe9\x67\xac\x82\x06\xea"
      "\xae\x86\xb9\x7e\xec\x0b\x2b\xed\x1e\xe2\x33\x64\xb1\x0d\x6a\xad\x51\x02"
      "\x00\x00\x00\xe2\xa5\xdb\x3c\x6a\x0d\xee\x4a\xfc\x66\xd2\x44\xfb\x3d\xa2"
      "\x68\xd5\x47\xc2\x6e\xd1\x8e\x6f\x24\x03\x01\xb6\x92\x28\x05\x20\x1c\x39"
      "\x38\x9a\x80\x4c\x41\xc2\x99\x3f\xc6\x7e\x8a\x14\x60\x45\xe1\x4a\x8a\x08"
      "\x00\x7a\xbe\x87\x0c\x06\x51\xb1\x55\x0e\x6a\x25\xc0\xef\x65\xf6\x00\x00"
      "\x00\x00\x00\x00\x00\x03\x18\x7f\xaf\xa4\xa1\xee\x6e\xce\x53\xc6\x73\x85"
      "\xb8\x83\xa3\x6a\xd2\x4a\x04\x00\x00\x00\x00\x00\x00\x00\x6a\x8a\xb1\x1b"
      "\x0a\x0b\x00\xe7\x7e\x6c\x16\x18\x9c\xfa\x16\xcb\xe0\x1a\x4c\xe4\x11\x37"
      "\x8e\xaa\xb7\x37\x2d\xab\x5e\xef\x84\xc3\x1b\x2d\xad\x86\x8a\x53\xe6\xf5"
      "\xe6\x97\x46\xa7\x86\xe5\xc8\x1e\xae\x45\xe3\xa2\x5b\x94\x2b\x8d\xa1\x1e"
      "\xdb\x57\x8b\x45\x3a\xca\xc0\x3a\x9d\x34\x48\x00\x00\x00\x00\x83\xd6\xd5"
      "\xfe\x4f\x83\x3d\x4d\x4c\xfb\xee\xf0\xe0\xe6\x2b\x7b\x09\x66\x49\xe2\x05"
      "\x00\x00\x00\x3c\x32\x98\x4c\x6c\x4b\x2b\x9c\x33\xd8\xa6\x24\xce\x00\x00"
      "\x00\x1f\xf2\x8d\x3c\xe3\xe3\xb8\xf8\x1e\x34\xcf\x97\xc9\xc8\x41\xcb\x2e"
      "\xf0\x81\x07\xa9\xa9\x65\x49\xe3\xd2\x59\xdf\x17\xe2\x9e\xd6\x4b\xd6\x12"
      "\x08\x13\xf9\xf0\x34\x4e\x13\x95\x06\x70\x1e\x8f\xde\xdb\x06\x00\x9b\x5e"
      "\x4d\x0c\x67\xbd\xa0\xb9\x28\xb7\x32\xcf\xf7\x82\xb0\x68\x40\x75\xf2\xcb"
      "\x78\x51\xef\xdd\x77\x97\xee\x95\xd2\xac\x28\xa8\xca\xbd\x26\xc1\x56\x82"
      "\xaa\x78\xd3\x1a\xec\x95\x6b\xd7\xc2\x78\x06\x40\x34\x34\xb3\xc3\x0b\x07"
      "\x0b\xcc\x82\x66\xe1\x2f\xa6\x66\x02\x05\x62\x56\xf7\x46\x75\xb7\xcb\x4d"
      "\x87\xb2\x70\x8d\x70\xc8\xf3\xdf\x53\xca\xf8\xfe\x18\x0c\x4d\xea\x3f\x5b"
      "\x7a\x92\xa3\x4b\x0e\xfa\xab\x02\xdc\xa0\x51\x7e\xee\x50\xff\x30\x20\x6f"
      "\x78\xec\x82\xc7\x2f\x33\xe9\x09\xc1\xae\x1f\xe9\x4f\xe0\x75\x9f\x77\x25"
      "\xb5\x66\x4f\xbe\xe8\x07\xd1\x64\x02\x59\x3e\x33\x35\xd4\x20\x00\x88\x8c"
      "\x90\x5f\xb4\x1d\xbd\xb4\x20\xea\x9f\x87\xfc\x86\x51\x26\x85\xeb\x72\x61"
      "\xe9\xc2\x5e\xdb\x00",
      743);
  syscall(__NR_write, r[0], 0x20000b40ul, 0x12eul);
  *(uint32_t*)0x200026c0 = 8;
  memcpy(
      (void*)0x200026c4,
      "\x4e\xa1\xbf\xd7\x0f\x5a\xeb\x0c\xc9\x7a\x8a\x47\xf7\x2d\x2c\x32\x39\xad"
      "\xfc\x94\x5c\x56\xc8\x08\x4c\x22\x42\x56\x34\x86\x82\x39\xb2\xcc\x6a\x58"
      "\x0b\x50\xd5\xa4\x7b\x84\xf4\xb8\x8c\xfe\x7d\x48\x15\xd6\xdc\xf7\xf3\x35"
      "\xad\x14\xcd\x82\x29\x8e\x3d\xa4\x69\x02\x68\x2d\x4f\xd1\x4d\xb5\xfc\x63"
      "\xcd\x87\x25\x50\xae\x49\x2e\xd9\x14\x87\x4e\x79\x0b\x7e\x22\x8d\xe7\xa7"
      "\x99\x1f\xcc\xfe\x15\xf8\x8d\x6f\x1c\xc9\x13\x76\x5e\xa9\x77\x0a\x3e\x67"
      "\xb4\xb8\xc6\x7d\x5a\x17\x8c\xe6\x92\x23\x9e\x7b\x59\x53\x4e\xf3\x92\xb5"
      "\x6c\x9c\xa8\xf0\x94\xfb\x4f\x05\xea\x79\xd7\x8b\x40\x6d\x61\xdb\xbc\x34"
      "\xf9\x5c\x97\x64\x4a\x6f\xc7\x31\xee\xc5\xdc\xb7\x5a\xe0\x20\xc2\x4a\xfa"
      "\x1a\x62\x11\xc7\x66\xa7\x50\xbf\x29\xe5\x7c\x0e\xe4\x77\x37\xa2\x2d\x99"
      "\xad\xe8\x45\xa7\x41\x67\x9d\xc3\x7e\x97\x07\x6c\x97\x7f\xbf\xba\x24\x8b"
      "\x57\x33\x47\x50\x78\xec\x92\xf7\xad\x64\xf3\x08\x61\xf0\x02\xb1\x8d\x86"
      "\x41\xb8\xb9\x08\x95\x5b\x6a\x23\x8d\xaa\xb6\xcd\xe9\xe4\x22\xed\x46\xe8"
      "\xa4\xee\x2e\x92\xde\x47\x30\x3b\x7e\x8d\x9b\x05\x12\x1a\x17\x3d\x36\x32"
      "\xec\xdd\x76\x83\x6e\x87\x0c\xb8\x8b\xbc\xa0\x92\x81\xa4\x5d\x9f\x37\x6a"
      "\x62\x48\x0e\xcc\xb8\x2f\x33\x09\x20\xcf\x5b\xfa\x6d\xf9\x78\xfd\x57\xd3"
      "\xe2\xf2\x93\xe5\xff\x95\xfa\x37\xc8\x49\x3f\xd1\xc4\xdf\x8e\x16\xd2\xf9"
      "\x0f\x98\xfe\x11\xb5\xc3\x56\x2a\x31\x29\x7d\x48\x72\xb1\x3f\x7c\xd7\x7b"
      "\xc2\xba\xfd\x35\x58\x4f\x38\x5f\xb6\x38\xc8\xa1\x33\xe0\x32\x7c\x8a\xae"
      "\xf3\x77\x63\x9c\xf9\x73\x9c\xc7\x8c\x49\xcb\x7b\x8a\x5f\xba\x96\xe9\xed"
      "\xdc\x1b\xb7\x61\xf8\xdc\x67\xfc\xd9\xd0\xc0\x56\x02\x3a\x19\xe8\x8a\x7d"
      "\x73\xb1\xc9\xa1\x30\xe2\x5e\xe9\x26\xf1\xaf\x86\x89\x5b\xde\x65\xe4\xef"
      "\xa2\x4a\xa1\x97\xef\xee\x4c\x5a\x16\xc5\xa6\x51\xce\x37\x94\x28\x3d\x39"
      "\x34\x57\xbc\xd2\x36\x19\x02\x5a\x50\xe8\x82\x9e\xce\x07\xc9\x73\xfd\xe6"
      "\x12\x80\x58\xef\xca\x24\x7e\x96\xbb\x04\x13\xc3\x3f\xbe\x97\x79\x21\xa4"
      "\x23\x7a\xd2\x3f\x85\xef\xde\x5d\x22\x36\x10\xe6\x48\x99\x44\x9d\xcb\xc7"
      "\xbd\xb1\xe9\x54\x91\x00\x27\x7d\x7c\x43\xca\x08\x04\x6a\x0f\xf2\x3e\xa3"
      "\xf6\x6f\x00\xbc\x74\x37\x95\x5c\xfe\xc0\x73\x32\x0b\xdf\xb6\xaf\x20\xac"
      "\x60\x17\x1a\xff\xdd\xce\x99\xc5\x7d\x40\x25\xbd\x06\x46\xf7\xa8\x45\xbf"
      "\x69\x04\x69\xc7\xb2\xce\x5f\xa6\xd7\xdd\xc2\x28\x92\xa2\x8e\x68\xe8\xc1"
      "\x9f\xd7\x39\x3b\x08\xaa\x16\x02\xb9\x82\xa0\x0d\x13\x5b\xb9\x42\x68\x50"
      "\x1c\x08\x39\x4c\x84\xb4\x08\x53\x4d\xd8\xd7\x90\xa4\xbe\x48\x9b\xb2\x64"
      "\x75\x0d\x21\x93\xc5\x11\xc9\xdb\x5e\x63\xa9\x7d\x53\xf5\x0d\x1f\x98\xed"
      "\x10\x92\x8b\x56\x01\x1a\x73\xba\xe4\xe4\x3b\x2e\x92\x16\x43\xf3\xb7\x42"
      "\xd7\x75\xbb\xbc\xf2\xee\xe3\xf0\xa1\xc9\xfa\xc5\xf7\xc8\x10\xe6\xb6\x83"
      "\x9f\x36\x10\x1f\x71\xeb\xe7\x7b\x15\x1a\xc8\x43\x28\xcb\xa7\xd8\xde\x95"
      "\x6c\x7f\x36\x00\xaf\x47\xe9\x46\xb8\x3b\xe0\x1a\x27\x5d\x9b\x57\xde\x08"
      "\xf3\x21\x93\x0c\x4d\x09\xc4\x3f\xc0\x9a\x0f\x13\x28\x73\xd9\xf3\x88\x43"
      "\x71\x6c\x12\x48\xdf\x0b\x4c\x41\x97\x25\xe7\x37\x90\x1b\xbe\x9b\x06\xdf"
      "\x6c\x8a\x55\x2e\x4a\x6c\x65\x28\xcd\xf6\xa0\xfa\x6b\xc8\x7c\x05\x3b\xf2"
      "\x90\xad\x8b\xb5\x9d\x70\xbb\xdf\x43\x2c\x50\xb4\xeb\x7c\x82\xf1\x89\xd8"
      "\x98\x1b\xcd\xa7\x2b\xc1\xdf\xd2\x6d\x88\x58\x38\x77\xc3\x64\x9b\xcf\x5a"
      "\x61\x05\xd4\xe4\x35\xdc\x73\xce\x18\xb6\x1f\x48\xba\x8d\x95\x1c\xa6\x1c"
      "\x57\x71\x6b\x89\xfd\x96\x0e\xc1\x45\xd1\x67\xaa\xd9\xd2\xa5\x4b\xee\x38"
      "\x0f\xe2\x82\xe8\x9b\x71\x34\x4a\x4d\x1f\x60\xfb\x69\x1a\x5b\xd9\xf3\x06"
      "\x55\x7a\xb9\x4c\x3b\x2c\x44\x6a\x9b\x19\xb4\x46\x09\xe1\xc5\xcd\x63\x4e"
      "\x9d\xd2\x73\xaa\xb2\xf0\x19\x6e\x98\x2a\x41\x94\xc1\x7c\x7e\xd2\x50\x26"
      "\xe2\xe2\xa0\xf3\x9a\xbc\x98\xd7\x2b\xe0\x18\x94\x95\x96\x5b\xca\x78\x7c"
      "\xa6\xa5\xa5\x9c\x4e\x2c\xe8\x82\xbe\xd1\x59\x1c\x4e\x9d\x77\xa1\xe3\xfc"
      "\x22\xeb\x4b\xf7\x45\x17\xb6\x5f\xe8\x5c\xee\xf9\x9c\xdc\xb8\x8b\xae\x10"
      "\x7b\xb0\xba\x67\x80\x4f\xf0\x36\xcb\x76\x91\xe8\x13\xd8\xdc\x27\xdd\x5c"
      "\x9c\x35\xbf\x9d\xa9\xb3\x49\x21\xf8\x29\xc2\x8f\xb6\xa3\xd4\x25\x93\xf3"
      "\xa6\x16\x67\xaa\x5a\x5b\xad\x81\xea\xf7\xbc\x4a\x49\x03\x8b\x8c\x10\xd3"
      "\x67\x25\x0a\xfe\xb5\xde\xc3\x69\x6a\x06\xd8\x80\x12\xc3\x52\x47\x7d\xb7"
      "\x77\x6e\xdc\x46\x5b\x15\x85\x4c\xa7\x7f\x11\x58\x73\x14\xab\x35\x6a\xac"
      "\xe7\x9a\xdb\xb7\x9f\x3b\x3c\x27\xcb\x9a\x44\x27\x1b\xf3\xff\x8c\x13\x72"
      "\xe0\xd6\x33\xaf\xe1\x73\xf9\xf9\xf9\x73\xfb\xab\xc2\x8c\x8f\x34\x96\x9b"
      "\xb5\xd2\xb4\xb2\xee\x57\x83\x04\xe4\x3d\x42\x15\x9c\x31\xc9\x13\x7c\x60"
      "\x40\x25\xb3\x12\x75\x7f\x4c\x3c\x54\xa6\x3b\x66\x75\x8a\x25\xac\x12\x2a"
      "\x3b\xcc\x95\x31\x18\x92\xce\xe4\x0a\x79\xe7\x19\xe7\x1d\xad\x53\x6e\xed"
      "\xd9\x1a\x83\xd1\xbd\x83\x55\x40\x97\x53\x5d\xda\xbc\xaa\xb6\x09\x9e\x3a"
      "\x05\xf9\xab\x9c\x92\xe6\xdd\xbd\xcf\xd0\x1e\xb9\xd7\xa1\xab\xdb\x7b\x5b"
      "\xa1\xea\xec\x60\x23\xc5\xb1\xa1\x03\x8f\x7f\x71\x9f\x82\x99\xff\x5d\x0e"
      "\x98\xd8\x22\x7d\x82\x68\xc4\xe0\xce\x76\xeb\xa6\x93\xf9\xf3\x88\xd9\x7f"
      "\x4c\xf9\xff\xdc\x6e\xeb\x05\x58\x72\x70\xa2\x50\xad\x0a\x01\xd5\x28\x3d"
      "\x3f\xb6\x63\xc9\x89\x92\xf7\x24\x0e\x39\xd1\x7d\xdb\x07\xf5\x34\x6b\xa1"
      "\x38\xa6\x37\xfa\x98\x68\x1b\xfc\xa9\x56\x41\x91\x16\xd3\x22\x42\xfa\xd5"
      "\x3f\x07\xae\x8b\xfa\xa3\xd9\xed\x72\xf1\x74\xa1\x39\x93\xc0\x04\xbf\x7d"
      "\x7b\xf4\xc4\x09\xee\x04\xc3\xad\xa6\xd3\x50\x20\x8a\x78\xb7\x23\xe2\xc4"
      "\x33\x65\x32\x8c\x88\xa4\x8a\x84\xe1\x2c\x25\x86\x14\x7d\xd8\xaa\x14\x5b"
      "\x7d\x54\xf2\xa0\xd5\xbc\xd1\xc6\x98\xd7\x7e\xa9\xa2\x38\x49\xaf\xff\x4c"
      "\x96\xd8\xb1\xa0\x71\x43\xc2\x09\xe7\xd2\xec\x82\xb7\x6c\x17\xd4\xd5\x6e"
      "\x3f\xe2\xdd\x4c\xf7\x9c\xea\x7f\x73\x41\x9f\xea\x1d\xaf\xa0\xf2\x5f\x13"
      "\x80\xe2\x89\x0e\x1c\x54\x0c\xe1\x48\xc8\x41\x90\x98\xcc\x14\x01\x40\xc3"
      "\x1c\xcd\x09\x7e\xf9\xe6\xe6\x04\x0f\x51\xd5\x94\x52\x7f\x35\xb4\xc5\xd8"
      "\x14\x4e\xbf\x47\xa3\xdc\x0a\x35\x47\xba\xdc\x67\xad\xbe\xe0\x91\x8f\xec"
      "\xe6\x12\x09\x1d\xaf\xcb\xe4\x4d\x74\x72\xda\x76\xed\x75\x2a\x43\x2a\xa9"
      "\x71\x89\x1d\xe6\xbb\x53\x0a\xfd\x98\x56\xed\xf1\xc7\x8f\x53\x21\x6b\x34"
      "\x70\x92\x58\xe6\xf8\x83\x79\xec\x82\x1f\xba\xc2\x7d\x41\x21\x4c\x46\x2e"
      "\xb5\x1d\xe9\xe7\x12\xb3\x22\x77\xcd\xca\x26\xd5\xbe\x40\x23\x20\xb5\xb4"
      "\xc5\x42\xab\xd7\xe1\xa4\x5d\x4a\x0e\x76\xf7\x5e\x60\xb4\x60\x7c\xfa\x90"
      "\xf1\x03\x78\x64\x87\x50\x8f\x2e\xfe\x77\xc6\x22\xd3\xe7\x37\xbc\x6f\x70"
      "\x6e\xe3\x58\xfd\xd8\x07\xa7\x49\x41\x50\x8f\x56\xee\x97\x8d\x84\x8b\x06"
      "\x1b\x76\xdd\x63\x51\xb2\x02\x8a\xe0\x7b\x38\x54\xe3\xcc\xd1\x44\xdf\x37"
      "\x72\xd3\xfb\xff\xdc\xc4\x75\x9b\x7d\x66\xab\xbe\x88\xc1\xbb\xd3\x1b\x81"
      "\x5e\x5d\x7d\x75\x7d\xb5\x30\x8c\x60\xa5\xeb\x2b\xcb\x24\xc0\x09\xe2\xa4"
      "\x0b\xad\xcd\x81\xcf\x6e\xeb\xbb\x46\x4d\x96\xf7\xa6\x54\xa7\xc1\x6b\xcd"
      "\xb3\x98\xd8\xaa\xe9\xb5\xcd\x0a\x5f\x3d\xc7\xf0\xf9\x16\x59\xf9\xf5\xfb"
      "\x07\xe4\x60\x27\x61\x9a\x67\x90\x68\xb7\x17\xb2\x75\x29\x2a\x74\x83\xb3"
      "\x3c\x7c\xea\xe7\x67\x4e\xa6\x15\xda\xe5\x91\x22\xc3\xb5\x6b\x85\x2f\x16"
      "\x28\xbe\xd1\x0e\xc1\xbb\xf3\x65\x68\x9a\x6a\xdd\x56\x91\x39\x62\x71\xcf"
      "\xe8\x12\xf0\xcb\x50\x4b\x33\x4c\xfd\xae\x60\xf1\xd3\x1b\x9c\xa1\x87\xa0"
      "\xc3\x77\xe5\xc6\x30\x58\x4d\x31\xde\x30\xa9\xaa\x81\x28\xbe\x5d\xc1\x6c"
      "\xb4\x39\x19\x7b\x0b\x3f\x00\xdf\xa1\x27\x3f\xf1\x0c\x94\xdc\x01\x13\xdf"
      "\xcf\x9c\x6f\x4c\x71\x46\xc2\xcf\x5e\xd9\x81\x6b\x04\x2b\xce\x6a\xdc\xb6"
      "\x07\x92\x72\x95\x12\xc9\x90\xb8\x45\x09\x26\x89\xd2\x98\x49\x4c\x14\x83"
      "\x41\xac\x31\xf4\x93\xf8\xd5\x26\xd3\x28\x2d\x5c\x85\x5f\xfa\xf5\x65\xb9"
      "\x25\x33\x51\x3d\x5a\x44\xa4\x6c\x01\x9b\xd1\x1a\xd0\xdb\x4f\xed\x73\xfc"
      "\x6f\x90\x5f\x8e\xe6\x7f\xb4\xfe\xbd\x04\x60\x09\xb8\x77\x0b\x47\x95\x5a"
      "\xd6\xd7\x5b\x80\x0d\xcd\x7c\xb8\xcb\x6a\xd0\xe8\x07\xe4\x65\xc0\xbc\x54"
      "\x3c\x36\x7d\x07\xb4\xbe\x07\x94\x88\x52\xce\x5a\x7b\x2b\x32\x79\xaf\xba"
      "\xad\xbc\x92\x7d\x9e\x42\x22\x08\x02\xab\xc0\x06\xca\x12\xeb\x6a\x5d\x96"
      "\xc6\x2e\x7a\x5a\x6c\x28\xca\x73\x2d\x84\x2d\x42\x02\xfb\x55\x2b\xa3\xe1"
      "\x9e\x7f\x80\xe9\x11\x86\xae\x75\x17\x3f\x3a\x7d\x12\x94\x06\x68\xc9\xff"
      "\x31\x54\x7a\xd5\x54\x68\x91\x0a\x86\xef\xc4\xd9\x32\x28\xfb\x75\x55\xdd"
      "\xc8\xf1\x20\x0f\x91\xbe\x90\x79\x28\x0a\x3a\x3d\x08\xa9\xbf\x33\xdd\x30"
      "\xfe\x5c\x4e\x5b\x54\x8f\xb6\x6d\xfa\xd3\x3e\x5d\xc1\xe0\xd3\x5d\x29\xb8"
      "\xf5\x94\x81\x0b\x3f\x27\x56\xc8\x24\x62\x33\x1e\xf3\xc5\x78\x8b\xa6\xb7"
      "\x44\xa7\x70\xc3\x8b\xee\x10\x5d\xbf\x35\x94\x66\xff\x54\x3f\xcc\xdb\x80"
      "\xdd\x1d\x14\x17\x56\x4b\xb3\x1f\x05\xa2\x61\xed\xc5\x68\x7e\x0d\x48\x7e"
      "\x35\xc1\x04\x7c\x05\x4d\x0c\x00\x3f\xba\xfc\x70\xb2\x97\x13\xef\x7f\x04"
      "\x9f\x1f\x81\xda\x03\x1e\xe6\x5b\x87\x29\x48\x3e\x5d\x9d\xee\xde\x45\xf3"
      "\x78\xde\xa5\xa8\x53\x0b\xff\xd7\x85\xc5\xb8\x48\xca\x86\x38\xba\xc3\x62"
      "\xa1\x96\x74\x3d\x5b\x85\x63\xa5\xbf\xd8\x1f\x72\xb8\x26\x4b\x64\x4f\xec"
      "\xeb\xf2\xa4\xe4\x4e\x53\xdf\x6d\x81\xdc\x34\xc6\xfa\xc1\xeb\x80\x48\xec"
      "\x10\x4c\x83\x3a\xae\x44\x29\x87\xa5\x3e\x50\xf7\xc3\x3d\x3d\xd4\x28\x0b"
      "\x81\x57\x03\xee\x0d\x21\xd3\x8d\xda\xb0\x8c\x8d\xb2\xdc\x7c\xa7\x75\x3d"
      "\x1d\x5f\x21\xa7\xaa\xb2\xaf\x50\xf4\x9b\xb7\x0d\xbc\x9f\x16\xfd\xfa\x31"
      "\xef\x18\x7d\x1b\xe5\x2b\xa8\x99\x48\x6a\x49\x32\xc0\x98\xdb\x90\x09\x84"
      "\x1d\xfa\x8c\x91\xf9\x66\x01\x1f\x8d\x51\x13\x69\x94\xcf\x86\x00\xa9\xca"
      "\xe4\x41\x99\x0e\x3e\xb7\x06\x38\x13\x38\x3a\x93\xd8\xed\xd6\xe1\x53\xa8"
      "\xe9\xab\x3a\x5e\x99\x16\x4c\x4d\xa6\x0f\x66\xf0\xcf\x46\x42\x22\x08\x35"
      "\xf7\x91\x48\x5e\x4c\x98\xbe\xb5\x50\x81\x3d\xa7\x16\x70\x9b\xdd\xad\xdf"
      "\xda\x4c\xcd\x1e\x8d\x02\xe6\x4f\xa4\x7c\xe2\xee\x4b\xbc\x25\xd1\x62\x01"
      "\x87\xce\xe6\x46\xb7\x7e\xf5\xad\xdc\xec\x81\x77\x71\x4a\x00\xb5\x33\xce"
      "\x59\x7b\xca\x33\x6f\x65\x6e\xd8\x68\xa9\xa1\xc7\x5a\xbf\xe7\x97\x5e\x43"
      "\x47\xcc\x36\x7d\x4f\x2a\x84\x85\x46\xa4\xb8\x48\x96\xfc\x78\x76\x19\xfa"
      "\x1d\x45\xd2\x1a\x05\xe5\x99\x60\xa0\xdc\xd8\x99\xa4\x88\xe4\x39\xd3\xa9"
      "\x00\x85\x32\xff\x6f\xf5\xa8\x5f\xb0\x32\x0f\x9b\x8e\x26\x04\x2b\xbc\x5e"
      "\x2e\xdf\xa5\x31\xfb\xca\x6e\x20\xeb\x28\xb9\x96\x60\x75\x72\x2c\xb6\x7f"
      "\x78\x1e\x13\xe3\x75\xb0\x97\x33\xa3\x24\x7d\x0a\xf8\x76\x2b\x14\x7a\x97"
      "\x18\xcd\x77\x82\xf2\xd5\x08\xdd\xfd\x23\x7b\x4e\xac\xfc\x1e\x11\x6a\x63"
      "\x27\x29\x3b\xda\x63\x81\xb0\x62\x2a\xba\x97\x8a\xf6\x8e\xe5\x18\x20\x23"
      "\xcb\x77\xe9\xbb\x83\x4e\x03\xec\x0f\x0d\x59\x04\xbe\x85\x69\x3e\xa0\x8f"
      "\x4e\x10\x18\xa6\x4c\x5a\xeb\x75\x91\x34\x98\x3d\x0b\xc1\x25\x27\x4d\x25"
      "\x43\xcc\xd3\xe6\xec\x59\x15\x09\xc9\xf2\xa2\x70\xce\x00\xc1\x52\x48\x42"
      "\x0a\x1e\x1c\x17\x2f\x67\xa1\x9c\x11\xa1\x79\x49\xf0\x04\x6f\x4f\x2f\x53"
      "\xa8\xb7\xaa\x0f\xd0\x83\xdc\x5f\x2c\xc4\x1d\x1a\x81\x6e\xfb\x90\x46\xef"
      "\x46\x3e\xe9\x60\x3c\x91\xf7\xbf\x2a\xe1\xc5\xfa\x59\xab\xdf\x6c\xcb\xb7"
      "\x90\xf3\x7a\xaf\xd9\x20\x83\xc0\x28\xb4\xe0\xba\x1d\x80\xd3\xc0\xa6\x54"
      "\x45\xea\x3c\x54\x4f\x08\x90\x31\x45\x41\x35\x92\x61\x97\x5c\xdc\xca\x27"
      "\xbe\xb5\x0c\x6b\x2e\x69\x9c\x43\xf0\xfc\x10\xed\x5c\xfc\xef\x06\xc0\x03"
      "\x65\x29\x57\xdc\x7b\x67\x80\x05\x39\xd9\x2c\x7b\x5d\xb6\xb6\xb9\x9f\x3b"
      "\x84\x87\xc8\x06\x28\x93\xed\x35\x5d\x27\x6d\xf9\xb0\x45\x5c\x93\xa3\x0a"
      "\x79\xc8\x5f\x6d\x9a\xa1\xa2\xad\x62\x82\x0f\x6a\x2d\x58\xb6\xe1\x50\x75"
      "\x4a\x9c\xda\xff\x48\x5b\x22\xd2\x4e\x86\x11\xe9\x17\xf6\xe9\x9e\x1e\x7e"
      "\x20\x7a\x6f\x9e\x92\x30\xca\xbc\x5c\x41\x97\xc0\x86\x1c\xe4\x21\x39\x79"
      "\x97\xcd\x08\xe6\x01\x5a\x4d\x1c\xa8\xfc\xc7\x79\xa6\x83\x7d\x9a\x72\x33"
      "\x6b\x00\x90\x4d\x53\x1d\x81\x7d\x0f\xef\x6d\x58\xfa\x6a\x51\x42\xbf\x17"
      "\x82\xc9\x17\xf3\x4f\x69\x9d\x0f\x98\x28\x49\xba\x40\x19\x94\x0f\xaf\x32"
      "\x48\x21\xb2\x1d\x7d\x60\x73\x9f\x35\x91\xb1\x3e\x2f\xbf\xd0\xf0\xa8\x6b"
      "\x05\xf7\x92\xff\xba\xd3\x24\xec\x07\xb7\x99\x0b\xd3\x69\x62\x6e\x20\x54"
      "\x7b\xd8\xec\xf1\x21\xf3\x42\x90\x1c\xb7\xef\x36\x49\x3b\x15\x0d\xff\xf5"
      "\xf6\x47\x0e\xd4\x8f\xfd\xca\xd1\x61\x7e\xc0\xd3\xac\xa9\x3b\xf2\xba\x1e"
      "\x23\xd2\xef\xa3\x2c\xb5\x42\xb1\x0e\x5a\xa1\x7c\x5d\xc9\x65\x57\xde\x6d"
      "\xd6\xf2\xa2\xeb\xf9\xa2\x44\x1e\x4f\xd2\x53\x4e\x76\x2d\x24\xaa\xf5\xa8"
      "\x75\x88\xdc\x0f\x36\x49\x12\x44\xd9\x37\x21\x9d\xdc\xeb\x3a\x29\x1a\x7d"
      "\x09\xa6\x53\x09\xa5\x3a\xcb\xc7\x23\xa6\xd7\x9b\xda\xb4\x4a\x43\xfc\x6d"
      "\x38\x94\xe8\x61\x73\x4a\x7e\x9a\xdd\xee\xea\x91\xf5\xc7\x74\x7a\x79\xcb"
      "\xf8\x11\xe2\xe2\x71\x00\x35\xf5\x60\x89\x45\xc1\xc5\x56\x37\x63\xd6\x06"
      "\x0c\xd4\xfb\x91\x40\x92\x48\xde\x58\x9a\x49\x68\x80\x2f\x2f\xe9\x10\xef"
      "\x83\xb7\xde\xc0\x56\x47\x37\x37\xb8\xaf\xf5\x2f\xea\x98\x46\xb8\x1c\x54"
      "\x16\xe2\xd9\x47\xff\x18\x81\x90\x80\x52\x48\x64\x5a\xf5\x59\x4c\xb4\x79"
      "\x65\xe1\x53\x57\x29\x49\xe3\xb4\x93\xf4\xa9\xc6\x6e\xe5\x77\x0d\x4d\xc6"
      "\xf3\x57\x4b\x26\xed\x95\xfe\x47\xdf\xc9\x24\x13\xc0\x61\x87\xb8\x9c\x86"
      "\xe5\xb8\x10\x45\x45\x2d\x6c\x2a\x5c\x92\xb1\x9f\xf4\xd8\x68\x6a\x9a\x34"
      "\x71\x58\xce\x30\xb2\x2a\x89\xb0\x61\x0a\x05\x28\xdb\x65\x82\xbd\x24\xe5"
      "\xbc\x23\x41\x8a\xe5\x46\xf5\xe0\x78\xac\xcf\xbc\x47\x3d\x47\x61\xe1\x61"
      "\x68\xea\x6a\x88\x7c\xa7\x11\x9b\xbe\xf0\xf8\xaf\xa3\x03\xa0\x80\x8f\x28"
      "\x4c\xcf\x28\xec\xc4\x92\x14\xfa\xb5\x04\x04\x57\x9d\xac\x9b\x79\xe0\x95"
      "\xbf\x80\x09\xe7\x94\x1c\x35\xea\xa7\xb4\x6e\x7e\x26\xfc\x05\x21\x3d\xf4"
      "\x2e\xd2\x08\xa1\x4d\xfa\x2a\xb7\xb7\x0d\xd5\x1f\x26\x67\xdc\xc6\x4b\x76"
      "\xbb\x12\x3c\x33\x88\x18\x04\xff\xe5\xaa\x33\xee\x5e\x67\x8f\x55\x64\x23"
      "\x64\x80\x2e\x61\x49\xc8\x1b\x90\xca\x51\x5a\x83\x43\x30\x7b\x32\x1b\xc4"
      "\x83\x09\xfc\x3e\x53\x18\x75\xd3\x98\x91\xd1\x2b\xb1\x04\x76\xd2\xc1\x66"
      "\xcd\x72\x49\xfc\xd1\x0b\x7f\x9c\x8f\x38\xe5\x18\x9e\xae\x91\xd8\x0c\xaa"
      "\x50\x91\xf4\xbc\x91\x0b\x3c\xb4\xfe\xee\xf1\x58\xeb\x7a\xb6\xb3\x9e\x6f"
      "\x4b\x0a\x89\x74\x09\x47\x40\xaf\x5e\xde\x23\x24\x5c\xc1\x2d\xb9\x63\xf4"
      "\xed\x57\x27\xc8\x63\x63\xc1\x56\xa2\x9a\x96\xe4\x6a\xf4\x62\x48\xe4\xc0"
      "\x69\x6f\x98\x41\xde\x5d\x10\x05\x8b\xe7\xe5\xfd\x48\x45\x8d\x2f\x00\x1a"
      "\xe8\x67\x42\x41\xf9\xa7\x06\xcb\x4a\x50\x4b\x96\x0b\x48\x6d\xfd\xbe\x6a"
      "\xc4\xe8\x50\x1a\x55\x95\xaa\x58\xad\x07\x4b\xd4\x3e\x70\x28\x6b\xb2\xd6"
      "\xdb\xa6\x63\xcc\x12\xae\xe3\xe0\xb0\x47\x22\x38\xc3\x52\x86\x5e\x31\x96"
      "\xca\x7f\x71\x00\x13\xb3\x42\xbd\xc1\xa5\xab\x6d\x22\x41\xd0\x56\x21\x1d"
      "\xe0\x03\xc7\x81\x60\xda\x2a\xf4\x52\xe4\xea\x5e\x70\x09\x4e\x30\xab\x55"
      "\x4d\x05\x81\xc7\x37\xa3\xf3\x68\x85\x98\x58\xc7\x09\xe9\x56\x12\xed\x27"
      "\x15\x7f\x85\x4a\x89\xf7\xd8\x96\x35\x3f\xc4\xb6\x02\xcc\x67\x35\x3c\x6c"
      "\xc3\x15\x48\x1d\x90\xfc\x07\x35\xc1\x3f\xf2\x9f\xc9\x56\xc0\x4d\x0b\xbb"
      "\x0a\xa7\xd4\x73\xda\xcf\xde\xa7\x92\x81\xe8\xa9\xbe\x31\x76\xe5\x1c\x74"
      "\xd4\x60\xe2\xd2\xdc\x50\x8c\xc9\x46\x99\x7e\x8e\x39\x46\xb3\x8e\x14\xac"
      "\x1b\xd4\xab\xa6\xaa\x6a\x9a\xae\xf6\xb9\xc6\xa9\xb5\xee\xbf\xf1\xb8\x37"
      "\x13\xd7\x72\x74\x37\xcb\x58\x6c\xc0\x86\xf7\xfa\x5b\x2a\xed\xd3\xdb\x2a"
      "\xfd\x1c\x09\x29\xf8\x74\x05\xff\x0e\xb5\x2a\xd7\xa9\x1c\x35\x3f\xa4\x9f"
      "\xba\x98\xe9\x4e\xe1\x6c\x89\x29\xb9\xf9\xba\x78\xa0\xa2\xb8\x65\x86\xa3"
      "\xdb\x2c\x9e\xc1\x0a\xe2\xaa\x5f\x8d\x3f\xef\x9b\xd7\x68\xb5\x27\x05\xe4"
      "\x77\x38\x52\xa0\x17\x37\x70\xe4\x3f\x4c\x83\x66\xa5\x61\x78\xe5\x90\x6c"
      "\x16\x40\xfa\x45\x3a\x99\x7e\xb0\x8b\x03\xa5\x62\x80\xb1\xb5\x92\xc7\x51"
      "\xb5\x95\x8b\x6f\x9a\x0c\x92\x80\x94\x5b\xed\xfe\x0d\x15\x90\x0f\x79\x74"
      "\xf3\x2f\xdd\xaa\x48\x96\x13\xd3\x15\xc9\x75\x5d\x2b\x90\x26\x33\x86\x16"
      "\x84\x8d\x47\x3d\x95\x78\x20\x55\xb2\x93\x5f\xec\x18\x5c\xb6\x3f\x3a\x00"
      "\x5a\x35\x06\xaf\x10\x86\xef\x13\x08\x76\x25\xaa\x7e\x15\xeb\x4a\x74\x3c"
      "\x59\xe3\x07\x8e\x2c\x54\x07\xd0\x35\x1f\xf8\x17\xa1\x2c\xa1\x6d\x54\xe4"
      "\xdd\x4d\x07\x82\x05\x40\xcc\x33\x00\xa8\xb5\x51\x67\x3e\x6e\xf2\x43\x85"
      "\xe0\x1f\xb6\xc0\xab\xd8\xca\x77\x7c\x79\x21\x72\x16\xa7\x2a\xc7\x18\x0a"
      "\x02\x52\x5f\x7c\x14\x50\x51\x02\x4c\x92\xeb\x83\x8d\xad\x60\x41\xe8\x01"
      "\x88\x3e\x64\xd3\xe2\x80\xc5\xd0\xd2\x77\x74\x42\x2f\x81\xbb\xf1\x22\x48"
      "\x09\xbc\x37\xf8\x82\xa5\x63\x95\x33\xe7\xc1\x24\xa9\x5b\x7a\x89\x8b\x36"
      "\x9c\xf7\xbb\xc8\x6e\x64\xac\x3b\xa4\x81\x31\x2a\x79\xad\xe1\xa6\x5a\x88"
      "\x58\x19\x73\x52\x29\xbb\x79\x92\x70\xff\xfb\x7b\x5f\x87\xeb\xea\x9f\x8a"
      "\x70\x05\x40\x6a\x65\x83\xda\xa8\x91\x3b\x37\x99\xea\x3f\x2c\xcb\x25\x46"
      "\x32\x42\x98\x75\x26\x43\xf9\x02\xb1\x27\x97\xde\xbb\xcf\x70\x2e\xd5\x82"
      "\x2c\xd0\x1c\x74\x1e\x1e\x6a\x97\x5b\xac\x69\xee\xf8\x92\xb9\xa7\x79\x18"
      "\xf5\x89\xa8\x57\x30\x21\x53\x04\x01\xf5\xc9\x79\x1b\x57\x6e\xa6\x57\x6e"
      "\x6c\x89\x79\x7e\x17\xc3\xa7\x90\xdb\xa0\x02\x15\xe3\x5d\xc5\x6e\xd1\x1f"
      "\xda\x8d\xe1\xcf\xde\x92\xd3\xa1\xc1\x8d\x53\x71\xfd\x24\xd5\x69\x77\x0b"
      "\xd4\xe4\x59\x07\x3c\x52\x83\x4e\xf7\x18\x7e\xba\x4d\x00\x51\xe1\xe2\xfd"
      "\xf9\x57\x1c\x09\x2a\x29\xc6\xef\xa7\x8e\x2e\x66\xb6\xc2\x96\x12\x2c\xb4"
      "\x20\x96\xa1\x9e\xf2\xb3\xf0\x28\x81\x9d\x8a\x21\xc5\x96\x55\x2f\xbf\xcf"
      "\xcc\x6d\x7a\x32\x04\xa1\x1e\x1c\xe2\x19\xa7\x27\xcf\x6f\x78\x98\x6e\x62"
      "\x6a\x0c\x72\x88\xea\xb8\x44\x1d\x39\xa8\x15\x3a\xb9\x00\x03\x13\xf8\xe9"
      "\x39\xc1\x98\xbe\x7e\x28\x8a\xe5\x4c\x2c\x06\x0e\xe3\x38\x58\x78\x9e\x4c"
      "\xee\x2a\x7d\xcb\xfc\x29\x98\x4c\xfc\x24\xe3\x3e\xa3\xcc\xcc\x7e\xcf\x55"
      "\x59\x69\xde\x2e\x78\xa3\x72\x2d\x95\x22\x20\xc1\xf6\x06\x8a\x5c\x80\xbd"
      "\xf7\xe7\x52\x9f\xb2\xf1\xe0\x62\x02\x2d\x20\x84\xac\x17\x4f\x0a\x2d\x95"
      "\x55\x3a\x38\x67\xbe\x06\x14\x1a\x72\x9a\x99\x4a\x73\x4a\x71\x5c\xb4\x64"
      "\x97\x7d\xb2\xf3\xc8\x90\x6f\x81\x6d\xe6\xd5\x8d\x51\x95\xfb\xc5\xf8\x69"
      "\xa3\xdb\x82\x3a\x86\x47\x4c\xc8\x3b\xa0\xd6\xff\x4b\x24\x86\xe2\xe0\x9a"
      "\x13\x03\x41\xae\x82\x63\xd0\xd0\xc4\x0c",
      4096);
  *(uint16_t*)0x200036c4 = 0x1000;
  syscall(__NR_write, r[0], 0x200026c0ul, 0x1006ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0ul);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
