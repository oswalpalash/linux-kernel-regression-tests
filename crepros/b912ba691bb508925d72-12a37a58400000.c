// https://syzkaller.appspot.com/bug?id=293f48c6a63935b5872fac5eafff89a15518864e
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <linux/futex.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
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

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();

  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = 160 << 20;
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 8 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 136 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);

  if (unshare(CLONE_NEWNS)) {
  }
  if (unshare(CLONE_NEWIPC)) {
  }
  if (unshare(0x02000000)) {
  }
  if (unshare(CLONE_NEWUTS)) {
  }
  if (unshare(CLONE_SYSVSEM)) {
  }
}

int wait_for_loop(int pid)
{
  if (pid < 0)
    fail("sandbox fork failed");
  int status = 0;
  while (waitpid(-1, &status, __WALL) != pid) {
  }
  return WEXITSTATUS(status);
}

static int do_sandbox_none(void)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid != 0)
    return wait_for_loop(pid);

  sandbox_common();
  if (unshare(CLONE_NEWNET)) {
  }
  loop();
  doexit(1);
}

static void execute_one();
extern unsigned long long procid;

void loop()
{
  while (1) {
    execute_one();
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
        if (__atomic_load_n(&running, __ATOMIC_RELAXED))
          usleep((call == num_calls - 1) ? 10000 : 1000);
        break;
      }
    }
  }
}

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    *(uint32_t*)0x20000200 = 2;
    *(uint32_t*)0x20000204 = 0x70;
    *(uint8_t*)0x20000208 = 0xe6;
    *(uint8_t*)0x20000209 = 0;
    *(uint8_t*)0x2000020a = 0;
    *(uint8_t*)0x2000020b = 0;
    *(uint32_t*)0x2000020c = 0;
    *(uint64_t*)0x20000210 = 0;
    *(uint64_t*)0x20000218 = 0;
    *(uint64_t*)0x20000220 = 0;
    STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 0, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 1, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 2, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 3, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 4, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 5, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 6, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 7, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 8, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 9, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 10, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 11, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 12, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 13, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 14, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 15, 2);
    STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 17, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 18, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 19, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 20, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 21, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 22, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 23, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 24, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 25, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 26, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 27, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 28, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 29, 35);
    *(uint32_t*)0x20000230 = 0;
    *(uint32_t*)0x20000234 = 0;
    *(uint64_t*)0x20000238 = 0;
    *(uint64_t*)0x20000240 = 0;
    *(uint64_t*)0x20000248 = 0;
    *(uint64_t*)0x20000250 = 0;
    *(uint32_t*)0x20000258 = 0;
    *(uint32_t*)0x2000025c = 0;
    *(uint64_t*)0x20000260 = 0;
    *(uint32_t*)0x20000268 = 0;
    *(uint16_t*)0x2000026c = 0;
    *(uint16_t*)0x2000026e = 0;
    syscall(__NR_perf_event_open, 0x20000200, 0, 0, -1, 0);
    break;
  case 1:
    *(uint32_t*)0x20000a40 = -1;
    syscall(__NR_setsockopt, -1, 0x29, 0x24, 0x20000a40, 0x173);
    break;
  case 2:
    res = syscall(__NR_socketpair, 0, 0, 0, 0x20000140);
    if (res != -1)
      r[0] = *(uint32_t*)0x20000140;
    break;
  case 3:
    *(uint32_t*)0x20000040 = r[0];
    syscall(__NR_ioctl, r[0], 0x800454e1, 0x20000040);
    break;
  case 4:
    syscall(__NR_socket, 0xa, 1, 0);
    break;
  case 5:
    *(uint32_t*)0x20000280 = 0x12;
    *(uint32_t*)0x20000284 = 9;
    *(uint32_t*)0x20000288 = 4;
    *(uint32_t*)0x2000028c = 1;
    *(uint32_t*)0x20000290 = 0;
    *(uint32_t*)0x20000294 = -1;
    *(uint32_t*)0x20000298 = 0;
    *(uint8_t*)0x2000029c = 0;
    *(uint8_t*)0x2000029d = 0;
    *(uint8_t*)0x2000029e = 0;
    *(uint8_t*)0x2000029f = 0;
    *(uint8_t*)0x200002a0 = 0;
    *(uint8_t*)0x200002a1 = 0;
    *(uint8_t*)0x200002a2 = 0;
    *(uint8_t*)0x200002a3 = 0;
    *(uint8_t*)0x200002a4 = 0;
    *(uint8_t*)0x200002a5 = 0;
    *(uint8_t*)0x200002a6 = 0;
    *(uint8_t*)0x200002a7 = 0;
    *(uint8_t*)0x200002a8 = 0;
    *(uint8_t*)0x200002a9 = 0;
    *(uint8_t*)0x200002aa = 0;
    *(uint8_t*)0x200002ab = 0;
    res = syscall(__NR_bpf, 0, 0x20000280, 0x34d);
    if (res != -1)
      r[1] = res;
    break;
  case 6:
    *(uint32_t*)0x20000000 = 0;
    *(uint32_t*)0x20000004 = 0;
    *(uint32_t*)0x20000008 = 4;
    *(uint32_t*)0x2000000c = 0;
    *(uint32_t*)0x20000010 = 0;
    *(uint32_t*)0x20000014 = -1;
    *(uint32_t*)0x20000018 = 0;
    *(uint8_t*)0x2000001c = 0;
    *(uint8_t*)0x2000001d = 0;
    *(uint8_t*)0x2000001e = 0;
    *(uint8_t*)0x2000001f = 0;
    *(uint8_t*)0x20000020 = 0;
    *(uint8_t*)0x20000021 = 0;
    *(uint8_t*)0x20000022 = 0;
    *(uint8_t*)0x20000023 = 0;
    *(uint8_t*)0x20000024 = 0;
    *(uint8_t*)0x20000025 = 0;
    *(uint8_t*)0x20000026 = 0;
    *(uint8_t*)0x20000027 = 0;
    *(uint8_t*)0x20000028 = 0;
    *(uint8_t*)0x20000029 = 0;
    *(uint8_t*)0x2000002a = 0;
    *(uint8_t*)0x2000002b = 0;
    syscall(__NR_bpf, 0, 0x20000000, 0xce);
    break;
  case 7:
    *(uint32_t*)0x20000180 = r[1];
    *(uint64_t*)0x20000188 = 0x20000000;
    *(uint64_t*)0x20000190 = 0x20000140;
    *(uint64_t*)0x20000198 = 0;
    syscall(__NR_bpf, 2, 0x20000180, 0x20);
    break;
  case 8:
    *(uint32_t*)0x20000180 = r[1];
    *(uint64_t*)0x20000188 = 0x20000080;
    *(uint64_t*)0x20000190 = 0x20000140;
    *(uint64_t*)0x20000198 = 0;
    syscall(__NR_bpf, 2, 0x20000180, 0x20);
    break;
  }
}

void execute_one()
{
  execute(9);
  collide = 1;
  execute(9);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    do_sandbox_none();
  }
}
