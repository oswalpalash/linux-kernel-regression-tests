// https://syzkaller.appspot.com/bug?id=0f3f51343d723b1f87bdf56364a2b3a1824e36d7
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifndef __NR_mmap
#define __NR_mmap 90
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#ifndef __NR_sendmsg
#define __NR_sendmsg 370
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[12];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  r[1] = syscall(__NR_socket, 0x1eul, 0x1ul, 0x0ul);
  *(uint32_t*)0x201ff000 = (uint32_t)0x2031af80;
  *(uint32_t*)0x201ff004 = (uint32_t)0x80;
  *(uint32_t*)0x201ff008 = (uint32_t)0x204ae000;
  *(uint32_t*)0x201ff00c = (uint32_t)0x0;
  *(uint32_t*)0x201ff010 = (uint32_t)0x202d4000;
  *(uint32_t*)0x201ff014 = (uint32_t)0x0;
  *(uint32_t*)0x201ff018 = (uint32_t)0x80d1;
  *(uint16_t*)0x2031af80 = (uint16_t)0x10000000001e;
  memcpy((void*)0x2031af82,
         "\x01\x01\x01\x00\x00\x00\x00\x00\x00\x95\x5b\x9c\xe5\x26\xcc"
         "\x57\x3c\x5b\xf8\x1e\x48\x37\x24\xc7\x6c\x14\xdd\x6a\x73\x9e"
         "\xbf\xea\x1b\x86\x28\x6b\xe6\x1f\xfe\x06\xd7\x9f\x00\x00\x00"
         "\x00\x00\x00\x00\x07\x6c\x3f\xac\x92\xb9\xd8\xf9\x86\x01\x80"
         "\x9b\x9b\xf3\x04\x77\x55\xaf\x50\xd5\xfe\x32\xc4\x19\xd6\x7b"
         "\xcb\xc7\xe3\xad\x31\x6a\x19\x83\x56\xed\xb9\xb7\x01\x00\x1f"
         "\xd4\x56\x24\x3a\x05\x00\x80\x0e\xce\x70\xb0\x76\xb7\x60\xc9"
         "\xae\xcc\x5a\xa6\x76\x7e\x2e\x68\x91\xdf\xd3\xfb\x88\x1a\x15"
         "\x65\xb3\xb1\x6d\x74\x36",
         126);
  r[11] = syscall(__NR_sendmsg, r[1], 0x201ff000ul, 0x4000ul);
}

int main()
{
  loop();
  return 0;
}
