// https://syzkaller.appspot.com/bug?id=7cd3db70971bc10523485d12d95fdefa301fb819
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <linux/futex.h>

static void sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
}

static uint64_t current_time_ms(void)
{
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    exit(1);
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void thread_start(void* (*fn)(void*), void* arg)
{
  pthread_t th;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 128 << 10);
  if (pthread_create(&th, &attr, fn, arg))
    exit(1);
  pthread_attr_destroy(&attr);
}

typedef struct {
  int state;
} event_t;

static void event_init(event_t* ev)
{
  ev->state = 0;
}

static void event_reset(event_t* ev)
{
  ev->state = 0;
}

static void event_set(event_t* ev)
{
  if (ev->state)
    exit(1);
  __atomic_store_n(&ev->state, 1, __ATOMIC_RELEASE);
  syscall(SYS_futex, &ev->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG);
}

static void event_wait(event_t* ev)
{
  while (!__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, 0);
}

static int event_isset(event_t* ev)
{
  return __atomic_load_n(&ev->state, __ATOMIC_ACQUIRE);
}

static int event_timedwait(event_t* ev, uint64_t timeout)
{
  uint64_t start = current_time_ms();
  uint64_t now = start;
  for (;;) {
    uint64_t remain = timeout - (now - start);
    struct timespec ts;
    ts.tv_sec = remain / 1000;
    ts.tv_nsec = (remain % 1000) * 1000 * 1000;
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, &ts);
    if (__atomic_load_n(&ev->state, __ATOMIC_RELAXED))
      return 1;
    now = current_time_ms();
    if (now - start > timeout)
      return 0;
  }
}

struct thread_t {
  int created, call;
  event_t ready, done;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    event_wait(&th->ready);
    event_reset(&th->ready);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    event_set(&th->done);
  }
  return 0;
}

static void loop(void)
{
  int i, call, thread;
  for (call = 0; call < 3; call++) {
    for (thread = 0; thread < (int)(sizeof(threads) / sizeof(threads[0]));
         thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        event_init(&th->ready);
        event_init(&th->done);
        event_set(&th->done);
        thread_start(thr, th);
      }
      if (!event_isset(&th->done))
        continue;
      event_reset(&th->done);
      th->call = call;
      __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
      event_set(&th->ready);
      event_timedwait(&th->done, 45);
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
}

#ifndef __NR_execveat
#define __NR_execveat 322
#endif
#ifndef __NR_memfd_create
#define __NR_memfd_create 319
#endif

uint64_t r[1] = {0xffffffffffffffff};

void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    memcpy((void*)0x20000080, "\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00", 10);
    res = syscall(__NR_memfd_create, 0x20000080, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    *(uint8_t*)0x20000100 = 0x7f;
    *(uint8_t*)0x20000101 = 0x45;
    *(uint8_t*)0x20000102 = 0x4c;
    *(uint8_t*)0x20000103 = 0x46;
    *(uint8_t*)0x20000104 = 0;
    *(uint8_t*)0x20000105 = 0;
    *(uint8_t*)0x20000106 = 0;
    *(uint8_t*)0x20000107 = 0;
    *(uint64_t*)0x20000108 = 0;
    *(uint16_t*)0x20000110 = 3;
    *(uint16_t*)0x20000112 = 6;
    *(uint32_t*)0x20000114 = 0;
    *(uint32_t*)0x20000118 = 0;
    *(uint32_t*)0x2000011c = 0x38;
    *(uint32_t*)0x20000120 = 0;
    *(uint32_t*)0x20000124 = 0;
    *(uint16_t*)0x20000128 = 0;
    *(uint16_t*)0x2000012a = 0x20;
    *(uint16_t*)0x2000012c = 2;
    *(uint16_t*)0x2000012e = 0;
    *(uint16_t*)0x20000130 = 0;
    *(uint16_t*)0x20000132 = 0;
    *(uint32_t*)0x20000138 = 0;
    *(uint32_t*)0x2000013c = 0;
    *(uint32_t*)0x20000140 = 0;
    *(uint32_t*)0x20000144 = 0;
    *(uint32_t*)0x20000148 = 0;
    *(uint32_t*)0x2000014c = 0;
    *(uint32_t*)0x20000150 = 0;
    *(uint32_t*)0x20000154 = 0;
    *(uint32_t*)0x20000158 = 0;
    *(uint32_t*)0x2000015c = 0;
    *(uint32_t*)0x20000160 = 0;
    *(uint32_t*)0x20000164 = 0;
    *(uint32_t*)0x20000168 = 0;
    *(uint32_t*)0x2000016c = 0;
    *(uint32_t*)0x20000170 = 0;
    *(uint32_t*)0x20000174 = 0;
    syscall(__NR_write, r[0], 0x20000100, 0x78);
    break;
  case 2:
    memcpy((void*)0x20000000, "", 1);
    syscall(__NR_execveat, r[0], 0x20000000, 0, 0, 0x1000);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
