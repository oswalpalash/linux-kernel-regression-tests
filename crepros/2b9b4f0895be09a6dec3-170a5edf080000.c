// https://syzkaller.appspot.com/bug?id=f55849742bdcdf58f6eaae61e747ac00e5c061f4
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

#ifndef __NR_userfaultfd
#define __NR_userfaultfd 323
#endif

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  syscall(__NR_mmap, 0x20000000ul, 0xfbe000ul, 2ul, 0x31ul, -1, 0ul);
  res = syscall(__NR_userfaultfd, 0x80001ul);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000000 = 0xaa;
  *(uint64_t*)0x20000008 = 0;
  *(uint64_t*)0x20000010 = 0;
  syscall(__NR_ioctl, r[0], 0xc018aa3f, 0x20000000ul);
  *(uint64_t*)0x20000080 = 0x200e2000;
  *(uint64_t*)0x20000088 = 0xc00000;
  *(uint64_t*)0x20000090 = 7;
  *(uint64_t*)0x20000098 = 0;
  syscall(__NR_ioctl, r[0], 0xc020aa00, 0x20000080ul);
  *(uint64_t*)0x200000c0 = 0x2063d000;
  *(uint64_t*)0x200000c8 = 0x3000;
  *(uint64_t*)0x200000d0 = 1;
  syscall(__NR_ioctl, r[0], 0xc018aa06, 0x200000c0ul);
  return 0;
}
