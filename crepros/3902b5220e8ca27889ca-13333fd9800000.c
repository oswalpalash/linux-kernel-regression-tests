// https://syzkaller.appspot.com/bug?id=3256c011ec68c385985faee3785aab3081e244b3
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/futex.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
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

static uint64_t current_time_ms()
{
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    fail("clock_gettime failed");
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static uintptr_t syz_open_dev(uintptr_t a0, uintptr_t a1, uintptr_t a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
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

long r[2];
uint64_t procid;
void execute_call(int call)
{
  switch (call) {
  case 0:
    syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
    break;
  case 1:
    memcpy((void*)0x2030e000, "/dev/snd/pcmC#D#p", 18);
    r[0] = syz_open_dev(0x2030e000, 6, 0);
    break;
  case 2:
    memcpy((void*)0x20006ff6, "/dev/dsp#", 10);
    r[1] = syz_open_dev(0x20006ff6, 1, 0);
    break;
  case 3:
    *(uint32_t*)0x20009fe0 = r[1];
    *(uint16_t*)0x20009fe4 = 0;
    *(uint16_t*)0x20009fe6 = 0;
    syscall(__NR_poll, 0x20009fe0, 1, 0);
    break;
  case 4:
    *(uint16_t*)0x20e25000 = 0xa;
    *(uint16_t*)0x20e25002 = htobe16(0x4e23 + procid * 4);
    *(uint32_t*)0x20e25004 = 1;
    *(uint8_t*)0x20e25008 = 0;
    *(uint8_t*)0x20e25009 = 0;
    *(uint8_t*)0x20e2500a = 0;
    *(uint8_t*)0x20e2500b = 0;
    *(uint8_t*)0x20e2500c = 0;
    *(uint8_t*)0x20e2500d = 0;
    *(uint8_t*)0x20e2500e = 0;
    *(uint8_t*)0x20e2500f = 0;
    *(uint8_t*)0x20e25010 = 0;
    *(uint8_t*)0x20e25011 = 0;
    *(uint8_t*)0x20e25012 = -1;
    *(uint8_t*)0x20e25013 = -1;
    *(uint8_t*)0x20e25014 = 0xac;
    *(uint8_t*)0x20e25015 = 0x14;
    *(uint8_t*)0x20e25016 = 0 + procid * 1;
    *(uint8_t*)0x20e25017 = 0xbb;
    *(uint32_t*)0x20e25018 = 1;
    syscall(__NR_setsockopt, r[0], 0x84, 0x64, 0x20e25000, 0x1c);
    break;
  case 5:
    *(uint32_t*)0x2001d000 = 2;
    *(uint32_t*)0x2001d004 = 0x78;
    *(uint8_t*)0x2001d008 = 0xe2;
    *(uint8_t*)0x2001d009 = 0;
    *(uint8_t*)0x2001d00a = 0;
    *(uint8_t*)0x2001d00b = 0;
    *(uint32_t*)0x2001d00c = 0;
    *(uint64_t*)0x2001d010 = 0;
    *(uint64_t*)0x2001d018 = 0;
    *(uint64_t*)0x2001d020 = 0;
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 0, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 1, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 2, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 3, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 4, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 5, 5, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 6, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 7, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 8, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 9, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 10, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 11, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 12, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 13, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 14, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 15, 2);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 17, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 18, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 19, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 20, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 21, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 22, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 23, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 24, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 25, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 26, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 27, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 28, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 29, 35);
    *(uint32_t*)0x2001d030 = 0;
    *(uint32_t*)0x2001d034 = 0;
    *(uint64_t*)0x2001d038 = 0x20000000;
    *(uint64_t*)0x2001d040 = 0;
    *(uint64_t*)0x2001d048 = 0;
    *(uint64_t*)0x2001d050 = 0;
    *(uint64_t*)0x2001d058 = 0;
    *(uint32_t*)0x2001d060 = 0;
    *(uint64_t*)0x2001d068 = 0;
    *(uint32_t*)0x2001d070 = 0;
    *(uint16_t*)0x2001d074 = 0;
    *(uint16_t*)0x2001d076 = 0;
    syscall(__NR_perf_event_open, 0x2001d000, 0, 0, -1, 0);
    break;
  case 6:
    *(uint64_t*)0x20003ff8 = 0x40000000fde;
    syscall(__NR_ioctl, r[1], 0x800000c0045002, 0x20003ff8);
    break;
  }
}

void test()
{
  memset(r, -1, sizeof(r));
  execute(7);
  collide = 1;
  execute(7);
}

int main()
{
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      for (;;) {
        loop();
      }
    }
  }
  sleep(1000000);
  return 0;
}
