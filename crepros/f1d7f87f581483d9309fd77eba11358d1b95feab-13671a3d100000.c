// https://syzkaller.appspot.com/bug?id=f1d7f87f581483d9309fd77eba11358d1b95feab
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

#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_openat
#define __NR_openat 295
#endif
#ifndef __NR_pwritev
#define __NR_pwritev 334
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
  memcpy((void*)0x200001c0, "/dev/qrtr-tun\000", 14);
  res = syscall(__NR_openat, 0xffffff9c, 0x200001c0, 1, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000440 = 0x20000380;
  memcpy((void*)0x20000380, "\x03\xd4\x4b\xe7", 4);
  *(uint32_t*)0x20000444 = 4;
  syscall(__NR_pwritev, (intptr_t)r[0], 0x20000440, 1, 0);
  return 0;
}
