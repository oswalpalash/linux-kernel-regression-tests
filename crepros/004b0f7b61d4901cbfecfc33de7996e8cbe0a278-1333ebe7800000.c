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
  *(uint64_t*)0x20006700 = 0;
  *(uint32_t*)0x20006708 = 0;
  *(uint64_t*)0x20006710 = 0x200013c0;
  *(uint64_t*)0x200013c0 = 0x20000080;
  *(uint64_t*)0x200013c8 = 0;
  *(uint64_t*)0x200013d0 = 0x20000000;
  *(uint64_t*)0x200013d8 = 0;
  *(uint64_t*)0x200013e0 = 0x20000180;
  *(uint64_t*)0x200013e8 = 0;
  *(uint64_t*)0x200013f0 = 0x200001c0;
  *(uint64_t*)0x200013f8 = 0;
  *(uint64_t*)0x20001400 = 0x20000200;
  *(uint64_t*)0x20001408 = 0;
  *(uint64_t*)0x20001410 = 0x20000300;
  *(uint64_t*)0x20001418 = 0;
  *(uint64_t*)0x20001420 = 0x20001300;
  *(uint64_t*)0x20001428 = 0;
  *(uint64_t*)0x20001430 = 0x20001340;
  *(uint64_t*)0x20001438 = 0;
  *(uint64_t*)0x20006718 = 8;
  *(uint64_t*)0x20006720 = 0x20001440;
  *(uint64_t*)0x20006728 = 0;
  *(uint32_t*)0x20006730 = 0x4000;
  *(uint32_t*)0x20006738 = 6;
  *(uint64_t*)0x20006740 = 0;
  *(uint32_t*)0x20006748 = 0;
  *(uint64_t*)0x20006750 = 0x200027c0;
  *(uint64_t*)0x200027c0 = 0x20002540;
  *(uint64_t*)0x200027c8 = 0;
  *(uint64_t*)0x200027d0 = 0x20002640;
  *(uint64_t*)0x200027d8 = 0;
  *(uint64_t*)0x200027e0 = 0x20002740;
  *(uint64_t*)0x200027e8 = 0;
  *(uint64_t*)0x20006758 = 3;
  *(uint64_t*)0x20006760 = 0;
  *(uint64_t*)0x20006768 = 0;
  *(uint32_t*)0x20006770 = 0x4000010;
  *(uint32_t*)0x20006778 = 0xe0;
  *(uint64_t*)0x20006780 = 0x20002800;
  *(uint16_t*)0x20002800 = 0x1f;
  *(uint8_t*)0x20002802 = 1;
  *(uint8_t*)0x20002803 = 1;
  *(uint8_t*)0x20002804 = 6;
  *(uint8_t*)0x20002805 = 0x1f;
  *(uint8_t*)0x20002806 = 6;
  *(uint8_t*)0x20002807 = 7;
  *(uint32_t*)0x20006788 = 0x80;
  *(uint64_t*)0x20006790 = 0x20002bc0;
  *(uint64_t*)0x20002bc0 = 0x20002880;
  *(uint64_t*)0x20002bc8 = 0;
  *(uint64_t*)0x20002bd0 = 0x200028c0;
  *(uint64_t*)0x20002bd8 = 0;
  *(uint64_t*)0x20002be0 = 0x200029c0;
  *(uint64_t*)0x20002be8 = 0;
  *(uint64_t*)0x20002bf0 = 0x20002a80;
  *(uint64_t*)0x20002bf8 = 0;
  *(uint64_t*)0x20002c00 = 0x20002ac0;
  *(uint64_t*)0x20002c08 = 0;
  *(uint64_t*)0x20002c10 = 0x20002b00;
  *(uint64_t*)0x20002c18 = 0;
  *(uint64_t*)0x20006798 = 6;
  *(uint64_t*)0x200067a0 = 0;
  *(uint64_t*)0x200067a8 = 0;
  *(uint32_t*)0x200067b0 = 0x800;
  *(uint32_t*)0x200067b8 = 3;
  *(uint64_t*)0x200067c0 = 0x20002d80;
  *(uint16_t*)0x20002d80 = 0x1d;
  *(uint32_t*)0x20002d84 = 0;
  *(uint32_t*)0x20002d88 = 0;
  *(uint32_t*)0x20002d8c = 0;
  *(uint32_t*)0x200067c8 = 0x80;
  *(uint64_t*)0x200067d0 = 0x20002f00;
  *(uint64_t*)0x20002f00 = 0x20002e00;
  *(uint64_t*)0x20002f08 = 0;
  *(uint64_t*)0x20002f10 = 0x20002ec0;
  *(uint64_t*)0x20002f18 = 0;
  *(uint64_t*)0x200067d8 = 2;
  *(uint64_t*)0x200067e0 = 0x20002f40;
  *(uint64_t*)0x20002f40 = 0x10;
  *(uint32_t*)0x20002f48 = 0x118;
  *(uint32_t*)0x20002f4c = 5;
  *(uint64_t*)0x20002f50 = 0x10;
  *(uint32_t*)0x20002f58 = 1;
  *(uint32_t*)0x20002f5c = 3;
  *(uint64_t*)0x200067e8 = 0x20;
  *(uint32_t*)0x200067f0 = 0x200448d0;
  *(uint32_t*)0x200067f8 = 5;
  *(uint64_t*)0x20006800 = 0;
  *(uint32_t*)0x20006808 = 0;
  *(uint64_t*)0x20006810 = 0x20003440;
  *(uint64_t*)0x20003440 = 0x20003100;
  *(uint64_t*)0x20003448 = 0;
  *(uint64_t*)0x20003450 = 0x20003180;
  *(uint64_t*)0x20003458 = 0;
  *(uint64_t*)0x20003460 = 0x20003280;
  *(uint64_t*)0x20003468 = 0;
  *(uint64_t*)0x20003470 = 0x20003340;
  *(uint64_t*)0x20003478 = 0;
  *(uint64_t*)0x20006818 = 4;
  *(uint64_t*)0x20006820 = 0;
  *(uint64_t*)0x20006828 = 0;
  *(uint32_t*)0x20006830 = 0x48010;
  *(uint32_t*)0x20006838 = 4;
  *(uint64_t*)0x20006840 = 0x20003480;
  *(uint16_t*)0x20003480 = 0x27;
  *(uint32_t*)0x20003484 = 1;
  *(uint32_t*)0x20003488 = 0;
  *(uint32_t*)0x2000348c = 3;
  *(uint32_t*)0x20006848 = 0x80;
  *(uint64_t*)0x20006850 = 0x200047c0;
  *(uint64_t*)0x200047c0 = 0x20003500;
  *(uint64_t*)0x200047c8 = 0;
  *(uint64_t*)0x200047d0 = 0x200035c0;
  *(uint64_t*)0x200047d8 = 0;
  *(uint64_t*)0x200047e0 = 0x200045c0;
  *(uint64_t*)0x200047e8 = 0;
  *(uint64_t*)0x200047f0 = 0x20004600;
  *(uint64_t*)0x200047f8 = 0;
  *(uint64_t*)0x20004800 = 0x200046c0;
  *(uint64_t*)0x20004808 = 0;
  *(uint64_t*)0x20006858 = 5;
  *(uint64_t*)0x20006860 = 0;
  *(uint64_t*)0x20006868 = 0;
  *(uint32_t*)0x20006870 = 0x48000;
  *(uint32_t*)0x20006878 = 0x1f;
  *(uint64_t*)0x20006880 = 0x20004840;
  *(uint16_t*)0x20004840 = 4;
  *(uint16_t*)0x20004842 = htobe16(0xccd);
  *(uint32_t*)0x20004844 = htobe32(7);
  memcpy((void*)0x20004848, "\xcb\x23\x5d\xda\x61\xb6", 6);
  *(uint8_t*)0x2000484e = 4;
  *(uint8_t*)0x2000484f = 0;
  *(uint32_t*)0x20006888 = 0x80;
  *(uint64_t*)0x20006890 = 0x200049c0;
  *(uint64_t*)0x200049c0 = 0x200048c0;
  *(uint64_t*)0x200049c8 = 0;
  *(uint64_t*)0x200049d0 = 0x20004940;
  *(uint64_t*)0x200049d8 = 0;
  *(uint64_t*)0x20006898 = 2;
  *(uint64_t*)0x200068a0 = 0x20004a00;
  *(uint64_t*)0x20004a00 = 0x10;
  *(uint32_t*)0x20004a08 = 0x10b;
  *(uint32_t*)0x20004a0c = 0x400;
  *(uint64_t*)0x20004a10 = 0x10;
  *(uint32_t*)0x20004a18 = 0x10a;
  *(uint32_t*)0x20004a1c = 0;
  *(uint64_t*)0x20004a20 = 0x10;
  *(uint32_t*)0x20004a28 = 0x107;
  *(uint32_t*)0x20004a2c = 6;
  *(uint64_t*)0x20004a30 = 0x10;
  *(uint32_t*)0x20004a38 = 0x11f;
  *(uint32_t*)0x20004a3c = 5;
  *(uint64_t*)0x20004a40 = 0x10;
  *(uint32_t*)0x20004a48 = 1;
  *(uint32_t*)0x20004a4c = 0;
  *(uint64_t*)0x20004a50 = 0x10;
  *(uint32_t*)0x20004a58 = 0x108;
  *(uint32_t*)0x20004a5c = 0xa4e;
  *(uint64_t*)0x20004a60 = 0x10;
  *(uint32_t*)0x20004a68 = 0;
  *(uint32_t*)0x20004a6c = 8;
  *(uint64_t*)0x20004a70 = 0x10;
  *(uint32_t*)0x20004a78 = 0x10d;
  *(uint32_t*)0x20004a7c = 6;
  *(uint64_t*)0x20004a80 = 0x10;
  *(uint32_t*)0x20004a88 = 0xfc174caa;
  *(uint32_t*)0x20004a8c = 1;
  *(uint64_t*)0x200068a8 = 0x90;
  *(uint32_t*)0x200068b0 = 1;
  *(uint32_t*)0x200068b8 = 5;
  *(uint64_t*)0x200068c0 = 0x20004e80;
  *(uint16_t*)0x20004e80 = 4;
  *(uint16_t*)0x20004e82 = htobe16(1);
  *(uint32_t*)0x20004e84 = htobe32(6);
  memcpy((void*)0x20004e88, "\x90\x52\x2f\xf2\x43\x5b", 6);
  *(uint8_t*)0x20004e8e = 0;
  *(uint8_t*)0x20004e8f = 0;
  *(uint32_t*)0x200068c8 = 0x80;
  *(uint64_t*)0x200068d0 = 0x20006200;
  *(uint64_t*)0x20006200 = 0x20004f00;
  *(uint64_t*)0x20006208 = 0;
  *(uint64_t*)0x20006210 = 0x20004f80;
  *(uint64_t*)0x20006218 = 0;
  *(uint64_t*)0x20006220 = 0x20005080;
  *(uint64_t*)0x20006228 = 0;
  *(uint64_t*)0x20006230 = 0x200050c0;
  *(uint64_t*)0x20006238 = 0;
  *(uint64_t*)0x20006240 = 0x200060c0;
  *(uint64_t*)0x20006248 = 0;
  *(uint64_t*)0x20006250 = 0x20006100;
  *(uint64_t*)0x20006258 = 0;
  *(uint64_t*)0x200068d8 = 6;
  *(uint64_t*)0x200068e0 = 0x20006280;
  *(uint64_t*)0x20006280 = 0x10;
  *(uint32_t*)0x20006288 = 0x11b;
  *(uint32_t*)0x2000628c = 0x7fff;
  *(uint64_t*)0x20006290 = 0x10;
  *(uint32_t*)0x20006298 = 0x11e;
  *(uint32_t*)0x2000629c = 0xd6;
  *(uint64_t*)0x200062a0 = 0x10;
  *(uint32_t*)0x200062a8 = 0x110;
  *(uint32_t*)0x200062ac = 0xfffffffb;
  *(uint64_t*)0x200062b0 = 0x10;
  *(uint32_t*)0x200062b8 = 0x116;
  *(uint32_t*)0x200062bc = 0xffff0000;
  *(uint64_t*)0x200062c0 = 0x10;
  *(uint32_t*)0x200062c8 = 0x115;
  *(uint32_t*)0x200062cc = 0xd56b;
  *(uint64_t*)0x200062d0 = 0x10;
  *(uint32_t*)0x200062d8 = 0x10c;
  *(uint32_t*)0x200062dc = 3;
  *(uint64_t*)0x200062e0 = 0x10;
  *(uint32_t*)0x200062e8 = 0x1ff;
  *(uint32_t*)0x200062ec = 4;
  *(uint64_t*)0x200062f0 = 0x10;
  *(uint32_t*)0x200062f8 = 0x118;
  *(uint32_t*)0x200062fc = 0;
  *(uint64_t*)0x20006300 = 0x10;
  *(uint32_t*)0x20006308 = 0x88;
  *(uint32_t*)0x2000630c = 4;
  *(uint64_t*)0x200068e8 = 0x90;
  *(uint32_t*)0x200068f0 = 0x40880;
  *(uint32_t*)0x200068f8 = 0x80000001;
  syscall(__NR_sendmmsg, r[0], 0x20006700, 8, 0x20000000);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
