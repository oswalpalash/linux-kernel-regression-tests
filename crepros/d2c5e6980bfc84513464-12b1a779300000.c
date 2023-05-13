// https://syzkaller.appspot.com/bug?id=517fa734b92b7db404c409b924cf5c997640e324
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

static void use_temporary_dir(void)
{
  char tmpdir_template[] = "./syzkaller.XXXXXX";
  char* tmpdir = mkdtemp(tmpdir_template);
  if (!tmpdir)
    exit(1);
  if (chmod(tmpdir, 0777))
    exit(1);
  if (chdir(tmpdir))
    exit(1);
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  use_temporary_dir();
  intptr_t res = 0;
  res = syscall(__NR_socket, 0xful, 3ul, 2);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000040 = 3;
  *(uint32_t*)0x20000048 = 0;
  *(uint64_t*)0x20000050 = 0x20000340;
  *(uint64_t*)0x20000340 = 0x20000100;
  *(uint8_t*)0x20000100 = 2;
  *(uint8_t*)0x20000101 = 3;
  *(uint8_t*)0x20000102 = 0;
  *(uint8_t*)0x20000103 = 3;
  *(uint16_t*)0x20000104 = 0xf;
  *(uint16_t*)0x20000106 = 0;
  *(uint32_t*)0x20000108 = 0;
  *(uint32_t*)0x2000010c = 0;
  *(uint16_t*)0x20000110 = 1;
  *(uint16_t*)0x20000112 = 9;
  *(uint16_t*)0x20000114 = 0;
  *(uint16_t*)0x20000116 = 0;
  *(uint16_t*)0x20000118 = 5;
  *(uint16_t*)0x2000011a = 6;
  *(uint8_t*)0x2000011c = 0;
  *(uint8_t*)0x2000011d = 0;
  *(uint16_t*)0x2000011e = 0;
  *(uint16_t*)0x20000120 = 0xa;
  *(uint16_t*)0x20000122 = htobe16(0);
  *(uint32_t*)0x20000124 = htobe32(0);
  *(uint64_t*)0x20000128 = htobe64(0);
  *(uint64_t*)0x20000130 = htobe64(1);
  *(uint32_t*)0x20000138 = 0;
  *(uint16_t*)0x20000140 = 2;
  *(uint16_t*)0x20000142 = 1;
  *(uint32_t*)0x20000144 = htobe32(0);
  *(uint8_t*)0x20000148 = 0;
  *(uint8_t*)0x20000149 = 0;
  *(uint8_t*)0x2000014a = 0;
  *(uint8_t*)0x2000014b = 0xb;
  *(uint32_t*)0x2000014c = 0;
  *(uint16_t*)0x20000150 = 5;
  *(uint16_t*)0x20000152 = 5;
  *(uint8_t*)0x20000154 = 0;
  *(uint8_t*)0x20000155 = 0;
  *(uint16_t*)0x20000156 = 0;
  *(uint16_t*)0x20000158 = 0xa;
  *(uint16_t*)0x2000015a = htobe16(0);
  *(uint32_t*)0x2000015c = htobe32(0);
  *(uint64_t*)0x20000160 = htobe64(0);
  *(uint64_t*)0x20000168 = htobe64(1);
  *(uint32_t*)0x20000170 = 0;
  *(uint64_t*)0x20000348 = 0x78;
  *(uint64_t*)0x20000058 = 1;
  *(uint64_t*)0x20000060 = 7;
  *(uint64_t*)0x20000068 = 0;
  *(uint32_t*)0x20000070 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000040ul, 0ul);
  res = syscall(__NR_socket, 0xaul, 3ul, 6);
  if (res != -1)
    r[1] = res;
  *(uint16_t*)0x200000c0 = 0xa;
  *(uint16_t*)0x200000c2 = htobe16(0);
  *(uint32_t*)0x200000c4 = htobe32(0);
  *(uint64_t*)0x200000c8 = htobe64(0);
  *(uint64_t*)0x200000d0 = htobe64(1);
  *(uint32_t*)0x200000d8 = 0;
  syscall(__NR_connect, r[1], 0x200000c0ul, 0x1cul);
  *(uint32_t*)0x20000100 = htobe32(0);
  *(uint8_t*)0x20000110 = 0xac;
  *(uint8_t*)0x20000111 = 0x1e;
  *(uint8_t*)0x20000112 = 0;
  *(uint8_t*)0x20000113 = 1;
  *(uint16_t*)0x20000120 = htobe16(0);
  *(uint16_t*)0x20000122 = htobe16(0);
  *(uint16_t*)0x20000124 = htobe16(0);
  *(uint16_t*)0x20000126 = htobe16(0);
  *(uint16_t*)0x20000128 = 0xa;
  *(uint8_t*)0x2000012a = 0;
  *(uint8_t*)0x2000012b = 0;
  *(uint8_t*)0x2000012c = 0;
  *(uint32_t*)0x20000130 = 0;
  *(uint32_t*)0x20000134 = -1;
  *(uint64_t*)0x20000138 = 0;
  *(uint64_t*)0x20000140 = 0;
  *(uint64_t*)0x20000148 = 0;
  *(uint64_t*)0x20000150 = 0;
  *(uint64_t*)0x20000158 = 0;
  *(uint64_t*)0x20000160 = 0;
  *(uint64_t*)0x20000168 = 0;
  *(uint64_t*)0x20000170 = 0;
  *(uint64_t*)0x20000178 = 0;
  *(uint64_t*)0x20000180 = 0;
  *(uint64_t*)0x20000188 = 0;
  *(uint64_t*)0x20000190 = 0;
  *(uint32_t*)0x20000198 = 0;
  *(uint32_t*)0x2000019c = 0;
  *(uint8_t*)0x200001a0 = 1;
  *(uint8_t*)0x200001a1 = 0;
  *(uint8_t*)0x200001a2 = 0;
  *(uint8_t*)0x200001a3 = 0;
  *(uint8_t*)0x200001a8 = -1;
  *(uint8_t*)0x200001a9 = 1;
  memset((void*)0x200001aa, 0, 13);
  *(uint8_t*)0x200001b7 = 1;
  *(uint32_t*)0x200001b8 = htobe32(0);
  *(uint8_t*)0x200001bc = 0x32;
  *(uint16_t*)0x200001c0 = 0;
  *(uint8_t*)0x200001c4 = 0xfc;
  *(uint8_t*)0x200001c5 = 0;
  memset((void*)0x200001c6, 0, 13);
  *(uint8_t*)0x200001d3 = 0;
  *(uint32_t*)0x200001d4 = 0;
  *(uint8_t*)0x200001d8 = 0;
  *(uint8_t*)0x200001d9 = 0;
  *(uint8_t*)0x200001da = 0;
  *(uint32_t*)0x200001dc = 0;
  *(uint32_t*)0x200001e0 = 0;
  *(uint32_t*)0x200001e4 = 0;
  syscall(__NR_setsockopt, r[1], 0x29, 0x23, 0x20000100ul, 0xe8ul);
  *(uint64_t*)0x20000500 = 0;
  *(uint32_t*)0x20000508 = 0;
  *(uint64_t*)0x20000510 = 0x20000480;
  *(uint64_t*)0x20000480 = 0x20000000;
  memcpy((void*)0x20000000, "\xf2\xec", 2);
  *(uint64_t*)0x20000488 = 0xffd0;
  *(uint64_t*)0x20000518 = 1;
  *(uint64_t*)0x20000520 = 0;
  *(uint64_t*)0x20000528 = 0;
  *(uint32_t*)0x20000530 = 0;
  syscall(__NR_sendmsg, r[1], 0x20000500ul, 0ul);
  return 0;
}
