// https://syzkaller.appspot.com/bug?id=449f5619489cae8df8f71d2c6bf3675f4af9d6ce
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

#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_sendmsg
#define __NR_sendmsg 370
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10, 3, 0xc);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20003e00 = 0;
  *(uint32_t*)0x20003e04 = 0;
  *(uint32_t*)0x20003e08 = 0x20003dc0;
  *(uint32_t*)0x20003dc0 = 0x20000600;
  memcpy((void*)0x20000600,
         "\x14\x00\x00\x00\x10\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x0a\x20\x00\x00\x00\x00\x0a\x05\x14\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x01\x00\x00\x00\x09\x00\x01\x00\x73\x79\x7a\x30\x00\x00\x00"
         "\x00\x58\x00\x00\x00\x16\x0a\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x01\x00\x00\x00\x09\x00\x01\x00\x73\x79\x7a\x30\x00\x00\x00\x00\x09"
         "\x00\x02\x00\x73\x79\x7a\x32\x00\x00\x00\x00\x2c\x00\x03\x80\x18\x00"
         "\x03\x80\x14\x00\x01\x00\x73\x69\x74\x30\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x08\x00\x02\x40\x00\x00\x00\x00\x08\x00\x01\x40"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x09\x00\x01\x00"
         "\x73\x79\x7a\x31\x00\x00\x00\x00\x08\x00\x02\x40\x00\x00\x00\x00\x0c"
         "\x00\x04\x40\x00\x00\x00\x00\x00\x00\x00\x00\x09\x00\x01\x00\x73\x79"
         "\x7a\x30\x00\x00\x00\x15\x0c\x00\x04\x40\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x08\x00\x02\x00\x11\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x0a\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\xb4\xee\x20\x79\x62\x28\x6f\xbb\x98\xa8\xe5"
         "\x26\x90\xe5\x23\x05\xe9\xed\x99\x6f\xef\x9b\x47\x65\x1f\x7b\xc1\xaf"
         "\xb6\xb2\x87\xa8\xab\x30\x03\x7a\xbc\x42\x28",
         283);
  *(uint32_t*)0x20003dc4 = 0xf4;
  *(uint32_t*)0x20003e0c = 1;
  *(uint32_t*)0x20003e10 = 0;
  *(uint32_t*)0x20003e14 = 0;
  *(uint32_t*)0x20003e18 = 0;
  syscall(__NR_sendmsg, (intptr_t)r[0], 0x20003e00, 0);
  return 0;
}
