// https://syzkaller.appspot.com/bug?id=f4d718d83eb69809d43c70555234f8b96010e83f
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
      if (current_time_ms() - start < 5000) {
        continue;
      }
      kill_and_wait(pid, &status);
      break;
    }
  }
}

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  *(uint32_t*)0x20000080 = 0x11;
  *(uint32_t*)0x20000084 = 8;
  *(uint64_t*)0x20000088 = 0x20001000;
  memcpy(
      (void*)0x20001000,
      "\x62\x0a\xf8\xff\x0c\x20\x00\x21\xbf\xa1\x00\x00\x00\x00\x00\x00\x07\x01"
      "\x00\x00\xf8\xff\xff\xff\xb7\x02\x00\x00\x03\x00\x00\x00\xbd\x12\x00\x00"
      "\x00\x00\x00\x00\x85\x00\x00\x00\x06\x00\x00\x00\xb7\x00\x00\x00\x00\x00"
      "\x00\x00\x95\x00\x00\x00\x00\x00\x00\x00\x3f\xaf\x4f\x2a\xa3\xd9\xb1\x8e"
      "\xd8\x12\x9d\xe2\xc4\x9e\x80\x20\xa6\xf4\xe0\xe4\xa9\x44\x6c\x76\x70\x56"
      "\x89\x82\xb4\xe0\x20\xf6\x98\x39\x3a\xa0\xf3\x88\x1f\x9c\x24\xaa\x56\xf1"
      "\x51\x99\xfa\xd0\x09\x3c\x59\xd6\x6b\x5e\xce\x9f\x36\xc7\x0d\x0f\x13\x90"
      "\x5e\xa2\x3c\x22\x62\x4c\x9f\x87\xf9\x79\x3f\x50\xbb\x54\x60\x40\x67\x7b"
      "\x0c\x50\x77\xda\x80\xfb\x98\x2c\x1e\x94\x00\xe6\x93\x14\x6c\xea\x48\x4a"
      "\x41\x5b\x76\x96\x61\x18\xb6\x4f\x75\x1a\x0f\x24\x1b\x07\x2e\x90\x08\x00"
      "\x08\x00\x2d\x75\x59\x3a\x28\x6c\xec\xc9\x3e\x64\xc2\x27\xc9\x5a\xa0\xb7"
      "\x84\x62\x57\x04\xf0\x7a\x72\xc2\x91\x84\xff\x7f\x00\x00\xce\xf8\x09\x60"
      "\x60\x56\xfe\x5c\x34\x66\x4c\x0a\xf9\x36\x0a\x1f\x7a\x5e\x6b\x60\x71\x30"
      "\xc8\x9f\x18\xc0\xc1\x08\x9d\x8b\x85\x88\xd7\x2e\xc2\x9c\x48\xb4\x5e\xf4"
      "\xad\xf6\x34\xbe\x76\x32\x89\xd0\x1a\xa2\x7a\xe8\xb0\x9e\x00\xe7\x9a\xb2"
      "\x0b\x0b\x8e\x11\x48\xf4\x9f\xaf\x2a\xd0\x00\x00\x00\x00\x00\x00\x00\x6f"
      "\xa0\x3c\x64\x68\x97\x20\x89\xb3\x02\xd7\xbf\x60\x23\xcd\xce\xdb\x5e\x01"
      "\x25\xeb\xbc\x08\xde\xe5\x10\xcb\x23\x64\x14\x92\x15\x10\x83\x33\x71\x9a"
      "\xcd\x97\xcf\xa1\x07\xd4\x02\x24\xed\xc5\xb6\x88\x93\x2b\x77\xe7\x4e\x80"
      "\x2a\x0d\x42\xbc\x60\x99\xad\x23\x00\x00\x00\x80\x00\x6e\xf6\xc1\xff\x09"
      "\x00\x00\x00\x00\x00\x00\x10\xc6\x3a\x94\x9e\x8b\x79\x55\x39\x4f\xfa\x82"
      "\xb8\xe9\x42\xc8\x91\x12\xf4\xab\x87\xb1\xbf\xed\xa7\xbe\x58\x66\x02\xd9"
      "\x85\x43\x0c\xea\x01\x62\xab\x3f\xcf\x45\x91\xc9\x26\xab\xfb\x07\x67\x19"
      "\x23\x7c\x8d\x0e\x60\xb0\xee\xa2\x44\x92\xa6\x60\x58\x3e\xec\xdb\xf5\xbc"
      "\xd3\xde\x3a\x83\x20\x9d\xa1\x1c\xc5\x91\x4a\x54\xc4\x5d\x72\x7a\xa5\xfa"
      "\x68\x48\x03\x66\xc9\xc6\xfd\x6f\xa5\x04\x3a\xa3\x92\x6b\x81\xe3\xb5\x9c"
      "\x9b\x08\x1d\x6a\x08\x00\x00\x00\xea\x2b\x1a\x52\x49\x6d\xfc\xaf\x99\x43"
      "\x14\x12\xfd\x13\x4a\x99\x63\x82\xa1\xa0\x4d\x5b\xb9\x24\xcf\xe5\xf3\x18"
      "\x54\x18\xd6\x05\xff\xff\x9c\x4d\x2e\xc7\xc3\x2f\x20\x95\xe6\x3c\x80\xaf"
      "\xf9\xfa\x74\x0b\x5b\x76\x32\xf3\x20\x30\x91\x6f\x89\xc6\xda\xd7\x60\x3f"
      "\x2b\xa2\xa7\x90\xd6\x2d\x6f\xae\xc2\xfe\xd4\x4d\xa4\x92\x8b\x30\x14\x2b"
      "\xa1\x1d\xe6\xc5\xd5\x0b\x83\xba\xe6\x16\xb5\x05\x4d\x1a\x7c\x13\xb1\x35"
      "\x5d\x6f\x4a\x82\x45\xff\xa4\x99\x7d\xa9\xc7\x7a\xf4\xc0\xeb\x97\xfc\xa5"
      "\x85\xec\x6b\xf5\x83\x51\xd5\x64\xbe\xb6\xd9\x52\xaa\xb9\xc7\x07\x64\xb0"
      "\xa8\xa7\x58\x3c\x90\xb3\x43\x3b\x80\x9b\xdb\x9f\xbd\x48\xbc\x87\x34\x95"
      "\xcb\xff\x8a\x32\x6e\xea\x31\xae\x4e\x0f\x75\x05\xeb\xf6\xc9\xd1\x33\x30"
      "\xca\x00\x6b\xce\x1a\x84\x52\x1f\x14\x51\x8c\x9b\x47\x6f\xcc\xbd\x6c\x71"
      "\x20\x16\x21\x98\x48\x62\x4b\x87\xce\xc2\xdb\xe9\x82\x23\xa0\xeb\x4f\xa3"
      "\x9f\x6b\x5c\x02\xe6\xd6\xd9\x07\x56\xff\x57\x90\x2a\x8f\x57\x00\x00\x00"
      "\x00\x97\x00\xcf\x0b\x4b\x8b\xc2\x29\x41\x33\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00"
      "\x8b\xc0\xd9\x55\x97\x11\xe6\xe8\x86\x1c\x46\x49\x5b\xa5\x85\xa4\xb2\xd0"
      "\x2e\xdc\x3e\x28\xdd\x27\x1c\x89\x62\x49\xed\x85\xb9\x80\x68\x0b\x6c\x4a"
      "\x00\x00\x00\x00\xff\x43\x5a\xc1\x5f\xc0\x28\x8d\x9b\x2a\x16\x9c\xdc\xac"
      "\xc4\x13\xb4\x8d\xaf\xb7\xa2\xc8\xcb\x48\x2b\xac\x0a\xdb\x02\xd9\xba\x96"
      "\xff\xff\xff\x7f\x00\x00\x10\x00\x00\x00\x00\x00\x00\x7d\x5a\xd8\x97\xef"
      "\x3b\x7c\xda\x42\x01\x3d\x53\x04\x6d\xa2\x1b\x40\x21\x6e\x14\xba\x2d\x6a"
      "\xd5\x65\x6b\xff\xf1\x7a\xdd\xae\xda\xb2\x5b\x30\x00\x2a\xbb\xba\x7f\xa7"
      "\x25\xf3\x84\x00\xbe\x7c\x1f\x00\x1b\x2c\xd3\x17\x90\x2f\x19\xe3\x85\xbe"
      "\xac\x48\xdc\xcf\xf7\x29\x43\x32\x82\x83\x06\x89\xda\x6b\x53\xb2\x63\x33"
      "\x98\x63\x29\x77\x71\x42\x9d\x12\x00\x00\x00\x33\x41\xbf\x4a\x00\xfc\xa0"
      "\x49\x3c\xf2\x9b\x33\xdc\xc9\xff\xff\xff\xff\xff\xff\xff\xd3\x9f\xec\x22"
      "\x71\xff\x01\x58\x96\x46\xef\xd1\xcf\x87\x0c\xd7\xbb\x23\x66\xfd\xe4\x1f"
      "\x94\x29\x0c\x2a\x5f\xf8\x70\xce\x5d\xfd\x34\x67\xde\xcb\x05\xcf\xd9\xfc"
      "\xb3\x2c\x8e\xd1\xdb\xd9\xd1\x0a\x64\xc1\x08\x28\x5e\x71\xb5\x56\x5b\x17"
      "\x68\xee\x58\x96\x9c\xed\x59\x52\x29\xdf\x17\xbc\xad\x70\xfb\x40\x21\x42"
      "\x8c\xe9\x70\x27\x5d\x13\xb7\x82\x49\x78\x8f\x11\xf7\x61\x03\x8b\x75\xd4"
      "\xfe\x32\xb5\x61\xd4\x6e\xa3\xab\xe0\xfa\x4d\x30\xfa\x94\xef\x24\x18\x75"
      "\xf3\xb4\xb6\xab\x79\x29\xa5\x7a\xff\xe7\xd7\xfa\x29\x82\x2a\xea\x68\xa6"
      "\x60\xe7\x17\xa0\x4b\xec\xff\x0f\x71\x91\x97\x72\x4f\x4f\xce\x10\x93\xb6"
      "\x2d\x7e\x8c\x71\x23\xd8\xec\xbb\xc5\x5b\xf4\x04\xe4\xe1\xf7\x4b\x7e\xed"
      "\x82\x57\x1b\xe5\x4c\x72\xd9\x78\xcf\x90\x6d\xf0\x04\x2e\x36\xac\xd3\x7d"
      "\x7f\x9e\x11\x9f\x2c\x06\xf8\x15\x31\x2e\x0c\xfe\x22\x2a\x06\xf5\x6d\xd0"
      "\x22\xc0\x74\xeb\x8a\x32\x2f\xb0\xbf\x47\xc0\xa8\xd1\x54\xb4\x05\xc3\x7f"
      "\xea\xf3\xdd\x95\xf6\xef\x2a\xcd\x1f\xe5\x82\x78\x61\x05\xc7\xdf\xa2\xe5"
      "\x87\x70\x50\xc9\x13\x01\xbb\x99\x73\x16\xdb\xf1\x78\x66\xfb\x84\xd4\x17"
      "\x37\x31\xef\xe8\x95\xff\x2e\x1c\x55\x60\x92\x6e\x90\x10\x9b\x59\x85\x02"
      "\xd3\xe9\x59\xef\xc7\x1f\x66\x5c\x4d\x75\xcf\x24\x58\xe3\x54\x2c\x90\x62"
      "\xec\xe8\x4c\x99\xa0\x61\x88\x7a\x20\x63\x9b\x41\xc8\xc1\x2e\xe8\x6c\x50"
      "\x80\x40\x42\xb3\xea\xc1\xf8\x79\xb1\x36\x34\x5c\xf6\x7c\xa3\xfb\x5a\xac"
      "\x51\x8a\x75\xfd\xe7\xd7\x10\x1d\x5e\x18\x6c\x48\x9b\x3a\x06\xfb\x99\xe0"
      "\xaa\x7f\x23\xa0\x54\xde\x2f\x4d\x92\xd6\xbd\x72\xee\x2c\x9f\xdc\x75\xaa"
      "\xaf\x1e\x3e\x48\x3b\x4a\xd0\x55\x73\xaf\x40\x32\x69\xb4\xa3\x9c\xe4\x02"
      "\x93\x94\x7d\x9a\x63\x1b\xcb\xf3\x58\x37\x84\xac\xbd\xa2\x16\x55\x0d\x7a"
      "\xec\x6b\x79\xe3\x0c\xbd\x12\x8f\x54\xc2\xd3\x4b\x54\x57\xac\xf3\x73\x31"
      "\x76\x6e\x47\x23\x91\xe3\x58\xc3\xb3\x77\x32\x7a\xc9\xec\xc3\x4f\x24\xc9"
      "\xae\x15\x3e\xc6\x0a\xc0\x69\x4d\xc5\x5b\xff\x9f\x5f\x4d\xf9\x04\x00\x00"
      "\x00\x00\x00\x00\x00\xd6\xb2\xc5\xea\x13\x93\xfd\xf2\x62\x85\xbf\x16\xb9"
      "\x9c\x9c\xc0\xad\x18\x57\x21\x6f\x1a\x98\x5f\x36\x91\x91\xae\x95\x4f\xeb"
      "\xb3\xdf\x46\x4b\x63\x51\x97\x35\x1a\x5e\xf0\xa0\xfb\x89\xd2\x77\x73\x99"
      "\xf5\x87\x44\x55\x3a\xeb\x37\x29\xcf\xfe\x86\xe6\x69\x64\xae\x09\xbb\x6d"
      "\x16\x31\x18\xe4\xcb\xe0\x24\xfd\x45\x22\x77\xc3\x88\x7d\x61\x16\xc6\xcc"
      "\x9d\x80\x46\xc2\x16\xc1\xf8\x95\x77\x8c\xb2\x51\x22\xa2\xa9\x98\xde\x44"
      "\xae\xad\xea\x2a\x40\xda\x8d\xac\xcf\x08\x08\x42\xa4\x86\x72\x17\x37\x39"
      "\xf9\x6d\x92\x05\x20\x95\x62\xa0\x0c\xbf\x3a\x74\xcb\x20\x03\x01\x6f\x15"
      "\x14\x21\x6b\xdf\x57\xd2\xa4\x0d\x40\xb5\x1a\xb6\x3e\x96\xec\x84\x85\xe2"
      "\x3e\xe1\xf4\x62\x20\x3b\x4f\x31\x00\xc2\xe0\x89\x38\x62\xee\xf5\x52\xfc"
      "\xde\x29\x81\xf4\x8c\x48\x2b\xde\x8a\x16\x8c\x3f\x5d\xb2\xfe\xa6\xf2\x6e"
      "\x4a\x43\x04\xe5\x0c\x34\x9f\x4f\x47\xce\xe2\x7d\xef\xc9\x38\x71\xc5\xf9"
      "\x9b\x35\x5b\x72\xd5\x38\xba\x49\x58\xea\x8e\x4a\xa3\x70\x94\x19\x1e\x10"
      "\x09\x6e\x7e\x60\xfc\x35\x41\xa2\xc9\x05\xa1\xa9\x5e\x95\x71\xbf\x38\xae"
      "\x19\x81\xc4\x23\x8e\xca\xee\x6f\x75\xcd\x0a\x68\x81\xbd\x15\x17\xa8\x25"
      "\x0d\xf9\x86\x74\x15\x2f\x94\xe3\x24\x09\xe2\xa3\xbc\xe1\x09\xb6\x00\x00"
      "\x00\x00\x00\x00\x00\x00\xd6\xd5\x21\x0d\x75\x60\xeb\x92\xd6\xa9\x7a\x27"
      "\x60\x2b\x81\xf7\x63\x86\xf1\x53\x5b\xef\x14\x97\xf9\x21\x86\x08\x6e\x29"
      "\xc6\xbc\x5a\x1f\xad\x6e\xc9\xa3\x11\x37\xab\xf9\x7a\xd2\x70\xc1\x0b\x0f"
      "\x39\xa3\xf6\xf4\x60\x44\x71\x06\x93\xb6\x11\xcf\x7a\xb7\x96\x6f\xc7\xa4"
      "\x04\xab\xde\x77\x50\x89\x8b\x1b\xd6\x27\xe8\x73\x06\x70\x3b\xe8\x67\x2d"
      "\x70\xd1\xab\x57\x07\x52\x28\xa9\xf4\x6e\xd9\xbd\x1f\x08\xfb\x81\x91\xbb"
      "\xab\x2d\xc5\x1d\xe3\xa6\x1f\x08\x68\xaf\xc4\x29\x48\x59\x32\x3e\x7a\x45"
      "\x31\x9f\x18\x10\x12\x88\xa0\x26\x88\x93\x37\x37\x50\xd1\xa8\xfe\x64\x68"
      "\x0b\x0a\x3f\xc2\x2d\xd7\x04\xe4\x21\x4d\xe5\x94\x49\x12\xd6\xc9\x8c\xd1"
      "\xa9\xfb\xe1\xe7\xd5\x8c\x08\xac\xaf\x30\x23\x5b\x92\x8a\x31\xd2\xec\xa5"
      "\x5f\x74\xa2\x36\x41\xf6\x1f\x2d\x5b\x30\x8c\xf0\xd0\x31\xb0\xc7\xf0\xce"
      "\xd6\x99\x93\xe9\x96\x0f\xf5\xf7\x60\x15\xe6\x00\x97\x56\x23\x7b\xad\xf4"
      "\xe7\x96\x5b\xbe\x27\x77\xe8\x08\xfc\xba\x82\x1a\xa8\xe8\xc5\xc3\x96\x09"
      "\xff\x85\x43\x56\xcb\x49\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\xc1\xfe\xe3\x0a\x3f\x7a\x85\xd1\xb2\x9e\x58\xc7\x76\x85\xef\xc0\xce"
      "\xb1\xc8\xe5\x72\x9c\x66\x41\x8d\x16\x9f\xc0\x2a\xa1\x88\x54\x6b\x3a\xd2"
      "\xa1\x82\x06\x8e\x1e\x3a\x0e\x25\x05\xbc\x7f\x41\x01\x96\x45\x46\x6a\xc9"
      "\x6e\x0d\x4b\x3b\xc1\x9f\xaa\x54\x49\x20\x9b\x08\x5f\x3c\x33\x4b\x47\xf0"
      "\x67\xbb\xab\x40\x74\x3b\x2a\x42\x8f\x1d\xa1\xf6\x8d\xf7\x5c\xf4\x3f\x8e"
      "\xcc\x8d\x37\x26\x60\x21\x11\xb4\x0e\x76\x1f\xd2\x10\x81\x92\x03\x82\xf1"
      "\x4d\x12\xca\x3c\x34\x31\xee\x97\x47\x1c\x78\x68\xdc\xda\x7e\xaa\x69\xeb"
      "\x7f\x7f\x80\x57\x2f\xdd\x11\xbb\x1d\x0d\x12\x80\xfb\xc2\x2b\xf7\x34\x68"
      "\x78\x8d\xf5\x17\x10\xd7\xd3\x1c\x63\x2f\xc5\xed\x17\x62\xeb\x0b\x42\x8e"
      "\xe7\x51\xc4\x7d\x8e\x89\xfc\x74\x5a\x86\x84\x04\xa0\xbf\x35\xf0\x12\x10"
      "\x08\xb7\x22\xb1\xea\xa6\xae\xdf\xa1\xbf\x2e\x7c\xcb\x2d\x61\xd5\xd7\x63"
      "\x31\x94\x5e\x20\xfa\x26\xb8\x47\x1d\x42\x64\x52\x88\xd7\x22\x6b\xbd\x9c"
      "\x9e\x9e\x1c\xc9\xeb\x3d\x54\x1e\x40\x7c\xc2\xda\xe5\xe6\x90\xcd\x62\x8a"
      "\xb8\x48\x75\xf2\xc5\x0b\xa8\x91\xce\xa5\x99\xb0\x79\xb4\xb4\xba\x68\x6f"
      "\xcd\xf2\x40\x43\x0a\x53\x7a\x39\x5d\xc7\x3b\xda\x36\x7b\xf1\x2c\xb7\xd8"
      "\x16\x91\xa5\xfe\x8c\x47\xbe\x39\x56\x56\xa2\x97\xe9\xdf\x0e\x71\xf9\x67"
      "\x56\xea\x5c\xce\x7d\xaa\xc4\xbe\x29\x01\x59\xf6\xbc\xd7\x5f\x0d\xda\x9d"
      "\xe5\x6b\x2e\x71\xae\x9e\x48\xb0\xed\x12\x54\xa8\x31\x86\xc1\x52\x6a\xf6"
      "\xfb\xb8\x69\x60\x4d\x51\xa3\x6a\x54\xc8\x32\xe4\x5b\x25\x69\xdc\x0d\x90"
      "\xb0\x75\x22\x5f\xde\x44\xc4\xe0\x97\x31\x71\xad\x47\xd6\xb7\x0e\xbc\x66"
      "\x03\x09\xe1\xe2\x45\xb0\xfd\xf9\x74\x3a\xf9\x30\xcd\x6d\xb4\x9a\x47\x61"
      "\x38\x08\xba\xd9\x59\x71\x9c\x00\x00\x00\x00\x00\x37\x8a\x92\x1c\x7f\x7f"
      "\x84\x33\xc2\xe2\x4c\x7e\x80\x00\x03\xc9\xe8\x09\x5e\x02\x09\x5f\x28\xe6"
      "\x78\xf6\x64\x22\x43\x6f\x94\x9e\x2a\xb8\xf1\x62\xd7\xe3\xf8\x55\xe3\x78"
      "\xf4\xa1\xf4\x0b\x0c\x6f\xb2\xd4\xb2\x05\xaa\x00\xb6\xd7\x13\xac\xeb\xc5"
      "\xb0\x14\xe6\x1a\x54\x3a\x5a\x19\x4f\x9a\xc1\x8d\x76\xb5\x44\x0e\x3b\x1a"
      "\x56\x9e\xd7\xaa\x28\x73\x78\xc6\x97\xf6\xca\xfa\x86\x96\x6d\x7b\xa1\x9e"
      "\x72\x04\x13\x26\x7a\x6c\xce\xa9\xc4\x39\x67\x1d\x2c\x68\x0f\x27\x53\xca"
      "\x18\x4e\xee\xb8\x43\x45\x03\x68\xac\xb4\x38\x3a\x01\xd2\x5e\xb3\xd1\xe2"
      "\x3e\x0f\x26\x45\xd1\xcd\xfa\x9f\xa4\x10\x63\x2f\x95\xa5\xf6\x22\xf8\x51"
      "\xc6\x6e\xe7\xe3\x03\x93\xcd\x7a\x4d\x67\xff\x2a\x49\xc4\xf9\x36\x09\x84"
      "\xb5\xc2\xd4\x52\x34\x97\xe4\xd6\x4f\x95\xf0\x84\x93\x56\x4a\x1d\xf8\x71"
      "\x11\xc9\xbf\x31\x94\xfe\xf9\x7d\xce\xcc\x46\x7a\xce\x45\xfe\xeb\x68\x5c"
      "\x58\x70\xd0\x5f\x88\xa0\xf4\x63\xdb\x88\xd3\x77\x44\x2e\x13\x49\xac\xaf"
      "\x76\x62\x18\xb5\x4a\x9d\x62\x47\x78\xe1\xc4\xcb\xa6\xe6\x39\x0a\x9f\x30"
      "\x2c\x6e\xb2\xdf\x77\x66\x41\x1b\xef\x0e\xbb\x50\x02\x51\x2b\xcb\xf9\xb2"
      "\x4a\xcc\xfe\xcb\x0f\x47\x7d\xb1\x03\x46\x3a\xf2\x84\x7e\x6a\xde\x5b\x9e"
      "\x06\x5e\xc0\xd0\xba\x58\xfe\xda\xe5\xf0\x88\x18\xfe\xa4\x75\xb1\x69\x46"
      "\x9f\x9e\xfd\x13\x19\x25\xd9\x8c\x34\xb3\xcb\x26\xfe\x26\x79\x6d\xd4\x3b"
      "\x87\xe1\xcd\xee\x39\xf5\xcf\x21\xd2\xe8\x0a\x64\xac\x97\xe7\x1c\xaf\xc2"
      "\x9b\xfb\x78\xdb\x09\x0d\xd1\x22\x25\xef\xed\xa2\xe9\x3b\xf7\xf6\xba\x78"
      "\x65\xe9\xc3\x75\xa7\x80\x92\x9d\xfa\x5a\x21\x0b\xf5\x85\x8e\x2a\x4f\xf8"
      "\xe8\xd1\xe8\xc9\xcc\xee\xd0\x7c\x63\x12\xb7\x34\xc7\x25\x10\xd3\x35\xac"
      "\xc9\x4f\x76\xe7\x07\x8c\xe4\x06\x6f\x1e\x0a\xc9\x42\x9f\x80\x13\x68\x33"
      "\x01\x27\x7a\x11\xe2\x5b\x24\x8b\x61\x18\x0c\xb6\x20\x7a\x0e\x26\x75\x7f"
      "\x3f\x1b\xfc\x6c\x27\xf3\x72\x0d\x1f\xb7\x4a\xfb\x17\xf3\xb5\x23\x9b\xc2"
      "\x24\x38\x53\xd5\x23\x4a\xfb\x05\xed\x60\x24\xe9\x4d\xee\x34\x66\x6c\x5b"
      "\x55\x22\xb5\x4c\xb4\x33\xef\xa5\x4b\x4e\x90\x22\x53\x3e\x6a\x45\x98\xb4"
      "\x53\xc9\xe5\x49\x84\x7c\x89\xbd\xee\x95\x7d\xfa\x9d\xed\x9f\x16\xfd\xcd"
      "\x1b\x68\x1e\x4c\x72\xf3\x2f\xa3\x33\x43\x13\xe3\x34\xcc\x14\x0d\xae\xc7"
      "\xdc\xb2\x2f\x46\x34\x57\xa1\xa5\xac\x23\x0b\xbd\xed\x86\x25\x82\x06\x04"
      "\x6f\x55\x65\x89\xf5\x62\x7a\xb2\x75\x1e\xb3\x4d\x94\x05\x19\xf0\x09\x41"
      "\x20\x98\x39\x8a\xd8\x22\xa8\x50\x9d\x2d\x32\xda\x65\x6a\xc5\x93\x5e\x4c"
      "\x7f\x95\x03\xce\x4d\xaf\x13\x30\x21\xfa\x02\xa0\x00\x02\xcc\x67\x54\x4d"
      "\x3e\x86\x13\x92\x6f\xbe\x26\x47\xe2\x58\x93\x25\x92\xc9\x12\x3f\x1c\x74"
      "\x02\x31\x44\x44\x2d\x09\xef\x90\xc6\x49\x50\x17\x6c\x66\x6d\xa6\xb6\x58"
      "\x10\x8a\xc5\x4b\xeb\x23\x79\xaa\x70\x50\x1c\x42\xbd\x9c\x4e\x4f\x42\x6a"
      "\xb5\xff\x2a\x33\x76\x7b\x40\x8e\xd1\x9e\x39\x9c\x33\xb5\x2a\xbc\x4a\xc2"
      "\x4d\xa0\xd4\xcf\x07\xd9\x30\x29\x58\x35\x91\xc3\xb4\x23\x3f\x0f\x80\x51"
      "\x3c\x54\x15\x81\x97\x7f\xba\xcb\x9e\x69\xed\xb6\x6f\xdb\x27\x22\xeb\x4b"
      "\x0b\x67\x0b\xa7\x4f\xde\xe2\xf1\xa3\xcf\x85\x77\x7a\xf5\xd8\x26\x7a\xca"
      "\xa9\xd8\x3c\x23\xa4\xb4\x0d\x0f\x53\xdc\xe0\x03\xc0\x3c\x59\x59\xca\xab"
      "\xbe\xff\x7c\x7c\xad\xbd\xc5\x7d\xef\x7f\x5f\x75\x8a\xa4\x6b\x6e\x29\xaa"
      "\xbf\xa4\x16\x2f\xdd\xc7\x79\x77\x0d\x7e\x39\x5c\x4a\xc2\xa1\x36\xa3\x0e"
      "\x7c\x0a\x30\x1d\xff\xc5\x56\x5d\x72\x4c\xfb\x29\xcd\x30\x2c\x36\xe7\x62"
      "\x21\xa3\xc4\xe2\x15\x69\xbe\x89\xe8\x8b\xb7\xd1\x9d\x4f\xb2\x36\xea\x14"
      "\xf9\x99\x87\x76\xa0\xae\x88\xdf\xec\xcc\x9a\xd6\x4d\x2f\xe3\xcf\xde\xb9"
      "\x68\x4b\x7b\x7e\x5c\x00\xf7\x32\x3f\x82\x14\xed\x01\x89\x53\x9e\x0e\x3e"
      "\x34\x80\x8e\xe0\xa9\xe1\xc6\x2b\x52\x25\x20\x80\x96\xcf\x5a\x69\xcb\x91"
      "\x66\xda\x72\xb7\xc1\x16\xa2\x34\x89\xae\x54\xf1\xff\x37\x94\x2d\x4d\xf1"
      "\x06\x17\xc1\xbd\x08\xc5\x45\x35\x54\x2a\xa8\x63\x20\xc5\xd8\x8d\x25\x57"
      "\x34\xed\xed\x87\x82\xf8\x44\x5a\x18\x54\x0d\x70\x68\xb4\x16\xf8\x67\xe7"
      "\xf1\xf8\x85\x3b\x46\x36\x05\x7b\x9f\x39\x62\x99\xa8\x87\xd6\x64\x93\xd8"
      "\x1d\x9a\xac\x2d\x7c\xa1\x93\x67\x21\xc3\xfe\x04\x88\x2c\xe0\x50\x02\x5a"
      "\x6b\x8f\x33\x43\x95\xb6\xa2\xe3\x82\x19\x9a\x02\x31\xa4\xd8\x24\x45\xba"
      "\x1e\xa3\x79\xff\xcf\x54\xb9\x84\x86\x5f\x76\x1b\x55\x7c\xfa\x72\x35\x5a"
      "\x18\x25\x07\xeb\x06\x26\x8d\x69\x7a\x97\x4e\x19\x76\x2a\x46\xc2\x85\x18"
      "\xc0\x93\x26\xd8\xb4\xe3\x42\xcb\x0d\xc9\xb7\x29\x7b\x5f\x3d\xe2\x98\xf1"
      "\x8c\x1b\x09\xa8\x33\xad\x0d\x3c\xc6\x2a\xc3\x0b\xeb\xdf\x9e\xaa\x97\x91"
      "\x7c\xaf\xfa\x95\x3f\x2d\xff\x39\x2c\xb9\x71\xb8\x6a\x4d\xb3\x73\x57\x12"
      "\x54\x13\x45\xa4\xd1\x39\x8e\x2b\x0b\xe3\x48\x28\xf5\x86\x0c\x96\x66\x4b"
      "\x7a\x8b\xda\xc5\xab\xb3\xe8\xe8\xb9\x99\xf0\xd3\x42\x6e\x66\xa9\x57\x25"
      "\x85\x6c\xf8\x7b\xe8\x41\x10\x98\xb8\x0b\x64\x5d\xcb\x70\x21\x95\x64\x3e"
      "\xd3\xec\xe7\x1d\x9e\x2f\x2a\x32\x5a\xcf\xa4\x97\x82\x9d\xc5\x8d\xee\xdd"
      "\x5a",
      3439);
  *(uint64_t*)0x20000090 = 0x20000100;
  memcpy((void*)0x20000100, "GPL\000", 4);
  *(uint32_t*)0x20000098 = 0;
  *(uint32_t*)0x2000009c = 0;
  *(uint64_t*)0x200000a0 = 0;
  *(uint32_t*)0x200000a8 = 0;
  *(uint32_t*)0x200000ac = 0;
  memset((void*)0x200000b0, 0, 16);
  *(uint32_t*)0x200000c0 = 0;
  *(uint32_t*)0x200000c4 = 0;
  *(uint32_t*)0x200000c8 = -1;
  *(uint32_t*)0x200000cc = 8;
  *(uint64_t*)0x200000d0 = 0x20000000;
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x200000d8 = 0;
  *(uint32_t*)0x200000dc = 0x10;
  *(uint64_t*)0x200000e0 = 0x20000000;
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x2000000c = 0;
  *(uint32_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000ec = 0;
  *(uint32_t*)0x200000f0 = -1;
  res = syscall(__NR_bpf, 5ul, 0x20000080ul, 0x48ul);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000040 = 0x20000140;
  memcpy((void*)0x20000140, "net_dev_start_xmit\000", 19);
  *(uint32_t*)0x20000048 = r[0];
  syscall(__NR_bpf, 0x11ul, 0x20000040ul, 0x10ul);
  *(uint32_t*)0x20000080 = 3;
  *(uint32_t*)0x20000084 = 8;
  *(uint64_t*)0x20000088 = 0x20001b80;
  memcpy(
      (void*)0x20001b80,
      "\x62\x0a\xf8\xff\x0c\x20\x00\x21\xbf\xa1\x00\x00\x00\x00\x00\x00\x07\x01"
      "\x00\x00\xf8\xff\xff\xff\xb7\x02\x00\x00\x03\x00\x00\x00\xbd\x12\x00\x00"
      "\x00\x00\x00\x00\x85\x00\x00\x00\x06\x00\x00\x00\xb7\x00\x00\x00\x00\x00"
      "\x00\x00\x95\x00\x00\x00\x00\x00\x00\x00\x3f\xaf\x4f\x2a\xa3\xd9\xb1\x8e"
      "\xd8\x12\xa2\xe2\xc4\x9e\x80\x20\xa6\xf4\xe0\xe4\xa9\x44\x6c\xa2\xb5\xf1"
      "\xcc\x1a\x10\x0a\x9a\xf6\x98\x39\x3a\xa0\xf3\x88\x1f\x9c\x24\xaa\x56\xf1"
      "\x51\x99\xfa\xd0\x09\x3c\x59\xd6\x6b\x5e\xce\x9f\x36\xc7\x0d\x0f\x13\x90"
      "\x5e\xa2\x3c\x22\x62\x4c\x9f\x87\xf9\x79\x3f\x50\xbb\x54\x60\x40\x67\x7b"
      "\x0c\x50\x77\xda\x80\xfb\x98\x2c\x1e\x94\x00\xc6\x93\x14\x6c\xea\x48\x4a"
      "\x41\x5b\x76\x96\x61\x18\xb6\x4f\x75\x1a\x0f\x24\x1b\x07\x2e\x90\x08\x00"
      "\x08\x00\x2d\x75\x59\x3a\x28\x6c\xec\xc9\x3e\x64\xc2\x27\xc9\x5a\xa0\xb7"
      "\x84\x62\x57\x04\xf0\x7a\x72\xc2\x91\x84\x51\xeb\xdc\xf4\xce\xf8\x09\x60"
      "\x60\x56\xfe\x5c\x34\x66\x4c\x0a\xf9\x36\x0a\x1f\x7a\x5e\x6b\x60\x71\x30"
      "\xc8\x9f\x18\xc0\xc1\x08\x9d\x8b\x85\x88\x00\x00\xc2\x9c\x48\xb4\x5e\xf4"
      "\xad\xf6\x34\xbe\x76\x32\x89\xd0\x1a\xa2\x7a\xe8\xb0\x9e\x00\xe7\x9a\xb2"
      "\x0b\x0b\x8e\xd8\xfb\x7a\x68\xaf\x2a\xd0\x00\x00\x00\x00\x00\x00\x00\x6f"
      "\x80\x3c\x64\x68\x97\x20\x89\xb3\x02\xd7\xbf\xf8\xf0\x6f\xa1\x91\x8d\x65"
      "\xea\xe3\x91\xcb\x41\x33\x60\x23\xcd\xce\xdb\x5e\x01\x25\xeb\xbc\xeb\xdd"
      "\xcf\x10\xcb\x23\x64\x14\x92\x15\x10\x83\x33\x71\x9a\xcd\x97\xcf\xa1\x07"
      "\xd4\x02\x24\xed\xc5\x46\x5a\x93\x2b\x77\xe7\x4e\x80\x2a\x0d\x42\xbc\x60"
      "\x99\xad\x23\x00\x00\x00\x80\x00\x6e\xf6\xc1\xff\x09\x00\x00\x00\x00\x00"
      "\x00\x10\xc6\x3a\x94\x9e\x8b\x79\x55\x39\x4f\xfa\x82\xb8\xe9\x42\xc8\x91"
      "\x12\xf4\x0c\xfd\x7c\x3a\x1d\x37\xa6\xab\x87\xb1\x58\x66\x02\xd9\x85\x43"
      "\x0c\xea\x01\x62\xab\x3f\xcf\x45\x91\xc9\x26\xab\xfb\x07\x67\x19\x23\x7c"
      "\x8d\x0e\x60\xb0\xee\xa2\x44\x92\xa6\x60\x58\x3e\xec\xdb\xf5\xbc\xd3\xde"
      "\x3a\x83\x20\x9d\xa1\x7a\x0f\xaf\x60\xfd\x6a\xd9\xb9\x7a\xa5\xfa\x68\x48"
      "\x03\x66\xc9\xc6\xfd\x6f\xa5\x04\x3a\xa3\x92\x6b\x81\xe3\xb5\x9c\x9b\x08"
      "\x1d\x6a\x08\x00\x00\x00\xea\x2b\x1a\x52\x49\x6d\xfc\xaf\x99\x43\x14\x12"
      "\xfd\x13\x4a\x99\x63\x82\xa1\xa0\x4d\x5b\xb9\x24\xcf\xe5\xf3\x18\x54\x18"
      "\xd6\x05\x32\xaf\x9c\x4d\x2e\xc7\xc3\x2f\x20\x95\xe6\x3c\x80\xaf\xf9\xfa"
      "\x74\x0b\x5b\x76\x32\xf3\x20\x30\x91\x6f\x89\xc6\xda\xd7\x60\x3f\x2b\xa2"
      "\xa7\x90\xd6\x2d\x6f\xae\xc2\xfe\xd4\x4d\xa4\x92\x8b\x30\x14\x2b\xa1\x1d"
      "\xe6\xc5\xd5\x0b\x83\xba\xe6\x16\xb5\x05\x4d\x1e\x7c\x13\xb1\x35\x5d\x6f"
      "\x4a\x82\x45\xea\xa4\x99\x7d\xa9\xc7\x7a\xf4\xc0\xeb\x97\xfc\xa5\x85\xec"
      "\x6b\xf5\x83\x51\xd5\x99\xe9\xb6\x1e\x8c\xaa\xb9\xc7\x07\x64\xb0\xa8\xa7"
      "\x58\x3c\x90\xb3\x43\x3b\x80\x9b\xdb\x9f\xbd\x48\xbc\x87\x34\x95\xcb\xff"
      "\x8a\x32\x6e\xea\x31\xae\x4e\x0f\x75\x05\x7d\xf3\xc9\xd1\x33\x30\xca\x00"
      "\x6b\xce\x1a\x84\x52\x1f\x14\x51\x8c\x9b\x47\x6f\xcc\xbd\x6c\x71\x20\x16"
      "\x21\x98\x48\x62\x4b\x87\xce\xc2\xdb\xe9\x82\x23\xa0\xeb\x4f\xa3\x9f\x6b"
      "\x5c\x02\xe6\xd6\xd9\x07\x56\xff\x57\x90\x2a\x8f\x57\x01\x00\x00\x00\x97"
      "\x00\xcf\x0b\x4b\x8b\xc2\x29\x41\x33\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x8b\xc0\xd9"
      "\x55\xf2\xa8\x33\x66\xb9\x97\x11\xe6\xe8\x86\x1c\x46\x49\x5b\xa5\x85\xa4"
      "\xb2\xd0\x2e\xdc\x3e\x28\xdd\x27\x1c\x89\x62\x49\xed\x85\xb9\x80\x68\x0b"
      "\x6c\x4a\x00\x00\x00\x00\x2b\x43\x5a\xc1\x5f\xc0\x28\x8d\x9b\x2a\x16\x9c"
      "\xdc\xac\xc4\x13\xb4\x8d\xaf\xb7\xa2\xc8\xcb\x48\x2b\xac\x0a\xc5\x02\xd9"
      "\xba\x96\xff\xff\xff\x7f\x00\x00\x00\x00\xdf\x73\xbe\x83\xbb\x7d\x5a\xd8"
      "\x83\xef\x3b\x7c\xda\x42\x01\x3d\x53\x04\x6d\xa2\x1b\x40\x21\x6e\x14\xba"
      "\x2d\x6a\xf8\x65\x6b\xff\xf1\x7a\xdd\xae\xda\xb2\x5b\x30\x00\x2a\xbb\xba"
      "\x7f\xa7\x25\xf3\x84\x00\xbe\x7c\x1f\x00\x1b\x2c\xd3\x17\x90\x2f\x19\xe3"
      "\x85\xbe\x9e\x48\xdc\xcf\xf7\x29\x43\x32\x82\x83\x06\x89\xda\x6b\x53\xb2"
      "\x63\x33\x98\x63\x1c\x77\x71\x42\x9d\x12\x00\x00\x00\x33\x41\xbf\x4a\x00"
      "\xfc\xff\x5e\x1c\x7c\x3d\x1d\x6e\x3a\x52\x87\x2b\xae\xf9\x75\x3f\xff\xff"
      "\xff\xff\xff\xff\xe0\x9f\xec\x22\x71\xfe\x01\x58\x96\x46\xef\xd1\xcf\x87"
      "\x0c\xd7\xbb\x23\x66\xfd\xe4\xa5\x94\x29\x73\x8f\xcc\x91\x7a\x57\xf9\x4f"
      "\x6c\x45\x3c\xea\x79\x3c\xc5\xee\x0c\x2a\x5f\xf8\x70\xce\x5d\xfd\x34\x67"
      "\xde\xcb\x05\xcf\xd9\xfc\xb3\x2c\x8e\xd1\xdb\xd9\xd1\x0a\x64\xc1\x08\x28"
      "\x5e\x71\xb5\x56\x5b\x17\x68\xee\x58\x96\x9c\x41\x59\x52\x29\xdf\x17\xbc"
      "\xad\x70\xfb\x40\x21\x42\x8c\xe9\x70\x27\x5d\x5b\xc8\x95\x57\x78\x56\x7b"
      "\xc7\x9e\x13\xb7\x82\x49\x78\x8f\x11\xf7\x61\x03\x8b\x75\xd4\xfe\x32\xb5"
      "\x61\xd4\x6e\xa3\xab\xe0\xfa\x4d\x30\xdc\x94\xef\x24\x18\x75\xf3\xb4\xb6"
      "\xab\x79\x29\xa5\x7a\xff\xe7\xd7\xfa\x29\x82\x2a\xea\x68\xa6\x60\xe7\x17"
      "\xa0\x4b\xec\xff\x0f\x71\x91\x97\x72\x4f\x4f\xce\x10\x93\xb6\x2d\x7e\x8c"
      "\x71\x23\xd8\xec\xbb\xc5\x5b\xf4\x04\x57\x1b\xe5\x4c\x72\xd9\x78\xcf\x90"
      "\x6d\xf0\x04\x2e\x36\xac\xd3\x7d\x7f\x9e\x11\x9f\x2c\x06\xf8\x15\x31\x2e"
      "\x0c\x6d\xd0\x22\xc0\x74\xeb\x8a\x32\x2f\xb0\xbf\x47\xc0\xa8\xd1\x54\xb4"
      "\x05\xc3\x7f\xea\xf3\xdd\x95\xf6\xef\x44\xcd\x1f\xe5\x82\x78\x61\x05\xc7"
      "\xdf\x8b\xe5\x87\x70\x84\xd4\x17\x37\x31\xef\xe8\x95\xef\xc7\x1f\x66\x5c"
      "\x4d\x75\xcf\x24\x58\xe3\x5d\x2c\x90\x62\xec\xe8\x4c\x99\xa0\x61\x88\x7a"
      "\x20\x63\x9b\x41\xc8\xc1\x2e\xe8\x6c\x50\x80\x40\x42\xb3\xea\xc1\xf8\x79"
      "\xb1\x36\x34\x5c\xf6\x7c\xa3\xfb\x5a\xac\x51\x8a\x75\xf9\xe7\xd7\x10\x1d"
      "\x5e\x18\x6c\x48\x9b\x3a\x06\xfb\x99\xe0\xaa\x7f\x23\xa0\x54\xde\x2f\x4d"
      "\x92\xd6\xbd\x72\xee\x2c\x9f\xdc\x75\xaa\xaf\x1e\x3e\x48\x3b\x4a\xd0\x55"
      "\x73\xaf\x40\x32\x69\xb4\xa3\x9c\xe4\x02\x93\x94\x7d\x9a\x63\x1b\xcb\xf3"
      "\x58\x37\x84\xac\xbd\xa2\x16\x55\x0d\x7a\xec\x6b\x79\xe3\x0c\xbd\x12\x8f"
      "\x54\xc2\xd3\x33\x54\x57\xac\x0e\xab\xa9\x9b\xf0\xbd\xc1\x4a\xe3\x58\xc3"
      "\xb3\x77\x32\x7a\xc9\xec\xc3\x4f\x24\xc9\xae\x15\x3e\xc6\x0a\xc0\x69\x4d"
      "\xa8\x5b\xff\x9f\x5f\x4d\xf9\xb3\xff\x37\x2b\x5f\x2e\xe1\x0c\xeb\xb2\xc5"
      "\xea\x13\x93\xfd\xf2\x42\x85\xbf\x16\xb9\x9c\x9c\xc0\xad\x18\x57\x21\x6f"
      "\x1a\x98\x5f\x36\x91\x91\xae\x95\x4f\xeb\xb3\xdf\x46\x4b\xfe\x0f\x77\x3e"
      "\xe9\xaf\xe7\x2f\x32\xa2\xbe\xfb\x89\xd3\x77\x73\x99\xf5\x87\x4c\x55\x3a"
      "\xeb\x37\x29\xcf\xfe\x86\xe6\x69\x64\xae\x09\xbb\x6d\x16\x31\x18\xe4\xcb"
      "\xe0\x24\xfd\x45\x22\x77\xc3\x88\x7d\x61\x16\xc6\xcc\x9d\x80\x46\xc2\x16"
      "\xc1\xf8\x95\x77\x8c\xb2\x6e\x22\xa2\xa9\x98\xde\x44\xae\xad\xea\x2a\x40"
      "\xda\x8d\xac\xcf\x08\x08\x42\xa4\x86\x72\x17\x37\x39\x0c\xbf\x3a\x74\xcb"
      "\x20\x03\x01\x6f\x1d\x14\x21\x6b\xdf\x57\xd2\xa4\x0d\x40\xb5\x1a\xb6\x3e"
      "\x96\x00\x04\x00\x00\xb8\xa8\xc9\xae\x3d\x14\xf9\x31\x00\xc2\xe0\x89\x38"
      "\x62\xee\xf5\x52\xfc\xde\x29\x81\xf4\x8c\x48\x2b\xde\x6e\x4a\x43\x04\xe5"
      "\x0c\x34\x9f\x4f\x9e\xce\xe2\x7d\xef\xc9\x38\x71\xc5\xf9\x9b\x35\x5b\x72"
      "\xd5\x38\xba\x49\x58\xea\x8e\x4a\xa3\x70\x94\x19\x1e\x10\x09\x6e\x7e\x60"
      "\xfc\x35\x41\xa2\xc9\x05\xa1\xa9\x5e\x95\x71\xbf\x38\xae\x19\x81\xc4\x23"
      "\x8e\xca\xfe\x6f\x75\xcd\x0a\x68\x81\xbd\x15\x17\xa8\x25\x0d\xf9\x86\x74"
      "\x15\x2f\x94\xe3\x24\x09\xe2\xa3\xbc\xe1\x09\xb6\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\xd6\xd5\x21\x0d\x75\x60\xeb\x92\xd6\xa9\x7a\x27\x60\x2b\x81"
      "\xf7\x63\x6d\xf1\x53\x5b\xef\x14\x97\xf9\x21\x86\x08\x6e\x29\xc6\xbc\x5a"
      "\x1f\xad\x6e\xc9\xa3\x11\x37\xab\xf9\xa4\x04\xab\xde\x77\x50\x89\x02\x00"
      "\xd6\x27\xe8\x73\x06\x70\x3b\xe8\x67\x2d\x70\xd1\xab\x57\x07\x52\x28\xa9"
      "\xf4\x6e\xd9\xbd\x1f\x08\xfb\x81\x91\xbb\xab\x2d\xc5\x1d\xe3\xa6\x1f\x08"
      "\x68\xaf\xc4\x29\x48\x59\x32\x3e\x7a\x45\x31\x9f\x18\x10\x12\x88\xa0\x26"
      "\x88\x93\x37\x37\x50\xd1\xa8\xfe\x64\x68\x0b\x0a\x3f\xc2\x2d\xd7\x04\xe4"
      "\x21\x4d\xe5\x94\x69\x12\xd6\xc9\x8c\xd1\xa9\xfb\xe1\xe7\xd5\x8c\x08\xac"
      "\xaf\x30\x23\x5b\x92\x05\x00\xd2\xec\xa5\x5f\x74\xa2\x36\x41\xf6\x1f\x2d"
      "\x5b\x30\x8c\xf0\xd0\x31\xb0\xc7\xf0\xce\xd6\x99\x93\xe9\x96\x0f\xf5\xf7"
      "\x60\x15\xe6\x00\x26\x3d\x97\x56\x23\x7b\xad\xf4\xe7\x96\x5b\xbe\x27\x77"
      "\xe8\x08\xfc\xba\x82\x1a\xa8\xe8\xc5\xc3\x96\x09\xff\x85\x43\x56\xcb\x49"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xc1\xfe\xe3\x0a\x3f"
      "\x7a\x85\xd1\xb2\x9e\x58\xc7\x76\x85\xef\xc0\xce\xb1\xc8\xe5\x72\x9c\x66"
      "\x41\x8d\x16\x9f\xc0\x3a\xa1\x88\x54\x6b\x3a\xd2\xa1\x82\x06\x8e\x1e\x3a"
      "\x0e\x25\x05\xbc\x7f\x41\x01\x96\x45\x46\x6a\xc9\x6e\x0d\x4b\x3b\xc1\x9f"
      "\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\xf0\x67\xbb\xab\x40\x74\x3b"
      "\x2a\x42\x8f\x1d\xa1\xf6\x8d\xf7\x5c\xf4\x3f\x8e\xcc\x8d\x37\x26\x60\x21"
      "\x11\xb4\x0e\x76\x1f\xd2\x10\x81\x92\x03\x82\xf1\x4d\x12\xca\x3c\x34\x31"
      "\xee\x97\x47\x1c\x78\x1d\x0d\x12\x80\xfb\x00\x81\x86\x54\xa5\x3b\x6d\xf4"
      "\xb2\xc9\x7c\xc1\xc9\x8d\x7a\xfd\xa8\xf8\x0f\xe9\x08\xb6\x55\x50\xb4\x41"
      "\x23\x31\x51\x12\x2b\x41\xa8\xd7\x30\x62\x19\x76\x55\xb7\xf0\x46\x92\x50"
      "\xa5\x98\x9c\xef\x0e\x10\x77\x39\x20\xed\x3c\xce\xe4\x47\x2c\x3e\xb8\x01"
      "\x59\xda\xed\x09\x25\x11\xe6\xeb\x93\x84\x20\x54\xcf\xce\x2a\xc3\x06\xcb"
      "\x6e\x47\x2d\xb3\xfd\x67\xa4\x9b\x68\x55\xa6\x94\xa8\xd3\x5b\xad\xd4\x39"
      "\x07\x00\x32\x23\xa4\x7a\x7f\xae\x4f\x37\x48\xd5\xa4\x32\x82\x5b\xc4\x0a"
      "\x03\xaa\xef\x1c\x84\x88\xd8\x6d\xc2\x01\xdd\x2a\x3b\xa7\x1e\x0f\x45\x49"
      "\x2e\xf1\xf8\xb6\x5c\xcb\x3d\xcd\x0e\x1a\x61\xb1\x52\xd0\x2c\x29\xca\x0a"
      "\x33\x28\xfa\x77\x53\xa5\xcd\xde\xa1\xac\xaa\xe5\x5a\xe8\x26\x3f\xb2\x84"
      "\xb7\xa6\x49\x8c\x4b\x4f\xbd\x0e\xea\x32\x25\xa5\x30\x72\x42\x3b\x90\x7c"
      "\x66\x02\xf8\x99\x9e\x03\x11\xda\x5b\x83\x78\xbc\x84\x1e\x17\x87\xe3\xa8"
      "\x12\x8d\xda\x38\x1a\x26\xcb\x2b\x36\x57\x02\xff\x8a\x27\x83\x13\x75\xb2"
      "\xdd\xaa\x2f\x56\xe2\x11\x69\xf7\xca\x4f\xd9\x65\x5c\xcd\x4a\x58\x4a\xcd"
      "\x24\x4e\x96\x6d\x0a\xfe\xda\xff\x7c\x41\x5f\xf6\x82\xa4\x04\x4b\x33\x81"
      "\xcc\x2d\xf2\x82\x78\xc9\xa6\x82\x4c\x52\x04\x8a\x7c\xfa\xbd\xa2\x94\x92"
      "\x5c\xc0\x95\x6b\xb5\xa8\xe9\x50\xff\x61\x49\xf4\x1a\xe6\xed\xd8\x30\x20"
      "\x7b\xf7\x28\xcd\x98\x07\x93\x3c\x3e\x16\xd8\x0b\xbe\xa6\x11\xa1\x8b\xec"
      "\xc2\xdc\x38\xca\x0a\x6f\x57\x40\xf3\x40\xb7\x6e\xdc\xd1\xf5\x39\xbd\x43"
      "\x00\x72\x31\xdc\xef\x58\xc7\xb8\x8b\x5a\xee\xda\xab\x37\xd4\xf9\x8a\x93"
      "\x4b\x0f\x90\x0e\x0e\xb6\x39\x87\x8a\x45\xe4\x62\x9f\x55\x03\xcf\x67\x91"
      "\x54\xd2\x76\x81\xd7\xa3\x74\x4c\xbc\xd4\x2a\xf5\x94\x07\xc9\xc8\xe3\x9c"
      "\x52\x71\x86\x89\x17\x95\x4e\x60\x03\x52\xba\x26\x17\x1d\x00\x4f\x1c\x55"
      "\x24\xf0\x61\x37\x8f\x94\xfb\x45\x37\x86\xc3\xa6\xf7\x8b\x10\xd3\x83\xb4"
      "\x31\xd1\x56\x8b\xd4\x3e\xe3\x4c\xe6\xe6\xbe\x23\x5a\xa6\x20\x72\x85\x66"
      "\x5c\x2f\xba\x77\x36\x71\xda\x41\x95\x9f\x51\x61\x09\x63\xb4\x89\x30\x65"
      "\x8e\x2d\x61\x25\xa2\x60\x85\xb1\x13\x45\xb0\x47\x32\x40\xb7\xe5\xe9\x18"
      "\x11\x31\x2c\x43\x66\xfa\xf0\xf7\x11\xd7\x21\x9e\xcd\xec\x75\xc7\xea\x1c"
      "\xf0\xf8\xf8\xff\xf4\x02\x47\xd5\x9b\xbd\xe2\xeb\xb8\x65\x91\x97\xe0\xf3"
      "\x7a\x71\xbe\x1b\x12\xa1\x82\xed\x7d\xe3\xac\xba\x28\x56\x1a\x04\xb8\x07"
      "\xf7\xa4\x64\x7e\x2e\xa6\xd8\xfb\x92\x54\x1d\x07\xc3\xd5\xe4\xba\x07\x7d"
      "\x3c\xad\x9f\x8b\xa1\x91\x95\x92\x01\x4c\x00\xc8\xec\xcb\x2c\xa5\xd4\x8b"
      "\xa7\xb1\xc3\xfb\x18\x5a\x4b\xb7\x97\x00\xcf\x51\xf8\x18\xb0\xc7\x01\xc8"
      "\xde\x47\xd1\x22\x81\xa6\x7b\xda\xf4\xb0\xc5\x0b\xee\x9e\x8f\x59\x36\x25"
      "\x0d\xf2\xe1\x58\x11\x7a\xe7\xea\x66\x19\xf7\xdb\x33\x04\x47\xd1\xe9\xe4"
      "\x2a\x03\x5e\x6f\xd5\x32\xf6\x1f\xbf\xed\x9c\x4a\x71\x24\xa1\xe3\x8e\xee"
      "\x50\xc6\xbb\xcd\x1d\x4e\x3f\x68\xc3\xf2\x7d\xd9\xa7\x0f\x1a\x7c\x60\x46"
      "\x23\x7d\xdf\xb0\xb2\x6e\x19\x73\x22\x22\x63\x60\x34\x98\x01\x04\x58\xcd"
      "\x4d\xf1\x0a\xf2\x49\xce\x71\x7f\x6f\x45\xe5\x17\x6e\x0d\xda\xe3\x05\x4d"
      "\x72\x89\xd4\xe1\x3a\xb0\x91\x27\x03\xee\x39\xce\x26\x45\x72\xb8\x91\x94"
      "\xfd\xf7\xac\xec\xc3\x5c\xf8\x30\x9d\x4b\x68\x0a\x08\xee\xd3\x67\xda\xd8"
      "\x55\xfc\xe2\x10\xf1\xa7\xc7\x22\x2d\xd3\x60\xea\xfb\x4b\xef\x7d\x58\xbf"
      "\x83\x36\x29\x30\xaf\x6e\x3f\x3f\x85\x1a\xbd\xc0\x00\x3b\xdf\x94\x01\xb5"
      "\x33\x01\x9e\x90\xfe\xb0\x6b\x18\x91\x00\x00\x7a\x82\xdf\x8d\x9b\x5f\x44"
      "\xeb\xf9\x35\x5e\x7b\x1b\x01\xc9\x47\x06\x08\xd4\xf3\x06\xd2\x10\x04\x73"
      "\x03\x96\xa4\xd6\xc6\xd4\x6e\x1f\xfa\xc9\x7a\xa9\x3c\x36\x12\x35\x32\xa3"
      "\x61\x86\x57\x52\x66\xbe\x49\x81\xc8\x47\x16\x00\x79\x42\x1d\x01\x37\x80"
      "\x1e\x55\x30\x69\xf8\xd0\x25\xc4\x0f\x28\x73\x78\x81\x0d\xef\xc7\xf2\xed"
      "\x4e\x15\xf6\x8f\x17\xb2\x11\x53\x39\x4f\x8b\xcf\xa6\xa2\x3a\x77\xc8\xd6"
      "\x1c\x9b\xbc\x12\x7a\x57\xb8\xd6\x31\xf3\x65\x58\xd9\x09\x3d\xee\x08\xbc"
      "\x53\xd9\x7a\x80\x03\x36\x34\x21\x73\x86\x50\xa2\x6c\x8f\xd8\x7b\x13\x02"
      "\x67\x99\xca\xf5\x8e\x59\x95\x1b\x12\x5e\x7f\x16\x29\xa3\x4e\x2c\x0d\xd6"
      "\x5a\x23\xd0\x1a\x3c\xb1\x91\xe7\x43\xde\x07\x24\x7c\x7f\x99\x3c\xf0\x11"
      "\x66\xfa\x2a\xc1\xba\x02\xf6\x05\x50\xe6\x3a\x7f\x50\x42\x2e\x47\x8c\x6b"
      "\x5d\x87\xf9\xbd\x05\xcd\xa7\x0c\x08\x36\x7e\x5c\x1b\x08\x67\xa2\x79\xa9"
      "\xd8\x5a\x38\x0d\xb2\x5c\x43\xbd\x05\x29\xad\x78\x3b\x9d\x64\xaa\xac\x5c"
      "\xda\xc2\x4b\x0c\x23\xc3\xd2\x67\x1b\x79\x3a\xfb\x44\xb7\x12\x6e\x17\xc2"
      "\xb7\xc0\xd6\xbe\x65\x0d\xe7\xee\xef\xbf\x36\x05\xaf\x34\x40\x15\xd0\x3c"
      "\x3e\x78\x19\x14\x5c\xb9\xfe\x19\x78\xc9\x8b\xf9\xef\x10\x77\x3d\xb5\x95"
      "\x05\xae\x33\x4d\x41\x13\xdb\x55\x70\x8c\x72\x88\x44\xc8\x72\xdf\x6e\x77"
      "\x48\xc1\xf9\xd2\xcb\x0b\x29\x75\x4f\x92\x8c\x59\x30\x6c\xe1\x05\xca\x18"
      "\xcb\x72\xf0\x94\x4d\x0e\x4f\xea\x0a\x0a\xbd\x02\x85\xbd\xaf\x1b\x41\x52"
      "\x0a\x10\x89\xd6\x40\xc2\xfa\xcb\x0d\x1e\x62\x43\x87\x3a\xc4\xb1\xe1\x06"
      "\x8c\x45\xc7\x15\xb6\x8e\xff\xb7\xd5\x8d\x1f\x9e\x72\x6d\xbf\x6b\xd9\x10"
      "\xca\x4c\xe0\xe0\x75\x65\x8e\xde\x42\x19\x2c\xf3\x93\xa5\x0d\xcc\x19\x7b"
      "\x03\x3a\x2f\xed\x75\x08\x36\x28\xe5\xdd\x38\x21\x3d\x35\x3b\x90\x49\xe7"
      "\x1f\x03\x70\x64\xb0\x5e\x73\xec\x00\xc7\x10\xf1\x4e\xc5\x73\x7d\x39\x7d"
      "\x55\x5d\x1c\xf8\x85\x9c\xc0\x5f\xea\x8d\xc3\xc6\xa5\xb3\xb6\xfa\x1c\x81"
      "\x70\x74\x79\xdb\x18",
      3245);
  *(uint64_t*)0x20000090 = 0x20000100;
  memcpy((void*)0x20000100, "GPL\000", 4);
  *(uint32_t*)0x20000098 = 0;
  *(uint32_t*)0x2000009c = 0;
  *(uint64_t*)0x200000a0 = 0;
  *(uint32_t*)0x200000a8 = 0;
  *(uint32_t*)0x200000ac = 0;
  memset((void*)0x200000b0, 0, 16);
  *(uint32_t*)0x200000c0 = 0;
  *(uint32_t*)0x200000c4 = 0;
  *(uint32_t*)0x200000c8 = -1;
  *(uint32_t*)0x200000cc = 8;
  *(uint64_t*)0x200000d0 = 0x20000000;
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x200000d8 = 0;
  *(uint32_t*)0x200000dc = 0x10;
  *(uint64_t*)0x200000e0 = 0x20000000;
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x2000000c = 0;
  *(uint32_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000ec = 0;
  *(uint32_t*)0x200000f0 = -1;
  res = syscall(__NR_bpf, 5ul, 0x20000080ul, 0x48ul);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x20000180 = r[1];
  *(uint32_t*)0x20000184 = 0xb3;
  *(uint32_t*)0x20000188 = 0xe;
  *(uint32_t*)0x2000018c = 0;
  *(uint64_t*)0x20000190 = 0x20000280;
  memcpy((void*)0x20000280,
         "\x5f\x39\x86\x8d\xe2\x63\x9b\x12\xc1\xa2\xff\x01\x00\x00", 14);
  *(uint64_t*)0x20000198 = 0;
  *(uint32_t*)0x200001a0 = 0x40002d7;
  *(uint32_t*)0x200001a4 = 0;
  *(uint32_t*)0x200001a8 = 0;
  *(uint32_t*)0x200001ac = 0;
  *(uint64_t*)0x200001b0 = 0;
  *(uint64_t*)0x200001b8 = 0;
  *(uint32_t*)0x200001c0 = 0;
  *(uint32_t*)0x200001c4 = 0;
  syscall(__NR_bpf, 0xaul, 0x20000180ul, 0x48ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  loop();
  return 0;
}
