// https://syzkaller.appspot.com/bug?id=44fa54548362cb84e26da7c1bbd356c86c54f36d
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
  res = syscall(__NR_socket, 0xa, 2, 0);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000200 = htobe64(0);
  *(uint64_t*)0x20000208 = htobe64(1);
  *(uint8_t*)0x20000210 = 0xfe;
  *(uint8_t*)0x20000211 = 0x80;
  *(uint8_t*)0x20000212 = 0;
  *(uint8_t*)0x20000213 = 0;
  *(uint8_t*)0x20000214 = 0;
  *(uint8_t*)0x20000215 = 0;
  *(uint8_t*)0x20000216 = 0;
  *(uint8_t*)0x20000217 = 0;
  *(uint8_t*)0x20000218 = 0;
  *(uint8_t*)0x20000219 = 0;
  *(uint8_t*)0x2000021a = 0;
  *(uint8_t*)0x2000021b = 0;
  *(uint8_t*)0x2000021c = 0;
  *(uint8_t*)0x2000021d = 0;
  *(uint8_t*)0x2000021e = 0;
  *(uint8_t*)0x2000021f = 0;
  *(uint16_t*)0x20000220 = htobe16(0);
  *(uint16_t*)0x20000222 = htobe16(0);
  *(uint16_t*)0x20000224 = htobe16(0);
  *(uint16_t*)0x20000226 = htobe16(0);
  *(uint16_t*)0x20000228 = 2;
  *(uint8_t*)0x2000022a = 0;
  *(uint8_t*)0x2000022b = 0;
  *(uint8_t*)0x2000022c = 0;
  *(uint32_t*)0x20000230 = 0;
  *(uint32_t*)0x20000234 = 0;
  *(uint64_t*)0x20000238 = 0;
  *(uint64_t*)0x20000240 = 0;
  *(uint64_t*)0x20000248 = 0;
  *(uint64_t*)0x20000250 = 0;
  *(uint64_t*)0x20000258 = 6;
  *(uint64_t*)0x20000260 = 0;
  *(uint64_t*)0x20000268 = 0;
  *(uint64_t*)0x20000270 = 0;
  *(uint64_t*)0x20000278 = 0;
  *(uint64_t*)0x20000280 = 0;
  *(uint64_t*)0x20000288 = 0;
  *(uint64_t*)0x20000290 = 0;
  *(uint32_t*)0x20000298 = 0;
  *(uint32_t*)0x2000029c = 0;
  *(uint8_t*)0x200002a0 = 1;
  *(uint8_t*)0x200002a1 = 0;
  *(uint8_t*)0x200002a2 = 0;
  *(uint8_t*)0x200002a3 = 0;
  *(uint8_t*)0x200002a8 = 0xfe;
  *(uint8_t*)0x200002a9 = 0x80;
  *(uint8_t*)0x200002aa = 0;
  *(uint8_t*)0x200002ab = 0;
  *(uint8_t*)0x200002ac = 0;
  *(uint8_t*)0x200002ad = 0;
  *(uint8_t*)0x200002ae = 0;
  *(uint8_t*)0x200002af = 0;
  *(uint8_t*)0x200002b0 = 0;
  *(uint8_t*)0x200002b1 = 0;
  *(uint8_t*)0x200002b2 = 0;
  *(uint8_t*)0x200002b3 = 0;
  *(uint8_t*)0x200002b4 = 0;
  *(uint8_t*)0x200002b5 = 0;
  *(uint8_t*)0x200002b6 = 0;
  *(uint8_t*)0x200002b7 = 0xaa;
  *(uint32_t*)0x200002b8 = htobe32(0);
  *(uint8_t*)0x200002bc = -1;
  *(uint16_t*)0x200002c0 = 0xa;
  *(uint8_t*)0x200002c4 = 0;
  *(uint8_t*)0x200002c5 = 0;
  *(uint8_t*)0x200002c6 = 0;
  *(uint8_t*)0x200002c7 = 0;
  *(uint8_t*)0x200002c8 = 0;
  *(uint8_t*)0x200002c9 = 0;
  *(uint8_t*)0x200002ca = 0;
  *(uint8_t*)0x200002cb = 0;
  *(uint8_t*)0x200002cc = 0;
  *(uint8_t*)0x200002cd = 0;
  *(uint8_t*)0x200002ce = 0;
  *(uint8_t*)0x200002cf = 0;
  *(uint8_t*)0x200002d0 = 0;
  *(uint8_t*)0x200002d1 = 0;
  *(uint8_t*)0x200002d2 = 0;
  *(uint8_t*)0x200002d3 = 0;
  *(uint32_t*)0x200002d4 = 1;
  *(uint8_t*)0x200002d8 = 3;
  *(uint8_t*)0x200002d9 = 0;
  *(uint8_t*)0x200002da = 0xfe;
  *(uint32_t*)0x200002dc = 0;
  *(uint32_t*)0x200002e0 = 0;
  *(uint32_t*)0x200002e4 = 0;
  syscall(__NR_setsockopt, r[0], 0x29, 0x23, 0x20000200, 0xe8);
  *(uint64_t*)0x20000a80 = 0x20000100;
  *(uint16_t*)0x20000100 = 2;
  *(uint16_t*)0x20000102 = htobe16(0xc67a);
  *(uint32_t*)0x20000104 = htobe32(0);
  *(uint8_t*)0x20000108 = 0;
  *(uint8_t*)0x20000109 = 0;
  *(uint8_t*)0x2000010a = 0;
  *(uint8_t*)0x2000010b = 0;
  *(uint8_t*)0x2000010c = 0;
  *(uint8_t*)0x2000010d = 0;
  *(uint8_t*)0x2000010e = 0;
  *(uint8_t*)0x2000010f = 0;
  *(uint32_t*)0x20000a88 = 0x80;
  *(uint64_t*)0x20000a90 = 0x20000240;
  *(uint64_t*)0x20000a98 = 0;
  *(uint64_t*)0x20000aa0 = 0x20000280;
  *(uint64_t*)0x20000aa8 = 0;
  *(uint32_t*)0x20000ab0 = 0;
  *(uint32_t*)0x20000ab8 = 0;
  syscall(__NR_sendmmsg, r[0], 0x20000a80, 1, 0);
  return 0;
}
