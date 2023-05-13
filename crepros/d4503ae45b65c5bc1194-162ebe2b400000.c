// https://syzkaller.appspot.com/bug?id=7022420cc54310220ebad2da89e499bdb1f0f5e8
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

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
    strncpy(buf, (char*)a0, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
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
    exit(1);
  sprintf(buf, "%d", nth + 1);
  if (write(fd, buf, strlen(buf)) != (ssize_t)strlen(buf))
    exit(1);
  return fd;
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  syscall(__NR_mmap, 0x20000000, 0xfbe000, 0, 0x10, -1, 0);
  res = syscall(__NR_socket, 0xa, 0x803, 3);
  if (res != -1)
    r[0] = res;
  syscall(__NR_ioctl, r[0], 0x1000008912, 0);
  memcpy((void*)0x20000140, "/dev/dsp#", 10);
  res = syz_open_dev(0x20000140, 1, 2);
  if (res != -1)
    r[1] = res;
  *(uint64_t*)0x20000040 = 0;
  syscall(__NR_ioctl, r[1], 0x800000c004500a, 0x20000040);
  *(uint32_t*)0x200000c0 = 0xffffff71;
  *(uint32_t*)0x200000c4 = 5;
  *(uint64_t*)0x200000c8 = 0x100000000000d00;
  *(uint64_t*)0x200000d0 = 0x186a0;
  *(uint64_t*)0x200000d8 = 0;
  *(uint64_t*)0x200000e0 = 0;
  *(uint32_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000ec = 0;
  write_file("/sys/kernel/debug/failslab/ignore-gfp-wait", "N");
  write_file("/sys/kernel/debug/fail_futex/ignore-private", "N");
  inject_fault(6);
  syscall(__NR_write, r[1], 0x200000c0, 0x56da83a0);
  return 0;
}
