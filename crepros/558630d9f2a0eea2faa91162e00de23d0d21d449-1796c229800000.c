// https://syzkaller.appspot.com/bug?id=558630d9f2a0eea2faa91162e00de23d0d21d449
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <linux/futex.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
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
void execute_call(int call)
{
  switch (call) {
  case 0:
    syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
    break;
  case 1:
    *(uint32_t*)0x20bab000 = 1;
    *(uint32_t*)0x20bab004 = 0x78;
    *(uint8_t*)0x20bab008 = 0;
    *(uint8_t*)0x20bab009 = 0;
    *(uint8_t*)0x20bab00a = 0;
    *(uint8_t*)0x20bab00b = 0;
    *(uint32_t*)0x20bab00c = 0;
    *(uint64_t*)0x20bab010 = 0;
    *(uint64_t*)0x20bab018 = 0;
    *(uint64_t*)0x20bab020 = 0;
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 0, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 1, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 2, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 3, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 4, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 5, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 6, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 7, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 8, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 1, 9, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 10, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 11, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 12, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 13, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 14, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 15, 2);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 17, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 18, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 19, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 20, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 21, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 22, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 23, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 24, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 25, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 26, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 27, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 28, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 29, 35);
    *(uint32_t*)0x20bab030 = 0;
    *(uint32_t*)0x20bab034 = 0;
    *(uint64_t*)0x20bab038 = 0;
    *(uint64_t*)0x20bab040 = 0;
    *(uint64_t*)0x20bab048 = 0;
    *(uint64_t*)0x20bab050 = 0;
    *(uint64_t*)0x20bab058 = 0;
    *(uint32_t*)0x20bab060 = 0;
    *(uint64_t*)0x20bab068 = 0;
    *(uint32_t*)0x20bab070 = 0;
    *(uint16_t*)0x20bab074 = 0;
    *(uint16_t*)0x20bab076 = 0;
    r[0] = syscall(__NR_perf_event_open, 0x20bab000, -1, 0, -1, 0);
    break;
  case 2:
    syscall(__NR_ioctl, r[0], 0x2403, 0x43017aa);
    break;
  case 3:
    r[1] = syscall(__NR_getpid);
    break;
  case 4:
    *(uint32_t*)0x2025c000 = 2;
    *(uint32_t*)0x2025c004 = 0x78;
    *(uint8_t*)0x2025c008 = 0xe3;
    *(uint8_t*)0x2025c009 = 0;
    *(uint8_t*)0x2025c00a = 0;
    *(uint8_t*)0x2025c00b = 0;
    *(uint32_t*)0x2025c00c = 0;
    *(uint64_t*)0x2025c010 = 0;
    *(uint64_t*)0x2025c018 = 0;
    *(uint64_t*)0x2025c020 = 0;
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 0, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 3, 1, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 2, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 3, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 4, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 5, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 4, 6, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 7, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 8, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 9, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 10, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0xfffffffffffffffc, 11, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 12, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 13, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 14, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, -1, 15, 2);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 17, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 18, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, -1, 19, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 20, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 21, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 8, 22, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 23, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 24, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 25, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 26, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 27, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 28, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 29, 35);
    *(uint32_t*)0x2025c030 = 0;
    *(uint32_t*)0x2025c034 = 0;
    *(uint64_t*)0x2025c038 = 0x20000000;
    *(uint64_t*)0x2025c040 = 0;
    *(uint64_t*)0x2025c048 = 0;
    *(uint64_t*)0x2025c050 = 0;
    *(uint64_t*)0x2025c058 = 0;
    *(uint32_t*)0x2025c060 = 0;
    *(uint64_t*)0x2025c068 = 0;
    *(uint32_t*)0x2025c070 = 0;
    *(uint16_t*)0x2025c074 = 0;
    *(uint16_t*)0x2025c076 = 0;
    syscall(__NR_perf_event_open, 0x2025c000, r[1], 0, -1, 0);
    break;
  }
}

void loop()
{
  memset(r, -1, sizeof(r));
  execute(5);
  collide = 1;
  execute(5);
}

int main()
{
  loop();
  return 0;
}
