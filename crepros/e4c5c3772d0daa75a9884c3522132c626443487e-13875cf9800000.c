// https://syzkaller.appspot.com/bug?id=e4c5c3772d0daa75a9884c3522132c626443487e
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifndef __NR_socket
#define __NR_socket 359
#endif
#ifndef __NR_setsockopt
#define __NR_setsockopt 366
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[1];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xf23000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 2, 2, 0);
  memcpy((void*)0x20f20000, "\x66\x69\x6c\x74\x65\x72\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20f20020 = 0;
  *(uint32_t*)0x20f20024 = 1;
  *(uint32_t*)0x20f20028 = 0x90;
  *(uint32_t*)0x20f2002c = 0;
  *(uint32_t*)0x20f20030 = 0;
  *(uint32_t*)0x20f20034 = 0;
  *(uint32_t*)0x20f20038 = 0;
  *(uint32_t*)0x20f2003c = 0;
  *(uint32_t*)0x20f20040 = 0;
  *(uint32_t*)0x20f20044 = 0;
  *(uint32_t*)0x20f20048 = 8;
  *(uint32_t*)0x20f2004c = 0;
  *(uint32_t*)0x20f20050 = 0;
  *(uint32_t*)0x20f20054 = 0x10;
  *(uint32_t*)0x20f20058 = 0x20875ff0;
  *(uint32_t*)0x20f2005c = htobe32(0xe0000001);
  *(uint32_t*)0x20f20060 = htobe32(0xe0000001);
  *(uint32_t*)0x20f20064 = htobe32(0);
  *(uint32_t*)0x20f20068 = htobe32(0);
  *(uint8_t*)0x20f2006c = 0x73;
  *(uint8_t*)0x20f2006d = 0x79;
  *(uint8_t*)0x20f2006e = 0x7a;
  *(uint8_t*)0x20f2006f = 0;
  *(uint8_t*)0x20f20070 = 0;
  memcpy((void*)0x20f2007c,
         "\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x30\x00\x00\x00\x00\x00\x00",
         16);
  *(uint8_t*)0x20f2008c = 0;
  *(uint8_t*)0x20f2008d = 0;
  *(uint8_t*)0x20f2008e = 0;
  *(uint8_t*)0x20f2008f = 0;
  *(uint8_t*)0x20f20090 = 0;
  *(uint8_t*)0x20f20091 = 0;
  *(uint8_t*)0x20f20092 = 0;
  *(uint8_t*)0x20f20093 = 0;
  *(uint8_t*)0x20f20094 = 0;
  *(uint8_t*)0x20f20095 = 0;
  *(uint8_t*)0x20f20096 = 0;
  *(uint8_t*)0x20f20097 = 0;
  *(uint8_t*)0x20f20098 = 0;
  *(uint8_t*)0x20f20099 = 0;
  *(uint8_t*)0x20f2009a = 0;
  *(uint8_t*)0x20f2009b = 0;
  *(uint8_t*)0x20f2009c = 0;
  *(uint8_t*)0x20f2009d = 0;
  *(uint8_t*)0x20f2009e = 0;
  *(uint8_t*)0x20f2009f = 0;
  *(uint8_t*)0x20f200a0 = 0;
  *(uint8_t*)0x20f200a1 = 0;
  *(uint8_t*)0x20f200a2 = 0;
  *(uint8_t*)0x20f200a3 = 0;
  *(uint8_t*)0x20f200a4 = 0;
  *(uint8_t*)0x20f200a5 = 0;
  *(uint8_t*)0x20f200a6 = 0;
  *(uint8_t*)0x20f200a7 = 0;
  *(uint8_t*)0x20f200a8 = 0;
  *(uint8_t*)0x20f200a9 = 0;
  *(uint8_t*)0x20f200aa = 0;
  *(uint8_t*)0x20f200ab = 0;
  *(uint16_t*)0x20f200ac = 0;
  *(uint8_t*)0x20f200ae = 0;
  *(uint8_t*)0x20f200af = 0;
  *(uint32_t*)0x20f200b0 = 0;
  *(uint16_t*)0x20f200b4 = 0x70;
  *(uint16_t*)0x20f200b6 = 0x90;
  *(uint32_t*)0x20f200b8 = 0;
  *(uint64_t*)0x20f200bc = 0;
  *(uint64_t*)0x20f200c4 = 0;
  *(uint16_t*)0x20f200cc = 0x20;
  memcpy((void*)0x20f200ce, "\xb6\x4c\x80\x73\x01\x2d\x55\x14\xd2\xfd\x88\x42"
                            "\x3d\x5d\x31\xc7\xc3\xbb\x7f\x1a\x45\x2a\x05\x2e"
                            "\x6d\xc3\xf8\xfa\xf8",
         29);
  *(uint8_t*)0x20f200eb = 0x81;
  syscall(__NR_setsockopt, r[0], 0, 0x40, 0x20f20000, 0xf0);
}

int main()
{
  loop();
  return 0;
}
