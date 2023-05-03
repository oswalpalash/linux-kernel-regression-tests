// https://syzkaller.appspot.com/bug?id=35a0665d90458093808bc8b1887ebd7d3c095aec
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
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

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

long r[2];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  memcpy((void*)0x209feff6, "/dev/cuse", 10);
  r[0] = syscall(__NR_openat, 0xffffffffffffff9c, 0x209feff6, 0, 0);
  memcpy((void*)0x20000000, "/dev/ion", 9);
  r[1] = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000000, 0, 0);
  syscall(__NR_close, r[0]);
  *(uint64_t*)0x20001fc8 = 0x20000000;
  *(uint32_t*)0x20001fd0 = 0;
  *(uint64_t*)0x20001fd8 = 0x2042b000;
  *(uint64_t*)0x20001fe0 = 0;
  *(uint64_t*)0x20001fe8 = 0x20002000;
  *(uint64_t*)0x20001ff0 = 0x10;
  *(uint32_t*)0x20001ff8 = 0;
  *(uint64_t*)0x20002000 = 0x10;
  *(uint32_t*)0x20002008 = 0x10e;
  *(uint32_t*)0x2000200c = 0;
  syscall(__NR_sendmsg, -1, 0x20001fc8, 0);
  syscall(__NR_ioctl, r[1], 0xc0184900, 0x20002000);
  *(uint64_t*)0x2071b000 = 2;
  syscall(__NR_ioctl, r[0], 0x40086200, 0x2071b000);
}

int main()
{
  use_temporary_dir();
  loop();
  return 0;
}
