// https://syzkaller.appspot.com/bug?id=c7fd949e1dcb317375d8c795cbb5965420e0c12d
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
  memcpy((void*)0x20002000, "./bus", 6);
  res = syscall(__NR_open, 0x20002000, 0x141042, 0);
  if (res != -1)
    r[0] = res;
  syscall(__NR_ioctl, r[0], 0x6611);
  return 0;
}
