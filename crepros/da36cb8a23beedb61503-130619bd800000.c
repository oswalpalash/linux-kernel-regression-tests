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
#ifndef __NR_connect
#define __NR_connect 362
#endif
#ifndef __NR_sendmsg
#define __NR_sendmsg 370
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[1];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 0x15, 5, 0);
  *(uint16_t*)0x20fc4ff0 = 2;
  *(uint16_t*)0x20fc4ff2 = 0;
  *(uint32_t*)0x20fc4ff4 = htobe32(0x7f000001);
  *(uint8_t*)0x20fc4ff8 = 0;
  *(uint8_t*)0x20fc4ff9 = 0;
  *(uint8_t*)0x20fc4ffa = 0;
  *(uint8_t*)0x20fc4ffb = 0;
  *(uint8_t*)0x20fc4ffc = 0;
  *(uint8_t*)0x20fc4ffd = 0;
  *(uint8_t*)0x20fc4ffe = 0;
  *(uint8_t*)0x20fc4fff = 0;
  syscall(__NR_bind, r[0], 0x20fc4ff0, 0x10);
  *(uint16_t*)0x20adf000 = 2;
  *(uint16_t*)0x20adf002 = 0;
  *(uint32_t*)0x20adf004 = htobe32(0x7f000001);
  *(uint8_t*)0x20adf008 = 0;
  *(uint8_t*)0x20adf009 = 0;
  *(uint8_t*)0x20adf00a = 0;
  *(uint8_t*)0x20adf00b = 0;
  *(uint8_t*)0x20adf00c = 0;
  *(uint8_t*)0x20adf00d = 0;
  *(uint8_t*)0x20adf00e = 0;
  *(uint8_t*)0x20adf00f = 0;
  syscall(__NR_connect, r[0], 0x20adf000, 0x10);
  *(uint32_t*)0x20159fc8 = 0;
  *(uint32_t*)0x20159fcc = 0;
  *(uint32_t*)0x20159fd0 = 0x20127000;
  *(uint32_t*)0x20159fd4 = 0;
  *(uint32_t*)0x20159fd8 = 0x20000e8e;
  *(uint32_t*)0x20159fdc = 0;
  *(uint32_t*)0x20159fe0 = 0;
  syscall(__NR_sendmsg, r[0], 0x20159fc8, 0);
}

int main()
{
  loop();
  return 0;
}
