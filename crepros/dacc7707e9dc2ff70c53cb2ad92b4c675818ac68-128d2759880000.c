// https://syzkaller.appspot.com/bug?id=dacc7707e9dc2ff70c53cb2ad92b4c675818ac68
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
  res = syscall(__NR_socket, 0x18ul, 1ul, 1);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_socket, 0xaul, 2ul, 0);
  if (res != -1)
    r[1] = res;
  *(uint16_t*)0x20000000 = 0x18;
  *(uint32_t*)0x20000002 = 1;
  *(uint32_t*)0x20000006 = 3;
  *(uint32_t*)0x2000000a = r[1];
  *(uint16_t*)0x2000000e = 2;
  *(uint16_t*)0x20000010 = htobe16(0);
  *(uint8_t*)0x20000012 = 0xac;
  *(uint8_t*)0x20000013 = 0x14;
  *(uint8_t*)0x20000014 = 0x14;
  *(uint8_t*)0x20000015 = 0;
  *(uint32_t*)0x2000001e = 3;
  *(uint32_t*)0x20000022 = 0;
  *(uint32_t*)0x20000026 = 0;
  *(uint32_t*)0x2000002a = 0;
  syscall(__NR_connect, r[0], 0x20000000ul, 0x2eul);
  return 0;
}
