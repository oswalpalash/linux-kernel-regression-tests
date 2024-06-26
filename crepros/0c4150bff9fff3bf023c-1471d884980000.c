// https://syzkaller.appspot.com/bug?id=2773ef3a95ba3bd938ca3f7e27cc2ad8d59ad812
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
  *(uint32_t*)0x20002c80 = 3;
  *(uint32_t*)0x20002c84 = 0xc;
  *(uint64_t*)0x20002c88 = 0x20000000;
  *(uint8_t*)0x20000000 = 0x18;
  STORE_BY_BITMASK(uint8_t, , 0x20000001, 2, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000001, 0, 4, 4);
  *(uint16_t*)0x20000002 = 0;
  *(uint32_t*)0x20000004 = 0xfffffff7;
  *(uint8_t*)0x20000008 = 0;
  *(uint8_t*)0x20000009 = 0;
  *(uint16_t*)0x2000000a = 0;
  *(uint32_t*)0x2000000c = 0;
  *(uint8_t*)0x20000010 = 0x85;
  *(uint8_t*)0x20000011 = 0;
  *(uint16_t*)0x20000012 = 0;
  *(uint32_t*)0x20000014 = 0x27;
  *(uint8_t*)0x20000018 = 0x18;
  STORE_BY_BITMASK(uint8_t, , 0x20000019, 1, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000019, 0, 4, 4);
  *(uint16_t*)0x2000001a = 0;
  *(uint32_t*)0x2000001c = 0x256c6c78;
  *(uint8_t*)0x20000020 = 0;
  *(uint8_t*)0x20000021 = 0;
  *(uint16_t*)0x20000022 = 0;
  *(uint32_t*)0x20000024 = 0x20202000;
  STORE_BY_BITMASK(uint8_t, , 0x20000028, 3, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000028, 3, 3, 2);
  STORE_BY_BITMASK(uint8_t, , 0x20000028, 3, 5, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000029, 0xa, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000029, 1, 4, 4);
  *(uint16_t*)0x2000002a = 0xfff8;
  *(uint32_t*)0x2000002c = 0;
  STORE_BY_BITMASK(uint8_t, , 0x20000030, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000030, 1, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000030, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000031, 1, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000031, 0xa, 4, 4);
  *(uint16_t*)0x20000032 = 0;
  *(uint32_t*)0x20000034 = 0;
  STORE_BY_BITMASK(uint8_t, , 0x20000038, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000038, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000038, 0, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000039, 1, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000039, 0, 4, 4);
  *(uint16_t*)0x2000003a = 0;
  *(uint32_t*)0x2000003c = 0xfffffff8;
  STORE_BY_BITMASK(uint8_t, , 0x20000040, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000040, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000040, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000041, 2, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000041, 0, 4, 4);
  *(uint16_t*)0x20000042 = 0;
  *(uint32_t*)0x20000044 = 8;
  STORE_BY_BITMASK(uint8_t, , 0x20000048, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000048, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000048, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000049, 3, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000049, 0, 4, 4);
  *(uint16_t*)0x2000004a = 0;
  *(uint32_t*)0x2000004c = 0;
  *(uint8_t*)0x20000050 = 0x85;
  *(uint8_t*)0x20000051 = 0;
  *(uint16_t*)0x20000052 = 0;
  *(uint32_t*)0x20000054 = 6;
  *(uint8_t*)0x20000058 = 0x95;
  *(uint8_t*)0x20000059 = 0;
  *(uint16_t*)0x2000005a = 0;
  *(uint32_t*)0x2000005c = 0;
  *(uint64_t*)0x20002c90 = 0x20000080;
  memcpy((void*)0x20000080, "GPL\000", 4);
  *(uint32_t*)0x20002c98 = 0;
  *(uint32_t*)0x20002c9c = 0;
  *(uint64_t*)0x20002ca0 = 0;
  *(uint32_t*)0x20002ca8 = 0;
  *(uint32_t*)0x20002cac = 0;
  memset((void*)0x20002cb0, 0, 16);
  *(uint32_t*)0x20002cc0 = 0;
  *(uint32_t*)0x20002cc4 = 0;
  *(uint32_t*)0x20002cc8 = -1;
  *(uint32_t*)0x20002ccc = 8;
  *(uint64_t*)0x20002cd0 = 0;
  *(uint32_t*)0x20002cd8 = 0;
  *(uint32_t*)0x20002cdc = 0x10;
  *(uint64_t*)0x20002ce0 = 0;
  *(uint32_t*)0x20002ce8 = 0;
  *(uint32_t*)0x20002cec = 0;
  *(uint32_t*)0x20002cf0 = 0;
  *(uint32_t*)0x20002cf4 = 0;
  *(uint64_t*)0x20002cf8 = 0;
  *(uint64_t*)0x20002d00 = 0;
  *(uint32_t*)0x20002d08 = 0x10;
  *(uint32_t*)0x20002d0c = 0;
  res = syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x20002c80ul, /*size=*/0x90ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x200002c0 = r[0];
  *(uint32_t*)0x200002c4 = 0;
  *(uint32_t*)0x200002c8 = 0xe;
  *(uint32_t*)0x200002cc = 0;
  *(uint64_t*)0x200002d0 = 0x20000100;
  memcpy((void*)0x20000100,
         "\xe0\xb9\x54\x7e\xd3\x87\xdb\xe9\xab\xc8\x9b\x6f\x5b\xec", 14);
  *(uint64_t*)0x200002d8 = 0;
  *(uint32_t*)0x200002e0 = 0;
  *(uint32_t*)0x200002e4 = 0;
  *(uint32_t*)0x200002e8 = 0;
  *(uint32_t*)0x200002ec = 0;
  *(uint64_t*)0x200002f0 = 0;
  *(uint64_t*)0x200002f8 = 0;
  *(uint32_t*)0x20000300 = 0;
  *(uint32_t*)0x20000304 = 0;
  *(uint32_t*)0x20000308 = 0;
  syscall(__NR_bpf, /*cmd=*/0xaul, /*arg=*/0x200002c0ul, /*size=*/0x50ul);
  return 0;
}
