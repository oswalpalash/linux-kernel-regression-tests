// https://syzkaller.appspot.com/bug?id=c4bd8ccfda8dc2245acfe073ea51d74b03c9c0fd
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

#ifndef __NR_ioctl
#define __NR_ioctl 54
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_openat
#define __NR_openat 295
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
  memcpy((void*)0x20000080, "/dev/fb0\000", 9);
  res = syscall(__NR_openat, 0xffffff9c, 0x20000080, 0, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000200 = 0x356;
  *(uint32_t*)0x20000204 = 8;
  *(uint32_t*)0x20000208 = 0x600;
  *(uint32_t*)0x2000020c = 0x10000000;
  *(uint32_t*)0x20000210 = 0;
  *(uint32_t*)0x20000214 = 0;
  *(uint32_t*)0x20000218 = 4;
  *(uint32_t*)0x2000021c = 0;
  *(uint32_t*)0x20000220 = 0x3000000;
  *(uint32_t*)0x20000224 = 0;
  *(uint32_t*)0x20000228 = 0;
  *(uint32_t*)0x2000022c = 0;
  *(uint32_t*)0x20000230 = 0;
  *(uint32_t*)0x20000234 = 0;
  *(uint32_t*)0x20000238 = 0;
  *(uint32_t*)0x2000023c = 0;
  *(uint32_t*)0x20000240 = 0;
  *(uint32_t*)0x20000244 = 0;
  *(uint32_t*)0x20000248 = 0;
  *(uint32_t*)0x2000024c = 0;
  *(uint32_t*)0x20000250 = 0;
  *(uint32_t*)0x20000254 = 0;
  *(uint32_t*)0x20000258 = 0;
  *(uint32_t*)0x2000025c = 0;
  *(uint32_t*)0x20000260 = 0;
  *(uint32_t*)0x20000264 = 0;
  *(uint32_t*)0x20000268 = 0;
  *(uint32_t*)0x2000026c = 0;
  *(uint32_t*)0x20000270 = 0;
  *(uint32_t*)0x20000274 = 0;
  *(uint32_t*)0x20000278 = 0;
  *(uint32_t*)0x2000027c = 0;
  *(uint32_t*)0x20000280 = 0;
  *(uint32_t*)0x20000284 = 0;
  *(uint32_t*)0x20000288 = 0;
  *(uint32_t*)0x2000028c = 0;
  memset((void*)0x20000290, 0, 16);
  syscall(__NR_ioctl, (intptr_t)r[0], 0x4601, 0x20000200);
  return 0;
}
