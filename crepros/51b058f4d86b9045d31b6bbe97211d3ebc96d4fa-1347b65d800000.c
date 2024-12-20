// https://syzkaller.appspot.com/bug?id=51b058f4d86b9045d31b6bbe97211d3ebc96d4fa
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

static void test();

void loop()
{
  while (1) {
    test();
  }
}

void test()
{
  *(uint32_t*)0x20c72000 = 1;
  *(uint32_t*)0x20c72004 = 0x78;
  *(uint8_t*)0x20c72008 = 3;
  *(uint8_t*)0x20c72009 = 0;
  *(uint8_t*)0x20c7200a = 0;
  *(uint8_t*)0x20c7200b = 0;
  *(uint32_t*)0x20c7200c = 0;
  *(uint64_t*)0x20c72010 = 0x300;
  *(uint64_t*)0x20c72018 = 0x3fe;
  *(uint64_t*)0x20c72020 = 0;
  STORE_BY_BITMASK(uint64_t, 0x20c72028, 0, 0, 1);
  STORE_BY_BITMASK(uint64_t, 0x20c72028, 0, 1, 1);
  STORE_BY_BITMASK(uint64_t, 0x20c72028, 0, 2, 1);
  STORE_BY_BITMASK(uint64_t, 0x20c72028, 0, 3, 1);
  STORE_BY_BITMASK(uint64_t, 0x20c72028, 0, 4, 1);
  STORE_BY_BITMASK(uint64_t, 0x20c72028, 0, 5, 1);
  STORE_BY_BITMASK(uint64_t, 0x20c72028, 0, 6, 1);
  STORE_BY_BITMASK(uint64_t, 0x20c72028, 0, 7, 1);
  STORE_BY_BITMASK(uint64_t, 0x20c72028, 0, 8, 1);
  STORE_BY_BITMASK(uint64_t, 0x20c72028, 0, 9, 1);
  STORE_BY_BITMASK(uint64_t, 0x20c72028, 0, 10, 1);
  STORE_BY_BITMASK(uint64_t, 0x20c72028, 0, 11, 1);
  STORE_BY_BITMASK(uint64_t, 0x20c72028, 0, 12, 1);
  STORE_BY_BITMASK(uint64_t, 0x20c72028, 0, 13, 1);
  STORE_BY_BITMASK(uint64_t, 0x20c72028, 0, 14, 1);
  STORE_BY_BITMASK(uint64_t, 0x20c72028, 0, 15, 2);
  STORE_BY_BITMASK(uint64_t, 0x20c72028, 0, 17, 1);
  STORE_BY_BITMASK(uint64_t, 0x20c72028, 0, 18, 1);
  STORE_BY_BITMASK(uint64_t, 0x20c72028, 0, 19, 1);
  STORE_BY_BITMASK(uint64_t, 0x20c72028, 0, 20, 1);
  STORE_BY_BITMASK(uint64_t, 0x20c72028, 0, 21, 1);
  STORE_BY_BITMASK(uint64_t, 0x20c72028, 0, 22, 1);
  STORE_BY_BITMASK(uint64_t, 0x20c72028, 0, 23, 1);
  STORE_BY_BITMASK(uint64_t, 0x20c72028, 0, 24, 1);
  STORE_BY_BITMASK(uint64_t, 0x20c72028, 0, 25, 1);
  STORE_BY_BITMASK(uint64_t, 0x20c72028, 0, 26, 1);
  STORE_BY_BITMASK(uint64_t, 0x20c72028, 0, 27, 1);
  STORE_BY_BITMASK(uint64_t, 0x20c72028, 0, 28, 1);
  STORE_BY_BITMASK(uint64_t, 0x20c72028, 0, 29, 35);
  *(uint32_t*)0x20c72030 = 0;
  *(uint32_t*)0x20c72034 = 0;
  *(uint64_t*)0x20c72038 = 0x20000000;
  *(uint64_t*)0x20c72040 = 0;
  *(uint64_t*)0x20c72048 = 0;
  *(uint64_t*)0x20c72050 = 0;
  *(uint64_t*)0x20c72058 = 0;
  *(uint32_t*)0x20c72060 = 0;
  *(uint64_t*)0x20c72068 = 0;
  *(uint32_t*)0x20c72070 = 0;
  *(uint16_t*)0x20c72074 = 0;
  *(uint16_t*)0x20c72076 = 0;
  syscall(__NR_perf_event_open, 0x20c72000, 0, 0, -1, 0);
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
  syscall(__NR_perf_event_open, 0x2025c000, 0, 0, -1, 0);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    loop();
  }
}
