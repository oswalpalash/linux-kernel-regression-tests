// https://syzkaller.appspot.com/bug?id=2a622455acd7051c6cf85c360cd116118a587726
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

static void execute_one();
extern unsigned long long procid;

void loop()
{
  while (1) {
    execute_one();
  }
}

#ifndef __NR_pipe2
#define __NR_pipe2 331
#endif
#ifndef __NR_open
#define __NR_open 5
#endif
#ifndef __NR_mount
#define __NR_mount 21
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
void execute_one()
{
  long res = 0;
  res = syscall(__NR_pipe2, 0x20000100, 0);
  if (res != -1)
    r[0] = *(uint32_t*)0x20000104;
  memcpy((void*)0x20000340, "./file0", 8);
  res = syscall(__NR_open, 0x20000340, 0x42, 0);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x20000000, "./file0", 8);
  memcpy((void*)0x20000080, "9p", 3);
  memcpy((void*)0x20000380, "trans=fd,", 9);
  memcpy((void*)0x20000389, "rfdno", 5);
  *(uint8_t*)0x2000038e = 0x3d;
  sprintf((char*)0x2000038f, "0x%016llx", (long long)r[1]);
  *(uint8_t*)0x200003a1 = 0x2c;
  memcpy((void*)0x200003a2, "wfdno", 5);
  *(uint8_t*)0x200003a7 = 0x3d;
  sprintf((char*)0x200003a8, "0x%016llx", (long long)r[0]);
  *(uint8_t*)0x200003ba = 0x2c;
  *(uint8_t*)0x200003bb = 0;
  syscall(__NR_mount, 0, 0x20000000, 0x20000080, 0, 0x20000380);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    loop();
  }
}
