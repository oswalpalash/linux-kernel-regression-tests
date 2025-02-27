// https://syzkaller.appspot.com/bug?id=d64830618d0b5ef44cc925225dd9b5cd9b7c0c4d
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

#ifndef __NR_mmap
#define __NR_mmap 222
#endif
#ifndef __NR_mprotect
#define __NR_mprotect 226
#endif
#ifndef __NR_openat
#define __NR_openat 56
#endif
#ifndef __NR_read
#define __NR_read 63
#endif
#ifndef __NR_write
#define __NR_write 64
#endif

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  intptr_t res = 0;
  memcpy((void*)0x200001c0, "/dev/cuse\000", 10);
  res = syscall(__NR_openat, /*fd=*/0xffffffffffffff9cul, /*file=*/0x200001c0ul,
                /*flags=*/2ul, /*mode=*/0ul);
  if (res != -1)
    r[0] = res;
  syscall(__NR_read, /*fd=*/r[0], /*buf=*/0x20000340ul, /*len=*/0x2020ul);
  *(uint32_t*)0x20000300 = 0x28;
  *(uint32_t*)0x20000304 = 7;
  *(uint64_t*)0x20000308 = 0;
  *(uint64_t*)0x20000310 = 0;
  *(uint64_t*)0x20000318 = 0;
  *(uint32_t*)0x20000320 = 0;
  *(uint32_t*)0x20000324 = 0;
  syscall(__NR_write, /*fd=*/r[0], /*arg=*/0x20000300ul, /*len=*/0x28ul);
  syscall(__NR_mprotect, /*addr=*/0x20000000ul, /*len=*/0x4000ul,
          /*prot=PROT_READ*/ 1ul);
  syscall(__NR_read, /*fd=*/r[0], /*buf=*/0x20000340ul, /*len=*/0x2020ul);
  return 0;
}
