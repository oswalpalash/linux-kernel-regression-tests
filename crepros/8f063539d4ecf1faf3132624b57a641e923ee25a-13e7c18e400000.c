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
  *(uint32_t*)0x20000100 = 0xfffffffe;
  *(uint32_t*)0x20000104 = 0;
  *(uint64_t*)0x20000108 = 0;
  *(uint64_t*)0x20000110 = 0;
  *(uint64_t*)0x20000118 = 0;
  *(uint64_t*)0x20000120 = 0;
  *(uint8_t*)0x20000128 = 0;
  *(uint8_t*)0x20000129 = 0;
  *(uint8_t*)0x2000012a = 0;
  *(uint8_t*)0x2000012b = 0;
  *(uint8_t*)0x2000012c = 0;
  *(uint8_t*)0x2000012d = 0;
  *(uint8_t*)0x2000012e = 0;
  *(uint8_t*)0x2000012f = 0;
  *(uint8_t*)0x20000130 = 0;
  *(uint8_t*)0x20000131 = 0;
  *(uint8_t*)0x20000132 = 0;
  *(uint8_t*)0x20000133 = 0;
  *(uint8_t*)0x20000134 = 0;
  *(uint8_t*)0x20000135 = 0;
  *(uint8_t*)0x20000136 = 0;
  *(uint8_t*)0x20000137 = 0;
  *(uint8_t*)0x20000138 = 0;
  *(uint8_t*)0x20000139 = 0;
  *(uint8_t*)0x2000013a = 0;
  *(uint8_t*)0x2000013b = 0;
  *(uint8_t*)0x2000013c = 0;
  *(uint8_t*)0x2000013d = 0;
  *(uint8_t*)0x2000013e = 0;
  *(uint8_t*)0x2000013f = 0;
  *(uint8_t*)0x20000140 = 0;
  *(uint8_t*)0x20000141 = 0;
  *(uint8_t*)0x20000142 = 0;
  *(uint8_t*)0x20000143 = 0;
  *(uint8_t*)0x20000144 = 0;
  *(uint8_t*)0x20000145 = 0;
  *(uint8_t*)0x20000146 = 0;
  *(uint8_t*)0x20000147 = 0;
  *(uint8_t*)0x20000148 = 0;
  *(uint8_t*)0x20000149 = 0;
  *(uint8_t*)0x2000014a = 0;
  *(uint8_t*)0x2000014b = 0;
  *(uint8_t*)0x2000014c = 0;
  *(uint8_t*)0x2000014d = 0;
  *(uint8_t*)0x2000014e = 0;
  *(uint8_t*)0x2000014f = 0;
  *(uint8_t*)0x20000150 = 0;
  *(uint8_t*)0x20000151 = 0;
  *(uint8_t*)0x20000152 = 0;
  *(uint8_t*)0x20000153 = 0;
  *(uint8_t*)0x20000154 = 0;
  *(uint8_t*)0x20000155 = 0;
  *(uint8_t*)0x20000156 = 0;
  *(uint8_t*)0x20000157 = 0;
  *(uint8_t*)0x20000158 = 0;
  *(uint8_t*)0x20000159 = 0;
  *(uint8_t*)0x2000015a = 0;
  *(uint8_t*)0x2000015b = 0;
  *(uint8_t*)0x2000015c = 0;
  *(uint8_t*)0x2000015d = 0;
  *(uint8_t*)0x2000015e = 0;
  *(uint8_t*)0x2000015f = 0;
  *(uint8_t*)0x20000160 = 0;
  *(uint8_t*)0x20000161 = 0;
  *(uint8_t*)0x20000162 = 0;
  *(uint8_t*)0x20000163 = 0;
  *(uint8_t*)0x20000164 = 0;
  *(uint8_t*)0x20000165 = 0;
  *(uint8_t*)0x20000166 = 0;
  *(uint8_t*)0x20000167 = 0;
  syscall(__NR_ioctl, r[2], 0xc080aebe, 0x20000100);
  return 0;
}
