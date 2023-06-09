// https://syzkaller.appspot.com/bug?id=38fd481002903a9f0a8c861cc142ff5180eaf444
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <fcntl.h>
#include <linux/futex.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

static uintptr_t syz_open_dev(uintptr_t a0, uintptr_t a1, uintptr_t a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf));
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

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

#ifndef __NR_memfd_create
#define __NR_memfd_create 319
#endif

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    memcpy((void*)0x20000200, "/dev/loop#", 11);
    res = syz_open_dev(0x20000200, 0, 0x82);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    memcpy((void*)0x200002c0,
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x76"
           "\x8e\x05\xf7\xc1\x55\xad\x7d\xc6\x94\x7c\x57\x3e\x5a\x69\x24\x4e"
           "\x76\x38\x2c\x0a\xa6\x3d\x57\x5e\xa3\x59\x7f\x8b\x17\x28\x27\x7e"
           "\xf7\x6b\x30\x54\x4d\x7b\xa9\x2d\xcf\x97\x8f\x1f\x81\xdc\x1b\x7f"
           "\x8f\x7b\x34\x51\xda\xda\x02\xec\xb4\xf1\xdd\xcc\x8b\x52\x41\xda"
           "\x89\x45\x66\x6e\x00\x73\xc2\x5a\x62\x87\xc6\x4d\xbe\xa3\x7a",
           95);
    res = syscall(__NR_memfd_create, 0x200002c0, 0);
    if (res != -1)
      r[1] = res;
    break;
  case 2:
    *(uint64_t*)0x20000340 = 0x200001c0;
    memcpy((void*)0x200001c0, "'", 1);
    *(uint64_t*)0x20000348 = 1;
    syscall(__NR_pwritev, r[1], 0x20000340, 1, 0x81806);
    break;
  case 3:
    syscall(__NR_ioctl, r[1], 0x4c00, r[0]);
    break;
  case 4:
    syscall(__NR_ioctl, -1, 0x4c07);
    break;
  case 5:
    *(uint64_t*)0x20000240 = 0;
    syscall(__NR_sendfile, r[0], r[0], 0x20000240, 0x20000102000007);
    break;
  case 6:
    syscall(__NR_ioctl, r[0], 0x4c01);
    break;
  case 7:
    syscall(__NR_ioctl, r[0], 0x4c00, r[1]);
    break;
  }
}

void execute_one()
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
