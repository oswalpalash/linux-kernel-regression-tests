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
  *(uint64_t*)0x20006fc0 = 0x20000180;
  *(uint16_t*)0x20000180 = 0x27;
  *(uint32_t*)0x20000184 = 1;
  *(uint32_t*)0x20000188 = 0;
  *(uint32_t*)0x2000018c = 5;
  *(uint8_t*)0x20000190 = 8;
  *(uint8_t*)0x20000191 = 0xaf;
  memcpy((void*)0x20000192,
         "\xb4\x98\xa6\x62\x83\x48\x81\x71\x3a\xcc\x44\x9e\x88\x75\x9b\x3e\xb5"
         "\x4f\x06\x2f\x24\x31\xc4\xcc\xfc\x49\xc9\x30\x7a\xe0\xcc\xf6\x75\x2c"
         "\x50\xa4\x46\xe1\x3a\x09\x2a\xeb\x16\x1f\xcf\x03\xcd\x9a\x02\x6d\x33"
         "\xe9\x26\x96\x64\xf8\xfa\xee\x50\x44\x75\x09\x3f",
         63);
  *(uint64_t*)0x200001d8 = 5;
  *(uint32_t*)0x20006fc8 = 0x80;
  *(uint64_t*)0x20006fd0 = 0x20000600;
  *(uint64_t*)0x20000600 = 0x20000200;
  *(uint64_t*)0x20000608 = 0;
  *(uint64_t*)0x20000610 = 0x20000240;
  *(uint64_t*)0x20000618 = 0;
  *(uint64_t*)0x20000620 = 0x200002c0;
  *(uint64_t*)0x20000628 = 0;
  *(uint64_t*)0x20000630 = 0x200003c0;
  *(uint64_t*)0x20000638 = 0;
  *(uint64_t*)0x20000640 = 0x20000400;
  *(uint64_t*)0x20000648 = 0;
  *(uint64_t*)0x20000650 = 0x20000500;
  *(uint64_t*)0x20000658 = 0;
  *(uint64_t*)0x20006fd8 = 6;
  *(uint64_t*)0x20006fe0 = 0x20000680;
  *(uint64_t*)0x20000680 = 0x10;
  *(uint32_t*)0x20000688 = 0x101;
  *(uint32_t*)0x2000068c = 0x5199;
  *(uint64_t*)0x20000690 = 0x10;
  *(uint32_t*)0x20000698 = 0x19d;
  *(uint32_t*)0x2000069c = 6;
  *(uint64_t*)0x200006a0 = 0x10;
  *(uint32_t*)0x200006a8 = 0x10e;
  *(uint32_t*)0x200006ac = 9;
  *(uint64_t*)0x20006fe8 = 0x30;
  *(uint32_t*)0x20006ff0 = 0x10;
  *(uint32_t*)0x20006ff8 = 0x2e;
  *(uint64_t*)0x20007000 = 0x200017c0;
  *(uint16_t*)0x200017c0 = 0x27;
  *(uint32_t*)0x200017c4 = 0;
  *(uint32_t*)0x200017c8 = 2;
  *(uint32_t*)0x200017cc = 0x4a25669f;
  *(uint8_t*)0x200017d0 = 3;
  *(uint8_t*)0x200017d1 = 0x18;
  memcpy((void*)0x200017d2,
         "\x30\xf5\xea\x57\x6e\x15\xd5\x43\x29\xaf\x62\xad\x6e\x1a\xf8\xe2\x89"
         "\x37\x4c\xe4\xdf\x15\x1e\x06\x31\x08\xef\xff\xcb\xcf\xa3\xdd\x3b\x88"
         "\x29\xdb\xe9\xb0\x41\x60\x4d\xaf\xca\x75\x48\xc7\xe9\xb4\x96\xf3\x45"
         "\xf8\xce\x58\x9d\x67\x7d\x97\xf1\x3b\xe8\x6a\x1e",
         63);
  *(uint64_t*)0x20001818 = 7;
  *(uint32_t*)0x20007008 = 0x80;
  *(uint64_t*)0x20007010 = 0x20002b40;
  *(uint64_t*)0x20002b40 = 0x20001840;
  *(uint64_t*)0x20002b48 = 0;
  *(uint64_t*)0x20002b50 = 0x20001880;
  *(uint64_t*)0x20002b58 = 0;
  *(uint64_t*)0x20002b60 = 0x20002880;
  *(uint64_t*)0x20002b68 = 0;
  *(uint64_t*)0x20002b70 = 0x20002900;
  *(uint64_t*)0x20002b78 = 0;
  *(uint64_t*)0x20002b80 = 0x20002940;
  *(uint64_t*)0x20002b88 = 0;
  *(uint64_t*)0x20002b90 = 0x20002a40;
  *(uint64_t*)0x20002b98 = 0;
  *(uint64_t*)0x20007018 = 6;
  *(uint64_t*)0x20007020 = 0x20002bc0;
  *(uint64_t*)0x20002bc0 = 0x10;
  *(uint32_t*)0x20002bc8 = 0x19f;
  *(uint32_t*)0x20002bcc = 1;
  *(uint64_t*)0x20007028 = 0x10;
  *(uint32_t*)0x20007030 = 0x40000;
  *(uint32_t*)0x20007038 = 0x400;
  *(uint64_t*)0x20007040 = 0x20002d00;
  *(uint16_t*)0x20002d00 = 0x18;
  *(uint32_t*)0x20002d02 = 0;
  *(uint16_t*)0x20002d06 = 1;
  *(uint8_t*)0x20002d08 = -1;
  *(uint8_t*)0x20002d09 = -1;
  *(uint8_t*)0x20002d0a = -1;
  *(uint8_t*)0x20002d0b = -1;
  *(uint8_t*)0x20002d0c = -1;
  *(uint8_t*)0x20002d0d = -1;
  memcpy((void*)0x20002d0e,
         "\x62\x6f\x6e\x64\x5f\x73\x6c\x61\x76\x65\x5f\x30\x00\x00\x00\x00",
         16);
  *(uint32_t*)0x20007048 = 0x80;
  *(uint64_t*)0x20007050 = 0x20002e00;
  *(uint64_t*)0x20002e00 = 0x20002d80;
  *(uint64_t*)0x20002e08 = 0;
  *(uint64_t*)0x20002e10 = 0x20002dc0;
  *(uint64_t*)0x20002e18 = 0;
  *(uint64_t*)0x20007058 = 2;
  *(uint64_t*)0x20007060 = 0x20002e40;
  *(uint64_t*)0x20002e40 = 0x10;
  *(uint32_t*)0x20002e48 = 0x112;
  *(uint32_t*)0x20002e4c = -1;
  *(uint64_t*)0x20002e50 = 0x10;
  *(uint32_t*)0x20002e58 = 0;
  *(uint32_t*)0x20002e5c = 4;
  *(uint64_t*)0x20002e60 = 0x10;
  *(uint32_t*)0x20002e68 = 0x11;
  *(uint32_t*)0x20002e6c = 0x800;
  *(uint64_t*)0x20002e70 = 0x10;
  *(uint32_t*)0x20002e78 = 0x100;
  *(uint32_t*)0x20002e7c = 9;
  *(uint64_t*)0x20002e80 = 0x10;
  *(uint32_t*)0x20002e88 = 0;
  *(uint32_t*)0x20002e8c = 8;
  *(uint64_t*)0x20007068 = 0x50;
  *(uint32_t*)0x20007070 = 0;
  *(uint32_t*)0x20007078 = 0xedb1;
  *(uint64_t*)0x20007080 = 0;
  *(uint32_t*)0x20007088 = 0;
  *(uint64_t*)0x20007090 = 0x20004280;
  *(uint64_t*)0x20007098 = 0;
  *(uint64_t*)0x200070a0 = 0x200042c0;
  *(uint64_t*)0x200042c0 = 0x10;
  *(uint32_t*)0x200042c8 = 0x117;
  *(uint32_t*)0x200042cc = 0;
  *(uint64_t*)0x200042d0 = 0x10;
  *(uint32_t*)0x200042d8 = 0x108;
  *(uint32_t*)0x200042dc = 0;
  *(uint64_t*)0x200070a8 = 0x20;
  *(uint32_t*)0x200070b0 = 0x4000000;
  *(uint32_t*)0x200070b8 = 3;
  *(uint64_t*)0x200070c0 = 0x20004400;
  *(uint16_t*)0x20004400 = 2;
  *(uint16_t*)0x20004402 = htobe16(0x1f);
  *(uint8_t*)0x20004404 = 0xac;
  *(uint8_t*)0x20004405 = 0x14;
  *(uint8_t*)0x20004406 = 0x14;
  *(uint8_t*)0x20004407 = 0xbb;
  *(uint8_t*)0x20004408 = 0;
  *(uint8_t*)0x20004409 = 0;
  *(uint8_t*)0x2000440a = 0;
  *(uint8_t*)0x2000440b = 0;
  *(uint8_t*)0x2000440c = 0;
  *(uint8_t*)0x2000440d = 0;
  *(uint8_t*)0x2000440e = 0;
  *(uint8_t*)0x2000440f = 0;
  *(uint32_t*)0x200070c8 = 0x80;
  *(uint64_t*)0x200070d0 = 0x200045c0;
  *(uint64_t*)0x200045c0 = 0x20004480;
  *(uint64_t*)0x200045c8 = 0;
  *(uint64_t*)0x200045d0 = 0x20004540;
  *(uint64_t*)0x200045d8 = 0;
  *(uint64_t*)0x200070d8 = 2;
  *(uint64_t*)0x200070e0 = 0x20004600;
  *(uint64_t*)0x20004600 = 0x10;
  *(uint32_t*)0x20004608 = 0x189;
  *(uint32_t*)0x2000460c = 3;
  *(uint64_t*)0x20004610 = 0x10;
  *(uint32_t*)0x20004618 = 0x11e;
  *(uint32_t*)0x2000461c = 8;
  *(uint64_t*)0x20004620 = 0x10;
  *(uint32_t*)0x20004628 = 0x18c;
  *(uint32_t*)0x2000462c = 7;
  *(uint64_t*)0x20004630 = 0x10;
  *(uint32_t*)0x20004638 = 0xc5da6df6;
  *(uint32_t*)0x2000463c = 0x49c;
  *(uint64_t*)0x20004640 = 0x10;
  *(uint32_t*)0x20004648 = 0x11f;
  *(uint32_t*)0x2000464c = 9;
  *(uint64_t*)0x200070e8 = 0x50;
  *(uint32_t*)0x200070f0 = 0x20040000;
  *(uint32_t*)0x200070f8 = 0xffffff76;
  *(uint64_t*)0x20007100 = 0x20004880;
  *(uint16_t*)0x20004880 = 0x18;
  *(uint32_t*)0x20004882 = 0;
  *(uint16_t*)0x20004886 = 4;
  *(uint8_t*)0x20004888 = 0xaa;
  *(uint8_t*)0x20004889 = 0xaa;
  *(uint8_t*)0x2000488a = 0xaa;
  *(uint8_t*)0x2000488b = 0xaa;
  *(uint8_t*)0x2000488c = 0xaa;
  *(uint8_t*)0x2000488d = 0xaa;
  memcpy((void*)0x2000488e,
         "\x67\x72\x65\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  *(uint32_t*)0x20007108 = 0x80;
  *(uint64_t*)0x20007110 = 0x200049c0;
  *(uint64_t*)0x200049c0 = 0x20004900;
  *(uint64_t*)0x200049c8 = 0;
  *(uint64_t*)0x20007118 = 1;
  *(uint64_t*)0x20007120 = 0x20004a00;
  *(uint64_t*)0x20004a00 = 0x10;
  *(uint32_t*)0x20004a08 = 0x110;
  *(uint32_t*)0x20004a0c = 0x7ff;
  *(uint64_t*)0x20007128 = 0x10;
  *(uint32_t*)0x20007130 = 0x40;
  *(uint32_t*)0x20007138 = 2;
  *(uint64_t*)0x20007140 = 0;
  *(uint32_t*)0x20007148 = 0;
  *(uint64_t*)0x20007150 = 0x20004c80;
  *(uint64_t*)0x20004c80 = 0x20004a80;
  *(uint64_t*)0x20004c88 = 0;
  *(uint64_t*)0x20004c90 = 0x20004b80;
  *(uint64_t*)0x20004c98 = 0;
  *(uint64_t*)0x20007158 = 2;
  *(uint64_t*)0x20007160 = 0x20004cc0;
  *(uint64_t*)0x20004cc0 = 0x10;
  *(uint32_t*)0x20004cc8 = 0x10e;
  *(uint32_t*)0x20004ccc = 0x101;
  *(uint64_t*)0x20007168 = 0x10;
  *(uint32_t*)0x20007170 = 0x800;
  *(uint32_t*)0x20007178 = 0x40;
  *(uint64_t*)0x20007180 = 0x20004d00;
  *(uint16_t*)0x20004d00 = 0x18;
  *(uint32_t*)0x20004d02 = 1;
  *(uint32_t*)0x20004d06 = 0;
  *(uint32_t*)0x20004d0a = r[0];
  *(uint16_t*)0x20004d0e = 1;
  *(uint16_t*)0x20004d10 = 1;
  *(uint16_t*)0x20004d12 = 2;
  *(uint16_t*)0x20004d14 = 1;
  *(uint16_t*)0x20004d16 = 0xa;
  *(uint16_t*)0x20004d18 = htobe16(0x4e20);
  *(uint32_t*)0x20004d1a = 0;
  *(uint8_t*)0x20004d1e = 0;
  *(uint8_t*)0x20004d1f = 0;
  *(uint8_t*)0x20004d20 = 0;
  *(uint8_t*)0x20004d21 = 0;
  *(uint8_t*)0x20004d22 = 0;
  *(uint8_t*)0x20004d23 = 0;
  *(uint8_t*)0x20004d24 = 0;
  *(uint8_t*)0x20004d25 = 0;
  *(uint8_t*)0x20004d26 = 0;
  *(uint8_t*)0x20004d27 = 0;
  *(uint8_t*)0x20004d28 = 0;
  *(uint8_t*)0x20004d29 = 0;
  *(uint8_t*)0x20004d2a = 0;
  *(uint8_t*)0x20004d2b = 0;
  *(uint8_t*)0x20004d2c = 0;
  *(uint8_t*)0x20004d2d = 0;
  *(uint32_t*)0x20004d2e = 7;
  *(uint32_t*)0x20007188 = 0x80;
  *(uint64_t*)0x20007190 = 0x20006ec0;
  *(uint64_t*)0x20006ec0 = 0x20004d80;
  *(uint64_t*)0x20006ec8 = 0;
  *(uint64_t*)0x20006ed0 = 0x20004e40;
  *(uint64_t*)0x20006ed8 = 0;
  *(uint64_t*)0x20006ee0 = 0x20004ec0;
  *(uint64_t*)0x20006ee8 = 0;
  *(uint64_t*)0x20006ef0 = 0x20005ec0;
  *(uint64_t*)0x20006ef8 = 0;
  *(uint64_t*)0x20007198 = 4;
  *(uint64_t*)0x200071a0 = 0x20006f00;
  *(uint64_t*)0x200071a8 = 0;
  *(uint32_t*)0x200071b0 = 0x4004800;
  *(uint32_t*)0x200071b8 = 7;
  syscall(__NR_sendmmsg, r[0], 0x20006fc0, 8, 0x24008000);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
