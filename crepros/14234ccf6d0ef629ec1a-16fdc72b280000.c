// https://syzkaller.appspot.com/bug?id=b05ead3cd4968b6d2575de9adad9f1573329f6a3
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

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x26ul, 5ul, 0);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20000080 = 0x26;
  memcpy((void*)0x20000082, "hash\000\000\000\000\000\000\000\000\000\000", 14);
  *(uint32_t*)0x20000090 = 0;
  *(uint32_t*)0x20000094 = 0;
  memcpy((void*)0x20000098,
         "cryptd(crct10dif-generic)"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000",
         64);
  syscall(__NR_bind, r[0], 0x20000080ul, 0x58ul);
  res = syscall(__NR_accept4, r[0], 0ul, 0ul, 0ul);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_accept4, r[1], 0ul, 0ul, 0ul);
  if (res != -1)
    r[2] = res;
  *(uint64_t*)0x20000200 = 0;
  *(uint32_t*)0x20000208 = 0;
  *(uint64_t*)0x20000210 = 0x200001c0;
  *(uint64_t*)0x200001c0 = 0;
  *(uint64_t*)0x200001c8 = 0;
  *(uint64_t*)0x20000218 = 1;
  *(uint64_t*)0x20000220 = 0;
  *(uint64_t*)0x20000228 = 0;
  *(uint32_t*)0x20000230 = 0;
  syscall(__NR_sendmsg, r[2], 0x20000200ul, 0x404c800ul);
  *(uint64_t*)0x20000040 = 0;
  *(uint32_t*)0x20000048 = 0;
  *(uint64_t*)0x20000050 = 0;
  *(uint64_t*)0x20000058 = 0;
  *(uint64_t*)0x20000060 = 0;
  *(uint64_t*)0x20000068 = 0;
  *(uint32_t*)0x20000070 = 0;
  syscall(__NR_recvmsg, r[2], 0x20000040ul, 0x40ul, 0);
  return 0;
}
