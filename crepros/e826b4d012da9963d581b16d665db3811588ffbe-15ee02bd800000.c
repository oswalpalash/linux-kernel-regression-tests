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
#ifndef __NR_sendto
#define __NR_sendto 369
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[1];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 0x15, 0x805, 0);
  *(uint16_t*)0x200a9000 = 2;
  *(uint16_t*)0x200a9002 = 0;
  *(uint32_t*)0x200a9004 = htobe32(0x7f000001);
  *(uint8_t*)0x200a9008 = 0;
  *(uint8_t*)0x200a9009 = 0;
  *(uint8_t*)0x200a900a = 0;
  *(uint8_t*)0x200a900b = 0;
  *(uint8_t*)0x200a900c = 0;
  *(uint8_t*)0x200a900d = 0;
  *(uint8_t*)0x200a900e = 0;
  *(uint8_t*)0x200a900f = 0;
  syscall(__NR_bind, r[0], 0x200a9000, 0x10);
  *(uint16_t*)0x20b2d000 = 2;
  *(uint16_t*)0x20b2d002 = 0;
  *(uint32_t*)0x20b2d004 = htobe32(0x7f000001);
  *(uint8_t*)0x20b2d008 = 0;
  *(uint8_t*)0x20b2d009 = 0;
  *(uint8_t*)0x20b2d00a = 0;
  *(uint8_t*)0x20b2d00b = 0;
  *(uint8_t*)0x20b2d00c = 0;
  *(uint8_t*)0x20b2d00d = 0;
  *(uint8_t*)0x20b2d00e = 0;
  *(uint8_t*)0x20b2d00f = 0;
  syscall(__NR_sendto, r[0], 0x20dbf000, 0, 0, 0x20b2d000, 0x10);
}

int main()
{
  loop();
  return 0;
}
