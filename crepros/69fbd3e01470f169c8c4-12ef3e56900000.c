// https://syzkaller.appspot.com/bug?id=0d7a0da1557dcd1989e00cb3692b26d4173b4132
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
  *(uint32_t*)0x200001c0 = 0;
  *(uint32_t*)0x200001c4 = 0;
  *(uint32_t*)0x200001c8 = 0;
  *(uint32_t*)0x200001cc = 0;
  *(uint32_t*)0x200001d0 = 0;
  *(uint32_t*)0x200001d4 = 0;
  *(uint32_t*)0x200001d8 = 8;
  *(uint32_t*)0x200001dc = 0;
  *(uint32_t*)0x200001e0 = 0;
  *(uint32_t*)0x200001e4 = 0xffffff00;
  *(uint32_t*)0x200001e8 = 0;
  *(uint32_t*)0x200001ec = 0;
  *(uint32_t*)0x200001f0 = 0;
  *(uint32_t*)0x200001f4 = 0;
  *(uint32_t*)0x200001f8 = 0;
  *(uint32_t*)0x200001fc = 0;
  *(uint32_t*)0x20000200 = 0;
  *(uint32_t*)0x20000204 = 0;
  *(uint32_t*)0x20000208 = 0;
  *(uint32_t*)0x2000020c = 0;
  *(uint32_t*)0x20000210 = 5;
  *(uint32_t*)0x20000214 = 0;
  *(uint32_t*)0x20000218 = 0;
  *(uint32_t*)0x2000021c = 0x80;
  *(uint32_t*)0x20000220 = 0;
  *(uint32_t*)0x20000224 = 0;
  *(uint32_t*)0x20000228 = 0;
  *(uint32_t*)0x2000022c = 0;
  *(uint32_t*)0x20000230 = 0;
  *(uint32_t*)0x20000234 = 0;
  *(uint32_t*)0x20000238 = 0;
  *(uint32_t*)0x2000023c = 0;
  *(uint32_t*)0x20000240 = 0;
  *(uint32_t*)0x20000244 = 0;
  *(uint32_t*)0x20000248 = 2;
  *(uint32_t*)0x2000024c = 0;
  *(uint32_t*)0x20000250 = 0;
  *(uint32_t*)0x20000254 = 0;
  *(uint32_t*)0x20000258 = 0;
  *(uint32_t*)0x2000025c = 0;
  syscall(__NR_ioctl, (intptr_t)r[0], 0x4601, 0x200001c0);
  return 0;
}
