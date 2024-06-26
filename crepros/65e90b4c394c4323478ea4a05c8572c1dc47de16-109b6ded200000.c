// https://syzkaller.appspot.com/bug?id=65e90b4c394c4323478ea4a05c8572c1dc47de16
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

#ifndef __NR_io_uring_setup
#define __NR_io_uring_setup 425
#endif

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);

  *(uint32_t*)0x20000200 = 0;
  *(uint32_t*)0x20000204 = 0;
  *(uint32_t*)0x20000208 = 6;
  *(uint32_t*)0x2000020c = 1;
  *(uint32_t*)0x20000210 = 0xcc;
  *(uint32_t*)0x20000214 = 0;
  *(uint32_t*)0x20000218 = 0;
  *(uint32_t*)0x2000021c = 0;
  *(uint32_t*)0x20000220 = 0;
  *(uint32_t*)0x20000224 = 0;
  *(uint32_t*)0x20000228 = 0;
  *(uint32_t*)0x2000022c = 0;
  *(uint32_t*)0x20000230 = 0;
  *(uint32_t*)0x20000234 = 0;
  *(uint32_t*)0x20000238 = 0;
  *(uint32_t*)0x2000023c = 0;
  *(uint32_t*)0x20000240 = 0;
  *(uint32_t*)0x20000244 = 0;
  *(uint64_t*)0x20000248 = 0;
  *(uint32_t*)0x20000250 = 0;
  *(uint32_t*)0x20000254 = 0;
  *(uint32_t*)0x20000258 = 0;
  *(uint32_t*)0x2000025c = 0;
  *(uint32_t*)0x20000260 = 0;
  *(uint32_t*)0x20000264 = 0;
  *(uint32_t*)0x20000268 = 0;
  *(uint32_t*)0x2000026c = 0;
  *(uint64_t*)0x20000270 = 0;
  syscall(__NR_io_uring_setup, 0xd44, 0x20000200);
  return 0;
}
