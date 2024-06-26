// https://syzkaller.appspot.com/bug?id=449f5619489cae8df8f71d2c6bf3675f4af9d6ce
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

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0xcul);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20003e00 = 0;
  *(uint32_t*)0x20003e08 = 0;
  *(uint64_t*)0x20003e10 = 0x20003dc0;
  *(uint64_t*)0x20003dc0 = 0x20000100;
  *(uint32_t*)0x20000100 = 0x14;
  *(uint16_t*)0x20000104 = 0x10;
  *(uint16_t*)0x20000106 = 1;
  *(uint32_t*)0x20000108 = 0;
  *(uint32_t*)0x2000010c = 0;
  *(uint8_t*)0x20000110 = 0;
  *(uint8_t*)0x20000111 = 0;
  *(uint16_t*)0x20000112 = htobe16(0xa);
  *(uint32_t*)0x20000114 = 0x20;
  *(uint8_t*)0x20000118 = 0;
  *(uint8_t*)0x20000119 = 0xa;
  *(uint16_t*)0x2000011a = 0x1405;
  *(uint32_t*)0x2000011c = 0;
  *(uint32_t*)0x20000120 = 0;
  *(uint8_t*)0x20000124 = 1;
  *(uint8_t*)0x20000125 = 0;
  *(uint16_t*)0x20000126 = htobe16(0);
  *(uint16_t*)0x20000128 = 9;
  *(uint16_t*)0x2000012a = 1;
  memcpy((void*)0x2000012c, "syz0\000", 5);
  *(uint32_t*)0x20000134 = 0x58;
  *(uint8_t*)0x20000138 = 0x16;
  *(uint8_t*)0x20000139 = 0xa;
  *(uint16_t*)0x2000013a = 1;
  *(uint32_t*)0x2000013c = 0;
  *(uint32_t*)0x20000140 = 0;
  *(uint8_t*)0x20000144 = 1;
  *(uint8_t*)0x20000145 = 0;
  *(uint16_t*)0x20000146 = htobe16(0);
  *(uint16_t*)0x20000148 = 9;
  *(uint16_t*)0x2000014a = 1;
  memcpy((void*)0x2000014c, "syz0\000", 5);
  *(uint16_t*)0x20000154 = 9;
  *(uint16_t*)0x20000156 = 2;
  memcpy((void*)0x20000158, "syz2\000", 5);
  *(uint16_t*)0x20000160 = 0x2c;
  STORE_BY_BITMASK(uint16_t, , 0x20000162, 3, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000163, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000163, 1, 7, 1);
  *(uint16_t*)0x20000164 = 0x18;
  STORE_BY_BITMASK(uint16_t, , 0x20000166, 3, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000167, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000167, 1, 7, 1);
  *(uint16_t*)0x20000168 = 0x14;
  *(uint16_t*)0x2000016a = 1;
  memcpy((void*)0x2000016c,
         "sit0\000\000\000\000\000\000\000\000\000\000\000\000", 16);
  *(uint16_t*)0x2000017c = 8;
  STORE_BY_BITMASK(uint16_t, , 0x2000017e, 2, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000017f, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000017f, 0, 7, 1);
  *(uint32_t*)0x20000180 = htobe32(0);
  *(uint16_t*)0x20000184 = 8;
  STORE_BY_BITMASK(uint16_t, , 0x20000186, 1, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000187, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000187, 0, 7, 1);
  *(uint32_t*)0x20000188 = htobe32(0);
  *(uint32_t*)0x2000018c = 0x14;
  *(uint8_t*)0x20000190 = 2;
  *(uint8_t*)0x20000191 = 0xa;
  *(uint16_t*)0x20000192 = 3;
  *(uint32_t*)0x20000194 = 0;
  *(uint32_t*)0x20000198 = 0;
  *(uint8_t*)0x2000019c = 0;
  *(uint8_t*)0x2000019d = 0;
  *(uint16_t*)0x2000019e = htobe16(0);
  *(uint32_t*)0x200001a0 = 0x14;
  *(uint16_t*)0x200001a4 = 0x11;
  *(uint16_t*)0x200001a6 = 1;
  *(uint32_t*)0x200001a8 = 0;
  *(uint32_t*)0x200001ac = 0;
  *(uint8_t*)0x200001b0 = 0;
  *(uint8_t*)0x200001b1 = 0;
  *(uint16_t*)0x200001b2 = htobe16(0xa);
  *(uint64_t*)0x20003dc8 = 0xb4;
  *(uint64_t*)0x20003e18 = 1;
  *(uint64_t*)0x20003e20 = 0;
  *(uint64_t*)0x20003e28 = 0;
  *(uint32_t*)0x20003e30 = 0;
  syscall(__NR_sendmsg, r[0], 0x20003e00ul, 0ul);
  return 0;
}
