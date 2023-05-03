// https://syzkaller.appspot.com/bug?id=d369eafab7836fe0dd2e027d2689da06cad8e1a3
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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

long r[32];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    memcpy((void*)0x20038000, "\x2f\x64\x65\x76\x2f\x6b\x76\x6d\x00",
           9);
    r[2] = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20038000ul,
                   0x200ul, 0x0ul);
    break;
  case 2:
    r[3] = syscall(__NR_ioctl, r[2], 0xae01ul, 0x0ul);
    break;
  case 3:
    *(uint32_t*)0x2002ef88 = (uint32_t)0x1;
    *(uint32_t*)0x2002ef8c = (uint32_t)0x78;
    *(uint8_t*)0x2002ef90 = (uint8_t)0x0;
    *(uint8_t*)0x2002ef91 = (uint8_t)0x0;
    *(uint8_t*)0x2002ef92 = (uint8_t)0x0;
    *(uint8_t*)0x2002ef93 = (uint8_t)0x0;
    *(uint32_t*)0x2002ef94 = (uint32_t)0x0;
    *(uint64_t*)0x2002ef98 = (uint64_t)0x6;
    *(uint64_t*)0x2002efa0 = (uint64_t)0x0;
    *(uint64_t*)0x2002efa8 = (uint64_t)0x0;
    *(uint8_t*)0x2002efb0 = (uint8_t)0xd34;
    *(uint8_t*)0x2002efb1 = (uint8_t)0x200000000000000;
    *(uint8_t*)0x2002efb2 = (uint8_t)0x0;
    *(uint8_t*)0x2002efb3 = (uint8_t)0x0;
    *(uint32_t*)0x2002efb4 = (uint32_t)0x0;
    *(uint32_t*)0x2002efb8 = (uint32_t)0x0;
    *(uint32_t*)0x2002efbc = (uint32_t)0x0;
    *(uint64_t*)0x2002efc0 = (uint64_t)0x81;
    *(uint64_t*)0x2002efc8 = (uint64_t)0x0;
    *(uint64_t*)0x2002efd0 = (uint64_t)0x0;
    *(uint64_t*)0x2002efd8 = (uint64_t)0x0;
    *(uint64_t*)0x2002efe0 = (uint64_t)0x0;
    *(uint32_t*)0x2002efe8 = (uint32_t)0x7;
    *(uint64_t*)0x2002eff0 = (uint64_t)0x0;
    *(uint32_t*)0x2002eff8 = (uint32_t)0x0;
    *(uint16_t*)0x2002effc = (uint16_t)0x0;
    *(uint16_t*)0x2002effe = (uint16_t)0x0;
    r[31] = syscall(__NR_perf_event_open, 0x2002ef88ul, 0x0ul,
                    0xfffffffful, 0xfffffffffffffffful, 0x0ul);
    break;
  }
  return 0;
}

void test()
{
  long i;
  pthread_t th[8];

  memset(r, -1, sizeof(r));
  srand(getpid());
  for (i = 0; i < 4; i++) {
    pthread_create(&th[i], 0, thr, (void*)i);
    usleep(rand() % 10000);
  }
  for (i = 0; i < 4; i++) {
    pthread_create(&th[4 + i], 0, thr, (void*)i);
    if (rand() % 2)
      usleep(rand() % 10000);
  }
  usleep(rand() % 100000);
}

int main()
{
  int i;
  for (i = 0; i < 8; i++) {
    if (fork() == 0) {
      loop();
      return 0;
    }
  }
  sleep(1000000);
  return 0;
}
