// https://syzkaller.appspot.com/bug?id=2bf7b7983c2398ec6f0c4c6c87cb50223e8873f8
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <linux/futex.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
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

#define BITMASK_LEN(type, bf_len) (type)((1ull << (bf_len)) - 1)

#define BITMASK_LEN_OFF(type, bf_off, bf_len)                                  \
  (type)(BITMASK_LEN(type, (bf_len)) << (bf_off))

#define STORE_BY_BITMASK(type, addr, val, bf_off, bf_len)                      \
  if ((bf_off) == 0 && (bf_len) == 0) {                                        \
    *(type*)(addr) = (type)(val);                                              \
  } else {                                                                     \
    type new_val = *(type*)(addr);                                             \
    new_val &= ~BITMASK_LEN_OFF(type, (bf_off), (bf_len));                     \
    new_val |= ((type)(val)&BITMASK_LEN(type, (bf_len))) << (bf_off);          \
    *(type*)(addr) = new_val;                                                  \
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

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();

  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = 128 << 20;
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 8 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);

#define CLONE_NEWCGROUP 0x02000000

  unshare(CLONE_NEWNS);
  unshare(CLONE_NEWIPC);
  unshare(CLONE_NEWCGROUP);
  unshare(CLONE_NEWNET);
  unshare(CLONE_NEWUTS);
  unshare(CLONE_SYSVSEM);
}

static int do_sandbox_none(int executor_pid, bool enable_tun)
{
  unshare(CLONE_NEWPID);
  int pid = fork();
  if (pid < 0)
    fail("sandbox fork failed");
  if (pid)
    return pid;

  sandbox_common();

  loop();
  doexit(1);
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
    syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
    break;
  case 1:
    NONFAILING(*(uint32_t*)0x2001d000 = 2);
    NONFAILING(*(uint32_t*)0x2001d004 = 0x78);
    NONFAILING(*(uint8_t*)0x2001d008 = 0xe2);
    NONFAILING(*(uint8_t*)0x2001d009 = 0);
    NONFAILING(*(uint8_t*)0x2001d00a = 0);
    NONFAILING(*(uint8_t*)0x2001d00b = 0);
    NONFAILING(*(uint32_t*)0x2001d00c = 0);
    NONFAILING(*(uint64_t*)0x2001d010 = 0);
    NONFAILING(*(uint64_t*)0x2001d018 = 0);
    NONFAILING(*(uint64_t*)0x2001d020 = 0);
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2001d028, 0xfe, 0, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 1, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 2, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 3, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 4, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 5, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 6, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 7, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 8, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 9, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 10, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 11, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 12, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 13, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 14, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 15, 2));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 17, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 18, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 19, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 20, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 21, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 22, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 23, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 24, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 25, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 26, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 27, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 28, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 29, 35));
    NONFAILING(*(uint32_t*)0x2001d030 = 0);
    NONFAILING(*(uint32_t*)0x2001d034 = 0);
    NONFAILING(*(uint64_t*)0x2001d038 = 0x20000000);
    NONFAILING(*(uint64_t*)0x2001d040 = 0);
    NONFAILING(*(uint64_t*)0x2001d048 = 0);
    NONFAILING(*(uint64_t*)0x2001d050 = 0);
    NONFAILING(*(uint64_t*)0x2001d058 = 0);
    NONFAILING(*(uint32_t*)0x2001d060 = 0);
    NONFAILING(*(uint64_t*)0x2001d068 = 0);
    NONFAILING(*(uint32_t*)0x2001d070 = 0);
    NONFAILING(*(uint16_t*)0x2001d074 = 0);
    NONFAILING(*(uint16_t*)0x2001d076 = 0);
    syscall(__NR_perf_event_open, 0x2001d000, 0, 0, -1, 0);
    break;
  case 2:
    NONFAILING(memcpy((void*)0x20919000, "/dev/kvm", 9));
    r[0] = syscall(__NR_openat, 0xffffffffffffff9c, 0x20919000, 0, 0);
    break;
  case 3:
    r[1] = syscall(__NR_ioctl, r[0], 0xae01, 0);
    break;
  case 4:
    NONFAILING(*(uint32_t*)0x208fa000 = 0x79);
    NONFAILING(*(uint32_t*)0x208fa004 = 0);
    NONFAILING(*(uint64_t*)0x208fa008 = 0);
    NONFAILING(*(uint64_t*)0x208fa010 = 0);
    NONFAILING(*(uint64_t*)0x208fa018 = 0);
    NONFAILING(*(uint64_t*)0x208fa020 = 0);
    NONFAILING(*(uint8_t*)0x208fa028 = 0);
    NONFAILING(*(uint8_t*)0x208fa029 = 0);
    NONFAILING(*(uint8_t*)0x208fa02a = 0);
    NONFAILING(*(uint8_t*)0x208fa02b = 0);
    NONFAILING(*(uint8_t*)0x208fa02c = 0);
    NONFAILING(*(uint8_t*)0x208fa02d = 0);
    NONFAILING(*(uint8_t*)0x208fa02e = 0);
    NONFAILING(*(uint8_t*)0x208fa02f = 0);
    NONFAILING(*(uint8_t*)0x208fa030 = 0);
    NONFAILING(*(uint8_t*)0x208fa031 = 0);
    NONFAILING(*(uint8_t*)0x208fa032 = 0);
    NONFAILING(*(uint8_t*)0x208fa033 = 0);
    NONFAILING(*(uint8_t*)0x208fa034 = 0);
    NONFAILING(*(uint8_t*)0x208fa035 = 0);
    NONFAILING(*(uint8_t*)0x208fa036 = 0);
    NONFAILING(*(uint8_t*)0x208fa037 = 0);
    NONFAILING(*(uint8_t*)0x208fa038 = 0);
    NONFAILING(*(uint8_t*)0x208fa039 = 0);
    NONFAILING(*(uint8_t*)0x208fa03a = 0);
    NONFAILING(*(uint8_t*)0x208fa03b = 0);
    NONFAILING(*(uint8_t*)0x208fa03c = 0);
    NONFAILING(*(uint8_t*)0x208fa03d = 0);
    NONFAILING(*(uint8_t*)0x208fa03e = 0);
    NONFAILING(*(uint8_t*)0x208fa03f = 0);
    NONFAILING(*(uint8_t*)0x208fa040 = 0);
    NONFAILING(*(uint8_t*)0x208fa041 = 0);
    NONFAILING(*(uint8_t*)0x208fa042 = 0);
    NONFAILING(*(uint8_t*)0x208fa043 = 0);
    NONFAILING(*(uint8_t*)0x208fa044 = 0);
    NONFAILING(*(uint8_t*)0x208fa045 = 0);
    NONFAILING(*(uint8_t*)0x208fa046 = 0);
    NONFAILING(*(uint8_t*)0x208fa047 = 0);
    NONFAILING(*(uint8_t*)0x208fa048 = 0);
    NONFAILING(*(uint8_t*)0x208fa049 = 0);
    NONFAILING(*(uint8_t*)0x208fa04a = 0);
    NONFAILING(*(uint8_t*)0x208fa04b = 0);
    NONFAILING(*(uint8_t*)0x208fa04c = 0);
    NONFAILING(*(uint8_t*)0x208fa04d = 0);
    NONFAILING(*(uint8_t*)0x208fa04e = 0);
    NONFAILING(*(uint8_t*)0x208fa04f = 0);
    NONFAILING(*(uint8_t*)0x208fa050 = 0);
    NONFAILING(*(uint8_t*)0x208fa051 = 0);
    NONFAILING(*(uint8_t*)0x208fa052 = 0);
    NONFAILING(*(uint8_t*)0x208fa053 = 0);
    NONFAILING(*(uint8_t*)0x208fa054 = 0);
    NONFAILING(*(uint8_t*)0x208fa055 = 0);
    NONFAILING(*(uint8_t*)0x208fa056 = 0);
    NONFAILING(*(uint8_t*)0x208fa057 = 0);
    NONFAILING(*(uint8_t*)0x208fa058 = 0);
    NONFAILING(*(uint8_t*)0x208fa059 = 0);
    NONFAILING(*(uint8_t*)0x208fa05a = 0);
    NONFAILING(*(uint8_t*)0x208fa05b = 0);
    NONFAILING(*(uint8_t*)0x208fa05c = 0);
    NONFAILING(*(uint8_t*)0x208fa05d = 0);
    NONFAILING(*(uint8_t*)0x208fa05e = 0);
    NONFAILING(*(uint8_t*)0x208fa05f = 0);
    NONFAILING(*(uint8_t*)0x208fa060 = 0);
    NONFAILING(*(uint8_t*)0x208fa061 = 0);
    NONFAILING(*(uint8_t*)0x208fa062 = 0);
    NONFAILING(*(uint8_t*)0x208fa063 = 0);
    NONFAILING(*(uint8_t*)0x208fa064 = 0);
    NONFAILING(*(uint8_t*)0x208fa065 = 0);
    NONFAILING(*(uint8_t*)0x208fa066 = 0);
    NONFAILING(*(uint8_t*)0x208fa067 = 0);
    syscall(__NR_ioctl, r[1], 0x4068aea3, 0x208fa000);
    break;
  case 5:
    r[2] = syscall(__NR_ioctl, r[1], 0xae41, 0);
    break;
  case 6:
    NONFAILING(*(uint32_t*)0x20789fe0 = 0);
    NONFAILING(*(uint32_t*)0x20789fe4 = 0);
    NONFAILING(*(uint64_t*)0x20789fe8 = 0);
    NONFAILING(*(uint64_t*)0x20789ff0 = 0x1000);
    NONFAILING(*(uint64_t*)0x20789ff8 = 0x202a0000);
    syscall(__NR_ioctl, r[1], 0x4020ae46, 0x20789fe0);
    break;
  case 7:
    NONFAILING(memcpy(
        (void*)0x20ebec00,
        "\xb3\xa6\x40\x56\x4b\x6b\x3c\x4a\x55\x2c\x1b\xf7\xa0\x97\xe2\x59\x9a"
        "\x7d\xce\xd9\x2e\xdc\x20\xee\xf8\x03\xca\x02\xb9\x00\x00\x00\x00\x00"
        "\x00\x80\x00\xc7\x3b\x52\xbc\x39\x10\xe9\x0c\x3c\xd9\xb0\x8f\x9f\x0c"
        "\x66\xd5\x06\x00\x00\x00\x7f\x65\x2c\xd8\x5a\x00\x9f\x83\x4c\xa0\x27"
        "\x73\x19\xbe\xcd\xe7\xfa\x12\x82\x4b\x8c\x67\xf2\x3e\x2e\x4b\x32\x9a"
        "\x5d\x70\xa0\x89\xb1\x9a\xdb\x31\x9c\x21\x3e\xca\x82\xfd\x2a\x3f\xba"
        "\x9e\xcd\x74\x0e\x45\x04\x8c\x23\xef\xba\xee\x09\x2f\x30\x20\x5a\x1c"
        "\x24\xc0\xf4\x04\x8b\xf5\xa4\xb0\xc4\xf3\xf1\x27\x87\xf1\x4e\x6a\xc5"
        "\x56\xaa\x27\x20\x2e\xb1\xfc\x1b\x53\x62\x90\x37\xa6\x89\x1e\x0c\x4a"
        "\xdb\xc0\x15\x7a\x66\xb4\xee\x4b\x05\x07\x87\x0f\x2d\x43\xb2\x2a\x43"
        "\x37\x11\xd0\x09\x8e\x74\x19\x6c\x34\x10\x41\x73\x32\xed\x4b\x74\x25"
        "\x2b\x0b\x35\xe0\xf9\x7c\xc5\x0d\x1a\xc5\x61\xc8\x86\x2c\xa2\xff\x90"
        "\x21\xf0\x14\xd5\xa1\x91\x6d\xa1\xa8\x5f\x43\xb1\x3b\x54\xd2\x09\x68"
        "\xd3\x58\x9c\x70\x23\xc5\xe5\x2e\xfc\xe2\x67\xbb\x40\x35\xcc\xfc\xaf"
        "\x06\xd8\xe1\x23\x8a\x43\x21\x4b\x2c\x01\x00\x01\x00\x8e\xf5\xd9\xad"
        "\xeb\xe5\x9d\x13\x4b\x7d\xdb\xc7\x5a\x52\x33\xf7\xe3\xbf\x29\x43\x8b"
        "\x9d\xc6\xb4\x29\x71\xb0\x8f\x7c\x31\x0c\xb7\x89\x30\x25\xc3\x77\xcd"
        "\x55\x2e\x24\x6a\x09\xca\x3e\x3c\x3e\x28\x57\x96\x15\x00\x01\x0d\xca"
        "\x5c\x05\x0f\x70\xff\xa0\xcd\x81\x6a\xaa\xc8\xff\xe4\x3a\xde\x51\xf8"
        "\xb2\x28\xf1\x10\x40\xbb\x05\x6a\x4d\xc0\x9e\x63\x6e\xa6\x3f\x6e\x46"
        "\xb9\x60\x35\x55\x53\x96\x9e\xe6\x22\x99\xd3\x91\xea\x90\x48\x4d\x7f"
        "\xc9\x01\x65\x9a\xb8\xca\x00\x00\x01\x00\xda\x88\x8e\x0a\x9c\x7a\xe2"
        "\x07\x63\x9f\xb7\xba\xd8\x67\x91\x89\x1f\xc5\xeb\x42\x6f\xeb\x1d\x03"
        "\xd0\x65\x59\x8e\x98\x43\xf6\xa6\x41\xe5\x00\x06\xeb\xae\xa5\xe0\x0d"
        "\x55\x4e\xa1\x1b\xd1\x7a\x55\x90\x3b\x55\xa5\xbd\xdc\x1c\x41\x0d\x8f"
        "\xce\x7f\xbc\xf9\x31\xaf\x94\xb0\x7d\x10\x51\xc0\x7b\x63\xfb\x26\x30"
        "\xca\x45\x1f\x8f\x8f\x61\x71\xea\x68\xba\x25\x03\xa4\x6e\xb6\x81\x07"
        "\x8c\xa5\xbc\x21\x2a\xc0\x3d\x2a\x19\xae\xc1\x37\x04\x8a\xc9\xfd\xb5"
        "\xf6\xec\xc8\x6a\x4d\x32\x7a\x65\x8d\x6d\xb1\x95\x41\x5b\x98\xbc\x9e"
        "\x40\xf3\x1f\x4a\x29\x98\x32\x0e\xfb\xf5\xb3\x36\x23\xbf\xb6\x8c\x88"
        "\x2f\x29\x5d\x60\xc7\x60\xf0\x14\xbd\x89\x0c\xc8\xce\x48\x82\x0d\x62"
        "\x44\x0a\x63\x40\x0f\x84\x39\x8b\x0e\xcd\x36\x6e\x33\x78\x6c\xb7\x03"
        "\x45\x99\x9f\x59\x2e\x25\xf9\xa6\xdd\xd1\xf3\x58\x82\xa9\x6c\x51\x17"
        "\x9e\x30\xa7\x98\x5a\xc3\xef\x08\x00\xb1\x9a\xbc\x81\x2d\x75\xfe\x2d"
        "\x1b\x72\xd4\xc3\x7b\x0b\x73\x11\x01\xb0\xe3\x6c\x55\x7b\x50\x36\xcb"
        "\x0f\x55\x9b\xcb\x8b\xde\x2d\xbd\x5e\xdd\xbb\x1a\x16\x15\xe7\x96\xba"
        "\xb2\xf5\xab\x8a\x81\x6f\x8e\x3f\x7d\x05\xf3\x00\x00\x00\x00\x00\x00"
        "\x00\x00\xf9\x68\x64\x52\x83\x45\x79\x21\x60\x48\x2c\x4c\x3d\x63\xb5"
        "\x35\xc4\x9c\xb0\xa1\xc7\x15\x41\x42\x19\xbf\xd1\x52\x18\x81\xf4\xe8"
        "\x69\x6d\xd3\x86\x67\xf3\x43\x1c\x29\x8d\x85\x59\xbc\x04\x6d\x73\xc7"
        "\x07\x67\xd0\xed\xa8\x39\xfd\xff\xff\xff\xff\xff\xff\xff\xbd\xcb\xbe"
        "\x08\x00\x00\x00\x08\x70\x79\x16\xae\xa7\x06\x5c\x6c\x48\x00\x00\x00"
        "\x00\x00\x00\x00\x40\x7d\x6b\xa1\x1a\x88\x15\x42\x27\x64\x62\xbb\x0d"
        "\xfd\x30\x56\x06\x15\xcd\x0e\xc4\x1c\x41\x18\x87\x8d\xae\xb1\xea\x2c"
        "\x67\xf2\x07\x1b\xcc\xde\x6a\x20\xef\xac\x7a\xfb\x70\x02\x71\x34\x23"
        "\x52\x2e\x3e\x24\x51\x94\x87\x60\x51\xae\x20\xf6\x6d\x35\x90\xbf\x1b"
        "\xfc\x3e\x96\x91\x71\x08\xd4\x22\x02\x09\xfd\x33\xf3\xda\x7e\x62\x07"
        "\xf6\x0a\x18\x7e\x38\x70\x4c\xa0\xb8\xb0\xcc\xc6\x90\x23\x8d\xfb\x59"
        "\x7e\xb3\x3a\x48\xb9\x72\xd6\x35\xfa\xb0\xdb\x5b\x2c\xda\x45\xdd\x74"
        "\x12\x7f\x5f\xa3\x7a\xc2\x7e\x30\xa6\x71\xae\x4a\xc3\x59\x0e\x99\x88"
        "\x71\x3d\xbd\x00\x13\x75\xdc\x0b\x9f\xf6\x89\x90\x57\x3b\x19\x6c\x82"
        "\x48\xea\x1d\xf6\x1d\xbf\xd5\x10\x7c\x87\x21\x23\xb7\xdd\xc9\x11\x07"
        "\x6a\x41\xec\xdc\x97\x39\x08\xbf\x65\x41\x7d\xc1\xc3\x3c\xdc\xd4\x5d"
        "\xf4\xdd\xf4\x38\x90\xae\xe9\x99\xf8\xbb\x2f\x2a\xa5\x31\x5c\xad\x61"
        "\x55\x02\x46\x06\x5a\x69\xc6\xf9\x7b\x21\x6a\xb7\x84\x7b\x0a\x4c\x34"
        "\xa3\x6e\xba\x20\x46\x06\x57\xb6\xc3\x69\xfc\xcc\x12\x1d\x3a\xc2\xc7"
        "\x5e\xa6\x4d\xf3\x87\x7b\xca\x56\x24\x08\xd4\x31\x09\x1c\xb1\x57\x83"
        "\x89\xc4\x8d\x25\x12\xf5\xf8\x08\x20\x4a\x21\x32\x64\xcf\xe8\x90\xc4"
        "\x43\x9c\xa9\xf3\x2e\x0f\x69\xad\x10\x06\xa6\x18\x12\x8d\xbd\xf1\x54"
        "\x50\x79\xd7\x97\x66\x47\x65\xf9\x72\xb5\x21\x26\xdc\xdc\x50\xcd\xbb"
        "\xa6\x01\x00\x43",
        1024));
    syscall(__NR_ioctl, r[2], 0x4400ae8f, 0x20ebec00);
    break;
  case 8:
    syscall(__NR_ioctl, r[2], 0xae80, 0);
    break;
  }
}

void loop()
{
  memset(r, -1, sizeof(r));
  execute(9);
  collide = 1;
  execute(9);
}

int main()
{
  install_segv_handler();
  int pid = do_sandbox_none(0, false);
  int status = 0;
  while (waitpid(pid, &status, __WALL) != pid) {
  }
  return 0;
}
