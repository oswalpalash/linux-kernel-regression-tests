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
  *(uint64_t*)0x20008640 = 0x20000000;
  *(uint16_t*)0x20000000 = 0x1d;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x2000000c = 0;
  *(uint32_t*)0x20008648 = 0x80;
  *(uint64_t*)0x20008650 = 0x20000340;
  *(uint64_t*)0x20000340 = 0x20000080;
  *(uint64_t*)0x20000348 = 0;
  *(uint64_t*)0x20000350 = 0x20000140;
  *(uint64_t*)0x20000358 = 0;
  *(uint64_t*)0x20000360 = 0x20000240;
  *(uint64_t*)0x20000368 = 0;
  *(uint64_t*)0x20008658 = 3;
  *(uint64_t*)0x20008660 = 0x20000380;
  *(uint64_t*)0x20000380 = 0x10;
  *(uint32_t*)0x20000388 = 0;
  *(uint32_t*)0x2000038c = 0xc6;
  *(uint64_t*)0x20008668 = 0x10;
  *(uint32_t*)0x20008670 = 0x4000;
  *(uint32_t*)0x20008678 = 2;
  *(uint64_t*)0x20008680 = 0x200003c0;
  *(uint16_t*)0x200003c0 = 0;
  *(uint16_t*)0x200003c2 = 0;
  *(uint32_t*)0x200003c4 = 0;
  *(uint32_t*)0x200003c8 = 0;
  *(uint32_t*)0x20008688 = 0x80;
  *(uint64_t*)0x20008690 = 0x20000580;
  *(uint64_t*)0x20000580 = 0x20000440;
  *(uint64_t*)0x20000588 = 0;
  *(uint64_t*)0x20000590 = 0x20000540;
  *(uint64_t*)0x20000598 = 0;
  *(uint64_t*)0x20008698 = 2;
  *(uint64_t*)0x200086a0 = 0;
  *(uint64_t*)0x200086a8 = 0;
  *(uint32_t*)0x200086b0 = 4;
  *(uint32_t*)0x200086b8 = 1;
  *(uint64_t*)0x200086c0 = 0x200005c0;
  *(uint16_t*)0x200005c0 = 0x1b;
  memcpy((void*)0x200005c2,
         "\x19\x1f\x59\xf0\xef\xed\xf6\xe7\x25\xd4\xe6\x4f\x72\x30\xda\x62\x83"
         "\xa2\xdf\x27\x13\x6e\x01\x81\x96\x47\xc9\xd2\xd8\x57\x69\xe6\x1c\xa9"
         "\xb4\x84\xa4\x98\xe5\xb3\xe7\x37\xc0\x9e\xfd\xb1\x43\x88\x5a\xdc\xa9"
         "\x3b\x73\x24\x52\x29\xbf\xcf\xf2\x78\x16\x17\x7f\x2c\x77\x9d\x52\x1f"
         "\x76\xac\x3f\x2e\x01\x25\xff\x3d\x3e\x33\xb8\x5b\x89\xee\xd5\x2b\x07"
         "\x8c\x14\x01\xcb\xa1\x63\x3c\x90\x14\xf2\x85\x5e\xc3\x57\xcc\x7f\xc3"
         "\x47\x5c\xdd\x34\x1b\x67\x5d\x97\x45\x84\x74\x8f\xa1\x61\x25\xbf\xc6"
         "\x2c\x72\xfc\x80\xec\xc7\x36",
         126);
  *(uint32_t*)0x200086c8 = 0x80;
  *(uint64_t*)0x200086d0 = 0x200007c0;
  *(uint64_t*)0x200007c0 = 0x20000640;
  *(uint64_t*)0x200007c8 = 0;
  *(uint64_t*)0x200007d0 = 0x200006c0;
  *(uint64_t*)0x200007d8 = 0;
  *(uint64_t*)0x200086d8 = 2;
  *(uint64_t*)0x200086e0 = 0x20000800;
  *(uint64_t*)0x20000800 = 0x10;
  *(uint32_t*)0x20000808 = 0x11a;
  *(uint32_t*)0x2000080c = 1;
  *(uint64_t*)0x20000810 = 0x10;
  *(uint32_t*)0x20000818 = 0x10f;
  *(uint32_t*)0x2000081c = 0x472;
  *(uint64_t*)0x20000820 = 0x10;
  *(uint32_t*)0x20000828 = 0x110;
  *(uint32_t*)0x2000082c = 1;
  *(uint64_t*)0x20000830 = 0x10;
  *(uint32_t*)0x20000838 = 0x102;
  *(uint32_t*)0x2000083c = 0;
  *(uint64_t*)0x20000840 = 0x10;
  *(uint32_t*)0x20000848 = 0x12b;
  *(uint32_t*)0x2000084c = 0xaca2;
  *(uint64_t*)0x20000850 = 0x10;
  *(uint32_t*)0x20000858 = 0;
  *(uint32_t*)0x2000085c = 0x8e;
  *(uint64_t*)0x200086e8 = 0x60;
  *(uint32_t*)0x200086f0 = 0x8001;
  *(uint32_t*)0x200086f8 = 7;
  *(uint64_t*)0x20008700 = 0x20000c40;
  *(uint16_t*)0x20000c40 = 0x27;
  *(uint32_t*)0x20000c44 = 0;
  *(uint32_t*)0x20000c48 = 0;
  *(uint32_t*)0x20000c4c = 5;
  *(uint32_t*)0x20008708 = 0x80;
  *(uint64_t*)0x20008710 = 0x20001f80;
  *(uint64_t*)0x20001f80 = 0x20000cc0;
  *(uint64_t*)0x20001f88 = 0;
  *(uint64_t*)0x20001f90 = 0x20000dc0;
  *(uint64_t*)0x20001f98 = 0;
  *(uint64_t*)0x20001fa0 = 0x20000e40;
  *(uint64_t*)0x20001fa8 = 0;
  *(uint64_t*)0x20001fb0 = 0x20000ec0;
  *(uint64_t*)0x20001fb8 = 0;
  *(uint64_t*)0x20001fc0 = 0x20000f80;
  *(uint64_t*)0x20001fc8 = 0;
  *(uint64_t*)0x20008718 = 5;
  *(uint64_t*)0x20008720 = 0x20002000;
  *(uint64_t*)0x20002000 = 0x10;
  *(uint32_t*)0x20002008 = 0x117;
  *(uint32_t*)0x2000200c = 0xf1;
  *(uint64_t*)0x20002010 = 0x10;
  *(uint32_t*)0x20002018 = 0x111;
  *(uint32_t*)0x2000201c = 8;
  *(uint64_t*)0x20002020 = 0x10;
  *(uint32_t*)0x20002028 = 0x110;
  *(uint32_t*)0x2000202c = 0x3ff;
  *(uint64_t*)0x20002030 = 0x10;
  *(uint32_t*)0x20002038 = 6;
  *(uint32_t*)0x2000203c = 0xfffffffd;
  *(uint64_t*)0x20008728 = 0x40;
  *(uint32_t*)0x20008730 = 0;
  *(uint32_t*)0x20008738 = 9;
  *(uint64_t*)0x20008740 = 0x20003240;
  *(uint16_t*)0x20003240 = 0x26;
  memcpy((void*)0x20003242,
         "\x72\x6e\x67\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 14);
  *(uint32_t*)0x20003250 = 0;
  *(uint32_t*)0x20003254 = 0;
  memcpy((void*)0x20003258,
         "\x64\x72\x62\x67\x5f\x6e\x6f\x70\x72\x5f\x68\x6d\x61\x63\x5f\x73\x68"
         "\x61\x32\x35\x36\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         64);
  *(uint32_t*)0x20008748 = 0x80;
  *(uint64_t*)0x20008750 = 0x200034c0;
  *(uint64_t*)0x200034c0 = 0x200032c0;
  *(uint64_t*)0x200034c8 = 0;
  *(uint64_t*)0x200034d0 = 0x20003340;
  *(uint64_t*)0x200034d8 = 0;
  *(uint64_t*)0x200034e0 = 0x20003400;
  *(uint64_t*)0x200034e8 = 0;
  *(uint64_t*)0x200034f0 = 0x20003480;
  *(uint64_t*)0x200034f8 = 0;
  *(uint64_t*)0x20008758 = 4;
  *(uint64_t*)0x20008760 = 0x20003500;
  *(uint64_t*)0x20003500 = 0x10;
  *(uint32_t*)0x20003508 = 0x11;
  *(uint32_t*)0x2000350c = 0x1f;
  *(uint64_t*)0x20003510 = 0x10;
  *(uint32_t*)0x20003518 = 0x88;
  *(uint32_t*)0x2000351c = 2;
  *(uint64_t*)0x20003520 = 0x10;
  *(uint32_t*)0x20003528 = 0x1ff;
  *(uint32_t*)0x2000352c = 1;
  *(uint64_t*)0x20003530 = 0x10;
  *(uint32_t*)0x20003538 = 0x101;
  *(uint32_t*)0x2000353c = 0;
  *(uint64_t*)0x20008768 = 0x40;
  *(uint32_t*)0x20008770 = 0x20000000;
  *(uint32_t*)0x20008778 = 0;
  *(uint64_t*)0x20008780 = 0;
  *(uint32_t*)0x20008788 = 0;
  *(uint64_t*)0x20008790 = 0x20004e40;
  *(uint64_t*)0x20004e40 = 0x20004740;
  *(uint64_t*)0x20004e48 = 0;
  *(uint64_t*)0x20004e50 = 0x20004780;
  *(uint64_t*)0x20004e58 = 0;
  *(uint64_t*)0x20004e60 = 0x20004880;
  *(uint64_t*)0x20004e68 = 0;
  *(uint64_t*)0x20004e70 = 0x200048c0;
  *(uint64_t*)0x20004e78 = 0;
  *(uint64_t*)0x20004e80 = 0x200049c0;
  *(uint64_t*)0x20004e88 = 0;
  *(uint64_t*)0x20004e90 = 0x20004a80;
  *(uint64_t*)0x20004e98 = 0;
  *(uint64_t*)0x20004ea0 = 0x20004b40;
  *(uint64_t*)0x20004ea8 = 0;
  *(uint64_t*)0x20004eb0 = 0x20004bc0;
  *(uint64_t*)0x20004eb8 = 0;
  *(uint64_t*)0x20004ec0 = 0x20004c80;
  *(uint64_t*)0x20004ec8 = 0;
  *(uint64_t*)0x20004ed0 = 0x20004d40;
  *(uint64_t*)0x20004ed8 = 0;
  *(uint64_t*)0x20008798 = 0xa;
  *(uint64_t*)0x200087a0 = 0x20004f00;
  *(uint64_t*)0x20004f00 = 0x10;
  *(uint32_t*)0x20004f08 = 0;
  *(uint32_t*)0x20004f0c = 4;
  *(uint64_t*)0x20004f10 = 0x10;
  *(uint32_t*)0x20004f18 = 0x13b;
  *(uint32_t*)0x20004f1c = 0x100;
  *(uint64_t*)0x20004f20 = 0x10;
  *(uint32_t*)0x20004f28 = 0x117;
  *(uint32_t*)0x20004f2c = 6;
  *(uint64_t*)0x20004f30 = 0x10;
  *(uint32_t*)0x20004f38 = 0x11f;
  *(uint32_t*)0x20004f3c = 8;
  *(uint64_t*)0x20004f40 = 0x10;
  *(uint32_t*)0x20004f48 = 0x10f;
  *(uint32_t*)0x20004f4c = 8;
  *(uint64_t*)0x20004f50 = 0x10;
  *(uint32_t*)0x20004f58 = 0x11c;
  *(uint32_t*)0x20004f5c = 0xfff;
  *(uint64_t*)0x20004f60 = 0x10;
  *(uint32_t*)0x20004f68 = 0x113;
  *(uint32_t*)0x20004f6c = 0x10001;
  *(uint64_t*)0x200087a8 = 0x70;
  *(uint32_t*)0x200087b0 = 1;
  *(uint32_t*)0x200087b8 = 9;
  *(uint64_t*)0x200087c0 = 0;
  *(uint32_t*)0x200087c8 = 0;
  *(uint64_t*)0x200087d0 = 0x200085c0;
  *(uint64_t*)0x200085c0 = 0x20006240;
  *(uint64_t*)0x200085c8 = 0;
  *(uint64_t*)0x200085d0 = 0x20006300;
  *(uint64_t*)0x200085d8 = 0;
  *(uint64_t*)0x200085e0 = 0x20006400;
  *(uint64_t*)0x200085e8 = 0;
  *(uint64_t*)0x200085f0 = 0x200064c0;
  *(uint64_t*)0x200085f8 = 0;
  *(uint64_t*)0x20008600 = 0x200074c0;
  *(uint64_t*)0x20008608 = 0;
  *(uint64_t*)0x20008610 = 0x200075c0;
  *(uint64_t*)0x20008618 = 0;
  *(uint64_t*)0x200087d8 = 6;
  *(uint64_t*)0x200087e0 = 0;
  *(uint64_t*)0x200087e8 = 0;
  *(uint32_t*)0x200087f0 = 0x24000000;
  *(uint32_t*)0x200087f8 = 0x1ff;
  syscall(__NR_sendmmsg, r[0], 0x20008640, 7, 0x20000000);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
