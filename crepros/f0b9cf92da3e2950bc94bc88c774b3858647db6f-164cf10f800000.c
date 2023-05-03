// https://syzkaller.appspot.com/bug?id=f0b9cf92da3e2950bc94bc88c774b3858647db6f
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
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
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

static void execute_one();
extern unsigned long long procid;

static void loop()
{
  int iter;
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      fail("clone failed");
    if (pid == 0) {
      prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
      setpgrp();
      execute_one();
      doexit(0);
    }

    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      int res = waitpid(-1, &status, __WALL | WNOHANG);
      if (res == pid) {
        break;
      }
      usleep(1000);
      if (current_time_ms() - start < 3 * 1000)
        continue;
      kill(-pid, SIGKILL);
      kill(pid, SIGKILL);
      while (waitpid(-1, &status, __WALL) != pid) {
      }
      break;
    }
  }
}

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[1] = {0xffffffffffffffff};
unsigned long long procid;
void execute_one()
{
  long res = 0;
  syscall(__NR_socketpair, 0, 0, 0, 0x20000140);
  syscall(__NR_socket, 0xa, 1, 0);
  *(uint32_t*)0x20000200 = 0x12;
  *(uint32_t*)0x20000204 = 7;
  *(uint32_t*)0x20000208 = 4;
  *(uint32_t*)0x2000020c = 1;
  *(uint32_t*)0x20000210 = 0;
  *(uint32_t*)0x20000214 = -1;
  *(uint32_t*)0x20000218 = 0;
  *(uint8_t*)0x2000021c = 0;
  *(uint8_t*)0x2000021d = 0;
  *(uint8_t*)0x2000021e = 0;
  *(uint8_t*)0x2000021f = 0;
  *(uint8_t*)0x20000220 = 0;
  *(uint8_t*)0x20000221 = 0;
  *(uint8_t*)0x20000222 = 0;
  *(uint8_t*)0x20000223 = 0;
  *(uint8_t*)0x20000224 = 0;
  *(uint8_t*)0x20000225 = 0;
  *(uint8_t*)0x20000226 = 0;
  *(uint8_t*)0x20000227 = 0;
  *(uint8_t*)0x20000228 = 0;
  *(uint8_t*)0x20000229 = 0;
  *(uint8_t*)0x2000022a = 0;
  *(uint8_t*)0x2000022b = 0;
  res = syscall(__NR_bpf, 0, 0x20000200, 0x2c);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000180 = r[0];
  *(uint64_t*)0x20000188 = 0x20000000;
  *(uint64_t*)0x20000190 = 0x20000140;
  *(uint64_t*)0x20000198 = 0;
  syscall(__NR_bpf, 2, 0x20000180, 0x20);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      for (;;) {
        loop();
      }
    }
  }
  sleep(1000000);
  return 0;
}
