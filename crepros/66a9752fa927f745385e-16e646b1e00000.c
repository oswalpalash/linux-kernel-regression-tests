// https://syzkaller.appspot.com/bug?id=26f3ca04ea5a2cabf390a85264ad25fe8474c1b6
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
  for (call = 0; call < 10; call++) {
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res;
  switch (call) {
  case 0:
    memcpy((void*)0x20000240, "./file1\000", 8);
    syscall(__NR_mkdir, 0x20000240ul, 0ul);
    break;
  case 1:
    memcpy((void*)0x20000180, "./bus\000", 6);
    syscall(__NR_mkdir, 0x20000180ul, 0ul);
    break;
  case 2:
    memcpy((void*)0x200000c0, "./bus/file0\000", 12);
    res = syscall(__NR_creat, 0x200000c0ul, 0ul);
    if (res != -1)
      r[0] = res;
    break;
  case 3:
    *(uint32_t*)0x20000040 = 0;
    *(uint8_t*)0x20000044 = 0;
    *(uint8_t*)0x20000045 = 0;
    *(uint8_t*)0x20000046 = 0;
    *(uint8_t*)0x20000047 = 0;
    syscall(__NR_write, r[0], 0x20000040ul, 0x5577b540ul);
    break;
  case 4:
    memcpy((void*)0x20000280, "./file0\000", 8);
    syscall(__NR_mkdir, 0x20000280ul, 0ul);
    break;
  case 5:
    memcpy((void*)0x20000000, "./bus\000", 6);
    memcpy((void*)0x20000080, "overlay\000", 8);
    memcpy((void*)0x20000100, "lowerdir=./bus,workdir=./file1,upperdir=./file0",
           47);
    syscall(__NR_mount, 0x400000ul, 0x20000000ul, 0x20000080ul, 0ul,
            0x20000100ul);
    break;
  case 6:
    memcpy((void*)0x200002c0, "./bus\000", 6);
    syscall(__NR_chdir, 0x200002c0ul);
    break;
  case 7:
    memcpy((void*)0x20000200, "./file0\000", 8);
    res = syscall(__NR_open, 0x20000200ul, 0ul, 0ul);
    if (res != -1)
      r[1] = res;
    break;
  case 8:
    memcpy((void*)0x20000000, "./file0\000", 8);
    syscall(__NR_lchown, 0x20000000ul, 0, 0);
    break;
  case 9:
    syscall(__NR_lseek, r[1], 0ul, 4ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  loop();
  return 0;
}
