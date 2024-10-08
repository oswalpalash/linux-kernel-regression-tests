// https://syzkaller.appspot.com/bug?id=40d69d7e4e6b8b6effdbee578ee417baa6da281a
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
  fd = open("/proc/thread-self/fail-nth", O_RDWR);
  if (fd == -1)
    exit(1);
  char buf[16];
  sprintf(buf, "%d", nth + 1);
  if (write(fd, buf, strlen(buf)) != (ssize_t)strlen(buf))
    exit(1);
  return fd;
}

static void setup_fault()
{
  static struct {
    const char* file;
    const char* val;
    bool fatal;
  } files[] = {
      {"/sys/kernel/debug/failslab/ignore-gfp-wait", "N", true},
      {"/sys/kernel/debug/fail_futex/ignore-private", "N", false},
      {"/sys/kernel/debug/fail_page_alloc/ignore-gfp-highmem", "N", false},
      {"/sys/kernel/debug/fail_page_alloc/ignore-gfp-wait", "N", false},
      {"/sys/kernel/debug/fail_page_alloc/min-order", "0", false},
  };
  unsigned i;
  for (i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
    if (!write_file(files[i].file, files[i].val)) {
      if (files[i].fatal)
        exit(1);
    }
  }
}

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  setup_fault();
  intptr_t res = 0;
  res = syscall(__NR_socket, 2, 2, 0x88);
  if (res != -1)
    r[0] = res;
  syscall(__NR_close, r[0]);
  res = syscall(__NR_socket, 0x2c, 3, 0);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x20000040 = r[0];
  syscall(__NR_ioctl, 0xffffff9c, 0x89e2, 0x20000040);
  *(uint64_t*)0x20000080 = 0x20000000;
  *(uint64_t*)0x20000088 = 0x12c000;
  *(uint32_t*)0x20000090 = 0x800;
  *(uint32_t*)0x20000094 = 0;
  syscall(__NR_setsockopt, r[1], 0x11b, 4, 0x20000080, 0x18);
  *(uint32_t*)0x200000c0 = 0x8000;
  syscall(__NR_setsockopt, r[1], 0x11b, 5, 0x200000c0, 4);
  *(uint32_t*)0x20003a80 = 0x40;
  syscall(__NR_setsockopt, r[1], 0x11b, 2, 0x20003a80, 4);
  *(uint32_t*)0x20000280 = 0x11;
  *(uint32_t*)0x20000284 = 4;
  *(uint32_t*)0x20000288 = 4;
  *(uint32_t*)0x2000028c = 0xf59;
  *(uint32_t*)0x20000290 = 0;
  *(uint32_t*)0x20000294 = -1;
  *(uint32_t*)0x20000298 = 0;
  *(uint8_t*)0x2000029c = 0;
  *(uint8_t*)0x2000029d = 0;
  *(uint8_t*)0x2000029e = 0;
  *(uint8_t*)0x2000029f = 0;
  *(uint8_t*)0x200002a0 = 0;
  *(uint8_t*)0x200002a1 = 0;
  *(uint8_t*)0x200002a2 = 0;
  *(uint8_t*)0x200002a3 = 0;
  *(uint8_t*)0x200002a4 = 0;
  *(uint8_t*)0x200002a5 = 0;
  *(uint8_t*)0x200002a6 = 0;
  *(uint8_t*)0x200002a7 = 0;
  *(uint8_t*)0x200002a8 = 0;
  *(uint8_t*)0x200002a9 = 0;
  *(uint8_t*)0x200002aa = 0;
  *(uint8_t*)0x200002ab = 0;
  *(uint32_t*)0x200002ac = 0;
  *(uint32_t*)0x200002b0 = -1;
  *(uint32_t*)0x200002b4 = 0;
  *(uint32_t*)0x200002b8 = 0;
  res = syscall(__NR_bpf, 0, 0x20000280, 0x2c);
  if (res != -1)
    r[2] = res;
  *(uint32_t*)0x20000100 = r[2];
  *(uint64_t*)0x20000108 = 0x200001c0;
  *(uint64_t*)0x20000110 = 0x20000040;
  *(uint64_t*)0x20000118 = 0;
  inject_fault(2);
  syscall(__NR_bpf, 2, 0x20000100, 0x20);
  return 0;
}
