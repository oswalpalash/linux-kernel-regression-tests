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
  *(uint64_t*)0x20005580 = 0x20000100;
  *(uint16_t*)0x20000100 = 4;
  *(uint16_t*)0x20000102 = htobe16(0x1000);
  *(uint32_t*)0x20000104 = htobe32(0x10001);
  memcpy((void*)0x20000108, "\x8d\x78\x80\xb2\xa7\xfc", 6);
  *(uint8_t*)0x2000010e = 0x22;
  *(uint8_t*)0x2000010f = 0;
  *(uint32_t*)0x20005588 = 0x80;
  *(uint64_t*)0x20005590 = 0x20000180;
  *(uint64_t*)0x20005598 = 0;
  *(uint64_t*)0x200055a0 = 0;
  *(uint64_t*)0x200055a8 = 0;
  *(uint32_t*)0x200055b0 = 0x80;
  *(uint32_t*)0x200055b8 = 6;
  *(uint64_t*)0x200055c0 = 0x200001c0;
  *(uint16_t*)0x200001c0 = 0x27;
  *(uint32_t*)0x200001c4 = 0;
  *(uint32_t*)0x200001c8 = 2;
  *(uint32_t*)0x200001cc = 7;
  *(uint32_t*)0x200055c8 = 0x80;
  *(uint64_t*)0x200055d0 = 0x200006c0;
  *(uint64_t*)0x200006c0 = 0x20000240;
  *(uint64_t*)0x200006c8 = 0;
  *(uint64_t*)0x200006d0 = 0x20000280;
  *(uint64_t*)0x200006d8 = 0;
  *(uint64_t*)0x200006e0 = 0x20000300;
  *(uint64_t*)0x200006e8 = 0;
  *(uint64_t*)0x200006f0 = 0x20000400;
  *(uint64_t*)0x200006f8 = 0;
  *(uint64_t*)0x20000700 = 0x200004c0;
  *(uint64_t*)0x20000708 = 0;
  *(uint64_t*)0x20000710 = 0x20000580;
  *(uint64_t*)0x20000718 = 0;
  *(uint64_t*)0x20000720 = 0x20000680;
  *(uint64_t*)0x20000728 = 0;
  *(uint64_t*)0x200055d8 = 7;
  *(uint64_t*)0x200055e0 = 0;
  *(uint64_t*)0x200055e8 = 0;
  *(uint32_t*)0x200055f0 = 0x40;
  *(uint32_t*)0x200055f8 = 6;
  *(uint64_t*)0x20005600 = 0;
  *(uint32_t*)0x20005608 = 0;
  *(uint64_t*)0x20005610 = 0x20001940;
  *(uint64_t*)0x20001940 = 0x20000740;
  *(uint64_t*)0x20001948 = 0;
  *(uint64_t*)0x20001950 = 0x20000840;
  *(uint64_t*)0x20001958 = 0;
  *(uint64_t*)0x20001960 = 0x200008c0;
  *(uint64_t*)0x20001968 = 0;
  *(uint64_t*)0x20001970 = 0x20000940;
  *(uint64_t*)0x20001978 = 0;
  *(uint64_t*)0x20005618 = 4;
  *(uint64_t*)0x20005620 = 0x20001980;
  *(uint64_t*)0x20001980 = 0x10;
  *(uint32_t*)0x20001988 = 0x11f;
  *(uint32_t*)0x2000198c = 0xff;
  *(uint64_t*)0x20001990 = 0x10;
  *(uint32_t*)0x20001998 = 0x1ff;
  *(uint32_t*)0x2000199c = 7;
  *(uint64_t*)0x200019a0 = 0x10;
  *(uint32_t*)0x200019a8 = 0x117;
  *(uint32_t*)0x200019ac = 0x3a24b7d4;
  *(uint64_t*)0x200019b0 = 0x10;
  *(uint32_t*)0x200019b8 = 0x117;
  *(uint32_t*)0x200019bc = 2;
  *(uint64_t*)0x200019c0 = 0x10;
  *(uint32_t*)0x200019c8 = 0x100;
  *(uint32_t*)0x200019cc = 7;
  *(uint64_t*)0x200019d0 = 0x10;
  *(uint32_t*)0x200019d8 = 0x13f;
  *(uint32_t*)0x200019dc = 0x400;
  *(uint64_t*)0x200019e0 = 0x10;
  *(uint32_t*)0x200019e8 = 0x102;
  *(uint32_t*)0x200019ec = 0xffffffc0;
  *(uint64_t*)0x20005628 = 0x70;
  *(uint32_t*)0x20005630 = 0x4014;
  *(uint32_t*)0x20005638 = 1;
  *(uint64_t*)0x20005640 = 0;
  *(uint32_t*)0x20005648 = 0;
  *(uint64_t*)0x20005650 = 0x20002f00;
  *(uint64_t*)0x20002f00 = 0x20002c00;
  *(uint64_t*)0x20002f08 = 0;
  *(uint64_t*)0x20002f10 = 0x20002d00;
  *(uint64_t*)0x20002f18 = 0;
  *(uint64_t*)0x20002f20 = 0x20002e00;
  *(uint64_t*)0x20002f28 = 0;
  *(uint64_t*)0x20002f30 = 0x20002e80;
  *(uint64_t*)0x20002f38 = 0;
  *(uint64_t*)0x20005658 = 4;
  *(uint64_t*)0x20005660 = 0x20002f40;
  *(uint64_t*)0x20002f40 = 0x10;
  *(uint32_t*)0x20002f48 = 0x117;
  *(uint32_t*)0x20002f4c = 0x8a;
  *(uint64_t*)0x20005668 = 0x10;
  *(uint32_t*)0x20005670 = 0x800;
  *(uint32_t*)0x20005678 = 2;
  *(uint64_t*)0x20005680 = 0x20003000;
  *(uint16_t*)0x20003000 = 3;
  memcpy((void*)0x20003002, "\xbd\x32\x63\xf8\x3e\xbd\x6a", 7);
  *(uint32_t*)0x2000300c = 1;
  *(uint32_t*)0x20005688 = 0x80;
  *(uint64_t*)0x20005690 = 0x20003400;
  *(uint64_t*)0x20003400 = 0x20003080;
  *(uint64_t*)0x20003408 = 0;
  *(uint64_t*)0x20003410 = 0x20003180;
  *(uint64_t*)0x20003418 = 0;
  *(uint64_t*)0x20003420 = 0x20003240;
  *(uint64_t*)0x20003428 = 0;
  *(uint64_t*)0x20003430 = 0x20003340;
  *(uint64_t*)0x20003438 = 0;
  *(uint64_t*)0x20003440 = 0x20003380;
  *(uint64_t*)0x20003448 = 0;
  *(uint64_t*)0x20003450 = 0x200033c0;
  *(uint64_t*)0x20003458 = 0;
  *(uint64_t*)0x20005698 = 6;
  *(uint64_t*)0x200056a0 = 0;
  *(uint64_t*)0x200056a8 = 0;
  *(uint32_t*)0x200056b0 = 0x4008001;
  *(uint32_t*)0x200056b8 = 8;
  *(uint64_t*)0x200056c0 = 0x20003480;
  *(uint16_t*)0x20003480 = 0x18;
  *(uint32_t*)0x20003482 = 1;
  *(uint32_t*)0x20003486 = 0;
  *(uint32_t*)0x2000348a = r[0];
  *(uint16_t*)0x2000348e = 4;
  *(uint16_t*)0x20003490 = 1;
  *(uint16_t*)0x20003492 = 0;
  *(uint16_t*)0x20003494 = 4;
  *(uint16_t*)0x20003496 = 0xa;
  *(uint16_t*)0x20003498 = htobe16(9);
  *(uint32_t*)0x2000349a = 0;
  *(uint8_t*)0x2000349e = 0xfe;
  *(uint8_t*)0x2000349f = 0x80;
  *(uint8_t*)0x200034a0 = 0;
  *(uint8_t*)0x200034a1 = 0;
  *(uint8_t*)0x200034a2 = 0;
  *(uint8_t*)0x200034a3 = 0;
  *(uint8_t*)0x200034a4 = 0;
  *(uint8_t*)0x200034a5 = 0;
  *(uint8_t*)0x200034a6 = 0;
  *(uint8_t*)0x200034a7 = 0;
  *(uint8_t*)0x200034a8 = 0;
  *(uint8_t*)0x200034a9 = 0;
  *(uint8_t*)0x200034aa = 0;
  *(uint8_t*)0x200034ab = 0;
  *(uint8_t*)0x200034ac = 0;
  *(uint8_t*)0x200034ad = 0xbb;
  *(uint32_t*)0x200034ae = 8;
  *(uint32_t*)0x200056c8 = 0x80;
  *(uint64_t*)0x200056d0 = 0x20004500;
  *(uint64_t*)0x20004500 = 0x20003500;
  *(uint64_t*)0x20004508 = 0;
  *(uint64_t*)0x200056d8 = 1;
  *(uint64_t*)0x200056e0 = 0x20004540;
  *(uint64_t*)0x20004540 = 0x10;
  *(uint32_t*)0x20004548 = 0x116;
  *(uint32_t*)0x2000454c = 0x6365;
  *(uint64_t*)0x200056e8 = 0x10;
  *(uint32_t*)0x200056f0 = 0x2004c881;
  *(uint32_t*)0x200056f8 = 0x1ff;
  syscall(__NR_sendmmsg, r[0], 0x20005580, 6, 0x20000800);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
