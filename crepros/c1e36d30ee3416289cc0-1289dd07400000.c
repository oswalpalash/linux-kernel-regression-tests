// https://syzkaller.appspot.com/bug?id=c50f1f108950ba0955e75bb0af5c4d28375185ba
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

#ifndef __NR_mkdirat
#define __NR_mkdirat 296
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_mount
#define __NR_mount 21
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

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x20000000, "./file0\x00", 8);
  syscall(__NR_mkdirat, 0xffffff9c, 0x20000000, 0);
  memcpy((void*)0x200000c0, "/dev/fuse\x00", 10);
  res = syscall(__NR_openat, 0xffffff9c, 0x200000c0, 2, 0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000100, "./file0\x00", 8);
  memcpy((void*)0x20000300, "fuse\x00", 5);
  memcpy((void*)0x20000400, "fd", 2);
  *(uint8_t*)0x20000402 = 0x3d;
  sprintf((char*)0x20000403, "0x%016llx", (long long)r[0]);
  *(uint8_t*)0x20000415 = 0x2c;
  memcpy((void*)0x20000416, "rootmode", 8);
  *(uint8_t*)0x2000041e = 0x3d;
  sprintf((char*)0x2000041f, "%023llo", (long long)0x4000);
  *(uint8_t*)0x20000436 = 0x2c;
  memcpy((void*)0x20000437, "user_id", 7);
  *(uint8_t*)0x2000043e = 0x3d;
  sprintf((char*)0x2000043f, "%020llu", (long long)0);
  *(uint8_t*)0x20000453 = 0x2c;
  memcpy((void*)0x20000454, "group_id", 8);
  *(uint8_t*)0x2000045c = 0x3d;
  sprintf((char*)0x2000045d, "%020llu", (long long)0);
  *(uint8_t*)0x20000471 = 0x2c;
  *(uint8_t*)0x20000472 = 0;
  syscall(__NR_mount, 0, 0x20000100, 0x20000300, 0, 0x20000400);
  syscall(__NR_read, (long)r[0], 0x200040c0, 0x1000);
  *(uint32_t*)0x20000240 = 0x10;
  *(uint32_t*)0x20000244 = 0;
  *(uint64_t*)0x20000248 = 2;
  syscall(__NR_write, (long)r[0], 0x20000240, 0x10);
  *(uint32_t*)0x20000080 = 0x30;
  *(uint32_t*)0x20000084 = 5;
  *(uint64_t*)0x20000088 = 0;
  *(uint64_t*)0x20000090 = 0;
  *(uint64_t*)0x20000098 = 1;
  *(uint64_t*)0x200000a0 = 0;
  *(uint32_t*)0x200000a8 = 9;
  *(uint32_t*)0x200000ac = 0;
  syscall(__NR_write, (long)r[0], 0x20000080, 0x30);
  return 0;
}
