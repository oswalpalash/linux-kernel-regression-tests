// https://syzkaller.appspot.com/bug?id=2b6a5e7ed9c189aadc974fc5ff168b131c005947
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <linux/futex.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <stdint.h>
#include <string.h>

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
    for (thread = 0; thread < sizeof(threads) / sizeof(threads[0]);
         thread++) {
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
#ifndef __NR_ioctl
#define __NR_ioctl 54
#endif
#ifndef __NR_openat
#define __NR_openat 295
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[3];
uint64_t procid;
void execute_call(int call)
{
  switch (call) {
  case 0:
    syscall(__NR_mmap, 0x20000000, 0xfff000, 0x3, 0x32, 0xffffffff,
            0x0);
    break;
  case 1:
    *(uint8_t*)0x20000000 = 0x4;
    *(uint8_t*)0x20000001 = 0x1;
    memcpy((void*)0x20000002,
           "\x70\x6f\x72\x74\x31\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00"
           "\x00\x0b\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00",
           64);
    *(uint32_t*)0x20000044 = 0x0;
    *(uint32_t*)0x20000048 = 0x41;
    *(uint32_t*)0x2000004c = 0x0;
    *(uint32_t*)0x20000050 = 0xff5;
    *(uint32_t*)0x20000054 = 0x0;
    *(uint32_t*)0x20000058 = 0x0;
    *(uint32_t*)0x2000005c = 0x0;
    *(uint32_t*)0x20000060 = 0x0;
    *(uint32_t*)0x20000064 = 0x0;
    *(uint32_t*)0x20000068 = 0x0;
    *(uint8_t*)0x2000006c = 0x0;
    *(uint8_t*)0x2000006d = 0x0;
    *(uint8_t*)0x2000006e = 0x0;
    *(uint8_t*)0x2000006f = 0x0;
    *(uint8_t*)0x20000070 = 0x0;
    *(uint8_t*)0x20000071 = 0x0;
    *(uint8_t*)0x20000072 = 0x0;
    *(uint8_t*)0x20000073 = 0x0;
    *(uint8_t*)0x20000074 = 0x0;
    *(uint8_t*)0x20000075 = 0x0;
    *(uint8_t*)0x20000076 = 0x0;
    *(uint8_t*)0x20000077 = 0x0;
    *(uint8_t*)0x20000078 = 0x0;
    *(uint8_t*)0x20000079 = 0x0;
    *(uint8_t*)0x2000007a = 0x0;
    *(uint8_t*)0x2000007b = 0x0;
    *(uint8_t*)0x2000007c = 0x0;
    *(uint8_t*)0x2000007d = 0x0;
    *(uint8_t*)0x2000007e = 0x0;
    *(uint8_t*)0x2000007f = 0x0;
    *(uint8_t*)0x20000080 = 0x0;
    *(uint8_t*)0x20000081 = 0x0;
    *(uint8_t*)0x20000082 = 0x0;
    *(uint8_t*)0x20000083 = 0x0;
    *(uint8_t*)0x20000084 = 0x0;
    *(uint8_t*)0x20000085 = 0x0;
    *(uint8_t*)0x20000086 = 0x0;
    *(uint8_t*)0x20000087 = 0x0;
    *(uint8_t*)0x20000088 = 0x0;
    *(uint8_t*)0x20000089 = 0x0;
    *(uint8_t*)0x2000008a = 0x0;
    *(uint8_t*)0x2000008b = 0x0;
    *(uint8_t*)0x2000008c = 0x0;
    *(uint8_t*)0x2000008d = 0x0;
    *(uint8_t*)0x2000008e = 0x0;
    *(uint8_t*)0x2000008f = 0x0;
    *(uint8_t*)0x20000090 = 0x0;
    *(uint8_t*)0x20000091 = 0x0;
    *(uint8_t*)0x20000092 = 0x0;
    *(uint8_t*)0x20000093 = 0x0;
    *(uint8_t*)0x20000094 = 0x0;
    *(uint8_t*)0x20000095 = 0x0;
    *(uint8_t*)0x20000096 = 0x0;
    *(uint8_t*)0x20000097 = 0x0;
    *(uint8_t*)0x20000098 = 0x0;
    *(uint8_t*)0x20000099 = 0x0;
    *(uint8_t*)0x2000009a = 0x0;
    *(uint8_t*)0x2000009b = 0x0;
    *(uint8_t*)0x2000009c = 0x0;
    *(uint8_t*)0x2000009d = 0x0;
    *(uint8_t*)0x2000009e = 0x0;
    *(uint8_t*)0x2000009f = 0x0;
    *(uint8_t*)0x200000a0 = 0x0;
    *(uint8_t*)0x200000a1 = 0x0;
    *(uint8_t*)0x200000a2 = 0x0;
    *(uint8_t*)0x200000a3 = 0x0;
    *(uint8_t*)0x200000a4 = 0x0;
    *(uint8_t*)0x200000a5 = 0x0;
    *(uint8_t*)0x200000a6 = 0x0;
    syscall(__NR_ioctl, 0xffffffff, 0x40a45323, 0x20000000);
    break;
  case 2:
    memcpy((void*)0x202bf000, "/dev/kvm", 9);
    r[0] = syscall(__NR_openat, 0xffffff9c, 0x202bf000, 0x0, 0x0);
    break;
  case 3:
    r[1] = syscall(__NR_ioctl, r[0], 0xae01, 0x0);
    break;
  case 4:
    r[2] = syscall(__NR_ioctl, r[1], 0xae41, 0x0);
    break;
  case 5:
    *(uint32_t*)0x20001000 = 0x10005;
    *(uint32_t*)0x20001004 = 0x0;
    *(uint64_t*)0x20001008 = 0x0;
    *(uint64_t*)0x20001010 = 0x2000;
    *(uint32_t*)0x20001018 = 0x20000000;
    syscall(__NR_ioctl, r[1], 0x4020ae46, 0x20001000);
    break;
  case 6:
    syscall(__NR_ioctl, r[2], 0xaeb7);
    break;
  case 7:
    syscall(__NR_ioctl, r[2], 0xae80, 0x0);
    break;
  case 8:
    syscall(__NR_ioctl, r[2], 0xae80, 0x0);
    break;
  }
}

void test()
{
  memset(r, -1, sizeof(r));
  execute(9);
  collide = 1;
  execute(9);
}

int main()
{
  int i;
  for (i = 0; i < 8; i++) {
    if (fork() == 0) {
      procid = i;
      loop();
      return 0;
    }
  }
  sleep(1000000);
  return 0;
}
