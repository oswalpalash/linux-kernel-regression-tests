// https://syzkaller.appspot.com/bug?id=8f4c492a5a2ba52c107a01f408c5402b3b9f0ec8
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
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  intptr_t res = 0;
  memcpy((void*)0x20000040, "/dev/uinput\000", 12);
  res = syscall(__NR_openat, /*fd=*/0xffffffffffffff9cul, /*file=*/0x20000040ul,
                /*flags=O_RDWR_NONBLOCK*/ 0x802ul, /*mode=*/0ul);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20000340 = 0x2f;
  *(uint32_t*)0x20000344 = 0;
  *(uint32_t*)0x20000348 = 0;
  *(uint32_t*)0x2000034c = 0;
  *(uint32_t*)0x20000350 = 0;
  *(uint32_t*)0x20000354 = 0;
  *(uint32_t*)0x20000358 = 0;
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0x401c5504, /*arg=*/0x20000340ul);
  memcpy((void*)0x20000e00,
         "syz1\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000",
         80);
  *(uint16_t*)0x20000e50 = 0;
  *(uint16_t*)0x20000e52 = 0;
  *(uint16_t*)0x20000e54 = 0;
  *(uint16_t*)0x20000e56 = 0;
  *(uint32_t*)0x20000e58 = 0;
  *(uint32_t*)0x20000e5c = 0;
  *(uint32_t*)0x20000e60 = 0;
  *(uint32_t*)0x20000e64 = 0;
  *(uint32_t*)0x20000e68 = 0;
  *(uint32_t*)0x20000e6c = 0;
  *(uint32_t*)0x20000e70 = 0;
  *(uint32_t*)0x20000e74 = 0;
  *(uint32_t*)0x20000e78 = 0;
  *(uint32_t*)0x20000e7c = 0;
  *(uint32_t*)0x20000e80 = 0;
  *(uint32_t*)0x20000e84 = 0;
  *(uint32_t*)0x20000e88 = 0;
  *(uint32_t*)0x20000e8c = 0;
  *(uint32_t*)0x20000e90 = 0;
  *(uint32_t*)0x20000e94 = 0;
  *(uint32_t*)0x20000e98 = 0;
  *(uint32_t*)0x20000e9c = 0;
  *(uint32_t*)0x20000ea0 = 0;
  *(uint32_t*)0x20000ea4 = 0;
  *(uint32_t*)0x20000ea8 = 0;
  *(uint32_t*)0x20000eac = 0;
  *(uint32_t*)0x20000eb0 = 0;
  *(uint32_t*)0x20000eb4 = 0;
  *(uint32_t*)0x20000eb8 = 0;
  *(uint32_t*)0x20000ebc = 0;
  *(uint32_t*)0x20000ec0 = 0;
  *(uint32_t*)0x20000ec4 = 0;
  *(uint32_t*)0x20000ec8 = 0;
  *(uint32_t*)0x20000ecc = 0;
  *(uint32_t*)0x20000ed0 = 0;
  *(uint32_t*)0x20000ed4 = 0;
  *(uint32_t*)0x20000ed8 = 0;
  *(uint32_t*)0x20000edc = 0;
  *(uint32_t*)0x20000ee0 = 0;
  *(uint32_t*)0x20000ee4 = 0;
  *(uint32_t*)0x20000ee8 = 0;
  *(uint32_t*)0x20000eec = 0;
  *(uint32_t*)0x20000ef0 = 0;
  *(uint32_t*)0x20000ef4 = 0;
  *(uint32_t*)0x20000ef8 = 0;
  *(uint32_t*)0x20000efc = 0;
  *(uint32_t*)0x20000f00 = 0;
  *(uint32_t*)0x20000f04 = 0;
  *(uint32_t*)0x20000f08 = 0;
  *(uint32_t*)0x20000f0c = 0;
  *(uint32_t*)0x20000f10 = 0;
  *(uint32_t*)0x20000f14 = 0;
  *(uint32_t*)0x20000f18 = 0;
  *(uint32_t*)0x20000f1c = 0;
  *(uint32_t*)0x20000f20 = 0;
  *(uint32_t*)0x20000f24 = 0;
  *(uint32_t*)0x20000f28 = 0;
  *(uint32_t*)0x20000f2c = 0;
  *(uint32_t*)0x20000f30 = 0;
  *(uint32_t*)0x20000f34 = 0;
  *(uint32_t*)0x20000f38 = 0;
  *(uint32_t*)0x20000f3c = 0;
  *(uint32_t*)0x20000f40 = 0;
  *(uint32_t*)0x20000f44 = 0;
  *(uint32_t*)0x20000f48 = 0;
  *(uint32_t*)0x20000f4c = 0;
  *(uint32_t*)0x20000f50 = 0;
  *(uint32_t*)0x20000f54 = 0;
  *(uint32_t*)0x20000f58 = 0;
  *(uint32_t*)0x20000f5c = 0;
  *(uint32_t*)0x20000f60 = 0;
  *(uint32_t*)0x20000f64 = 0;
  *(uint32_t*)0x20000f68 = 0;
  *(uint32_t*)0x20000f6c = 0;
  *(uint32_t*)0x20000f70 = 0;
  *(uint32_t*)0x20000f74 = 0;
  *(uint32_t*)0x20000f78 = 0;
  *(uint32_t*)0x20000f7c = 0;
  *(uint32_t*)0x20000f80 = 0;
  *(uint32_t*)0x20000f84 = 0;
  *(uint32_t*)0x20000f88 = 0;
  *(uint32_t*)0x20000f8c = 0;
  *(uint32_t*)0x20000f90 = 0;
  *(uint32_t*)0x20000f94 = 0;
  *(uint32_t*)0x20000f98 = 0;
  *(uint32_t*)0x20000f9c = 0;
  *(uint32_t*)0x20000fa0 = 0;
  *(uint32_t*)0x20000fa4 = 0;
  *(uint32_t*)0x20000fa8 = 0;
  *(uint32_t*)0x20000fac = 0;
  *(uint32_t*)0x20000fb0 = 0;
  *(uint32_t*)0x20000fb4 = 0;
  *(uint32_t*)0x20000fb8 = 0;
  *(uint32_t*)0x20000fbc = 0;
  *(uint32_t*)0x20000fc0 = 0;
  *(uint32_t*)0x20000fc4 = 0;
  *(uint32_t*)0x20000fc8 = 0;
  *(uint32_t*)0x20000fcc = 0;
  *(uint32_t*)0x20000fd0 = 0;
  *(uint32_t*)0x20000fd4 = 0;
  *(uint32_t*)0x20000fd8 = 0;
  *(uint32_t*)0x20000fdc = 0;
  *(uint32_t*)0x20000fe0 = 0;
  *(uint32_t*)0x20000fe4 = 0;
  *(uint32_t*)0x20000fe8 = 0;
  *(uint32_t*)0x20000fec = 0;
  *(uint32_t*)0x20000ff0 = 0;
  *(uint32_t*)0x20000ff4 = 0;
  *(uint32_t*)0x20000ff8 = 0;
  *(uint32_t*)0x20000ffc = 0;
  *(uint32_t*)0x20001000 = 0;
  *(uint32_t*)0x20001004 = 0;
  *(uint32_t*)0x20001008 = 0;
  *(uint32_t*)0x2000100c = 0;
  *(uint32_t*)0x20001010 = 0;
  *(uint32_t*)0x20001014 = 0;
  *(uint32_t*)0x20001018 = 0;
  *(uint32_t*)0x2000101c = 0;
  *(uint32_t*)0x20001020 = 0;
  *(uint32_t*)0x20001024 = 0;
  *(uint32_t*)0x20001028 = 0;
  *(uint32_t*)0x2000102c = 0;
  *(uint32_t*)0x20001030 = 0;
  *(uint32_t*)0x20001034 = 0;
  *(uint32_t*)0x20001038 = 0;
  *(uint32_t*)0x2000103c = 0;
  *(uint32_t*)0x20001040 = 0;
  *(uint32_t*)0x20001044 = 0;
  *(uint32_t*)0x20001048 = 0;
  *(uint32_t*)0x2000104c = 0;
  *(uint32_t*)0x20001050 = 0;
  *(uint32_t*)0x20001054 = 0;
  *(uint32_t*)0x20001058 = 0;
  *(uint32_t*)0x2000105c = 0;
  *(uint32_t*)0x20001060 = 0;
  *(uint32_t*)0x20001064 = 0;
  *(uint32_t*)0x20001068 = 0;
  *(uint32_t*)0x2000106c = 0;
  *(uint32_t*)0x20001070 = 0;
  *(uint32_t*)0x20001074 = 0;
  *(uint32_t*)0x20001078 = 0;
  *(uint32_t*)0x2000107c = 0;
  *(uint32_t*)0x20001080 = 0;
  *(uint32_t*)0x20001084 = 0;
  *(uint32_t*)0x20001088 = 0;
  *(uint32_t*)0x2000108c = 0;
  *(uint32_t*)0x20001090 = 0;
  *(uint32_t*)0x20001094 = 0;
  *(uint32_t*)0x20001098 = 0;
  *(uint32_t*)0x2000109c = 0;
  *(uint32_t*)0x200010a0 = 0;
  *(uint32_t*)0x200010a4 = 0;
  *(uint32_t*)0x200010a8 = 0;
  *(uint32_t*)0x200010ac = 0;
  *(uint32_t*)0x200010b0 = 0;
  *(uint32_t*)0x200010b4 = 0;
  *(uint32_t*)0x200010b8 = 0;
  *(uint32_t*)0x200010bc = 0;
  *(uint32_t*)0x200010c0 = 0;
  *(uint32_t*)0x200010c4 = 0;
  *(uint32_t*)0x200010c8 = 0;
  *(uint32_t*)0x200010cc = 0;
  *(uint32_t*)0x200010d0 = 0;
  *(uint32_t*)0x200010d4 = 0;
  *(uint32_t*)0x200010d8 = 0;
  *(uint32_t*)0x200010dc = 0;
  *(uint32_t*)0x200010e0 = 0;
  *(uint32_t*)0x200010e4 = 0;
  *(uint32_t*)0x200010e8 = 0;
  *(uint32_t*)0x200010ec = 0;
  *(uint32_t*)0x200010f0 = 0;
  *(uint32_t*)0x200010f4 = 0;
  *(uint32_t*)0x200010f8 = 0;
  *(uint32_t*)0x200010fc = 0;
  *(uint32_t*)0x20001100 = 0;
  *(uint32_t*)0x20001104 = 0;
  *(uint32_t*)0x20001108 = 0;
  *(uint32_t*)0x2000110c = 0;
  *(uint32_t*)0x20001110 = 0;
  *(uint32_t*)0x20001114 = 0;
  *(uint32_t*)0x20001118 = 0;
  *(uint32_t*)0x2000111c = 0;
  *(uint32_t*)0x20001120 = 0;
  *(uint32_t*)0x20001124 = 0;
  *(uint32_t*)0x20001128 = 0;
  *(uint32_t*)0x2000112c = 0;
  *(uint32_t*)0x20001130 = 0;
  *(uint32_t*)0x20001134 = 0;
  *(uint32_t*)0x20001138 = 0;
  *(uint32_t*)0x2000113c = 0;
  *(uint32_t*)0x20001140 = 0;
  *(uint32_t*)0x20001144 = 0;
  *(uint32_t*)0x20001148 = 0;
  *(uint32_t*)0x2000114c = 0;
  *(uint32_t*)0x20001150 = 0;
  *(uint32_t*)0x20001154 = 0;
  *(uint32_t*)0x20001158 = 0;
  *(uint32_t*)0x2000115c = 0;
  *(uint32_t*)0x20001160 = 0;
  *(uint32_t*)0x20001164 = 0;
  *(uint32_t*)0x20001168 = 0;
  *(uint32_t*)0x2000116c = 0;
  *(uint32_t*)0x20001170 = 0;
  *(uint32_t*)0x20001174 = 0;
  *(uint32_t*)0x20001178 = 0;
  *(uint32_t*)0x2000117c = 0;
  *(uint32_t*)0x20001180 = 0;
  *(uint32_t*)0x20001184 = 0;
  *(uint32_t*)0x20001188 = 0;
  *(uint32_t*)0x2000118c = 0;
  *(uint32_t*)0x20001190 = 0;
  *(uint32_t*)0x20001194 = 0;
  *(uint32_t*)0x20001198 = 0;
  *(uint32_t*)0x2000119c = 0;
  *(uint32_t*)0x200011a0 = 0;
  *(uint32_t*)0x200011a4 = 0;
  *(uint32_t*)0x200011a8 = 0;
  *(uint32_t*)0x200011ac = 0;
  *(uint32_t*)0x200011b0 = 0;
  *(uint32_t*)0x200011b4 = 0;
  *(uint32_t*)0x200011b8 = 0;
  *(uint32_t*)0x200011bc = 0;
  *(uint32_t*)0x200011c0 = 0;
  *(uint32_t*)0x200011c4 = 0;
  *(uint32_t*)0x200011c8 = 0;
  *(uint32_t*)0x200011cc = 0;
  *(uint32_t*)0x200011d0 = 0;
  *(uint32_t*)0x200011d4 = 0;
  *(uint32_t*)0x200011d8 = 0;
  *(uint32_t*)0x200011dc = 0;
  *(uint32_t*)0x200011e0 = 0;
  *(uint32_t*)0x200011e4 = 0;
  *(uint32_t*)0x200011e8 = 0;
  *(uint32_t*)0x200011ec = 0;
  *(uint32_t*)0x200011f0 = 0;
  *(uint32_t*)0x200011f4 = 0;
  *(uint32_t*)0x200011f8 = 0;
  *(uint32_t*)0x200011fc = 0;
  *(uint32_t*)0x20001200 = 0;
  *(uint32_t*)0x20001204 = 0;
  *(uint32_t*)0x20001208 = 0;
  *(uint32_t*)0x2000120c = 0;
  *(uint32_t*)0x20001210 = 0;
  *(uint32_t*)0x20001214 = 0;
  *(uint32_t*)0x20001218 = 0;
  *(uint32_t*)0x2000121c = 0;
  *(uint32_t*)0x20001220 = 0;
  *(uint32_t*)0x20001224 = 0;
  *(uint32_t*)0x20001228 = 0;
  *(uint32_t*)0x2000122c = 0;
  *(uint32_t*)0x20001230 = 0;
  *(uint32_t*)0x20001234 = 0;
  *(uint32_t*)0x20001238 = 0;
  *(uint32_t*)0x2000123c = 0;
  *(uint32_t*)0x20001240 = 0;
  *(uint32_t*)0x20001244 = 0;
  *(uint32_t*)0x20001248 = 0;
  *(uint32_t*)0x2000124c = 0;
  *(uint32_t*)0x20001250 = 0;
  *(uint32_t*)0x20001254 = 0;
  *(uint32_t*)0x20001258 = 0;
  syscall(__NR_write, /*fd=*/r[0], /*data=*/0x20000e00ul, /*len=*/0x45cul);
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0x40045564, /*arg=*/3ul);
  memcpy((void*)0x20000980,
         "syz0\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000",
         80);
  *(uint16_t*)0x200009d0 = 0;
  *(uint16_t*)0x200009d2 = 0;
  *(uint16_t*)0x200009d4 = 0;
  *(uint16_t*)0x200009d6 = 0;
  *(uint32_t*)0x200009d8 = 0;
  *(uint32_t*)0x200009dc = 0;
  *(uint32_t*)0x200009e0 = 0;
  *(uint32_t*)0x200009e4 = 0;
  *(uint32_t*)0x200009e8 = 0;
  *(uint32_t*)0x200009ec = 0;
  *(uint32_t*)0x200009f0 = 0;
  *(uint32_t*)0x200009f4 = 0;
  *(uint32_t*)0x200009f8 = 0;
  *(uint32_t*)0x200009fc = 0;
  *(uint32_t*)0x20000a00 = 0;
  *(uint32_t*)0x20000a04 = 0;
  *(uint32_t*)0x20000a08 = 0;
  *(uint32_t*)0x20000a0c = 0;
  *(uint32_t*)0x20000a10 = 0;
  *(uint32_t*)0x20000a14 = 0;
  *(uint32_t*)0x20000a18 = 0;
  *(uint32_t*)0x20000a1c = 0;
  *(uint32_t*)0x20000a20 = 0;
  *(uint32_t*)0x20000a24 = 0;
  *(uint32_t*)0x20000a28 = 0;
  *(uint32_t*)0x20000a2c = 0;
  *(uint32_t*)0x20000a30 = 0;
  *(uint32_t*)0x20000a34 = 0;
  *(uint32_t*)0x20000a38 = 0;
  *(uint32_t*)0x20000a3c = 0;
  *(uint32_t*)0x20000a40 = 0;
  *(uint32_t*)0x20000a44 = 0;
  *(uint32_t*)0x20000a48 = 0;
  *(uint32_t*)0x20000a4c = 0;
  *(uint32_t*)0x20000a50 = 0;
  *(uint32_t*)0x20000a54 = 0;
  *(uint32_t*)0x20000a58 = 0;
  *(uint32_t*)0x20000a5c = 0;
  *(uint32_t*)0x20000a60 = 0;
  *(uint32_t*)0x20000a64 = 0;
  *(uint32_t*)0x20000a68 = 0;
  *(uint32_t*)0x20000a6c = 0;
  *(uint32_t*)0x20000a70 = 0;
  *(uint32_t*)0x20000a74 = 0;
  *(uint32_t*)0x20000a78 = 0;
  *(uint32_t*)0x20000a7c = 0;
  *(uint32_t*)0x20000a80 = 0;
  *(uint32_t*)0x20000a84 = 0;
  *(uint32_t*)0x20000a88 = 0;
  *(uint32_t*)0x20000a8c = 0;
  *(uint32_t*)0x20000a90 = 0;
  *(uint32_t*)0x20000a94 = 0;
  *(uint32_t*)0x20000a98 = 0x80000001;
  *(uint32_t*)0x20000a9c = 0;
  *(uint32_t*)0x20000aa0 = 0;
  *(uint32_t*)0x20000aa4 = 0;
  *(uint32_t*)0x20000aa8 = 0;
  *(uint32_t*)0x20000aac = 0;
  *(uint32_t*)0x20000ab0 = 0;
  *(uint32_t*)0x20000ab4 = 0;
  *(uint32_t*)0x20000ab8 = 0;
  *(uint32_t*)0x20000abc = 0;
  *(uint32_t*)0x20000ac0 = 0;
  *(uint32_t*)0x20000ac4 = 0;
  *(uint32_t*)0x20000ac8 = 0;
  *(uint32_t*)0x20000acc = 0;
  *(uint32_t*)0x20000ad0 = 0;
  *(uint32_t*)0x20000ad4 = 0;
  *(uint32_t*)0x20000ad8 = 0;
  *(uint32_t*)0x20000adc = 0;
  *(uint32_t*)0x20000ae0 = 0;
  *(uint32_t*)0x20000ae4 = 0;
  *(uint32_t*)0x20000ae8 = 0;
  *(uint32_t*)0x20000aec = 0;
  *(uint32_t*)0x20000af0 = 0;
  *(uint32_t*)0x20000af4 = 0;
  *(uint32_t*)0x20000af8 = 0;
  *(uint32_t*)0x20000afc = 0;
  *(uint32_t*)0x20000b00 = 0;
  *(uint32_t*)0x20000b04 = 0;
  *(uint32_t*)0x20000b08 = 0;
  *(uint32_t*)0x20000b0c = 0;
  *(uint32_t*)0x20000b10 = 0;
  *(uint32_t*)0x20000b14 = 0;
  *(uint32_t*)0x20000b18 = 0;
  *(uint32_t*)0x20000b1c = 0;
  *(uint32_t*)0x20000b20 = 0;
  *(uint32_t*)0x20000b24 = 0;
  *(uint32_t*)0x20000b28 = 0;
  *(uint32_t*)0x20000b2c = 0;
  *(uint32_t*)0x20000b30 = 0;
  *(uint32_t*)0x20000b34 = 0;
  *(uint32_t*)0x20000b38 = 0;
  *(uint32_t*)0x20000b3c = 0;
  *(uint32_t*)0x20000b40 = 0;
  *(uint32_t*)0x20000b44 = 0;
  *(uint32_t*)0x20000b48 = 0;
  *(uint32_t*)0x20000b4c = 0;
  *(uint32_t*)0x20000b50 = 0;
  *(uint32_t*)0x20000b54 = 0;
  *(uint32_t*)0x20000b58 = 0;
  *(uint32_t*)0x20000b5c = 0;
  *(uint32_t*)0x20000b60 = 0;
  *(uint32_t*)0x20000b64 = 0;
  *(uint32_t*)0x20000b68 = 0;
  *(uint32_t*)0x20000b6c = 0;
  *(uint32_t*)0x20000b70 = 0;
  *(uint32_t*)0x20000b74 = 0;
  *(uint32_t*)0x20000b78 = 0;
  *(uint32_t*)0x20000b7c = 0;
  *(uint32_t*)0x20000b80 = 0;
  *(uint32_t*)0x20000b84 = 0;
  *(uint32_t*)0x20000b88 = 0;
  *(uint32_t*)0x20000b8c = 0;
  *(uint32_t*)0x20000b90 = 0;
  *(uint32_t*)0x20000b94 = 0;
  *(uint32_t*)0x20000b98 = 0;
  *(uint32_t*)0x20000b9c = 0;
  *(uint32_t*)0x20000ba0 = 0;
  *(uint32_t*)0x20000ba4 = 0;
  *(uint32_t*)0x20000ba8 = 0;
  *(uint32_t*)0x20000bac = 0;
  *(uint32_t*)0x20000bb0 = 0;
  *(uint32_t*)0x20000bb4 = 0;
  *(uint32_t*)0x20000bb8 = 0;
  *(uint32_t*)0x20000bbc = 0;
  *(uint32_t*)0x20000bc0 = 0;
  *(uint32_t*)0x20000bc4 = 0;
  *(uint32_t*)0x20000bc8 = 0;
  *(uint32_t*)0x20000bcc = 0;
  *(uint32_t*)0x20000bd0 = 0;
  *(uint32_t*)0x20000bd4 = 0;
  *(uint32_t*)0x20000bd8 = 0;
  *(uint32_t*)0x20000bdc = 0;
  *(uint32_t*)0x20000be0 = 0;
  *(uint32_t*)0x20000be4 = 0;
  *(uint32_t*)0x20000be8 = 0;
  *(uint32_t*)0x20000bec = 0;
  *(uint32_t*)0x20000bf0 = 0;
  *(uint32_t*)0x20000bf4 = 0;
  *(uint32_t*)0x20000bf8 = 0;
  *(uint32_t*)0x20000bfc = 0;
  *(uint32_t*)0x20000c00 = 0;
  *(uint32_t*)0x20000c04 = 0;
  *(uint32_t*)0x20000c08 = 0;
  *(uint32_t*)0x20000c0c = 0;
  *(uint32_t*)0x20000c10 = 0;
  *(uint32_t*)0x20000c14 = 0;
  *(uint32_t*)0x20000c18 = 0;
  *(uint32_t*)0x20000c1c = 0;
  *(uint32_t*)0x20000c20 = 0;
  *(uint32_t*)0x20000c24 = 0;
  *(uint32_t*)0x20000c28 = 0;
  *(uint32_t*)0x20000c2c = 0;
  *(uint32_t*)0x20000c30 = 0;
  *(uint32_t*)0x20000c34 = 0;
  *(uint32_t*)0x20000c38 = 0;
  *(uint32_t*)0x20000c3c = 0;
  *(uint32_t*)0x20000c40 = 0;
  *(uint32_t*)0x20000c44 = 0;
  *(uint32_t*)0x20000c48 = 0;
  *(uint32_t*)0x20000c4c = 0;
  *(uint32_t*)0x20000c50 = 0;
  *(uint32_t*)0x20000c54 = 0;
  *(uint32_t*)0x20000c58 = 0;
  *(uint32_t*)0x20000c5c = 0;
  *(uint32_t*)0x20000c60 = 0;
  *(uint32_t*)0x20000c64 = 0;
  *(uint32_t*)0x20000c68 = 0;
  *(uint32_t*)0x20000c6c = 0;
  *(uint32_t*)0x20000c70 = 0;
  *(uint32_t*)0x20000c74 = 0;
  *(uint32_t*)0x20000c78 = 0;
  *(uint32_t*)0x20000c7c = 0;
  *(uint32_t*)0x20000c80 = 0;
  *(uint32_t*)0x20000c84 = 0;
  *(uint32_t*)0x20000c88 = 0;
  *(uint32_t*)0x20000c8c = 0;
  *(uint32_t*)0x20000c90 = 0;
  *(uint32_t*)0x20000c94 = 0;
  *(uint32_t*)0x20000c98 = 0;
  *(uint32_t*)0x20000c9c = 0;
  *(uint32_t*)0x20000ca0 = 0;
  *(uint32_t*)0x20000ca4 = 0;
  *(uint32_t*)0x20000ca8 = 0;
  *(uint32_t*)0x20000cac = 0;
  *(uint32_t*)0x20000cb0 = 0;
  *(uint32_t*)0x20000cb4 = 0;
  *(uint32_t*)0x20000cb8 = 0;
  *(uint32_t*)0x20000cbc = 0;
  *(uint32_t*)0x20000cc0 = 0;
  *(uint32_t*)0x20000cc4 = 0;
  *(uint32_t*)0x20000cc8 = 0;
  *(uint32_t*)0x20000ccc = 0;
  *(uint32_t*)0x20000cd0 = 0;
  *(uint32_t*)0x20000cd4 = 0;
  *(uint32_t*)0x20000cd8 = 0;
  *(uint32_t*)0x20000cdc = 0;
  *(uint32_t*)0x20000ce0 = 0;
  *(uint32_t*)0x20000ce4 = 0;
  *(uint32_t*)0x20000ce8 = 0;
  *(uint32_t*)0x20000cec = 0;
  *(uint32_t*)0x20000cf0 = 0;
  *(uint32_t*)0x20000cf4 = 0;
  *(uint32_t*)0x20000cf8 = 0;
  *(uint32_t*)0x20000cfc = 0;
  *(uint32_t*)0x20000d00 = 0;
  *(uint32_t*)0x20000d04 = 0;
  *(uint32_t*)0x20000d08 = 0;
  *(uint32_t*)0x20000d0c = 0;
  *(uint32_t*)0x20000d10 = 0;
  *(uint32_t*)0x20000d14 = 0;
  *(uint32_t*)0x20000d18 = 0;
  *(uint32_t*)0x20000d1c = 0;
  *(uint32_t*)0x20000d20 = 0;
  *(uint32_t*)0x20000d24 = 0;
  *(uint32_t*)0x20000d28 = 0;
  *(uint32_t*)0x20000d2c = 0;
  *(uint32_t*)0x20000d30 = 0;
  *(uint32_t*)0x20000d34 = 0;
  *(uint32_t*)0x20000d38 = 0;
  *(uint32_t*)0x20000d3c = 0;
  *(uint32_t*)0x20000d40 = 0;
  *(uint32_t*)0x20000d44 = 0;
  *(uint32_t*)0x20000d48 = 0;
  *(uint32_t*)0x20000d4c = 0;
  *(uint32_t*)0x20000d50 = 0;
  *(uint32_t*)0x20000d54 = 0;
  *(uint32_t*)0x20000d58 = 0;
  *(uint32_t*)0x20000d5c = 0;
  *(uint32_t*)0x20000d60 = 0;
  *(uint32_t*)0x20000d64 = 0;
  *(uint32_t*)0x20000d68 = 0;
  *(uint32_t*)0x20000d6c = 0;
  *(uint32_t*)0x20000d70 = 0;
  *(uint32_t*)0x20000d74 = 0;
  *(uint32_t*)0x20000d78 = 0;
  *(uint32_t*)0x20000d7c = 0;
  *(uint32_t*)0x20000d80 = 0;
  *(uint32_t*)0x20000d84 = 0;
  *(uint32_t*)0x20000d88 = 0;
  *(uint32_t*)0x20000d8c = 0;
  *(uint32_t*)0x20000d90 = 0;
  *(uint32_t*)0x20000d94 = 0;
  *(uint32_t*)0x20000d98 = 0;
  *(uint32_t*)0x20000d9c = 0;
  *(uint32_t*)0x20000da0 = 0;
  *(uint32_t*)0x20000da4 = 0;
  *(uint32_t*)0x20000da8 = 0;
  *(uint32_t*)0x20000dac = 0;
  *(uint32_t*)0x20000db0 = 0;
  *(uint32_t*)0x20000db4 = 0;
  *(uint32_t*)0x20000db8 = 0;
  *(uint32_t*)0x20000dbc = 0;
  *(uint32_t*)0x20000dc0 = 0;
  *(uint32_t*)0x20000dc4 = 0;
  *(uint32_t*)0x20000dc8 = 0;
  *(uint32_t*)0x20000dcc = 0;
  *(uint32_t*)0x20000dd0 = 0;
  *(uint32_t*)0x20000dd4 = 0;
  *(uint32_t*)0x20000dd8 = 0;
  syscall(__NR_write, /*fd=*/r[0], /*data=*/0x20000980ul, /*len=*/0x45cul);
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0x5501, 0);
  return 0;
}
