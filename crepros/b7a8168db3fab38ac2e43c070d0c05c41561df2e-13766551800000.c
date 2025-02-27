// https://syzkaller.appspot.com/bug?id=b7a8168db3fab38ac2e43c070d0c05c41561df2e
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/futex.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/stat.h>
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
#include <setjmp.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>
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

#define NONFAILING(...)                                                        \
  {                                                                            \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    if (_setjmp(segv_env) == 0) {                                              \
      __VA_ARGS__;                                                             \
    }                                                                          \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
  }

static uint64_t current_time_ms()
{
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    fail("clock_gettime failed");
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

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
        if (running)
          usleep((call == num_calls - 1) ? 10000 : 1000);
        break;
      }
    }
  }
}

long r[2];
uint64_t procid;
void execute_call(int call)
{
  switch (call) {
  case 0:
    syscall(__NR_mmap, 0x20000000, 0x8000, 3, 0x32, -1, 0);
    break;
  case 1:
    NONFAILING(memcpy((void*)0x20002ff3, "/dev/binder#", 13));
    r[0] = syz_open_dev(0x20002ff3, 0, 0);
    break;
  case 2:
    syscall(__NR_close, r[0]);
    break;
  case 3:
    NONFAILING(memcpy((void*)0x20008ff3, "/dev/binder#", 13));
    r[1] = syz_open_dev(0x20008ff3, 0, 0);
    break;
  case 4:
    syscall(__NR_mmap, 0x20000000, 0x2000, 0, 0x20011, r[1], 0);
    break;
  case 5:
    syscall(__NR_mmap, 0x20009000, 0x1000, 3, 0x32, -1, 0);
    break;
  case 6:
    syscall(__NR_ioctl, r[1], 0x40046207, 0);
    break;
  case 7:
    syscall(__NR_mmap, 0x2000a000, 0x1000, 3, 0x32, -1, 0);
    break;
  case 8:
    NONFAILING(*(uint64_t*)0x2000a000 = 4);
    NONFAILING(*(uint64_t*)0x2000a008 = 0);
    NONFAILING(*(uint64_t*)0x2000a010 = 0x2000aff0);
    NONFAILING(*(uint64_t*)0x2000a018 = 0x44);
    NONFAILING(*(uint64_t*)0x2000a020 = 0);
    NONFAILING(*(uint64_t*)0x2000a028 = 0x20004000);
    NONFAILING(*(uint32_t*)0x2000aff0 = 0x630c);
    NONFAILING(memcpy((void*)0x20004000,
                      "\x27\x16\xc8\x4b\xf8\x1a\x47\x7a\xd3\x31\x8a\xa4\xe2\x2d"
                      "\x24\x4b\x80\x14\x91\xb0\xab\x31\x99\xa9\x2b\x02\x2c\xdb"
                      "\x39\x0a\xa9\x7e\x9e\xd8\xe4\x52\xda\xe8\xd7\x24\xc2\x6d"
                      "\x0e\xb9\x3c\x4d\x31\xa7\x18\x51\xa3\x52\x6c\xb6\x1f\xba"
                      "\x5e\xa3\x38\x23\xf4\xf6\x92\xf0\x13\x16\x7a\x90",
                      68));
    syscall(__NR_ioctl, r[0], 0xc0306201, 0x2000a000);
    break;
  case 9:
    NONFAILING(*(uint64_t*)0x20007000 = 4);
    NONFAILING(*(uint64_t*)0x20007008 = 0);
    NONFAILING(*(uint64_t*)0x20007010 = 0x20005fd4);
    NONFAILING(*(uint64_t*)0x20007018 = 0);
    NONFAILING(*(uint64_t*)0x20007020 = 0);
    NONFAILING(*(uint64_t*)0x20007028 = 0x20002000);
    NONFAILING(*(uint32_t*)0x20005fd4 = 0x40486311);
    NONFAILING(*(uint64_t*)0x20005fd8 = 0);
    NONFAILING(*(uint64_t*)0x20005fe0 = 0);
    syscall(__NR_ioctl, r[0], 0xc0306201, 0x20007000);
    break;
  case 10:
    syscall(__NR_ioctl, r[1], 0x40046207, 0);
    break;
  case 11:
    NONFAILING(*(uint64_t*)0x2000afd0 = 0x44);
    NONFAILING(*(uint64_t*)0x2000afd8 = 0);
    NONFAILING(*(uint64_t*)0x2000afe0 = 0x20009f84);
    NONFAILING(*(uint64_t*)0x2000afe8 = 0);
    NONFAILING(*(uint64_t*)0x2000aff0 = 0);
    NONFAILING(*(uint64_t*)0x2000aff8 = 0x20003fb3);
    NONFAILING(*(uint32_t*)0x20009f84 = 0x40486311);
    NONFAILING(*(uint32_t*)0x20009f88 = 0);
    NONFAILING(*(uint32_t*)0x20009f8c = 0);
    NONFAILING(*(uint64_t*)0x20009f90 = 0);
    NONFAILING(*(uint32_t*)0x20009f98 = 0);
    NONFAILING(*(uint32_t*)0x20009f9c = 0);
    NONFAILING(*(uint32_t*)0x20009fa0 = 0);
    NONFAILING(*(uint32_t*)0x20009fa4 = 0);
    NONFAILING(*(uint64_t*)0x20009fa8 = 0);
    NONFAILING(*(uint64_t*)0x20009fb0 = 0);
    NONFAILING(*(uint64_t*)0x20009fb8 = 0x2000a000);
    NONFAILING(*(uint64_t*)0x20009fc0 = 0x20003000);
    syscall(__NR_ioctl, r[0], 0xc0306201, 0x2000afd0);
    break;
  }
}

void test()
{
  memset(r, -1, sizeof(r));
  execute(12);
}

int main()
{
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      install_segv_handler();
      for (;;) {
        loop();
      }
    }
  }
  sleep(1000000);
  return 0;
}
