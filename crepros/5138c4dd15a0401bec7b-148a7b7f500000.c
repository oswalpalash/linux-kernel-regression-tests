// https://syzkaller.appspot.com/bug?id=b66d8de2cec1e3878a0524807b93d96bba182fba
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
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  memcpy((void*)0x20000080,
         "cpu\t\t&6\n\000\377\000\334.\337\273k\255\033\363\366_|S\223>"
         "\264\025#b\221\354\362\022\273\320\3150\244\332\250\341o+\275}"
         "EV\2726\256\356(4K\\\237?"
         "\370S\302\035\3028\373\031\224\337\021JE\002\230Pm\017(\230\377\005["
         "\331\255|\243\311Y~"
         "\364\367\371F\234\360\203H\265\022\336M\2002\037\246\212\211\034",
         106);
  syscall(__NR_ioctl, -1, 0x40082406, 0x20000080ul);
  memcpy((void*)0x20000040, "/dev/net/tun\000", 13);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000040ul, 0ul, 0ul);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x200000c0, "syzkaller1\000\000\000\000\000\000", 16);
  syscall(__NR_ioctl, r[0], 0x400454ca, 0x200000c0ul);
  syscall(__NR_ioctl, r[0], 0x400454cd, 0x118ul);
  return 0;
}
