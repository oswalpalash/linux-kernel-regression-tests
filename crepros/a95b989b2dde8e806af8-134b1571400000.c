// https://syzkaller.appspot.com/bug?id=7f89ee40d35beb6d8f3fc3d122e43287cc3825cd
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

#define BITMASK_LEN(type, bf_len) (type)((1ull << (bf_len)) - 1)

#define BITMASK_LEN_OFF(type, bf_off, bf_len)                                  \
  (type)(BITMASK_LEN(type, (bf_len)) << (bf_off))

#define STORE_BY_BITMASK(type, addr, val, bf_off, bf_len)                      \
  if ((bf_off) == 0 && (bf_len) == 0) {                                        \
    *(type*)(addr) = (type)(val);                                              \
  } else {                                                                     \
    type new_val = *(type*)(addr);                                             \
    new_val &= ~BITMASK_LEN_OFF(type, (bf_off), (bf_len));                     \
    new_val |= ((type)(val)&BITMASK_LEN(type, (bf_len))) << (bf_off);          \
    *(type*)(addr) = new_val;                                                  \
  }

#ifndef __NR_mkdir
#define __NR_mkdir 39
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_mount
#define __NR_mount 21
#endif
#ifndef __NR_perf_event_open
#define __NR_perf_event_open 336
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);

  memcpy((void*)0x20000000, "./file0", 8);
  syscall(__NR_mkdir, 0x20000000, 0);
  *(uint32_t*)0x2000a000 = 5;
  *(uint32_t*)0x2000a004 = 0x70;
  *(uint8_t*)0x2000a008 = 0;
  *(uint8_t*)0x2000a009 = 0;
  *(uint8_t*)0x2000a00a = 0;
  *(uint8_t*)0x2000a00b = 0;
  *(uint32_t*)0x2000a00c = 0;
  *(uint64_t*)0x2000a010 = 0;
  *(uint64_t*)0x2000a018 = 0;
  *(uint64_t*)0x2000a020 = 0;
  STORE_BY_BITMASK(uint64_t, 0x2000a028, 0, 0, 1);
  STORE_BY_BITMASK(uint64_t, 0x2000a028, 0, 1, 1);
  STORE_BY_BITMASK(uint64_t, 0x2000a028, 0, 2, 1);
  STORE_BY_BITMASK(uint64_t, 0x2000a028, 0, 3, 1);
  STORE_BY_BITMASK(uint64_t, 0x2000a028, 0, 4, 1);
  STORE_BY_BITMASK(uint64_t, 0x2000a028, 0, 5, 1);
  STORE_BY_BITMASK(uint64_t, 0x2000a028, 1, 6, 1);
  STORE_BY_BITMASK(uint64_t, 0x2000a028, 0, 7, 1);
  STORE_BY_BITMASK(uint64_t, 0x2000a028, 0, 8, 1);
  STORE_BY_BITMASK(uint64_t, 0x2000a028, 0, 9, 1);
  STORE_BY_BITMASK(uint64_t, 0x2000a028, 0, 10, 1);
  STORE_BY_BITMASK(uint64_t, 0x2000a028, 0, 11, 1);
  STORE_BY_BITMASK(uint64_t, 0x2000a028, 0, 12, 1);
  STORE_BY_BITMASK(uint64_t, 0x2000a028, 0, 13, 1);
  STORE_BY_BITMASK(uint64_t, 0x2000a028, 0, 14, 1);
  STORE_BY_BITMASK(uint64_t, 0x2000a028, 0, 15, 2);
  STORE_BY_BITMASK(uint64_t, 0x2000a028, 0, 17, 1);
  STORE_BY_BITMASK(uint64_t, 0x2000a028, 0, 18, 1);
  STORE_BY_BITMASK(uint64_t, 0x2000a028, 0, 19, 1);
  STORE_BY_BITMASK(uint64_t, 0x2000a028, 0, 20, 1);
  STORE_BY_BITMASK(uint64_t, 0x2000a028, 0, 21, 1);
  STORE_BY_BITMASK(uint64_t, 0x2000a028, 0, 22, 1);
  STORE_BY_BITMASK(uint64_t, 0x2000a028, 0, 23, 1);
  STORE_BY_BITMASK(uint64_t, 0x2000a028, 0, 24, 1);
  STORE_BY_BITMASK(uint64_t, 0x2000a028, 0, 25, 1);
  STORE_BY_BITMASK(uint64_t, 0x2000a028, 0, 26, 1);
  STORE_BY_BITMASK(uint64_t, 0x2000a028, 0, 27, 1);
  STORE_BY_BITMASK(uint64_t, 0x2000a028, 0, 28, 1);
  STORE_BY_BITMASK(uint64_t, 0x2000a028, 0, 29, 35);
  *(uint32_t*)0x2000a030 = 0;
  *(uint32_t*)0x2000a034 = 0;
  *(uint64_t*)0x2000a038 = 0;
  *(uint64_t*)0x2000a040 = 0;
  *(uint64_t*)0x2000a048 = 0xa000003fe;
  *(uint64_t*)0x2000a050 = 0;
  *(uint32_t*)0x2000a058 = 0xfffffffc;
  *(uint32_t*)0x2000a05c = 0;
  *(uint64_t*)0x2000a060 = 0;
  *(uint32_t*)0x2000a068 = 0;
  *(uint16_t*)0x2000a06c = 0;
  *(uint16_t*)0x2000a06e = 0;
  syscall(__NR_perf_event_open, 0x2000a000, 0, 0, -1, 0);
  memcpy((void*)0x20343ff8, "./file0", 8);
  memcpy((void*)0x2015bffc, "nfs", 4);
  syscall(__NR_mount, 0x20000080, 0x20343ff8, 0x2015bffc, 0, 0x2000a000);
  return 0;
}
