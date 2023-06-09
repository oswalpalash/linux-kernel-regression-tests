// https://syzkaller.appspot.com/bug?id=f9566581b22b1e25ac0dd9f669dff283c0d28efc
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/futex.h>

static void sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
}

static uint64_t current_time_ms(void)
{
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    exit(1);
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void thread_start(void* (*fn)(void*), void* arg)
{
  pthread_t th;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 128 << 10);
  if (pthread_create(&th, &attr, fn, arg))
    exit(1);
  pthread_attr_destroy(&attr);
}

typedef struct {
  int state;
} event_t;

static void event_init(event_t* ev)
{
  ev->state = 0;
}

static void event_reset(event_t* ev)
{
  ev->state = 0;
}

static void event_set(event_t* ev)
{
  if (ev->state)
    exit(1);
  __atomic_store_n(&ev->state, 1, __ATOMIC_RELEASE);
  syscall(SYS_futex, &ev->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG);
}

static void event_wait(event_t* ev)
{
  while (!__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, 0);
}

static int event_isset(event_t* ev)
{
  return __atomic_load_n(&ev->state, __ATOMIC_ACQUIRE);
}

static int event_timedwait(event_t* ev, uint64_t timeout)
{
  uint64_t start = current_time_ms();
  uint64_t now = start;
  for (;;) {
    uint64_t remain = timeout - (now - start);
    struct timespec ts;
    ts.tv_sec = remain / 1000;
    ts.tv_nsec = (remain % 1000) * 1000 * 1000;
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, &ts);
    if (__atomic_load_n(&ev->state, __ATOMIC_RELAXED))
      return 1;
    now = current_time_ms();
    if (now - start > timeout)
      return 0;
  }
}

static void setup_common()
{
  if (mount(0, "/sys/fs/fuse/connections", "fusectl", 0, 0)) {
  }
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();
  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = 200 << 20;
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 32 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 136 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 256;
  setrlimit(RLIMIT_NOFILE, &rlim);
  if (unshare(CLONE_NEWNS)) {
  }
  if (unshare(CLONE_NEWIPC)) {
  }
  if (unshare(0x02000000)) {
  }
  if (unshare(CLONE_NEWUTS)) {
  }
  if (unshare(CLONE_SYSVSEM)) {
  }
}

int wait_for_loop(int pid)
{
  if (pid < 0)
    exit(1);
  int status = 0;
  while (waitpid(-1, &status, __WALL) != pid) {
  }
  return WEXITSTATUS(status);
}

static int do_sandbox_none(void)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid != 0)
    return wait_for_loop(pid);
  setup_common();
  sandbox_common();
  if (unshare(CLONE_NEWNET)) {
  }
  loop();
  exit(1);
}

struct thread_t {
  int created, call;
  event_t ready, done;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    event_wait(&th->ready);
    event_reset(&th->ready);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    event_set(&th->done);
  }
  return 0;
}

static void loop(void)
{
  int i, call, thread;
  for (call = 0; call < 8; call++) {
    for (thread = 0; thread < (int)(sizeof(threads) / sizeof(threads[0]));
         thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        event_init(&th->ready);
        event_init(&th->done);
        event_set(&th->done);
        thread_start(thr, th);
      }
      if (!event_isset(&th->done))
        continue;
      event_reset(&th->done);
      th->call = call;
      __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
      event_set(&th->ready);
      event_timedwait(&th->done, 45);
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
}

uint64_t r[2] = {0xffffffffffffffff, 0x0};

void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    memcpy((void*)0x20000000, "./file0", 8);
    syscall(__NR_mkdirat, 0xffffff9c, 0x20000000, 0);
    break;
  case 1:
    memcpy((void*)0x20000180, "/dev/fuse", 10);
    res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000180, 2, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 2:
    memcpy((void*)0x20000100, "./file0", 8);
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
    syscall(__NR_mount, 0, 0x20000100, 0x20000300, 0, 0x20000400);
    break;
  case 3:
    syscall(__NR_read, r[0], 0x200040c0, 0x1000);
    break;
  case 4:
    syscall(__NR_read, r[0], 0x20002000, 0x46d);
    break;
  case 5:
    *(uint32_t*)0x20000240 = 0x10;
    *(uint32_t*)0x20000244 = 0;
    *(uint64_t*)0x20000248 = 2;
    syscall(__NR_write, r[0], 0x20000240, 0x10);
    break;
  case 6:
    memcpy((void*)0x20000780, "./file0/file0", 14);
    res = syscall(__NR_lstat, 0x20000780, 0x200007c0);
    if (res != -1)
      r[1] = *(uint32_t*)0x200007d0;
    break;
  case 7:
    *(uint32_t*)0x20000880 = 0x78;
    *(uint32_t*)0x20000884 = 0;
    *(uint64_t*)0x20000888 = 4;
    *(uint64_t*)0x20000890 = 0x100000001;
    *(uint32_t*)0x20000898 = 5;
    *(uint32_t*)0x2000089c = 0;
    *(uint64_t*)0x200008a0 = 4;
    *(uint64_t*)0x200008a8 = 2;
    *(uint64_t*)0x200008b0 = 0x1f;
    *(uint64_t*)0x200008b8 = 0;
    *(uint64_t*)0x200008c0 = 1;
    *(uint64_t*)0x200008c8 = 7;
    *(uint32_t*)0x200008d0 = 0;
    *(uint32_t*)0x200008d4 = 0x219c8b2e;
    *(uint32_t*)0x200008d8 = 3;
    *(uint32_t*)0x200008dc = 4;
    *(uint32_t*)0x200008e0 = 0x80000000;
    *(uint32_t*)0x200008e4 = 0;
    *(uint32_t*)0x200008e8 = r[1];
    *(uint32_t*)0x200008ec = 0x59a4;
    *(uint32_t*)0x200008f0 = 0x7a;
    *(uint32_t*)0x200008f4 = 0;
    syscall(__NR_write, r[0], 0x20000880, 0x78);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  do_sandbox_none();
  return 0;
}
