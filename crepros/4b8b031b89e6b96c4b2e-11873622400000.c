// https://syzkaller.appspot.com/bug?id=e02419c12131c24e2a957ea050c2ab6dcbbc3270
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

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x20b4508a, "/dev/ashmem", 12);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20b4508a, 0, 0);
  if (res != -1)
    r[0] = res;
  syscall(__NR_ioctl, r[0], 0x40087703, 0xfffffffa);
  syscall(__NR_mmap, 0x206ff000, 0x3000, 0, 0x12, r[0], 0);
  memcpy((void*)0x200011c0, "\x00\x00\x00\x80\x00\x00\x80\x00\x00", 9);
  syscall(__NR_ioctl, r[0], 0x40087708, 0x200011c0);
  syscall(__NR_ioctl, r[0], 0x770a, 0);
  return 0;
}
