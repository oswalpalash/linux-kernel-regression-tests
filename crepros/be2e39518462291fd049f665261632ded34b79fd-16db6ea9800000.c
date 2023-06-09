// https://syzkaller.appspot.com/bug?id=be2e39518462291fd049f665261632ded34b79fd
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
#include <setjmp.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>
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

static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* uctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  if (__atomic_load_n(&skip_segv, __ATOMIC_RELAXED) &&
      (addr < prog_start || addr > prog_end)) {
    _longjmp(segv_env, 1);
  }
  doexit(sig);
}

static void install_segv_handler()
{
  struct sigaction sa;

  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_IGN;
  syscall(SYS_rt_sigaction, 0x20, &sa, NULL, 8);
  syscall(SYS_rt_sigaction, 0x21, &sa, NULL, 8);

  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = segv_handler;
  sa.sa_flags = SA_NODEFER | SA_SIGINFO;
  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGBUS, &sa, NULL);
}

#define NONFAILING(...)                                                        \
  {                                                                            \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    if (_setjmp(segv_env) == 0) {                                              \
      __VA_ARGS__;                                                             \
    }                                                                          \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
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
uint64_t procid;
void test()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  NONFAILING(*(uint16_t*)0x20002000 = 0xa);
  NONFAILING(*(uint16_t*)0x20002002 = htobe16(0x4e20 + procid * 4));
  NONFAILING(*(uint32_t*)0x20002004 = 0);
  NONFAILING(*(uint8_t*)0x20002008 = 0xfe);
  NONFAILING(*(uint8_t*)0x20002009 = 0x80);
  NONFAILING(*(uint8_t*)0x2000200a = 0);
  NONFAILING(*(uint8_t*)0x2000200b = 0);
  NONFAILING(*(uint8_t*)0x2000200c = 0);
  NONFAILING(*(uint8_t*)0x2000200d = 0);
  NONFAILING(*(uint8_t*)0x2000200e = 0);
  NONFAILING(*(uint8_t*)0x2000200f = 0);
  NONFAILING(*(uint8_t*)0x20002010 = 0);
  NONFAILING(*(uint8_t*)0x20002011 = 0);
  NONFAILING(*(uint8_t*)0x20002012 = 0);
  NONFAILING(*(uint8_t*)0x20002013 = 0);
  NONFAILING(*(uint8_t*)0x20002014 = 0);
  NONFAILING(*(uint8_t*)0x20002015 = 0);
  NONFAILING(*(uint8_t*)0x20002016 = 0 + procid * 1);
  NONFAILING(*(uint8_t*)0x20002017 = 0xaa);
  NONFAILING(*(uint32_t*)0x20002018 = 0);
  NONFAILING(*(uint16_t*)0x2000201c = 0xa);
  NONFAILING(*(uint16_t*)0x2000201e = htobe16(0x4e20 + procid * 4));
  NONFAILING(*(uint32_t*)0x20002020 = 0);
  NONFAILING(*(uint8_t*)0x20002024 = 0);
  NONFAILING(*(uint8_t*)0x20002025 = 0);
  NONFAILING(*(uint8_t*)0x20002026 = 0);
  NONFAILING(*(uint8_t*)0x20002027 = 0);
  NONFAILING(*(uint8_t*)0x20002028 = 0);
  NONFAILING(*(uint8_t*)0x20002029 = 0);
  NONFAILING(*(uint8_t*)0x2000202a = 0);
  NONFAILING(*(uint8_t*)0x2000202b = 0);
  NONFAILING(*(uint8_t*)0x2000202c = 0);
  NONFAILING(*(uint8_t*)0x2000202d = 0);
  NONFAILING(*(uint8_t*)0x2000202e = 0);
  NONFAILING(*(uint8_t*)0x2000202f = 0);
  NONFAILING(*(uint8_t*)0x20002030 = 0);
  NONFAILING(*(uint8_t*)0x20002031 = 0);
  NONFAILING(*(uint8_t*)0x20002032 = 0);
  NONFAILING(*(uint8_t*)0x20002033 = 0);
  NONFAILING(*(uint32_t*)0x20002034 = 1);
  NONFAILING(*(uint16_t*)0x20002038 = 0);
  NONFAILING(*(uint32_t*)0x2000203c = 0);
  NONFAILING(*(uint32_t*)0x20002040 = 0);
  NONFAILING(*(uint32_t*)0x20002044 = 0x7fff);
  NONFAILING(*(uint32_t*)0x20002048 = 0);
  NONFAILING(*(uint32_t*)0x2000204c = 0);
  NONFAILING(*(uint32_t*)0x20002050 = 0);
  NONFAILING(*(uint32_t*)0x20002054 = 0);
  NONFAILING(*(uint32_t*)0x20002058 = 0);
  syscall(__NR_setsockopt, -1, 0x29, 0xd3, 0x20002000, 0x5c);
  r[0] = syscall(__NR_socket, 0xa, 1, 0);
  NONFAILING(*(uint32_t*)0x20001fde = 0);
  syscall(__NR_setsockopt, r[0], 0x29, 0x40, 0x20001fde, 4);
}

int main()
{
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      install_segv_handler();
      for (;;) {
        loop();
      }
    }
  }
  sleep(1000000);
  return 0;
}
