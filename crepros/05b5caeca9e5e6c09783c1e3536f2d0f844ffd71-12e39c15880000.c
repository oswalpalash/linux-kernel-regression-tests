// https://syzkaller.appspot.com/bug?id=05b5caeca9e5e6c09783c1e3536f2d0f844ffd71
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
  *(uint64_t*)0x20005c00 = 0;
  *(uint32_t*)0x20005c08 = 0;
  *(uint64_t*)0x20005c10 = 0;
  *(uint64_t*)0x20005c18 = 0;
  *(uint64_t*)0x20005c20 = 0x20000680;
  memcpy((void*)0x20000680,
         "\x18\x00\x00\x00\x00\x00\x00\x00\x10\x01\x00\x00\x01", 13);
  *(uint64_t*)0x20005c28 = 0x18;
  *(uint32_t*)0x20005c30 = 0;
  *(uint32_t*)0x20005c38 = 0;
  syscall(__NR_sendmmsg, -1, 0x20005c00ul, 1ul, 0ul);
  res = syscall(__NR_socket, 0x2000000000000021ul, 2ul, 2);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20000080 = 0x21;
  *(uint16_t*)0x20000082 = 0;
  *(uint16_t*)0x20000084 = 2;
  *(uint16_t*)0x20000086 = 0x1c;
  *(uint16_t*)0x20000088 = 0xa;
  *(uint16_t*)0x2000008a = htobe16(0);
  *(uint32_t*)0x2000008c = htobe32(0);
  memset((void*)0x20000090, 0, 16);
  *(uint32_t*)0x200000a0 = 0;
  syscall(__NR_connect, r[0], 0x20000080ul, 0x24ul);
  syscall(__NR_sendmmsg, r[0], 0x20005c00ul, 0x40000000000037bul, 0xc000ul);
  return 0;
}
