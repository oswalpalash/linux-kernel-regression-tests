// https://syzkaller.appspot.com/bug?id=2e5f3ff8a156f4fcba5d0d5362bde5c669a93662
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
#define __NR_mmap 192
#endif
#ifndef __NR_openat
#define __NR_openat 295
#endif
#ifndef __NR_write
#define __NR_write 4
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000, 0x1000, 0, 0x32, -1, 0);
  syscall(__NR_mmap, 0x20000000, 0x1000000, 7, 0x32, -1, 0);
  syscall(__NR_mmap, 0x21000000, 0x1000, 0, 0x32, -1, 0);
  intptr_t res = 0;
  memcpy((void*)0x20000300, "/sys/kernel/profiling", 21);
  res = syscall(__NR_openat, 0xffffff9c, 0x20000300, 0x41, 0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x200000c0, "1000000\000", 8);
  syscall(__NR_write, (intptr_t)r[0], 0x200000c0, 0xffffff82);
  return 0;
}
