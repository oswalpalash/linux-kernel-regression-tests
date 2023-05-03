// https://syzkaller.appspot.com/bug?id=828a3c71bd344a6de8b6a31233d51a72099f27fd
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

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[1] = {0xffffffffffffffff};
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    memcpy((void*)0x20000100, "/dev/snapshot", 14);
    res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000100, 1, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    *(uint32_t*)0x20001200 = 0;
    *(uint32_t*)0x20001204 = -1;
    syscall(__NR_ioctl, r[0], 0x3305, 0x20001200);
    break;
  case 2:
    syscall(__NR_fcntl, r[0], 9);
    break;
  case 3:
    *(uint32_t*)0x200000c0 = r[0];
    *(uint64_t*)0x200000c8 = 0x20000000;
    *(uint64_t*)0x200000d0 = 0x20000040;
    *(uint64_t*)0x200000d8 = 2;
    syscall(__NR_bpf, 2, 0x200000c0, 0x20);
    break;
  case 4:
    *(uint8_t*)0x20000200 = 0x7f;
    *(uint8_t*)0x20000201 = 0x45;
    *(uint8_t*)0x20000202 = 0x4c;
    *(uint8_t*)0x20000203 = 0x46;
    *(uint8_t*)0x20000204 = 0x20;
    *(uint8_t*)0x20000205 = 0xda;
    *(uint8_t*)0x20000206 = 1;
    *(uint8_t*)0x20000207 = 3;
    *(uint64_t*)0x20000208 = 0x3f;
    *(uint16_t*)0x20000210 = 2;
    *(uint16_t*)0x20000212 = 0x3e;
    *(uint32_t*)0x20000214 = 0x10001;
    *(uint32_t*)0x20000218 = 0x31;
    *(uint32_t*)0x2000021c = 0x38;
    *(uint32_t*)0x20000220 = 0;
    *(uint32_t*)0x20000224 = 3;
    *(uint16_t*)0x20000228 = 5;
    *(uint16_t*)0x2000022a = 0x20;
    *(uint16_t*)0x2000022c = 2;
    *(uint16_t*)0x2000022e = 3;
    *(uint16_t*)0x20000230 = 6;
    *(uint16_t*)0x20000232 = 4;
    *(uint32_t*)0x20000238 = 1;
    *(uint32_t*)0x2000023c = 1;
    *(uint32_t*)0x20000240 = 7;
    *(uint32_t*)0x20000244 = 1;
    *(uint32_t*)0x20000248 = 3;
    *(uint32_t*)0x2000024c = 0x10001;
    *(uint32_t*)0x20000250 = 6;
    *(uint32_t*)0x20000254 = 0x36e;
    *(uint32_t*)0x20000258 = 0x6474e553;
    *(uint32_t*)0x2000025c = 4;
    *(uint32_t*)0x20000260 = 4;
    *(uint32_t*)0x20000264 = 2;
    *(uint32_t*)0x20000268 = 0x1000;
    *(uint32_t*)0x2000026c = 0x80000001;
    *(uint32_t*)0x20000270 = 9;
    *(uint32_t*)0x20000274 = 7;
    *(uint64_t*)0x20000278 = 0;
    *(uint64_t*)0x20000280 = 0;
    *(uint64_t*)0x20000288 = 0;
    *(uint64_t*)0x20000290 = 0;
    *(uint64_t*)0x20000298 = 0;
    *(uint64_t*)0x200002a0 = 0;
    *(uint64_t*)0x200002a8 = 0;
    *(uint64_t*)0x200002b0 = 0;
    *(uint64_t*)0x200002b8 = 0;
    *(uint64_t*)0x200002c0 = 0;
    *(uint64_t*)0x200002c8 = 0;
    *(uint64_t*)0x200002d0 = 0;
    *(uint64_t*)0x200002d8 = 0;
    *(uint64_t*)0x200002e0 = 0;
    *(uint64_t*)0x200002e8 = 0;
    *(uint64_t*)0x200002f0 = 0;
    *(uint64_t*)0x200002f8 = 0;
    *(uint64_t*)0x20000300 = 0;
    *(uint64_t*)0x20000308 = 0;
    *(uint64_t*)0x20000310 = 0;
    *(uint64_t*)0x20000318 = 0;
    *(uint64_t*)0x20000320 = 0;
    *(uint64_t*)0x20000328 = 0;
    *(uint64_t*)0x20000330 = 0;
    *(uint64_t*)0x20000338 = 0;
    *(uint64_t*)0x20000340 = 0;
    *(uint64_t*)0x20000348 = 0;
    *(uint64_t*)0x20000350 = 0;
    *(uint64_t*)0x20000358 = 0;
    *(uint64_t*)0x20000360 = 0;
    *(uint64_t*)0x20000368 = 0;
    *(uint64_t*)0x20000370 = 0;
    *(uint64_t*)0x20000378 = 0;
    *(uint64_t*)0x20000380 = 0;
    *(uint64_t*)0x20000388 = 0;
    *(uint64_t*)0x20000390 = 0;
    *(uint64_t*)0x20000398 = 0;
    *(uint64_t*)0x200003a0 = 0;
    *(uint64_t*)0x200003a8 = 0;
    *(uint64_t*)0x200003b0 = 0;
    *(uint64_t*)0x200003b8 = 0;
    *(uint64_t*)0x200003c0 = 0;
    *(uint64_t*)0x200003c8 = 0;
    *(uint64_t*)0x200003d0 = 0;
    *(uint64_t*)0x200003d8 = 0;
    *(uint64_t*)0x200003e0 = 0;
    *(uint64_t*)0x200003e8 = 0;
    *(uint64_t*)0x200003f0 = 0;
    *(uint64_t*)0x200003f8 = 0;
    *(uint64_t*)0x20000400 = 0;
    *(uint64_t*)0x20000408 = 0;
    *(uint64_t*)0x20000410 = 0;
    *(uint64_t*)0x20000418 = 0;
    *(uint64_t*)0x20000420 = 0;
    *(uint64_t*)0x20000428 = 0;
    *(uint64_t*)0x20000430 = 0;
    *(uint64_t*)0x20000438 = 0;
    *(uint64_t*)0x20000440 = 0;
    *(uint64_t*)0x20000448 = 0;
    *(uint64_t*)0x20000450 = 0;
    *(uint64_t*)0x20000458 = 0;
    *(uint64_t*)0x20000460 = 0;
    *(uint64_t*)0x20000468 = 0;
    *(uint64_t*)0x20000470 = 0;
    *(uint64_t*)0x20000478 = 0;
    *(uint64_t*)0x20000480 = 0;
    *(uint64_t*)0x20000488 = 0;
    *(uint64_t*)0x20000490 = 0;
    *(uint64_t*)0x20000498 = 0;
    *(uint64_t*)0x200004a0 = 0;
    *(uint64_t*)0x200004a8 = 0;
    *(uint64_t*)0x200004b0 = 0;
    *(uint64_t*)0x200004b8 = 0;
    *(uint64_t*)0x200004c0 = 0;
    *(uint64_t*)0x200004c8 = 0;
    *(uint64_t*)0x200004d0 = 0;
    *(uint64_t*)0x200004d8 = 0;
    *(uint64_t*)0x200004e0 = 0;
    *(uint64_t*)0x200004e8 = 0;
    *(uint64_t*)0x200004f0 = 0;
    *(uint64_t*)0x200004f8 = 0;
    *(uint64_t*)0x20000500 = 0;
    *(uint64_t*)0x20000508 = 0;
    *(uint64_t*)0x20000510 = 0;
    *(uint64_t*)0x20000518 = 0;
    *(uint64_t*)0x20000520 = 0;
    *(uint64_t*)0x20000528 = 0;
    *(uint64_t*)0x20000530 = 0;
    *(uint64_t*)0x20000538 = 0;
    *(uint64_t*)0x20000540 = 0;
    *(uint64_t*)0x20000548 = 0;
    *(uint64_t*)0x20000550 = 0;
    *(uint64_t*)0x20000558 = 0;
    *(uint64_t*)0x20000560 = 0;
    *(uint64_t*)0x20000568 = 0;
    *(uint64_t*)0x20000570 = 0;
    *(uint64_t*)0x20000578 = 0;
    *(uint64_t*)0x20000580 = 0;
    *(uint64_t*)0x20000588 = 0;
    *(uint64_t*)0x20000590 = 0;
    *(uint64_t*)0x20000598 = 0;
    *(uint64_t*)0x200005a0 = 0;
    *(uint64_t*)0x200005a8 = 0;
    *(uint64_t*)0x200005b0 = 0;
    *(uint64_t*)0x200005b8 = 0;
    *(uint64_t*)0x200005c0 = 0;
    *(uint64_t*)0x200005c8 = 0;
    *(uint64_t*)0x200005d0 = 0;
    *(uint64_t*)0x200005d8 = 0;
    *(uint64_t*)0x200005e0 = 0;
    *(uint64_t*)0x200005e8 = 0;
    *(uint64_t*)0x200005f0 = 0;
    *(uint64_t*)0x200005f8 = 0;
    *(uint64_t*)0x20000600 = 0;
    *(uint64_t*)0x20000608 = 0;
    *(uint64_t*)0x20000610 = 0;
    *(uint64_t*)0x20000618 = 0;
    *(uint64_t*)0x20000620 = 0;
    *(uint64_t*)0x20000628 = 0;
    *(uint64_t*)0x20000630 = 0;
    *(uint64_t*)0x20000638 = 0;
    *(uint64_t*)0x20000640 = 0;
    *(uint64_t*)0x20000648 = 0;
    *(uint64_t*)0x20000650 = 0;
    *(uint64_t*)0x20000658 = 0;
    *(uint64_t*)0x20000660 = 0;
    *(uint64_t*)0x20000668 = 0;
    *(uint64_t*)0x20000670 = 0;
    syscall(__NR_write, r[0], 0x20000200, 0x478);
    break;
  }
}

void loop()
{
  execute(5);
  collide = 1;
  execute(5);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
