// https://syzkaller.appspot.com/bug?id=24452f89446639c901ac07379ccc702808471e8e
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

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  *(uint32_t*)0x20000140 = 0x1e;
  *(uint32_t*)0x20000144 = 0;
  *(uint32_t*)0x20000148 = 0xfffffffe;
  *(uint32_t*)0x2000014c = 0x1c00;
  *(uint32_t*)0x20000150 = 0;
  *(uint32_t*)0x20000154 = 1;
  *(uint32_t*)0x20000158 = 0xfffffffb;
  memset((void*)0x2000015c, 0, 16);
  *(uint32_t*)0x2000016c = 0;
  *(uint32_t*)0x20000170 = -1;
  *(uint32_t*)0x20000174 = 0;
  *(uint32_t*)0x20000178 = 0;
  *(uint32_t*)0x2000017c = 0;
  res = syscall(__NR_bpf, 0ul, 0x20000140ul, 0x40ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000100 = r[0];
  *(uint64_t*)0x20000108 = 0;
  *(uint64_t*)0x20000110 = 0;
  *(uint64_t*)0x20000118 = 0;
  syscall(__NR_bpf, 1ul, 0x20000100ul, 0x20ul);
  return 0;
}
