// https://syzkaller.appspot.com/bug?id=f2d1d7a456381df6affb60da9693d8c9b0fce6f6
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
  memcpy((void*)0x20000080, "/sys/kernel/profiling", 21);
  res = syscall(__NR_openat, 0xffffff9cul, 0x20000080ul, 2ul, 0ul);
  if (res != -1)
    r[0] = res;
  sprintf((char*)0x200001c0, "0x%016llx", (long long)0);
  syscall(__NR_write, r[0], 0x200001c0ul, 0x12ul);
  return 0;
}
