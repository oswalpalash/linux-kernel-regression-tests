// https://syzkaller.appspot.com/bug?id=26567b12e74b8791e1db50da6039ee1705e5a7ed
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  memcpy((void*)0x20000140, "/dev/fb1\000", 9);
  res =
      syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000140ul, 0x40000ul, 0ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000100 = 0x3f;
  *(uint32_t*)0x20000104 = 1;
  syscall(__NR_ioctl, r[0], 0x4610, 0x20000100ul);
  memcpy((void*)0x20000040, "/dev/fb1\000", 9);
  res =
      syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000040ul, 0x101200ul, 0ul);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x20000080 = 1;
  *(uint32_t*)0x20000084 = 1;
  syscall(__NR_ioctl, r[1], 0x4610, 0x20000080ul);
  return 0;
}
