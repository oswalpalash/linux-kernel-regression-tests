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

#ifndef __NR_connect
#define __NR_connect 203
#endif
#ifndef __NR_mmap
#define __NR_mmap 222
#endif
#ifndef __NR_socket
#define __NR_socket 198
#endif

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
  *(uint16_t*)0x200001c0 = 0x18;
  *(uint32_t*)0x200001c2 = 1;
  *(uint32_t*)0x200001c6 = 3;
  *(uint32_t*)0x200001ca = r[1];
  *(uint16_t*)0x200001ce = 2;
  *(uint16_t*)0x200001d0 = htobe16(0);
  *(uint32_t*)0x200001d2 = htobe32(0xe0000002);
  *(uint32_t*)0x200001de = 3;
  *(uint32_t*)0x200001e2 = 0;
  *(uint32_t*)0x200001e6 = 0;
  *(uint32_t*)0x200001ea = 0;
  syscall(__NR_connect, r[0], 0x200001c0ul, 0x2eul);
  return 0;
}
