// https://syzkaller.appspot.com/bug?id=43ea073f71f9a4fc875712c0c52bed34dc201314
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x20000000, "/dev/uinput", 12);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000000, 0x805, 0);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20000040 = 0x2f;
  *(uint32_t*)0x20000044 = 0;
  *(uint32_t*)0x20000048 = 0;
  *(uint32_t*)0x2000004c = 0;
  *(uint32_t*)0x20000050 = 0;
  *(uint32_t*)0x20000054 = 0;
  *(uint32_t*)0x20000058 = 0;
  syscall(__NR_ioctl, r[0], 0x401c5504, 0x20000040);
  memcpy((void*)0x200000c0,
         "\x73\x79\x7a\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         80);
  *(uint16_t*)0x20000110 = 0;
  *(uint16_t*)0x20000112 = 0;
  *(uint16_t*)0x20000114 = 0;
  *(uint16_t*)0x20000116 = 0;
  *(uint32_t*)0x20000118 = 0;
  *(uint32_t*)0x2000011c = 0;
  *(uint32_t*)0x20000120 = 0;
  *(uint32_t*)0x20000124 = 0;
  *(uint32_t*)0x20000128 = 0;
  *(uint32_t*)0x2000012c = 0;
  *(uint32_t*)0x20000130 = 0;
  *(uint32_t*)0x20000134 = 0;
  *(uint32_t*)0x20000138 = 0;
  *(uint32_t*)0x2000013c = 0;
  *(uint32_t*)0x20000140 = 0;
  *(uint32_t*)0x20000144 = 0;
  *(uint32_t*)0x20000148 = 0;
  *(uint32_t*)0x2000014c = 0;
  *(uint32_t*)0x20000150 = 0;
  *(uint32_t*)0x20000154 = 0;
  *(uint32_t*)0x20000158 = 0;
  *(uint32_t*)0x2000015c = 0;
  *(uint32_t*)0x20000160 = 0;
  *(uint32_t*)0x20000164 = 0;
  *(uint32_t*)0x20000168 = 0;
  *(uint32_t*)0x2000016c = 0;
  *(uint32_t*)0x20000170 = 0;
  *(uint32_t*)0x20000174 = 0;
  *(uint32_t*)0x20000178 = 0;
  *(uint32_t*)0x2000017c = 0;
  *(uint32_t*)0x20000180 = 0;
  *(uint32_t*)0x20000184 = 0;
  *(uint32_t*)0x20000188 = 0;
  *(uint32_t*)0x2000018c = 0;
  *(uint32_t*)0x20000190 = 0;
  *(uint32_t*)0x20000194 = 0;
  *(uint32_t*)0x20000198 = 0;
  *(uint32_t*)0x2000019c = 0;
  *(uint32_t*)0x200001a0 = 0;
  *(uint32_t*)0x200001a4 = 0;
  *(uint32_t*)0x200001a8 = 0;
  *(uint32_t*)0x200001ac = 0;
  *(uint32_t*)0x200001b0 = 0;
  *(uint32_t*)0x200001b4 = 0;
  *(uint32_t*)0x200001b8 = 0;
  *(uint32_t*)0x200001bc = 0;
  *(uint32_t*)0x200001c0 = 0;
  *(uint32_t*)0x200001c4 = 0;
  *(uint32_t*)0x200001c8 = 0;
  *(uint32_t*)0x200001cc = 0;
  *(uint32_t*)0x200001d0 = 0;
  *(uint32_t*)0x200001d4 = 0;
  *(uint32_t*)0x200001d8 = 0;
  *(uint32_t*)0x200001dc = 0;
  *(uint32_t*)0x200001e0 = 0;
  *(uint32_t*)0x200001e4 = 0;
  *(uint32_t*)0x200001e8 = 0;
  *(uint32_t*)0x200001ec = 0;
  *(uint32_t*)0x200001f0 = 0;
  *(uint32_t*)0x200001f4 = 0;
  *(uint32_t*)0x200001f8 = 0;
  *(uint32_t*)0x200001fc = 0;
  *(uint32_t*)0x20000200 = 0;
  *(uint32_t*)0x20000204 = 0;
  *(uint32_t*)0x20000208 = 0;
  *(uint32_t*)0x2000020c = 0;
  *(uint32_t*)0x20000210 = 0;
  *(uint32_t*)0x20000214 = 0;
  *(uint32_t*)0x20000218 = 0;
  *(uint32_t*)0x2000021c = 0;
  *(uint32_t*)0x20000220 = 0;
  *(uint32_t*)0x20000224 = 0;
  *(uint32_t*)0x20000228 = 0;
  *(uint32_t*)0x2000022c = 0;
  *(uint32_t*)0x20000230 = 0;
  *(uint32_t*)0x20000234 = 0;
  *(uint32_t*)0x20000238 = 0;
  *(uint32_t*)0x2000023c = 0;
  *(uint32_t*)0x20000240 = 0;
  *(uint32_t*)0x20000244 = 0;
  *(uint32_t*)0x20000248 = 0;
  *(uint32_t*)0x2000024c = 0;
  *(uint32_t*)0x20000250 = 0;
  *(uint32_t*)0x20000254 = 0;
  *(uint32_t*)0x20000258 = 0;
  *(uint32_t*)0x2000025c = 0;
  *(uint32_t*)0x20000260 = 0;
  *(uint32_t*)0x20000264 = 0;
  *(uint32_t*)0x20000268 = 0;
  *(uint32_t*)0x2000026c = 0;
  *(uint32_t*)0x20000270 = 0;
  *(uint32_t*)0x20000274 = 0;
  *(uint32_t*)0x20000278 = 0;
  *(uint32_t*)0x2000027c = 0;
  *(uint32_t*)0x20000280 = 0;
  *(uint32_t*)0x20000284 = 0;
  *(uint32_t*)0x20000288 = 0;
  *(uint32_t*)0x2000028c = 0;
  *(uint32_t*)0x20000290 = 0;
  *(uint32_t*)0x20000294 = 0;
  *(uint32_t*)0x20000298 = 0;
  *(uint32_t*)0x2000029c = 0;
  *(uint32_t*)0x200002a0 = 0;
  *(uint32_t*)0x200002a4 = 0;
  *(uint32_t*)0x200002a8 = 0;
  *(uint32_t*)0x200002ac = 0;
  *(uint32_t*)0x200002b0 = 0;
  *(uint32_t*)0x200002b4 = 0;
  *(uint32_t*)0x200002b8 = 0;
  *(uint32_t*)0x200002bc = 0;
  *(uint32_t*)0x200002c0 = 0;
  *(uint32_t*)0x200002c4 = 0;
  *(uint32_t*)0x200002c8 = 0;
  *(uint32_t*)0x200002cc = 0;
  *(uint32_t*)0x200002d0 = 0;
  *(uint32_t*)0x200002d4 = 0;
  *(uint32_t*)0x200002d8 = 0;
  *(uint32_t*)0x200002dc = 0;
  *(uint32_t*)0x200002e0 = 0;
  *(uint32_t*)0x200002e4 = 0;
  *(uint32_t*)0x200002e8 = 0;
  *(uint32_t*)0x200002ec = 0;
  *(uint32_t*)0x200002f0 = 0;
  *(uint32_t*)0x200002f4 = 0;
  *(uint32_t*)0x200002f8 = 0;
  *(uint32_t*)0x200002fc = 0;
  *(uint32_t*)0x20000300 = 0;
  *(uint32_t*)0x20000304 = 0;
  *(uint32_t*)0x20000308 = 0;
  *(uint32_t*)0x2000030c = 0;
  *(uint32_t*)0x20000310 = 0;
  *(uint32_t*)0x20000314 = 0;
  *(uint32_t*)0x20000318 = 0;
  *(uint32_t*)0x2000031c = 0;
  *(uint32_t*)0x20000320 = 0;
  *(uint32_t*)0x20000324 = 0;
  *(uint32_t*)0x20000328 = 0;
  *(uint32_t*)0x2000032c = 0;
  *(uint32_t*)0x20000330 = 0;
  *(uint32_t*)0x20000334 = 0;
  *(uint32_t*)0x20000338 = 0;
  *(uint32_t*)0x2000033c = 0;
  *(uint32_t*)0x20000340 = 0;
  *(uint32_t*)0x20000344 = 0;
  *(uint32_t*)0x20000348 = 0;
  *(uint32_t*)0x2000034c = 0;
  *(uint32_t*)0x20000350 = 0;
  *(uint32_t*)0x20000354 = 0;
  *(uint32_t*)0x20000358 = 0;
  *(uint32_t*)0x2000035c = 0;
  *(uint32_t*)0x20000360 = 0;
  *(uint32_t*)0x20000364 = 0;
  *(uint32_t*)0x20000368 = 0;
  *(uint32_t*)0x2000036c = 0;
  *(uint32_t*)0x20000370 = 0;
  *(uint32_t*)0x20000374 = 0;
  *(uint32_t*)0x20000378 = 0;
  *(uint32_t*)0x2000037c = 0;
  *(uint32_t*)0x20000380 = 0;
  *(uint32_t*)0x20000384 = 0;
  *(uint32_t*)0x20000388 = 0;
  *(uint32_t*)0x2000038c = 0;
  *(uint32_t*)0x20000390 = 0;
  *(uint32_t*)0x20000394 = 0;
  *(uint32_t*)0x20000398 = 0;
  *(uint32_t*)0x2000039c = 0;
  *(uint32_t*)0x200003a0 = 0;
  *(uint32_t*)0x200003a4 = 0;
  *(uint32_t*)0x200003a8 = 0;
  *(uint32_t*)0x200003ac = 0;
  *(uint32_t*)0x200003b0 = 0;
  *(uint32_t*)0x200003b4 = 0;
  *(uint32_t*)0x200003b8 = 0;
  *(uint32_t*)0x200003bc = 0;
  *(uint32_t*)0x200003c0 = 0;
  *(uint32_t*)0x200003c4 = 0;
  *(uint32_t*)0x200003c8 = 0;
  *(uint32_t*)0x200003cc = 0;
  *(uint32_t*)0x200003d0 = 0;
  *(uint32_t*)0x200003d4 = 0;
  *(uint32_t*)0x200003d8 = 0;
  *(uint32_t*)0x200003dc = 0;
  *(uint32_t*)0x200003e0 = 0;
  *(uint32_t*)0x200003e4 = 0;
  *(uint32_t*)0x200003e8 = 0;
  *(uint32_t*)0x200003ec = 0;
  *(uint32_t*)0x200003f0 = 0;
  *(uint32_t*)0x200003f4 = 0;
  *(uint32_t*)0x200003f8 = 0;
  *(uint32_t*)0x200003fc = 0;
  *(uint32_t*)0x20000400 = 0;
  *(uint32_t*)0x20000404 = 0;
  *(uint32_t*)0x20000408 = 0;
  *(uint32_t*)0x2000040c = 0;
  *(uint32_t*)0x20000410 = 0;
  *(uint32_t*)0x20000414 = 0;
  *(uint32_t*)0x20000418 = 0;
  *(uint32_t*)0x2000041c = 0;
  *(uint32_t*)0x20000420 = 0;
  *(uint32_t*)0x20000424 = 0;
  *(uint32_t*)0x20000428 = 0;
  *(uint32_t*)0x2000042c = 0;
  *(uint32_t*)0x20000430 = 0;
  *(uint32_t*)0x20000434 = 0;
  *(uint32_t*)0x20000438 = 0;
  *(uint32_t*)0x2000043c = 0;
  *(uint32_t*)0x20000440 = 0;
  *(uint32_t*)0x20000444 = 0;
  *(uint32_t*)0x20000448 = 0;
  *(uint32_t*)0x2000044c = 0;
  *(uint32_t*)0x20000450 = 0;
  *(uint32_t*)0x20000454 = 0;
  *(uint32_t*)0x20000458 = 0;
  *(uint32_t*)0x2000045c = 0;
  *(uint32_t*)0x20000460 = 0;
  *(uint32_t*)0x20000464 = 0;
  *(uint32_t*)0x20000468 = 0;
  *(uint32_t*)0x2000046c = 0;
  *(uint32_t*)0x20000470 = 0;
  *(uint32_t*)0x20000474 = 0;
  *(uint32_t*)0x20000478 = 0;
  *(uint32_t*)0x2000047c = 0;
  *(uint32_t*)0x20000480 = 0;
  *(uint32_t*)0x20000484 = 0;
  *(uint32_t*)0x20000488 = 0;
  *(uint32_t*)0x2000048c = 0;
  *(uint32_t*)0x20000490 = 0;
  *(uint32_t*)0x20000494 = 0;
  *(uint32_t*)0x20000498 = 0;
  *(uint32_t*)0x2000049c = 0;
  *(uint32_t*)0x200004a0 = 0;
  *(uint32_t*)0x200004a4 = 0;
  *(uint32_t*)0x200004a8 = 0;
  *(uint32_t*)0x200004ac = 0;
  *(uint32_t*)0x200004b0 = 0;
  *(uint32_t*)0x200004b4 = 0;
  *(uint32_t*)0x200004b8 = 0;
  *(uint32_t*)0x200004bc = 0;
  *(uint32_t*)0x200004c0 = 0;
  *(uint32_t*)0x200004c4 = 0;
  *(uint32_t*)0x200004c8 = 0;
  *(uint32_t*)0x200004cc = 0;
  *(uint32_t*)0x200004d0 = 0;
  *(uint32_t*)0x200004d4 = 0;
  *(uint32_t*)0x200004d8 = 0;
  *(uint32_t*)0x200004dc = 0;
  *(uint32_t*)0x200004e0 = 0;
  *(uint32_t*)0x200004e4 = 0;
  *(uint32_t*)0x200004e8 = 0;
  *(uint32_t*)0x200004ec = 0;
  *(uint32_t*)0x200004f0 = 0;
  *(uint32_t*)0x200004f4 = 0;
  *(uint32_t*)0x200004f8 = 0;
  *(uint32_t*)0x200004fc = 0;
  *(uint32_t*)0x20000500 = 0;
  *(uint32_t*)0x20000504 = 0;
  *(uint32_t*)0x20000508 = 0;
  *(uint32_t*)0x2000050c = 0;
  *(uint32_t*)0x20000510 = 0;
  *(uint32_t*)0x20000514 = 0;
  *(uint32_t*)0x20000518 = 0;
  syscall(__NR_write, r[0], 0x200000c0, 0x45c);
  syscall(__NR_ioctl, r[0], 0x40045564, 3);
  memcpy((void*)0x20000540,
         "\x73\x79\x7a\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         80);
  *(uint16_t*)0x20000590 = 0;
  *(uint16_t*)0x20000592 = 0xe4b9;
  *(uint16_t*)0x20000594 = 6;
  *(uint16_t*)0x20000596 = 8;
  *(uint32_t*)0x20000598 = 0x54;
  *(uint32_t*)0x2000059c = 2;
  *(uint32_t*)0x200005a0 = 0x107;
  *(uint32_t*)0x200005a4 = 5;
  *(uint32_t*)0x200005a8 = 0x884;
  *(uint32_t*)0x200005ac = 5;
  *(uint32_t*)0x200005b0 = 0xd5;
  *(uint32_t*)0x200005b4 = 0xfff;
  *(uint32_t*)0x200005b8 = 0xfffffff9;
  *(uint32_t*)0x200005bc = 0x200;
  *(uint32_t*)0x200005c0 = 0;
  *(uint32_t*)0x200005c4 = 3;
  *(uint32_t*)0x200005c8 = 7;
  *(uint32_t*)0x200005cc = 8;
  *(uint32_t*)0x200005d0 = 7;
  *(uint32_t*)0x200005d4 = 5;
  *(uint32_t*)0x200005d8 = 6;
  *(uint32_t*)0x200005dc = 5;
  *(uint32_t*)0x200005e0 = 6;
  *(uint32_t*)0x200005e4 = 0x200;
  *(uint32_t*)0x200005e8 = 0x97;
  *(uint32_t*)0x200005ec = 3;
  *(uint32_t*)0x200005f0 = 0x1cb;
  *(uint32_t*)0x200005f4 = 7;
  *(uint32_t*)0x200005f8 = 0x80000000;
  *(uint32_t*)0x200005fc = 0x3bc;
  *(uint32_t*)0x20000600 = 0x7b5970f7;
  *(uint32_t*)0x20000604 = 7;
  *(uint32_t*)0x20000608 = 0x1ff;
  *(uint32_t*)0x2000060c = 0xca7;
  *(uint32_t*)0x20000610 = 0x80000000;
  *(uint32_t*)0x20000614 = 4;
  *(uint32_t*)0x20000618 = 0x81;
  *(uint32_t*)0x2000061c = 0x100;
  *(uint32_t*)0x20000620 = 1;
  *(uint32_t*)0x20000624 = 0xfffffeff;
  *(uint32_t*)0x20000628 = 6;
  *(uint32_t*)0x2000062c = 2;
  *(uint32_t*)0x20000630 = 0xffff;
  *(uint32_t*)0x20000634 = 8;
  *(uint32_t*)0x20000638 = 4;
  *(uint32_t*)0x2000063c = 5;
  *(uint32_t*)0x20000640 = 0x81;
  *(uint32_t*)0x20000644 = 6;
  *(uint32_t*)0x20000648 = 7;
  *(uint32_t*)0x2000064c = 0x7ff;
  *(uint32_t*)0x20000650 = 5;
  *(uint32_t*)0x20000654 = 7;
  *(uint32_t*)0x20000658 = 0xfffffffc;
  *(uint32_t*)0x2000065c = 0x40;
  *(uint32_t*)0x20000660 = 0;
  *(uint32_t*)0x20000664 = 0x20;
  *(uint32_t*)0x20000668 = 0x14e;
  *(uint32_t*)0x2000066c = 1;
  *(uint32_t*)0x20000670 = 5;
  *(uint32_t*)0x20000674 = 1;
  *(uint32_t*)0x20000678 = 6;
  *(uint32_t*)0x2000067c = 0x191;
  *(uint32_t*)0x20000680 = 0x95c;
  *(uint32_t*)0x20000684 = 3;
  *(uint32_t*)0x20000688 = 9;
  *(uint32_t*)0x2000068c = 0xff;
  *(uint32_t*)0x20000690 = 0x400;
  *(uint32_t*)0x20000694 = 4;
  *(uint32_t*)0x20000698 = 0;
  *(uint32_t*)0x2000069c = 0xf0c;
  *(uint32_t*)0x200006a0 = 0;
  *(uint32_t*)0x200006a4 = 0x8000;
  *(uint32_t*)0x200006a8 = 5;
  *(uint32_t*)0x200006ac = 7;
  *(uint32_t*)0x200006b0 = 5;
  *(uint32_t*)0x200006b4 = 0;
  *(uint32_t*)0x200006b8 = 5;
  *(uint32_t*)0x200006bc = 0x207;
  *(uint32_t*)0x200006c0 = 0x8000;
  *(uint32_t*)0x200006c4 = 0;
  *(uint32_t*)0x200006c8 = 0xfff;
  *(uint32_t*)0x200006cc = 7;
  *(uint32_t*)0x200006d0 = 3;
  *(uint32_t*)0x200006d4 = 0;
  *(uint32_t*)0x200006d8 = 0xa79;
  *(uint32_t*)0x200006dc = 0x61e1;
  *(uint32_t*)0x200006e0 = 0x101;
  *(uint32_t*)0x200006e4 = 0x200;
  *(uint32_t*)0x200006e8 = 4;
  *(uint32_t*)0x200006ec = 0x200;
  *(uint32_t*)0x200006f0 = 1;
  *(uint32_t*)0x200006f4 = 0xe57;
  *(uint32_t*)0x200006f8 = 5;
  *(uint32_t*)0x200006fc = 0;
  *(uint32_t*)0x20000700 = 0;
  *(uint32_t*)0x20000704 = 1;
  *(uint32_t*)0x20000708 = 1;
  *(uint32_t*)0x2000070c = 0;
  *(uint32_t*)0x20000710 = 7;
  *(uint32_t*)0x20000714 = 7;
  *(uint32_t*)0x20000718 = 0;
  *(uint32_t*)0x2000071c = 6;
  *(uint32_t*)0x20000720 = 1;
  *(uint32_t*)0x20000724 = 2;
  *(uint32_t*)0x20000728 = 1;
  *(uint32_t*)0x2000072c = 9;
  *(uint32_t*)0x20000730 = 0;
  *(uint32_t*)0x20000734 = 5;
  *(uint32_t*)0x20000738 = 5;
  *(uint32_t*)0x2000073c = 6;
  *(uint32_t*)0x20000740 = 6;
  *(uint32_t*)0x20000744 = 4;
  *(uint32_t*)0x20000748 = 1;
  *(uint32_t*)0x2000074c = 1;
  *(uint32_t*)0x20000750 = 0x400;
  *(uint32_t*)0x20000754 = 0x1000;
  *(uint32_t*)0x20000758 = 0x9c8;
  *(uint32_t*)0x2000075c = 7;
  *(uint32_t*)0x20000760 = 0xc86;
  *(uint32_t*)0x20000764 = 0;
  *(uint32_t*)0x20000768 = 0x8000;
  *(uint32_t*)0x2000076c = 9;
  *(uint32_t*)0x20000770 = 4;
  *(uint32_t*)0x20000774 = 2;
  *(uint32_t*)0x20000778 = 0x3798;
  *(uint32_t*)0x2000077c = 0x8af;
  *(uint32_t*)0x20000780 = 4;
  *(uint32_t*)0x20000784 = 0x3ff;
  *(uint32_t*)0x20000788 = 0x46e;
  *(uint32_t*)0x2000078c = 2;
  *(uint32_t*)0x20000790 = 5;
  *(uint32_t*)0x20000794 = 8;
  *(uint32_t*)0x20000798 = 2;
  *(uint32_t*)0x2000079c = 0;
  *(uint32_t*)0x200007a0 = 0x80000000;
  *(uint32_t*)0x200007a4 = 2;
  *(uint32_t*)0x200007a8 = 0x207;
  *(uint32_t*)0x200007ac = 8;
  *(uint32_t*)0x200007b0 = 0x7fff;
  *(uint32_t*)0x200007b4 = 0x1000;
  *(uint32_t*)0x200007b8 = 0xfffffff9;
  *(uint32_t*)0x200007bc = 0x80e;
  *(uint32_t*)0x200007c0 = 4;
  *(uint32_t*)0x200007c4 = 5;
  *(uint32_t*)0x200007c8 = 0;
  *(uint32_t*)0x200007cc = 8;
  *(uint32_t*)0x200007d0 = 9;
  *(uint32_t*)0x200007d4 = 5;
  *(uint32_t*)0x200007d8 = 9;
  *(uint32_t*)0x200007dc = 0xff;
  *(uint32_t*)0x200007e0 = 5;
  *(uint32_t*)0x200007e4 = 0xc23f;
  *(uint32_t*)0x200007e8 = 8;
  *(uint32_t*)0x200007ec = 1;
  *(uint32_t*)0x200007f0 = 4;
  *(uint32_t*)0x200007f4 = 0xd4;
  *(uint32_t*)0x200007f8 = 0xffff104f;
  *(uint32_t*)0x200007fc = 1;
  *(uint32_t*)0x20000800 = 0xa9d;
  *(uint32_t*)0x20000804 = 6;
  *(uint32_t*)0x20000808 = 0x800;
  *(uint32_t*)0x2000080c = 0x25;
  *(uint32_t*)0x20000810 = 6;
  *(uint32_t*)0x20000814 = 0x8000;
  *(uint32_t*)0x20000818 = 7;
  *(uint32_t*)0x2000081c = 0x200001;
  *(uint32_t*)0x20000820 = 0x3f;
  *(uint32_t*)0x20000824 = 0x8001;
  *(uint32_t*)0x20000828 = 0;
  *(uint32_t*)0x2000082c = 7;
  *(uint32_t*)0x20000830 = 7;
  *(uint32_t*)0x20000834 = 0xab8;
  *(uint32_t*)0x20000838 = 0xe078;
  *(uint32_t*)0x2000083c = 0xe2;
  *(uint32_t*)0x20000840 = 0x8000;
  *(uint32_t*)0x20000844 = 2;
  *(uint32_t*)0x20000848 = 0x8000;
  *(uint32_t*)0x2000084c = 0;
  *(uint32_t*)0x20000850 = 1;
  *(uint32_t*)0x20000854 = 1;
  *(uint32_t*)0x20000858 = 0x80;
  *(uint32_t*)0x2000085c = 0x1f;
  *(uint32_t*)0x20000860 = 0x101;
  *(uint32_t*)0x20000864 = 2;
  *(uint32_t*)0x20000868 = 0x501;
  *(uint32_t*)0x2000086c = 0xbdc2;
  *(uint32_t*)0x20000870 = 0xfffffffe;
  *(uint32_t*)0x20000874 = 0x7fff;
  *(uint32_t*)0x20000878 = 0x800;
  *(uint32_t*)0x2000087c = 9;
  *(uint32_t*)0x20000880 = 6;
  *(uint32_t*)0x20000884 = 9;
  *(uint32_t*)0x20000888 = 0x47c51446;
  *(uint32_t*)0x2000088c = 0x40000;
  *(uint32_t*)0x20000890 = 3;
  *(uint32_t*)0x20000894 = 2;
  *(uint32_t*)0x20000898 = 9;
  *(uint32_t*)0x2000089c = 9;
  *(uint32_t*)0x200008a0 = 1;
  *(uint32_t*)0x200008a4 = 0x3f;
  *(uint32_t*)0x200008a8 = 0;
  *(uint32_t*)0x200008ac = 0xffff;
  *(uint32_t*)0x200008b0 = 0x20;
  *(uint32_t*)0x200008b4 = 0x7f;
  *(uint32_t*)0x200008b8 = 3;
  *(uint32_t*)0x200008bc = 4;
  *(uint32_t*)0x200008c0 = 0x6c4;
  *(uint32_t*)0x200008c4 = 0;
  *(uint32_t*)0x200008c8 = 6;
  *(uint32_t*)0x200008cc = 0;
  *(uint32_t*)0x200008d0 = 0;
  *(uint32_t*)0x200008d4 = 0x5c5;
  *(uint32_t*)0x200008d8 = 1;
  *(uint32_t*)0x200008dc = 4;
  *(uint32_t*)0x200008e0 = 0;
  *(uint32_t*)0x200008e4 = 0xfffffffb;
  *(uint32_t*)0x200008e8 = 1;
  *(uint32_t*)0x200008ec = 0xfffffff9;
  *(uint32_t*)0x200008f0 = 5;
  *(uint32_t*)0x200008f4 = 0x810;
  *(uint32_t*)0x200008f8 = 1;
  *(uint32_t*)0x200008fc = 0xfffffffd;
  *(uint32_t*)0x20000900 = 7;
  *(uint32_t*)0x20000904 = 0xb457;
  *(uint32_t*)0x20000908 = 9;
  *(uint32_t*)0x2000090c = 0x7f;
  *(uint32_t*)0x20000910 = 0x89;
  *(uint32_t*)0x20000914 = 5;
  *(uint32_t*)0x20000918 = 0x80;
  *(uint32_t*)0x2000091c = 5;
  *(uint32_t*)0x20000920 = 8;
  *(uint32_t*)0x20000924 = 4;
  *(uint32_t*)0x20000928 = 0xfff;
  *(uint32_t*)0x2000092c = 0x75;
  *(uint32_t*)0x20000930 = 0x81;
  *(uint32_t*)0x20000934 = 0x7fffffff;
  *(uint32_t*)0x20000938 = 0xc5f0;
  *(uint32_t*)0x2000093c = 3;
  *(uint32_t*)0x20000940 = 0x80000000;
  *(uint32_t*)0x20000944 = 0xc4;
  *(uint32_t*)0x20000948 = 0xf47c;
  *(uint32_t*)0x2000094c = 0x547;
  *(uint32_t*)0x20000950 = 0x7d4;
  *(uint32_t*)0x20000954 = 0x10001;
  *(uint32_t*)0x20000958 = 0x1ed;
  *(uint32_t*)0x2000095c = 0;
  *(uint32_t*)0x20000960 = 0x800;
  *(uint32_t*)0x20000964 = 3;
  *(uint32_t*)0x20000968 = 0;
  *(uint32_t*)0x2000096c = 0xa40;
  *(uint32_t*)0x20000970 = 9;
  *(uint32_t*)0x20000974 = -1;
  *(uint32_t*)0x20000978 = 2;
  *(uint32_t*)0x2000097c = 0x8001;
  *(uint32_t*)0x20000980 = 0x6b;
  *(uint32_t*)0x20000984 = 1;
  *(uint32_t*)0x20000988 = 0x3ff;
  *(uint32_t*)0x2000098c = 4;
  *(uint32_t*)0x20000990 = 5;
  *(uint32_t*)0x20000994 = 0xadf;
  *(uint32_t*)0x20000998 = 2;
  syscall(__NR_write, r[0], 0x20000540, 0x45c);
  syscall(__NR_ioctl, r[0], 0x5501);
  return 0;
}
