// https://syzkaller.appspot.com/bug?id=3d423b1e2dff471bafd33c63ed2c3efda80dceaf
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

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  res = syscall(__NR_socket, 2ul, 1ul, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x2000e000 = 0xe;
  *(uint32_t*)0x2000e004 = 4;
  *(uint64_t*)0x2000e008 = 0x20000040;
  memcpy((void*)0x20000040,
         "\xb4\x05\x00\x00\x20\x00\x00\x00\x61\x10\x60\x00\x00\x00\x00\x00\xc6"
         "\x00\x00\x00\x00\x00\x00\x00\x95\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint64_t*)0x2000e010 = 0x20003ff6;
  memcpy((void*)0x20003ff6, "GPL\000", 4);
  *(uint32_t*)0x2000e018 = 4;
  *(uint32_t*)0x2000e01c = 0xfd90;
  *(uint64_t*)0x2000e020 = 0x2000cf3d;
  *(uint32_t*)0x2000e028 = 0;
  *(uint32_t*)0x2000e02c = 0;
  memset((void*)0x2000e030, 0, 16);
  *(uint32_t*)0x2000e040 = 0;
  *(uint32_t*)0x2000e044 = 0;
  *(uint32_t*)0x2000e048 = -1;
  *(uint32_t*)0x2000e04c = 8;
  *(uint64_t*)0x2000e050 = 0x20000000;
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x2000e058 = 0x366;
  *(uint32_t*)0x2000e05c = 0x10;
  *(uint64_t*)0x2000e060 = 0x20000000;
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x2000000c = 0;
  *(uint32_t*)0x2000e068 = 0x1dd;
  *(uint32_t*)0x2000e06c = 0;
  *(uint32_t*)0x2000e070 = -1;
  *(uint32_t*)0x2000e074 = 0;
  *(uint64_t*)0x2000e078 = 0;
  res = syscall(__NR_bpf, 5ul, 0x2000e000ul, 0x48ul);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x20000200 = 0x12;
  *(uint32_t*)0x20000204 = 4;
  *(uint32_t*)0x20000208 = 4;
  *(uint32_t*)0x2000020c = 0x12;
  *(uint32_t*)0x20000210 = 0;
  *(uint32_t*)0x20000214 = -1;
  *(uint32_t*)0x20000218 = 0;
  memset((void*)0x2000021c, 0, 16);
  *(uint32_t*)0x2000022c = 0;
  *(uint32_t*)0x20000230 = -1;
  *(uint32_t*)0x20000234 = 0;
  *(uint32_t*)0x20000238 = 0;
  *(uint32_t*)0x2000023c = 0;
  *(uint64_t*)0x20000240 = 0;
  res = syscall(__NR_bpf, 0ul, 0x20000200ul, 0x48ul);
  if (res != -1)
    r[2] = res;
  *(uint32_t*)0x20000140 = r[2];
  *(uint32_t*)0x20000144 = r[1];
  *(uint32_t*)0x20000148 = 5;
  *(uint32_t*)0x2000014c = 0;
  syscall(__NR_bpf, 8ul, 0x20000140ul, 0x10ul);
  *(uint16_t*)0x200000c0 = 2;
  *(uint16_t*)0x200000c2 = htobe16(0x4e20);
  *(uint32_t*)0x200000c4 = htobe32(0xe0000002);
  syscall(__NR_bind, r[0], 0x200000c0ul, 0x10ul);
  *(uint16_t*)0x20000080 = 2;
  *(uint16_t*)0x20000082 = htobe16(0x4e20);
  *(uint32_t*)0x20000084 = htobe32(0);
  syscall(__NR_sendto, r[0], 0ul, 0ul, 0x20020001ul, 0x20000080ul, 0x10ul);
  *(uint32_t*)0x20000380 = r[2];
  *(uint64_t*)0x20000388 = 0x200002c0;
  *(uint64_t*)0x20000390 = 0x20000340;
  *(uint32_t*)0x20000340 = r[0];
  *(uint64_t*)0x20000398 = 0;
  syscall(__NR_bpf, 2ul, 0x20000380ul, 0x20ul);
  syscall(__NR_shutdown, r[0], 1ul);
  return 0;
}
