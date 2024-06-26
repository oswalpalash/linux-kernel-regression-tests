// https://syzkaller.appspot.com/bug?id=b61716c2020c98e885af88c7de5896425947313f
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
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
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

static void execute_one();
extern unsigned long long procid;

static void loop()
{
  int iter;
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      fail("clone failed");
    if (pid == 0) {
      prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
      setpgrp();
      execute_one();
      int fd;
      for (fd = 3; fd < 30; fd++)
        close(fd);
      doexit(0);
    }

    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      int res = waitpid(-1, &status, __WALL | WNOHANG);
      if (res == pid) {
        break;
      }
      usleep(1000);
      if (current_time_ms() - start < 5 * 1000)
        continue;
      kill(-pid, SIGKILL);
      kill(pid, SIGKILL);
      while (waitpid(-1, &status, __WALL) != pid) {
      }
      break;
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

uint64_t r[1] = {0xffffffffffffffff};
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    memcpy((void*)0x20000000, "./file0", 8);
    syscall(__NR_mkdirat, 0xffffff9c, 0x20000000, 0);
    break;
  case 1:
    memcpy((void*)0x20000100, "/dev/fuse", 10);
    res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000100, 2, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 2:
    memcpy((void*)0x20000200, "./file0", 8);
    memcpy((void*)0x20000300, "fuse", 5);
    memcpy((void*)0x20000400, "fd", 2);
    *(uint8_t*)0x20000402 = 0x3d;
    sprintf((char*)0x20000403, "0x%016llx", (long long)r[0]);
    *(uint8_t*)0x20000415 = 0x2c;
    memcpy((void*)0x20000416, "rootmode", 8);
    *(uint8_t*)0x2000041e = 0x3d;
    sprintf((char*)0x2000041f, "%023llo", (long long)0x4000);
    *(uint8_t*)0x20000436 = 0x2c;
    memcpy((void*)0x20000437, "user_id", 7);
    *(uint8_t*)0x2000043e = 0x3d;
    sprintf((char*)0x2000043f, "%020llu", (long long)0);
    *(uint8_t*)0x20000453 = 0x2c;
    memcpy((void*)0x20000454, "group_id", 8);
    *(uint8_t*)0x2000045c = 0x3d;
    sprintf((char*)0x2000045d, "%020llu", (long long)0);
    *(uint8_t*)0x20000471 = 0x2c;
    *(uint8_t*)0x20000472 = 0;
    syscall(__NR_mount, 0, 0x20000200, 0x20000300, 0, 0x20000400);
    break;
  case 3:
    memcpy((void*)0x200000c0, "./file0/file0", 14);
    syscall(__NR_stat, 0x200000c0, 0x20000480);
    break;
  case 4:
    syscall(__NR_read, r[0], 0x20001000, 0x1000);
    break;
  case 5:
    *(uint32_t*)0x20000340 = 0x50;
    *(uint32_t*)0x20000344 = 0;
    *(uint64_t*)0x20000348 = 1;
    *(uint32_t*)0x20000350 = 7;
    *(uint32_t*)0x20000354 = 0x1b;
    *(uint32_t*)0x20000358 = 4;
    *(uint32_t*)0x2000035c = 0x60000;
    *(uint16_t*)0x20000360 = 0x200;
    *(uint16_t*)0x20000362 = 2;
    *(uint32_t*)0x20000364 = 0x800;
    *(uint32_t*)0x20000368 = 0x800000;
    *(uint32_t*)0x2000036c = 0;
    *(uint32_t*)0x20000370 = 0;
    *(uint32_t*)0x20000374 = 0;
    *(uint32_t*)0x20000378 = 0;
    *(uint32_t*)0x2000037c = 0;
    *(uint32_t*)0x20000380 = 0;
    *(uint32_t*)0x20000384 = 0;
    *(uint32_t*)0x20000388 = 0;
    *(uint32_t*)0x2000038c = 0;
    syscall(__NR_write, r[0], 0x20000340, 0x50);
    break;
  }
}

void execute_one()
{
  execute(6);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    loop();
  }
}
