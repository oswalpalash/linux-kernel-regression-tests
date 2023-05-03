// https://syzkaller.appspot.com/bug?id=b315b27d2852388c7d29abadff62f84b14256522
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* ctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  if (__atomic_load_n(&skip_segv, __ATOMIC_RELAXED) &&
      (addr < prog_start || addr > prog_end)) {
    _longjmp(segv_env, 1);
  }
  exit(sig);
}

static void install_segv_handler(void)
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

static long syz_open_dev(long a0, long a1, long a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    NONFAILING(strncpy(buf, (char*)a0, sizeof(buf) - 1));
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  install_segv_handler();
  long res = 0;
  NONFAILING(memcpy((void*)0x20000f80, "/dev/binder#\000", 13));
  res = syz_open_dev(0x20000f80, 0, 0);
  if (res != -1)
    r[0] = res;
  res = syz_open_dev(0, 0, 0);
  if (res != -1)
    r[1] = res;
  syscall(__NR_mmap, 0x20001000, 0x3000, 0, 0x20011, r[1], 0);
  syscall(__NR_ioctl, r[1], 0x40046207, 0);
  NONFAILING(*(uint64_t*)0x20000440 = 0x44);
  NONFAILING(*(uint64_t*)0x20000448 = 0);
  NONFAILING(*(uint64_t*)0x20000450 = 0x20000300);
  NONFAILING(*(uint32_t*)0x20000300 = 0x40406300);
  NONFAILING(*(uint32_t*)0x20000304 = 0);
  NONFAILING(*(uint32_t*)0x20000308 = 0);
  NONFAILING(*(uint64_t*)0x2000030c = 0);
  NONFAILING(*(uint32_t*)0x20000314 = 0);
  NONFAILING(*(uint32_t*)0x20000318 = 0);
  NONFAILING(*(uint32_t*)0x2000031c = 0);
  NONFAILING(*(uint32_t*)0x20000320 = 0);
  NONFAILING(*(uint64_t*)0x20000324 = 0);
  NONFAILING(*(uint64_t*)0x2000032c = 8);
  NONFAILING(*(uint64_t*)0x20000334 = 0);
  NONFAILING(*(uint64_t*)0x2000033c = 0x20000240);
  NONFAILING(*(uint64_t*)0x20000240 = 2);
  NONFAILING(*(uint64_t*)0x20000458 = 0);
  NONFAILING(*(uint64_t*)0x20000460 = 0);
  NONFAILING(*(uint64_t*)0x20000468 = 0);
  syscall(__NR_ioctl, r[0], 0xc0306201, 0x20000440);
  return 0;
}
