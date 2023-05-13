// https://syzkaller.appspot.com/bug?id=e46c3d7e02688fb37386c0de69c51ff28b722774
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
void loop()
{
  long res = 0;
  res = syscall(__NR_socket, 0xa, 0x1000000000002, 0);
  if (res != -1)
    r[0] = res;
  syscall(__NR_ioctl, r[0], 0x8912, 0x20000280);
  res = syscall(__NR_socket, 0xa, 1, 0);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x200000c0 = 1;
  syscall(__NR_setsockopt, r[1], 6, 0x13, 0x200000c0, 0x151);
  *(uint16_t*)0x20000140 = 0xa;
  *(uint16_t*)0x20000142 = htobe16(0);
  *(uint32_t*)0x20000144 = 0;
  *(uint8_t*)0x20000148 = 0;
  *(uint8_t*)0x20000149 = 0;
  *(uint8_t*)0x2000014a = 0;
  *(uint8_t*)0x2000014b = 0;
  *(uint8_t*)0x2000014c = 0;
  *(uint8_t*)0x2000014d = 0;
  *(uint8_t*)0x2000014e = 0;
  *(uint8_t*)0x2000014f = 0;
  *(uint8_t*)0x20000150 = 0;
  *(uint8_t*)0x20000151 = 0;
  *(uint8_t*)0x20000152 = 0;
  *(uint8_t*)0x20000153 = 0;
  *(uint8_t*)0x20000154 = 0;
  *(uint8_t*)0x20000155 = 0;
  *(uint8_t*)0x20000156 = 0;
  *(uint8_t*)0x20000157 = 0;
  *(uint32_t*)0x20000158 = 0;
  syscall(__NR_connect, r[1], 0x20000140, 0x1c);
  memcpy((void*)0x20000080, "tls", 4);
  syscall(__NR_setsockopt, r[1], 6, 0x1f, 0x20000080, 0x152);
  syscall(__NR_mmap, 0x20003000, 0xffc000, 2, 0x32, -1, 0);
  *(uint16_t*)0x20000100 = 0x303;
  *(uint16_t*)0x20000102 = 0x33;
  syscall(__NR_setsockopt, r[1], 0x11a, 1, 0x20000100, 0x28);
  *(uint16_t*)0x20000000 = 0xa;
  *(uint16_t*)0x20000002 = htobe16(0);
  *(uint32_t*)0x20000004 = 0;
  *(uint8_t*)0x20000008 = 0xfe;
  *(uint8_t*)0x20000009 = 0x80;
  *(uint8_t*)0x2000000a = 0;
  *(uint8_t*)0x2000000b = 0;
  *(uint8_t*)0x2000000c = 0;
  *(uint8_t*)0x2000000d = 0;
  *(uint8_t*)0x2000000e = 0;
  *(uint8_t*)0x2000000f = 0;
  *(uint8_t*)0x20000010 = 0;
  *(uint8_t*)0x20000011 = 0;
  *(uint8_t*)0x20000012 = 0;
  *(uint8_t*)0x20000013 = 0;
  *(uint8_t*)0x20000014 = 0;
  *(uint8_t*)0x20000015 = 0;
  *(uint8_t*)0x20000016 = 0;
  *(uint8_t*)0x20000017 = 0xbb;
  *(uint32_t*)0x20000018 = 0;
  write_file("/sys/kernel/debug/failslab/ignore-gfp-wait", "N");
  write_file("/sys/kernel/debug/fail_futex/ignore-private", "N");
  inject_fault(2);
  syscall(__NR_sendto, r[1], 0x200005c0, 0xfffffdef, 0x40, 0x20000000, 0x1c);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
