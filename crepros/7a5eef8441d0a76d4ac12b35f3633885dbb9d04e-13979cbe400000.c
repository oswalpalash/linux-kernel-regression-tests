// https://syzkaller.appspot.com/bug?id=7a5eef8441d0a76d4ac12b35f3633885dbb9d04e
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

uint64_t r[2] = {0x0, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  res = syscall(__NR_io_setup, 3, 0x20000240);
  if (res != -1)
    r[0] = *(uint64_t*)0x20000240;
  memcpy((void*)0x20000000, "/dev/md0", 9);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000000, 0, 0);
  if (res != -1)
    r[1] = res;
  syscall(__NR_close, r[1]);
  memcpy((void*)0x20000180, "/dev/cuse", 10);
  syscall(__NR_openat, 0xffffffffffffff9c, 0x20000180, 2, 0);
  *(uint64_t*)0x20000b00 = 0x200002c0;
  *(uint64_t*)0x200002c0 = 0;
  *(uint32_t*)0x200002c8 = 0;
  *(uint32_t*)0x200002cc = 0;
  *(uint16_t*)0x200002d0 = 5;
  *(uint16_t*)0x200002d2 = 0;
  *(uint32_t*)0x200002d4 = r[1];
  *(uint64_t*)0x200002d8 = 0;
  *(uint64_t*)0x200002e0 = 0;
  *(uint64_t*)0x200002e8 = 0;
  *(uint64_t*)0x200002f0 = 0;
  *(uint32_t*)0x200002f8 = 0;
  *(uint32_t*)0x200002fc = -1;
  syscall(__NR_io_submit, r[0], 1, 0x20000b00);
  return 0;
}
