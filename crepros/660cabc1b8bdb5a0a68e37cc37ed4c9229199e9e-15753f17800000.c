// https://syzkaller.appspot.com/bug?id=660cabc1b8bdb5a0a68e37cc37ed4c9229199e9e
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
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

#define NONFAILING(...)                                                        \
  {                                                                            \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    if (_setjmp(segv_env) == 0) {                                              \
      __VA_ARGS__;                                                             \
    }                                                                          \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
  }

static void execute_one();
extern unsigned long long procid;

void loop()
{
  while (1) {
    execute_one();
  }
}

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

void execute_one()
{
  NONFAILING(*(uint32_t*)0x20008000 = 0);
  NONFAILING(*(uint32_t*)0x20008004 = 0);
  NONFAILING(*(uint64_t*)0x20008008 = 0x20001fe8);
  NONFAILING(*(uint64_t*)0x20008010 = 0x20003ff6);
  NONFAILING(memcpy((void*)0x20003ff6, "syzkaller", 10));
  NONFAILING(*(uint32_t*)0x20008018 = 0);
  NONFAILING(*(uint32_t*)0x2000801c = 0xc3);
  NONFAILING(*(uint64_t*)0x20008020 = 0x20009f3d);
  NONFAILING(*(uint32_t*)0x20008028 = 0);
  NONFAILING(*(uint32_t*)0x2000802c = 0);
  NONFAILING(*(uint8_t*)0x20008030 = 0);
  NONFAILING(*(uint8_t*)0x20008031 = 0);
  NONFAILING(*(uint8_t*)0x20008032 = 0);
  NONFAILING(*(uint8_t*)0x20008033 = 0);
  NONFAILING(*(uint8_t*)0x20008034 = 0);
  NONFAILING(*(uint8_t*)0x20008035 = 0);
  NONFAILING(*(uint8_t*)0x20008036 = 0);
  NONFAILING(*(uint8_t*)0x20008037 = 0);
  NONFAILING(*(uint8_t*)0x20008038 = 0);
  NONFAILING(*(uint8_t*)0x20008039 = 0);
  NONFAILING(*(uint8_t*)0x2000803a = 0);
  NONFAILING(*(uint8_t*)0x2000803b = 0);
  NONFAILING(*(uint8_t*)0x2000803c = 0);
  NONFAILING(*(uint8_t*)0x2000803d = 0);
  NONFAILING(*(uint8_t*)0x2000803e = 0);
  NONFAILING(*(uint8_t*)0x2000803f = 0);
  NONFAILING(*(uint32_t*)0x20008040 = 0);
  syscall(__NR_bpf, 5, 0x20008000, 0x48);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  install_segv_handler();
  for (;;) {
    loop();
  }
}
