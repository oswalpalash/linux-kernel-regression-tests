// https://syzkaller.appspot.com/bug?id=e2dc9aea0465d1eea101bb24cb463e2a7efe7d17
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}

#include <stdint.h>
#include <string.h>

const int kFailStatus = 67;
const int kRetryStatus = 69;

static void fail(const char* msg, ...)
{
  int e = errno;
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit((e == ENOMEM || e == EAGAIN) ? kRetryStatus : kFailStatus);
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
      fail("loop fork failed");
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

long r[58];
void test()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  r[1] = syscall(__NR_socket, 0x26ul, 0x5ul, 0x0ul);
  *(uint16_t*)0x20dfffa8 = (uint16_t)0x26;
  memcpy((void*)0x20dfffaa,
         "\x73\x6b\x63\x69\x70\x68\x65\x72\x00\x00\x00\x00\x00\x00",
         14);
  *(uint32_t*)0x20dfffb8 = (uint32_t)0x1;
  *(uint32_t*)0x20dfffbc = (uint32_t)0xa00;
  memcpy((void*)0x20dfffc0,
         "\x6c\x72\x77\x28\x74\x77\x6f\x66\x69\x73\x68\x29\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00",
         64);
  r[7] = syscall(__NR_bind, r[1], 0x20dfffa8ul, 0x58ul);
  memcpy((void*)0x2057afe0, "\x0a\x07\x75\xb0\xd5\xe3\x83\xe5\xb3\xb6"
                            "\x0c\xed\x5c\x54\xdb\xb7\x29\x5d\xf0\xdf"
                            "\x82\x17\xad\x40\x00\x00\x00\x00\x00\x00"
                            "\x00\xe6",
         32);
  r[9] = syscall(__NR_setsockopt, r[1], 0x117ul, 0x1ul, 0x2057afe0ul,
                 0x20ul);
  r[10] = syscall(__NR_accept, r[1], 0x0ul, 0x0ul);
  *(uint64_t*)0x20f6d000 = (uint64_t)0x0;
  *(uint32_t*)0x20f6d008 = (uint32_t)0x0;
  *(uint64_t*)0x20f6d010 = (uint64_t)0x206bcf60;
  *(uint64_t*)0x20f6d018 = (uint64_t)0xa;
  *(uint64_t*)0x20f6d020 = (uint64_t)0x20953000;
  *(uint64_t*)0x20f6d028 = (uint64_t)0x0;
  *(uint32_t*)0x20f6d030 = (uint32_t)0x8090;
  *(uint64_t*)0x206bcf60 = (uint64_t)0x2043cf5a;
  *(uint64_t*)0x206bcf68 = (uint64_t)0x0;
  *(uint64_t*)0x206bcf70 = (uint64_t)0x20d53000;
  *(uint64_t*)0x206bcf78 = (uint64_t)0x0;
  *(uint64_t*)0x206bcf80 = (uint64_t)0x20504000;
  *(uint64_t*)0x206bcf88 = (uint64_t)0x0;
  *(uint64_t*)0x206bcf90 = (uint64_t)0x20dd7000;
  *(uint64_t*)0x206bcf98 = (uint64_t)0x0;
  *(uint64_t*)0x206bcfa0 = (uint64_t)0x202f2000;
  *(uint64_t*)0x206bcfa8 = (uint64_t)0x0;
  *(uint64_t*)0x206bcfb0 = (uint64_t)0x20db7f7b;
  *(uint64_t*)0x206bcfb8 = (uint64_t)0x0;
  *(uint64_t*)0x206bcfc0 = (uint64_t)0x2020a000;
  *(uint64_t*)0x206bcfc8 = (uint64_t)0x0;
  *(uint64_t*)0x206bcfd0 = (uint64_t)0x204db000;
  *(uint64_t*)0x206bcfd8 = (uint64_t)0x0;
  *(uint64_t*)0x206bcfe0 = (uint64_t)0x207a7fdf;
  *(uint64_t*)0x206bcfe8 = (uint64_t)0x0;
  *(uint64_t*)0x206bcff0 = (uint64_t)0x20f7a000;
  *(uint64_t*)0x206bcff8 = (uint64_t)0x58;
  memcpy((void*)0x20f7a000,
         "\x87\xeb\x0c\xc7\xd9\x3b\xc0\xb4\xcd\x4d\xc2\xb9\xde\x47\x23"
         "\x3c\x9b\xa5\x08\x30\x5d\xcd\x60\x1d\x22\x0c\x34\x6a\x12\x61"
         "\x4a\x52\xd2\x88\x47\x4b\x57\xf7\x9a\xf3\x53\x6f\xe1\xb4\x9e"
         "\x93\xd1\xfe\xa0\x5f\x5d\x58\xde\xd4\xa2\x40\xf6\xe4\x86\xb0"
         "\x75\xf2\x3d\x08\xf7\xf3\xa4\x6f\xc2\x11\x0d\xb8\x49\x60\x9a"
         "\x13\x6e\x9c\x4f\x6a\x86\x63\xec\xcb\x39\x9a\xe2\x8f",
         88);
  r[39] = syscall(__NR_sendmsg, r[10], 0x20f6d000ul, 0x0ul);
  *(uint64_t*)0x2022efc8 = (uint64_t)0x20f6eff0;
  *(uint32_t*)0x2022efd0 = (uint32_t)0x10;
  *(uint64_t*)0x2022efd8 = (uint64_t)0x20892fb0;
  *(uint64_t*)0x2022efe0 = (uint64_t)0x5;
  *(uint64_t*)0x2022efe8 = (uint64_t)0x20aa4fba;
  *(uint64_t*)0x2022eff0 = (uint64_t)0x0;
  *(uint32_t*)0x2022eff8 = (uint32_t)0x8;
  *(uint64_t*)0x20892fb0 = (uint64_t)0x2000bf7c;
  *(uint64_t*)0x20892fb8 = (uint64_t)0x0;
  *(uint64_t*)0x20892fc0 = (uint64_t)0x20f6ef87;
  *(uint64_t*)0x20892fc8 = (uint64_t)0x0;
  *(uint64_t*)0x20892fd0 = (uint64_t)0x20f72f2b;
  *(uint64_t*)0x20892fd8 = (uint64_t)0x0;
  *(uint64_t*)0x20892fe0 = (uint64_t)0x208a8f50;
  *(uint64_t*)0x20892fe8 = (uint64_t)0xb0;
  *(uint64_t*)0x20892ff0 = (uint64_t)0x20f6e000;
  *(uint64_t*)0x20892ff8 = (uint64_t)0x0;
  r[57] = syscall(__NR_recvmsg, r[10], 0x2022efc8ul, 0x2ul);
}

int main()
{
  loop();
  return 0;
}
