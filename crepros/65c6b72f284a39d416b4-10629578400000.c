// https://syzkaller.appspot.com/bug?id=2a622455acd7051c6cf85c360cd116118a587726
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

static void execute_one();
extern unsigned long long procid;

void loop()
{
  while (1) {
    execute_one();
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
void execute_one()
{
  long res = 0;
  res = syscall(__NR_pipe2, 0x20000100, 0);
  if (res != -1) {
    r[0] = *(uint32_t*)0x20000100;
    r[1] = *(uint32_t*)0x20000104;
  }
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
  memcpy((void*)0x20000000, "./file0", 8);
  memcpy((void*)0x20000900, "9p", 3);
  memcpy((void*)0x20000840, "trans=fd,", 9);
  memcpy((void*)0x20000849, "rfdno", 5);
  *(uint8_t*)0x2000084e = 0x3d;
  sprintf((char*)0x2000084f, "0x%016llx", (long long)r[0]);
  *(uint8_t*)0x20000861 = 0x2c;
  memcpy((void*)0x20000862, "wfdno", 5);
  *(uint8_t*)0x20000867 = 0x3d;
  sprintf((char*)0x20000868, "0x%016llx", (long long)r[1]);
  *(uint8_t*)0x2000087a = 0x2c;
  memcpy((void*)0x2000087b, "access", 6);
  *(uint8_t*)0x20000881 = 0x3d;
  sprintf((char*)0x20000882, "%020llu", (long long)0);
  *(uint8_t*)0x20000896 = 0x2c;
  *(uint8_t*)0x20000897 = 0;
  syscall(__NR_mount, 0, 0x20000000, 0x20000900, 0, 0x20000840);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    loop();
  }
}
