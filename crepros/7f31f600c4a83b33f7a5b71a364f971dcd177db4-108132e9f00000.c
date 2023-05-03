// https://syzkaller.appspot.com/bug?id=7f31f600c4a83b33f7a5b71a364f971dcd177db4
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
#include <sys/ioctl.h>
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

static long syz_open_procfs(volatile long a0, volatile long a1)
{
  char buf[128];
  memset(buf, 0, sizeof(buf));
  if (a0 == 0) {
    snprintf(buf, sizeof(buf), "/proc/self/%s", (char*)a1);
  } else if (a0 == -1) {
    snprintf(buf, sizeof(buf), "/proc/thread-self/%s", (char*)a1);
  } else {
    snprintf(buf, sizeof(buf), "/proc/self/task/%d/%s", (int)a0, (char*)a1);
  }
  int fd = open(buf, O_RDWR);
  if (fd == -1)
    fd = open(buf, O_RDONLY);
  return fd;
}

static long syz_open_pts(volatile long a0, volatile long a1)
{
  int ptyno = 0;
  if (ioctl(a0, TIOCGPTN, &ptyno))
    return -1;
  char buf[128];
  sprintf(buf, "/dev/pts/%d", ptyno);
  return open(buf, a1, 0);
}

static int inject_fault(int nth)
{
  int fd;
  fd = open("/proc/thread-self/fail-nth", O_RDWR);
  if (fd == -1)
    exit(1);
  char buf[16];
  sprintf(buf, "%d", nth);
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

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  setup_fault();
  intptr_t res = 0;
  memcpy((void*)0x20000080, "net/dev_snmp6\000", 14);
  res = -1;
  res = syz_open_procfs(0, 0x20000080);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000040, "/dev/ptmx\000", 10);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000040ul, 0ul, 0ul);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x200000c0 = 0;
  *(uint32_t*)0x200000c4 = 0;
  *(uint32_t*)0x200000c8 = 0;
  *(uint32_t*)0x200000cc = 0;
  *(uint8_t*)0x200000d0 = 0;
  memset((void*)0x200000d1, 0, 19);
  syscall(__NR_ioctl, r[1], 0x40045431, 0x200000c0ul);
  res = -1;
  res = syz_open_pts(r[1], 0x4000000000000002);
  if (res != -1)
    r[2] = res;
  res = syscall(__NR_dup3, r[2], r[0], 0ul);
  if (res != -1)
    r[3] = res;
  *(uint32_t*)0x200001c0 = 0x14;
  *(uint8_t*)0x200001c4 = 0x69;
  *(uint16_t*)0x200001c5 = 0;
  *(uint8_t*)0x200001c7 = 0;
  *(uint32_t*)0x200001c8 = 0;
  *(uint64_t*)0x200001cc = 0;
  inject_fault(3);
  syscall(__NR_write, r[0], 0x200001c0ul, 0xffffffdeul);
  syscall(__NR_ioctl, r[3], 0x540a, 1ul);
  return 0;
}
