// https://syzkaller.appspot.com/bug?id=bcaff554170b1e402c645b3d24be70770b2aa64a
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
  r[0] = syscall(__NR_socket, 0x10, 3, 0x10);
  *(uint32_t*)0x20b3dfc8 = 0x20d49ff4;
  *(uint32_t*)0x20b3dfcc = 0xc;
  *(uint32_t*)0x20b3dfd0 = 0x20007000;
  *(uint32_t*)0x20b3dfd4 = 1;
  *(uint32_t*)0x20b3dfd8 = 0;
  *(uint32_t*)0x20b3dfdc = 0;
  *(uint32_t*)0x20b3dfe0 = 0;
  *(uint16_t*)0x20d49ff4 = 0x10;
  *(uint16_t*)0x20d49ff6 = 0;
  *(uint32_t*)0x20d49ff8 = 0;
  *(uint32_t*)0x20d49ffc = 0;
  *(uint32_t*)0x20007000 = 0x20b5af58;
  *(uint32_t*)0x20007004 = 0xa8;
  *(uint32_t*)0x20b5af58 = 0xa8;
  *(uint16_t*)0x20b5af5c = 0x1c;
  *(uint16_t*)0x20b5af5e = 0x109;
  *(uint32_t*)0x20b5af60 = 0;
  *(uint32_t*)0x20b5af64 = 0;
  *(uint8_t*)0x20b5af68 = 4;
  *(uint8_t*)0x20b5af69 = 0;
  *(uint16_t*)0x20b5af6a = 0;
  *(uint16_t*)0x20b5af6c = 0x94;
  *(uint16_t*)0x20b5af6e = 0x11;
  memcpy((void*)0x20b5af70,
         "\x6a\xa2\x10\x0e\x91\x23\xe2\x97\xa0\xd9\x40\x41\xd6\x12\x7d\xc8\xd0"
         "\x0d\xb0\xa0\x93\x6f\xf4\xac\x1d\x7b\x42\x76\x80\x4f\x14\x8e\x6c\xdf"
         "\xcf\xc6\xc0\xf5\x07\x47\xc1\x74\xb0\xd2\xf7\x9c\xb0\xa3\x96\x09\x1d"
         "\xe8\xfc\xed\xea\xf9\xae\xb1\xa6\x76\xe2\x9c\x3d\x01\x66\x41\xe4\x9e"
         "\x6f\x96\x8c\xa5\x77\xd8\xbf\x39\xc9\x6e\x06\xf6\xb9\x1e\xc2\xfa\xea"
         "\xc3\x80\x6c\x79\x61\x86\x10\xea\xf4\x33\x2b\x18\x14\x68\xb8\x62\xc9"
         "\xda\xfc\x6b\xb6\xa1\x8f\x48\x77\x8d\xdb\xfe\xee\x72\x6d\xca\x27\x2d"
         "\xc3\x78\x34\x7a\x96\xaf\xe3\x5b\x0f\x21\x4b\xd9\xc7\xc1\x9c\x00\xd6"
         "\x33\x10\x6e\x13\x18\x3e",
         142);
  syscall(__NR_sendmsg, r[0], 0x20b3dfc8, 0);
}

int main()
{
  loop();
  return 0;
}
