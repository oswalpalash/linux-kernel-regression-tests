// https://syzkaller.appspot.com/bug?id=6dc9ae7ccfcf2d2e5237d4e68f1c9f63e866d0ef
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

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_madvise, 0x20ffc000ul, 0x4000ul, 0xaul);
  memcpy((void*)0x20000000, "/dev/usbmon#\000", 13);
  syscall(__NR_prctl, 0x53564d41ul, 0ul, 0x20ffb000ul, 0x2000ul, 0x20000000ul);
  syscall(__NR_madvise, 0x20ffc000ul, 0x1000ul, 0xbul);
  return 0;
}
