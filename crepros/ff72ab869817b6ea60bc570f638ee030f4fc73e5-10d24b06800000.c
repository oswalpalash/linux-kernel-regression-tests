// https://syzkaller.appspot.com/bug?id=f8c72b4d164dbb9fb08573b34df022df46136910
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
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
  for (;;) {
  }
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

#define NONFAILING(...)                                                \
  {                                                                    \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);               \
    if (_setjmp(segv_env) == 0) {                                      \
      __VA_ARGS__;                                                     \
    }                                                                  \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);               \
  }

static bool write_file(const char* file, const char* what, ...)
{
  char buf[1024];
  va_list args;
  va_start(args, what);
  vsnprintf(buf, sizeof(buf), what, args);
  va_end(args);
  buf[sizeof(buf) - 1] = 0;
  int len = strlen(buf);

  int fd = open(file, O_WRONLY | O_CLOEXEC);
  if (fd == -1)
    return false;
  if (write(fd, buf, len) != len) {
    close(fd);
    return false;
  }
  close(fd);
  return true;
}

static int inject_fault(int nth)
{
  int fd;
  char buf[16];

  fd = open("/proc/thread-self/fail-nth", O_RDWR);
  if (fd == -1)
    fail("failed to open /proc/thread-self/fail-nth");
  sprintf(buf, "%d", nth + 1);
  if (write(fd, buf, strlen(buf)) != (ssize_t)strlen(buf))
    fail("failed to write /proc/thread-self/fail-nth");
  return fd;
}

long r[35];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xf7e000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  r[1] = syscall(__NR_socket, 0x26ul, 0x5ul, 0x0ul);
  NONFAILING(*(uint16_t*)0x20f72000 = (uint16_t)0x26);
  NONFAILING(memcpy(
      (void*)0x20f72002,
      "\x61\x65\x61\x64\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 14));
  NONFAILING(*(uint32_t*)0x20f72010 = (uint32_t)0x0);
  NONFAILING(*(uint32_t*)0x20f72014 = (uint32_t)0x0);
  NONFAILING(memcpy((void*)0x20f72018,
                    "\x72\x66\x63\x37\x35\x33\x39\x28\x63\x74\x72\x28"
                    "\x73\x65\x72\x70\x65\x6e\x74\x29\x2c\x72\x6d\x64"
                    "\x31\x32\x38\x2d\x67\x65\x6e\x65\x72\x69\x63\x29"
                    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                    "\x00\x00\x00\x00",
                    64));
  r[7] = syscall(__NR_bind, r[1], 0x20f72000ul, 0x58ul);
  NONFAILING(memcpy((void*)0x201ec000,
                    "\x0a\x07\x75\xb0\xd5\xe3\x83\xe5\xb3\xb6\x0c\xed"
                    "\x5c\x54\xdb\xb7\x29\x5d\xf0\xdf\x82\x17\xad\x40"
                    "\x00\x00\x00\x00\x00\x00\x00\xe6",
                    32));
  r[9] = syscall(__NR_setsockopt, r[1], 0x117ul, 0x1ul, 0x201ec000ul,
                 0x20ul);
  r[10] = syscall(__NR_accept, r[1], 0x0ul, 0x0ul);
  NONFAILING(*(uint64_t*)0x20141f88 = (uint64_t)0x20876ff6);
  NONFAILING(*(uint32_t*)0x20141f90 = (uint32_t)0xa);
  NONFAILING(*(uint64_t*)0x20141f98 = (uint64_t)0x20f32000);
  NONFAILING(*(uint64_t*)0x20141fa0 = (uint64_t)0x1);
  NONFAILING(*(uint64_t*)0x20141fa8 = (uint64_t)0x20c3cce0);
  NONFAILING(*(uint64_t*)0x20141fb0 = (uint64_t)0x0);
  NONFAILING(*(uint32_t*)0x20141fb8 = (uint32_t)0x0);
  NONFAILING(*(uint32_t*)0x20141fc0 = (uint32_t)0x0);
  NONFAILING(*(uint16_t*)0x20876ff6 = (uint16_t)0x0);
  NONFAILING(
      memcpy((void*)0x20876ff8, "\x2e\x2f\x66\x69\x6c\x65\x30\x00", 8));
  NONFAILING(*(uint64_t*)0x20f32000 = (uint64_t)0x20f6f000);
  NONFAILING(*(uint64_t*)0x20f32008 = (uint64_t)0x10);
  NONFAILING(memcpy((void*)0x20f6f000, "\xf7\xe6\x4d\x45\xd2\x97\x91"
                                       "\x9d\x46\xa5\x46\xd8\xd8\x1d"
                                       "\x6d\x6c",
                    16));
  r[24] = syscall(__NR_sendmmsg, r[10], 0x20141f88ul, 0x1ul, 0x0ul);
  NONFAILING(*(uint64_t*)0x209c6fc8 = (uint64_t)0x20a06000);
  NONFAILING(*(uint32_t*)0x209c6fd0 = (uint32_t)0x58);
  NONFAILING(*(uint64_t*)0x209c6fd8 = (uint64_t)0x20233fd0);
  NONFAILING(*(uint64_t*)0x209c6fe0 = (uint64_t)0x1);
  NONFAILING(*(uint64_t*)0x209c6fe8 = (uint64_t)0x20e7b000);
  NONFAILING(*(uint64_t*)0x209c6ff0 = (uint64_t)0x0);
  NONFAILING(*(uint32_t*)0x209c6ff8 = (uint32_t)0x0);
  NONFAILING(*(uint64_t*)0x20233fd0 = (uint64_t)0x20be4ffb);
  NONFAILING(*(uint64_t*)0x20233fd8 = (uint64_t)0x1);
  write_file("/sys/kernel/debug/failslab/ignore-gfp-wait", "N");
  write_file("/sys/kernel/debug/fail_futex/ignore-private", "N");
  inject_fault(1);
  r[34] = syscall(__NR_recvmsg, r[10], 0x209c6fc8ul, 0x0ul);
}

int main()
{
  install_segv_handler();
  loop();
  return 0;
}
