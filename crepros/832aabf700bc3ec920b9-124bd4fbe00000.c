// https://syzkaller.appspot.com/bug?id=230369ae4ebfc5ff69b611abf18668602f7cfb12
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

#ifndef __NR_close
#define __NR_close 6
#endif
#ifndef __NR_getpid
#define __NR_getpid 20
#endif
#ifndef __NR_io_setup
#define __NR_io_setup 245
#endif
#ifndef __NR_io_submit
#define __NR_io_submit 248
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_pidfd_open
#define __NR_pidfd_open 434
#endif
#ifndef __NR_socketpair
#define __NR_socketpair 360
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[3] = {0xffffffffffffffff, 0x0, 0x0};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  intptr_t res = 0;
  res = syscall(__NR_socketpair, 1, 1, 0, 0x20000080);
  if (res != -1)
    r[0] = *(uint32_t*)0x20000080;
  syscall(__NR_close, (intptr_t)r[0]);
  res = syscall(__NR_getpid);
  if (res != -1)
    r[1] = res;
  syscall(__NR_pidfd_open, (intptr_t)r[1], 0);
  res = syscall(__NR_io_setup, 0x7ff, 0x200006c0);
  if (res != -1)
    r[2] = *(uint32_t*)0x200006c0;
  *(uint32_t*)0x20000040 = 0x200000c0;
  *(uint64_t*)0x200000c0 = 0;
  *(uint32_t*)0x200000c8 = 0;
  *(uint32_t*)0x200000cc = 0;
  *(uint16_t*)0x200000d0 = 5;
  *(uint16_t*)0x200000d2 = 1;
  *(uint32_t*)0x200000d4 = r[0];
  *(uint64_t*)0x200000d8 = 0;
  *(uint64_t*)0x200000e0 = 0;
  *(uint64_t*)0x200000e8 = 0;
  *(uint64_t*)0x200000f0 = 0;
  *(uint32_t*)0x200000f8 = 0;
  *(uint32_t*)0x200000fc = -1;
  syscall(__NR_io_submit, (intptr_t)r[2], 1, 0x20000040);
  return 0;
}
