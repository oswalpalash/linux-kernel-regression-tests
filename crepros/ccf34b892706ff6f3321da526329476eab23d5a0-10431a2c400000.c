// https://syzkaller.appspot.com/bug?id=ccf34b892706ff6f3321da526329476eab23d5a0
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <errno.h>
#include <fcntl.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

const int kFailStatus = 67;
const int kRetryStatus = 69;

static void exitf(const char* msg, ...)
{
  int e = errno;
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit(kRetryStatus);
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
    int err = errno;
    close(fd);
    errno = err;
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
    exitf("failed to open /proc/thread-self/fail-nth");
  sprintf(buf, "%d", nth + 1);
  if (write(fd, buf, strlen(buf)) != (ssize_t)strlen(buf))
    exitf("failed to write /proc/thread-self/fail-nth");
  return fd;
}

void loop()
{
  syscall(__NR_mmap, 0x20000000, 0xc1f000, 2, 0x71, -1, 0);
  *(uint64_t*)0x20000000 = 5;
  syscall(__NR_mbind, 0x20072000, 0x4000, 3, 0x20000000, 0x401c, 0);
  write_file("/sys/kernel/debug/failslab/ignore-gfp-wait", "N");
  write_file("/sys/kernel/debug/fail_futex/ignore-private", "N");
  inject_fault(1);
  syscall(__NR_get_mempolicy, 0x2029a000, 0x203e8000, 0x401, 0x20377000, 2);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
