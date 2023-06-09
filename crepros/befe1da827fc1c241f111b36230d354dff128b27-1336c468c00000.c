// https://syzkaller.appspot.com/bug?id=befe1da827fc1c241f111b36230d354dff128b27
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

#ifndef __NR_memfd_create
#define __NR_memfd_create 319
#endif

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x20000000, "/dev/loop#\000", 11);
  res = syz_open_dev(0x20000000, 0, 0x105082);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000100,
         "\000\000\000\000\214\000\000\000\000\0004\334\223\220\000", 15);
  res = syscall(__NR_memfd_create, 0x20000100, 1);
  if (res != -1)
    r[1] = res;
  *(uint64_t*)0x200004c0 = 0x20000480;
  memcpy((void*)0x20000480, "\xa8", 1);
  *(uint64_t*)0x200004c8 = 1;
  syscall(__NR_pwritev, r[1], 0x200004c0, 1, 0x81004);
  syscall(__NR_ioctl, r[0], 0x4c00, r[1]);
  syscall(__NR_sendfile, r[0], r[0], 0, 0x102000000);
  return 0;
}
