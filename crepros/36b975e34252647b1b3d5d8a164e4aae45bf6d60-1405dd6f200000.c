// https://syzkaller.appspot.com/bug?id=36b975e34252647b1b3d5d8a164e4aae45bf6d60
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
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x20000140, "/selinux/load\000", 14);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000140, 2, 0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000180,
         "\x8c\xff\x7c\xf9\x08\x00\x00\x00\x53\x45\x20\x4c\x69\x6e\x75\x78\x15"
         "\x00\x00\x00\x00\xf6\x00\x00\x08\x00\x00\x00\x07\x00\x00\x00\x40\x2c"
         "\x11\x00\x00\x00\x00\x00\x09\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x01\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x6d\xbd\x00\x60\x15\x24\xc9",
         80);
  syscall(__NR_write, r[0], 0x20000180, 0x50);
  return 0;
}
