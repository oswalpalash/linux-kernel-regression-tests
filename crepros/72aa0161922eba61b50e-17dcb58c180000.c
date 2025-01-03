// https://syzkaller.appspot.com/bug?id=3a4e1d02176ad842ee862cb63e9172801374f9b2
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
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  intptr_t res = 0;
  *(uint32_t*)0x200000c0 = 3;
  *(uint32_t*)0x200000c4 = 0x13;
  *(uint64_t*)0x200000c8 = 0x20000000;
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
  STORE_BY_BITMASK(uint8_t, , 0x20000011, 0, 4, 4);
  *(uint16_t*)0x20000012 = 0;
  *(uint32_t*)0x20000014 = 0x25702020;
  *(uint8_t*)0x20000018 = 0;
  *(uint8_t*)0x20000019 = 0;
  *(uint16_t*)0x2000001a = 0;
  *(uint32_t*)0x2000001c = 0x20202000;
  STORE_BY_BITMASK(uint8_t, , 0x20000020, 3, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000020, 3, 3, 2);
  STORE_BY_BITMASK(uint8_t, , 0x20000020, 3, 5, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000021, 0xa, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000021, 1, 4, 4);
  *(uint16_t*)0x20000022 = 0xfff8;
  *(uint32_t*)0x20000024 = 0;
  STORE_BY_BITMASK(uint8_t, , 0x20000028, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000028, 1, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000028, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000029, 1, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000029, 0xa, 4, 4);
  *(uint16_t*)0x2000002a = 0;
  *(uint32_t*)0x2000002c = 0;
  STORE_BY_BITMASK(uint8_t, , 0x20000030, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000030, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000030, 0, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000031, 1, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000031, 0, 4, 4);
  *(uint16_t*)0x20000032 = 0;
  *(uint32_t*)0x20000034 = 0xfffffff8;
  STORE_BY_BITMASK(uint8_t, , 0x20000038, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000038, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000038, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000039, 2, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000039, 0, 4, 4);
  *(uint16_t*)0x2000003a = 0;
  *(uint32_t*)0x2000003c = 8;
  STORE_BY_BITMASK(uint8_t, , 0x20000040, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000040, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000040, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000041, 3, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000041, 0, 4, 4);
  *(uint16_t*)0x20000042 = 0;
  *(uint32_t*)0x20000044 = 0;
  *(uint8_t*)0x20000048 = 0x85;
  *(uint8_t*)0x20000049 = 0;
  *(uint16_t*)0x2000004a = 0;
  *(uint32_t*)0x2000004c = 6;
  *(uint8_t*)0x20000050 = 0x18;
  STORE_BY_BITMASK(uint8_t, , 0x20000051, 1, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000051, 0, 4, 4);
  *(uint16_t*)0x20000052 = 0;
  *(uint32_t*)0x20000054 = 0x256c6c69;
  *(uint8_t*)0x20000058 = 0;
  *(uint8_t*)0x20000059 = 0;
  *(uint16_t*)0x2000005a = 0;
  *(uint32_t*)0x2000005c = 0x20202000;
  STORE_BY_BITMASK(uint8_t, , 0x20000060, 3, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000060, 3, 3, 2);
  STORE_BY_BITMASK(uint8_t, , 0x20000060, 3, 5, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000061, 0xa, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000061, 1, 4, 4);
  *(uint16_t*)0x20000062 = 0xfff8;
  *(uint32_t*)0x20000064 = 0;
  STORE_BY_BITMASK(uint8_t, , 0x20000068, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000068, 1, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000068, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000069, 1, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000069, 0xa, 4, 4);
  *(uint16_t*)0x2000006a = 0;
  *(uint32_t*)0x2000006c = 0;
  STORE_BY_BITMASK(uint8_t, , 0x20000070, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000070, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000070, 0, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000071, 1, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000071, 0, 4, 4);
  *(uint16_t*)0x20000072 = 0;
  *(uint32_t*)0x20000074 = 0xfffffff8;
  STORE_BY_BITMASK(uint8_t, , 0x20000078, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000078, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000078, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000079, 2, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000079, 0, 4, 4);
  *(uint16_t*)0x2000007a = 0;
  *(uint32_t*)0x2000007c = 8;
  STORE_BY_BITMASK(uint8_t, , 0x20000080, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000080, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000080, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000081, 3, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000081, 0, 4, 4);
  *(uint16_t*)0x20000082 = 0;
  *(uint32_t*)0x20000084 = 0xff600000;
  *(uint8_t*)0x20000088 = 0x85;
  *(uint8_t*)0x20000089 = 0;
  *(uint16_t*)0x2000008a = 0;
  *(uint32_t*)0x2000008c = 0x71;
  *(uint8_t*)0x20000090 = 0x95;
  *(uint8_t*)0x20000091 = 0;
  *(uint16_t*)0x20000092 = 0;
  *(uint32_t*)0x20000094 = 0;
  *(uint64_t*)0x200000d0 = 0x20000200;
  memcpy((void*)0x20000200, "GPL\000", 4);
  *(uint32_t*)0x200000d8 = 0;
  *(uint32_t*)0x200000dc = 0;
  *(uint64_t*)0x200000e0 = 0;
  *(uint32_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000ec = 0;
  memset((void*)0x200000f0, 0, 16);
  *(uint32_t*)0x20000100 = 0;
  *(uint32_t*)0x20000104 = 0;
  *(uint32_t*)0x20000108 = 0;
  *(uint32_t*)0x2000010c = 0;
  *(uint64_t*)0x20000110 = 0;
  *(uint32_t*)0x20000118 = 0;
  *(uint32_t*)0x2000011c = 0;
  *(uint64_t*)0x20000120 = 0;
  *(uint32_t*)0x20000128 = 0;
  *(uint32_t*)0x2000012c = 0;
  *(uint32_t*)0x20000130 = 0;
  *(uint32_t*)0x20000134 = 0x43;
  *(uint64_t*)0x20000138 = 0;
  *(uint64_t*)0x20000140 = 0;
  *(uint32_t*)0x20000148 = 0;
  *(uint32_t*)0x2000014c = 0;
  res = syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x200000c0ul, /*size=*/0x90ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000240 = r[0];
  *(uint32_t*)0x20000244 = 0;
  *(uint32_t*)0x20000248 = 0x10;
  *(uint32_t*)0x2000024c = 0x10;
  *(uint64_t*)0x20000250 = 0x200002c0;
  memcpy((void*)0x200002c0, "\x00\x00\xff\xff\xff\xff\xa0\x00", 8);
  *(uint64_t*)0x20000258 = 0x20000300;
  *(uint32_t*)0x20000260 = 0;
  *(uint32_t*)0x20000264 = 0;
  *(uint32_t*)0x20000268 = 0;
  *(uint32_t*)0x2000026c = 0;
  *(uint64_t*)0x20000270 = 0;
  *(uint64_t*)0x20000278 = 0;
  *(uint32_t*)0x20000280 = 0;
  *(uint32_t*)0x20000284 = 0;
  *(uint32_t*)0x20000288 = 0;
  syscall(__NR_bpf, /*cmd=*/0xaul, /*arg=*/0x20000240ul, /*size=*/0x4cul);
  return 0;
}
