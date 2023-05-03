// https://syzkaller.appspot.com/bug?id=5c8b4d8ea14d505d11874f51a1943b2869733719
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

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);

  *(uint32_t*)0x20000a00 = 0x17;
  *(uint32_t*)0x20000a04 = 4;
  *(uint64_t*)0x20000a08 = 0x20000140;
  *(uint8_t*)0x20000140 = 0x18;
  STORE_BY_BITMASK(uint8_t, , 0x20000141, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000141, 0, 4, 4);
  *(uint16_t*)0x20000142 = 0;
  *(uint32_t*)0x20000144 = 0;
  *(uint8_t*)0x20000148 = 0;
  *(uint8_t*)0x20000149 = 0;
  *(uint16_t*)0x2000014a = 0;
  *(uint32_t*)0x2000014c = 0;
  *(uint8_t*)0x20000150 = 0x85;
  *(uint8_t*)0x20000151 = 0;
  *(uint16_t*)0x20000152 = 0;
  *(uint32_t*)0x20000154 = 0xbb;
  *(uint8_t*)0x20000158 = 0x95;
  *(uint8_t*)0x20000159 = 0;
  *(uint16_t*)0x2000015a = 0;
  *(uint32_t*)0x2000015c = 0;
  *(uint64_t*)0x20000a10 = 0x20000180;
  memcpy((void*)0x20000180, "GPL\000", 4);
  *(uint32_t*)0x20000a18 = 4;
  *(uint32_t*)0x20000a1c = 0xee;
  *(uint64_t*)0x20000a20 = 0x200007c0;
  *(uint32_t*)0x20000a28 = 0;
  *(uint32_t*)0x20000a2c = 0;
  memset((void*)0x20000a30, 0, 16);
  *(uint32_t*)0x20000a40 = 0;
  *(uint32_t*)0x20000a44 = 0x2b;
  *(uint32_t*)0x20000a48 = -1;
  *(uint32_t*)0x20000a4c = 8;
  *(uint64_t*)0x20000a50 = 0;
  *(uint32_t*)0x20000a58 = 0;
  *(uint32_t*)0x20000a5c = 0x10;
  *(uint64_t*)0x20000a60 = 0;
  *(uint32_t*)0x20000a68 = 0;
  *(uint32_t*)0x20000a6c = 0;
  *(uint32_t*)0x20000a70 = 0;
  *(uint32_t*)0x20000a74 = 0;
  *(uint64_t*)0x20000a78 = 0;
  syscall(__NR_bpf, 5ul, 0x20000a00ul, 0x80ul);
  return 0;
}
