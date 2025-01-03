// https://syzkaller.appspot.com/bug?id=9fcc32fe46ce159d6a3e64b64ab3402a7ac2e9b8
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

long r[33];
void test()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  *(uint16_t*)0x20db4ff0 = (uint16_t)0x2;
  *(uint16_t*)0x20db4ff2 = (uint16_t)0x204e;
  *(uint32_t*)0x20db4ff4 = (uint32_t)0x0;
  *(uint8_t*)0x20db4ff8 = (uint8_t)0x0;
  *(uint8_t*)0x20db4ff9 = (uint8_t)0x0;
  *(uint8_t*)0x20db4ffa = (uint8_t)0x0;
  *(uint8_t*)0x20db4ffb = (uint8_t)0x0;
  *(uint8_t*)0x20db4ffc = (uint8_t)0x0;
  *(uint8_t*)0x20db4ffd = (uint8_t)0x0;
  *(uint8_t*)0x20db4ffe = (uint8_t)0x0;
  *(uint8_t*)0x20db4fff = (uint8_t)0x0;
  r[12] = syscall(__NR_sendto, 0xfffffffffffffffful, 0x20296f19ul,
                  0x0ul, 0x20008045ul, 0x20db4ff0ul, 0x10ul);
  *(uint16_t*)0x20019000 = (uint16_t)0x2;
  *(uint16_t*)0x20019002 = (uint16_t)0x234e;
  *(uint32_t*)0x20019004 = (uint32_t)0x0;
  *(uint8_t*)0x20019008 = (uint8_t)0x0;
  *(uint8_t*)0x20019009 = (uint8_t)0x0;
  *(uint8_t*)0x2001900a = (uint8_t)0x0;
  *(uint8_t*)0x2001900b = (uint8_t)0x0;
  *(uint8_t*)0x2001900c = (uint8_t)0x0;
  *(uint8_t*)0x2001900d = (uint8_t)0x0;
  *(uint8_t*)0x2001900e = (uint8_t)0x0;
  *(uint8_t*)0x2001900f = (uint8_t)0x0;
  r[24] =
      syscall(__NR_connect, 0xfffffffffffffffful, 0x20019000ul, 0x10ul);
  r[25] = syscall(__NR_socket, 0x26ul, 0x5ul, 0x0ul);
  *(uint16_t*)0x202bf000 = (uint16_t)0x26;
  memcpy((void*)0x202bf002,
         "\x68\x61\x73\x68\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         14);
  *(uint32_t*)0x202bf010 = (uint32_t)0x21f;
  *(uint32_t*)0x202bf014 = (uint32_t)0x8000;
  memcpy((void*)0x202bf018,
         "\x68\x6d\x61\x63\x28\x73\x68\x61\x32\x35\x36\x29\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00",
         64);
  r[31] = syscall(__NR_bind, r[25], 0x202bf000ul, 0x58ul);
  r[32] = syscall(__NR_setsockopt, r[25], 0x117ul, 0x1ul, 0x20a7ff91ul,
                  0x0ul);
}

int main()
{
  loop();
  return 0;
}
