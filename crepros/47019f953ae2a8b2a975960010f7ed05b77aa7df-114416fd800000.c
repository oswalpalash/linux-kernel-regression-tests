// https://syzkaller.appspot.com/bug?id=47019f953ae2a8b2a975960010f7ed05b77aa7df
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

long r[1];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0x17000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 0x10, 3, 0x10);
  *(uint64_t*)0x20003000 = 0x20000000;
  *(uint32_t*)0x20003008 = 0;
  *(uint64_t*)0x20003010 = 0x20004000;
  *(uint64_t*)0x20003018 = 1;
  *(uint64_t*)0x20003020 = 0x20000000;
  *(uint64_t*)0x20003028 = 0;
  *(uint32_t*)0x20003030 = 0;
  *(uint64_t*)0x20004000 = 0x20011f8e;
  *(uint64_t*)0x20004008 = 0x5e;
  memcpy((void*)0x20011f8e,
         "\x23\x00\x00\x00\x24\x00\x91\x15\x46\x00\x00\x00\x00\x00\x00\x12\x01"
         "\x00\x00\x00\x00\x00\x00\x27\x08\x41\x00\x00\x00\x00\x00\x18\x00\x73"
         "\x52\x18\x88\x76\xb5\x16\xac\x30\x1e\x66\x61\x5e\x3e\x09\x08\x37\xa0"
         "\xb4\x92\x84\xa6\x56\xa0\x0e\x76\x14\x0a\xf1\xf0\x1f\x96\x38\x01\x50"
         "\x9e\x81\xcc\xa6\x63\x80\xf2\x0a\x38\x84\x9b\x7c\x14\x0b\xee\x51\x07"
         "\x1f\xc0\xeb\x1a\xcc\x2d\xbc\x78\x40",
         94);
  syscall(__NR_sendmsg, r[0], 0x20003000, 0);
}

int main()
{
  loop();
  return 0;
}
