// https://syzkaller.appspot.com/bug?id=004b0f7b61d4901cbfecfc33de7996e8cbe0a278
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

uint64_t r[1] = {0xffffffffffffffff};
void loop()
{
  long res = 0;
  res = syscall(__NR_socket, 0x2b, 1, 0);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20003dc0 = 0x20000340;
  *(uint16_t*)0x20000340 = 0x1d;
  *(uint32_t*)0x20000344 = 0;
  *(uint32_t*)0x20000348 = 0;
  *(uint32_t*)0x2000034c = 0;
  *(uint32_t*)0x20003dc8 = 0x80;
  *(uint64_t*)0x20003dd0 = 0x200008c0;
  *(uint64_t*)0x200008c0 = 0x200003c0;
  *(uint64_t*)0x200008c8 = 0;
  *(uint64_t*)0x200008d0 = 0x200004c0;
  *(uint64_t*)0x200008d8 = 0;
  *(uint64_t*)0x200008e0 = 0x20000540;
  *(uint64_t*)0x200008e8 = 0;
  *(uint64_t*)0x200008f0 = 0x20000640;
  *(uint64_t*)0x200008f8 = 0;
  *(uint64_t*)0x20000900 = 0x20000680;
  *(uint64_t*)0x20000908 = 0;
  *(uint64_t*)0x20000910 = 0x20000700;
  *(uint64_t*)0x20000918 = 0;
  *(uint64_t*)0x20000920 = 0x20000740;
  *(uint64_t*)0x20000928 = 0;
  *(uint64_t*)0x20000930 = 0x20000780;
  *(uint64_t*)0x20000938 = 0;
  *(uint64_t*)0x20000940 = 0x20000840;
  *(uint64_t*)0x20000948 = 0;
  *(uint64_t*)0x20003dd8 = 9;
  *(uint64_t*)0x20003de0 = 0;
  *(uint64_t*)0x20003de8 = 0;
  *(uint32_t*)0x20003df0 = 0x20000000;
  *(uint32_t*)0x20003df8 = 4;
  *(uint64_t*)0x20003e00 = 0;
  *(uint32_t*)0x20003e08 = 0;
  *(uint64_t*)0x20003e10 = 0x20000a80;
  *(uint64_t*)0x20000a80 = 0x20000980;
  *(uint64_t*)0x20000a88 = 0;
  *(uint64_t*)0x20003e18 = 1;
  *(uint64_t*)0x20003e20 = 0;
  *(uint64_t*)0x20003e28 = 0;
  *(uint32_t*)0x20003e30 = 0x80;
  *(uint32_t*)0x20003e38 = 0x3f;
  *(uint64_t*)0x20003e40 = 0x20000ac0;
  *(uint16_t*)0x20000ac0 = 0x28;
  *(uint16_t*)0x20000ac2 = 0;
  *(uint32_t*)0x20000ac4 = -1;
  *(uint32_t*)0x20000ac8 = 1;
  *(uint32_t*)0x20000acc = 0;
  *(uint32_t*)0x20003e48 = 0x80;
  *(uint64_t*)0x20003e50 = 0x20000e40;
  *(uint64_t*)0x20000e40 = 0x20000b40;
  *(uint64_t*)0x20000e48 = 0;
  *(uint64_t*)0x20000e50 = 0x20000b80;
  *(uint64_t*)0x20000e58 = 0;
  *(uint64_t*)0x20000e60 = 0x20000c00;
  *(uint64_t*)0x20000e68 = 0;
  *(uint64_t*)0x20000e70 = 0x20000d00;
  *(uint64_t*)0x20000e78 = 0;
  *(uint64_t*)0x20000e80 = 0x20000d40;
  *(uint64_t*)0x20000e88 = 0;
  *(uint64_t*)0x20003e58 = 5;
  *(uint64_t*)0x20003e60 = 0x20000ec0;
  *(uint64_t*)0x20000ec0 = 0x10;
  *(uint32_t*)0x20000ec8 = 0x10a;
  *(uint32_t*)0x20000ecc = 0xfffffffb;
  *(uint64_t*)0x20000ed0 = 0x10;
  *(uint32_t*)0x20000ed8 = 0x11f;
  *(uint32_t*)0x20000edc = 4;
  *(uint64_t*)0x20000ee0 = 0x10;
  *(uint32_t*)0x20000ee8 = 0x1ff;
  *(uint32_t*)0x20000eec = 0xabd;
  *(uint64_t*)0x20000ef0 = 0x10;
  *(uint32_t*)0x20000ef8 = 0x11f;
  *(uint32_t*)0x20000efc = 0x8001;
  *(uint64_t*)0x20000f00 = 0x10;
  *(uint32_t*)0x20000f08 = 0x13f;
  *(uint32_t*)0x20000f0c = 0x542a;
  *(uint64_t*)0x20000f10 = 0x10;
  *(uint32_t*)0x20000f18 = 0x100;
  *(uint32_t*)0x20000f1c = 0x400;
  *(uint64_t*)0x20000f20 = 0x10;
  *(uint32_t*)0x20000f28 = 0x117;
  *(uint32_t*)0x20000f2c = 0x33;
  *(uint64_t*)0x20000f30 = 0x10;
  *(uint32_t*)0x20000f38 = 1;
  *(uint32_t*)0x20000f3c = 0;
  *(uint64_t*)0x20003e68 = 0x80;
  *(uint32_t*)0x20003e70 = 0x20004010;
  *(uint32_t*)0x20003e78 = 0x7fffffff;
  *(uint64_t*)0x20003e80 = 0x20001240;
  *(uint16_t*)0x20001240 = 0x11;
  *(uint16_t*)0x20001242 = htobe16(0xff);
  *(uint32_t*)0x20001244 = 0;
  *(uint16_t*)0x20001248 = 1;
  *(uint8_t*)0x2000124a = 1;
  *(uint8_t*)0x2000124b = 6;
  memcpy((void*)0x2000124c, "\xc2\xe0\x9b\x8a\xb9\x38", 6);
  *(uint8_t*)0x20001252 = 0;
  *(uint8_t*)0x20001253 = 0;
  *(uint32_t*)0x20003e88 = 0x80;
  *(uint64_t*)0x20003e90 = 0x20002540;
  *(uint64_t*)0x20002540 = 0x200012c0;
  *(uint64_t*)0x20002548 = 0;
  *(uint64_t*)0x20002550 = 0x200022c0;
  *(uint64_t*)0x20002558 = 0;
  *(uint64_t*)0x20002560 = 0x200023c0;
  *(uint64_t*)0x20002568 = 0;
  *(uint64_t*)0x20002570 = 0x20002480;
  *(uint64_t*)0x20002578 = 0;
  *(uint64_t*)0x20003e98 = 4;
  *(uint64_t*)0x20003ea0 = 0x20002580;
  *(uint64_t*)0x20002580 = 0x10;
  *(uint32_t*)0x20002588 = 0x13a;
  *(uint32_t*)0x2000258c = 0x4227;
  *(uint64_t*)0x20002590 = 0x10;
  *(uint32_t*)0x20002598 = 0x11f;
  *(uint32_t*)0x2000259c = 0x10001;
  *(uint64_t*)0x200025a0 = 0x10;
  *(uint32_t*)0x200025a8 = 0x107;
  *(uint32_t*)0x200025ac = 0x8144;
  *(uint64_t*)0x20003ea8 = 0x30;
  *(uint32_t*)0x20003eb0 = 0x20000000;
  *(uint32_t*)0x20003eb8 = 5;
  *(uint64_t*)0x20003ec0 = 0x20002740;
  *(uint16_t*)0x20002740 = 0x26;
  memcpy((void*)0x20002742,
         "\x68\x61\x73\x68\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 14);
  *(uint32_t*)0x20002750 = 0;
  *(uint32_t*)0x20002754 = 0;
  memcpy((void*)0x20002758,
         "\x73\x68\x61\x32\x32\x34\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         64);
  *(uint32_t*)0x20003ec8 = 0x80;
  *(uint64_t*)0x20003ed0 = 0x20002940;
  *(uint64_t*)0x20002940 = 0x200027c0;
  *(uint64_t*)0x20002948 = 0;
  *(uint64_t*)0x20002950 = 0x20002800;
  *(uint64_t*)0x20002958 = 0;
  *(uint64_t*)0x20002960 = 0x20002840;
  *(uint64_t*)0x20002968 = 0;
  *(uint64_t*)0x20002970 = 0x20002880;
  *(uint64_t*)0x20002978 = 0;
  *(uint64_t*)0x20003ed8 = 4;
  *(uint64_t*)0x20003ee0 = 0x20002980;
  *(uint64_t*)0x20002980 = 0x10;
  *(uint32_t*)0x20002988 = 0x11d;
  *(uint32_t*)0x2000298c = 0;
  *(uint64_t*)0x20002990 = 0x10;
  *(uint32_t*)0x20002998 = 0x117;
  *(uint32_t*)0x2000299c = 2;
  *(uint64_t*)0x20003ee8 = 0x20;
  *(uint32_t*)0x20003ef0 = 0x40;
  *(uint32_t*)0x20003ef8 = 3;
  *(uint64_t*)0x20003f00 = 0x20002a40;
  *(uint16_t*)0x20002a40 = 0x27;
  *(uint32_t*)0x20002a44 = 1;
  *(uint32_t*)0x20002a48 = 2;
  *(uint32_t*)0x20002a4c = 6;
  *(uint32_t*)0x20003f08 = 0x80;
  *(uint64_t*)0x20003f10 = 0x20003c00;
  *(uint64_t*)0x20003c00 = 0x20002ac0;
  *(uint64_t*)0x20003c08 = 0;
  *(uint64_t*)0x20003c10 = 0x20002bc0;
  *(uint64_t*)0x20003c18 = 0;
  *(uint64_t*)0x20003c20 = 0x20003bc0;
  *(uint64_t*)0x20003c28 = 0;
  *(uint64_t*)0x20003f18 = 3;
  *(uint64_t*)0x20003f20 = 0x20003f40;
  *(uint64_t*)0x20003f28 = 0;
  *(uint32_t*)0x20003f30 = 1;
  *(uint32_t*)0x20003f38 = 0x2951;
  syscall(__NR_sendmmsg, r[0], 0x20003dc0, 6, 0x20000000);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
