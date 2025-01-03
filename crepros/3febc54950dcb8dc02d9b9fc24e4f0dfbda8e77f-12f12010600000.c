// https://syzkaller.appspot.com/bug?id=3febc54950dcb8dc02d9b9fc24e4f0dfbda8e77f
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

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  use_temporary_dir();
  intptr_t res = 0;
  *(uint32_t*)0x20000180 = 0xc;
  *(uint32_t*)0x20000184 = 4;
  *(uint64_t*)0x20000188 = 0x20000000;
  *(uint8_t*)0x20000000 = 0x18;
  STORE_BY_BITMASK(uint8_t, , 0x20000001, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000001, 0, 4, 4);
  *(uint16_t*)0x20000002 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint8_t*)0x20000008 = 0;
  *(uint8_t*)0x20000009 = 0;
  *(uint16_t*)0x2000000a = 0;
  *(uint32_t*)0x2000000c = 0;
  *(uint8_t*)0x20000010 = 0x61;
  *(uint8_t*)0x20000011 = 0x11;
  *(uint16_t*)0x20000012 = 0xa4;
  *(uint32_t*)0x20000014 = 0;
  *(uint8_t*)0x20000018 = 0x95;
  *(uint8_t*)0x20000019 = 0;
  *(uint16_t*)0x2000001a = 0;
  *(uint32_t*)0x2000001c = 0;
  *(uint64_t*)0x20000190 = 0x200000c0;
  memcpy((void*)0x200000c0, "GPL\000", 4);
  *(uint32_t*)0x20000198 = 4;
  *(uint32_t*)0x2000019c = 0x1000;
  *(uint64_t*)0x200001a0 = 0x20000200;
  *(uint32_t*)0x200001a8 = 0;
  *(uint32_t*)0x200001ac = 0;
  *(uint8_t*)0x200001b0 = 0;
  *(uint8_t*)0x200001b1 = 0;
  *(uint8_t*)0x200001b2 = 0;
  *(uint8_t*)0x200001b3 = 0;
  *(uint8_t*)0x200001b4 = 0;
  *(uint8_t*)0x200001b5 = 0;
  *(uint8_t*)0x200001b6 = 0;
  *(uint8_t*)0x200001b7 = 0;
  *(uint8_t*)0x200001b8 = 0;
  *(uint8_t*)0x200001b9 = 0;
  *(uint8_t*)0x200001ba = 0;
  *(uint8_t*)0x200001bb = 0;
  *(uint8_t*)0x200001bc = 0;
  *(uint8_t*)0x200001bd = 0;
  *(uint8_t*)0x200001be = 0;
  *(uint8_t*)0x200001bf = 0;
  *(uint32_t*)0x200001c0 = 0;
  *(uint32_t*)0x200001c4 = 0;
  *(uint32_t*)0x200001c8 = -1;
  *(uint32_t*)0x200001cc = 8;
  *(uint64_t*)0x200001d0 = 0;
  *(uint32_t*)0x200001d8 = 0;
  *(uint32_t*)0x200001dc = 0x10;
  *(uint64_t*)0x200001e0 = 0;
  *(uint32_t*)0x200001e8 = 0;
  res = syscall(__NR_bpf, 5, 0x20000180, 0x70);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000040 = r[0];
  *(uint32_t*)0x20000044 = 0;
  *(uint32_t*)0x20000048 = 0x8f;
  *(uint32_t*)0x2000004c = 0;
  *(uint64_t*)0x20000050 = 0x20001200;
  memcpy((void*)0x20001200,
         "\x75\x9a\x33\x2a\x4d\xc0\xc2\x49\x99\xc0\x99\x3f\x9b\x29\x01\xe9\x95"
         "\x6d\xad\x4a\x4c\xdd\x48\x0f\x7b\x27\x0d\x99\x56\x8b\x8d\x3b\x33\x0f"
         "\xad\x73\xb4\x47\xd6\x6a\x16\x48\x78\xac\xf9\x95\xe8\x0a\x93\x8e\x25"
         "\xda\x88\x3d\x8f\x66\xa9\x4f\x5b\x84\x29\x40\x50\xc1\x42\xe1\xd5\x93"
         "\x72\x40\xa2\x19\xa1\x30\xec\xde\xb0\xce\x16\xb5\x27\x1a\xe8\x0a\xf2"
         "\x0e\x15\x23\x8e\x42\x63\xea\xaf\x4a\x76\x63\x1c\x70\xf7\xc6\xea\x07"
         "\x3f\x8f\x97\x45\xeb\x61\xfb\x6d\x35\x3e\x23\x14\xf6\xca\x7e\x6f\x4e"
         "\x6a\xe1\xa1\x5b\x45\x64\xbb\x6a\x31\x13\xfe\xa3\xbf\xe6\xd3\x81\x89"
         "\xdf\xf3\x0c\xa5\x0c\xc1\x78",
         143);
  *(uint64_t*)0x20000058 = 0;
  *(uint32_t*)0x20000060 = 0;
  *(uint32_t*)0x20000064 = 0;
  syscall(__NR_bpf, 0xa, 0x20000040, 0x28);
  return 0;
}
