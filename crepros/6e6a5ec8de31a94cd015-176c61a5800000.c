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

long r[2];
void test()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xe7a000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 0xa, 2, 0);
  r[1] = syscall(__NR_socket, 0x18, 1, 1);
  NONFAILING(*(uint16_t*)0x20e71000 = 0x18);
  NONFAILING(*(uint32_t*)0x20e71002 = 1);
  NONFAILING(*(uint32_t*)0x20e71006 = 0);
  NONFAILING(*(uint32_t*)0x20e7100a = r[0]);
  NONFAILING(*(uint16_t*)0x20e7100e = 2);
  NONFAILING(*(uint16_t*)0x20e71010 = 0);
  NONFAILING(*(uint16_t*)0x20e71012 = 2);
  NONFAILING(*(uint16_t*)0x20e71014 = 0);
  NONFAILING(*(uint16_t*)0x20e71016 = 0xa);
  NONFAILING(*(uint16_t*)0x20e71018 = htobe16(0x4e21));
  NONFAILING(*(uint32_t*)0x20e7101a = 3);
  NONFAILING(*(uint8_t*)0x20e7101e = -1);
  NONFAILING(*(uint8_t*)0x20e7101f = 1);
  NONFAILING(*(uint8_t*)0x20e71020 = 0);
  NONFAILING(*(uint8_t*)0x20e71021 = 0);
  NONFAILING(*(uint8_t*)0x20e71022 = 0);
  NONFAILING(*(uint8_t*)0x20e71023 = 0);
  NONFAILING(*(uint8_t*)0x20e71024 = 0);
  NONFAILING(*(uint8_t*)0x20e71025 = 0);
  NONFAILING(*(uint8_t*)0x20e71026 = 0);
  NONFAILING(*(uint8_t*)0x20e71027 = 0);
  NONFAILING(*(uint8_t*)0x20e71028 = 0);
  NONFAILING(*(uint8_t*)0x20e71029 = 0);
  NONFAILING(*(uint8_t*)0x20e7102a = 0);
  NONFAILING(*(uint8_t*)0x20e7102b = 0);
  NONFAILING(*(uint8_t*)0x20e7102c = 0);
  NONFAILING(*(uint8_t*)0x20e7102d = 1);
  NONFAILING(*(uint32_t*)0x20e7102e = 4);
  syscall(__NR_connect, r[1], 0x20e71000, 0x32);
  NONFAILING(*(uint16_t*)0x206cafe4 = 0xa);
  NONFAILING(*(uint16_t*)0x206cafe6 = htobe16(0x4e22));
  NONFAILING(*(uint32_t*)0x206cafe8 = 1);
  NONFAILING(*(uint8_t*)0x206cafec = 0xfe);
  NONFAILING(*(uint8_t*)0x206cafed = 0x80);
  NONFAILING(*(uint8_t*)0x206cafee = 0);
  NONFAILING(*(uint8_t*)0x206cafef = 0);
  NONFAILING(*(uint8_t*)0x206caff0 = 0);
  NONFAILING(*(uint8_t*)0x206caff1 = 0);
  NONFAILING(*(uint8_t*)0x206caff2 = 0);
  NONFAILING(*(uint8_t*)0x206caff3 = 0);
  NONFAILING(*(uint8_t*)0x206caff4 = 0);
  NONFAILING(*(uint8_t*)0x206caff5 = 0);
  NONFAILING(*(uint8_t*)0x206caff6 = 0);
  NONFAILING(*(uint8_t*)0x206caff7 = 0);
  NONFAILING(*(uint8_t*)0x206caff8 = 0);
  NONFAILING(*(uint8_t*)0x206caff9 = 0);
  NONFAILING(*(uint8_t*)0x206caffa = 0);
  NONFAILING(*(uint8_t*)0x206caffb = 0xbb);
  NONFAILING(*(uint32_t*)0x206caffc = 1);
  syscall(__NR_sendto, r[0], 0x2033dfb1, 0, 0x4045, 0x206cafe4, 0x1c);
}

int main()
{
  install_segv_handler();
  for (;;) {
    loop();
  }
}
