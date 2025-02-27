// https://syzkaller.appspot.com/bug?id=9c55af67ce995cf6c4f11ab6f5d3ee805d67fc00
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
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

static void use_temporary_dir(void)
{
  char tmpdir_template[] = "./syzkaller.XXXXXX";
  char* tmpdir = mkdtemp(tmpdir_template);
  if (!tmpdir)
    exit(1);
  if (chmod(tmpdir, 0777))
    exit(1);
  if (chdir(tmpdir))
    exit(1);
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
  int collide = 0;
again:
  for (call = 0; call < 6; call++) {
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

uint64_t r[1] = {0xffffffffffffffff};

void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    res = syscall(__NR_socket, 0xa, 3, 0x3e);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    *(uint16_t*)0x20000300 = 0xa;
    *(uint16_t*)0x20000302 = htobe16(0);
    *(uint32_t*)0x20000304 = 0;
    *(uint64_t*)0x20000308 = htobe64(0);
    *(uint64_t*)0x20000310 = htobe64(1);
    *(uint32_t*)0x20000318 = 0;
    syscall(__NR_connect, r[0], 0x20000300, 0x1c);
    break;
  case 2:
    *(uint64_t*)0x200000c0 = 0;
    *(uint32_t*)0x200000c8 = 0;
    *(uint64_t*)0x200000d0 = 0;
    *(uint64_t*)0x200000d8 = 0;
    *(uint64_t*)0x200000e0 = 0;
    *(uint64_t*)0x200000e8 = 0;
    *(uint32_t*)0x200000f0 = 0;
    syscall(__NR_sendmsg, r[0], 0x200000c0, 0xc100);
    break;
  case 3:
    *(uint64_t*)0x20000380 = 0;
    *(uint32_t*)0x20000388 = 0;
    *(uint64_t*)0x20000390 = 0x20000240;
    *(uint64_t*)0x20000240 = 0x20000200;
    memcpy((void*)0x20000200, "\xb1\x0b\x93\x86", 4);
    *(uint64_t*)0x20000248 = 4;
    *(uint64_t*)0x20000398 = 1;
    *(uint64_t*)0x200003a0 = 0;
    *(uint64_t*)0x200003a8 = 0;
    *(uint32_t*)0x200003b0 = 0;
    syscall(__NR_sendmsg, r[0], 0x20000380, 0);
    break;
  case 4:
    *(uint32_t*)0x20000000 = 0xe2b;
    syscall(__NR_setsockopt, r[0], 0x29, 0x4a, 0x20000000, 4);
    break;
  case 5:
    *(uint64_t*)0x20000140 = 0;
    *(uint32_t*)0x20000148 = 0;
    *(uint64_t*)0x20000150 = 0;
    *(uint64_t*)0x20000158 = 0;
    *(uint64_t*)0x20000160 = 0;
    *(uint64_t*)0x20000168 = 0;
    *(uint32_t*)0x20000170 = 0;
    syscall(__NR_recvmsg, r[0], 0x20000140, 2);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  use_temporary_dir();
  loop();
  return 0;
}
