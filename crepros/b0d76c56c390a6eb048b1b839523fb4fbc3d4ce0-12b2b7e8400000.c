// https://syzkaller.appspot.com/bug?id=b0d76c56c390a6eb048b1b839523fb4fbc3d4ce0
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

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  res = syscall(__NR_socket, 0x10, 3, 0x10);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000040 = 0x20000000;
  *(uint16_t*)0x20000000 = 0x10;
  *(uint16_t*)0x20000002 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x20000048 = 0xc;
  *(uint64_t*)0x20000050 = 0x20000080;
  *(uint64_t*)0x20000080 = 0x20000140;
  *(uint32_t*)0x20000140 = 0x20;
  *(uint16_t*)0x20000144 = 0x26;
  *(uint16_t*)0x20000146 = 1;
  *(uint32_t*)0x20000148 = 0;
  *(uint32_t*)0x2000014c = 0;
  *(uint8_t*)0x20000150 = 1;
  *(uint8_t*)0x20000151 = 0;
  *(uint16_t*)0x20000152 = 0;
  *(uint16_t*)0x20000154 = 0xc;
  *(uint16_t*)0x20000156 = 1;
  *(uint64_t*)0x20000158 = 0;
  *(uint64_t*)0x20000088 = 0x20;
  *(uint64_t*)0x20000058 = 1;
  *(uint64_t*)0x20000060 = 0;
  *(uint64_t*)0x20000068 = 0;
  *(uint32_t*)0x20000070 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000040, 0);
  return 0;
}
