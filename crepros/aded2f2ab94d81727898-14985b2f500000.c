// https://syzkaller.appspot.com/bug?id=9fd089d44734021c09c696938bb74d3b3f8273c2
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

  memcpy((void*)0x20000180, "./file0\000", 8);
  syscall(__NR_mkdir, 0x20000180ul, 0ul);
  memcpy((void*)0x20000140, "./file0\000", 8);
  memcpy((void*)0x20000040, "bdev\000", 5);
  syscall(__NR_mount, 0ul, 0x20000140ul, 0x20000040ul, 0ul, 0ul);
  return 0;
}
