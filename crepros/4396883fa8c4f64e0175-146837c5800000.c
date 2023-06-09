// https://syzkaller.appspot.com/bug?id=be2e39518462291fd049f665261632ded34b79fd
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
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
#include <stdint.h>
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

  if (unshare(CLONE_NEWNS)) {
  }
  if (unshare(CLONE_NEWIPC)) {
  }
  if (unshare(CLONE_NEWCGROUP)) {
  }
  if (unshare(CLONE_NEWUTS)) {
  }
  if (unshare(CLONE_SYSVSEM)) {
  }
}

static int do_sandbox_none(int executor_pid, bool enable_tun)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid < 0)
    fail("sandbox fork failed");
  if (pid)
    return pid;

  sandbox_common();
  if (unshare(CLONE_NEWNET)) {
  }

  loop();
  doexit(1);
}

static void test();

void loop()
{
  while (1) {
    test();
  }
}

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t procid;
void test()
{
  syscall(__NR_mmap, 0x20000000, 0xeb3000, 3, 0x32, -1, 0);
  *(uint32_t*)0x203dc000 = 6;
  *(uint32_t*)0x203dc004 = 4;
  *(uint32_t*)0x203dc008 = 0x338d;
  *(uint32_t*)0x203dc00c = 0x20004;
  *(uint32_t*)0x203dc010 = 0;
  *(uint32_t*)0x203dc014 = 0;
  *(uint32_t*)0x203dc018 = 0;
  *(uint8_t*)0x203dc01c = 0;
  *(uint8_t*)0x203dc01d = 0;
  *(uint8_t*)0x203dc01e = 0;
  *(uint8_t*)0x203dc01f = 0;
  *(uint8_t*)0x203dc020 = 0;
  *(uint8_t*)0x203dc021 = 0;
  *(uint8_t*)0x203dc022 = 0;
  *(uint8_t*)0x203dc023 = 0;
  *(uint8_t*)0x203dc024 = 0;
  *(uint8_t*)0x203dc025 = 0;
  *(uint8_t*)0x203dc026 = 0;
  *(uint8_t*)0x203dc027 = 0;
  *(uint8_t*)0x203dc028 = 0;
  *(uint8_t*)0x203dc029 = 0;
  *(uint8_t*)0x203dc02a = 0;
  *(uint8_t*)0x203dc02b = 0;
  syscall(__NR_bpf, 0, 0x203dc000, 0x2c);
}

int main()
{
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      for (;;) {
        int pid = do_sandbox_none(procid, false);
        int status = 0;
        while (waitpid(pid, &status, __WALL) != pid) {
        }
      }
    }
  }
  sleep(1000000);
  return 0;
}
