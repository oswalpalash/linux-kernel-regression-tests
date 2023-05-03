// https://syzkaller.appspot.com/bug?id=9f640e969e36776be02eb136b1f0a75fc6f5ed49
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

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  *(uint32_t*)0x20000100 = 1;
  *(uint32_t*)0x20000104 = 4;
  *(uint32_t*)0x20000108 = 4;
  *(uint32_t*)0x2000010c = 9;
  *(uint32_t*)0x20000110 = 1;
  *(uint32_t*)0x20000114 = 1;
  *(uint32_t*)0x20000118 = 0;
  memset((void*)0x2000011c, 0, 16);
  *(uint32_t*)0x2000012c = 0;
  *(uint32_t*)0x20000130 = -1;
  *(uint32_t*)0x20000134 = 0;
  *(uint32_t*)0x20000138 = 0;
  *(uint32_t*)0x2000013c = 0;
  *(uint64_t*)0x20000140 = 0;
  res = syscall(__NR_bpf, 0ul, 0x20000100ul, 0x48ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000180 = 2;
  *(uint32_t*)0x20000184 = 5;
  *(uint64_t*)0x20000188 = 0x20000000;
  *(uint8_t*)0x20000000 = 0x18;
  STORE_BY_BITMASK(uint8_t, , 0x20000001, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000001, 0, 4, 4);
  *(uint16_t*)0x20000002 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint8_t*)0x20000008 = 0;
  *(uint8_t*)0x20000009 = 0;
  *(uint16_t*)0x2000000a = 0;
  *(uint32_t*)0x2000000c = 0;
  *(uint8_t*)0x20000010 = 0x18;
  STORE_BY_BITMASK(uint8_t, , 0x20000011, 1, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000011, 1, 4, 4);
  *(uint16_t*)0x20000012 = 0;
  *(uint32_t*)0x20000014 = r[0];
  *(uint8_t*)0x20000018 = 0;
  *(uint8_t*)0x20000019 = 0;
  *(uint16_t*)0x2000001a = 0;
  *(uint32_t*)0x2000001c = 0;
  *(uint8_t*)0x20000020 = 0x95;
  *(uint8_t*)0x20000021 = 0;
  *(uint16_t*)0x20000022 = 0;
  *(uint32_t*)0x20000024 = 0;
  *(uint64_t*)0x20000190 = 0x20000080;
  memcpy((void*)0x20000080, "GPL\000", 4);
  *(uint32_t*)0x20000198 = 0;
  *(uint32_t*)0x2000019c = 0;
  *(uint64_t*)0x200001a0 = 0;
  *(uint32_t*)0x200001a8 = 0;
  *(uint32_t*)0x200001ac = 0;
  memset((void*)0x200001b0, 0, 16);
  *(uint32_t*)0x200001c0 = 0;
  *(uint32_t*)0x200001c4 = 0;
  *(uint32_t*)0x200001c8 = -1;
  *(uint32_t*)0x200001cc = 8;
  *(uint64_t*)0x200001d0 = 0;
  *(uint32_t*)0x200001d8 = 0;
  *(uint32_t*)0x200001dc = 0x10;
  *(uint64_t*)0x200001e0 = 0;
  *(uint32_t*)0x200001e8 = 0;
  *(uint32_t*)0x200001ec = 0;
  *(uint32_t*)0x200001f0 = 0;
  *(uint32_t*)0x200001f4 = 0;
  *(uint64_t*)0x200001f8 = 0;
  syscall(__NR_bpf, 5ul, 0x20000180ul, 0x80ul);
  return 0;
}
