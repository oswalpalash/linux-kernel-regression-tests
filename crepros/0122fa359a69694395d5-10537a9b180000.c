// https://syzkaller.appspot.com/bug?id=8f4c492a5a2ba52c107a01f408c5402b3b9f0ec8
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  intptr_t res = 0;
  memcpy((void*)0x20000040, "/dev/uinput\000", 12);
  res = syscall(__NR_openat, /*fd=*/0xffffffffffffff9cul, /*file=*/0x20000040ul,
                /*flags=O_RDWR_NONBLOCK*/ 0x802ul, /*mode=*/0ul);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20000340 = 0x2f;
  *(uint32_t*)0x20000344 = 0;
  *(uint32_t*)0x20000348 = 0;
  *(uint32_t*)0x2000034c = 0;
  *(uint32_t*)0x20000350 = 0;
  *(uint32_t*)0x20000354 = 0;
  *(uint32_t*)0x20000358 = 0;
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0x401c5504, /*arg=*/0x20000340ul);
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0x40045564, /*arg=*/3ul);
  res = syscall(__NR_dup, /*oldfd=*/r[0]);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x20000380,
         "syz1\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000",
         80);
  *(uint16_t*)0x200003d0 = 0;
  *(uint16_t*)0x200003d2 = 0;
  *(uint16_t*)0x200003d4 = 0;
  *(uint16_t*)0x200003d6 = 0;
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
  *(uint32_t*)0x2000051c = 0;
  *(uint32_t*)0x20000520 = 0;
  *(uint32_t*)0x20000524 = 0;
  *(uint32_t*)0x20000528 = 0;
  *(uint32_t*)0x2000052c = 0;
  *(uint32_t*)0x20000530 = 0;
  *(uint32_t*)0x20000534 = 0;
  *(uint32_t*)0x20000538 = 0;
  *(uint32_t*)0x2000053c = 0;
  *(uint32_t*)0x20000540 = 0;
  *(uint32_t*)0x20000544 = 0;
  *(uint32_t*)0x20000548 = 0;
  *(uint32_t*)0x2000054c = 0;
  *(uint32_t*)0x20000550 = 0;
  *(uint32_t*)0x20000554 = 0;
  *(uint32_t*)0x20000558 = 0;
  *(uint32_t*)0x2000055c = 0;
  *(uint32_t*)0x20000560 = 0;
  *(uint32_t*)0x20000564 = 0;
  *(uint32_t*)0x20000568 = 0;
  *(uint32_t*)0x2000056c = 0;
  *(uint32_t*)0x20000570 = 0;
  *(uint32_t*)0x20000574 = 0;
  *(uint32_t*)0x20000578 = 0;
  *(uint32_t*)0x2000057c = 0;
  *(uint32_t*)0x20000580 = 0;
  *(uint32_t*)0x20000584 = 0;
  *(uint32_t*)0x20000588 = 0;
  *(uint32_t*)0x2000058c = 0;
  *(uint32_t*)0x20000590 = 0;
  *(uint32_t*)0x20000594 = 0;
  *(uint32_t*)0x20000598 = 0;
  *(uint32_t*)0x2000059c = 0;
  *(uint32_t*)0x200005a0 = 0;
  *(uint32_t*)0x200005a4 = 0;
  *(uint32_t*)0x200005a8 = 0;
  *(uint32_t*)0x200005ac = 0;
  *(uint32_t*)0x200005b0 = 0;
  *(uint32_t*)0x200005b4 = 0;
  *(uint32_t*)0x200005b8 = 0;
  *(uint32_t*)0x200005bc = 0;
  *(uint32_t*)0x200005c0 = 0;
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
  *(uint32_t*)0x200005ec = 0;
  *(uint32_t*)0x200005f0 = 0;
  *(uint32_t*)0x200005f4 = 0;
  *(uint32_t*)0x200005f8 = 0;
  *(uint32_t*)0x200005fc = 0;
  *(uint32_t*)0x20000600 = 0;
  *(uint32_t*)0x20000604 = 0;
  *(uint32_t*)0x20000608 = 0;
  *(uint32_t*)0x2000060c = 0;
  *(uint32_t*)0x20000610 = 0;
  *(uint32_t*)0x20000614 = 0;
  *(uint32_t*)0x20000618 = 0;
  *(uint32_t*)0x2000061c = 0;
  *(uint32_t*)0x20000620 = 0;
  *(uint32_t*)0x20000624 = 0;
  *(uint32_t*)0x20000628 = 0;
  *(uint32_t*)0x2000062c = 0;
  *(uint32_t*)0x20000630 = 0;
  *(uint32_t*)0x20000634 = 0;
  *(uint32_t*)0x20000638 = 0;
  *(uint32_t*)0x2000063c = 0;
  *(uint32_t*)0x20000640 = 0;
  *(uint32_t*)0x20000644 = 0;
  *(uint32_t*)0x20000648 = 0;
  *(uint32_t*)0x2000064c = 0;
  *(uint32_t*)0x20000650 = 0;
  *(uint32_t*)0x20000654 = 0;
  *(uint32_t*)0x20000658 = 0;
  *(uint32_t*)0x2000065c = 0;
  *(uint32_t*)0x20000660 = 0;
  *(uint32_t*)0x20000664 = 0;
  *(uint32_t*)0x20000668 = 0;
  *(uint32_t*)0x2000066c = 0;
  *(uint32_t*)0x20000670 = 0;
  *(uint32_t*)0x20000674 = 0;
  *(uint32_t*)0x20000678 = 0;
  *(uint32_t*)0x2000067c = 0;
  *(uint32_t*)0x20000680 = 0;
  *(uint32_t*)0x20000684 = 0;
  *(uint32_t*)0x20000688 = 0;
  *(uint32_t*)0x2000068c = 0;
  *(uint32_t*)0x20000690 = 0;
  *(uint32_t*)0x20000694 = 0;
  *(uint32_t*)0x20000698 = 0;
  *(uint32_t*)0x2000069c = 0;
  *(uint32_t*)0x200006a0 = 0;
  *(uint32_t*)0x200006a4 = 0;
  *(uint32_t*)0x200006a8 = 0;
  *(uint32_t*)0x200006ac = 0;
  *(uint32_t*)0x200006b0 = 0;
  *(uint32_t*)0x200006b4 = 0;
  *(uint32_t*)0x200006b8 = 0;
  *(uint32_t*)0x200006bc = 0;
  *(uint32_t*)0x200006c0 = 0;
  *(uint32_t*)0x200006c4 = 0;
  *(uint32_t*)0x200006c8 = 0;
  *(uint32_t*)0x200006cc = 0;
  *(uint32_t*)0x200006d0 = 0;
  *(uint32_t*)0x200006d4 = 0;
  *(uint32_t*)0x200006d8 = 0;
  *(uint32_t*)0x200006dc = 0;
  *(uint32_t*)0x200006e0 = 0;
  *(uint32_t*)0x200006e4 = 0;
  *(uint32_t*)0x200006e8 = 0;
  *(uint32_t*)0x200006ec = 0;
  *(uint32_t*)0x200006f0 = 0;
  *(uint32_t*)0x200006f4 = 0;
  *(uint32_t*)0x200006f8 = 0;
  *(uint32_t*)0x200006fc = 0;
  *(uint32_t*)0x20000700 = 0;
  *(uint32_t*)0x20000704 = 0;
  *(uint32_t*)0x20000708 = 0;
  *(uint32_t*)0x2000070c = 0;
  *(uint32_t*)0x20000710 = 0;
  *(uint32_t*)0x20000714 = 0;
  *(uint32_t*)0x20000718 = 0;
  *(uint32_t*)0x2000071c = 0;
  *(uint32_t*)0x20000720 = 0;
  *(uint32_t*)0x20000724 = 0;
  *(uint32_t*)0x20000728 = 0;
  *(uint32_t*)0x2000072c = 0;
  *(uint32_t*)0x20000730 = 0;
  *(uint32_t*)0x20000734 = 0;
  *(uint32_t*)0x20000738 = 0;
  *(uint32_t*)0x2000073c = 0;
  *(uint32_t*)0x20000740 = 0;
  *(uint32_t*)0x20000744 = 0;
  *(uint32_t*)0x20000748 = 0;
  *(uint32_t*)0x2000074c = 0;
  *(uint32_t*)0x20000750 = 0;
  *(uint32_t*)0x20000754 = 0;
  *(uint32_t*)0x20000758 = 0;
  *(uint32_t*)0x2000075c = 0;
  *(uint32_t*)0x20000760 = 0;
  *(uint32_t*)0x20000764 = 0;
  *(uint32_t*)0x20000768 = 0;
  *(uint32_t*)0x2000076c = 0;
  *(uint32_t*)0x20000770 = 0;
  *(uint32_t*)0x20000774 = 0;
  *(uint32_t*)0x20000778 = 0;
  *(uint32_t*)0x2000077c = 0;
  *(uint32_t*)0x20000780 = 0;
  *(uint32_t*)0x20000784 = 0;
  *(uint32_t*)0x20000788 = 0;
  *(uint32_t*)0x2000078c = 0;
  *(uint32_t*)0x20000790 = 0;
  *(uint32_t*)0x20000794 = 0;
  *(uint32_t*)0x20000798 = 0;
  *(uint32_t*)0x2000079c = 0;
  *(uint32_t*)0x200007a0 = 0;
  *(uint32_t*)0x200007a4 = 0;
  *(uint32_t*)0x200007a8 = 0;
  *(uint32_t*)0x200007ac = 0;
  *(uint32_t*)0x200007b0 = 0;
  *(uint32_t*)0x200007b4 = 0;
  *(uint32_t*)0x200007b8 = 0;
  *(uint32_t*)0x200007bc = 0;
  *(uint32_t*)0x200007c0 = 0;
  *(uint32_t*)0x200007c4 = 0;
  *(uint32_t*)0x200007c8 = 0;
  *(uint32_t*)0x200007cc = 0;
  *(uint32_t*)0x200007d0 = 0;
  *(uint32_t*)0x200007d4 = 0;
  *(uint32_t*)0x200007d8 = 0;
  syscall(__NR_write, /*fd=*/r[1], /*data=*/0x20000380ul, /*len=*/0x45cul);
  memcpy((void*)0x20000c80,
         "syz0\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000",
         80);
  *(uint16_t*)0x20000cd0 = 0;
  *(uint16_t*)0x20000cd2 = 0;
  *(uint16_t*)0x20000cd4 = 0;
  *(uint16_t*)0x20000cd6 = 0;
  *(uint32_t*)0x20000cd8 = 0;
  *(uint32_t*)0x20000cdc = 0;
  *(uint32_t*)0x20000ce0 = 0;
  *(uint32_t*)0x20000ce4 = 0;
  *(uint32_t*)0x20000ce8 = 0;
  *(uint32_t*)0x20000cec = 0;
  *(uint32_t*)0x20000cf0 = 0;
  *(uint32_t*)0x20000cf4 = 0;
  *(uint32_t*)0x20000cf8 = 0;
  *(uint32_t*)0x20000cfc = 0;
  *(uint32_t*)0x20000d00 = 0;
  *(uint32_t*)0x20000d04 = 0;
  *(uint32_t*)0x20000d08 = 0;
  *(uint32_t*)0x20000d0c = 0;
  *(uint32_t*)0x20000d10 = 0;
  *(uint32_t*)0x20000d14 = 0;
  *(uint32_t*)0x20000d18 = 0;
  *(uint32_t*)0x20000d1c = 0;
  *(uint32_t*)0x20000d20 = 0;
  *(uint32_t*)0x20000d24 = 0;
  *(uint32_t*)0x20000d28 = 0;
  *(uint32_t*)0x20000d2c = 0;
  *(uint32_t*)0x20000d30 = 0;
  *(uint32_t*)0x20000d34 = 0;
  *(uint32_t*)0x20000d38 = 0;
  *(uint32_t*)0x20000d3c = 0;
  *(uint32_t*)0x20000d40 = 0;
  *(uint32_t*)0x20000d44 = 0;
  *(uint32_t*)0x20000d48 = 0;
  *(uint32_t*)0x20000d4c = 0;
  *(uint32_t*)0x20000d50 = 0;
  *(uint32_t*)0x20000d54 = 0;
  *(uint32_t*)0x20000d58 = 0;
  *(uint32_t*)0x20000d5c = 0;
  *(uint32_t*)0x20000d60 = 0;
  *(uint32_t*)0x20000d64 = 0;
  *(uint32_t*)0x20000d68 = 0;
  *(uint32_t*)0x20000d6c = 0;
  *(uint32_t*)0x20000d70 = 0;
  *(uint32_t*)0x20000d74 = 0;
  *(uint32_t*)0x20000d78 = 0;
  *(uint32_t*)0x20000d7c = 0;
  *(uint32_t*)0x20000d80 = 0;
  *(uint32_t*)0x20000d84 = 0;
  *(uint32_t*)0x20000d88 = 0;
  *(uint32_t*)0x20000d8c = 0;
  *(uint32_t*)0x20000d90 = 0;
  *(uint32_t*)0x20000d94 = 0;
  *(uint32_t*)0x20000d98 = 0x80000000;
  *(uint32_t*)0x20000d9c = 0;
  *(uint32_t*)0x20000da0 = 0;
  *(uint32_t*)0x20000da4 = 0;
  *(uint32_t*)0x20000da8 = 0;
  *(uint32_t*)0x20000dac = 0;
  *(uint32_t*)0x20000db0 = 0;
  *(uint32_t*)0x20000db4 = 0;
  *(uint32_t*)0x20000db8 = 0;
  *(uint32_t*)0x20000dbc = 0;
  *(uint32_t*)0x20000dc0 = 0;
  *(uint32_t*)0x20000dc4 = 0;
  *(uint32_t*)0x20000dc8 = 0;
  *(uint32_t*)0x20000dcc = 0;
  *(uint32_t*)0x20000dd0 = 0;
  *(uint32_t*)0x20000dd4 = 0;
  *(uint32_t*)0x20000dd8 = 0;
  *(uint32_t*)0x20000ddc = 0;
  *(uint32_t*)0x20000de0 = 0;
  *(uint32_t*)0x20000de4 = 0;
  *(uint32_t*)0x20000de8 = 0;
  *(uint32_t*)0x20000dec = 0;
  *(uint32_t*)0x20000df0 = 0;
  *(uint32_t*)0x20000df4 = 0;
  *(uint32_t*)0x20000df8 = 0;
  *(uint32_t*)0x20000dfc = 0;
  *(uint32_t*)0x20000e00 = 0;
  *(uint32_t*)0x20000e04 = 0;
  *(uint32_t*)0x20000e08 = 0;
  *(uint32_t*)0x20000e0c = 0;
  *(uint32_t*)0x20000e10 = 0;
  *(uint32_t*)0x20000e14 = 0;
  *(uint32_t*)0x20000e18 = 0;
  *(uint32_t*)0x20000e1c = 0;
  *(uint32_t*)0x20000e20 = 0;
  *(uint32_t*)0x20000e24 = 0;
  *(uint32_t*)0x20000e28 = 0;
  *(uint32_t*)0x20000e2c = 0;
  *(uint32_t*)0x20000e30 = 0;
  *(uint32_t*)0x20000e34 = 0;
  *(uint32_t*)0x20000e38 = 0;
  *(uint32_t*)0x20000e3c = 0;
  *(uint32_t*)0x20000e40 = 0;
  *(uint32_t*)0x20000e44 = 0;
  *(uint32_t*)0x20000e48 = 0;
  *(uint32_t*)0x20000e4c = 0;
  *(uint32_t*)0x20000e50 = 0;
  *(uint32_t*)0x20000e54 = 0;
  *(uint32_t*)0x20000e58 = 0;
  *(uint32_t*)0x20000e5c = 0;
  *(uint32_t*)0x20000e60 = 0;
  *(uint32_t*)0x20000e64 = 0;
  *(uint32_t*)0x20000e68 = 0;
  *(uint32_t*)0x20000e6c = 0;
  *(uint32_t*)0x20000e70 = 0;
  *(uint32_t*)0x20000e74 = 0;
  *(uint32_t*)0x20000e78 = 0;
  *(uint32_t*)0x20000e7c = 0;
  *(uint32_t*)0x20000e80 = 0;
  *(uint32_t*)0x20000e84 = 0;
  *(uint32_t*)0x20000e88 = 0;
  *(uint32_t*)0x20000e8c = 0;
  *(uint32_t*)0x20000e90 = 0;
  *(uint32_t*)0x20000e94 = 0;
  *(uint32_t*)0x20000e98 = 0;
  *(uint32_t*)0x20000e9c = 0;
  *(uint32_t*)0x20000ea0 = 0;
  *(uint32_t*)0x20000ea4 = 0;
  *(uint32_t*)0x20000ea8 = 0;
  *(uint32_t*)0x20000eac = 0;
  *(uint32_t*)0x20000eb0 = 0;
  *(uint32_t*)0x20000eb4 = 0;
  *(uint32_t*)0x20000eb8 = 0;
  *(uint32_t*)0x20000ebc = 0;
  *(uint32_t*)0x20000ec0 = 0;
  *(uint32_t*)0x20000ec4 = 0;
  *(uint32_t*)0x20000ec8 = 0;
  *(uint32_t*)0x20000ecc = 0;
  *(uint32_t*)0x20000ed0 = 0;
  *(uint32_t*)0x20000ed4 = 0;
  *(uint32_t*)0x20000ed8 = 0;
  *(uint32_t*)0x20000edc = 0;
  *(uint32_t*)0x20000ee0 = 0;
  *(uint32_t*)0x20000ee4 = 0;
  *(uint32_t*)0x20000ee8 = 0;
  *(uint32_t*)0x20000eec = 0;
  *(uint32_t*)0x20000ef0 = 0;
  *(uint32_t*)0x20000ef4 = 0;
  *(uint32_t*)0x20000ef8 = 0;
  *(uint32_t*)0x20000efc = 0;
  *(uint32_t*)0x20000f00 = 0;
  *(uint32_t*)0x20000f04 = 0;
  *(uint32_t*)0x20000f08 = 0;
  *(uint32_t*)0x20000f0c = 0;
  *(uint32_t*)0x20000f10 = 0;
  *(uint32_t*)0x20000f14 = 0;
  *(uint32_t*)0x20000f18 = 0;
  *(uint32_t*)0x20000f1c = 0;
  *(uint32_t*)0x20000f20 = 0;
  *(uint32_t*)0x20000f24 = 0;
  *(uint32_t*)0x20000f28 = 0;
  *(uint32_t*)0x20000f2c = 0;
  *(uint32_t*)0x20000f30 = 0;
  *(uint32_t*)0x20000f34 = 0;
  *(uint32_t*)0x20000f38 = 0;
  *(uint32_t*)0x20000f3c = 0;
  *(uint32_t*)0x20000f40 = 0;
  *(uint32_t*)0x20000f44 = 0;
  *(uint32_t*)0x20000f48 = 0;
  *(uint32_t*)0x20000f4c = 0;
  *(uint32_t*)0x20000f50 = 0;
  *(uint32_t*)0x20000f54 = 0;
  *(uint32_t*)0x20000f58 = 0;
  *(uint32_t*)0x20000f5c = 0;
  *(uint32_t*)0x20000f60 = 0;
  *(uint32_t*)0x20000f64 = 0;
  *(uint32_t*)0x20000f68 = 0;
  *(uint32_t*)0x20000f6c = 0;
  *(uint32_t*)0x20000f70 = 0;
  *(uint32_t*)0x20000f74 = 0;
  *(uint32_t*)0x20000f78 = 0;
  *(uint32_t*)0x20000f7c = 0;
  *(uint32_t*)0x20000f80 = 0;
  *(uint32_t*)0x20000f84 = 0;
  *(uint32_t*)0x20000f88 = 0;
  *(uint32_t*)0x20000f8c = 0;
  *(uint32_t*)0x20000f90 = 0;
  *(uint32_t*)0x20000f94 = 0;
  *(uint32_t*)0x20000f98 = 0;
  *(uint32_t*)0x20000f9c = 0;
  *(uint32_t*)0x20000fa0 = 0;
  *(uint32_t*)0x20000fa4 = 0;
  *(uint32_t*)0x20000fa8 = 0;
  *(uint32_t*)0x20000fac = 0;
  *(uint32_t*)0x20000fb0 = 0;
  *(uint32_t*)0x20000fb4 = 0;
  *(uint32_t*)0x20000fb8 = 0;
  *(uint32_t*)0x20000fbc = 0;
  *(uint32_t*)0x20000fc0 = 0;
  *(uint32_t*)0x20000fc4 = 0;
  *(uint32_t*)0x20000fc8 = 0;
  *(uint32_t*)0x20000fcc = 0;
  *(uint32_t*)0x20000fd0 = 0;
  *(uint32_t*)0x20000fd4 = 0;
  *(uint32_t*)0x20000fd8 = 0;
  *(uint32_t*)0x20000fdc = 0;
  *(uint32_t*)0x20000fe0 = 0;
  *(uint32_t*)0x20000fe4 = 0;
  *(uint32_t*)0x20000fe8 = 0;
  *(uint32_t*)0x20000fec = 0;
  *(uint32_t*)0x20000ff0 = 0;
  *(uint32_t*)0x20000ff4 = 0;
  *(uint32_t*)0x20000ff8 = 0;
  *(uint32_t*)0x20000ffc = 0;
  *(uint32_t*)0x20001000 = 0;
  *(uint32_t*)0x20001004 = 0;
  *(uint32_t*)0x20001008 = 0;
  *(uint32_t*)0x2000100c = 0;
  *(uint32_t*)0x20001010 = 0;
  *(uint32_t*)0x20001014 = 0;
  *(uint32_t*)0x20001018 = 0;
  *(uint32_t*)0x2000101c = 0;
  *(uint32_t*)0x20001020 = 0;
  *(uint32_t*)0x20001024 = 0;
  *(uint32_t*)0x20001028 = 0;
  *(uint32_t*)0x2000102c = 0;
  *(uint32_t*)0x20001030 = 0;
  *(uint32_t*)0x20001034 = 0;
  *(uint32_t*)0x20001038 = 0;
  *(uint32_t*)0x2000103c = 0;
  *(uint32_t*)0x20001040 = 0;
  *(uint32_t*)0x20001044 = 0;
  *(uint32_t*)0x20001048 = 0;
  *(uint32_t*)0x2000104c = 0;
  *(uint32_t*)0x20001050 = 0;
  *(uint32_t*)0x20001054 = 0;
  *(uint32_t*)0x20001058 = 0;
  *(uint32_t*)0x2000105c = 0;
  *(uint32_t*)0x20001060 = 0;
  *(uint32_t*)0x20001064 = 0;
  *(uint32_t*)0x20001068 = 0;
  *(uint32_t*)0x2000106c = 0;
  *(uint32_t*)0x20001070 = 0;
  *(uint32_t*)0x20001074 = 0;
  *(uint32_t*)0x20001078 = 0;
  *(uint32_t*)0x2000107c = 0;
  *(uint32_t*)0x20001080 = 0;
  *(uint32_t*)0x20001084 = 0;
  *(uint32_t*)0x20001088 = 0;
  *(uint32_t*)0x2000108c = 0;
  *(uint32_t*)0x20001090 = 0;
  *(uint32_t*)0x20001094 = 0;
  *(uint32_t*)0x20001098 = 0;
  *(uint32_t*)0x2000109c = 0;
  *(uint32_t*)0x200010a0 = 0;
  *(uint32_t*)0x200010a4 = 0;
  *(uint32_t*)0x200010a8 = 0;
  *(uint32_t*)0x200010ac = 0;
  *(uint32_t*)0x200010b0 = 0;
  *(uint32_t*)0x200010b4 = 0;
  *(uint32_t*)0x200010b8 = 0;
  *(uint32_t*)0x200010bc = 0;
  *(uint32_t*)0x200010c0 = 0;
  *(uint32_t*)0x200010c4 = 0;
  *(uint32_t*)0x200010c8 = 0;
  *(uint32_t*)0x200010cc = 0;
  *(uint32_t*)0x200010d0 = 0;
  *(uint32_t*)0x200010d4 = 0;
  *(uint32_t*)0x200010d8 = 0;
  syscall(__NR_write, /*fd=*/r[1], /*data=*/0x20000c80ul, /*len=*/0x45cul);
  syscall(__NR_ioctl, /*fd=*/r[1], /*cmd=*/0x5501, 0);
  return 0;
}
