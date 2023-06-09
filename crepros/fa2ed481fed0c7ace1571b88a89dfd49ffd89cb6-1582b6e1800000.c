// https://syzkaller.appspot.com/bug?id=d767177245c54af614d5241159cce56995eef0db
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
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

static uint64_t current_time_ms()
{
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    fail("clock_gettime failed");
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
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

long r[3];
void test()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000ul, 0xf7f000ul, 0x3ul, 0x32ul,
          0xfffffffffffffffful, 0x0ul);
  r[0] = syscall(__NR_socket, 0x26ul, 0x5ul, 0x0ul);
  *(uint16_t*)0x20590fa8 = (uint16_t)0x26;
  memcpy((void*)0x20590faa,
         "\x73\x6b\x63\x69\x70\x68\x65\x72\x00\x00\x00\x00\x00\x00",
         14);
  *(uint32_t*)0x20590fb8 = (uint32_t)0x0;
  *(uint32_t*)0x20590fbc = (uint32_t)0x0;
  memcpy((void*)0x20590fc0,
         "\x63\x68\x61\x63\x68\x61\x32\x30\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00",
         64);
  syscall(__NR_bind, r[0], 0x20590fa8ul, 0x58ul);
  memcpy((void*)0x201ec000, "\x0a\x07\x75\xb0\xd5\xe3\x83\xe5\xb3\xb6"
                            "\x0c\xed\x5c\x54\xdb\xb7\x29\x5d\xf0\xdf"
                            "\x82\x17\xad\x40\x00\x00\x00\x00\x00\x00"
                            "\x00\xe6",
         32);
  syscall(__NR_setsockopt, r[0], 0x117ul, 0x1ul, 0x201ec000ul, 0x20ul);
  r[1] = syscall(__NR_accept, r[0], 0x0ul, 0x0ul);
  if (syscall(__NR_io_setup, 0x1ul, 0x20479000ul) != -1) {
    r[2] = *(uint64_t*)0x20479000;
  }
  *(uint64_t*)0x20738000 = (uint64_t)0x20f73fc0;
  *(uint64_t*)0x20f73fc0 = (uint64_t)0x0;
  *(uint32_t*)0x20f73fc8 = (uint32_t)0x0;
  *(uint32_t*)0x20f73fcc = (uint32_t)0x0;
  *(uint16_t*)0x20f73fd0 = (uint16_t)0x0;
  *(uint16_t*)0x20f73fd2 = (uint16_t)0x0;
  *(uint32_t*)0x20f73fd4 = r[1];
  *(uint64_t*)0x20f73fd8 = (uint64_t)0x2079a000;
  *(uint64_t*)0x20f73fe0 = (uint64_t)0x1;
  *(uint64_t*)0x20f73fe8 = (uint64_t)0x0;
  *(uint64_t*)0x20f73ff0 = (uint64_t)0x0;
  *(uint32_t*)0x20f73ff8 = (uint32_t)0x0;
  *(uint32_t*)0x20f73ffc = (uint32_t)0xffffffffffffffff;
  memcpy((void*)0x2079a000, "\x16", 1);
  syscall(__NR_io_submit, r[2], 0x1ul, 0x20738000ul);
}

int main()
{
  int i;
  for (i = 0; i < 8; i++) {
    if (fork() == 0) {
      loop();
      return 0;
    }
  }
  sleep(1000000);
  return 0;
}
