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
  *(uint32_t*)0x20000140 = 0x10;
  *(uint32_t*)0x20000144 = 0;
  *(uint64_t*)0x20000148 = 0x1a0;
  *(uint64_t*)0x20000150 = 0;
  *(uint64_t*)0x20000158 = 0;
  *(uint64_t*)0x20000160 = 0;
  *(uint8_t*)0x20000168 = 0;
  *(uint8_t*)0x20000169 = 2;
  *(uint8_t*)0x2000016a = 0;
  *(uint8_t*)0x2000016b = 0;
  *(uint8_t*)0x2000016c = 0;
  *(uint8_t*)0x2000016d = 0;
  *(uint8_t*)0x2000016e = 0;
  *(uint8_t*)0x2000016f = 0;
  *(uint8_t*)0x20000170 = 0;
  *(uint8_t*)0x20000171 = 0;
  *(uint8_t*)0x20000172 = 0;
  *(uint8_t*)0x20000173 = 0;
  *(uint8_t*)0x20000174 = 0;
  *(uint8_t*)0x20000175 = 0;
  *(uint8_t*)0x20000176 = 0;
  *(uint8_t*)0x20000177 = 0;
  *(uint8_t*)0x20000178 = 0;
  *(uint8_t*)0x20000179 = 0;
  *(uint8_t*)0x2000017a = 0;
  *(uint8_t*)0x2000017b = 0;
  *(uint8_t*)0x2000017c = 0;
  *(uint8_t*)0x2000017d = 0;
  *(uint8_t*)0x2000017e = 0;
  *(uint8_t*)0x2000017f = 0;
  *(uint8_t*)0x20000180 = 0;
  *(uint8_t*)0x20000181 = 0;
  *(uint8_t*)0x20000182 = 0;
  *(uint8_t*)0x20000183 = 0;
  *(uint8_t*)0x20000184 = 0;
  *(uint8_t*)0x20000185 = 0;
  *(uint8_t*)0x20000186 = 0;
  *(uint8_t*)0x20000187 = 0;
  *(uint8_t*)0x20000188 = 0;
  *(uint8_t*)0x20000189 = 0;
  *(uint8_t*)0x2000018a = 0;
  *(uint8_t*)0x2000018b = 0;
  *(uint8_t*)0x2000018c = 0;
  *(uint8_t*)0x2000018d = 0;
  *(uint8_t*)0x2000018e = 0;
  *(uint8_t*)0x2000018f = 0;
  *(uint8_t*)0x20000190 = 0;
  *(uint8_t*)0x20000191 = 0;
  *(uint8_t*)0x20000192 = 0;
  *(uint8_t*)0x20000193 = 0;
  *(uint8_t*)0x20000194 = 0;
  *(uint8_t*)0x20000195 = 0;
  *(uint8_t*)0x20000196 = 0;
  *(uint8_t*)0x20000197 = 0;
  *(uint8_t*)0x20000198 = 0;
  *(uint8_t*)0x20000199 = 0;
  *(uint8_t*)0x2000019a = 0;
  *(uint8_t*)0x2000019b = 0;
  *(uint8_t*)0x2000019c = 0;
  *(uint8_t*)0x2000019d = 0;
  *(uint8_t*)0x2000019e = 0;
  *(uint8_t*)0x2000019f = 0;
  *(uint8_t*)0x200001a0 = 0;
  *(uint8_t*)0x200001a1 = 0;
  *(uint8_t*)0x200001a2 = 0;
  *(uint8_t*)0x200001a3 = 0;
  *(uint8_t*)0x200001a4 = 0;
  *(uint8_t*)0x200001a5 = 0;
  *(uint8_t*)0x200001a6 = 0;
  *(uint8_t*)0x200001a7 = 0;
  syscall(__NR_ioctl, r[2], 0xc080aebe, 0x20000140);
  return 0;
}
