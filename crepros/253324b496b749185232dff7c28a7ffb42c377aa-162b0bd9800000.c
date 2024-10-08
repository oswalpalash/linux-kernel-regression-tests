// https://syzkaller.appspot.com/bug?id=253324b496b749185232dff7c28a7ffb42c377aa
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <linux/futex.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
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

#define BITMASK_LEN(type, bf_len) (type)((1ull << (bf_len)) - 1)

#define BITMASK_LEN_OFF(type, bf_off, bf_len)                                  \
  (type)(BITMASK_LEN(type, (bf_len)) << (bf_off))

#define STORE_BY_BITMASK(type, addr, val, bf_off, bf_len)                      \
  if ((bf_off) == 0 && (bf_len) == 0) {                                        \
    *(type*)(addr) = (type)(val);                                              \
  } else {                                                                     \
    type new_val = *(type*)(addr);                                             \
    new_val &= ~BITMASK_LEN_OFF(type, (bf_off), (bf_len));                     \
    new_val |= ((type)(val)&BITMASK_LEN(type, (bf_len))) << (bf_off);          \
    *(type*)(addr) = new_val;                                                  \
  }

struct csum_inet {
  uint32_t acc;
};

static void csum_inet_init(struct csum_inet* csum)
{
  csum->acc = 0;
}

static void csum_inet_update(struct csum_inet* csum, const uint8_t* data,
                             size_t length)
{
  if (length == 0)
    return;

  size_t i;
  for (i = 0; i < length - 1; i += 2)
    csum->acc += *(uint16_t*)&data[i];

  if (length & 1)
    csum->acc += (uint16_t)data[length - 1];

  while (csum->acc > 0xffff)
    csum->acc = (csum->acc & 0xffff) + (csum->acc >> 16);
}

static uint16_t csum_inet_digest(struct csum_inet* csum)
{
  return ~csum->acc;
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

struct thread_t {
  int created, running, call;
  pthread_t th;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;
static int collide;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    while (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE))
      syscall(SYS_futex, &th->running, FUTEX_WAIT, 0, 0);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    __atomic_store_n(&th->running, 0, __ATOMIC_RELEASE);
    syscall(SYS_futex, &th->running, FUTEX_WAKE);
  }
  return 0;
}

static void execute(int num_calls)
{
  int call, thread;
  running = 0;
  for (call = 0; call < num_calls; call++) {
    for (thread = 0; thread < sizeof(threads) / sizeof(threads[0]); thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 128 << 10);
        pthread_create(&th->th, &attr, thr, th);
      }
      if (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE)) {
        th->call = call;
        __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
        __atomic_store_n(&th->running, 1, __ATOMIC_RELEASE);
        syscall(SYS_futex, &th->running, FUTEX_WAKE);
        if (collide && call % 2)
          break;
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 20 * 1000 * 1000;
        syscall(SYS_futex, &th->running, FUTEX_WAIT, 1, &ts);
        if (running)
          usleep((call == num_calls - 1) ? 10000 : 1000);
        break;
      }
    }
  }
}

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

long r[3];
uint64_t procid;
void execute_call(int call)
{
  switch (call) {
  case 0:
    syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
    break;
  case 1:
    r[0] = syscall(__NR_socket, 2, 1, 0);
    break;
  case 2:
    NONFAILING(*(uint16_t*)0x20e5b000 = 2);
    NONFAILING(*(uint16_t*)0x20e5b002 = htobe16(0x4e20 + procid * 4));
    NONFAILING(*(uint32_t*)0x20e5b004 = htobe32(0xe0000001));
    NONFAILING(*(uint8_t*)0x20e5b008 = 0);
    NONFAILING(*(uint8_t*)0x20e5b009 = 0);
    NONFAILING(*(uint8_t*)0x20e5b00a = 0);
    NONFAILING(*(uint8_t*)0x20e5b00b = 0);
    NONFAILING(*(uint8_t*)0x20e5b00c = 0);
    NONFAILING(*(uint8_t*)0x20e5b00d = 0);
    NONFAILING(*(uint8_t*)0x20e5b00e = 0);
    NONFAILING(*(uint8_t*)0x20e5b00f = 0);
    syscall(__NR_bind, r[0], 0x20e5b000, 0x10);
    break;
  case 3:
    NONFAILING(*(uint16_t*)0x20ccb000 = 2);
    NONFAILING(*(uint16_t*)0x20ccb002 = htobe16(0x4e20 + procid * 4));
    NONFAILING(*(uint32_t*)0x20ccb004 = htobe32(0));
    NONFAILING(*(uint8_t*)0x20ccb008 = 0);
    NONFAILING(*(uint8_t*)0x20ccb009 = 0);
    NONFAILING(*(uint8_t*)0x20ccb00a = 0);
    NONFAILING(*(uint8_t*)0x20ccb00b = 0);
    NONFAILING(*(uint8_t*)0x20ccb00c = 0);
    NONFAILING(*(uint8_t*)0x20ccb00d = 0);
    NONFAILING(*(uint8_t*)0x20ccb00e = 0);
    NONFAILING(*(uint8_t*)0x20ccb00f = 0);
    syscall(__NR_connect, r[0], 0x20ccb000, 0x10);
    break;
  case 4:
    NONFAILING(*(uint32_t*)0x20eb0fb8 = 1);
    NONFAILING(*(uint32_t*)0x20eb0fbc = 3);
    NONFAILING(*(uint64_t*)0x20eb0fc0 = 0x209ff000);
    NONFAILING(*(uint64_t*)0x20eb0fc8 = 0x202bf000);
    NONFAILING(*(uint32_t*)0x20eb0fd0 = 4);
    NONFAILING(*(uint32_t*)0x20eb0fd4 = 0xb7);
    NONFAILING(*(uint64_t*)0x20eb0fd8 = 0x206ab000);
    NONFAILING(*(uint32_t*)0x20eb0fe0 = 0);
    NONFAILING(*(uint32_t*)0x20eb0fe4 = 0);
    NONFAILING(*(uint8_t*)0x20eb0fe8 = 0);
    NONFAILING(*(uint8_t*)0x20eb0fe9 = 0);
    NONFAILING(*(uint8_t*)0x20eb0fea = 0);
    NONFAILING(*(uint8_t*)0x20eb0feb = 0);
    NONFAILING(*(uint8_t*)0x20eb0fec = 0);
    NONFAILING(*(uint8_t*)0x20eb0fed = 0);
    NONFAILING(*(uint8_t*)0x20eb0fee = 0);
    NONFAILING(*(uint8_t*)0x20eb0fef = 0);
    NONFAILING(*(uint8_t*)0x20eb0ff0 = 0);
    NONFAILING(*(uint8_t*)0x20eb0ff1 = 0);
    NONFAILING(*(uint8_t*)0x20eb0ff2 = 0);
    NONFAILING(*(uint8_t*)0x20eb0ff3 = 0);
    NONFAILING(*(uint8_t*)0x20eb0ff4 = 0);
    NONFAILING(*(uint8_t*)0x20eb0ff5 = 0);
    NONFAILING(*(uint8_t*)0x20eb0ff6 = 0);
    NONFAILING(*(uint8_t*)0x20eb0ff7 = 0);
    NONFAILING(*(uint32_t*)0x20eb0ff8 = 0);
    NONFAILING(*(uint8_t*)0x209ff000 = 0x18);
    NONFAILING(STORE_BY_BITMASK(uint8_t, 0x209ff001, 0, 0, 4));
    NONFAILING(STORE_BY_BITMASK(uint8_t, 0x209ff001, 0, 4, 4));
    NONFAILING(*(uint16_t*)0x209ff002 = 0);
    NONFAILING(*(uint32_t*)0x209ff004 = 3);
    NONFAILING(*(uint8_t*)0x209ff008 = 0);
    NONFAILING(*(uint8_t*)0x209ff009 = 0);
    NONFAILING(*(uint16_t*)0x209ff00a = 0);
    NONFAILING(*(uint32_t*)0x209ff00c = 0);
    NONFAILING(*(uint8_t*)0x209ff010 = 0x95);
    NONFAILING(*(uint8_t*)0x209ff011 = 0);
    NONFAILING(*(uint16_t*)0x209ff012 = 0);
    NONFAILING(*(uint32_t*)0x209ff014 = 0);
    NONFAILING(memcpy((void*)0x202bf000, "syzkaller", 10));
    r[1] = syscall(__NR_bpf, 5, 0x20eb0fb8, 0x48);
    break;
  case 5:
    r[2] = syscall(__NR_socket, 0x29, 0x1000000000000005, 0);
    break;
  case 6:
    NONFAILING(*(uint32_t*)0x2031aff8 = r[0]);
    NONFAILING(*(uint32_t*)0x2031affc = r[1]);
    syscall(__NR_ioctl, r[2], 0x89e0, 0x2031aff8);
    break;
  case 7:
    NONFAILING(*(uint16_t*)0x209eaff0 = 4);
    NONFAILING(*(uint16_t*)0x209eaff2 = htobe16(0));
    NONFAILING(*(uint32_t*)0x209eaff4 = htobe32(0));
    NONFAILING(memcpy((void*)0x209eaff8, "\xa8\x64\x54\xf4\xc0\x10", 6));
    NONFAILING(*(uint8_t*)0x209eaffe = 0);
    NONFAILING(*(uint8_t*)0x209eafff = 0);
    syscall(__NR_recvfrom, r[2], 0x2043ef08, 0xf8, 2, 0x209eaff0, 0x10);
    break;
  case 8:
    NONFAILING(*(uint8_t*)0x20800000 = 1);
    NONFAILING(*(uint8_t*)0x20800001 = 0x80);
    NONFAILING(*(uint8_t*)0x20800002 = 0xc2);
    NONFAILING(*(uint8_t*)0x20800003 = 0);
    NONFAILING(*(uint8_t*)0x20800004 = 0);
    NONFAILING(*(uint8_t*)0x20800005 = 0);
    NONFAILING(*(uint8_t*)0x20800006 = 0xaa);
    NONFAILING(*(uint8_t*)0x20800007 = 0xaa);
    NONFAILING(*(uint8_t*)0x20800008 = 0xaa);
    NONFAILING(*(uint8_t*)0x20800009 = 0xaa);
    NONFAILING(*(uint8_t*)0x2080000a = 0);
    NONFAILING(*(uint8_t*)0x2080000b = 0xaa);
    NONFAILING(*(uint16_t*)0x2080000c = htobe16(0x800));
    NONFAILING(STORE_BY_BITMASK(uint8_t, 0x2080000e, 5, 0, 4));
    NONFAILING(STORE_BY_BITMASK(uint8_t, 0x2080000e, 4, 4, 4));
    NONFAILING(STORE_BY_BITMASK(uint8_t, 0x2080000f, 0, 0, 2));
    NONFAILING(STORE_BY_BITMASK(uint8_t, 0x2080000f, 0, 2, 6));
    NONFAILING(*(uint16_t*)0x20800010 = htobe16(0x30));
    NONFAILING(*(uint16_t*)0x20800012 = 0);
    NONFAILING(*(uint16_t*)0x20800014 = htobe16(0));
    NONFAILING(*(uint8_t*)0x20800016 = 0);
    NONFAILING(*(uint8_t*)0x20800017 = 1);
    NONFAILING(*(uint16_t*)0x20800018 = 0);
    NONFAILING(*(uint8_t*)0x2080001a = 0xac);
    NONFAILING(*(uint8_t*)0x2080001b = 0x14);
    NONFAILING(*(uint8_t*)0x2080001c = 0);
    NONFAILING(*(uint8_t*)0x2080001d = 0xbb);
    NONFAILING(*(uint8_t*)0x2080001e = 0xac);
    NONFAILING(*(uint8_t*)0x2080001f = 0x14);
    NONFAILING(*(uint8_t*)0x20800020 = 0 + procid * 1);
    NONFAILING(*(uint8_t*)0x20800021 = 0xf);
    NONFAILING(*(uint8_t*)0x20800022 = 3);
    NONFAILING(*(uint8_t*)0x20800023 = 0);
    NONFAILING(*(uint16_t*)0x20800024 = 0);
    NONFAILING(*(uint8_t*)0x20800026 = 0);
    NONFAILING(*(uint8_t*)0x20800027 = 0);
    NONFAILING(*(uint16_t*)0x20800028 = htobe16(0));
    NONFAILING(STORE_BY_BITMASK(uint8_t, 0x2080002a, 5, 0, 4));
    NONFAILING(STORE_BY_BITMASK(uint8_t, 0x2080002a, 4, 4, 4));
    NONFAILING(STORE_BY_BITMASK(uint8_t, 0x2080002b, 0, 0, 2));
    NONFAILING(STORE_BY_BITMASK(uint8_t, 0x2080002b, 0, 2, 6));
    NONFAILING(*(uint16_t*)0x2080002c = htobe16(0));
    NONFAILING(*(uint16_t*)0x2080002e = 0);
    NONFAILING(*(uint16_t*)0x20800030 = htobe16(0));
    NONFAILING(*(uint8_t*)0x20800032 = 0);
    NONFAILING(*(uint8_t*)0x20800033 = 1);
    NONFAILING(*(uint16_t*)0x20800034 = htobe16(0));
    NONFAILING(*(uint32_t*)0x20800036 = htobe32(0));
    NONFAILING(*(uint8_t*)0x2080003a = 0xac);
    NONFAILING(*(uint8_t*)0x2080003b = 0x14);
    NONFAILING(*(uint8_t*)0x2080003c = 0);
    NONFAILING(*(uint8_t*)0x2080003d = 0xbb);
    NONFAILING(*(uint32_t*)0x2048fff4 = 0);
    NONFAILING(*(uint32_t*)0x2048fff8 = 1);
    NONFAILING(*(uint32_t*)0x2048fffc = 0);
    struct csum_inet csum_1;
    csum_inet_init(&csum_1);
    NONFAILING(csum_inet_update(&csum_1, (const uint8_t*)0x20800022, 28));
    NONFAILING(*(uint16_t*)0x20800024 = csum_inet_digest(&csum_1));
    struct csum_inet csum_2;
    csum_inet_init(&csum_2);
    NONFAILING(csum_inet_update(&csum_2, (const uint8_t*)0x2080000e, 20));
    NONFAILING(*(uint16_t*)0x20800018 = csum_inet_digest(&csum_2));
    break;
  }
}

void test()
{
  memset(r, -1, sizeof(r));
  execute(9);
  collide = 1;
  execute(9);
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
