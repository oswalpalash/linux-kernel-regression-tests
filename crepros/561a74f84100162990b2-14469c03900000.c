// https://syzkaller.appspot.com/bug?id=8025da1388d1e3d3800ac273d7c4c68f6433bc84
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
  memcpy((void*)0x200000c0, "/dev/audio1\000", 12);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x200000c0ul, 0x42ul, 0ul);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000080 = 0x20000040;
  memcpy((void*)0x20000040, "\x7a\x7a\xcd\x52\x5e\xaf\x25\xec\xe6\x59\xd5\xc9"
                            "\x45\xc9\x71\x6a\xda\x7e\x55\x39\xe2\x86\x40\x8d"
                            "\x91\x48\x08\x79",
         28);
  *(uint64_t*)0x20000088 = 0x1c;
  syscall(__NR_writev, r[0], 0x20000080ul, 1ul);
  return 0;
}
