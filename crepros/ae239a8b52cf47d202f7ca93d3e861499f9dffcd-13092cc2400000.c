// https://syzkaller.appspot.com/bug?id=ae239a8b52cf47d202f7ca93d3e861499f9dffcd
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
void loop()
{
  long res = 0;
  memcpy((void*)0x200001c0, "./file0", 8);
  syscall(__NR_mkdir, 0x200001c0, 0);
  res = syscall(__NR_pipe2, 0x200030c0, 0);
  if (res != -1) {
    r[0] = *(uint32_t*)0x200030c0;
    r[1] = *(uint32_t*)0x200030c4;
  }
  *(uint32_t*)0x20000040 = 0xb;
  *(uint8_t*)0x20000044 = 0x65;
  *(uint16_t*)0x20000045 = -1;
  *(uint32_t*)0x20000047 = 5;
  *(uint16_t*)0x2000004b = 8;
  memcpy((void*)0x2000004d, "9P2000.u", 8);
  syscall(__NR_write, r[1], 0x20000040, 0x15);
  memcpy((void*)0x20000140, "./file0", 8);
  memcpy((void*)0x20000180, "9p", 3);
  memcpy((void*)0x20000200, "trans=fd,", 9);
  memcpy((void*)0x20000209, "rfdno", 5);
  *(uint8_t*)0x2000020e = 0x3d;
  sprintf((char*)0x2000020f, "0x%016llx", (long long)r[0]);
  *(uint8_t*)0x20000221 = 0x2c;
  memcpy((void*)0x20000222, "wfdno", 5);
  *(uint8_t*)0x20000227 = 0x3d;
  sprintf((char*)0x20000228, "0x%016llx", (long long)r[1]);
  *(uint8_t*)0x2000023a = 0x2c;
  *(uint8_t*)0x2000023b = 0;
  syscall(__NR_mount, 0, 0x20000140, 0x20000180, 0, 0x20000200);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
