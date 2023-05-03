// https://syzkaller.appspot.com/bug?id=11f60158f34305e6ff1e0db2205247afbc9f9604
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
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

long r[2];
void test()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xe75000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 0xa, 2, 0);
  *(uint16_t*)0x20e6f000 = 0xa;
  *(uint16_t*)0x20e6f002 = htobe16(0x4e22);
  *(uint32_t*)0x20e6f004 = 0;
  *(uint8_t*)0x20e6f008 = 0;
  *(uint8_t*)0x20e6f009 = 0;
  *(uint8_t*)0x20e6f00a = 0;
  *(uint8_t*)0x20e6f00b = 0;
  *(uint8_t*)0x20e6f00c = 0;
  *(uint8_t*)0x20e6f00d = 0;
  *(uint8_t*)0x20e6f00e = 0;
  *(uint8_t*)0x20e6f00f = 0;
  *(uint8_t*)0x20e6f010 = 0;
  *(uint8_t*)0x20e6f011 = 0;
  *(uint8_t*)0x20e6f012 = -1;
  *(uint8_t*)0x20e6f013 = -1;
  *(uint8_t*)0x20e6f014 = 0xac;
  *(uint8_t*)0x20e6f015 = 0x14;
  *(uint8_t*)0x20e6f016 = 0;
  *(uint8_t*)0x20e6f017 = 0xaa;
  *(uint32_t*)0x20e6f018 = 1;
  syscall(__NR_connect, r[0], 0x20e6f000, 0x1c);
  r[1] = syscall(__NR_socket, 0x18, 1, 1);
  *(uint16_t*)0x205fafd2 = 0x18;
  *(uint32_t*)0x205fafd4 = 1;
  *(uint32_t*)0x205fafd8 = 0;
  *(uint32_t*)0x205fafdc = r[0];
  *(uint16_t*)0x205fafe0 = 2;
  *(uint16_t*)0x205fafe2 = htobe16(0x4e21);
  *(uint32_t*)0x205fafe4 = htobe32(0xe0000002);
  *(uint8_t*)0x205fafe8 = 0;
  *(uint8_t*)0x205fafe9 = 0;
  *(uint8_t*)0x205fafea = 0;
  *(uint8_t*)0x205fafeb = 0;
  *(uint8_t*)0x205fafec = 0;
  *(uint8_t*)0x205fafed = 0;
  *(uint8_t*)0x205fafee = 0;
  *(uint8_t*)0x205fafef = 0;
  *(uint32_t*)0x205faff0 = 4;
  *(uint32_t*)0x205faff4 = 0;
  *(uint32_t*)0x205faff8 = 2;
  *(uint32_t*)0x205faffc = 0;
  syscall(__NR_connect, r[1], 0x205fafd2, 0x2e);
}

int main()
{
  for (;;) {
    loop();
  }
}
