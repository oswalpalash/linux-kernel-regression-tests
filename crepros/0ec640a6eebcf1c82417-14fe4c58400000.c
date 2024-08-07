// https://syzkaller.appspot.com/bug?id=ce80a10f7234f816218d7c257396c00d575b52d8
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <linux/futex.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

static void execute_one();
extern unsigned long long procid;

void loop()
{
  while (1) {
    execute_one();
  }
}

struct thread_t {
  int created, running, call;
  pthread_t th;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

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
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 20 * 1000 * 1000;
        syscall(SYS_futex, &th->running, FUTEX_WAIT, 1, &ts);
        if (__atomic_load_n(&running, __ATOMIC_RELAXED))
          usleep((call == num_calls - 1) ? 10000 : 1000);
        break;
      }
    }
  }
}

#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_mkdir
#define __NR_mkdir 39
#endif
#ifndef __NR_mount
#define __NR_mount 21
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

unsigned long long procid;
void execute_call(int call)
{
  switch (call) {
  case 0:
    memcpy((void*)0x20000100, "./file0", 8);
    syscall(__NR_mkdir, 0x20000100, 0x80);
    break;
  case 1:
    memcpy((void*)0x200000c0, "./file0", 8);
    memcpy((void*)0x20000340, "9p", 3);
    syscall(__NR_mount, 0, 0x200000c0, 0x20000340, 0, 0x20000180);
    break;
  case 2:
    memcpy((void*)0x20000000, "127.0.0.1", 10);
    memcpy((void*)0x20000040, "./file0", 8);
    memcpy((void*)0x20000080, "9p", 3);
    memcpy((void*)0x20000180, "trans=tcp,", 10);
    memcpy((void*)0x2000018a, "port", 4);
    *(uint8_t*)0x2000018e = 0x3d;
    sprintf((char*)0x2000018f, "0x%016llx", (long long)0x4e22);
    *(uint8_t*)0x200001a1 = 0x2c;
    memcpy((void*)0x200001a2, "cache=fscache", 13);
    *(uint8_t*)0x200001af = 0x2c;
    memcpy((void*)0x200001b0, "uname", 5);
    *(uint8_t*)0x200001b5 = 0x3d;
    memcpy((void*)0x200001b6, "9p", 3);
    *(uint8_t*)0x200001b9 = 0x2c;
    memcpy((void*)0x200001ba, "access=any", 10);
    *(uint8_t*)0x200001c4 = 0x2c;
    memcpy((void*)0x200001c5, "cache=none", 10);
    *(uint8_t*)0x200001cf = 0x2c;
    memcpy((void*)0x200001d0, "cache=loose", 11);
    *(uint8_t*)0x200001db = 0x2c;
    memcpy((void*)0x200001dc, "cache=fscache", 13);
    *(uint8_t*)0x200001e9 = 0x2c;
    *(uint8_t*)0x200001ea = 0;
    syscall(__NR_mount, 0x20000000, 0x20000040, 0x20000080, 0x8400, 0x20000180);
    break;
  }
}

void execute_one()
{
  execute(3);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      for (;;) {
        loop();
      }
    }
  }
  sleep(1000000);
  return 0;
}
