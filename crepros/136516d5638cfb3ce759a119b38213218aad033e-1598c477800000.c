// https://syzkaller.appspot.com/bug?id=136516d5638cfb3ce759a119b38213218aad033e
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[1] = {0xffffffffffffffff};
void loop()
{
  long res = 0;
  *(uint32_t*)0x200000c0 = 0x12;
  *(uint32_t*)0x200000c4 = 0xfffffffd;
  *(uint32_t*)0x200000c8 = 4;
  *(uint32_t*)0x200000cc = 4;
  *(uint32_t*)0x200000d0 = 0;
  *(uint32_t*)0x200000d4 = -1;
  *(uint32_t*)0x200000d8 = 0;
  *(uint8_t*)0x200000dc = 0;
  *(uint8_t*)0x200000dd = 0;
  *(uint8_t*)0x200000de = 0;
  *(uint8_t*)0x200000df = 0;
  *(uint8_t*)0x200000e0 = 0;
  *(uint8_t*)0x200000e1 = 0;
  *(uint8_t*)0x200000e2 = 0;
  *(uint8_t*)0x200000e3 = 0;
  *(uint8_t*)0x200000e4 = 0;
  *(uint8_t*)0x200000e5 = 0;
  *(uint8_t*)0x200000e6 = 0;
  *(uint8_t*)0x200000e7 = 0;
  *(uint8_t*)0x200000e8 = 0;
  *(uint8_t*)0x200000e9 = 0;
  *(uint8_t*)0x200000ea = 0;
  *(uint8_t*)0x200000eb = 0;
  res = syscall(__NR_bpf, 0, 0x200000c0, 0x2c);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000000 = r[0];
  *(uint64_t*)0x20000008 = 0x20000240;
  syscall(__NR_bpf, 3, 0x20000000, 0x10);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
