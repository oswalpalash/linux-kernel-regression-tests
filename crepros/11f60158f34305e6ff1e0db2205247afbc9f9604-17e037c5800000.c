// https://syzkaller.appspot.com/bug?id=11f60158f34305e6ff1e0db2205247afbc9f9604
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <linux/futex.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}
#include <stdint.h>
#include <string.h>

const int kFailStatus = 67;
const int kRetryStatus = 69;

static void fail(const char* msg, ...)
{
  int e = errno;
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit((e == ENOMEM || e == EAGAIN) ? kRetryStatus : kFailStatus);
}

static uint64_t current_time_ms()
{
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    fail("clock_gettime failed");
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void test();

void loop()
{
  int iter;
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      fail("loop fork failed");
    if (pid == 0) {
      prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
      setpgrp();
      test();
      doexit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      int res = waitpid(-1, &status, __WALL | WNOHANG);
      if (res == pid)
        break;
      usleep(1000);
      if (current_time_ms() - start > 5 * 1000) {
        kill(-pid, SIGKILL);
        kill(pid, SIGKILL);
        while (waitpid(-1, &status, __WALL) != pid) {
        }
        break;
      }
    }
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

long r[2];
void execute_call(int call)
{
  switch (call) {
  case 0:
    syscall(__NR_mmap, 0x20000000, 0xe78000, 3, 0x32, -1, 0);
    break;
  case 1:
    r[0] = syscall(__NR_socket, 0xa, 2, 0);
    break;
  case 2:
    r[1] = syscall(__NR_socket, 0x18, 1, 1);
    break;
  case 3:
    *(uint16_t*)0x205fafd2 = 0x18;
    *(uint32_t*)0x205fafd4 = 1;
    *(uint32_t*)0x205fafd8 = 0;
    *(uint32_t*)0x205fafdc = r[0];
    *(uint16_t*)0x205fafe0 = 2;
    *(uint16_t*)0x205fafe2 = htobe16(0x4e21);
    *(uint32_t*)0x205fafe4 = htobe32(0xe0000002);
    *(uint8_t*)0x205fafe8 = 0;
    *(uint8_t*)0x205fafe9 = 0;
    *(uint8_t*)0x205fafea = 0;
    *(uint8_t*)0x205fafeb = 0;
    *(uint8_t*)0x205fafec = 0;
    *(uint8_t*)0x205fafed = 0;
    *(uint8_t*)0x205fafee = 0;
    *(uint8_t*)0x205fafef = 0;
    *(uint32_t*)0x205faff0 = 4;
    *(uint32_t*)0x205faff4 = 0;
    *(uint32_t*)0x205faff8 = 2;
    *(uint32_t*)0x205faffc = 0;
    syscall(__NR_connect, r[1], 0x205fafd2, 0x2e);
    break;
  case 4:
    *(uint16_t*)0x208f0000 = 0xa;
    *(uint16_t*)0x208f0002 = htobe16(0x4e21);
    *(uint32_t*)0x208f0004 = 0x200;
    *(uint8_t*)0x208f0008 = -1;
    *(uint8_t*)0x208f0009 = 2;
    *(uint8_t*)0x208f000a = 0;
    *(uint8_t*)0x208f000b = 0;
    *(uint8_t*)0x208f000c = 0;
    *(uint8_t*)0x208f000d = 0;
    *(uint8_t*)0x208f000e = 0;
    *(uint8_t*)0x208f000f = 0;
    *(uint8_t*)0x208f0010 = 0;
    *(uint8_t*)0x208f0011 = 0;
    *(uint8_t*)0x208f0012 = 0;
    *(uint8_t*)0x208f0013 = 0;
    *(uint8_t*)0x208f0014 = 0;
    *(uint8_t*)0x208f0015 = 0;
    *(uint8_t*)0x208f0016 = 0;
    *(uint8_t*)0x208f0017 = 1;
    *(uint32_t*)0x208f0018 = 0;
    syscall(__NR_sendto, r[0], 0x20e76f4e, 0, 0x20040000, 0x208f0000, 0x1c);
    break;
  }
}

void test()
{
  memset(r, -1, sizeof(r));
  execute(5);
  collide = 1;
  execute(5);
}

int main()
{
  for (;;) {
    loop();
  }
}
