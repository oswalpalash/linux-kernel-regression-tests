// https://syzkaller.appspot.com/bug?id=6ffe7d1ebf1efaddb7ddd04784b9b22a8562b8d0
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
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul, /*prot=*/7ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  intptr_t res = 0;
  res = syscall(__NR_socket, /*domain=*/0x29ul, /*type=*/2ul, /*proto=*/0);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x200006c0 = 0;
  *(uint32_t*)0x200006c8 = 0;
  *(uint64_t*)0x200006d0 = 0;
  *(uint64_t*)0x200006d8 = 0;
  *(uint64_t*)0x200006e0 = 0;
  *(uint64_t*)0x200006e8 = 0;
  *(uint32_t*)0x200006f0 = 0;
  *(uint32_t*)0x200006f8 = 0;
  syscall(__NR_sendmmsg, /*fd=*/r[0], /*mmsg=*/0x200006c0ul, /*vlen=*/1ul,
          /*f=*/0ul);
  return 0;
}
