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
  *(uint32_t*)0x200001c0 = r[0];
  *(uint32_t*)0x200001c4 = 0;
  *(uint32_t*)0x200001c8 = 0x60;
  *(uint32_t*)0x200001cc = 0x2d;
  *(uint64_t*)0x200001d0 = 0x200000c0;
  memcpy((void*)0x200000c0,
         "\x19\x35\xe8\xfc\x84\xa7\x18\x1a\x63\x15\x10\x96\x76\x19\xab\xc7\x9f"
         "\xc4\x2b\x31\x4a\xf8\xd6\x38\xf6\xac\x5b\x9f\x74\x77\xec\xc4\x29\x38"
         "\x10\x4a\x89\x05\xb7\xc4\x0a\xd8\x4e\xde\x11\x6d\x3b\x07\x6b\xb5\x8a"
         "\x9f\x55\xf1\x35\x26\x4a\x0a\xca\x4d\x0d\x8b\x33\x5c\x87\xc8\x27\xa5"
         "\xc0\x67\xbf\xd8\x84\x5a\xb9\x82\x54\xc9\x49\x84\xff\x1b\x0f\x73\x25"
         "\x3d\xbb\x63\xe4\xb4\x6a\x83\xe7\x98\x4f\x87",
         96);
  *(uint64_t*)0x200001d8 = 0x20000180;
  *(uint32_t*)0x200001e0 = 0x9520;
  *(uint32_t*)0x200001e4 = 0;
  syscall(__NR_bpf, 0xa, 0x200001c0, 0x28);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
