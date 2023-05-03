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

uint64_t r[1] = {0xffffffffffffffff};
void loop()
{
  long res = 0;
  res = syscall(__NR_socket, 0x10, 0x200000000002, 0xc);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20390000, "\x1f\x00\x00\x00\x00\x07\xfd\x02\x01\x0c\x00\xf6"
                            "\x00\x00\xff\x00\x81\x09\xf8\x00\x09\x00\x01\x80"
                            "\x06\x08\x0b\xec\x00\x00\x00",
         31);
  syscall(__NR_write, r[0], 0x20390000, 0x1f);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  use_temporary_dir();
  loop();
  return 0;
}
