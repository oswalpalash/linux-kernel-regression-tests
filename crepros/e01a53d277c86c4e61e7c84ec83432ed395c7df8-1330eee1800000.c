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

long r[2];
void test()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
          0xfffffffffffffffful, 0x0ul);
  r[0] = syscall(__NR_socket, 0x26ul, 0x5ul, 0x0ul);
  *(uint16_t*)0x20f84fa8 = (uint16_t)0x26;
  memcpy((void*)0x20f84faa,
         "\x73\x6b\x63\x69\x70\x68\x65\x72\x00\x00\x00\x00\x00\x00",
         14);
  *(uint32_t*)0x20f84fb8 = (uint32_t)0x0;
  *(uint32_t*)0x20f84fbc = (uint32_t)0x0;
  memcpy((void*)0x20f84fc0,
         "\x65\x63\x62\x28\x74\x77\x6f\x66\x69\x73\x68\x29\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00",
         64);
  syscall(__NR_bind, r[0], 0x20f84fa8ul, 0x58ul);
  memcpy((void*)0x203c1000, "\x0a\x07\x75\xb0\xd5\xe3\x83\xe5\xb3\xb6"
                            "\x0c\xed\x5c\x54\xdb\xb7",
         16);
  syscall(__NR_setsockopt, r[0], 0x117ul, 0x1ul, 0x203c1000ul, 0x10ul);
  r[1] = syscall(__NR_accept, r[0], 0x0ul, 0x0ul);
  *(uint64_t*)0x2037df20 = (uint64_t)0x0;
  *(uint32_t*)0x2037df28 = (uint32_t)0x0;
  *(uint64_t*)0x2037df30 = (uint64_t)0x2059e000;
  *(uint64_t*)0x2037df38 = (uint64_t)0x1;
  *(uint64_t*)0x2037df40 = (uint64_t)0x20423000;
  *(uint64_t*)0x2037df48 = (uint64_t)0x0;
  *(uint32_t*)0x2037df50 = (uint32_t)0x0;
  *(uint64_t*)0x2059e000 = (uint64_t)0x20f7a000;
  *(uint64_t*)0x2059e008 = (uint64_t)0x30;
  memcpy((void*)0x20f7a000, "\xa2\x20\x07\xa5\x24\x2d\xbe\x84\x29\x6e"
                            "\xc1\xe6\x76\x61\x12\x41\xc2\x1a\x86\x71"
                            "\x75\x83\xb6\x94\xb9\x88\x24\x56\xa8\x18"
                            "\x36\x68\x90\x1c\xb1\x8c\xe9\x71\xe5\x41"
                            "\xaf\x3e\x6f\x16\xfd\x6c\x39\x5f",
         48);
  syscall(__NR_sendmmsg, r[1], 0x2037df20ul, 0x1ul, 0x0ul);
  *(uint64_t*)0x201f5000 = (uint64_t)0x20f7ffa8;
  *(uint32_t*)0x201f5008 = (uint32_t)0x58;
  *(uint64_t*)0x201f5010 = (uint64_t)0x20f82000;
  *(uint64_t*)0x201f5018 = (uint64_t)0x1;
  *(uint64_t*)0x201f5020 = (uint64_t)0x20f7ffca;
  *(uint64_t*)0x201f5028 = (uint64_t)0x57;
  *(uint32_t*)0x201f5030 = (uint32_t)0x0;
  *(uint64_t*)0x20f82000 = (uint64_t)0x20f81000;
  *(uint64_t*)0x20f82008 = (uint64_t)0x1000;
  syscall(__NR_recvmsg, r[1], 0x201f5000ul, 0x0ul);
}

int main()
{
  loop();
  return 0;
}
