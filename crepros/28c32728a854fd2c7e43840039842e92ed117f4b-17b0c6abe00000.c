// https://syzkaller.appspot.com/bug?id=28c32728a854fd2c7e43840039842e92ed117f4b
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
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  memcpy((void*)0x200002c0, "/dev/kvm\000", 9);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x200002c0ul, 0ul, 0ul);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_ioctl, r[0], 0xae01, 0ul);
  if (res != -1)
    r[1] = res;
  syscall(__NR_ioctl, r[1], 0xae60, 0);
  res = syscall(__NR_ioctl, r[1], 0xae41, 0ul);
  if (res != -1)
    r[2] = res;
  *(uint64_t*)0x20000080 = 4;
  syscall(__NR_ioctl, r[2], 0x4008ae93, 0x20000080ul);
  return 0;
}
