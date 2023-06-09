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
  syscall(__NR_mmap, 0x20000000, 0xe7a000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 0xa, 2, 0);
  r[1] = syscall(__NR_socket, 0x18, 1, 1);
  *(uint16_t*)0x2014cfd2 = 0x18;
  *(uint32_t*)0x2014cfd4 = 1;
  *(uint32_t*)0x2014cfd8 = 0;
  *(uint32_t*)0x2014cfdc = r[0];
  *(uint16_t*)0x2014cfe0 = 2;
  *(uint16_t*)0x2014cfe2 = htobe16(0x4e22);
  *(uint32_t*)0x2014cfe4 = htobe32(-1);
  *(uint8_t*)0x2014cfe8 = 0;
  *(uint8_t*)0x2014cfe9 = 0;
  *(uint8_t*)0x2014cfea = 0;
  *(uint8_t*)0x2014cfeb = 0;
  *(uint8_t*)0x2014cfec = 0;
  *(uint8_t*)0x2014cfed = 0;
  *(uint8_t*)0x2014cfee = 0;
  *(uint8_t*)0x2014cfef = 0;
  *(uint32_t*)0x2014cff0 = 4;
  *(uint32_t*)0x2014cff4 = 0;
  *(uint32_t*)0x2014cff8 = 3;
  *(uint32_t*)0x2014cffc = 0;
  syscall(__NR_connect, r[1], 0x2014cfd2, 0x2e);
}

int main()
{
  for (;;) {
    loop();
  }
}
