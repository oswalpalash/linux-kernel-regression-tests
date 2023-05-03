// https://syzkaller.appspot.com/bug?id=904dd6aad6dd746b275792875fc52385eac81f04
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

#ifndef __NR_madvise
#define __NR_madvise 233
#endif
#ifndef __NR_mmap
#define __NR_mmap 222
#endif
#ifndef __NR_mremap
#define __NR_mremap 216
#endif

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_madvise, 0x20a93000ul, 0x4000ul, 0xeul);
  syscall(__NR_mremap, 0x20a96000ul, 0x1000ul, 0x800000ul, 3ul, 0x20130000ul);
  syscall(__NR_madvise, 0x20400000ul, 0x2000ul, 0x64ul);
  return 0;
}
