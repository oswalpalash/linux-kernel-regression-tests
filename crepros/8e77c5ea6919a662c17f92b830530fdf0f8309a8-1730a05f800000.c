// https://syzkaller.appspot.com/bug?id=8e77c5ea6919a662c17f92b830530fdf0f8309a8
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_add_key
#define __NR_add_key 286
#endif
#ifndef __NR_keyctl
#define __NR_keyctl 288
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[5] = {0x0, 0x0, 0x0, 0x0, 0x0};
void loop()
{
  long res = 0;
  memcpy((void*)0x20000000, "keyring", 8);
  *(uint8_t*)0x20000040 = 0x73;
  *(uint8_t*)0x20000041 = 0x79;
  *(uint8_t*)0x20000042 = 0x7a;
  *(uint8_t*)0x20000043 = 0;
  *(uint8_t*)0x20000044 = 0;
  res = syscall(__NR_add_key, 0x20000000, 0x20000040, 0, 0, 0xfffffffd);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x200004c0, "keyring", 8);
  *(uint8_t*)0x20000100 = 0x73;
  *(uint8_t*)0x20000101 = 0x79;
  *(uint8_t*)0x20000102 = 0x7a;
  *(uint8_t*)0x20000103 = 0;
  *(uint8_t*)0x20000104 = 0;
  res = syscall(__NR_add_key, 0x200004c0, 0x20000100, 0, 0, r[0]);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x200001c0, "user", 5);
  *(uint8_t*)0x20000200 = 0x73;
  *(uint8_t*)0x20000201 = 0x79;
  *(uint8_t*)0x20000202 = 0x7a;
  *(uint8_t*)0x20000203 = 0x22;
  *(uint8_t*)0x20000204 = 0;
  memcpy((void*)0x20000240, "\xfa", 1);
  res = syscall(__NR_add_key, 0x200001c0, 0x20000200, 0x20000240, 1, r[0]);
  if (res != -1)
    r[2] = res;
  memcpy((void*)0x20688000, "user", 5);
  *(uint8_t*)0x208d9ffb = 0x73;
  *(uint8_t*)0x208d9ffc = 0x79;
  *(uint8_t*)0x208d9ffd = 0x7a;
  *(uint8_t*)0x208d9ffe = 0;
  *(uint8_t*)0x208d9fff = 0;
  memcpy((void*)0x20f19edc,
         "\xb3\x3a\xb7\x60\x79\xeb\xe0\xd1\x4f\x72\x9c\xd6\x53\xe5\x20\xd2\x9a"
         "\xd7\xef\x00\x00\x00\x00\x00\x00\x00\x44\xc2\x49\xb5\x44\x23\x0b\x93"
         "\x87\xfb\x8b\xd6\xed\x26\x6c\xcf\x59\xef\x70\x99\x5b\xf2\xe8\xe0\xec"
         "\xd3\xff\xf3\x28\x53\x74\x7e\xda\x22\xd2\x81\x8d\x08\xca\x27\xe0\xec"
         "\x82\x16\x20\xe3\x65\xa0\xe6\xb9\x48\x5f\x2d\x92\x54\x93\xf6\x21\x13"
         "\xe3\x3e\x5f\x8c\x7e\xba\x67\xfc\x19\xa9\x49\x7f\x5b\x07\xe5\x84\x9d"
         "\x2e\x87\x5b\x06\x6c\xd6\x40\x1d\x36\x61\x6f\xe0\xf3\xc3\x00\x28\x01"
         "\xb4\x62\x7e\xe7\x59\x76\x89\x52\x5e\x8e\x81\xf7\x50\xa8\x6e\xb5\x80"
         "\xfb\x46\x90\xea\x52\x24\x6b\xd3\xd3\x2b\x1a\x91\xf9\x44\xed\xb7\x4b"
         "\x1f\x50\xae\x08\xc5\x38\x7e\xd8\xfd\x05\x98\xb6\x00\x57\x9f\x3a\xf3"
         "\xf8\x64\xe1\xc3\x24\xf6\x92\x8f\x66\x72\xf9\x8f\x7e\x14\x9b\xd6\x1b"
         "\xd7\x8b\x50\x6e\x8b",
         192);
  res = syscall(__NR_add_key, 0x20688000, 0x208d9ffb, 0x20f19edc, 0xc0, r[1]);
  if (res != -1)
    r[3] = res;
  memcpy((void*)0x200002c0, "user", 5);
  *(uint8_t*)0x20000300 = 0x73;
  *(uint8_t*)0x20000301 = 0x79;
  *(uint8_t*)0x20000302 = 0x7a;
  *(uint8_t*)0x20000303 = 0x22;
  *(uint8_t*)0x20000304 = 0;
  memcpy((void*)0x20000340, "\xc2\x07\x83\x97\xa1\x7c\xd3\x1a\xd9\xc5\x26\x84"
                            "\x42\xe6\xed\xc2\xc7\x94\x5c\xd8\x8e\xe7\x27\xff"
                            "\xec\xa5\xbd\x51\x4c\x97\xe5\x01\x6a\x36\xee\x92"
                            "\x96\x97\x2e\xc5\xc8\x14\xfd\x40\x1d\x71\xd8\x10"
                            "\xa3\xcf\x92\xfc\x7b\x33\x87\xe7",
         56);
  res = syscall(__NR_add_key, 0x200002c0, 0x20000300, 0x20000340, 0x38, r[1]);
  if (res != -1)
    r[4] = res;
  *(uint32_t*)0x20000380 = r[4];
  *(uint32_t*)0x20000384 = r[3];
  *(uint32_t*)0x20000388 = r[2];
  *(uint32_t*)0x200005c0 = 0x20000580;
  memcpy((void*)0x20000580,
         "\x74\x67\x72\x31\x39\x32\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         64);
  *(uint32_t*)0x200005c4 = 0;
  *(uint32_t*)0x200005c8 = 0;
  *(uint32_t*)0x200005cc = 0;
  *(uint32_t*)0x200005d0 = 0;
  *(uint32_t*)0x200005d4 = 0;
  *(uint32_t*)0x200005d8 = 0;
  *(uint32_t*)0x200005dc = 0;
  *(uint32_t*)0x200005e0 = 0;
  *(uint32_t*)0x200005e4 = 0;
  *(uint32_t*)0x200005e8 = 0;
  syscall(__NR_keyctl, 0x17, 0x20000380, 0x20000540, 0x10, 0x200005c0);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
