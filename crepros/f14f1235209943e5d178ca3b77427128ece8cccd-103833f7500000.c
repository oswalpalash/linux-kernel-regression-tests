// https://syzkaller.appspot.com/bug?id=f14f1235209943e5d178ca3b77427128ece8cccd
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

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);

  *(uint32_t*)0x20000000 = 9;
  *(uint32_t*)0x20000004 = 0x200000;
  *(uint32_t*)0x20000008 = 0x7ffd;
  *(uint32_t*)0x2000000c = 0x107c;
  *(uint32_t*)0x20000010 = 0;
  *(uint32_t*)0x20000014 = 1;
  *(uint32_t*)0x20000018 = 0;
  *(uint8_t*)0x2000001c = 0;
  *(uint8_t*)0x2000001d = 0;
  *(uint8_t*)0x2000001e = 0;
  *(uint8_t*)0x2000001f = 0;
  *(uint8_t*)0x20000020 = 0;
  *(uint8_t*)0x20000021 = 0;
  *(uint8_t*)0x20000022 = 0;
  *(uint8_t*)0x20000023 = 0;
  *(uint8_t*)0x20000024 = 0;
  *(uint8_t*)0x20000025 = 0;
  *(uint8_t*)0x20000026 = 0;
  *(uint8_t*)0x20000027 = 0;
  *(uint8_t*)0x20000028 = 0;
  *(uint8_t*)0x20000029 = 0;
  *(uint8_t*)0x2000002a = 0;
  *(uint8_t*)0x2000002b = 0;
  *(uint32_t*)0x2000002c = 0;
  *(uint32_t*)0x20000030 = -1;
  *(uint32_t*)0x20000034 = 0;
  *(uint32_t*)0x20000038 = 0;
  *(uint32_t*)0x2000003c = 0;
  syscall(__NR_bpf, 0xd00000000000000ul, 0x20000000ul, 0x40ul);
  return 0;
}
