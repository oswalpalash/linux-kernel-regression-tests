// https://syzkaller.appspot.com/bug?id=e84ebd53b11608595ca4fd595d48ae9ddce9cf11
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

uint64_t r[1] = {0xffffffffffffffff};
void loop()
{
  long res = 0;
  *(uint32_t*)0x2001d000 = 2;
  *(uint32_t*)0x2001d004 = 0x78;
  *(uint8_t*)0x2001d008 = 0;
  *(uint8_t*)0x2001d009 = 1;
  *(uint8_t*)0x2001d00a = 0;
  *(uint8_t*)0x2001d00b = 0;
  *(uint32_t*)0x2001d00c = 0;
  *(uint64_t*)0x2001d010 = 0;
  *(uint64_t*)0x2001d018 = 0;
  *(uint64_t*)0x2001d020 = 0;
  STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 0, 1);
  STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 1, 1);
  STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 2, 1);
  STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 3, 1);
  STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 4, 1);
  STORE_BY_BITMASK(uint64_t, 0x2001d028, -1, 5, 1);
  STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 6, 1);
  STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 7, 1);
  STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 8, 1);
  STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 9, 1);
  STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 10, 1);
  STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 11, 1);
  STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 12, 1);
  STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 13, 1);
  STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 14, 1);
  STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 15, 2);
  STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 17, 1);
  STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 18, 1);
  STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 19, 1);
  STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 20, 1);
  STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 21, 1);
  STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 22, 1);
  STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 23, 1);
  STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 24, 1);
  STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 25, 1);
  STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 26, 1);
  STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 27, 1);
  STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 28, 1);
  STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 29, 35);
  *(uint32_t*)0x2001d030 = 0;
  *(uint32_t*)0x2001d034 = 0;
  *(uint64_t*)0x2001d038 = 0x20000000;
  *(uint64_t*)0x2001d040 = 0;
  *(uint64_t*)0x2001d048 = 0;
  *(uint64_t*)0x2001d050 = 0;
  *(uint32_t*)0x2001d058 = 0;
  *(uint32_t*)0x2001d05c = 0;
  *(uint64_t*)0x2001d060 = 0;
  *(uint32_t*)0x2001d068 = 0;
  *(uint16_t*)0x2001d06c = 0;
  *(uint16_t*)0x2001d06e = 0;
  res = syscall(__NR_perf_event_open, 0x2001d000, 0, 0, -1, 0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x200000c0, "\x21\x00\x00\x00\x00\x00\x10\x00\x00\x74\x30\x23"
                            "\x6d\x64\x35\x73\x75\x6d\x77\x6c\x61\x6e\x30\x27"
                            "\x00",
         25);
  syscall(__NR_ioctl, r[0], 0x40082406, 0x200000c0);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
