// https://syzkaller.appspot.com/bug?id=8f063539d4ecf1faf3132624b57a641e923ee25a
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

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x20000000, "/dev/kvm", 9);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000000, 0, 0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_ioctl, r[0], 0xae01, 0);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_ioctl, r[1], 0xae41, 0);
  if (res != -1)
    r[2] = res;
  *(uint32_t*)0x20000100 = 1;
  *(uint32_t*)0x20000104 = 0;
  *(uint32_t*)0x20000108 = 0x40000108;
  *(uint32_t*)0x2000010c = 0;
  *(uint64_t*)0x20000110 = 0;
  syscall(__NR_ioctl, r[2], 0x4080aebf, 0x20000100);
  *(uint16_t*)0x200000c0 = 8;
  *(uint16_t*)0x200000c2 = 6;
  *(uint16_t*)0x200000c4 = 1;
  *(uint16_t*)0x200000c6 = 0x58;
  memcpy((void*)0x200000c8,
         "\x73\x79\x7a\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         80);
  *(uint32_t*)0x20000118 = 0x53;
  syscall(__NR_ioctl, -1, 0x405c5503, 0x200000c0);
  *(uint8_t*)0x20000240 = 1;
  *(uint8_t*)0x20000241 = 0;
  syscall(__NR_write, -1, 0x20000240, 2);
  return 0;
}
