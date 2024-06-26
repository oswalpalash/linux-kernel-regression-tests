// https://syzkaller.appspot.com/bug?id=953fe83dc9299c195ac5e266b918dd0d986544e3
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul, /*prot=*/7ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  intptr_t res = 0;
  res = syscall(__NR_socket, /*domain=*/0x26ul, /*type=*/5ul, /*proto=*/0);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20000000 = 0x26;
  memcpy((void*)0x20000002, "hash\000\000\000\000\000\000\000\000\000\000", 14);
  *(uint32_t*)0x20000010 = 0;
  *(uint32_t*)0x20000014 = 0;
  memcpy(
      (void*)0x20000018,
      "ghash-"
      "clmulni\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
      "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
      "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000",
      64);
  syscall(__NR_bind, /*fd=*/r[0], /*addr=*/0x20000000ul, /*addrlen=*/0x58ul);
  memset((void*)0x20000440, 0, 16);
  syscall(__NR_setsockopt, /*fd=*/r[0], /*level=*/0x117, /*opt=*/1,
          /*key=*/0x20000440ul, /*keylen=*/0x10ul);
  res = syscall(__NR_accept4, /*fd=*/r[0], /*peer=*/0ul, /*peerlen=*/0ul,
                /*flags=*/0ul);
  if (res != -1)
    r[1] = res;
  *(uint64_t*)0x20000180 = 0;
  *(uint32_t*)0x20000188 = 0;
  *(uint64_t*)0x20000190 = 0;
  *(uint64_t*)0x20000198 = 0;
  *(uint64_t*)0x200001a0 = 0;
  *(uint64_t*)0x200001a8 = 0;
  *(uint32_t*)0x200001b0 = 0;
  *(uint32_t*)0x200001b8 = 0;
  syscall(__NR_sendmmsg, /*fd=*/r[1], /*mmsg=*/0x20000180ul, /*vlen=*/1ul,
          /*f=*/0x20008012ul);
  *(uint64_t*)0x200001c0 = 0;
  *(uint32_t*)0x200001c8 = 0;
  *(uint64_t*)0x200001d0 = 0x20000140;
  *(uint64_t*)0x20000140 = 0x20000100;
  *(uint64_t*)0x20000148 = 0xfffffdef;
  *(uint64_t*)0x200001d8 = 1;
  *(uint64_t*)0x200001e0 = 0;
  *(uint64_t*)0x200001e8 = 0;
  *(uint32_t*)0x200001f0 = 0;
  syscall(__NR_sendmsg, /*fd=*/r[1], /*msg=*/0x200001c0ul, /*f=*/0ul);
  return 0;
}
