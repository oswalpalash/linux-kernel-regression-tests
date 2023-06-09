// https://syzkaller.appspot.com/bug?id=48de800ab2740f8a9946ad02e91f04abe311309e
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0xcul);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000280 = 0;
  *(uint32_t*)0x20000288 = 8;
  *(uint64_t*)0x20000290 = 0x20000240;
  *(uint64_t*)0x20000240 = 0x20000100;
  memcpy((void*)0x20000100,
         "\x14\x00\x00\x00\x10\x00\x00\x00\x1e\x6c\x00\x00\x00\x08\x00\x00\x00"
         "\x00\x00\x0a\x20\x00\x00\x00\x00\x0a\x01\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x09\x00\x01\x00\x73\x79\x7a\x30\x00\x00\x00"
         "\x00\x38\x00\x00\x00\x12\x0a\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x08\x04\x00\x04\x80\x09\x00\x02\x00\x00\x39\x7d\x00\x00"
         "\x00\x00\x00\x09\x00\x01\x00\x73\x79\x7a\x30\x00\x00\x00\x00\x08\x00"
         "\x03\x40\x00\x00\x00\x01\x14\x00\x00\x00\x11\x00\xdf\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x0a",
         128);
  *(uint64_t*)0x20000248 = 0x80;
  *(uint64_t*)0x20000298 = 1;
  *(uint64_t*)0x200002a0 = 0;
  *(uint64_t*)0x200002a8 = 0;
  *(uint32_t*)0x200002b0 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000280ul, 0ul);
  res = syscall(__NR_socket, 0x10ul, 3ul, 0xcul);
  if (res != -1)
    r[1] = res;
  *(uint64_t*)0x2000d400 = 0;
  *(uint32_t*)0x2000d408 = 0;
  *(uint64_t*)0x2000d410 = 0x2000d3c0;
  *(uint64_t*)0x2000d3c0 = 0x20009e80;
  *(uint32_t*)0x20009e80 = 0x14;
  *(uint16_t*)0x20009e84 = 0x10;
  *(uint16_t*)0x20009e86 = 1;
  *(uint32_t*)0x20009e88 = 0;
  *(uint32_t*)0x20009e8c = 0;
  *(uint8_t*)0x20009e90 = 0;
  *(uint8_t*)0x20009e91 = 0;
  *(uint16_t*)0x20009e92 = htobe16(0xa);
  *(uint32_t*)0x20009e94 = 0x40;
  *(uint8_t*)0x20009e98 = 3;
  *(uint8_t*)0x20009e99 = 0xa;
  *(uint16_t*)0x20009e9a = 0x401;
  *(uint32_t*)0x20009e9c = 0;
  *(uint32_t*)0x20009ea0 = 0;
  *(uint8_t*)0x20009ea4 = 0;
  *(uint8_t*)0x20009ea5 = 0;
  *(uint16_t*)0x20009ea6 = htobe16(0);
  *(uint16_t*)0x20009ea8 = 9;
  *(uint16_t*)0x20009eaa = 3;
  memcpy((void*)0x20009eac, "syz2\000", 5);
  *(uint16_t*)0x20009eb4 = 9;
  *(uint16_t*)0x20009eb6 = 1;
  memcpy((void*)0x20009eb8, "syz0\000", 5);
  *(uint16_t*)0x20009ec0 = 0x14;
  STORE_BY_BITMASK(uint16_t, , 0x20009ec2, 4, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20009ec3, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20009ec3, 1, 7, 1);
  *(uint16_t*)0x20009ec4 = 8;
  STORE_BY_BITMASK(uint16_t, , 0x20009ec6, 1, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20009ec7, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20009ec7, 0, 7, 1);
  *(uint32_t*)0x20009ec8 = htobe32(0);
  *(uint16_t*)0x20009ecc = 8;
  STORE_BY_BITMASK(uint16_t, , 0x20009ece, 2, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20009ecf, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20009ecf, 0, 7, 1);
  *(uint32_t*)0x20009ed0 = htobe32(0);
  *(uint32_t*)0x20009ed4 = 0x14;
  *(uint16_t*)0x20009ed8 = 0x11;
  *(uint16_t*)0x20009eda = 1;
  *(uint32_t*)0x20009edc = 0;
  *(uint32_t*)0x20009ee0 = 0;
  *(uint8_t*)0x20009ee4 = 0;
  *(uint8_t*)0x20009ee5 = 0;
  *(uint16_t*)0x20009ee6 = htobe16(0xa);
  *(uint64_t*)0x2000d3c8 = 0x68;
  *(uint64_t*)0x2000d418 = 1;
  *(uint64_t*)0x2000d420 = 0;
  *(uint64_t*)0x2000d428 = 0;
  *(uint32_t*)0x2000d430 = 0x4000000;
  syscall(__NR_sendmsg, r[1], 0x2000d400ul, 0x4000000ul);
  return 0;
}
