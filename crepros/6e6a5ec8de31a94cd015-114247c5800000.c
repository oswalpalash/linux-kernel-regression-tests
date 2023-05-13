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

long r[3];
void execute_call(int call)
{
  switch (call) {
  case 0:
    syscall(__NR_mmap, 0x20000000, 0xe76000, 3, 0x32, -1, 0);
    break;
  case 1:
    r[0] = syscall(__NR_socket, 0xa, 2, 0);
    break;
  case 2:
    r[1] = syscall(__NR_socket, 0x18, 1, 1);
    break;
  case 3:
    *(uint16_t*)0x20354000 = 0x18;
    *(uint32_t*)0x20354002 = 1;
    *(uint32_t*)0x20354006 = 0;
    *(uint32_t*)0x2035400a = r[0];
    *(uint16_t*)0x2035400e = 3;
    *(uint16_t*)0x20354010 = 0;
    *(uint16_t*)0x20354012 = 3;
    *(uint16_t*)0x20354014 = 0;
    *(uint16_t*)0x20354016 = 0xa;
    *(uint16_t*)0x20354018 = htobe16(0x4e22);
    *(uint32_t*)0x2035401a = 3;
    *(uint8_t*)0x2035401e = 0xfe;
    *(uint8_t*)0x2035401f = 0x80;
    *(uint8_t*)0x20354020 = 0;
    *(uint8_t*)0x20354021 = 0;
    *(uint8_t*)0x20354022 = 0;
    *(uint8_t*)0x20354023 = 0;
    *(uint8_t*)0x20354024 = 0;
    *(uint8_t*)0x20354025 = 0;
    *(uint8_t*)0x20354026 = 0;
    *(uint8_t*)0x20354027 = 0;
    *(uint8_t*)0x20354028 = 0;
    *(uint8_t*)0x20354029 = 0;
    *(uint8_t*)0x2035402a = 0;
    *(uint8_t*)0x2035402b = 0;
    *(uint8_t*)0x2035402c = 0;
    *(uint8_t*)0x2035402d = 0xbb;
    *(uint32_t*)0x2035402e = 4;
    syscall(__NR_connect, r[1], 0x20354000, 0x32);
    break;
  case 4:
    r[2] = syscall(__NR_socket, 0x18, 1, 1);
    break;
  case 5:
    syscall(__NR_close, r[0]);
    break;
  case 6:
    syscall(__NR_dup2, r[2], r[1]);
    break;
  }
}

void test()
{
  memset(r, -1, sizeof(r));
  execute(7);
  collide = 1;
  execute(7);
}

int main()
{
  for (;;) {
    loop();
  }
}
