// https://syzkaller.appspot.com/bug?id=9c40b805ffd4d30a5e3b150f58fe924a7338ce68
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

uint64_t r[6] = {0xffffffffffffffff, 0xffffffffffffffff, 0x0,
                 0xffffffffffffffff, 0xffffffffffffffff, 0x0};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_socket, 0x10ul, 0x803ul, 0);
  if (res != -1)
    r[1] = res;
  *(uint64_t*)0x20000040 = 0;
  *(uint32_t*)0x20000048 = 0;
  *(uint64_t*)0x20000050 = 0x20000180;
  *(uint64_t*)0x20000180 = 0;
  *(uint64_t*)0x20000188 = 0;
  *(uint64_t*)0x20000058 = 1;
  *(uint64_t*)0x20000060 = 0;
  *(uint64_t*)0x20000068 = 0;
  *(uint32_t*)0x20000070 = 0;
  syscall(__NR_sendmsg, r[1], 0x20000040ul, 0ul);
  *(uint32_t*)0x20000200 = 0x14;
  res = syscall(__NR_getsockname, r[1], 0x20000100ul, 0x20000200ul);
  if (res != -1)
    r[2] = *(uint32_t*)0x20000104;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0);
  if (res != -1)
    r[3] = res;
  res = syscall(__NR_socket, 0x10ul, 0x803ul, 0);
  if (res != -1)
    r[4] = res;
  *(uint64_t*)0x20000280 = 0;
  *(uint32_t*)0x20000288 = 0;
  *(uint64_t*)0x20000290 = 0x20000180;
  *(uint64_t*)0x20000180 = 0;
  *(uint64_t*)0x20000188 = 0;
  *(uint64_t*)0x20000298 = 1;
  *(uint64_t*)0x200002a0 = 0;
  *(uint64_t*)0x200002a8 = 0;
  *(uint32_t*)0x200002b0 = 0;
  syscall(__NR_sendmsg, r[4], 0x20000280ul, 0ul);
  *(uint32_t*)0x20000100 = 0x14;
  res = syscall(__NR_getsockname, r[4], 0x20000580ul, 0x20000100ul);
  if (res != -1)
    r[5] = *(uint32_t*)0x20000584;
  *(uint64_t*)0x20000040 = 0;
  *(uint32_t*)0x20000048 = 0;
  *(uint64_t*)0x20000050 = 0x20000000;
  *(uint64_t*)0x20000000 = 0x20000080;
  memcpy((void*)0x20000080,
         "\x48\x00\x00\x00\x10\x00\x05\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00",
         20);
  *(uint32_t*)0x20000094 = r[5];
  memcpy((void*)0x20000098,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x28\x00\x12\x00\x09\x00\x01\x00\x76"
         "\x65\x74\x68",
         20);
  *(uint64_t*)0x20000008 = 0x48;
  *(uint64_t*)0x20000058 = 1;
  *(uint64_t*)0x20000060 = 0;
  *(uint64_t*)0x20000068 = 0;
  *(uint32_t*)0x20000070 = 0;
  syscall(__NR_sendmsg, r[3], 0x20000040ul, 0ul);
  *(uint64_t*)0x20000000 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint64_t*)0x20000010 = 0x20000180;
  *(uint64_t*)0x20000180 = 0x20000500;
  *(uint32_t*)0x20000500 = 0x40;
  *(uint16_t*)0x20000504 = 0x10;
  *(uint16_t*)0x20000506 = 0xe3b;
  *(uint32_t*)0x20000508 = 0;
  *(uint32_t*)0x2000050c = 0;
  *(uint8_t*)0x20000510 = 0;
  *(uint8_t*)0x20000511 = 0;
  *(uint16_t*)0x20000512 = 0;
  *(uint32_t*)0x20000514 = r[5];
  *(uint32_t*)0x20000518 = 0;
  *(uint32_t*)0x2000051c = 0;
  *(uint16_t*)0x20000520 = 0x20;
  STORE_BY_BITMASK(uint16_t, , 0x20000522, 0x12, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000523, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000523, 1, 7, 1);
  *(uint16_t*)0x20000524 = 0xe;
  *(uint16_t*)0x20000526 = 1;
  memcpy((void*)0x20000528, "ip6gretap\000", 10);
  *(uint16_t*)0x20000534 = 0xc;
  STORE_BY_BITMASK(uint16_t, , 0x20000536, 2, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000537, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000537, 1, 7, 1);
  *(uint16_t*)0x20000538 = 8;
  *(uint16_t*)0x2000053a = 1;
  *(uint32_t*)0x2000053c = r[2];
  *(uint64_t*)0x20000188 = 0x40;
  *(uint64_t*)0x20000018 = 1;
  *(uint64_t*)0x20000020 = 0;
  *(uint64_t*)0x20000028 = 0;
  *(uint32_t*)0x20000030 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000000ul, 0ul);
  return 0;
}
