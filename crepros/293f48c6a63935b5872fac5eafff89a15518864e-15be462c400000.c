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

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0x0,
                 0xffffffffffffffff};
unsigned long long procid;
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    res = syscall(__NR_socketpair, 0xa, 2, 0x88, 0x20000040);
    if (res != -1)
      NONFAILING(r[0] = *(uint32_t*)0x20000040);
    break;
  case 1:
    NONFAILING(*(uint32_t*)0x20000200 = 0x10);
    NONFAILING(*(uint32_t*)0x20000204 = 8);
    NONFAILING(*(uint64_t*)0x20000208 = 0x20000080);
    NONFAILING(*(uint8_t*)0x20000080 = 0x95);
    NONFAILING(*(uint8_t*)0x20000081 = 0);
    NONFAILING(*(uint16_t*)0x20000082 = 0);
    NONFAILING(*(uint32_t*)0x20000084 = 0);
    NONFAILING(*(uint8_t*)0x20000090 = 0x95);
    NONFAILING(*(uint8_t*)0x20000091 = 0);
    NONFAILING(*(uint16_t*)0x20000092 = 0);
    NONFAILING(*(uint32_t*)0x20000094 = 0);
    NONFAILING(*(uint8_t*)0x200000a0 = 5);
    NONFAILING(*(uint8_t*)0x200000a1 = 8);
    NONFAILING(*(uint16_t*)0x200000a2 = 0x1ff);
    NONFAILING(*(uint32_t*)0x200000a4 = 0x7f);
    NONFAILING(STORE_BY_BITMASK(uint8_t, 0x200000b0, 0, 0, 3));
    NONFAILING(STORE_BY_BITMASK(uint8_t, 0x200000b0, 3, 3, 2));
    NONFAILING(STORE_BY_BITMASK(uint8_t, 0x200000b0, 0, 5, 3));
    NONFAILING(STORE_BY_BITMASK(uint8_t, 0x200000b1, 7, 0, 4));
    NONFAILING(STORE_BY_BITMASK(uint8_t, 0x200000b1, 0xb, 4, 4));
    NONFAILING(*(uint16_t*)0x200000b2 = 0x30);
    NONFAILING(*(uint32_t*)0x200000b4 = 1);
    NONFAILING(*(uint64_t*)0x20000210 = 0x200000c0);
    NONFAILING(memcpy((void*)0x200000c0, "GPL", 4));
    NONFAILING(*(uint32_t*)0x20000218 = 0x20);
    NONFAILING(*(uint32_t*)0x2000021c = 0xb);
    NONFAILING(*(uint64_t*)0x20000220 = 0x20000100);
    NONFAILING(*(uint32_t*)0x20000228 = 0x40f00);
    NONFAILING(*(uint32_t*)0x2000022c = 1);
    NONFAILING(*(uint8_t*)0x20000230 = 0);
    NONFAILING(*(uint8_t*)0x20000231 = 0);
    NONFAILING(*(uint8_t*)0x20000232 = 0);
    NONFAILING(*(uint8_t*)0x20000233 = 0);
    NONFAILING(*(uint8_t*)0x20000234 = 0);
    NONFAILING(*(uint8_t*)0x20000235 = 0);
    NONFAILING(*(uint8_t*)0x20000236 = 0);
    NONFAILING(*(uint8_t*)0x20000237 = 0);
    NONFAILING(*(uint8_t*)0x20000238 = 0);
    NONFAILING(*(uint8_t*)0x20000239 = 0);
    NONFAILING(*(uint8_t*)0x2000023a = 0);
    NONFAILING(*(uint8_t*)0x2000023b = 0);
    NONFAILING(*(uint8_t*)0x2000023c = 0);
    NONFAILING(*(uint8_t*)0x2000023d = 0);
    NONFAILING(*(uint8_t*)0x2000023e = 0);
    NONFAILING(*(uint8_t*)0x2000023f = 0);
    NONFAILING(*(uint32_t*)0x20000240 = 0);
    NONFAILING(*(uint32_t*)0x20000244 = 0xb);
    res = syscall(__NR_bpf, 5, 0x20000200, 0x48);
    if (res != -1)
      r[1] = res;
    break;
  case 2:
    NONFAILING(*(uint32_t*)0x200002c0 = r[1]);
    syscall(__NR_setsockopt, r[0], 1, 0x32, 0x200002c0, 4);
    break;
  case 3:
    NONFAILING(*(uint32_t*)0x20000180 = 2);
    NONFAILING(*(uint32_t*)0x20000184 = 0x70);
    NONFAILING(*(uint8_t*)0x20000188 = 0xe6);
    NONFAILING(*(uint8_t*)0x20000189 = 0);
    NONFAILING(*(uint8_t*)0x2000018a = 0);
    NONFAILING(*(uint8_t*)0x2000018b = 0);
    NONFAILING(*(uint32_t*)0x2000018c = 0);
    NONFAILING(*(uint64_t*)0x20000190 = 0);
    NONFAILING(*(uint64_t*)0x20000198 = 0);
    NONFAILING(*(uint64_t*)0x200001a0 = 0);
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 0, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 1, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 2, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 3, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 4, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 5, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 6, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 7, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 8, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 9, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 10, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 11, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 12, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 13, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 14, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 15, 2));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 17, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 18, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 19, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 20, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 21, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 22, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 23, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 24, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 25, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 26, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 27, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 28, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x200001a8, 0, 29, 35));
    NONFAILING(*(uint32_t*)0x200001b0 = 0);
    NONFAILING(*(uint32_t*)0x200001b4 = 0);
    NONFAILING(*(uint64_t*)0x200001b8 = 0x20000000);
    NONFAILING(*(uint64_t*)0x200001c0 = 0);
    NONFAILING(*(uint64_t*)0x200001c8 = 0);
    NONFAILING(*(uint64_t*)0x200001d0 = 0);
    NONFAILING(*(uint32_t*)0x200001d8 = 0);
    NONFAILING(*(uint32_t*)0x200001dc = 0);
    NONFAILING(*(uint64_t*)0x200001e0 = 0);
    NONFAILING(*(uint32_t*)0x200001e8 = 0);
    NONFAILING(*(uint16_t*)0x200001ec = 0);
    NONFAILING(*(uint16_t*)0x200001ee = 0);
    syscall(__NR_perf_event_open, 0x20000180, 0, 0, -1, 0);
    break;
  case 4:
    syscall(__NR_socketpair, 0, 0, 0, 0x20000140);
    break;
  case 5:
    syscall(__NR_socket, 0xa, 1, 0);
    break;
  case 6:
    res = syscall(__NR_getpid);
    if (res != -1)
      r[2] = res;
    break;
  case 7:
    NONFAILING(*(uint32_t*)0x20000340 = 5);
    NONFAILING(*(uint32_t*)0x20000344 = 0x70);
    NONFAILING(*(uint8_t*)0x20000348 = 0x7f);
    NONFAILING(*(uint8_t*)0x20000349 = 0);
    NONFAILING(*(uint8_t*)0x2000034a = 5);
    NONFAILING(*(uint8_t*)0x2000034b = 1);
    NONFAILING(*(uint32_t*)0x2000034c = 0);
    NONFAILING(*(uint64_t*)0x20000350 = 5);
    NONFAILING(*(uint64_t*)0x20000358 = 0);
    NONFAILING(*(uint64_t*)0x20000360 = 8);
    NONFAILING(
        STORE_BY_BITMASK(uint64_t, 0x20000368, 0xfffffffffffffeff, 0, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20000368, 0x7f, 1, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20000368, 0x4b, 2, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20000368, 8, 3, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20000368, 3, 4, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20000368, 0xfff, 5, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20000368, 2, 6, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20000368, 0x100000001, 7, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20000368, 0x3c18, 8, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20000368, 8, 9, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20000368, 0x3f5, 10, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20000368, 1, 11, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20000368, 0x3bdd, 12, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20000368, 0xf1, 13, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20000368, 4, 14, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20000368, 4, 15, 2));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20000368, 9, 17, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20000368, 0x7fff, 18, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20000368, 0xd77b, 19, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20000368, 7, 20, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20000368, 0x100, 21, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20000368, 0xb31, 22, 1));
    NONFAILING(
        STORE_BY_BITMASK(uint64_t, 0x20000368, 0xffffffffffff8015, 23, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20000368, 5, 24, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20000368, 6, 25, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20000368, 2, 26, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20000368, 2, 27, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20000368, 3, 28, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20000368, 0, 29, 35));
    NONFAILING(*(uint32_t*)0x20000370 = 0x7ff);
    NONFAILING(*(uint32_t*)0x20000374 = 2);
    NONFAILING(*(uint64_t*)0x20000378 = 0x20000300);
    NONFAILING(*(uint64_t*)0x20000380 = 8);
    NONFAILING(*(uint64_t*)0x20000388 = 1);
    NONFAILING(*(uint64_t*)0x20000390 = 6);
    NONFAILING(*(uint32_t*)0x20000398 = 0xb);
    NONFAILING(*(uint32_t*)0x2000039c = 3);
    NONFAILING(*(uint64_t*)0x200003a0 = 8);
    NONFAILING(*(uint32_t*)0x200003a8 = 0);
    NONFAILING(*(uint16_t*)0x200003ac = 0xca0e);
    NONFAILING(*(uint16_t*)0x200003ae = 0);
    syscall(__NR_perf_event_open, 0x20000340, r[2], 0xc, 0xffffff9c, 8);
    break;
  case 8:
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
      r[3] = res;
    break;
  case 9:
    NONFAILING(*(uint32_t*)0x20000180 = r[3]);
    NONFAILING(*(uint64_t*)0x20000188 = 0x20000000);
    NONFAILING(*(uint64_t*)0x20000190 = 0x20000140);
    NONFAILING(*(uint64_t*)0x20000198 = 0);
    syscall(__NR_bpf, 2, 0x20000180, 0x20);
    break;
  case 10:
    NONFAILING(*(uint32_t*)0x20000180 = r[3]);
    NONFAILING(*(uint64_t*)0x20000188 = 0x20000080);
    NONFAILING(*(uint64_t*)0x20000190 = 0x20000140);
    NONFAILING(*(uint64_t*)0x20000198 = 0);
    syscall(__NR_bpf, 2, 0x20000180, 0x20);
    break;
  }
}

void execute_one()
{
  execute(11);
  collide = 1;
  execute(11);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
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
