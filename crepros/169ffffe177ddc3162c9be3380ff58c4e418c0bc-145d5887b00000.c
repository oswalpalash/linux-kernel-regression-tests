// https://syzkaller.appspot.com/bug?id=169ffffe177ddc3162c9be3380ff58c4e418c0bc
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

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x11ul, 0x800000003ul, 0);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20000080 = 0x11;
  memcpy((void*)0x20000082,
         "\x00\x00\x01\x00\x00\x00\x00\x00\x08\x00\x44\x94\x4e\xeb\xa7\x1a\x49"
         "\x76\xe2\x52\x92\x2c\xb1\x8f\x6e\x2e\x2a\xba\x00\x00\x00\x01\x2e\x0b"
         "\x38\x36\x00\x54\x04\xb0\xe0\x30\x1a\x4c\xe8\x75\xf2\xe3\xff\x5f\x16"
         "\x3e\xe3\x40\xb7\x67\x95\x00\x80\x00\x00\x00\x00\x00\x00\x01\x01\x01"
         "\x3c\x58\x11\x03\x9e\x15\x77\x50\x27\xec\xce\x66\xfd\x79\x2b\xbf\x0e"
         "\x5b\xf5\xff\x1b\x08\x16\xf3\xf6\xdb\x1c\x00\x01\x00\x00\x00\x00\x00"
         "\x00\x00\x49\x74\x00\x00\x00\x00\x00\x00\x00\x06\xad\x8e\x5e\xcc\x32"
         "\x6d\x3a\x09\xff\xc2\xc6\x54",
         126);
  syscall(__NR_bind, r[0], 0x20000080ul, 0x80ul);
  *(uint16_t*)0x20000080 = 0x11;
  memcpy((void*)0x20000082,
         "\x00\x00\x01\x00\x00\x00\x00\x00\x08\x00\x44\x94\x4e\xeb\xa7\x1a\x49"
         "\x76\xe2\x52\x92\x2c\xb1\x8f\x6e\x2e\x2a\xba\x00\x00\x00\x01\x2e\x0b"
         "\x38\x36\x00\x54\x04\xb0\xe0\x30\x1a\x4c\xe8\x75\xf2\xe3\xff\x5f\x16"
         "\x3e\xe3\x40\xb7\x67\x95\x00\x80\x00\x00\x00\x00\x00\x00\x01\x01\x01"
         "\x3c\x58\x11\x03\x9e\x15\x77\x50\x27\xec\xce\x66\xfd\x79\x2b\xbf\x0e"
         "\x5b\xf5\xff\x1b\x08\x16\xf3\xf6\xdb\x1c\x00\x01\x00\x00\x00\x00\x00"
         "\x00\x00\x49\x74\x00\x00\x00\x00\x00\x00\x00\x06\xad\x8e\x5e\xcc\x32"
         "\x6d\x3a\x09\xff\xc2\xc6\x54",
         126);
  syscall(__NR_bind, r[0], 0x20000080ul, 0x80ul);
  return 0;
}
