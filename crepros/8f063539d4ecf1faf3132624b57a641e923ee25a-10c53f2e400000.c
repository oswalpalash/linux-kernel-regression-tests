// https://syzkaller.appspot.com/bug?id=8f063539d4ecf1faf3132624b57a641e923ee25a
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
  syscall(__NR_socket, 2, 3, 0xb);
  memcpy((void*)0x20000000, "/dev/kvm", 9);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000000, 0, 0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_ioctl, r[0], 0xae01, 0);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_ioctl, r[1], 0xae41, 0);
  if (res != -1)
    r[2] = res;
  syscall(__NR_ioctl, r[2], 0x4080aebf, 0x20000040);
  *(uint32_t*)0x200000c0 = 0;
  syscall(__NR_sched_setscheduler, 0, 5, 0x200000c0);
  *(uint16_t*)0x20000100 = 0;
  *(uint8_t*)0x20000102 = 0;
  *(uint8_t*)0x20000103 = 0;
  syscall(__NR_setsockopt, -1, 0x107, 0x12, 0x20000100, 4);
  return 0;
}
