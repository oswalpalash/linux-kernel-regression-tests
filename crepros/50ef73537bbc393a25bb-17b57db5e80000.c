// https://syzkaller.appspot.com/bug?id=952064d00e4000d0d4c6c3c2565bd0aa3e131a36
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
  res = syscall(__NR_socket, /*domain=*/0x11ul, /*type=*/2ul, /*proto=*/0x300);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x200000c0 = 0x8000;
  *(uint32_t*)0x200000c4 = 0x200;
  *(uint32_t*)0x200000c8 = 0x80;
  *(uint32_t*)0x200000cc = 0x20000;
  *(uint32_t*)0x200000d0 = 0;
  *(uint32_t*)0x200000d4 = 0;
  *(uint32_t*)0x200000d8 = 0x400;
  syscall(__NR_setsockopt, /*fd=*/r[0], /*level=*/0x107, /*optname=*/5,
          /*optval=*/0x200000c0ul, /*optlen=*/0x1cul);
  syscall(__NR_mmap, /*addr=*/0x20568000ul, /*len=*/0x1000000ul, /*prot=*/0ul,
          /*flags=*/0x11ul, /*fd=*/r[0], /*offset=*/0ul);
  return 0;
}
