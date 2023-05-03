// https://syzkaller.appspot.com/bug?id=a542ec645924fcf860feb1d7c7fb73f4bf9d05ed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
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

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

long r[2];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  *(uint32_t*)0x20973000 = 5;
  *(uint32_t*)0x20973004 = 3;
  *(uint64_t*)0x20973008 = 0x209ff000;
  *(uint64_t*)0x20973010 = 0x202bf000;
  *(uint32_t*)0x20973018 = 4;
  *(uint32_t*)0x2097301c = 0xb7;
  *(uint64_t*)0x20973020 = 0x206ab000;
  *(uint32_t*)0x20973028 = 0;
  *(uint32_t*)0x2097302c = 0;
  *(uint8_t*)0x20973030 = 0;
  *(uint8_t*)0x20973031 = 0;
  *(uint8_t*)0x20973032 = 0;
  *(uint8_t*)0x20973033 = 0;
  *(uint8_t*)0x20973034 = 0;
  *(uint8_t*)0x20973035 = 0;
  *(uint8_t*)0x20973036 = 0;
  *(uint8_t*)0x20973037 = 0;
  *(uint8_t*)0x20973038 = 0;
  *(uint8_t*)0x20973039 = 0;
  *(uint8_t*)0x2097303a = 0;
  *(uint8_t*)0x2097303b = 0;
  *(uint8_t*)0x2097303c = 0;
  *(uint8_t*)0x2097303d = 0;
  *(uint8_t*)0x2097303e = 0;
  *(uint8_t*)0x2097303f = 0;
  *(uint32_t*)0x20973040 = 0;
  *(uint8_t*)0x209ff000 = 0x18;
  STORE_BY_BITMASK(uint8_t, 0x209ff001, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x209ff001, 0, 4, 4);
  *(uint16_t*)0x209ff002 = 0;
  *(uint32_t*)0x209ff004 = 0;
  *(uint8_t*)0x209ff008 = 0;
  *(uint8_t*)0x209ff009 = 0;
  *(uint16_t*)0x209ff00a = 0;
  *(uint32_t*)0x209ff00c = 0;
  *(uint8_t*)0x209ff010 = 0x95;
  *(uint8_t*)0x209ff011 = 0;
  *(uint16_t*)0x209ff012 = 0;
  *(uint32_t*)0x209ff014 = 0;
  memcpy((void*)0x202bf000, "syzkaller", 10);
  r[0] = syscall(__NR_bpf, 5, 0x20973000, 0x48);
  *(uint32_t*)0x2025c000 = 2;
  *(uint32_t*)0x2025c004 = 0x78;
  *(uint8_t*)0x2025c008 = 0xe2;
  *(uint8_t*)0x2025c009 = 0;
  *(uint8_t*)0x2025c00a = 0;
  *(uint8_t*)0x2025c00b = 0;
  *(uint32_t*)0x2025c00c = 0;
  *(uint64_t*)0x2025c010 = 0;
  *(uint64_t*)0x2025c018 = 0;
  *(uint64_t*)0x2025c020 = 0;
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 0, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 1, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 2, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 3, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 4, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 5, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 6, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 7, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 8, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 9, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 10, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 11, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 12, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 13, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 14, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 15, 2);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 17, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 18, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 19, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 20, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 21, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 22, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 23, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 24, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 25, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 26, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 27, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 28, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 29, 35);
  *(uint32_t*)0x2025c030 = 0;
  *(uint32_t*)0x2025c034 = 0;
  *(uint64_t*)0x2025c038 = 0x20000000;
  *(uint64_t*)0x2025c040 = 0;
  *(uint64_t*)0x2025c048 = 0;
  *(uint64_t*)0x2025c050 = 0;
  *(uint64_t*)0x2025c058 = 0;
  *(uint32_t*)0x2025c060 = 0;
  *(uint64_t*)0x2025c068 = 0;
  *(uint32_t*)0x2025c070 = 0;
  *(uint16_t*)0x2025c074 = 0;
  *(uint16_t*)0x2025c076 = 0;
  r[1] = syscall(__NR_perf_event_open, 0x2025c000, 0, 0, -1, 0);
  syscall(__NR_ioctl, r[1], 0x40042408, r[0]);
  *(uint32_t*)0x20a40000 = 7;
  *(uint32_t*)0x20a40004 = 0;
  *(uint32_t*)0x20a40008 = 0;
  *(uint32_t*)0x20a4000c = 0;
  *(uint32_t*)0x20a40010 = 0;
  *(uint32_t*)0x20a40014 = 0;
  *(uint32_t*)0x20a40018 = 0;
  *(uint32_t*)0x20a4001c = 0;
  *(uint32_t*)0x20a40020 = 0;
  syscall(__NR_ioctl, r[1], 0xc008240a, 0x20a40000);
}

int main()
{
  loop();
  return 0;
}
