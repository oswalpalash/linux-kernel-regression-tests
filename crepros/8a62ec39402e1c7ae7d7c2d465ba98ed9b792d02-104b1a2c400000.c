// https://syzkaller.appspot.com/bug?id=8a62ec39402e1c7ae7d7c2d465ba98ed9b792d02
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
void loop()
{
  long res = 0;
  res = syscall(__NR_pipe2, 0x20000100, 0);
  if (res != -1) {
    r[0] = *(uint32_t*)0x20000100;
    r[1] = *(uint32_t*)0x20000104;
  }
  *(uint32_t*)0x20000040 = 0xffffffca;
  *(uint8_t*)0x20000044 = 0x4d;
  *(uint16_t*)0x20000045 = 0;
  syscall(__NR_write, r[1], 0x20000040, 7);
  memcpy((void*)0x20000140, "./file0", 8);
  syscall(__NR_mkdir, 0x20000140, 0);
  memcpy((void*)0x200000c0, "./file0", 8);
  memcpy((void*)0x20000340, "9p", 3);
  memcpy((void*)0x200001c0, "trans=fd,", 9);
  memcpy((void*)0x200001c9, "rfdno", 5);
  *(uint8_t*)0x200001ce = 0x3d;
  sprintf((char*)0x200001cf, "0x%016llx", (long long)r[0]);
  *(uint8_t*)0x200001e1 = 0x2c;
  memcpy((void*)0x200001e2, "wfdno", 5);
  *(uint8_t*)0x200001e7 = 0x3d;
  sprintf((char*)0x200001e8, "0x%016llx", (long long)r[1]);
  *(uint8_t*)0x200001fa = 0x2c;
  *(uint8_t*)0x200001fb = 0;
  syscall(__NR_mount, 0, 0x200000c0, 0x20000340, 0, 0x200001c0);
  *(uint32_t*)0x20000480 = 0x2a;
  *(uint8_t*)0x20000484 = 0x29;
  *(uint16_t*)0x20000485 = 1;
  *(uint32_t*)0x20000487 = 0;
  *(uint8_t*)0x2000048b = 0;
  *(uint32_t*)0x2000048c = 0;
  *(uint64_t*)0x20000490 = 0;
  *(uint64_t*)0x20000498 = 0;
  *(uint8_t*)0x200004a0 = 0;
  *(uint16_t*)0x200004a1 = 7;
  memcpy((void*)0x200004a3, "./file0", 7);
  syscall(__NR_write, r[1], 0x20000480, 0x2a);
  *(uint32_t*)0x20000200 = 0xa0;
  *(uint8_t*)0x20000204 = 0x19;
  *(uint16_t*)0x20000205 = 1;
  *(uint64_t*)0x20000207 = 0;
  *(uint8_t*)0x2000020f = 0;
  *(uint32_t*)0x20000210 = 0;
  *(uint64_t*)0x20000214 = 0;
  *(uint32_t*)0x2000021c = 0;
  *(uint32_t*)0x20000220 = 0;
  *(uint32_t*)0x20000224 = 0;
  *(uint64_t*)0x20000228 = 0;
  *(uint64_t*)0x20000230 = 0;
  *(uint64_t*)0x20000238 = 0;
  *(uint64_t*)0x20000240 = 0;
  *(uint64_t*)0x20000248 = 0;
  *(uint64_t*)0x20000250 = 0;
  *(uint64_t*)0x20000258 = 0;
  *(uint64_t*)0x20000260 = 0;
  *(uint64_t*)0x20000268 = 0;
  *(uint64_t*)0x20000270 = 0;
  *(uint64_t*)0x20000278 = 0;
  *(uint64_t*)0x20000280 = 0;
  *(uint64_t*)0x20000288 = 0;
  *(uint64_t*)0x20000290 = 0;
  *(uint64_t*)0x20000298 = 0;
  syscall(__NR_write, r[1], 0x20000200, 0xa0);
  memcpy((void*)0x20000400, "\x2a\x00\x00\x00\x29\x01\x00\x00\x00\x00\x00\x07"
                            "\x00\x00\x00\x00\x00\x00\x00\xc1\xc7\xac\xd2\x89"
                            "\xe7\x13\x25\x6a\xf9\x5c\x5b\x8a\x50\x92",
         34);
  syscall(__NR_write, r[1], 0x20000400, 0x22);
  memcpy((void*)0x20000000, "./file0", 8);
  memcpy((void*)0x20000900, "9p", 3);
  memcpy((void*)0x20000540, "trans=fd,", 9);
  memcpy((void*)0x20000549, "rfdno", 5);
  *(uint8_t*)0x2000054e = 0x3d;
  sprintf((char*)0x2000054f, "0x%016llx", (long long)r[0]);
  *(uint8_t*)0x20000561 = 0x2c;
  memcpy((void*)0x20000562, "wfdno", 5);
  *(uint8_t*)0x20000567 = 0x3d;
  sprintf((char*)0x20000568, "0x%016llx", (long long)r[1]);
  *(uint8_t*)0x2000057a = 0x2c;
  memcpy((void*)0x2000057b, "posixacl", 8);
  *(uint8_t*)0x20000583 = 0x2c;
  *(uint8_t*)0x20000584 = 0;
  syscall(__NR_mount, 0, 0x20000000, 0x20000900, 0, 0x20000540);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
