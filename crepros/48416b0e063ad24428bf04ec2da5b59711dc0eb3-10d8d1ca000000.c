// https://syzkaller.appspot.com/bug?id=48416b0e063ad24428bf04ec2da5b59711dc0eb3
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <errno.h>
#include <pthread.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

const int kFailStatus = 67;
const int kRetryStatus = 69;

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}

__attribute__((noreturn)) static void fail(const char* msg, ...)
{
  int e = errno;
  fflush(stdout);
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

static void use_temporary_dir()
{
  char tmpdir_template[] = "./syzkaller.XXXXXX";
  char* tmpdir = mkdtemp(tmpdir_template);
  if (!tmpdir)
    fail("failed to mkdtemp");
  if (chmod(tmpdir, 0777))
    fail("failed to chmod");
  if (chdir(tmpdir))
    fail("failed to chdir");
}

long r[7];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0xafe000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    NONFAILING(memcpy((void*)0x20af7000,
                      "\x2f\x64\x65\x76\x2f\x6b\x76\x6d\x00", 9));
    r[2] = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20af7000ul,
                   0x109000ul, 0x0ul);
    break;
  case 2:
    r[3] = syscall(__NR_ioctl, r[2], 0xae01ul, 0x0ul);
    break;
  case 3:
    r[4] = syscall(__NR_ioctl, r[3], 0xae41ul, 0x2000000000000ul);
    break;
  case 4:
    r[5] = syscall(__NR_ioctl, r[4], 0xae80ul, 0x0ul);
    break;
  case 5:
    r[6] = syscall(__NR_ioctl, r[4], 0xae80ul, 0x0ul);
    break;
  }
  return 0;
}

void loop()
{
  long i;
  pthread_t th[12];

  memset(r, -1, sizeof(r));
  srand(getpid());
  for (i = 0; i < 6; i++) {
    pthread_create(&th[i], 0, thr, (void*)i);
    usleep(rand() % 10000);
  }
  for (i = 0; i < 6; i++) {
    pthread_create(&th[6 + i], 0, thr, (void*)i);
    if (rand() % 2)
      usleep(rand() % 10000);
  }
  usleep(rand() % 100000);
}

int main()
{
  install_segv_handler();
  use_temporary_dir();
  loop();
  return 0;
}
