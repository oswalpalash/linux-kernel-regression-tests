// https://syzkaller.appspot.com/bug?id=059cee5623ce519359e7440ba6d0d6af8b82694e
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[1] = {0xffffffffffffffff};
void loop()
{
  long res = 0;
  *(uint32_t*)0x20000200 = 0xc;
  *(uint32_t*)0x20000204 = 0xe;
  *(uint64_t*)0x20000208 = 0x20000000;
  memcpy((void*)0x20000000,
         "\xb7\x02\x00\x00\x02\x00\x00\x00\xbf\xa3\x00\x00\x00\x00\x00\x00\x07"
         "\x03\x00\x00\x00\xfe\xff\xff\x7a\x0a\xf0\xff\xf8\xff\xff\xff\x79\xa4"
         "\xf0\xff\x00\x00\x00\x00\xb7\x06\x00\x00\xff\xff\xff\xff\x2d\x64\x05"
         "\x00\x00\x00\x00\x00\x65\x04\x04\x00\x01\x00\x00\x00\x04\x04\x00\x00"
         "\x01\x00\x00\x00\xb7\x03\x00\x00\x00\x00\x00\x00\x6a\x0a\x00\xfe\x00"
         "\x00\x00\x00\x85\x00\x00\x00\x2b\x00\x00\x00\xb7\x00\x00\x00\x00\x00"
         "\x00\x00\x95\x00\x00\x00\x00\x00\x00\x00",
         112);
  *(uint64_t*)0x20000210 = 0x20000340;
  memcpy((void*)0x20000340, "syzkaller", 10);
  *(uint32_t*)0x20000218 = 0;
  *(uint32_t*)0x2000021c = 0;
  *(uint64_t*)0x20000220 = 0;
  *(uint32_t*)0x20000228 = 0;
  *(uint32_t*)0x2000022c = 0;
  *(uint8_t*)0x20000230 = 0;
  *(uint8_t*)0x20000231 = 0;
  *(uint8_t*)0x20000232 = 0;
  *(uint8_t*)0x20000233 = 0;
  *(uint8_t*)0x20000234 = 0;
  *(uint8_t*)0x20000235 = 0;
  *(uint8_t*)0x20000236 = 0;
  *(uint8_t*)0x20000237 = 0;
  *(uint8_t*)0x20000238 = 0;
  *(uint8_t*)0x20000239 = 0;
  *(uint8_t*)0x2000023a = 0;
  *(uint8_t*)0x2000023b = 0;
  *(uint8_t*)0x2000023c = 0;
  *(uint8_t*)0x2000023d = 0;
  *(uint8_t*)0x2000023e = 0;
  *(uint8_t*)0x2000023f = 0;
  *(uint32_t*)0x20000240 = 0;
  *(uint32_t*)0x20000244 = 0;
  res = syscall(__NR_bpf, 5, 0x20000200, 0x48);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000180 = r[0];
  *(uint32_t*)0x20000184 = 0;
  *(uint32_t*)0x20000188 = 0x4b;
  *(uint32_t*)0x2000018c = 0xd4;
  *(uint64_t*)0x20000190 = 0x200000c0;
  memcpy((void*)0x200000c0,
         "\x48\x5e\x98\xaa\x12\xd7\x33\x0d\x7c\x8b\x2c\x2b\xda\x1c\xb5\xca\xcc"
         "\x3e\xd8\x67\xf0\xaf\x0b\xa5\xa4\x3f\x9a\xda\x2a\x35\x36\x8e\x7f\xeb"
         "\x7c\x1e\x20\xb3\x59\x01\x0c\xdd\x06\xc2\xb8\xca\xa4\x3e\xbc\xa6\x2b"
         "\xb4\xe5\xd2\x23\x8b\x72\x28\xb6\xaf\x76\xa9\x8b\x2f\xf1\x5d\xdc\x2a"
         "\x3b\x2c\x57\x8b\x3d\x1c\xf3",
         75);
  *(uint64_t*)0x20000198 = 0x20000480;
  *(uint32_t*)0x200001a0 = 0x7ff;
  *(uint32_t*)0x200001a4 = 0;
  syscall(__NR_bpf, 0xa, 0x20000180, 0x28);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
