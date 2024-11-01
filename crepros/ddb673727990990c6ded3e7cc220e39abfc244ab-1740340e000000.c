// https://syzkaller.appspot.com/bug?id=ddb673727990990c6ded3e7cc220e39abfc244ab
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/stat.h>
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

static uintptr_t syz_open_dev(uintptr_t a0, uintptr_t a1, uintptr_t a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block",
            (uint8_t)a1, (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf));
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
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

long r[45];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    *(uint32_t*)0x20c7e000 = (uint32_t)0x58;
    r[2] = syscall(__NR_accept, 0xffffffffffffff9cul, 0x20792000ul,
                   0x20c7e000ul);
    break;
  case 2:
    *(uint32_t*)0x20000000 = (uint32_t)0x0;
    *(uint32_t*)0x20000004 = (uint32_t)0xa94;
    *(uint32_t*)0x20000008 = (uint32_t)0x800;
    *(uint32_t*)0x2000000c = (uint32_t)0x0;
    *(uint32_t*)0x20000010 = (uint32_t)0xa;
    *(uint64_t*)0x20000018 = (uint64_t)0x0;
    r[9] = syscall(__NR_ioctl, 0xfffffffffffffffful, 0xc0206416ul,
                   0x20000000ul);
    break;
  case 3:
    *(uint32_t*)0x2001d000 = (uint32_t)0x2;
    *(uint32_t*)0x2001d004 = (uint32_t)0x78;
    *(uint8_t*)0x2001d008 = (uint8_t)0xd4e9;
    *(uint8_t*)0x2001d009 = (uint8_t)0x0;
    *(uint8_t*)0x2001d00a = (uint8_t)0x0;
    *(uint8_t*)0x2001d00b = (uint8_t)0x0;
    *(uint32_t*)0x2001d00c = (uint32_t)0x0;
    *(uint64_t*)0x2001d010 = (uint64_t)0x0;
    *(uint64_t*)0x2001d018 = (uint64_t)0x0;
    *(uint64_t*)0x2001d020 = (uint64_t)0x0;
    *(uint8_t*)0x2001d028 = (uint8_t)0xfe;
    *(uint8_t*)0x2001d029 = (uint8_t)0x0;
    *(uint8_t*)0x2001d02a = (uint8_t)0x0;
    *(uint8_t*)0x2001d02b = (uint8_t)0x0;
    *(uint32_t*)0x2001d02c = (uint32_t)0x0;
    *(uint32_t*)0x2001d030 = (uint32_t)0x0;
    *(uint32_t*)0x2001d034 = (uint32_t)0x0;
    *(uint64_t*)0x2001d038 = (uint64_t)0x0;
    *(uint64_t*)0x2001d040 = (uint64_t)0x0;
    *(uint64_t*)0x2001d048 = (uint64_t)0x0;
    *(uint64_t*)0x2001d050 = (uint64_t)0x0;
    *(uint64_t*)0x2001d058 = (uint64_t)0x0;
    *(uint32_t*)0x2001d060 = (uint32_t)0x0;
    *(uint64_t*)0x2001d068 = (uint64_t)0x0;
    *(uint32_t*)0x2001d070 = (uint32_t)0x0;
    *(uint16_t*)0x2001d074 = (uint16_t)0x0;
    *(uint16_t*)0x2001d076 = (uint16_t)0x0;
    r[37] = syscall(__NR_perf_event_open, 0x2001d000ul, 0x0ul,
                    0xfffffffffffffffful, 0xfffffffffffffffful, 0x0ul);
    break;
  case 4:
    memcpy((void*)0x200bf000,
           "\x2f\x64\x65\x76\x2f\x6e\x65\x74\x2f\x74\x75\x6e\x00", 13);
    r[39] = syz_open_dev(0x200bf000ul, 0x0ul, 0x0ul);
    break;
  case 5:
    memcpy((void*)0x20521fd8, "\x30\x60\x9c\x02\x00\x00\x00\x00\x00\x00"
                              "\x5c\x7d\x00\xe9\xbd\x06",
           16);
    *(uint32_t*)0x20521fe8 = (uint32_t)0x1;
    r[42] = syscall(__NR_ioctl, r[39], 0x400454caul, 0x20521fd8ul);
    break;
  case 6:
    *(uint32_t*)0x20190ffc = (uint32_t)0x2;
    r[44] = syscall(__NR_ioctl, r[39], 0x400454daul, 0x20190ffcul);
    break;
  }
  return 0;
}

void test()
{
  long i;
  pthread_t th[14];

  memset(r, -1, sizeof(r));
  for (i = 0; i < 7; i++) {
    pthread_create(&th[i], 0, thr, (void*)i);
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
