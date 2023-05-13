// https://syzkaller.appspot.com/bug?id=35b110cc00ee9d9ea92b6c98acb2aaf36808b63d
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

#ifndef __NR_io_uring_setup
#define __NR_io_uring_setup 425
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);

  *(uint32_t*)0x20000140 = 0;
  *(uint32_t*)0x20000144 = 0;
  *(uint32_t*)0x20000148 = 1;
  *(uint32_t*)0x2000014c = -1;
  *(uint32_t*)0x20000150 = 0x3b6;
  *(uint32_t*)0x20000154 = 0;
  *(uint32_t*)0x20000158 = 0;
  *(uint32_t*)0x2000015c = 0;
  *(uint32_t*)0x20000160 = 0;
  *(uint32_t*)0x20000164 = 0;
  *(uint32_t*)0x20000168 = 0;
  *(uint32_t*)0x2000016c = 0;
  *(uint32_t*)0x20000170 = 0;
  *(uint32_t*)0x20000174 = 0;
  *(uint32_t*)0x20000178 = 0;
  *(uint32_t*)0x2000017c = 0;
  *(uint32_t*)0x20000180 = 0;
  *(uint32_t*)0x20000184 = 0;
  *(uint64_t*)0x20000188 = 0;
  *(uint32_t*)0x20000190 = 0;
  *(uint32_t*)0x20000194 = 0;
  *(uint32_t*)0x20000198 = 0;
  *(uint32_t*)0x2000019c = 0;
  *(uint32_t*)0x200001a0 = 0;
  *(uint32_t*)0x200001a4 = 0;
  *(uint32_t*)0x200001a8 = 0;
  *(uint32_t*)0x200001ac = 0;
  *(uint64_t*)0x200001b0 = 0;
  syscall(__NR_io_uring_setup, 0x14a, 0x20000140);
  return 0;
}
