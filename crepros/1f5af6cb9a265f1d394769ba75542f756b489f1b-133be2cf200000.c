// https://syzkaller.appspot.com/bug?id=1f5af6cb9a265f1d394769ba75542f756b489f1b
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
  long res = 0;
  res = syscall(__NR_socket, 2, 0x80001, 0x84);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20000080 = 2;
  *(uint16_t*)0x20000082 = htobe16(0x4e20);
  *(uint32_t*)0x20000084 = htobe32(0x7f000001);
  syscall(__NR_bind, r[0], 0x20000080, 0x10);
  *(uint16_t*)0x200000c0 = 0x401;
  *(uint16_t*)0x200000c2 = 4;
  *(uint16_t*)0x200000c4 = 6;
  *(uint16_t*)0x200000c6 = 3;
  syscall(__NR_setsockopt, r[0], 0x84, 2, 0x200000c0, 8);
  *(uint32_t*)0x20000180 = 0;
  *(uint32_t*)0x20000184 = 1;
  syscall(__NR_setsockopt, r[0], 0x84, 0x7b, 0x20000180, 8);
  *(uint64_t*)0x2001afc8 = 0x20006000;
  *(uint16_t*)0x20006000 = 2;
  *(uint16_t*)0x20006002 = htobe16(0x4e20);
  *(uint32_t*)0x20006004 = htobe32(0x7f000001);
  *(uint32_t*)0x2001afd0 = 0x80;
  *(uint64_t*)0x2001afd8 = 0x20007f80;
  *(uint64_t*)0x20007f80 = 0x200001c0;
  memcpy((void*)0x200001c0, "*", 1);
  *(uint64_t*)0x20007f88 = 1;
  *(uint64_t*)0x2001afe0 = 1;
  *(uint64_t*)0x2001afe8 = 0;
  *(uint64_t*)0x2001aff0 = 0;
  *(uint32_t*)0x2001aff8 = 0;
  syscall(__NR_sendmsg, r[0], 0x2001afc8, 0);
  return 0;
}
