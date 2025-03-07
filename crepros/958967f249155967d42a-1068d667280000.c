// https://syzkaller.appspot.com/bug?id=a279e1c1463f955ad678ef3e7b2c967c5638f248
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
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul, /*prot=*/7ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  intptr_t res = 0;
  *(uint64_t*)0x20000000 = 0x20000040;
  *(uint16_t*)0x20000040 = 0xeb9f;
  *(uint8_t*)0x20000042 = 1;
  *(uint8_t*)0x20000043 = 0;
  *(uint32_t*)0x20000044 = 0x18;
  *(uint32_t*)0x20000048 = 0;
  *(uint32_t*)0x2000004c = 0x38;
  *(uint32_t*)0x20000050 = 0x38;
  *(uint32_t*)0x20000054 = 4;
  *(uint32_t*)0x20000058 = 2;
  *(uint16_t*)0x2000005c = 0;
  *(uint8_t*)0x2000005e = 0;
  *(uint8_t*)0x2000005f = 0x11;
  *(uint32_t*)0x20000060 = 4;
  *(uint32_t*)0x20000064 = -1;
  *(uint32_t*)0x20000068 = 0;
  *(uint16_t*)0x2000006c = 0;
  *(uint8_t*)0x2000006e = 0;
  *(uint8_t*)0x2000006f = 2;
  *(uint32_t*)0x20000070 = 0;
  *(uint32_t*)0x20000074 = 2;
  *(uint16_t*)0x20000078 = 0;
  *(uint8_t*)0x2000007a = 0;
  *(uint8_t*)0x2000007b = 0x12;
  *(uint32_t*)0x2000007c = 2;
  *(uint32_t*)0x20000080 = 2;
  *(uint16_t*)0x20000084 = 0;
  *(uint8_t*)0x20000086 = 0;
  *(uint8_t*)0x20000087 = 0xe;
  *(uint32_t*)0x20000088 = 3;
  *(uint32_t*)0x2000008c = 0;
  *(uint8_t*)0x20000090 = 0;
  *(uint8_t*)0x20000091 = 0;
  *(uint8_t*)0x20000092 = 0x61;
  *(uint8_t*)0x20000093 = 0;
  *(uint64_t*)0x20000008 = 0;
  *(uint32_t*)0x20000010 = 0x54;
  *(uint32_t*)0x20000014 = 0;
  *(uint32_t*)0x20000018 = 0;
  res =
      syscall(__NR_bpf, /*cmd=*/0x12ul, /*arg=*/0x20000000ul, /*size=*/0x20ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000180 = 1;
  *(uint32_t*)0x20000184 = 0x80;
  *(uint32_t*)0x20000188 = 2;
  *(uint32_t*)0x2000018c = 8;
  *(uint32_t*)0x20000190 = 0;
  *(uint32_t*)0x20000194 = -1;
  *(uint32_t*)0x20000198 = 0xfffffffb;
  memset((void*)0x2000019c, 0, 16);
  *(uint32_t*)0x200001ac = 0;
  *(uint32_t*)0x200001b0 = r[0];
  *(uint32_t*)0x200001b4 = 1;
  *(uint32_t*)0x200001b8 = 5;
  *(uint32_t*)0x200001bc = 0;
  *(uint64_t*)0x200001c0 = 0;
  syscall(__NR_bpf, /*cmd=*/0ul, /*arg=*/0x20000180ul, /*size=*/0x48ul);
  return 0;
}
