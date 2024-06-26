// https://syzkaller.appspot.com/bug?id=e02419c12131c24e2a957ea050c2ab6dcbbc3270
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
  syscall(SYS_futex, &ev->state, FUTEX_WAKE);
}

static void event_wait(event_t* ev)
{
  while (!__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
    syscall(SYS_futex, &ev->state, FUTEX_WAIT, 0, 0);
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
    syscall(SYS_futex, &ev->state, FUTEX_WAIT, 0, &ts);
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
  for (call = 0; call < 27; call++) {
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

uint64_t r[6] = {0xffffffffffffffff, 0x0,
                 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff, 0xffffffffffffffff};

void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    res = syscall(__NR_dup2, -1, -1);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    *(uint32_t*)0x200000c0 = 0;
    *(uint32_t*)0x200000c4 = 0;
    *(uint32_t*)0x200000c8 = 0;
    memcpy((void*)0x200000cc,
           "\x71\x75\x65\x75\x65\x31\x00\x00\x00\x00\x00\x00\x00\x00\x31\x3b"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\xcc\xbf\x7d\xdd\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
           64);
    *(uint32_t*)0x2000010c = 0;
    *(uint8_t*)0x20000110 = 0;
    *(uint8_t*)0x20000111 = 0;
    *(uint8_t*)0x20000112 = 0;
    *(uint8_t*)0x20000113 = 0;
    *(uint8_t*)0x20000114 = 0;
    *(uint8_t*)0x20000115 = 0;
    *(uint8_t*)0x20000116 = 0;
    *(uint8_t*)0x20000117 = 0;
    *(uint8_t*)0x20000118 = 0;
    *(uint8_t*)0x20000119 = 0;
    *(uint8_t*)0x2000011a = 0;
    *(uint8_t*)0x2000011b = 0;
    *(uint8_t*)0x2000011c = 0;
    *(uint8_t*)0x2000011d = 0;
    *(uint8_t*)0x2000011e = 0;
    *(uint8_t*)0x2000011f = 0;
    *(uint8_t*)0x20000120 = 0;
    *(uint8_t*)0x20000121 = 0;
    *(uint8_t*)0x20000122 = 0;
    *(uint8_t*)0x20000123 = 0;
    *(uint8_t*)0x20000124 = 0;
    *(uint8_t*)0x20000125 = 0;
    *(uint8_t*)0x20000126 = 0;
    *(uint8_t*)0x20000127 = 0;
    *(uint8_t*)0x20000128 = 0;
    *(uint8_t*)0x20000129 = 0;
    *(uint8_t*)0x2000012a = 0;
    *(uint8_t*)0x2000012b = 0;
    *(uint8_t*)0x2000012c = 0;
    *(uint8_t*)0x2000012d = 0;
    *(uint8_t*)0x2000012e = 0;
    *(uint8_t*)0x2000012f = 0;
    *(uint8_t*)0x20000130 = 0;
    *(uint8_t*)0x20000131 = 0;
    *(uint8_t*)0x20000132 = 0;
    *(uint8_t*)0x20000133 = 0;
    *(uint8_t*)0x20000134 = 0;
    *(uint8_t*)0x20000135 = 0;
    *(uint8_t*)0x20000136 = 0;
    *(uint8_t*)0x20000137 = 0;
    *(uint8_t*)0x20000138 = 0;
    *(uint8_t*)0x20000139 = 0;
    *(uint8_t*)0x2000013a = 0;
    *(uint8_t*)0x2000013b = 0;
    *(uint8_t*)0x2000013c = 0;
    *(uint8_t*)0x2000013d = 0;
    *(uint8_t*)0x2000013e = 0;
    *(uint8_t*)0x2000013f = 0;
    *(uint8_t*)0x20000140 = 0;
    *(uint8_t*)0x20000141 = 0;
    *(uint8_t*)0x20000142 = 0;
    *(uint8_t*)0x20000143 = 0;
    *(uint8_t*)0x20000144 = 0;
    *(uint8_t*)0x20000145 = 0;
    *(uint8_t*)0x20000146 = 0;
    *(uint8_t*)0x20000147 = 0;
    *(uint8_t*)0x20000148 = 0;
    *(uint8_t*)0x20000149 = 0;
    *(uint8_t*)0x2000014a = 0;
    *(uint8_t*)0x2000014b = 0;
    syscall(__NR_ioctl, r[0], 0xc08c5332, 0x200000c0);
    break;
  case 2:
    memcpy((void*)0x20000080, "security.evm", 13);
    syscall(__NR_fremovexattr, -1, 0x20000080);
    break;
  case 3:
    *(uint32_t*)0x200001c0 = 0xf48b;
    *(uint32_t*)0x200001c4 = 0;
    *(uint32_t*)0x200001c8 = 0;
    memcpy((void*)0x200001cc,
           "\x71\x75\x65\x75\x65\x31\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
           64);
    *(uint32_t*)0x2000020c = 0;
    *(uint8_t*)0x20000210 = 0;
    *(uint8_t*)0x20000211 = 0;
    *(uint8_t*)0x20000212 = 0;
    *(uint8_t*)0x20000213 = 0;
    *(uint8_t*)0x20000214 = 0;
    *(uint8_t*)0x20000215 = 0;
    *(uint8_t*)0x20000216 = 0;
    *(uint8_t*)0x20000217 = 0;
    *(uint8_t*)0x20000218 = 0;
    *(uint8_t*)0x20000219 = 0;
    *(uint8_t*)0x2000021a = 0;
    *(uint8_t*)0x2000021b = 0;
    *(uint8_t*)0x2000021c = 0;
    *(uint8_t*)0x2000021d = 0;
    *(uint8_t*)0x2000021e = 0;
    *(uint8_t*)0x2000021f = 0;
    *(uint8_t*)0x20000220 = 0;
    *(uint8_t*)0x20000221 = 0;
    *(uint8_t*)0x20000222 = 0;
    *(uint8_t*)0x20000223 = 0;
    *(uint8_t*)0x20000224 = 0;
    *(uint8_t*)0x20000225 = 0;
    *(uint8_t*)0x20000226 = 0;
    *(uint8_t*)0x20000227 = 0;
    *(uint8_t*)0x20000228 = 0;
    *(uint8_t*)0x20000229 = 0;
    *(uint8_t*)0x2000022a = 0;
    *(uint8_t*)0x2000022b = 0;
    *(uint8_t*)0x2000022c = 0;
    *(uint8_t*)0x2000022d = 0;
    *(uint8_t*)0x2000022e = 0;
    *(uint8_t*)0x2000022f = 0;
    *(uint8_t*)0x20000230 = 0;
    *(uint8_t*)0x20000231 = 0;
    *(uint8_t*)0x20000232 = 0;
    *(uint8_t*)0x20000233 = 0;
    *(uint8_t*)0x20000234 = 0;
    *(uint8_t*)0x20000235 = 0;
    *(uint8_t*)0x20000236 = 0;
    *(uint8_t*)0x20000237 = 0;
    *(uint8_t*)0x20000238 = 0;
    *(uint8_t*)0x20000239 = 0;
    *(uint8_t*)0x2000023a = 0;
    *(uint8_t*)0x2000023b = 0;
    *(uint8_t*)0x2000023c = 0;
    *(uint8_t*)0x2000023d = 0;
    *(uint8_t*)0x2000023e = 0;
    *(uint8_t*)0x2000023f = 0;
    *(uint8_t*)0x20000240 = 0;
    *(uint8_t*)0x20000241 = 0;
    *(uint8_t*)0x20000242 = 0;
    *(uint8_t*)0x20000243 = 0;
    *(uint8_t*)0x20000244 = 0;
    *(uint8_t*)0x20000245 = 0;
    *(uint8_t*)0x20000246 = 0;
    *(uint8_t*)0x20000247 = 0;
    *(uint8_t*)0x20000248 = 0;
    *(uint8_t*)0x20000249 = 0;
    *(uint8_t*)0x2000024a = 0;
    *(uint8_t*)0x2000024b = 0;
    syscall(__NR_ioctl, -1, 0xc08c5334, 0x200001c0);
    break;
  case 4:
    syscall(__NR_socket, 2, 0x800, 1);
    break;
  case 5:
    syscall(__NR_socket, 2, 2, 0x88);
    break;
  case 6:
    res = syscall(__NR_gettid);
    if (res != -1)
      r[1] = res;
    break;
  case 7:
    syscall(__NR_exit, 0);
    break;
  case 8:
    *(uint64_t*)0x20000180 = 0;
    *(uint64_t*)0x20000188 = 0;
    syscall(__NR_prlimit64, r[1], 0, 0x20000180, 0x20000240);
    break;
  case 9:
    syscall(__NR_write, -1, 0x20000000, 0);
    break;
  case 10:
    syscall(__NR_ioctl, -1, 0x8912, 0x400200);
    break;
  case 11:
    *(uint64_t*)0x20000180 = 0;
    *(uint32_t*)0x20000188 = 0;
    *(uint64_t*)0x20000190 = 0x20000000;
    *(uint64_t*)0x20000000 = 0x20000600;
    *(uint64_t*)0x20000008 = 0xfffffe7e;
    *(uint64_t*)0x20000198 = 1;
    *(uint64_t*)0x200001a0 = 0;
    *(uint64_t*)0x200001a8 = 0;
    *(uint32_t*)0x200001b0 = 0;
    syscall(__NR_sendmsg, -1, 0x20000180, 0);
    break;
  case 12:
    *(uint64_t*)0x200001c0 = 0;
    *(uint32_t*)0x200001c8 = 0;
    *(uint64_t*)0x200001d0 = 0x20000000;
    *(uint64_t*)0x20000000 = 0x20000080;
    *(uint64_t*)0x20000008 = 0;
    *(uint64_t*)0x200001d8 = 1;
    *(uint64_t*)0x200001e0 = 0;
    *(uint64_t*)0x200001e8 = 0;
    *(uint32_t*)0x200001f0 = 0;
    syscall(__NR_sendmsg, -1, 0x200001c0, 0);
    break;
  case 13:
    res = syscall(__NR_socket, 0xf, 3, 2);
    if (res != -1)
      r[2] = res;
    break;
  case 14:
    syscall(__NR_sendmmsg, r[2], 0x20000180, 0x400000f, 0);
    break;
  case 15:
    memcpy((void*)0x20b4508a, "/dev/ashmem", 12);
    res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20b4508a, 0, 0);
    if (res != -1)
      r[3] = res;
    break;
  case 16:
    syscall(__NR_ioctl, r[3], 0x40087703, 0xfffffffa);
    break;
  case 17:
    syscall(__NR_mmap, 0x206ff000, 0x3000, 0, 0x12, r[3], 0);
    break;
  case 18:
    memcpy((void*)0x20000040, "\x00\x00\x00\x80\x00\x00\x80\x00\x00", 9);
    syscall(__NR_ioctl, r[3], 0x40087708, 0x20000040);
    break;
  case 19:
    memcpy((void*)0x20000000, "/dev/hwrng", 11);
    syscall(__NR_openat, 0xffffffffffffff9c, 0x20000000, 0, 0);
    break;
  case 20:
    syscall(__NR_ioctl, r[3], 0x770a, 0);
    break;
  case 21:
    syscall(__NR_close, r[3]);
    break;
  case 22:
    res = syscall(__NR_socketpair, 1, 1, 0, 0x20000180);
    if (res != -1)
      r[4] = *(uint32_t*)0x20000184;
    break;
  case 23:
    syscall(__NR_ioctl, r[4], 0x8912, 0x400200);
    break;
  case 24:
    *(uint64_t*)0x20000180 = 0;
    *(uint32_t*)0x20000188 = 0;
    *(uint64_t*)0x20000190 = 0x20000000;
    *(uint64_t*)0x20000000 = 0x20000600;
    *(uint64_t*)0x20000008 = 0xfffffe7e;
    *(uint64_t*)0x20000198 = 1;
    *(uint64_t*)0x200001a0 = 0;
    *(uint64_t*)0x200001a8 = 0;
    *(uint32_t*)0x200001b0 = 0;
    syscall(__NR_sendmsg, -1, 0x20000180, 0);
    break;
  case 25:
    res = syscall(__NR_socket, 0xf, 3, 2);
    if (res != -1)
      r[5] = res;
    break;
  case 26:
    syscall(__NR_sendmmsg, r[5], 0x20000180, 0x400000f, 0);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
