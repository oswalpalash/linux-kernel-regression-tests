// https://syzkaller.appspot.com/bug?id=1d06e732324171bb1b1748e64b71a5536d745f7f
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <stdint.h>
#include <string.h>

long r[1];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 0x3, 0x32, 0xffffffff, 0x0);
  r[0] = syscall(__NR_socket, 0x40000000015, 0x5, 0x0);
  *(uint16_t*)0x20849ff0 = 0x2;
  *(uint16_t*)0x20849ff2 = htobe16(0x4e20);
  *(uint32_t*)0x20849ff4 = htobe32(0x7f000001);
  *(uint8_t*)0x20849ff8 = 0x0;
  *(uint8_t*)0x20849ff9 = 0x0;
  *(uint8_t*)0x20849ffa = 0x0;
  *(uint8_t*)0x20849ffb = 0x0;
  *(uint8_t*)0x20849ffc = 0x0;
  *(uint8_t*)0x20849ffd = 0x0;
  *(uint8_t*)0x20849ffe = 0x0;
  *(uint8_t*)0x20849fff = 0x0;
  syscall(__NR_bind, r[0], 0x20849ff0, 0x10);
  *(uint64_t*)0x20573000 = 0x2083f000;
  *(uint32_t*)0x20573008 = 0x10;
  *(uint64_t*)0x20573010 = 0x2054ffe0;
  *(uint64_t*)0x20573018 = 0x0;
  *(uint64_t*)0x20573020 = 0x20861f88;
  *(uint64_t*)0x20573028 = 0x10;
  *(uint32_t*)0x20573030 = 0x0;
  *(uint32_t*)0x20573038 = 0x0;
  *(uint16_t*)0x2083f000 = 0x2;
  *(uint16_t*)0x2083f002 = htobe16(0x4e20);
  *(uint8_t*)0x2083f004 = 0xac;
  *(uint8_t*)0x2083f005 = 0x14;
  *(uint8_t*)0x2083f006 = 0x0;
  *(uint8_t*)0x2083f007 = 0xaa;
  *(uint8_t*)0x2083f008 = 0x0;
  *(uint8_t*)0x2083f009 = 0x0;
  *(uint8_t*)0x2083f00a = 0x0;
  *(uint8_t*)0x2083f00b = 0x0;
  *(uint8_t*)0x2083f00c = 0x0;
  *(uint8_t*)0x2083f00d = 0x0;
  *(uint8_t*)0x2083f00e = 0x0;
  *(uint8_t*)0x2083f00f = 0x0;
  *(uint64_t*)0x20861f88 = 0x10;
  *(uint32_t*)0x20861f90 = 0x114;
  *(uint32_t*)0x20861f94 = 0x1;
  syscall(__NR_sendmmsg, r[0], 0x20573000, 0x1, 0x0);
}

int main()
{
  loop();
  return 0;
}
