// https://syzkaller.appspot.com/bug?id=4b1e841004ca235843fe3dd609a5dda6d3fb9a3d
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  memcpy((void*)0x20000000, "/dev/qrtr-tun\000", 14);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000000ul, 0x8041ul, 0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000040,
         "\x03\x07\xbf\x00\xfe\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  syscall(__NR_write, r[0], 0x20000040ul, 0x10ul);
  return 0;
}
