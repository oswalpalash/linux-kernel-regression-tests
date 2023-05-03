// https://syzkaller.appspot.com/bug?id=6225e7b911f6977538f7bd519ba9811d9fc2cb94
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <fcntl.h>
#include <linux/futex.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}

#include <setjmp.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>
#include <string.h>

static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* uctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  if (__atomic_load_n(&skip_segv, __ATOMIC_RELAXED) &&
      (addr < prog_start || addr > prog_end)) {
    _longjmp(segv_env, 1);
  }
  doexit(sig);
}

static void install_segv_handler()
{
  struct sigaction sa;

  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_IGN;
  syscall(SYS_rt_sigaction, 0x20, &sa, NULL, 8);
  syscall(SYS_rt_sigaction, 0x21, &sa, NULL, 8);

  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = segv_handler;
  sa.sa_flags = SA_NODEFER | SA_SIGINFO;
  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGBUS, &sa, NULL);
}

#define NONFAILING(...)                                                \
  {                                                                    \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);               \
    if (_setjmp(segv_env) == 0) {                                      \
      __VA_ARGS__;                                                     \
    }                                                                  \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);               \
  }

static uintptr_t syz_open_dev(uintptr_t a0, uintptr_t a1, uintptr_t a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block",
            (uint8_t)a1, (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    NONFAILING(strncpy(buf, (char*)a0, sizeof(buf)));
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

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

long r[3];
void execute_call(int call)
{
  switch (call) {
  case 0:
    syscall(__NR_mmap, 0x20000000, 0x1000, 0x3, 0x32, 0xffffffff, 0x0);
    break;
  case 1:
    NONFAILING(memcpy((void*)0x20000ff3, "/dev/usbmon#", 13));
    r[0] = syz_open_dev(0x20000ff3, 0x8, 0x0);
    break;
  case 2:
    NONFAILING(*(uint32_t*)0x20d15ff8 = 0x0);
    NONFAILING(*(uint32_t*)0x20d15ffc = 0x7fff);
    NONFAILING(*(uint32_t*)0x2019e7f9 = 0x8);
    if (syscall(__NR_getsockopt, 0xffffff9c, 0x84, 0x71, 0x20d15ff8,
                0x2019e7f9) != -1) {
      NONFAILING(r[1] = *(uint32_t*)0x20d15ff8);
    }
    break;
  case 3:
    syscall(__NR_mmap, 0x20000000, 0x1000, 0x3, 0x32, 0xffffffff, 0x0);
    break;
  case 4:
    NONFAILING(*(uint16_t*)0x20000000 = 0x71);
    NONFAILING(*(uint16_t*)0x20000002 = 0x40);
    NONFAILING(*(uint16_t*)0x20000004 = 0x8002);
    NONFAILING(*(uint32_t*)0x20000008 = 0x6);
    NONFAILING(*(uint32_t*)0x2000000c = 0x20);
    NONFAILING(*(uint32_t*)0x20000010 = 0xe880);
    NONFAILING(*(uint32_t*)0x20000014 = 0x7fff);
    NONFAILING(*(uint32_t*)0x20000018 = 0x3);
    NONFAILING(*(uint32_t*)0x2000001c = r[1]);
    syscall(__NR_setsockopt, r[0], 0x84, 0xa, 0x20000000, 0x20);
    break;
  case 5:
    syscall(__NR_mmap, 0x20000000, 0xfff000, 0x3, 0x32, 0xffffffff,
            0x0);
    break;
  case 6:
    NONFAILING(memcpy((void*)0x208be000, "/dev/usbmon#", 13));
    r[2] = syz_open_dev(0x208be000, 0x0, 0x0);
    break;
  case 7:
    syscall(__NR_mmap, 0x20ac6000, 0x4000, 0x1000004, 0x8011, r[2],
            0x0);
    break;
  case 8:
    syscall(__NR_ioctl, r[2], 0x9204, 0xf0b1);
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
  install_segv_handler();
  loop();
  return 0;
}
