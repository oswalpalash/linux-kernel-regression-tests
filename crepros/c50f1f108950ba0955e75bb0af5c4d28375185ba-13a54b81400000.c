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

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x20000000, "./file0", 8);
  syscall(__NR_mkdirat, 0xffffff9c, 0x20000000, 0);
  memcpy((void*)0x20000100, "/dev/fuse", 10);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000100, 2, 0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000040, "./file0", 8);
  memcpy((void*)0x20000300, "fuse", 5);
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
  syscall(__NR_mount, 0, 0x20000040, 0x20000300, 0, 0x20000400);
  syscall(__NR_read, r[0], 0x200020c0, 0x1000);
  *(uint32_t*)0x20000140 = 0x20;
  *(uint32_t*)0x20000144 = 0;
  *(uint64_t*)0x20000148 = 2;
  *(uint64_t*)0x20000150 = 7;
  *(uint32_t*)0x20000158 = 0;
  *(uint32_t*)0x2000015c = 0;
  syscall(__NR_write, r[0], 0x20000140, 0x20);
  *(uint32_t*)0x20000080 = 0x30;
  *(uint32_t*)0x20000084 = 5;
  *(uint64_t*)0x20000088 = 0;
  *(uint64_t*)0x20000090 = 0;
  *(uint64_t*)0x20000098 = 1;
  *(uint64_t*)0x200000a0 = 0;
  *(uint32_t*)0x200000a8 = 0;
  *(uint32_t*)0x200000ac = 0;
  syscall(__NR_write, r[0], 0x20000080, 0x30);
  return 0;
}
