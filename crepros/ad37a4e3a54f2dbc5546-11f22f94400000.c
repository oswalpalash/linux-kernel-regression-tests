// https://syzkaller.appspot.com/bug?id=ae239a8b52cf47d202f7ca93d3e861499f9dffcd
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  res = syscall(__NR_pipe2, 0x20000100, 0);
  if (res != -1) {
    r[0] = *(uint32_t*)0x20000100;
    r[1] = *(uint32_t*)0x20000104;
  }
  *(uint32_t*)0x20000300 = 7;
  *(uint8_t*)0x20000304 = 0x4d;
  *(uint16_t*)0x20000305 = -1;
  syscall(__NR_write, r[1], 0x20000300, 7);
  memcpy((void*)0x20000140, "./file0", 8);
  syscall(__NR_mkdir, 0x20000140, 0);
  memcpy((void*)0x200000c0, "./file0", 8);
  memcpy((void*)0x20000340, "9p", 3);
  memcpy((void*)0x200001c0, "trans=fd,", 9);
  memcpy((void*)0x200001c9, "rfdno", 5);
  *(uint8_t*)0x200001ce = 0x3d;
  sprintf((char*)0x200001cf, "0x%016llx", (long long)r[0]);
  *(uint8_t*)0x200001e1 = 0x2c;
  memcpy((void*)0x200001e2, "wfdno", 5);
  *(uint8_t*)0x200001e7 = 0x3d;
  sprintf((char*)0x200001e8, "0x%016llx", (long long)r[1]);
  *(uint8_t*)0x200001fa = 0x2c;
  *(uint8_t*)0x200001fb = 0;
  syscall(__NR_mount, 0, 0x200000c0, 0x20000340, 0, 0x200001c0);
  return 0;
}
