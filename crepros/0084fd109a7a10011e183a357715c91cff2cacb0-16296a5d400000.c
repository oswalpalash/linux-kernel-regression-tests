// https://syzkaller.appspot.com/bug?id=0084fd109a7a10011e183a357715c91cff2cacb0
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
  *(uint64_t*)0x20000200 = 0xd273;
  syscall(__NR_set_mempolicy, 2, 0x20000200, 3);
  res = syscall(__NR_socket, 0xa, 1, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000000 = 4;
  syscall(__NR_getsockopt, r[0], 6, 5, 0x20d11000, 0x20000000);
  return 0;
}
