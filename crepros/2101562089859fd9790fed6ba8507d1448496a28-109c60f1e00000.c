// https://syzkaller.appspot.com/bug?id=2101562089859fd9790fed6ba8507d1448496a28
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

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_sendmsg
#define __NR_sendmsg 370
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10, 3, 0xc);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000140 = 0;
  *(uint32_t*)0x20000144 = 0;
  *(uint32_t*)0x20000148 = 0x20000100;
  *(uint32_t*)0x20000100 = 0x20000040;
  *(uint32_t*)0x20000040 = 0x5c;
  *(uint8_t*)0x20000044 = 2;
  *(uint8_t*)0x20000045 = 6;
  *(uint16_t*)0x20000046 = 0x101;
  *(uint32_t*)0x20000048 = 0;
  *(uint32_t*)0x2000004c = 0xf0ffff;
  *(uint8_t*)0x20000050 = 0;
  *(uint8_t*)0x20000051 = 0;
  *(uint16_t*)0x20000052 = htobe16(0);
  *(uint16_t*)0x20000054 = 5;
  *(uint16_t*)0x20000056 = 1;
  *(uint8_t*)0x20000058 = 7;
  *(uint16_t*)0x2000005c = 5;
  *(uint16_t*)0x2000005e = 4;
  *(uint8_t*)0x20000060 = 0;
  *(uint16_t*)0x20000064 = 0x10;
  *(uint16_t*)0x20000066 = 3;
  memcpy((void*)0x20000068, "bitmap:port\000", 12);
  *(uint16_t*)0x20000074 = 0x14;
  STORE_BY_BITMASK(uint16_t, , 0x20000076, 7, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000077, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000077, 1, 7, 1);
  *(uint16_t*)0x20000078 = 6;
  STORE_BY_BITMASK(uint16_t, , 0x2000007a, 5, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000007b, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000007b, 0, 7, 1);
  *(uint16_t*)0x2000007c = htobe16(0);
  *(uint16_t*)0x20000080 = 6;
  STORE_BY_BITMASK(uint16_t, , 0x20000082, 4, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000083, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000083, 0, 7, 1);
  *(uint16_t*)0x20000084 = htobe16(0);
  *(uint16_t*)0x20000088 = 9;
  *(uint16_t*)0x2000008a = 2;
  memcpy((void*)0x2000008c, "syz0\000", 5);
  *(uint16_t*)0x20000094 = 5;
  *(uint16_t*)0x20000096 = 5;
  *(uint8_t*)0x20000098 = 0;
  *(uint32_t*)0x20000104 = 0x5c;
  *(uint32_t*)0x2000014c = 1;
  *(uint32_t*)0x20000150 = 0;
  *(uint32_t*)0x20000154 = 0;
  *(uint32_t*)0x20000158 = 0;
  syscall(__NR_sendmsg, (intptr_t)r[0], 0x20000140, 0);
  res = syscall(__NR_socket, 0x10, 3, 0xc);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x20000540 = 0;
  *(uint32_t*)0x20000544 = 0;
  *(uint32_t*)0x20000548 = 0x20000500;
  *(uint32_t*)0x20000500 = 0x20000480;
  *(uint32_t*)0x20000480 = 0x24;
  *(uint8_t*)0x20000484 = 7;
  *(uint8_t*)0x20000485 = 6;
  *(uint16_t*)0x20000486 = 5;
  *(uint32_t*)0x20000488 = 0;
  *(uint32_t*)0x2000048c = 0;
  *(uint8_t*)0x20000490 = 0;
  *(uint8_t*)0x20000491 = 0;
  *(uint16_t*)0x20000492 = htobe16(0);
  *(uint16_t*)0x20000494 = 8;
  STORE_BY_BITMASK(uint16_t, , 0x20000496, 6, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000497, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000497, 0, 7, 1);
  *(uint32_t*)0x20000498 = htobe32(0);
  *(uint16_t*)0x2000049c = 5;
  *(uint16_t*)0x2000049e = 1;
  *(uint8_t*)0x200004a0 = 7;
  *(uint32_t*)0x20000504 = 0x24;
  *(uint32_t*)0x2000054c = 1;
  *(uint32_t*)0x20000550 = 0;
  *(uint32_t*)0x20000554 = 0;
  *(uint32_t*)0x20000558 = 0;
  syscall(__NR_sendmsg, (intptr_t)r[1], 0x20000540, 0);
  return 0;
}
