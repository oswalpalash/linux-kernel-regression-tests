// https://syzkaller.appspot.com/bug?id=7a4919b3b2bb049b59d5d26996456bda9f12b081
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

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x20000340, "/dev/kvm\000", 9);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000340, 0, 0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_ioctl, r[0], 0xae01, 0);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_ioctl, r[1], 0xae41, 0);
  if (res != -1)
    r[2] = res;
  *(uint32_t*)0x20bf7000 = 0;
  *(uint32_t*)0x20bf7004 = 0;
  *(uint64_t*)0x20bf7008 = 0;
  *(uint64_t*)0x20bf7010 = 0x2000;
  *(uint64_t*)0x20bf7018 = 0x20000000;
  syscall(__NR_ioctl, r[1], 0x4020ae46, 0x20bf7000);
  syscall(__NR_ioctl, r[2], 0xae80, 0);
  return 0;
}
