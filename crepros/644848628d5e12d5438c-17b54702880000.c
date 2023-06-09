// https://syzkaller.appspot.com/bug?id=4a9c99e1f2ab5cbbb916b1ffe11680448d62e115
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

static long syz_open_dev(volatile long a0, volatile long a1, volatile long a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  memcpy((void*)0x20000100, "/dev/kvm\000", 9);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000100ul, 0ul, 0ul);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_ioctl, r[0], 0xae01, 0ul);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x20000180, "/dev/bus/usb/00#/00#\000", 21);
  res = -1;
  res = syz_open_dev(0x20000180, 0x1aa1, 0);
  if (res != -1)
    r[2] = res;
  syscall(__NR_mmap, 0x2000d000ul, 0x2000ul, 0x1000000ul, 0x1012ul, r[2], 0ul);
  *(uint32_t*)0x20000400 = 0;
  *(uint32_t*)0x20000404 = 0;
  *(uint64_t*)0x20000408 = 0;
  *(uint64_t*)0x20000410 = 0x20002000;
  *(uint64_t*)0x20000418 = 0x20000000;
  syscall(__NR_ioctl, r[1], 0x4020ae46, 0x20000400ul);
  res = syscall(__NR_ioctl, r[1], 0xae41, 0ul);
  if (res != -1)
    r[3] = res;
  syscall(__NR_ioctl, r[3], 0xae80, 0ul);
  syscall(__NR_ioctl, r[3], 0xae80, 0ul);
  return 0;
}
