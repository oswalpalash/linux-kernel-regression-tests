// https://syzkaller.appspot.com/bug?id=02bf0cd7047b7943f67dcb46ac31ff62403ca7b2
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    res = syscall(__NR_socketpair, 0, 0, 0, 0x20000140);
    if (res != -1)
      r[0] = *(uint32_t*)0x20000140;
    break;
  case 1:
    syscall(__NR_socket, 0xa, 1, 0);
    break;
  case 2:
    *(uint32_t*)0x20000080 = 0x12;
    *(uint32_t*)0x20000084 = 6;
    *(uint32_t*)0x20000088 = 4;
    *(uint32_t*)0x2000008c = 9;
    *(uint32_t*)0x20000090 = 0;
    *(uint32_t*)0x20000094 = 1;
    *(uint32_t*)0x20000098 = 0;
    *(uint8_t*)0x2000009c = 0;
    *(uint8_t*)0x2000009d = 0;
    *(uint8_t*)0x2000009e = 0;
    *(uint8_t*)0x2000009f = 0;
    *(uint8_t*)0x200000a0 = 0;
    *(uint8_t*)0x200000a1 = 0;
    *(uint8_t*)0x200000a2 = 0;
    *(uint8_t*)0x200000a3 = 0;
    *(uint8_t*)0x200000a4 = 0;
    *(uint8_t*)0x200000a5 = 0;
    *(uint8_t*)0x200000a6 = 0;
    *(uint8_t*)0x200000a7 = 0;
    *(uint8_t*)0x200000a8 = 0;
    *(uint8_t*)0x200000a9 = 0;
    *(uint8_t*)0x200000aa = 0;
    *(uint8_t*)0x200000ab = 0;
    res = syscall(__NR_bpf, 0, 0x20000080, 0x2c);
    if (res != -1)
      r[1] = res;
    break;
  case 3:
    *(uint32_t*)0x20000180 = r[1];
    *(uint64_t*)0x20000188 = 0x20000000;
    *(uint64_t*)0x20000190 = 0x20000140;
    *(uint64_t*)0x20000198 = 0;
    syscall(__NR_bpf, 2, 0x20000180, 0x20);
    break;
  case 4:
    *(uint32_t*)0x20000000 = r[0];
    *(uint32_t*)0x20000004 = 0;
    *(uint32_t*)0x20000008 = 0;
    *(uint32_t*)0x2000000c = 0xab;
    *(uint64_t*)0x20000010 = 0x20000200;
    *(uint64_t*)0x20000018 = 0x20001200;
    *(uint32_t*)0x20000020 = 6;
    *(uint32_t*)0x20000024 = 0;
    syscall(__NR_bpf, 0xa, 0x20000000, 0x28);
    break;
  case 5:
    *(uint32_t*)0x200001c0 = r[1];
    *(uint64_t*)0x200001c8 = 0x20000040;
    syscall(__NR_bpf, 3, 0x200001c0, 0x10);
    break;
  }
}

void loop()
{
  execute(6);
  collide = 1;
  execute(6);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
