// https://syzkaller.appspot.com/bug?id=8f063539d4ecf1faf3132624b57a641e923ee25a
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

static long syz_open_dev(long a0, long a1, long a2)
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

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
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
  *(uint32_t*)0x200000c0 = 0x7b;
  *(uint32_t*)0x200000c4 = 0xbff;
  *(uint64_t*)0x200000c8 = 0xc2;
  *(uint64_t*)0x200000d0 = 0;
  *(uint64_t*)0x200000d8 = 0x40000105;
  *(uint64_t*)0x200000e0 = 0;
  *(uint8_t*)0x200000e8 = 0;
  *(uint8_t*)0x200000e9 = 0;
  *(uint8_t*)0x200000ea = 0;
  *(uint8_t*)0x200000eb = 0;
  *(uint8_t*)0x200000ec = 0;
  *(uint8_t*)0x200000ed = 0;
  *(uint8_t*)0x200000ee = 0;
  *(uint8_t*)0x200000ef = 0;
  *(uint8_t*)0x200000f0 = 0;
  *(uint8_t*)0x200000f1 = 0;
  *(uint8_t*)0x200000f2 = 0;
  *(uint8_t*)0x200000f3 = 0;
  *(uint8_t*)0x200000f4 = 0;
  *(uint8_t*)0x200000f5 = 0;
  *(uint8_t*)0x200000f6 = 0;
  *(uint8_t*)0x200000f7 = 0;
  *(uint8_t*)0x200000f8 = 0;
  *(uint8_t*)0x200000f9 = 0;
  *(uint8_t*)0x200000fa = 0;
  *(uint8_t*)0x200000fb = 0;
  *(uint8_t*)0x200000fc = 0;
  *(uint8_t*)0x200000fd = 0;
  *(uint8_t*)0x200000fe = 0;
  *(uint8_t*)0x200000ff = 0;
  *(uint8_t*)0x20000100 = 0;
  *(uint8_t*)0x20000101 = 0;
  *(uint8_t*)0x20000102 = 0;
  *(uint8_t*)0x20000103 = 0;
  *(uint8_t*)0x20000104 = 0;
  *(uint8_t*)0x20000105 = 0;
  *(uint8_t*)0x20000106 = 0;
  *(uint8_t*)0x20000107 = 0;
  *(uint8_t*)0x20000108 = 0;
  *(uint8_t*)0x20000109 = 0;
  *(uint8_t*)0x2000010a = 0;
  *(uint8_t*)0x2000010b = 0;
  *(uint8_t*)0x2000010c = 0;
  *(uint8_t*)0x2000010d = 0;
  *(uint8_t*)0x2000010e = 0;
  *(uint8_t*)0x2000010f = 0;
  *(uint8_t*)0x20000110 = 0;
  *(uint8_t*)0x20000111 = 0;
  *(uint8_t*)0x20000112 = 0;
  *(uint8_t*)0x20000113 = 0;
  *(uint8_t*)0x20000114 = 0;
  *(uint8_t*)0x20000115 = 0;
  *(uint8_t*)0x20000116 = 0;
  *(uint8_t*)0x20000117 = 0;
  *(uint8_t*)0x20000118 = 0;
  *(uint8_t*)0x20000119 = 0;
  *(uint8_t*)0x2000011a = 0;
  *(uint8_t*)0x2000011b = 0;
  *(uint8_t*)0x2000011c = 0;
  *(uint8_t*)0x2000011d = 0;
  *(uint8_t*)0x2000011e = 0;
  *(uint8_t*)0x2000011f = 0;
  *(uint8_t*)0x20000120 = 0;
  *(uint8_t*)0x20000121 = 0;
  *(uint8_t*)0x20000122 = 0;
  *(uint8_t*)0x20000123 = 0;
  *(uint8_t*)0x20000124 = 0;
  *(uint8_t*)0x20000125 = 0;
  *(uint8_t*)0x20000126 = 0;
  *(uint8_t*)0x20000127 = 0;
  syscall(__NR_ioctl, r[2], 0x4080aebf, 0x200000c0);
  memcpy((void*)0x20000000, "/dev/loop#", 11);
  syz_open_dev(0x20000000, 0, 0);
  return 0;
}
