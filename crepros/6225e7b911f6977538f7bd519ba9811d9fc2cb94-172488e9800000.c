// https://syzkaller.appspot.com/bug?id=6225e7b911f6977538f7bd519ba9811d9fc2cb94
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <fcntl.h>
#include <linux/futex.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

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
  while (1) {
    test();
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

#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_perf_event_open
#define __NR_perf_event_open 336
#endif
#ifndef __NR_ioctl
#define __NR_ioctl 54
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[1];
void execute_call(int call)
{
  switch (call) {
  case 0:
    syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
    break;
  case 1:
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
  case 2:
    memcpy((void*)0x20be8ff5, "/dev/midi#", 11);
    syz_open_dev(0x20be8ff5, 1, 0x80000);
    break;
  case 3:
    syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
    break;
  case 4:
    memcpy((void*)0x208be000, "/dev/usbmon#", 13);
    r[0] = syz_open_dev(0x208be000, 0, 0);
    break;
  case 5:
    syscall(__NR_mmap, 0x20ac6000, 0x4000, 0x1000004, 0x8011, r[0], 0);
    break;
  case 6:
    syscall(__NR_ioctl, r[0], 0x9204, 0xf0b1);
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
  for (;;) {
    loop();
  }
}
