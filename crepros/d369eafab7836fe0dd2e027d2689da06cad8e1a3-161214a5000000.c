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

long r[148];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    memcpy((void*)0x2001dff7, "\x2f\x64\x65\x76\x2f\x6b\x76\x6d\x00",
           9);
    r[2] = syscall(__NR_openat, 0xffffffffffffff9cul, 0x2001dff7ul,
                   0x1ul, 0x0ul);
    break;
  case 2:
    r[3] = syscall(__NR_ioctl, r[2], 0xae01ul, 0x0ul);
    break;
  case 3:
    r[4] = syscall(__NR_ioctl, r[3], 0xae41ul, 0x1ul);
    break;
  case 4:
    r[5] = syscall(__NR_ioctl, r[3], 0xae41ul, 0x0ul);
    break;
  case 5:
    *(uint32_t*)0x20168fb0 = (uint32_t)0x1;
    *(uint32_t*)0x20168fb4 = (uint32_t)0x0;
    *(uint32_t*)0x20168fb8 = (uint32_t)0x6;
    *(uint32_t*)0x20168fbc = (uint32_t)0x0;
    *(uint32_t*)0x20168fc0 = (uint32_t)0x7;
    *(uint8_t*)0x20168fc4 = (uint8_t)0x0;
    *(uint8_t*)0x20168fc5 = (uint8_t)0x0;
    *(uint8_t*)0x20168fc6 = (uint8_t)0x0;
    *(uint8_t*)0x20168fc7 = (uint8_t)0x0;
    *(uint8_t*)0x20168fc8 = (uint8_t)0x0;
    *(uint8_t*)0x20168fc9 = (uint8_t)0x0;
    *(uint8_t*)0x20168fca = (uint8_t)0x0;
    *(uint8_t*)0x20168fcb = (uint8_t)0x0;
    *(uint8_t*)0x20168fcc = (uint8_t)0x0;
    *(uint8_t*)0x20168fcd = (uint8_t)0x0;
    *(uint8_t*)0x20168fce = (uint8_t)0x0;
    *(uint8_t*)0x20168fcf = (uint8_t)0x0;
    *(uint8_t*)0x20168fd0 = (uint8_t)0x0;
    *(uint8_t*)0x20168fd1 = (uint8_t)0x0;
    *(uint8_t*)0x20168fd2 = (uint8_t)0x0;
    *(uint8_t*)0x20168fd3 = (uint8_t)0x0;
    *(uint8_t*)0x20168fd4 = (uint8_t)0x0;
    *(uint8_t*)0x20168fd5 = (uint8_t)0x0;
    *(uint8_t*)0x20168fd6 = (uint8_t)0x0;
    *(uint8_t*)0x20168fd7 = (uint8_t)0x0;
    *(uint8_t*)0x20168fd8 = (uint8_t)0x0;
    *(uint8_t*)0x20168fd9 = (uint8_t)0x0;
    *(uint8_t*)0x20168fda = (uint8_t)0x0;
    *(uint8_t*)0x20168fdb = (uint8_t)0x0;
    *(uint8_t*)0x20168fdc = (uint8_t)0x0;
    *(uint8_t*)0x20168fdd = (uint8_t)0x0;
    *(uint8_t*)0x20168fde = (uint8_t)0x0;
    *(uint8_t*)0x20168fdf = (uint8_t)0x0;
    *(uint8_t*)0x20168fe0 = (uint8_t)0x0;
    *(uint8_t*)0x20168fe1 = (uint8_t)0x0;
    *(uint8_t*)0x20168fe2 = (uint8_t)0x0;
    *(uint8_t*)0x20168fe3 = (uint8_t)0x0;
    *(uint8_t*)0x20168fe4 = (uint8_t)0x0;
    *(uint8_t*)0x20168fe5 = (uint8_t)0x0;
    *(uint8_t*)0x20168fe6 = (uint8_t)0x0;
    *(uint8_t*)0x20168fe7 = (uint8_t)0x0;
    *(uint8_t*)0x20168fe8 = (uint8_t)0x0;
    *(uint8_t*)0x20168fe9 = (uint8_t)0x0;
    *(uint8_t*)0x20168fea = (uint8_t)0x0;
    *(uint8_t*)0x20168feb = (uint8_t)0x0;
    *(uint8_t*)0x20168fec = (uint8_t)0x0;
    *(uint8_t*)0x20168fed = (uint8_t)0x0;
    *(uint8_t*)0x20168fee = (uint8_t)0x0;
    *(uint8_t*)0x20168fef = (uint8_t)0x0;
    *(uint8_t*)0x20168ff0 = (uint8_t)0x0;
    *(uint8_t*)0x20168ff1 = (uint8_t)0x0;
    *(uint8_t*)0x20168ff2 = (uint8_t)0x0;
    *(uint8_t*)0x20168ff3 = (uint8_t)0x0;
    *(uint8_t*)0x20168ff4 = (uint8_t)0x0;
    *(uint8_t*)0x20168ff5 = (uint8_t)0x0;
    *(uint8_t*)0x20168ff6 = (uint8_t)0x0;
    *(uint8_t*)0x20168ff7 = (uint8_t)0x0;
    *(uint8_t*)0x20168ff8 = (uint8_t)0x0;
    *(uint8_t*)0x20168ff9 = (uint8_t)0x0;
    *(uint8_t*)0x20168ffa = (uint8_t)0x0;
    *(uint8_t*)0x20168ffb = (uint8_t)0x0;
    *(uint8_t*)0x20168ffc = (uint8_t)0x0;
    *(uint8_t*)0x20168ffd = (uint8_t)0x0;
    *(uint8_t*)0x20168ffe = (uint8_t)0x0;
    *(uint8_t*)0x20168fff = (uint8_t)0x0;
    r[71] = syscall(__NR_ioctl, 0xfffffffffffffffful, 0x40505412ul,
                    0x20168fb0ul);
    break;
  case 6:
    r[72] = syscall(__NR_ioctl, r[5], 0x9000aea4ul, 0x209b0c00ul);
    break;
  case 7:
    *(uint32_t*)0x20415000 = (uint32_t)0x1;
    r[74] = syscall(__NR_ioctl, 0xfffffffffffffffful, 0x40045402ul,
                    0x20415000ul);
    break;
  case 8:
    *(uint8_t*)0x201f7000 = (uint8_t)0x80;
    *(uint8_t*)0x201f7001 = (uint8_t)0x2;
    memcpy((void*)0x201f7002,
           "\x70\x6f\x72\x74\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00",
           64);
    *(uint32_t*)0x201f7044 = (uint32_t)0x4;
    *(uint32_t*)0x201f7048 = (uint32_t)0x0;
    *(uint32_t*)0x201f704c = (uint32_t)0x0;
    *(uint32_t*)0x201f7050 = (uint32_t)0x400;
    *(uint32_t*)0x201f7054 = (uint32_t)0x7;
    *(uint32_t*)0x201f7058 = (uint32_t)0xffffffff;
    *(uint32_t*)0x201f705c = (uint32_t)0x9;
    *(uint64_t*)0x201f7060 = (uint64_t)0x0;
    *(uint32_t*)0x201f7068 = (uint32_t)0x0;
    *(uint32_t*)0x201f706c = (uint32_t)0x1;
    *(uint8_t*)0x201f7070 = (uint8_t)0x0;
    *(uint8_t*)0x201f7071 = (uint8_t)0x0;
    *(uint8_t*)0x201f7072 = (uint8_t)0x0;
    *(uint8_t*)0x201f7073 = (uint8_t)0x0;
    *(uint8_t*)0x201f7074 = (uint8_t)0x0;
    *(uint8_t*)0x201f7075 = (uint8_t)0x0;
    *(uint8_t*)0x201f7076 = (uint8_t)0x0;
    *(uint8_t*)0x201f7077 = (uint8_t)0x0;
    *(uint8_t*)0x201f7078 = (uint8_t)0x0;
    *(uint8_t*)0x201f7079 = (uint8_t)0x0;
    *(uint8_t*)0x201f707a = (uint8_t)0x0;
    *(uint8_t*)0x201f707b = (uint8_t)0x0;
    *(uint8_t*)0x201f707c = (uint8_t)0x0;
    *(uint8_t*)0x201f707d = (uint8_t)0x0;
    *(uint8_t*)0x201f707e = (uint8_t)0x0;
    *(uint8_t*)0x201f707f = (uint8_t)0x0;
    *(uint8_t*)0x201f7080 = (uint8_t)0x0;
    *(uint8_t*)0x201f7081 = (uint8_t)0x0;
    *(uint8_t*)0x201f7082 = (uint8_t)0x0;
    *(uint8_t*)0x201f7083 = (uint8_t)0x0;
    *(uint8_t*)0x201f7084 = (uint8_t)0x0;
    *(uint8_t*)0x201f7085 = (uint8_t)0x0;
    *(uint8_t*)0x201f7086 = (uint8_t)0x0;
    *(uint8_t*)0x201f7087 = (uint8_t)0x0;
    *(uint8_t*)0x201f7088 = (uint8_t)0x0;
    *(uint8_t*)0x201f7089 = (uint8_t)0x0;
    *(uint8_t*)0x201f708a = (uint8_t)0x0;
    *(uint8_t*)0x201f708b = (uint8_t)0x0;
    *(uint8_t*)0x201f708c = (uint8_t)0x0;
    *(uint8_t*)0x201f708d = (uint8_t)0x0;
    *(uint8_t*)0x201f708e = (uint8_t)0x0;
    *(uint8_t*)0x201f708f = (uint8_t)0x0;
    *(uint8_t*)0x201f7090 = (uint8_t)0x0;
    *(uint8_t*)0x201f7091 = (uint8_t)0x0;
    *(uint8_t*)0x201f7092 = (uint8_t)0x0;
    *(uint8_t*)0x201f7093 = (uint8_t)0x0;
    *(uint8_t*)0x201f7094 = (uint8_t)0x0;
    *(uint8_t*)0x201f7095 = (uint8_t)0x0;
    *(uint8_t*)0x201f7096 = (uint8_t)0x0;
    *(uint8_t*)0x201f7097 = (uint8_t)0x0;
    *(uint8_t*)0x201f7098 = (uint8_t)0x0;
    *(uint8_t*)0x201f7099 = (uint8_t)0x0;
    *(uint8_t*)0x201f709a = (uint8_t)0x0;
    *(uint8_t*)0x201f709b = (uint8_t)0x0;
    *(uint8_t*)0x201f709c = (uint8_t)0x0;
    *(uint8_t*)0x201f709d = (uint8_t)0x0;
    *(uint8_t*)0x201f709e = (uint8_t)0x0;
    *(uint8_t*)0x201f709f = (uint8_t)0x0;
    *(uint8_t*)0x201f70a0 = (uint8_t)0x0;
    *(uint8_t*)0x201f70a1 = (uint8_t)0x0;
    *(uint8_t*)0x201f70a2 = (uint8_t)0x0;
    *(uint8_t*)0x201f70a3 = (uint8_t)0x0;
    *(uint8_t*)0x201f70a4 = (uint8_t)0x0;
    *(uint8_t*)0x201f70a5 = (uint8_t)0x0;
    *(uint8_t*)0x201f70a6 = (uint8_t)0x0;
    *(uint8_t*)0x201f70a7 = (uint8_t)0x0;
    *(uint8_t*)0x201f70a8 = (uint8_t)0x0;
    *(uint8_t*)0x201f70a9 = (uint8_t)0x0;
    *(uint8_t*)0x201f70aa = (uint8_t)0x0;
    r[147] = syscall(__NR_ioctl, 0xfffffffffffffffful, 0x40a85323ul,
                     0x201f7000ul);
    break;
  }
  return 0;
}

void test()
{
  long i;
  pthread_t th[18];

  memset(r, -1, sizeof(r));
  srand(getpid());
  for (i = 0; i < 9; i++) {
    pthread_create(&th[i], 0, thr, (void*)i);
    usleep(rand() % 10000);
  }
  for (i = 0; i < 9; i++) {
    pthread_create(&th[9 + i], 0, thr, (void*)i);
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
