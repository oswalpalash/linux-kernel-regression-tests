// https://syzkaller.appspot.com/bug?id=27489f11432fa6d7ffedf7d6e23d966da9cb9dc0
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
  res = syscall(__NR_socket, 0x10ul, 2ul, 0x10);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x200000c0 = 0;
  *(uint32_t*)0x200000c8 = 0x384;
  *(uint64_t*)0x200000d0 = 0x20000080;
  *(uint64_t*)0x20000080 = 0x20000100;
  memcpy((void*)0x20000100, "\x2e\x00\x00\x00\x33\x00\x05\x35\xa4\xab\xd3\x2b"
                            "\x80\x18\x07\x7a\x01\x24\xfc\x60\x10\x4a\x02\x40"
                            "\x0c\x00\x02\x00\x05\x35\x82\xc1\x2d\x15\x3e\x37"
                            "\x09\x00\x01\x80\x2e\x25\x64\x25\x00\xbd",
         46);
  *(uint64_t*)0x20000088 = 0x2e;
  *(uint64_t*)0x200000d8 = 1;
  *(uint64_t*)0x200000e0 = 0;
  *(uint64_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000f0 = 0;
  syscall(__NR_sendmsg, r[0], 0x200000c0ul, 0ul);
  return 0;
}
