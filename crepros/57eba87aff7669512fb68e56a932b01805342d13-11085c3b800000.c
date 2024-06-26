// https://syzkaller.appspot.com/bug?id=57eba87aff7669512fb68e56a932b01805342d13
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

#define BITMASK_LEN(type, bf_len) (type)((1ull << (bf_len)) - 1)

#define BITMASK_LEN_OFF(type, bf_off, bf_len)                                  \
  (type)(BITMASK_LEN(type, (bf_len)) << (bf_off))

#define STORE_BY_BITMASK(type, addr, val, bf_off, bf_len)                      \
  if ((bf_off) == 0 && (bf_len) == 0) {                                        \
    *(type*)(addr) = (type)(val);                                              \
  } else {                                                                     \
    type new_val = *(type*)(addr);                                             \
    new_val &= ~BITMASK_LEN_OFF(type, (bf_off), (bf_len));                     \
    new_val |= ((type)(val)&BITMASK_LEN(type, (bf_len))) << (bf_off);          \
    *(type*)(addr) = new_val;                                                  \
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

uint64_t r[1] = {0xffffffffffffffff};
void loop()
{
  long res = 0;
  *(uint32_t*)0x20348f88 = 2;
  *(uint32_t*)0x20348f8c = 0x70;
  *(uint8_t*)0x20348f90 = 0xe6;
  *(uint8_t*)0x20348f91 = 0;
  *(uint8_t*)0x20348f92 = 0;
  *(uint8_t*)0x20348f93 = 0;
  *(uint32_t*)0x20348f94 = 0;
  *(uint64_t*)0x20348f98 = 0;
  *(uint64_t*)0x20348fa0 = 0;
  *(uint64_t*)0x20348fa8 = 0;
  STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 0, 1);
  STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 1, 1);
  STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 2, 1);
  STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 3, 1);
  STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 4, 1);
  STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 5, 1);
  STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 6, 1);
  STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 7, 1);
  STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 8, 1);
  STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 9, 1);
  STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 10, 1);
  STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 11, 1);
  STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 12, 1);
  STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 13, 1);
  STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 14, 1);
  STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 15, 2);
  STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 17, 1);
  STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 18, 1);
  STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 19, 1);
  STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 20, 1);
  STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 21, 1);
  STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 22, 1);
  STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 23, 1);
  STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 24, 1);
  STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 25, 1);
  STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 26, 1);
  STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 27, 1);
  STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 28, 1);
  STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 29, 35);
  *(uint32_t*)0x20348fb8 = 0;
  *(uint32_t*)0x20348fbc = 0;
  *(uint64_t*)0x20348fc0 = 0x20000000;
  *(uint64_t*)0x20348fc8 = 0;
  *(uint64_t*)0x20348fd0 = 0;
  *(uint64_t*)0x20348fd8 = 0;
  *(uint32_t*)0x20348fe0 = 0;
  *(uint32_t*)0x20348fe4 = 0;
  *(uint64_t*)0x20348fe8 = 0;
  *(uint32_t*)0x20348ff0 = 0;
  *(uint16_t*)0x20348ff4 = 0;
  *(uint16_t*)0x20348ff6 = 0;
  syscall(__NR_perf_event_open, 0x20348f88, 0, 0, -1, 0);
  *(uint32_t*)0x20000080 = 1;
  syscall(__NR_getsockopt, -1, 0, 0x61, 0x20000940, 0x20000080);
  memcpy((void*)0x20000000, "/dev/ppp", 9);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000000, 0, 0);
  if (res != -1)
    r[0] = res;
  write_file("/sys/kernel/debug/failslab/ignore-gfp-wait", "N");
  write_file("/sys/kernel/debug/fail_futex/ignore-private", "N");
  inject_fault(6);
  syscall(__NR_ioctl, r[0], 0xc004743e, 0x20000a80);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
