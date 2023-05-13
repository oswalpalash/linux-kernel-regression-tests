// https://syzkaller.appspot.com/bug?id=0a514ede78a3da2ec4c94df199c30dc8addcba08
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

uint64_t r[5] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff, 0x0};
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    res = syscall(__NR_pipe2, 0x20000040, 0x4000);
    if (res != -1) {
      r[0] = *(uint32_t*)0x20000040;
      r[1] = *(uint32_t*)0x20000044;
    }
    break;
  case 1:
    *(uint32_t*)0x20000080 = 7;
    *(uint8_t*)0x20000084 = 0x21;
    *(uint16_t*)0x20000085 = 1;
    syscall(__NR_write, r[1], 0x20000080, 7);
    break;
  case 2:
    res = syscall(__NR_pipe2, 0x20000100, 0);
    if (res != -1)
      r[2] = *(uint32_t*)0x20000100;
    break;
  case 3:
    memcpy((void*)0x20000200, "./file0", 8);
    res = syscall(__NR_open, 0x20000200, 0x200800, 0x30);
    if (res != -1)
      r[3] = res;
    break;
  case 4:
    memcpy((void*)0x20000340, "./file0", 8);
    syscall(__NR_open, 0x20000340, 0x40, 0);
    break;
  case 5:
    res = syscall(__NR_fstat, r[0], 0x20000440);
    if (res != -1)
      r[4] = *(uint32_t*)0x2000044c;
    break;
  case 6:
    memcpy((void*)0x200003c0, "./file0", 8);
    memcpy((void*)0x20000400, "9p", 3);
    memcpy((void*)0x200004c0, "trans=fd,", 9);
    memcpy((void*)0x200004c9, "rfdno", 5);
    *(uint8_t*)0x200004ce = 0x3d;
    sprintf((char*)0x200004cf, "0x%016llx", (long long)r[2]);
    *(uint8_t*)0x200004e1 = 0x2c;
    memcpy((void*)0x200004e2, "wfdno", 5);
    *(uint8_t*)0x200004e7 = 0x3d;
    sprintf((char*)0x200004e8, "0x%016llx", (long long)r[3]);
    *(uint8_t*)0x200004fa = 0x2c;
    memcpy((void*)0x200004fb, "dfltuid", 7);
    *(uint8_t*)0x20000502 = 0x3d;
    sprintf((char*)0x20000503, "0x%016llx", (long long)r[4]);
    *(uint8_t*)0x20000515 = 0x2c;
    memcpy((void*)0x20000516, "version=9p2000.u", 16);
    *(uint8_t*)0x20000526 = 0x2c;
    memcpy((void*)0x20000527, "fscache", 7);
    *(uint8_t*)0x2000052e = 0x2c;
    memcpy((void*)0x2000052f, "version=9p2000.u", 16);
    *(uint8_t*)0x2000053f = 0x2c;
    memcpy((void*)0x20000540, "fscache", 7);
    *(uint8_t*)0x20000547 = 0x2c;
    *(uint8_t*)0x20000548 = 0;
    syscall(__NR_mount, 0, 0x200003c0, 0x20000400, 0x1000000, 0x200004c0);
    break;
  case 7:
    memcpy((void*)0x20000000, "./file0", 8);
    memcpy((void*)0x20000380, "9p", 3);
    memcpy((void*)0x20000240, "trans=fd,", 9);
    memcpy((void*)0x20000249, "rfdno", 5);
    *(uint8_t*)0x2000024e = 0x3d;
    sprintf((char*)0x2000024f, "0x%016llx", (long long)r[2]);
    *(uint8_t*)0x20000261 = 0x2c;
    memcpy((void*)0x20000262, "wfdno", 5);
    *(uint8_t*)0x20000267 = 0x3d;
    sprintf((char*)0x20000268, "0x%016llx", (long long)r[0]);
    *(uint8_t*)0x2000027a = 0x2c;
    memcpy((void*)0x2000027b, "access=any", 10);
    *(uint8_t*)0x20000285 = 0x2c;
    memcpy((void*)0x20000286, "version=9p2000", 14);
    *(uint8_t*)0x20000294 = 0x2c;
    *(uint8_t*)0x20000295 = 0;
    syscall(__NR_mount, 0, 0x20000000, 0x20000380, 0, 0x20000240);
    break;
  }
}

void execute_one()
{
  execute(8);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    loop();
  }
}
