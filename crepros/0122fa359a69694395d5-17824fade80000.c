// https://syzkaller.appspot.com/bug?id=8f4c492a5a2ba52c107a01f408c5402b3b9f0ec8
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
  memcpy((void*)0x20000040, "/dev/uinput\000", 12);
  res = syscall(__NR_openat, /*fd=*/0xffffffffffffff9cul, /*file=*/0x20000040ul,
                /*flags=*/0ul, /*mode=*/0ul);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20000000 = 0x2f;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x2000000c = 0x100003;
  *(uint32_t*)0x20000010 = 0;
  *(uint32_t*)0x20000014 = 0;
  *(uint32_t*)0x20000018 = 0;
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0x401c5504, /*arg=*/0x20000000ul);
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0x40045564, /*arg=*/3ul);
  *(uint16_t*)0x20000080 = 0;
  *(uint16_t*)0x20000082 = 0;
  *(uint16_t*)0x20000084 = 0;
  *(uint16_t*)0x20000086 = 0;
  memcpy((void*)0x20000088,
         "syz1\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000",
         80);
  *(uint32_t*)0x200000d8 = 0;
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0x405c5503, /*arg=*/0x20000080ul);
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0x5501, 0);
  return 0;
}
