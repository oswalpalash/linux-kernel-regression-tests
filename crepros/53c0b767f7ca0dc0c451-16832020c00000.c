// https://syzkaller.appspot.com/bug?id=6408a8ba0fa0e3940c5c2dfa40e808cbf4228689
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
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
  int i;
  for (i = 0; i < 100; i++) {
    if (pthread_create(&th, &attr, fn, arg) == 0) {
      pthread_attr_destroy(&attr);
      return;
    }
    if (errno == EAGAIN) {
      usleep(50);
      continue;
    }
    break;
  }
  exit(1);
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
  for (call = 0; call < 9; call++) {
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

#ifndef __NR_userfaultfd
#define __NR_userfaultfd 323
#endif

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0x0};

void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    res = syscall(__NR_userfaultfd, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    *(uint64_t*)0x20000140 = 0xaa;
    *(uint64_t*)0x20000148 = 0x40;
    *(uint64_t*)0x20000150 = 0;
    syscall(__NR_ioctl, r[0], 0xc018aa3f, 0x20000140);
    break;
  case 2:
    syscall(__NR_read, r[0], 0x20000000, 0x80);
    break;
  case 3:
    syscall(__NR_mmap, 0x20000000, 0xb36000, 0x1e, 0x8031, -1, 0);
    break;
  case 4:
    res = syscall(__NR_socketpair, 1, 1, 0, 0x20000000);
    if (res != -1)
      r[1] = *(uint32_t*)0x20000004;
    break;
  case 5:
    res = syscall(__NR_io_setup, 1, 0x20000080);
    if (res != -1)
      r[2] = *(uint64_t*)0x20000080;
    break;
  case 6:
    syscall(__NR_close, r[1]);
    break;
  case 7:
    syscall(__NR_userfaultfd, 0);
    break;
  case 8:
    *(uint64_t*)0x20000600 = 0x20000180;
    *(uint64_t*)0x20000180 = 0;
    *(uint32_t*)0x20000188 = 0;
    *(uint32_t*)0x2000018c = 0;
    *(uint16_t*)0x20000190 = 5;
    *(uint16_t*)0x20000192 = 0;
    *(uint32_t*)0x20000194 = r[1];
    *(uint64_t*)0x20000198 = 0;
    *(uint64_t*)0x200001a0 = 0;
    *(uint64_t*)0x200001a8 = 0;
    *(uint64_t*)0x200001b0 = 0;
    *(uint32_t*)0x200001b8 = 0;
    *(uint32_t*)0x200001bc = -1;
    syscall(__NR_io_submit, r[2], 1, 0x20000600);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
