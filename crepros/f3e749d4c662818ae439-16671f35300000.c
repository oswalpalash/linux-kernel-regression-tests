// https://syzkaller.appspot.com/bug?id=4c9ab8c7d0f8b551950db06559dc9cde4119ac83
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

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  *(uint64_t*)0x200001c0 = 0x20000140;
  *(uint16_t*)0x20000140 = 0xeb9f;
  *(uint8_t*)0x20000142 = 1;
  *(uint8_t*)0x20000143 = 0;
  *(uint32_t*)0x20000144 = 0x18;
  *(uint32_t*)0x20000148 = 0;
  *(uint32_t*)0x2000014c = 0x10;
  *(uint32_t*)0x20000150 = 0x10;
  *(uint32_t*)0x20000154 = 2;
  *(uint32_t*)0x20000158 = 0;
  *(uint16_t*)0x2000015c = 0;
  *(uint8_t*)0x2000015e = 0;
  *(uint8_t*)0x2000015f = 1;
  *(uint32_t*)0x20000160 = 0x6000000;
  *(uint8_t*)0x20000164 = 0;
  *(uint8_t*)0x20000165 = 0;
  *(uint8_t*)0x20000166 = 0;
  *(uint8_t*)0x20000167 = 0;
  *(uint8_t*)0x20000168 = 0;
  *(uint8_t*)0x20000169 = 0;
  *(uint64_t*)0x200001c8 = 0x20000800;
  *(uint32_t*)0x200001d0 = 0x2a;
  *(uint32_t*)0x200001d4 = 0xf4;
  *(uint32_t*)0x200001d8 = 1;
  res = syscall(__NR_bpf, 0x12ul, 0x200001c0ul, 0x3feul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000900 = 0x11;
  *(uint32_t*)0x20000904 = 3;
  *(uint64_t*)0x20000908 = 0x20000540;
  *(uint8_t*)0x20000540 = 0x18;
  STORE_BY_BITMASK(uint8_t, , 0x20000541, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000541, 0, 4, 4);
  *(uint16_t*)0x20000542 = 0;
  *(uint32_t*)0x20000544 = 0;
  *(uint8_t*)0x20000548 = 0;
  *(uint8_t*)0x20000549 = 0;
  *(uint16_t*)0x2000054a = 0;
  *(uint32_t*)0x2000054c = 0;
  *(uint8_t*)0x20000550 = 0x95;
  *(uint8_t*)0x20000551 = 0;
  *(uint16_t*)0x20000552 = 0;
  *(uint32_t*)0x20000554 = 0;
  *(uint64_t*)0x20000910 = 0x200002c0;
  memcpy((void*)0x200002c0, "syzkaller\000", 10);
  *(uint32_t*)0x20000918 = 7;
  *(uint32_t*)0x2000091c = 0xab;
  *(uint64_t*)0x20000920 = 0x200005c0;
  *(uint32_t*)0x20000928 = 0;
  *(uint32_t*)0x2000092c = 0;
  memset((void*)0x20000930, 0, 16);
  *(uint32_t*)0x20000940 = 0;
  *(uint32_t*)0x20000944 = 0;
  *(uint32_t*)0x20000948 = r[0];
  *(uint32_t*)0x2000094c = 8;
  *(uint64_t*)0x20000950 = 0;
  *(uint32_t*)0x20000958 = 0;
  *(uint32_t*)0x2000095c = 0x10;
  *(uint64_t*)0x20000960 = 0x200006c0;
  *(uint32_t*)0x200006c0 = 0;
  *(uint32_t*)0x200006c4 = 0;
  *(uint32_t*)0x200006c8 = 0;
  *(uint32_t*)0x200006cc = 0;
  *(uint32_t*)0x20000968 = 0x200006d0;
  *(uint32_t*)0x2000096c = 0;
  *(uint32_t*)0x20000970 = 0;
  syscall(__NR_bpf, 5ul, 0x20000900ul, 0x78ul);
  return 0;
}
