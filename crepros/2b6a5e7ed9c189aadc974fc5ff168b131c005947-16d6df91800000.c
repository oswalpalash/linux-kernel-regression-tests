// https://syzkaller.appspot.com/bug?id=2b6a5e7ed9c189aadc974fc5ff168b131c005947
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

long r[3];
uint64_t procid;
void execute_call(int call)
{
  switch (call) {
  case 0:
    syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
    break;
  case 1:
    memcpy((void*)0x2099fff7, "/dev/kvm", 9);
    r[0] = syscall(__NR_openat, 0xffffffffffffff9c, 0x2099fff7, 0, 0);
    break;
  case 2:
    r[1] = syscall(__NR_ioctl, r[0], 0xae01, 0);
    break;
  case 3:
    r[2] = syscall(__NR_ioctl, r[1], 0xae41, 0);
    break;
  case 4:
    *(uint32_t*)0x20c18000 = 0x10000;
    *(uint32_t*)0x20c18004 = 0;
    *(uint64_t*)0x20c18008 = 0;
    *(uint64_t*)0x20c18010 = 0x1000;
    *(uint64_t*)0x20c18018 = 0x20a98000;
    syscall(__NR_ioctl, r[1], 0x4020ae46, 0x20c18000);
    break;
  case 5:
    syscall(__NR_ioctl, r[2], 0xaeb7);
    break;
  case 6:
    *(uint32_t*)0x20609000 = 0x3fffd;
    *(uint32_t*)0x20609004 = 0;
    *(uint64_t*)0x20609008 = 0;
    *(uint64_t*)0x20609010 = 0;
    *(uint64_t*)0x20609018 = 0;
    *(uint64_t*)0x20609020 = 3;
    *(uint64_t*)0x20609028 = 1;
    *(uint64_t*)0x20609030 = 0;
    *(uint64_t*)0x20609038 = 0;
    *(uint64_t*)0x20609040 = 3;
    syscall(__NR_ioctl, r[2], 0x4048ae9b, 0x20609000);
    break;
  case 7:
    syscall(__NR_ioctl, r[2], 0xae80, 0);
    break;
  case 8:
    syscall(__NR_ioctl, r[2], 0xae80, 0);
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
