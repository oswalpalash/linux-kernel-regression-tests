// https://syzkaller.appspot.com/bug?id=44177111953ecfd2266bc8fa356c035c2f6dee46
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

#ifndef __NR_memfd_create
#define __NR_memfd_create 319
#endif

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  memcpy((void*)0x20000200, "/dev/udmabuf\000", 13);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000200ul, 2ul, 0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x200002c0,
         "y\0205%\372,\037\231\242\311\216\315\\\372\366\022\225^\337T\342="
         "\016~F\315s\243\377\211>"
         "\204\251\206\206\242F\220\223\230N\005e\222Jw\316c\316\2372\310\002f"
         "\003\am\b\264H\217\236\245\026\217a\377\262\"\212\025\023\242\027%!"
         "T\213\241\271-\023\371\357g\225$T\357\312hw\365\377u\177u\270*"
         "\323C\257\302H\a\000\000\000\020\2760\212\272\004\000\000\000\000\000"
         "\000\000-\302\350\377mv\025\242`z\254eM\326)="
         "JW9\217\340\224\326\001\177\251h\337\360[wP\206\002\240R\363#\330,"
         "\261I\366\225\365\205x\322\227\315\242$\260\003u\311\302}"
         "\313\264\325\032}"
         "\032\304\245\207\304I\214\316\355\321Y\02752\330Y\a\247^"
         "\310\350\237\003\000\000\000\000\000\000\000\000\000\000\000\000b\016"
         "\370\250\032\247\357D\335\244u\320\364\027\314\037n\231\\="
         "\353\021\"\263:\206u\247\222\321@\v\256\320@/"
         "3\360\372\232\226\362\'\304b\031KH\247\035j\234\376",
         272);
  res = syscall(__NR_memfd_create, 0x200002c0ul, 6ul);
  if (res != -1)
    r[1] = res;
  syscall(__NR_fcntl, r[1], 0x409ul, 7ul);
  *(uint32_t*)0x20000180 = r[1];
  *(uint32_t*)0x20000184 = 0;
  *(uint64_t*)0x20000188 = 0;
  *(uint64_t*)0x20000190 = 0x10000;
  syscall(__NR_ioctl, r[0], 0x40187542, 0x20000180ul);
  return 0;
}
