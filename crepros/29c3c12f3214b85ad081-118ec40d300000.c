// https://syzkaller.appspot.com/bug?id=de17cc3f6902e8bf07334a3eb4a7bb30120c9fbb
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  *(uint32_t*)0x200000c0 = 0x12;
  *(uint32_t*)0x200000c4 = 4;
  *(uint32_t*)0x200000c8 = 8;
  *(uint32_t*)0x200000cc = 4;
  *(uint32_t*)0x200000d0 = 0;
  *(uint32_t*)0x200000d4 = -1;
  *(uint32_t*)0x200000d8 = 0;
  memset((void*)0x200000dc, 0, 16);
  *(uint32_t*)0x200000ec = 0;
  *(uint32_t*)0x200000f0 = -1;
  *(uint32_t*)0x200000f4 = 0;
  *(uint32_t*)0x200000f8 = 0;
  *(uint32_t*)0x200000fc = 0;
  res = syscall(__NR_bpf, 0x100000000000000ul, 0x200000c0ul, 0x40ul);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_socket, 2ul, 1ul, 0);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x200006c0 = 1;
  syscall(__NR_setsockopt, r[1], 6, 0x13, 0x200006c0ul, 4ul);
  memcpy((void*)0x200001c0, "syz_tun\000\000\000\000\000\000\000\000\000", 16);
  syscall(__NR_setsockopt, r[1], 1, 0x19, 0x200001c0ul, 0x10ul);
  *(uint16_t*)0x20000140 = 2;
  *(uint16_t*)0x20000142 = htobe16(0);
  *(uint8_t*)0x20000144 = 0xac;
  *(uint8_t*)0x20000145 = 0x1e;
  *(uint8_t*)0x20000146 = 0;
  *(uint8_t*)0x20000147 = 1;
  syscall(__NR_connect, r[1], 0x20000140ul, 0x10ul);
  memcpy((void*)0x20000040, "tls\000", 4);
  syscall(__NR_setsockopt, r[1], 6, 0x1f, 0x20000040ul, 4ul);
  *(uint32_t*)0x20000080 = r[0];
  *(uint64_t*)0x20000088 = 0x20000340;
  *(uint64_t*)0x20000090 = 0x20000100;
  *(uint32_t*)0x20000100 = r[1];
  *(uint64_t*)0x20000098 = 0;
  syscall(__NR_bpf, 2ul, 0x20000080ul, 0x20ul);
  return 0;
}
