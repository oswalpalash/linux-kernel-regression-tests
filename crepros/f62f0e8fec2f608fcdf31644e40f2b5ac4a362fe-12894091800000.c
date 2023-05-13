// https://syzkaller.appspot.com/bug?id=ddf389071baf9399451103c8316bd602bb0690cb
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

long r[1];
void test()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000ul, 0x2ee000ul, 0x3ul, 0x32ul,
          0xfffffffffffffffful, 0x0ul);
  r[0] = syscall(__NR_socket, 0x26ul, 0x5ul, 0x0ul);
  *(uint16_t*)0x202ebfa8 = (uint16_t)0x26;
  memcpy((void*)0x202ebfaa,
         "\x61\x65\x61\x64\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         14);
  *(uint32_t*)0x202ebfb8 = (uint32_t)0x1;
  *(uint32_t*)0x202ebfbc = (uint32_t)0x400;
  memcpy((void*)0x202ebfc0,
         "\x70\x63\x72\x79\x70\x74\x28\x72\x66\x63\x34\x31\x30\x36\x2d"
         "\x67\x63\x6d\x2d\x61\x65\x73\x6e\x69\x29\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00",
         64);
  syscall(__NR_bind, r[0], 0x202ebfa8ul, 0x58ul);
}

int main()
{
  loop();
  return 0;
}
