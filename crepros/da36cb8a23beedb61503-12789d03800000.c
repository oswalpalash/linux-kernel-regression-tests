// https://syzkaller.appspot.com/bug?id=e826b4d012da9963d581b16d665db3811588ffbe
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#ifndef __NR_bind
#define __NR_bind 361
#endif
#ifndef __NR_sendmmsg
#define __NR_sendmmsg 345
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[1];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 0x15, 5, 0);
  *(uint16_t*)0x20fc4000 = 2;
  *(uint16_t*)0x20fc4002 = htobe16(0x4e20);
  *(uint32_t*)0x20fc4004 = htobe32(0x7f000001);
  *(uint8_t*)0x20fc4008 = 0;
  *(uint8_t*)0x20fc4009 = 0;
  *(uint8_t*)0x20fc400a = 0;
  *(uint8_t*)0x20fc400b = 0;
  *(uint8_t*)0x20fc400c = 0;
  *(uint8_t*)0x20fc400d = 0;
  *(uint8_t*)0x20fc400e = 0;
  *(uint8_t*)0x20fc400f = 0;
  syscall(__NR_bind, r[0], 0x20fc4000, 0x10);
  *(uint32_t*)0x20573000 = 0x2085dff0;
  *(uint32_t*)0x20573004 = 0x10;
  *(uint32_t*)0x20573008 = 0x20035000;
  *(uint32_t*)0x2057300c = 0;
  *(uint32_t*)0x20573010 = 0x20b6a9f8;
  *(uint32_t*)0x20573014 = 0x10;
  *(uint32_t*)0x20573018 = 0;
  *(uint32_t*)0x2057301c = 0;
  *(uint16_t*)0x2085dff0 = 2;
  *(uint16_t*)0x2085dff2 = htobe16(0x4e20);
  *(uint8_t*)0x2085dff4 = 0xac;
  *(uint8_t*)0x2085dff5 = 0x14;
  *(uint8_t*)0x2085dff6 = 0;
  *(uint8_t*)0x2085dff7 = 0xaa;
  *(uint8_t*)0x2085dff8 = 0;
  *(uint8_t*)0x2085dff9 = 0;
  *(uint8_t*)0x2085dffa = 0;
  *(uint8_t*)0x2085dffb = 0;
  *(uint8_t*)0x2085dffc = 0;
  *(uint8_t*)0x2085dffd = 0;
  *(uint8_t*)0x2085dffe = 0;
  *(uint8_t*)0x2085dfff = 0;
  *(uint32_t*)0x20b6a9f8 = 0x10;
  *(uint32_t*)0x20b6a9fc = 0x114;
  *(uint32_t*)0x20b6aa00 = 2;
  syscall(__NR_sendmmsg, r[0], 0x20573000, 1, 0);
}

int main()
{
  loop();
  return 0;
}
