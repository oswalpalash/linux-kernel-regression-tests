// https://syzkaller.appspot.com/bug?id=c7c08abc80d977ee1d2ca48828269ee08858a58c
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
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10, 3, 4);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000040 = 0;
  *(uint32_t*)0x20000048 = 0;
  *(uint64_t*)0x20000050 = 0x2000d379;
  *(uint64_t*)0x2000d379 = 0x20000240;
  *(uint32_t*)0x20000240 = 0x74;
  *(uint16_t*)0x20000244 = 0x14;
  *(uint16_t*)0x20000246 = 0x301;
  *(uint32_t*)0x20000248 = 0;
  *(uint32_t*)0x2000024c = 0;
  *(uint8_t*)0x20000250 = 0x1e;
  *(uint8_t*)0x20000251 = 0;
  *(uint16_t*)0x20000252 = 0;
  memcpy((void*)0x20000254,
         "\xaf\x73\x4a\xb3\x56\xe6\x2c\x5f\xd0\x0d\x8e\x92\x34\x41\x9d\xff\x07"
         "\x58\x34\x32\x97\x27\x13\xff\xcf\x39\x33\xcb\x9e\xfe\x19\x8b\xe2\x94"
         "\xaa\x4c\x47\x2d\x76\x48\xff\xd2\x9d\xc5\x12\xa6\x4c\xcc\xb1\x78\xeb"
         "\xcf\xad\x3b\x7d\x4e\x1e\xd1\xdf\xb5\x60\x68\xb5\xa7\xb8\x57\xb5\x6a"
         "\x22\xdd\xd5\x9f\x4c\x05\x54\x69\x40\xbd\x96\xb2\x5a\x40\xdd\x68\xc9"
         "\xc6\x56\x80\xe2\xe8\x2b\xce\xbe\x56\x8d\xdc",
         96);
  *(uint64_t*)0x2000d381 = 0x74;
  *(uint64_t*)0x20000058 = 1;
  *(uint64_t*)0x20000060 = 0;
  *(uint64_t*)0x20000068 = 0;
  *(uint32_t*)0x20000070 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000040, 0);
  return 0;
}
