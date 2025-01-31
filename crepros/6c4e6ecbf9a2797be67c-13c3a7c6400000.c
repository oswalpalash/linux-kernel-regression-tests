// https://syzkaller.appspot.com/bug?id=695e56989a39369134b51e9d4036acb65f09a08f
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
  char buf[16];
  fd = open("/proc/thread-self/fail-nth", O_RDWR);
  if (fd == -1)
    exit(1);
  sprintf(buf, "%d", nth + 1);
  if (write(fd, buf, strlen(buf)) != (ssize_t)strlen(buf))
    exit(1);
  return fd;
}

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  res = syscall(__NR_socket, 0xa, 1, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x200000c0 = 1;
  syscall(__NR_setsockopt, r[0], 6, 0x13, 0x200000c0, 0x151);
  *(uint16_t*)0x20000240 = 0xa;
  *(uint16_t*)0x20000242 = htobe16(0);
  *(uint32_t*)0x20000244 = 0;
  *(uint8_t*)0x20000248 = 0;
  *(uint8_t*)0x20000249 = 0;
  *(uint8_t*)0x2000024a = 0;
  *(uint8_t*)0x2000024b = 0;
  *(uint8_t*)0x2000024c = 0;
  *(uint8_t*)0x2000024d = 0;
  *(uint8_t*)0x2000024e = 0;
  *(uint8_t*)0x2000024f = 0;
  *(uint8_t*)0x20000250 = 0;
  *(uint8_t*)0x20000251 = 0;
  *(uint8_t*)0x20000252 = 0;
  *(uint8_t*)0x20000253 = 0;
  *(uint8_t*)0x20000254 = 0;
  *(uint8_t*)0x20000255 = 0;
  *(uint8_t*)0x20000256 = 0;
  *(uint8_t*)0x20000257 = 0;
  *(uint32_t*)0x20000258 = 0;
  *(uint16_t*)0x200002c0 = 0;
  *(uint16_t*)0x200002c2 = 0x1f;
  *(uint32_t*)0x200002c4 = 0;
  memcpy((void*)0x200002c8,
         "\x38\x16\x70\xe2\xfa\xdc\x5c\xd7\xdc\xa9\x02\xf1\x84\x8d\x33\xe3\x38"
         "\xdd\xe6\x99\x12\x20\xc2\xd9\x21\xe7\x4a\x86\x66\xc9\x67\x13\xf3\xcf"
         "\xd5\x5c\x2c\xa6\x2a\xbd\x0e\xab\x3d\x68\xff\x06\x1e\xe8\x40\x96\xa5"
         "\xe8\x5b\x6f\xcc\x57\xc7\x7e\xe0\xb0\x1c\x3b\xb3\xd8\xd2\xf3\x62\x71"
         "\xdb\x3f\x12\x86\x1a\x12\x1b\x89\x18\xd6\xf7\x4b",
         80);
  syscall(__NR_setsockopt, r[0], 6, 0xe, 0x20000240, 0xd8);
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
  syscall(__NR_connect, r[0], 0x20000140, 0x1c);
  memcpy((void*)0x20000540, "tls", 4);
  syscall(__NR_setsockopt, r[0], 6, 0x1f, 0x20000540, 4);
  syscall(__NR_mmap, 0x20003000, 0xffc000, 2, 0x32, -1, 0);
  *(uint16_t*)0x20000100 = 0x303;
  *(uint16_t*)0x20000102 = 0x33;
  syscall(__NR_setsockopt, r[0], 0x11a, 1, 0x20000100, 0x28);
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
  syscall(__NR_sendto, r[0], 0x200005c0, 0xfffffdef, 0x40, 0x20000000, 0x1c);
  return 0;
}
