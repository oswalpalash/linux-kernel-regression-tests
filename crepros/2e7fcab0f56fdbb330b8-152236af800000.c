// https://syzkaller.appspot.com/bug?id=638315cdb2995506f6c1404695773403d5840628
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

uint64_t r[1] = {0xffffffffffffffff};
void loop()
{
  long res = 0;
  memcpy((void*)0x200004c0, "./file0", 8);
  syscall(__NR_mkdir, 0x200004c0, 0x7c);
  *(uint32_t*)0x200ba000 = 6;
  *(uint32_t*)0x200ba004 = 3;
  *(uint64_t*)0x200ba008 = 0x20346fc8;
  *(uint8_t*)0x20346fc8 = 0x18;
  STORE_BY_BITMASK(uint8_t, 0x20346fc9, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x20346fc9, 0, 4, 4);
  *(uint16_t*)0x20346fca = 0;
  *(uint32_t*)0x20346fcc = 0;
  *(uint8_t*)0x20346fd0 = 0;
  *(uint8_t*)0x20346fd1 = 0;
  *(uint16_t*)0x20346fd2 = 0;
  *(uint32_t*)0x20346fd4 = 0;
  *(uint8_t*)0x20346fd8 = 0x95;
  *(uint8_t*)0x20346fd9 = 0;
  *(uint16_t*)0x20346fda = 0;
  *(uint32_t*)0x20346fdc = 0;
  *(uint64_t*)0x200ba010 = 0x20f6bffb;
  memcpy((void*)0x20f6bffb, "GPL", 4);
  *(uint32_t*)0x200ba018 = 1;
  *(uint32_t*)0x200ba01c = 0xfb;
  *(uint64_t*)0x200ba020 = 0x201a7f05;
  *(uint32_t*)0x200ba028 = 0;
  *(uint32_t*)0x200ba02c = 0;
  *(uint8_t*)0x200ba030 = 0;
  *(uint8_t*)0x200ba031 = 0;
  *(uint8_t*)0x200ba032 = 0;
  *(uint8_t*)0x200ba033 = 0;
  *(uint8_t*)0x200ba034 = 0;
  *(uint8_t*)0x200ba035 = 0;
  *(uint8_t*)0x200ba036 = 0;
  *(uint8_t*)0x200ba037 = 0;
  *(uint8_t*)0x200ba038 = 0;
  *(uint8_t*)0x200ba039 = 0;
  *(uint8_t*)0x200ba03a = 0;
  *(uint8_t*)0x200ba03b = 0;
  *(uint8_t*)0x200ba03c = 0;
  *(uint8_t*)0x200ba03d = 0;
  *(uint8_t*)0x200ba03e = 0;
  *(uint8_t*)0x200ba03f = 0;
  *(uint32_t*)0x200ba040 = 0;
  *(uint32_t*)0x200ba044 = 0;
  res = syscall(__NR_bpf, 5, 0x200ba000, 0x48);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x200000c0, "./file0", 8);
  memcpy((void*)0x20000000, "./file0", 8);
  memcpy((void*)0x200001c0, "bpf", 4);
  syscall(__NR_mount, 0x200000c0, 0x20000000, 0x200001c0, 0, 0x20000080);
  *(uint64_t*)0x200005c0 = 0x20000600;
  memcpy((void*)0x20000600, "./file0/file0", 14);
  *(uint32_t*)0x200005c8 = r[0];
  syscall(__NR_bpf, 6, 0x200005c0, 0x10);
  memcpy((void*)0x20000100, "./file0/file0", 14);
  syscall(__NR_openat, 0xffffffffffffff9c, 0x20000100, 0x40, 4);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
