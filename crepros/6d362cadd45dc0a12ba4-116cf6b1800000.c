// https://syzkaller.appspot.com/bug?id=ed26a1394fb5b2d8776ff3f66d198cca9e0d7919
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

void loop()
{
  syscall(__NR_mmap, 0x20000000, 0xd000, 3, 0x32, -1, 0);
  *(uint64_t*)0x20002000 = 0;
  *(uint64_t*)0x20002008 = 0;
  *(uint64_t*)0x20002010 = 0;
  *(uint64_t*)0x20002018 = 0x11f;
  syscall(__NR_ioctl, -1, 0x40206435, 0x20002000);
  *(uint32_t*)0x2000a000 = 1;
  *(uint32_t*)0x2000a004 = 5;
  *(uint64_t*)0x2000a008 = 0x20002000;
  *(uint64_t*)0x2000a010 = 0x20003ff6;
  *(uint32_t*)0x2000a018 = 1;
  *(uint32_t*)0x2000a01c = 0xc3;
  *(uint64_t*)0x2000a020 = 0x20386000;
  *(uint32_t*)0x2000a028 = 0;
  *(uint32_t*)0x2000a02c = 0;
  *(uint8_t*)0x2000a030 = 0;
  *(uint8_t*)0x2000a031 = 0;
  *(uint8_t*)0x2000a032 = 0;
  *(uint8_t*)0x2000a033 = 0;
  *(uint8_t*)0x2000a034 = 0;
  *(uint8_t*)0x2000a035 = 0;
  *(uint8_t*)0x2000a036 = 0;
  *(uint8_t*)0x2000a037 = 0;
  *(uint8_t*)0x2000a038 = 0;
  *(uint8_t*)0x2000a039 = 0;
  *(uint8_t*)0x2000a03a = 0;
  *(uint8_t*)0x2000a03b = 0;
  *(uint8_t*)0x2000a03c = 0;
  *(uint8_t*)0x2000a03d = 0;
  *(uint8_t*)0x2000a03e = 0;
  *(uint8_t*)0x2000a03f = 0;
  *(uint32_t*)0x2000a040 = 0;
  *(uint8_t*)0x20002000 = 0x18;
  STORE_BY_BITMASK(uint8_t, 0x20002001, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x20002001, 0, 4, 4);
  *(uint16_t*)0x20002002 = 0;
  *(uint32_t*)0x20002004 = 0xf3af;
  *(uint8_t*)0x20002008 = 0;
  *(uint8_t*)0x20002009 = 0;
  *(uint16_t*)0x2000200a = 0;
  *(uint32_t*)0x2000200c = 0;
  STORE_BY_BITMASK(uint8_t, 0x20002010, 5, 0, 3);
  STORE_BY_BITMASK(uint8_t, 0x20002010, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, 0x20002010, 3, 4, 4);
  STORE_BY_BITMASK(uint8_t, 0x20002011, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x20002011, 0, 4, 4);
  *(uint16_t*)0x20002012 = 0;
  *(uint32_t*)0x20002014 = 2;
  *(uint8_t*)0x20002020 = 0x95;
  *(uint8_t*)0x20002021 = 0;
  *(uint16_t*)0x20002022 = 0;
  *(uint32_t*)0x20002024 = 0;
  memcpy((void*)0x20003ff6, "GPL", 4);
  syscall(__NR_bpf, 5, 0x2000a000, 0x48);
}

int main()
{
  loop();
  return 0;
}
