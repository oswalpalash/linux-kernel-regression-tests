// https://syzkaller.appspot.com/bug?id=0a6d199d4e60c03fe2cb8e2398cc7f4d654eb634
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
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0xaul, 1ul, 0ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000100 = 1;
  syscall(__NR_setsockopt, r[0], 6ul, 0x13ul, 0x20000100ul, 4ul);
  *(uint16_t*)0x20000300 = 0xa;
  *(uint16_t*)0x20000302 = htobe16(0);
  *(uint32_t*)0x20000304 = htobe32(0);
  *(uint8_t*)0x20000308 = 0;
  *(uint8_t*)0x20000309 = 0;
  *(uint8_t*)0x2000030a = 0;
  *(uint8_t*)0x2000030b = 0;
  *(uint8_t*)0x2000030c = 0;
  *(uint8_t*)0x2000030d = 0;
  *(uint8_t*)0x2000030e = 0;
  *(uint8_t*)0x2000030f = 0;
  *(uint8_t*)0x20000310 = 0;
  *(uint8_t*)0x20000311 = 0;
  *(uint8_t*)0x20000312 = -1;
  *(uint8_t*)0x20000313 = -1;
  *(uint8_t*)0x20000314 = 0xac;
  *(uint8_t*)0x20000315 = 0x14;
  *(uint8_t*)0x20000316 = 0x14;
  *(uint8_t*)0x20000317 = 0xaa;
  *(uint32_t*)0x20000318 = 0;
  syscall(__NR_connect, r[0], 0x20000300ul, 0x1cul);
  memcpy((void*)0x20000340, "tls\000", 4);
  syscall(__NR_setsockopt, r[0], 6ul, 0x1ful, 0x20000340ul, 4ul);
  *(uint16_t*)0x20000180 = 0x303;
  *(uint16_t*)0x20000182 = 0x34;
  memcpy((void*)0x20000184, "\xe5\x73\xb0\x37\xde\x86\x16\xdc", 8);
  memcpy((void*)0x2000018c, "\xc4\x08\xee\x5d\xfa\x1b\xb5\xf0\x44\xb4\xea\x4a"
                            "\xb5\x7a\x3a\x93\x29\x43\x17\x02\x43\x33\x3b\x60"
                            "\x85\x93\x35\x87\x82\x86\xb7\xfb",
         32);
  memcpy((void*)0x200001ac, "\x99\x8a\x9d\xe9", 4);
  memcpy((void*)0x200001b0, "\xff\xff\xff\xff\xff\xff\xff\xfe", 8);
  syscall(__NR_setsockopt, r[0], 0x11aul, 1ul, 0x20000180ul, 0x38ul);
  memcpy((void*)0x200001c0, "\x36\x49\x12\x5c\xfe\xf1\x9c\x02\xad\xa8\x8a\x6e"
                            "\x5b\x0f\x90\x85\xd7\x7e\xfe",
         19);
  syscall(__NR_sendto, r[0], 0x200001c0ul, 0xfffffdeful, 0ul, 0ul, 0ul);
  *(uint32_t*)0x200001c0 = 2;
  syscall(__NR_setsockopt, r[0], 0x29ul, 1ul, 0x200001c0ul, 4ul);
  return 0;
}
