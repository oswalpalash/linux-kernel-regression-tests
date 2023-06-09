// https://syzkaller.appspot.com/bug?id=a8e52aea23a08661ca01bb5346bb78d35df76b50
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
  syscall(SYS_futex, &ev->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1000000);
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
    if (__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
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
  int collide = 0;
again:
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
      if (collide && (call % 2) == 0)
        break;
      event_timedwait(&th->done, 45);
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
  if (!collide) {
    collide = 1;
    goto again;
  }
}

uint64_t r[4] = {0xffffffffffffffff, 0x0, 0xffffffffffffffff,
                 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    res = syscall(__NR_socket, 0x11ul, 0x800000003ul, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    *(uint16_t*)0x20000100 = 0x11;
    memcpy((void*)0x20000102,
           "\x00\x00\x01\x00\x00\x00\x00\x00\x08\x00\x44\x94\x4e\xeb\xa7\x1a"
           "\x49\x76\xe2\x52\x92\x2c\xb1\x8f\x6e\x2e\x2a\xba\x00\x00\x00\x01"
           "\x2e\x0b\x38\x36\x00\x54\x04\xb0\xe0\x30\x1a\x4c\xe8\x75\xf2\xe3"
           "\xff\x5f\x16\x3e\xe3\x40\xb7\x67\x95\x00\x80\x00\x00\x00\x00\x00"
           "\x00\x01\x01\x01\x3c\x58\x11\x03\x9e\x15\x77\x50\x27\xec\xce\x66"
           "\xfd\x79\x2b\xbf\x0e\x5b\xf5\xff\x1b\x08\x16\xf3\xf6\xdb\x1c\x00"
           "\x01\x00\x00\x00\x00\x00\x00\x00\x49\x74\x00\x00\x00\x00\x00\x00"
           "\x00\x06\xad\x8e\x5e\xcc\x32\x6d\x3a\x09\xff\xc2\xc6\x54",
           126);
    syscall(__NR_bind, r[0], 0x20000100ul, 0x80ul);
    break;
  case 2:
    *(uint32_t*)0x20000000 = 0x14;
    res = syscall(__NR_getsockname, r[0], 0x200003c0ul, 0x20000000ul);
    if (res != -1)
      r[1] = *(uint32_t*)0x200003c4;
    break;
  case 3:
    res = syscall(__NR_socket, 0x10ul, 3ul, 0);
    if (res != -1)
      r[2] = res;
    break;
  case 4:
    *(uint64_t*)0x20000280 = 0;
    *(uint32_t*)0x20000288 = 0;
    *(uint64_t*)0x20000290 = 0x20000780;
    *(uint64_t*)0x20000780 = 0x200001c0;
    *(uint32_t*)0x200001c0 = 0x40;
    *(uint16_t*)0x200001c4 = 0x24;
    *(uint16_t*)0x200001c6 = 0xf0b;
    *(uint32_t*)0x200001c8 = 0;
    *(uint32_t*)0x200001cc = 0;
    *(uint8_t*)0x200001d0 = 0;
    *(uint8_t*)0x200001d1 = 0;
    *(uint16_t*)0x200001d2 = 0;
    *(uint32_t*)0x200001d4 = r[1];
    *(uint16_t*)0x200001d8 = 0;
    *(uint16_t*)0x200001da = 0;
    *(uint16_t*)0x200001dc = -1;
    *(uint16_t*)0x200001de = -1;
    *(uint16_t*)0x200001e0 = 0;
    *(uint16_t*)0x200001e2 = 0;
    *(uint16_t*)0x200001e4 = 0xd;
    *(uint16_t*)0x200001e6 = 1;
    memcpy((void*)0x200001e8, "fq_codel\000", 9);
    *(uint16_t*)0x200001f4 = 0xc;
    *(uint16_t*)0x200001f6 = 2;
    *(uint16_t*)0x200001f8 = 8;
    *(uint16_t*)0x200001fa = 2;
    *(uint32_t*)0x200001fc = 0;
    *(uint64_t*)0x20000788 = 0x40;
    *(uint64_t*)0x20000298 = 1;
    *(uint64_t*)0x200002a0 = 0;
    *(uint64_t*)0x200002a8 = 0;
    *(uint32_t*)0x200002b0 = 0;
    syscall(__NR_sendmsg, r[2], 0x20000280ul, 0ul);
    break;
  case 5:
    res = syscall(__NR_socket, 0x11ul, 0x800000003ul, 0);
    if (res != -1)
      r[3] = res;
    break;
  case 6:
    memcpy((void*)0x200001c0, "\xa2\xe6\x99\x9b", 4);
    syscall(__NR_setsockopt, r[3], 0x107, 0xf, 0x200001c0ul, 4ul);
    break;
  case 7:
    *(uint16_t*)0x20000080 = 0x11;
    memcpy((void*)0x20000082,
           "\x00\x00\x01\x00\x00\x00\x00\x00\x08\x00\x44\x94\x4e\xeb\xa7\x1a"
           "\x49\x76\xe2\x52\x92\x2c\xb1\x8f\x6e\x2e\x2a\xba\x00\x00\x00\x01"
           "\x2e\x0b\x38\x36\x00\x54\x04\xb0\xe0\x30\x1a\x4c\xe8\x75\xf2\xe3"
           "\xff\x5f\x16\x3e\xe3\x40\xb7\x67\x95\x00\x80\x00\x00\x00\x00\x00"
           "\x00\x01\x01\x01\x3c\x58\x11\x03\x9e\x15\x77\x50\x27\xec\xce\x66"
           "\xfd\x79\x2b\xbf\x0e\x5b\xf5\xff\x1b\x08\x16\xf3\xf6\xdb\x1c\x00"
           "\x01\x00\x00\x00\x00\x00\x00\x00\x49\x74\x00\x00\x00\x00\x00\x00"
           "\x00\x06\xad\x8e\x5e\xcc\x32\x6d\x3a\x09\xff\xc2\xc6\x54",
           126);
    syscall(__NR_bind, r[3], 0x20000080ul, 0x80ul);
    break;
  case 8:
    *(uint16_t*)0x200003c0 = 0x108;
    *(uint8_t*)0x200003c2 = 0;
    *(uint8_t*)0x200003c3 = 0;
    *(uint32_t*)0x200003c4 = 0x367;
    *(uint32_t*)0x200003c8 = 0;
    *(uint32_t*)0x200003cc = 0;
    *(uint32_t*)0x200003d0 = 0;
    *(uint32_t*)0x200003d4 = 0;
    *(uint32_t*)0x200003d8 = 8;
    *(uint32_t*)0x200003dc = 0;
    memcpy((void*)0x200003e0,
           "\xe6\x06\xec\x00\x24\x05\x04\x00\x07\x00\x00\x00\xbe\xe5\x7b\x6f"
           "\xcf\x21\xab\xf4\xb9\x04\x0d\x47\x2a\x0f\x14\xe5\x2e\x61\xa2\xc2"
           "\x20\x4f\x44\x68\xd3\xcd\xcc\xf5\x99\x72\x41\x2d\xc7\xdb\xd1\x11"
           "\x8f\x73\xfe\x67\x43\x36\x8e\x2b\xe0\xe4\x76\x32\x8e\xf2\x18\xc2"
           "\x84\xb5\x16\x9a\xa4\x96\xf6\xf4\x35\xa3\x05\xc1",
           76);
    syscall(__NR_write, r[3], 0x200003c0ul, 0x6cul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  loop();
  return 0;
}
