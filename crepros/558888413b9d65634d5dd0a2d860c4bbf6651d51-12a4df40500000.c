// https://syzkaller.appspot.com/bug?id=558888413b9d65634d5dd0a2d860c4bbf6651d51
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000040 = 0;
  *(uint32_t*)0x20000048 = 0;
  *(uint64_t*)0x20000050 = 0x20000000;
  *(uint64_t*)0x20000000 = 0x20000380;
  memcpy((void*)0x20000380, "\x3c\x00\x00\x00\x10\x00\x85\x06\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         20);
  *(uint32_t*)0x20000394 = -1;
  memcpy((void*)0x20000398, "\x01\x00\x00\x00\x00\x00\x00\x00\x1c\x00\x12\x00"
                            "\x0c\x00\x01\x00\x62\x6f\x6e\x64\x00\x00\x00\x00"
                            "\x0c\x00\x02\x00\x08\x00\x01\x00\x06",
         33);
  *(uint64_t*)0x20000008 = 0x3c;
  *(uint64_t*)0x20000058 = 1;
  *(uint64_t*)0x20000060 = 0;
  *(uint64_t*)0x20000068 = 0;
  *(uint32_t*)0x20000070 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000040ul, 0ul);
  res = syscall(__NR_socket, 0x10ul, 3ul, 6);
  if (res != -1)
    r[1] = res;
  *(uint64_t*)0x20000180 = 0;
  *(uint32_t*)0x20000188 = 0;
  *(uint64_t*)0x20000190 = 0x20000340;
  *(uint64_t*)0x20000340 = 0x200001c0;
  memcpy((void*)0x200001c0,
         "\x44\x01\x00\x00\x10\x00\x13\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xac\x14"
         "\x14\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00",
         64);
  *(uint32_t*)0x20000200 = 0;
  *(uint32_t*)0x20000204 = 0;
  memcpy((void*)0x20000208,
         "\x20\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x32\x00\x00\x00\xac\x14\x14\xaa\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x48\x00"
         "\x02\x00\x65\x63\x62\x28\x63\x69\x70\x68\x65\x72\x5f\x6e\x75\x6c\x6c"
         "\x29\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x0c\x00\x1c\x00",
         244);
  *(uint32_t*)0x200002fc = -1;
  *(uint64_t*)0x20000348 = 0x144;
  *(uint64_t*)0x20000198 = 1;
  *(uint64_t*)0x200001a0 = 0;
  *(uint64_t*)0x200001a8 = 0;
  *(uint32_t*)0x200001b0 = 0;
  syscall(__NR_sendmsg, r[1], 0x20000180ul, 0ul);
  return 0;
}
