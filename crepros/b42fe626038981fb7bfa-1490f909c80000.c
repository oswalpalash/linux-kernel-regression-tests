// https://syzkaller.appspot.com/bug?id=d94a809ad1d24d41b7c4caf892d204dbf1112bff
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  memcpy((void*)0x20000040, "memory.events\000", 14);
  syscall(__NR_openat, 0xffffff9c, 0x20000040ul, 0x275aul, 0ul);
  memcpy((void*)0x20000040, "memory.events\000", 14);
  syscall(__NR_openat, 0xffffff9c, 0x20000040ul, 0x275aul, 0ul);
  memcpy((void*)0x20000240, "./file0\000", 8);
  syscall(__NR_mkdirat, 0xffffff9c, 0x20000240ul, 0ul);
  memcpy((void*)0x200000c0, "./file0\000", 8);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x200000c0ul, 0ul, 0ul);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x200003c0, "./file0\000", 8);
  syscall(__NR_mknodat, r[0], 0x200003c0ul, 0ul, 0);
  memcpy((void*)0x20000180, "./file1\000", 8);
  syscall(__NR_mkdirat, 0xffffff9c, 0x20000180ul, 0ul);
  memcpy((void*)0x200000c0, "./bus\000", 6);
  syscall(__NR_mkdirat, 0xffffff9c, 0x200000c0ul, 0ul);
  memcpy((void*)0x20000040, "./bus\000", 6);
  memcpy((void*)0x200002c0, "overlay\000", 8);
  memcpy((void*)0x20000080,
         "workdir=./file1,lowerdir=./file0,upperdir=./bus,index=on", 56);
  syscall(__NR_mount, 0ul, 0x20000040ul, 0x200002c0ul, 0ul, 0x20000080ul);
  memcpy((void*)0x20000140, "./bus\000", 6);
  syscall(__NR_chdir, 0x20000140ul);
  memcpy((void*)0x20000040, ".\000", 2);
  res = syscall(__NR_openat, 0xffffff9c, 0x20000040ul, 0ul, 0ul);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x20000340, "./file0\000", 8);
  syscall(__NR_openat, r[1], 0x20000340ul, 3ul, 0ul);
  return 0;
}
