// https://syzkaller.appspot.com/bug?id=ee0dc3e7ce224b8359225bb7da0c168cf8c8283d
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

uint64_t r[3] = {0xffffffffffffffff, 0xffffffff, 0xffffffff};
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    memcpy((void*)0x20000180, "/dev/infiniband/rdma_cm", 24);
    res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000180, 2, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    *(uint64_t*)0x20000200 = 0x200004c0;
    *(uint32_t*)0x200004c0 = -1;
    res = syscall(__NR_write, r[0], 0x20000200, 8);
    if (res != -1)
      r[1] = *(uint32_t*)0x200004c0;
    break;
  case 2:
    *(uint32_t*)0x20000540 = 3;
    *(uint16_t*)0x20000544 = 0x40;
    *(uint16_t*)0x20000546 = 0xfa00;
    *(uint16_t*)0x20000548 = 0xa;
    *(uint16_t*)0x2000054a = htobe16(0);
    *(uint32_t*)0x2000054c = 0;
    *(uint8_t*)0x20000550 = 0;
    *(uint8_t*)0x20000551 = 0;
    *(uint8_t*)0x20000552 = 0;
    *(uint8_t*)0x20000553 = 0;
    *(uint8_t*)0x20000554 = 0;
    *(uint8_t*)0x20000555 = 0;
    *(uint8_t*)0x20000556 = 0;
    *(uint8_t*)0x20000557 = 0;
    *(uint8_t*)0x20000558 = 0;
    *(uint8_t*)0x20000559 = 0;
    *(uint8_t*)0x2000055a = 0;
    *(uint8_t*)0x2000055b = 0;
    *(uint8_t*)0x2000055c = 0;
    *(uint8_t*)0x2000055d = 0;
    *(uint8_t*)0x2000055e = 0;
    *(uint8_t*)0x2000055f = 0;
    *(uint32_t*)0x20000560 = 0;
    *(uint16_t*)0x20000564 = 0xa;
    *(uint16_t*)0x20000566 = htobe16(0);
    *(uint32_t*)0x20000568 = 0;
    *(uint8_t*)0x2000056c = 0xfe;
    *(uint8_t*)0x2000056d = 0x80;
    *(uint8_t*)0x2000056e = 0;
    *(uint8_t*)0x2000056f = 0;
    *(uint8_t*)0x20000570 = 0;
    *(uint8_t*)0x20000571 = 0;
    *(uint8_t*)0x20000572 = 0;
    *(uint8_t*)0x20000573 = 0;
    *(uint8_t*)0x20000574 = 0;
    *(uint8_t*)0x20000575 = 0;
    *(uint8_t*)0x20000576 = 0;
    *(uint8_t*)0x20000577 = 0;
    *(uint8_t*)0x20000578 = 0;
    *(uint8_t*)0x20000579 = 0;
    *(uint8_t*)0x2000057a = 0;
    *(uint8_t*)0x2000057b = 0xbb;
    *(uint32_t*)0x2000057c = 0;
    *(uint32_t*)0x20000580 = r[1];
    *(uint32_t*)0x20000584 = 0;
    syscall(__NR_write, r[0], 0x20000540, 0x48);
    break;
  case 3:
    *(uint32_t*)0x20000fc0 = 0;
    *(uint16_t*)0x20000fc4 = 0x18;
    *(uint16_t*)0x20000fc6 = 0xfa00;
    *(uint64_t*)0x20000fc8 = 3;
    *(uint64_t*)0x20000fd0 = 0x20000f80;
    *(uint16_t*)0x20000fd8 = 0x106;
    *(uint8_t*)0x20000fda = 0xb;
    *(uint8_t*)0x20000fdb = 0;
    *(uint8_t*)0x20000fdc = 0;
    *(uint8_t*)0x20000fdd = 0;
    *(uint8_t*)0x20000fde = 0;
    *(uint8_t*)0x20000fdf = 0;
    syscall(__NR_write, r[0], 0x20000fc0, 0x20);
    break;
  case 4:
    *(uint32_t*)0x20002280 = 0;
    *(uint16_t*)0x20002284 = 0x18;
    *(uint16_t*)0x20002286 = 0xfa00;
    *(uint64_t*)0x20002288 = 1;
    *(uint64_t*)0x20002290 = 0x20002240;
    *(uint16_t*)0x20002298 = 0;
    *(uint8_t*)0x2000229a = 0;
    *(uint8_t*)0x2000229b = 0;
    *(uint8_t*)0x2000229c = 0;
    *(uint8_t*)0x2000229d = 0;
    *(uint8_t*)0x2000229e = 0;
    *(uint8_t*)0x2000229f = 0;
    syscall(__NR_write, r[0], 0x20002280, 0x20);
    break;
  case 5:
    *(uint32_t*)0x20004200 = 0;
    *(uint16_t*)0x20004204 = 0x18;
    *(uint16_t*)0x20004206 = 0xfa00;
    *(uint64_t*)0x20004208 = 2;
    *(uint64_t*)0x20004210 = 0x200041c0;
    *(uint16_t*)0x20004218 = 0x13f;
    *(uint8_t*)0x2000421a = 9;
    *(uint8_t*)0x2000421b = 0;
    *(uint8_t*)0x2000421c = 0;
    *(uint8_t*)0x2000421d = 0;
    *(uint8_t*)0x2000421e = 0;
    *(uint8_t*)0x2000421f = 0;
    res = syscall(__NR_write, r[0], 0x20004200, 0x20);
    if (res != -1)
      r[2] = *(uint32_t*)0x200041c0;
    break;
  case 6:
    *(uint32_t*)0x200022c0 = 0x13;
    *(uint16_t*)0x200022c4 = 0x10;
    *(uint16_t*)0x200022c6 = 0xfa00;
    *(uint64_t*)0x200022c8 = 0x20002400;
    *(uint32_t*)0x200022d0 = r[2];
    *(uint32_t*)0x200022d4 = 0;
    syscall(__NR_write, r[0], 0x200022c0, 0x18);
    break;
  case 7:
    syscall(__NR_close, r[0]);
    break;
  }
}

void test()
{
  execute(8);
  collide = 1;
  execute(8);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    loop();
  }
}
