// https://syzkaller.appspot.com/bug?id=ec69610f74e78616269c2ec8f697d8efc701a31c
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

static void test();

void loop()
{
  while (1) {
    test();
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
void test()
{
  long res;
  res = syscall(__NR_socket, 0xa, 2, 0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_socket, 0x18, 1, 1);
  if (res != -1)
    r[1] = res;
  *(uint16_t*)0x205fafd2 = 0x18;
  *(uint32_t*)0x205fafd4 = 1;
  *(uint32_t*)0x205fafd8 = 0;
  *(uint32_t*)0x205fafdc = r[0];
  *(uint16_t*)0x205fafe0 = 2;
  *(uint16_t*)0x205fafe2 = htobe16(0x4e21);
  *(uint8_t*)0x205fafe4 = 0xac;
  *(uint8_t*)0x205fafe5 = 0x14;
  *(uint8_t*)0x205fafe6 = 0;
  *(uint8_t*)0x205fafe7 = 0xbb;
  *(uint8_t*)0x205fafe8 = 0;
  *(uint8_t*)0x205fafe9 = 0;
  *(uint8_t*)0x205fafea = 0;
  *(uint8_t*)0x205fafeb = 0;
  *(uint8_t*)0x205fafec = 0;
  *(uint8_t*)0x205fafed = 0;
  *(uint8_t*)0x205fafee = 0;
  *(uint8_t*)0x205fafef = 0;
  *(uint32_t*)0x205faff0 = 4;
  *(uint32_t*)0x205faff4 = 0;
  *(uint32_t*)0x205faff8 = 2;
  *(uint32_t*)0x205faffc = 0;
  syscall(__NR_connect, r[1], 0x205fafd2, 0x2e);
  *(uint16_t*)0x200000c0 = 0xa;
  *(uint16_t*)0x200000c2 = htobe16(0x4e21);
  *(uint32_t*)0x200000c4 = 0x7fff;
  *(uint8_t*)0x200000c8 = 0;
  *(uint8_t*)0x200000c9 = 0;
  *(uint8_t*)0x200000ca = 0;
  *(uint8_t*)0x200000cb = 0;
  *(uint8_t*)0x200000cc = 0;
  *(uint8_t*)0x200000cd = 0;
  *(uint8_t*)0x200000ce = 0;
  *(uint8_t*)0x200000cf = 0;
  *(uint8_t*)0x200000d0 = 0;
  *(uint8_t*)0x200000d1 = 0;
  *(uint8_t*)0x200000d2 = -1;
  *(uint8_t*)0x200000d3 = -1;
  *(uint8_t*)0x200000d4 = 0xac;
  *(uint8_t*)0x200000d5 = 0x14;
  *(uint8_t*)0x200000d6 = 0x14;
  *(uint8_t*)0x200000d7 = 0xaa;
  *(uint32_t*)0x200000d8 = 3;
  syscall(__NR_connect, r[0], 0x200000c0, 0x1c);
  *(uint16_t*)0x200021c0 = 0x18;
  *(uint32_t*)0x200021c2 = 0;
  *(uint16_t*)0x200021c6 = 0;
  *(uint8_t*)0x200021c8 = 0;
  *(uint8_t*)0x200021c9 = 0;
  *(uint8_t*)0x200021ca = 0;
  *(uint8_t*)0x200021cb = 0;
  *(uint8_t*)0x200021cc = 0;
  *(uint8_t*)0x200021cd = 0;
  memcpy((void*)0x200021ce,
         "\x69\x66\x62\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  syscall(__NR_sendto, r[1], 0x20001180, 0, 0x40001, 0x200021c0, 0x80);
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x2000000c = -1;
  *(uint32_t*)0x20000010 = 0;
  *(uint32_t*)0x20000014 = 0;
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    loop();
  }
}
