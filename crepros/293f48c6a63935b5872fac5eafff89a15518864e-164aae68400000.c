// https://syzkaller.appspot.com/bug?id=293f48c6a63935b5872fac5eafff89a15518864e
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <linux/futex.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/syscall.h>
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

uint64_t r[1] = {0xffffffffffffffff};
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    syscall(__NR_socketpair, 0, 0, 0, 0x20000140);
    break;
  case 1:
    syscall(__NR_socket, 0xa, 1, 0);
    break;
  case 2:
    NONFAILING(*(uint32_t*)0x20000280 = 0x12);
    NONFAILING(*(uint32_t*)0x20000284 = 0);
    NONFAILING(*(uint32_t*)0x20000288 = 4);
    NONFAILING(*(uint32_t*)0x2000028c = 1);
    NONFAILING(*(uint32_t*)0x20000290 = 0);
    NONFAILING(*(uint32_t*)0x20000294 = 1);
    NONFAILING(*(uint32_t*)0x20000298 = 0);
    NONFAILING(*(uint8_t*)0x2000029c = 0);
    NONFAILING(*(uint8_t*)0x2000029d = 0);
    NONFAILING(*(uint8_t*)0x2000029e = 0);
    NONFAILING(*(uint8_t*)0x2000029f = 0);
    NONFAILING(*(uint8_t*)0x200002a0 = 0);
    NONFAILING(*(uint8_t*)0x200002a1 = 0);
    NONFAILING(*(uint8_t*)0x200002a2 = 0);
    NONFAILING(*(uint8_t*)0x200002a3 = 0);
    NONFAILING(*(uint8_t*)0x200002a4 = 0);
    NONFAILING(*(uint8_t*)0x200002a5 = 0);
    NONFAILING(*(uint8_t*)0x200002a6 = 0);
    NONFAILING(*(uint8_t*)0x200002a7 = 0);
    NONFAILING(*(uint8_t*)0x200002a8 = 0);
    NONFAILING(*(uint8_t*)0x200002a9 = 0);
    NONFAILING(*(uint8_t*)0x200002aa = 0);
    NONFAILING(*(uint8_t*)0x200002ab = 0);
    res = syscall(__NR_bpf, 0, 0x20000280, 0x2c);
    if (res != -1)
      r[0] = res;
    break;
  case 3:
    NONFAILING(*(uint32_t*)0x20000180 = r[0]);
    NONFAILING(*(uint64_t*)0x20000188 = 0x20000000);
    NONFAILING(*(uint64_t*)0x20000190 = 0x20000140);
    NONFAILING(*(uint64_t*)0x20000198 = 0);
    syscall(__NR_bpf, 2, 0x20000180, 0x20);
    break;
  case 4:
    NONFAILING(*(uint32_t*)0x20000180 = r[0]);
    NONFAILING(*(uint64_t*)0x20000188 = 0x20000080);
    NONFAILING(*(uint64_t*)0x20000190 = 0x20000140);
    NONFAILING(*(uint64_t*)0x20000198 = 0);
    syscall(__NR_bpf, 2, 0x20000180, 0x20);
    break;
  }
}

void execute_one()
{
  execute(5);
  collide = 1;
  execute(5);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  install_segv_handler();
  for (;;) {
    loop();
  }
}
