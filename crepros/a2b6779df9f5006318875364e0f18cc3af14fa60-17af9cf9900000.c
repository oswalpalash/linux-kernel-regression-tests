// https://syzkaller.appspot.com/bug?id=a2b6779df9f5006318875364e0f18cc3af14fa60
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

#ifndef __NR_io_uring_setup
#define __NR_io_uring_setup 425
#endif

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);

  *(uint32_t*)0x20000344 = 0xaba5;
  *(uint32_t*)0x20000348 = 0x3b;
  *(uint32_t*)0x2000034c = 0;
  *(uint32_t*)0x20000350 = 0;
  *(uint32_t*)0x20000358 = -1;
  *(uint32_t*)0x2000035c = 0;
  *(uint32_t*)0x20000360 = 0;
  *(uint32_t*)0x20000364 = 0;
  syscall(__NR_io_uring_setup, 0x32f8, 0x20000340ul);
  return 0;
}
