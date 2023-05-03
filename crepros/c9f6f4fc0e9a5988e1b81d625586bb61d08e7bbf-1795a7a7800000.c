// https://syzkaller.appspot.com/bug?id=c9f6f4fc0e9a5988e1b81d625586bb61d08e7bbf
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

uint64_t r[1] = {0xffffffffffffffff};
void loop()
{
  long res = 0;
  res = syscall(__NR_socket, 0x10, 0x200000000002, 0xc);
  if (res != -1)
    r[0] = res;
  NONFAILING(memcpy((void*)0x20390000, "\x1f\x00\x00\x00\x00\x07\xfd\x02\x01"
                                       "\x0c\x00\xf6\x00\x00\xff\x00\x81\x09"
                                       "\xf8\x00\x09\x00\x01\x80\x06\x08\x0b"
                                       "\xec\x00\x00\x00",
                    31));
  syscall(__NR_write, r[0], 0x20390000, 0x1f);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  install_segv_handler();
  loop();
  return 0;
}
