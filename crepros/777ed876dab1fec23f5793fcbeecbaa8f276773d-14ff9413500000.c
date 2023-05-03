// https://syzkaller.appspot.com/bug?id=777ed876dab1fec23f5793fcbeecbaa8f276773d
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

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);

  memcpy((void*)0x20000080, "./file0\000", 8);
  syscall(__NR_mknod, 0x20000080ul, 0ul, 0);
  memcpy((void*)0x200000c0,
         "\x3f\x00\xfd\x00\xe0\xbb\xb5\x57\xf3\xb0\xb9\x37\x42\xc3\x79\x98\x3f"
         "\xe4\x8e\x87\x39\xad\x15\xbe\x14\x4a\x70\xa0\x53\xdf\xcd\xeb\x98\x7f"
         "\xbd\x3c\x23\x6b\xfa\x1c\x25\x1b\x4a\x45\xbf\xe3\x0f\x44\x4d\xc5\xe3"
         "\x14\x65\x8f\x7c\x94\x5f\xa4\xad\xbb\x2c\x92\x49\xdd\x44\xbe\x32\xdc"
         "\x79\xb3\x98\x12\xdd\xa8\x76\x84\x0f\x3f\x97\x1d\xd7\x5e\x67\x26\x24"
         "\xfe\x15\x84\xa5\x3a",
         90);
  syscall(__NR_getsockopt, 0xffffff9c, 0x84, 0x6d, 0x200000c0ul, 0ul);
  *(uint32_t*)0x2000a000 = 3;
  *(uint32_t*)0x2000a004 = 0x70;
  *(uint8_t*)0x2000a008 = 0;
  *(uint8_t*)0x2000a009 = 0x40;
  *(uint8_t*)0x2000a00a = 0xfd;
  *(uint8_t*)0x2000a00b = 0x27;
  *(uint32_t*)0x2000a00c = 0;
  *(uint64_t*)0x2000a010 = -1;
  *(uint64_t*)0x2000a018 = 0;
  *(uint64_t*)0x2000a020 = 6;
  STORE_BY_BITMASK(uint64_t, , 0x2000a028, 0, 0, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2000a028, 0, 1, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2000a028, 0, 2, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2000a028, 0, 3, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2000a028, 0, 4, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2000a028, 0, 5, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2000a028, 0, 6, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2000a028, 0, 7, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2000a028, 0, 8, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2000a028, 0, 9, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2000a028, 0, 10, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2000a028, 0, 11, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2000a028, 0, 12, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2000a028, 0, 13, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2000a028, 0, 14, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2000a028, 0, 15, 2);
  STORE_BY_BITMASK(uint64_t, , 0x2000a028, 0, 17, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2000a028, 0, 18, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2000a028, 0, 19, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2000a028, 0, 20, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2000a028, 0, 21, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2000a028, 0, 22, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2000a028, 0, 23, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2000a028, 0, 24, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2000a028, 0, 25, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2000a028, 0, 26, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2000a028, 0, 27, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2000a028, 0, 28, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2000a028, 0, 29, 35);
  *(uint32_t*)0x2000a030 = 0x48f;
  *(uint32_t*)0x2000a034 = 1;
  *(uint64_t*)0x2000a038 = 0x20000140;
  *(uint64_t*)0x2000a040 = 3;
  *(uint64_t*)0x2000a048 = 0x8000000200052203;
  *(uint64_t*)0x2000a050 = 0x40000000800007f;
  *(uint32_t*)0x2000a058 = 0;
  *(uint32_t*)0x2000a05c = 7;
  *(uint64_t*)0x2000a060 = 0;
  *(uint32_t*)0x2000a068 = 0;
  *(uint16_t*)0x2000a06c = 0;
  *(uint16_t*)0x2000a06e = 0;
  syscall(__NR_perf_event_open, 0x2000a000ul, 0, 4ul, -1, 0ul);
  memcpy((void*)0x20343ff8, "./file0\000", 8);
  memcpy((void*)0x20fb5ffc, "nfs\000", 4);
  syscall(__NR_mount, 0x20000100ul, 0x20343ff8ul, 0x20fb5ffcul, 0ul,
          0x2000a000ul);
  return 0;
}
