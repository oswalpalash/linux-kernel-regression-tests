// https://syzkaller.appspot.com/bug?id=6396c10c7afb0b3d676e283953c87c90886416a6
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
#ifndef __NR_creat
#define __NR_creat 8
#endif
#ifndef __NR_lsetxattr
#define __NR_lsetxattr 227
#endif
#ifndef __NR_getxattr
#define __NR_getxattr 229
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

void loop()
{
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  memcpy((void*)0x208b2000, "./file0", 8);
  syscall(__NR_creat, 0x208b2000, 0);
  memcpy((void*)0x20712ff8, "./file0", 8);
  memcpy((void*)0x20fb4000, "security.capability", 20);
  syscall(__NR_lsetxattr, 0x20712ff8, 0x20fb4000, 0x202b2fec, 0, 0);
  memcpy((void*)0x2072d000, "./file0", 8);
  memcpy((void*)0x20b29f57,
         "\x73\x65\x63\x75\x72\x69\x74\x79\x2e\x63\x61\x70\x61\x62\x69\x6c\x69"
         "\x74\x79\x00\xc9\xbd\x30\x12\x98\x3e\x02\x15\x59\x60\x19\x6a\x2a\x84"
         "\x58\x5f\x19\xc1\xc8\x26\x17\x29\x82\xa8\xde\x1c\xcc\x4c\xdc\x69\x87"
         "\xff\xc9\x71\x5c\xef\x22\xce\xe4\xc6\x73\xd4\x38\x8d\x5e\x56\x04\xa0"
         "\xb8\x38\xaf\xb3\x5b\xd3\x52\xe4\xc9\xee\x64\x9a\x77\x12\x08\x30\x9c"
         "\xa4\x36\xbf\x91\x9f\xd9\x63\x6b\xfa\x25\x5f\xf4\xb3\x1f\x40\xf8\x8e"
         "\xcc\x9c\x2d\x81\x21\x71\xc4\x12\x62\x4f\x0c\x14\x2f\xc5\x4f\x65\xf4"
         "\x92\x39\xf2\x76\x86\xc1\x7d\x48\x9e\x92\x5d\xa1\x9b\xf3\x1e\x30\xec"
         "\x75\xcd\x92\x23\x79\x57\x66\x1c\xe8\xa1\x56\x84\x18\xe6\xd3\xd4\x28"
         "\x17\x4f\x89\xa1\x1c\xc8\x21\xfb\xf6\x41\x22\xa3\x6f\x6b\x41\x1c",
         169);
  syscall(__NR_getxattr, 0x2072d000, 0x20b29f57, 0x20227f74, 0x8c);
}

int main()
{
  loop();
  return 0;
}
