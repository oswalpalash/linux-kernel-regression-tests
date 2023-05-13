// https://syzkaller.appspot.com/bug?id=17d5de7f1fcab794cb8c40032f893f52de899324
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

#ifndef __NR_dup
#define __NR_dup 41
#endif
#ifndef __NR_ioctl
#define __NR_ioctl 54
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_openat
#define __NR_openat 295
#endif
#ifndef __NR_read
#define __NR_read 3
#endif
#ifndef __NR_write
#define __NR_write 4
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x20000040, "/dev/ptmx\x00", 10);
  res = syscall(__NR_openat, 0xffffff9c, 0x20000040, 2, 0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_dup, (long)r[0]);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x20000000 = 0xd;
  syscall(__NR_ioctl, (long)r[0], 0x5423, 0x20000000);
  *(uint32_t*)0x20000340 = 7;
  *(uint8_t*)0x20000344 = 0x21;
  *(uint16_t*)0x20000345 = 2;
  syscall(__NR_write, (long)r[1], 0x20000340, 7);
  syscall(__NR_read, (long)r[0], 0x20000080, 0x57);
  return 0;
}
