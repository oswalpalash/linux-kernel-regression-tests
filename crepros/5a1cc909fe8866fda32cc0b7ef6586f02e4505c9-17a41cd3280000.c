// https://syzkaller.appspot.com/bug?id=5a1cc909fe8866fda32cc0b7ef6586f02e4505c9
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
  res = syscall(__NR_socket, /*domain=*/0x10ul, /*type=*/3ul, /*proto=*/0);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x200001c0 = 0;
  *(uint32_t*)0x200001c8 = 0;
  *(uint64_t*)0x200001d0 = 0x20000180;
  *(uint64_t*)0x20000180 = 0x200000c0;
  *(uint32_t*)0x200000c0 = 0x30;
  *(uint16_t*)0x200000c4 = 0x18;
  *(uint16_t*)0x200000c6 = 1;
  *(uint32_t*)0x200000c8 = 0;
  *(uint32_t*)0x200000cc = 0;
  *(uint8_t*)0x200000d0 = 0xa;
  *(uint8_t*)0x200000d1 = 0;
  *(uint8_t*)0x200000d2 = 0;
  *(uint8_t*)0x200000d3 = 0;
  *(uint8_t*)0x200000d4 = 0;
  *(uint8_t*)0x200000d5 = 0;
  *(uint8_t*)0x200000d6 = 0;
  *(uint8_t*)0x200000d7 = 0;
  *(uint32_t*)0x200000d8 = 0;
  *(uint16_t*)0x200000dc = 0x14;
  *(uint16_t*)0x200000de = 5;
  *(uint64_t*)0x200000e0 = htobe64(0);
  *(uint64_t*)0x200000e8 = htobe64(1);
  *(uint64_t*)0x20000188 = 0x30;
  *(uint64_t*)0x200001d8 = 1;
  *(uint64_t*)0x200001e0 = 0;
  *(uint64_t*)0x200001e8 = 0;
  *(uint32_t*)0x200001f0 = 0;
  syscall(__NR_sendmsg, /*fd=*/r[0], /*msg=*/0x200001c0ul, /*f=*/0ul);
  return 0;
}
