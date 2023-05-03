// https://syzkaller.appspot.com/bug?id=e2e9d44579c04a51b263ce34113b2d6aae10b4c0
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

const int kFailStatus = 67;
const int kRetryStatus = 69;

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}

__attribute__((noreturn)) static void fail(const char* msg, ...)
{
  int e = errno;
  fflush(stdout);
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit((e == ENOMEM || e == EAGAIN) ? kRetryStatus : kFailStatus);
}

__attribute__((noreturn)) static void exitf(const char* msg, ...)
{
  int e = errno;
  fflush(stdout);
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit(kRetryStatus);
}

static uint64_t current_time_ms()
{
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    fail("clock_gettime failed");
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void test();

void loop()
{
  int iter;
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      fail("clone failed");
    if (pid == 0) {
      prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
      setpgrp();
      test();
      doexit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      int res = waitpid(-1, &status, __WALL | WNOHANG);
      if (res == pid)
        break;
      usleep(1000);
      if (current_time_ms() - start > 5 * 1000) {
        kill(-pid, SIGKILL);
        kill(pid, SIGKILL);
        while (waitpid(-1, &status, __WALL) != pid) {
        }
        break;
      }
    }
  }
}

long r[14];
void test()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                 r[0], 0x0ul);
  r[1] = syscall(__NR_inotify_init1, 0x0ul);
  *(uint32_t*)0x20c1fffc = (uint32_t)0xc;
  r[3] = syscall(__NR_getsockopt, r[0], 0x1ul, 0x11ul, 0x20049ff4ul,
                 0x20c1fffcul);
  if (r[3] != -1)
    r[4] = *(uint32_t*)0x20049ff4;
  r[5] = syscall(__NR_fcntl, r[1], 0x8ul, r[4]);
  r[6] = syscall(__NR_fcntl, r[1], 0x10ul, 0x2000d000ul);
  if (r[6] != -1)
    r[7] = *(uint32_t*)0x2000d004;
  r[8] = syscall(__NR_ptrace, 0x4206ul, r[7], 0x0ul, 0x20ul);
  r[9] = syscall(__NR_ptrace, 0x4207ul, r[7]);
  *(uint64_t*)0x20000000 = (uint64_t)0x2000c000;
  *(uint64_t*)0x20000008 = (uint64_t)0x1000;
  memcpy(
      (void*)0x2000c000,
      "\x83\xed\xd7\x97\xa6\x18\xb1\x03\xf7\x18\x7c\xec\x41\xec\x67\x87"
      "\x43\x9d\x78\x95\x84\x45\x81\x04\xc7\x4c\xa8\xe8\x62\xaa\x78\x36"
      "\x8b\xcc\x22\x8c\x7a\xfa\x18\xf2\xac\x85\x41\xad\x2b\xbc\x58\x32"
      "\xca\xc0\x54\xfa\x2d\x4a\x25\x68\x7f\x3c\x6e\x5f\x5e\xb1\x68\xa6"
      "\x96\x02\xe6\xd4\xdc\x29\xd0\x17\x57\x26\x33\x87\xaf\x8b\x04\xbd"
      "\xc3\x58\xdb\x89\x92\xc1\x47\x05\x37\x4d\x69\x98\x73\xad\xb4\x4c"
      "\x93\x77\x37\x71\x33\x24\x6b\x51\xce\x09\x8f\x9c\x04\x00\x29\x51"
      "\x2d\x46\x71\x00\x10\x94\xaf\x37\x8b\xa6\x19\x14\xc9\x74\x5d\x1f"
      "\x06\xe1\x4d\xfe\x76\x8a\x75\x1c\xaf\xb9\xbf\x32\xac\x53\x6a\xab"
      "\x28\x0f\x69\x6e\xae\xe2\x04\xd1\x0f\x8d\x1f\xc6\x52\xb0\x91\xef"
      "\x8f\x8b\xdd\xf4\x95\xb7\xa0\x8f\x2c\x42\xae\xc1\x5d\x09\xc9\xee"
      "\xec\xf0\xa3\x02\xc2\x1e\x88\x55\x82\x2b\x15\x69\xe3\x04\x9b\x1b"
      "\x98\x0d\x2f\x51\xd1\xb0\x96\x53\x9c\x8a\x7f\xaa\xcd\x36\x30\x05"
      "\xbe\xb3\xbe\xf2\x4f\xd9\x0a\xd0\x19\xf4\x6d\x29\x63\xa0\x6d\x72"
      "\xca\x69\x58\x02\x1e\x91\xea\xe6\x38\xf4\xc2\xe6\x3c\xc7\x3c\x41"
      "\xd6\xe0\x69\x46\x87\xa0\x4a\xa6\xd3\x90\x9e\x99\x43\xc3\x91\xd4"
      "\x59\x54\x36\x83\x75\x16\x8a\x96\xda\x24\xbc\x78\xc9\x22\xb3\x06"
      "\x6d\x01\x00\x04\x52\x09\xe0\x09\xfa\x5d\x9f\x6a\xfc\xd3\x13\xe6"
      "\xc4\x8f\xfb\x9c\x97\x6d\x2d\x96\x99\xfd\x9a\x80\xc8\x2c\x47\x6b"
      "\xc9\xc8\x04\xba\x9c\xed\x51\x0e\x45\x81\xf0\xdc\xcd\xd8\x09\xfb"
      "\xd2\x75\x3a\xf9\xe9\x6c\xc2\x4a\x4e\xa1\xe7\x12\xd4\x19\xdf\x37"
      "\x44\x9a\xed\x13\x7b\x5f\xc2\xc3\x14\x9b\xad\x0d\xe4\x1f\x65\x7e"
      "\xae\x7d\xcb\x75\x95\x43\x9a\xcc\xd9\x10\x83\x94\xb6\x79\x37\x89"
      "\x4a\x03\xdf\x43\xfa\x84\xa3\x23\x4b\xdc\xe0\x50\x4f\x80\xd8\xb8"
      "\x43\xac\x0e\x4b\xed\x94\xf6\x3c\xbf\xba\xd4\xd4\x2f\x8d\xb8\xca"
      "\xfb\x04\xb2\xc2\xfb\x8b\x5f\x40\xe3\x77\xa8\xde\x66\x74\x47\x5d"
      "\x5f\x41\xd4\xec\x8e\x28\x36\xdd\x09\x4d\x61\xdb\x37\xaf\x78\xce"
      "\x4c\x8c\x08\x44\x4c\xda\x69\x3c\x3c\xe0\x71\xe1\x07\x0e\x78\x04"
      "\xbc\xa2\x43\xe2\xea\x04\xe0\xe9\xcd\x92\x28\x16\x05\xab\x7d\xab"
      "\x5f\xa7\x1d\xf2\xf5\x10\x92\x01\x12\x9c\x80\xe6\x90\x4e\x61\xec"
      "\x86\x92\xe9\x49\x12\xb7\x71\x1f\x1a\xec\xb0\x0d\x6f\x9a\x97\xb5"
      "\x42\x94\x6d\x75\x51\x5d\xcd\x71\x4e\x5e\x6f\x28\x1f\xf5\xf8\xb1"
      "\x95\x3c\xdc\xbc\x20\x63\xd9\x20\x18\xa1\xe5\x08\x52\x46\x76\x00"
      "\xfc\x85\x4f\x57\x44\x0e\x39\x15\x10\x8b\xc4\xb0\x49\xf6\x45\xa1"
      "\xed\x2c\x93\x84\xdd\xeb\x56\x4b\xae\x50\x91\x6f\x73\x6f\x53\xa3"
      "\x57\x9d\x41\x54\xe8\x40\xd8\xbb\x6d\x14\x23\xda\xb4\xaa\x82\xd1"
      "\xc2\xb3\x3f\x08\x76\x5d\x9b\xfd\xd9\x1a\xaa\x9a\x1d\x4b\x2f\xcc"
      "\xe5\xdf\xf8\xed\x4b\x0c\xa7\x45\xdc\xc9\x7f\x6d\x6a\xd3\x5e\x63"
      "\x25\x1c\x32\x93\x2b\x2c\x62\xba\xdb\x90\x88\xef\xfa\x4b\xa6\xa9"
      "\x1b\x27\x12\x50\xbb\x16\x47\x62\x19\x82\xf9\xe5\x3e\x2e\xaf\xc2"
      "\x56\x6c\x8f\x59\x3e\x12\x77\x50\xa8\x8d\x89\x0b\x0c\xa7\xb3\xa3"
      "\x79\x98\xa7\x3c\x96\x13\xfd\x75\x79\x2e\x1a\x33\x85\x98\x34\x4f"
      "\xcc\xe1\xd9\x8f\xeb\xcb\xa2\xb6\x66\x17\xff\x57\x6a\x6a\x32\x0a"
      "\xb5\x3a\xca\x66\xad\xd5\x2d\x6d\x2a\x73\xe5\xd4\x89\xec\xfc\x0c"
      "\x46\x84\xfd\x3b\x69\xb8\x2c\x37\x46\xcc\x7c\x75\xe2\x30\x60\xa5"
      "\x4a\x2e\xa2\x53\x77\x9a\x3c\x6c\x90\xe6\xa7\xf2\x8a\x63\xcb\xff"
      "\x8c\xac\xa5\x93\x7c\x6b\x2c\x42\xbb\xd2\xcf\x2d\x39\xfa\xf9\x89"
      "\xfc\xf0\x01\x98\x2b\x17\x89\x80\xac\x39\xb3\x03\xbe\x01\x8a\x27"
      "\x40\x20\x89\x03\x2d\xf1\xec\xf9\xb4\xfc\x20\xcd\x31\xe9\xe3\xe4"
      "\xa2\x38\x3f\x9e\xdb\xa9\xb8\x5b\x4b\x2e\x0b\xc8\x0c\xee\x62\x0e"
      "\x0d\x2b\xd7\x20\x07\xda\x61\xd6\xa0\x90\x35\x92\xe7\x79\xfc\x03"
      "\x55\xab\x1e\xe3\x5d\xe8\xbc\x6f\x40\x1b\x7a\xf4\x10\xe0\xe1\x1e"
      "\x0d\xaf\x9f\x60\xe6\xd2\x26\x11\xaf\x2f\xfa\x5f\xa2\xf0\xeb\x58"
      "\xb1\x2c\xf9\xc6\xa9\xc1\x04\x6f\x8e\x77\xb3\xee\x5e\xaf\x1f\x88"
      "\x21\x1b\x62\x6d\x04\xd6\xa2\x82\x69\x99\x42\x48\x8f\x1d\xda\xcb"
      "\x65\x25\x20\x85\x9d\x0c\x36\x5d\x17\x7a\xfb\x84\xc2\x0d\x31\x6a"
      "\x38\x16\xc3\xa9\xbd\xe8\xad\xe1\x60\xb3\x02\xc1\x12\x86\x9a\x79"
      "\x8b\x5b\x27\x2c\x47\x8f\x21\xbc\xd1\x99\x6a\x57\xe4\x16\xa3\x85"
      "\x18\xb4\x91\x90\xa9\x58\x5d\x4b\x16\x60\x41\xee\x4d\x65\xd0\x79"
      "\xe9\xb9\xf5\x54\xee\xae\x8e\xb6\x52\x14\x24\xe3\x77\xcf\xe9\xe1"
      "\x87\x81\xb0\x5f\xaa\x48\xa2\x07\x7e\xae\x08\x9b\x4d\xc0\x76\x46"
      "\x0f\x26\xc7\xf8\xc0\x0d\xdb\x77\x14\x26\x1c\xcb\xf6\xbb\xe9\xa3"
      "\x51\x59\x7a\x90\xe7\x6d\xd6\xfb\x27\x65\xba\xd6\x7f\x43\x2d\xc6"
      "\x04\x3f\x6f\xa0\x1f\xdb\xab\x10\xb9\x63\xb1\x35\xa1\x80\x57\x58"
      "\xcb\xd0\xdc\x8f\xc0\x61\xf4\x93\xcd\x0d\x15\xea\xb3\x37\xd0\xd6"
      "\xb3\x48\x65\xd5\xdd\x97\x2e\x29\xda\x25\xde\x28\xdb\xa9\xed\xa0"
      "\x86\xe5\x1b\x29\xa3\xbb\x92\x86\xee\x74\x45\x11\x74\x89\x99\xc2"
      "\xb5\x2b\x1d\xcd\x30\x9a\xdb\x74\x72\x20\x76\xc6\xe1\x48\x66\x82"
      "\x96\x3e\xf4\x31\x9b\xce\x07\xd7\x5b\xf0\x29\xea\x8f\xa8\x95\x24"
      "\xb2\x7b\x25\xf1\xbe\xb4\x44\x75\x70\x1f\x57\x1a\x54\x9a\xa2\x81"
      "\x95\x8d\xe8\x1a\x29\x82\xe1\x1c\x11\x73\x7f\x06\x35\xd4\xaf\x1c"
      "\x0e\x70\x73\xa6\x7b\x3d\xeb\xb3\x0d\x2e\x49\x8b\x3f\xd4\xbf\xc8"
      "\xf8\xf9\xa9\xfe\x71\xf3\x57\xe5\xae\xa8\xea\x6e\xf4\x15\xb1\x88"
      "\x07\x7a\x6b\xac\xea\x96\xa3\x1a\xf5\x83\x8e\xb7\x6b\x94\x58\x5d"
      "\x2c\xcb\x63\x9e\x89\xe3\x8d\xfa\xd6\x7c\x33\x87\xa2\x13\xe3\x88"
      "\x64\x11\xa1\xdd\x9c\x51\x9e\x3a\x15\x96\xcd\xab\xc2\xab\xc9\xf6"
      "\x95\x1c\x4d\x21\x81\x51\x68\x69\x47\xda\x03\x1e\x2f\x1d\x78\x04"
      "\x08\x9a\xd2\x6c\x76\xf2\xab\xbb\x89\x77\x94\x47\xca\xfd\x83\xcc"
      "\xb3\x9f\x80\x05\x0b\x37\x5e\x91\x05\x9e\x32\x29\x73\xa9\x5e\x62"
      "\x3b\xf8\x96\x11\xa3\xe4\xe3\x4f\x2a\x42\x98\x40\x6f\xb4\x92\xfa"
      "\xa8\xdd\xce\x5b\x6c\xc1\x91\x11\x27\xb8\xbb\x1b\xa0\xd4\x8a\x0d"
      "\x93\x95\x37\x8a\x65\xed\xcf\x1f\x06\xb3\xe2\x9e\xd7\xaf\x43\x16"
      "\x5b\xb1\x84\xcf\x8d\x09\xce\x74\xf5\x57\x9a\xc1\x59\x3b\x4f\x38"
      "\xcf\x29\x38\x92\xc4\xdf\x28\x23\x6d\x95\x66\xcf\x74\x3f\x89\x26"
      "\x6e\x5b\xbf\x9a\x38\x5d\xdc\x97\x46\x42\x4e\xb0\xc0\xda\xd1\x62"
      "\x72\x97\xd6\x20\x48\x24\x4f\xb4\x10\x0f\xf4\xe7\xeb\x19\xcf\xd6"
      "\x44\x3c\x7e\x52\xe3\xab\xae\xf8\x72\x91\xed\x89\x04\x38\x7f\x44"
      "\xba\xd4\x2c\xc4\xc3\xe2\xde\x2d\x4a\x1c\x3b\x8b\xf4\x97\xe2\xbd"
      "\x47\xe6\x90\x7b\x80\xdd\x13\x8c\x43\x43\xc1\x30\x05\x3b\xd8\x14"
      "\x73\x16\xdc\x07\xe6\x2d\x62\x97\x00\x28\x9d\xfa\x4f\xd5\x34\xbf"
      "\x38\xa7\xdb\x52\x7e\x10\xa8\x53\x4a\xf0\xe3\x31\xa7\x57\xed\x09"
      "\x89\x26\x92\x0c\x5f\x8d\x52\x1b\x71\x8a\x6a\x91\x37\xc1\x9c\x04"
      "\xa4\xc7\x7e\xa6\xb3\x3f\xd3\x07\xd3\x58\xdf\x5d\xc9\x9d\x77\x2e"
      "\x1b\xb6\xa1\xea\x2d\xb8\xaf\x80\xe4\x43\x5e\xc5\x2c\x60\x88\xb1"
      "\x1b\xcb\x44\x8f\x55\x23\xd3\x0b\x6f\x8c\x39\xa6\xce\x65\x77\x0f"
      "\x28\xda\xcb\xc7\x95\x2c\xf4\x2f\x1e\x6f\xf5\x16\x3f\x17\x2f\x76"
      "\x16\xcd\x89\x3f\x6a\x01\x30\xfa\xb0\x6b\x31\xe6\xa6\x3f\x2d\x7c"
      "\xb4\x9e\x32\xbd\x09\xfa\x78\xe3\x25\x4c\x9c\x32\xea\x89\xb5\x11"
      "\x8f\x44\xf5\xe1\xd2\x07\xf5\x4d\xce\x0f\x4e\x30\x5e\x4d\xf6\x52"
      "\x3d\x22\xaa\x44\xdc\x69\x81\xe2\x9b\x12\x22\x90\x7c\x42\xde\xa0"
      "\x91\x4a\xb9\x20\xff\x9a\xdb\x50\x88\x28\x6c\xe6\x13\xa2\x06\x8d"
      "\x66\xde\xfc\xde\x75\xe3\x6e\x95\xfa\xf7\x77\xf0\x30\x18\x33\x9d"
      "\x33\x4f\x27\x97\x82\x3d\xb8\x1d\xc2\x26\xfa\xa8\x1e\xbc\xf0\x82"
      "\x1f\xaa\x11\x5b\xe9\x11\xd4\x88\x1d\x51\xba\xc8\x33\xf1\xd9\x06"
      "\x2a\xd6\xfe\xd0\x80\x56\x69\xcc\xa1\x8f\x2d\xaf\x15\x8e\x59\x98"
      "\xe9\x65\xb3\xc1\x15\x5c\x51\x86\xad\xba\x09\xa2\x79\x6b\xbe\x6e"
      "\x53\x68\x48\x78\xd6\xee\x04\xbe\x88\x29\x12\x2c\xa2\xe4\xf9\xe7"
      "\x3a\x5d\x99\x9a\x06\xc3\x85\x19\x73\xa2\x0a\x38\xcb\x49\xd9\x83"
      "\x97\x71\x40\xbb\x8e\x40\xa9\x63\x8e\xf1\x91\xa0\x66\xe6\xde\xab"
      "\x75\x18\x6d\xfa\xe3\x9e\xf2\xe1\x36\x1d\xe4\x1e\x32\x88\x65\x70"
      "\x27\xc5\xb4\x12\xde\x85\x9d\x5d\x29\xce\xb1\x52\xdb\xde\xe9\x90"
      "\x2c\x87\x77\x2e\x25\xf8\x3e\xc8\x8e\xfe\x43\xa4\x3b\x26\xc2\xa3"
      "\xda\x8b\x78\x54\xaa\x22\x27\xcb\xac\x51\xf7\x97\x2a\x23\x9f\x94"
      "\x87\xb8\xaa\x25\x90\x08\x3c\x45\x90\x7d\x17\xf3\x91\x6f\x35\xc3"
      "\x99\x01\xc0\xb4\x0f\x72\x64\xc0\xbe\x29\x6b\xcb\x50\x62\x8e\xbd"
      "\xac\x61\xb0\x6c\xd3\x7e\xc3\x7e\xc4\x62\x56\x19\x2e\xe1\x1a\xa9"
      "\xf1\xbd\x28\xb5\x7b\x3a\xee\x86\x80\xcd\x3b\x96\x3a\xda\xa5\x1a"
      "\x99\x28\x5b\x5c\x05\xe3\xea\x06\x92\x21\x68\xdb\x9a\xa5\x9d\xaf"
      "\x6b\x34\x39\x68\x9f\x07\xf5\x96\x4e\xb6\x12\x17\xc4\xbb\x26\xc6"
      "\x32\x6b\xd9\x8c\x85\x51\x5d\xbe\xd0\xa4\xc4\xd1\x57\x62\xd3\x4b"
      "\x34\xa3\x79\x9f\x65\xb8\x86\xca\xdf\xb8\x12\x13\x04\x82\x29\xa8"
      "\x2f\x63\x68\xc2\xb0\xa8\x79\xd6\x61\x07\x5f\xa2\xef\x9e\xb5\xc8"
      "\xf4\xa8\x00\xeb\x1e\xf1\xf3\x97\xc9\xfb\x6a\xca\x7c\x4b\xce\x31"
      "\x06\x2a\x89\x90\x9d\x25\xa2\x15\xcd\x7d\x10\xfe\x45\x71\xbb\x3e"
      "\x23\xb9\xb0\x7c\x45\xb4\xbc\xf8\xaa\x60\x2c\x0e\x70\x4b\x8e\x93"
      "\x58\xc4\x36\x03\x9e\x6c\x6e\xf4\x7f\xca\x7d\x86\x37\x48\xa4\x25"
      "\x91\x05\x93\xed\xd8\xf7\x2a\x36\x25\xf7\x4e\xe9\x59\x0a\x9d\xd9"
      "\x86\x04\xcd\x9b\x40\x40\x1c\x9e\x86\xc6\xc8\x01\x28\x0f\x9e\x22"
      "\x38\x07\xda\xf6\x3e\xce\x4c\x89\xb8\x72\x2f\xeb\xfc\x28\x30\x4b"
      "\xca\x6c\x97\xd8\xe1\xfc\x3e\x99\x55\x17\xe3\x2e\x59\xc6\x27\x44"
      "\x5b\x06\x5e\x5d\x65\x63\x0f\x0f\x9e\x6f\xdb\xe0\x91\x09\x9d\x51"
      "\x79\x3a\xec\xed\x6c\xbe\xf2\x48\x03\x1c\xee\x98\x2c\xd9\x14\xaa"
      "\x89\xf0\xf1\x05\x90\xa6\x89\x32\x0b\xfd\x08\x24\x7e\xa5\xf0\x42"
      "\x84\xbd\x84\xbd\xe2\x70\x6a\x0c\xf5\x76\xe4\x7b\xad\x69\x51\xbb"
      "\xe7\x22\x67\xc5\x5e\x36\xdd\x9a\x71\xc3\x07\x0e\x1a\xc8\x22\x51"
      "\x4d\x9e\xf2\x42\x62\x18\xeb\x1b\xe2\xea\xe9\xfd\xa0\x7a\xcd\xb6"
      "\x0b\x28\x4c\xac\x94\x7a\x9e\x2b\xce\x1e\xd5\x22\x1f\x4d\xb4\x10"
      "\x6d\xde\xe8\x55\x09\xe6\x9b\x21\xdc\x02\x49\x8f\x56\xe3\x22\xbb"
      "\xf3\xe8\x83\x68\x5d\x8b\xf8\x02\xfe\xce\xed\x73\x45\xf5\x2c\x1b"
      "\xa6\xf0\x86\x75\xcf\x8d\x87\xee\x8f\x40\xeb\x9a\xe3\xb0\xe8\x02"
      "\x80\x2d\xc1\x0c\xce\x55\x42\x64\x3b\xc7\x25\x2f\xba\x36\xf4\xda"
      "\xe8\x34\x15\x9a\x8d\x1a\x3d\xda\x49\x65\x29\x5f\x00\x4a\x51\xb8"
      "\x69\x56\x94\xb6\x5a\x07\x39\x0f\x84\xad\x7a\x92\x14\x91\xe1\x8d"
      "\x8b\x44\xc7\xa8\x4a\x2a\x7e\xd7\xf0\x2e\x99\xc7\x55\x18\x79\xf0"
      "\x41\xcf\x7c\xa3\x89\x45\xff\xa5\xc7\x99\xf2\x6b\xe6\x22\xf6\xd9"
      "\x7f\xd1\xa1\xe1\xba\xc4\xe7\xbf\x2c\xc3\xaa\xfb\x35\xd6\xac\xc8"
      "\xbd\x0a\x28\x26\x19\x8b\x5f\xcd\x8a\xe7\xb7\x49\x14\x37\x72\x9d"
      "\x5c\xc8\x01\x42\xfc\x57\x45\x46\x2d\x82\x8e\x0d\x68\xf6\x79\xc5"
      "\x16\x2d\x4d\xca\x73\x0a\x1b\x1b\x16\xb5\x6f\xca\x77\x16\x6a\x40"
      "\x32\x01\x1e\x17\xfe\xcf\x6e\x18\x31\x4e\xa2\x43\x4b\x40\x49\xcb"
      "\xce\xfd\x11\x2e\xdf\x09\x3b\x17\x5e\xec\x0d\x0e\x9e\x4d\xfd\xd9"
      "\x73\x91\x48\xf7\xcd\x9a\x7c\x6d\x18\x88\xb7\x99\x7d\x50\x8b\xbb"
      "\x53\x0d\x52\x01\xb5\xc7\xd8\xa9\x44\x4e\xb6\x39\x5f\x2f\x37\x79"
      "\x18\xbb\x95\x20\x2a\x58\xf8\x63\x8d\x24\x37\x03\x04\x39\xf7\xa6"
      "\xcd\x95\x94\x64\x90\xf7\x6c\x1c\xef\x17\x99\x83\x69\xfb\xfd\xbc"
      "\xe3\x4e\x45\x89\x4b\x47\x91\x8a\x23\x67\xa9\x88\x19\xe2\xa5\x98"
      "\x50\xe0\x59\x26\x7d\xc5\x23\xd5\xa5\xa1\x58\xb0\x55\x0a\x76\x1b"
      "\x6d\x5a\xba\x46\x5e\x2b\x14\x13\x09\x00\x99\x7d\x91\x8d\x81\x29"
      "\x7d\x61\xd4\x7a\x52\x14\xe4\x1d\x92\x5b\x78\x2d\xe1\x29\xf3\x80"
      "\x1b\xd2\x19\x44\x5e\xd3\xdd\x9a\x17\xe4\x50\x69\x31\xca\x59\xd5"
      "\x35\x8b\x30\xe6\x9a\x3d\xc2\x47\xd3\x5a\xf9\x16\x12\x23\x49\x7f"
      "\x1a\xcb\x60\x1a\xc9\x2b\x5b\xcd\x22\xd8\x97\x38\xdc\xa1\x61\x19"
      "\x92\x30\x4d\xe7\x96\x76\xbd\x92\x20\x58\xd2\x1b\x46\xc3\x69\x04"
      "\x8c\xf3\x0e\xc6\x92\x87\x80\xdf\x97\x07\x6d\xef\x92\xb9\x5d\x03"
      "\xa3\xef\xb6\xd9\x39\x27\x8d\x6a\x3d\x7b\x6b\xe8\x18\x06\xcf\x00"
      "\xca\xe3\xc8\x8d\x0b\x90\xa8\xa2\xe2\xdf\xb6\x98\xda\x15\x38\xa3"
      "\x48\x0c\xf7\x5d\xe5\x4b\x58\x27\x8a\x60\x3a\x9f\x6c\x7b\x8b\x13"
      "\x37\x65\xd6\xf2\x2b\x1d\xa2\x91\x2f\x41\x1b\x66\xd5\xc6\x91\x58"
      "\xe0\xd2\xef\xad\x5b\xdd\xaa\x1a\xdd\xf3\x69\x48\xa2\x1b\xa1\x04"
      "\xf6\x3f\xfb\xbe\xc0\xa7\x0f\xdc\xc6\x33\xe7\xe2\x13\x74\xd9\xc1"
      "\x69\x09\xc1\x25\xc6\xb4\x8b\x04\x61\x96\x49\xb6\xa8\xf5\x7c\x7c"
      "\x63\x51\x53\x77\xd4\xfa\x03\x62\x92\xd3\x93\x46\xed\xba\xb6\xe5"
      "\xde\x4a\x13\xa3\xc0\x28\xe1\x03\x4e\x2b\x75\xc4\x50\x3b\x33\xf2"
      "\x36\x43\x22\xbb\x3f\x68\xe0\x06\xc0\x8e\x9e\xf9\x2a\x22\xc0\x82"
      "\x43\xb9\x07\x4f\xc1\xfd\x57\xe2\x32\xde\x23\xb8\xcb\x46\x9e\xef"
      "\x07\x65\xb7\xdd\x60\xee\x35\xc1\x1f\xdc\xea\xcf\x10\x04\xe5\xe6"
      "\x69\x9f\x3b\x76\xd0\x57\xd5\xdc\x68\x05\x57\x95\x5f\x5d\xe5\x00"
      "\xc6\xbc\x01\xff\x73\x21\x2c\x51\x87\xbe\xf5\xdd\xc7\xfa\x25\xca"
      "\xac\xdc\xd7\x2e\x6f\x0b\x8f\x97\x95\x48\x08\x52\x6e\xd1\x7b\x49"
      "\x4e\xa7\xcb\xdb\x01\xd3\x50\x06\xf7\x49\xb7\xd2\x65\x97\x7c\xd3"
      "\x5a\x21\xa9\x68\xce\xcb\x97\x04\x8d\x11\x9a\x82\x80\x74\x46\xe8"
      "\x48\x6a\x98\x00\x04\xb8\xfe\x7e\x99\xd7\x43\xc1\x00\x91\xf7\x11"
      "\xcf\x47\x87\x51\xd3\x0b\x35\x6f\x3b\xd5\x4a\x9c\xba\xd0\xef\xb8"
      "\x74\xea\xed\xb2\x22\xcc\x0b\x85\xea\x34\x5c\x6f\x75\x18\x43\x48"
      "\x1e\x60\x8b\xef\x7e\x8b\xea\x2d\x78\xe6\x8b\x30\x9e\x47\x63\xee"
      "\x20\xe1\x14\xeb\x05\xa0\x23\xd7\x9c\x6b\x01\xd1\x3f\x1c\xe0\xb8"
      "\x32\x43\x0a\x6f\xc8\xdc\xc3\x26\x92\x36\xfe\x5e\xf8\x7d\xb4\x0b"
      "\xc2\x87\x20\xc0\xf5\x35\xcc\xbc\x53\x04\xda\x94\xf6\xf2\xc1\x4a"
      "\x79\x75\x80\x33\x80\x10\x12\x62\x4f\x9f\xe2\x31\x93\xea\x94\x2a"
      "\x76\x26\x42\xe1\x3e\x3c\x96\xf9\x8c\x06\xc4\xf3\x14\xdb\xf7\x86"
      "\x00\x88\x44\xd7\x82\x18\x12\x25\x55\x76\xc7\xfe\x28\x62\x9b\xd8"
      "\x3e\xd9\xd9\x32\xa7\xb9\x60\xe4\x06\x6a\xe1\x28\xf2\xc2\xed\x6b"
      "\x84\x9c\xc9\x47\xd5\xcd\x0a\x25\xd8\xb9\x60\x54\xca\x0d\xc0\xc9"
      "\x34\xaf\x29\x86\xbd\x62\xe2\xf9\x43\x12\x8d\x70\x77\x04\xf6\x36"
      "\xe0\x1f\xe3\x7e\xc5\x97\xf6\xd1\xf3\xe0\x6f\x4b\xe7\x3e\x16\x93"
      "\x25\x67\x23\x35\x59\x83\x1f\x99\x97\x2e\xc5\x9a\xe0\xf0\xfa\xf9"
      "\xab\x23\x21\x1e\xf0\xe6\xf1\xe2\x62\x70\x76\x99\xfc\x58\x18\x97"
      "\xa6\x74\x65\x08\x3c\xab\x66\xae\xe9\xdd\x7b\xcb\x4c\xda\x4a\x7c"
      "\x74\x78\x50\x32\x3e\xc6\xff\xd2\xd8\x93\xee\x55\xe4\x49\x10\x12"
      "\xdd\x2f\x61\x50\x7d\x2e\xd8\x58\x48\xd3\x0e\x27\x3e\x1b\xea\x81"
      "\xe8\xc2\xd1\x84\x28\xdf\x5e\x2b\xdd\xbd\x4f\xa2\x33\xdf\x1d\xd0"
      "\x3c\x2d\x26\x1e\x76\xfe\x6b\x82\x35\xe5\xd5\x1e\xe9\xfb\x4b\x06"
      "\xde\xd7\xbb\x4d\x12\x93\x0b\x61\x53\x08\x53\x19\xd8\x0e\x1c\xab"
      "\x9c\xfb\xd1\x15\xff\x70\x7e\xa6\x05\x49\xce\xd8\x74\x30\x20\xfc"
      "\x18\xba\xe4\x05\x0b\x4e\x5d\x56\x1d\x8d\xa3\x47\x10\x81\x05\xf8"
      "\xb0\x4f\x68\xab\x50\x23\xa1\x41\xa3\x6a\x06\xb8\xf8\x25\x42\x74"
      "\xfc\x66\xd3\x9d\xbf\xe8\x57\x74\xec\xf5\x4e\x67\x0f\xb6\xe0\xed"
      "\xc6\xfc\x47\x05\x7f\x92\xf4\xd8\x59\x3c\x58\xdb\x72\x01\x23\x05"
      "\xe1\xff\xff\xa0\xe6\x33\xbe\xc2\x7b\x50\x20\x8a\x1d\x6e\x3e\x51"
      "\xbc\xd7\x13\x9e\x42\xd9\x30\x9c\xf8\x6c\xff\x4d\x0f\x81\xf2\x9f"
      "\x14\x48\x53\xe9\xaf\x9e\x66\x31\x24\xb8\xd6\xdd\xac\xaa\xf7\x2d"
      "\x79\x03\xdb\x22\xe7\xa5\x9d\xb6\x5f\x91\xd9\x99\x6d\xa7\xc0\x5a"
      "\x4c\x2c\xb6\xe4\x32\xb0\x65\x51\x77\xba\xe7\x90\xe0\x7e\xdf\x02"
      "\x5f\x62\x01\x82\x0c\x25\xa7\x11\xbf\x4d\x3f\x87\x9e\xe2\x3e\x82"
      "\x37\xe1\xea\x36\x60\x12\x87\xcf\xb3\x41\x72\xda\x29\x32\xda\x10"
      "\x39\x76\x78\xc4\x53\x6b\x17\x87\x91\xce\x70\xb5\x50\x0a\x14\xbd"
      "\xee\xb4\x5e\xd0\x5c\x0d\xc5\x6e\xa0\x50\x25\x7d\xf9\xb9\x65\x39"
      "\x29\xb3\x9e\xd3\x58\x78\xe5\x56\x74\x95\x8f\xb5\x3a\xaf\xb3\x78"
      "\xf1\xe5\x96\xf5\xea\x0e\x03\xf8\x93\xcf\x68\x56\x93\x30\xe4\x93"
      "\x71\x76\xd7\xbf\x05\x11\x11\xa4\x39\x96\xc9\x53\xab\xcf\x20\xec"
      "\xf2\x24\x4d\xa4\x21\x34\xe0\xb9\x7b\x50\xe5\x89\xa9\x64\xe8\x95"
      "\xe6\x06\x6c\x07\xd2\x16\xd3\x48\x8b\x79\xfc\x19\x5f\x54\xca\x24"
      "\x59\x3c\x3f\xe6\x36\x3b\x51\x1a\x28\x2f\x9a\xa4\x00\x5b\x14\x63"
      "\x55\x10\x51\xd9\x3e\xe2\xbf\xf3\xdf\xee\xc7\x8e\x83\xf0\x6f\x12"
      "\x31\xd0\x6b\x10\xe1\x43\xb8\x87\x0e\x57\x9b\x51\x59\x27\x0d\xe5"
      "\x8c\x02\xc0\xf4\xd6\xb2\xef\x11\xc6\x55\xe5\x16\x7c\xdf\xb9\xb8"
      "\xc6\x3d\x0a\x94\xae\x54\x3b\x80\x3c\xdb\x9d\x93\x38\xa3\xac\x69"
      "\xc5\x56\x2a\x91\x26\x45\x97\xf2\x37\xe9\xac\xeb\xb9\xa3\x31\xf7"
      "\xb5\xbe\xdb\x5e\x3b\xf9\xcb\xf2\x2d\x27\x90\x01\xfb\x09\x80\xfe"
      "\x33\x26\x5f\x5e\x9e\x93\x95\x6c\xbb\x59\xe3\xd9\x53\xce\xe9\x63"
      "\x9e\x29\xaa\xd6\x50\x16\x4a\x80\xca\x1f\xa1\x09\x94\x5f\xab\x73"
      "\x32\x60\x68\xa8\x98\x42\x0b\xcb\xe1\x40\xb2\xe9\xdf\x1f\x68\xbb"
      "\x30\x1c\xf4\x3d\x5b\x4a\x3c\x0a\x67\x00\x19\x57\xf3\x66\x9e\x39"
      "\x24\x11\xea\xb7\x53\x7b\xa6\x5d\x9d\x66\xea\x06\x6d\xf4\xb2\x10"
      "\x5d\x4b\xd1\x8e\xba\x49\x5b\x48\xc6\xeb\x2f\x8c\x0d\x53\x47\x59"
      "\xec\x57\xee\xd2\xa0\x50\xbc\xed\xd5\x0f\x9a\xb4\x30\xdd\xac\x59"
      "\x0b\xc2\xa9\x16\x85\x05\xe3\xea\x37\x27\xeb\xae\x9d\xbc\xe1\xfd"
      "\x1c\x53\x74\xca\xd3\xfa\x8d\x28\x8c\x8a\x78\x24\xa5\x94\xe6\xd1"
      "\x58\xb8\x8a\x94\xe6\x30\x42\xb1\x05\xda\x1c\x30\xf8\x7d\x24\x06"
      "\x51\x7f\x5a\xb6\xe5\xa1\xbe\x86\x00\xfe\x4e\x84\x9f\x4b\x62\xf2"
      "\x3b\x11\x5e\x64\x6c\x49\x44\x13\xfc\x32\x07\x9f\x9c\xa0\x9b\x0c"
      "\x0d\x03\xe6\x6a\x81\x60\xe9\xbe\xd0\xdd\x2f\x10\x82\x79\x24\x25"
      "\xa0\x71\x07\xcc\x42\x33\x09\x0d\x9e\x69\x9c\xf5\x97\x82\xd7\x69"
      "\x08\x2e\xe8\x5b\xb8\x4f\x96\xdf\xea\x42\x94\x95\xb9\x7a\x18\xa8"
      "\xd0\x99\xf3\xe9\x79\x7d\xb9\x84\xe7\x85\x50\xbb\xa2\x99\x91\xf7"
      "\xf4\xfd\xc3\x5b\xf0\x2f\xd1\x24\x2f\x86\x03\x03\x03\x1e\x0f\x62"
      "\xfe\x32\xc7\x45\x91\x5c\x55\x11\xbc\xdb\xe3\x00\x48\x91\x1f\xa6"
      "\x3b\xb1\xc7\xec\xaf\x80\xb3\x43\xec\xca\x6d\xb1\x8b\x87\xfe\xbf"
      "\xbc\x85\x4a\x17\x46\xfe\x73\x30\x84\xb8\x1e\x5e\x94\xd6\x4c\xe2"
      "\xbe\x0b\x3f\xdb\xe3\xf8\x01\x9c\x9c\x41\x99\x87\xff\xb3\xc0\xea"
      "\x66\xdb\xf6\x1d\x68\xb8\xcc\x1d\x18\x84\xf6\xca\x84\x4c\xf6\x12"
      "\x73\x5e\xce\xdd\xb4\x61\x5a\x1a\x95\x05\x72\xcd\x6e\x4c\xd6\x7e"
      "\x3b\xd7\x8d\x4e\xe4\x28\x20\x6f\x2e\x48\xe9\xc3\xe7\x0b\xee\xe5"
      "\xcf\x84\x86\xfd\x81\x9d\x8d\x74\xb8\xa7\xdd\x3c\xb9\xb2\x79\xd0"
      "\xb2\x7a\x23\xd0\xd7\x99\x68\x2d\x78\xb2\x07\x72\xdd\x0b\xcd\x0f"
      "\x85\xf6\x8c\xe2\x0d\xcc\x86\xb7\x47\x95\x8f\x56\xdf\xab\xf5"
      "\x15",
      4096);
  r[13] = syscall(__NR_ptrace, 0x4205ul, r[7], 0x202ul, 0x20000000ul);
}

int main()
{
  loop();
  return 0;
}
