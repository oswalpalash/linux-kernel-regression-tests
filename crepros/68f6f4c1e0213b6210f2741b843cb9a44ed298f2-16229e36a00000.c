// https://syzkaller.appspot.com/bug?id=68f6f4c1e0213b6210f2741b843cb9a44ed298f2
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
  intptr_t res = 0;
  *(uint64_t*)0x20000000 = 0x20000080;
  memcpy((void*)0x20000080, "\x39\x00\x00\x00\x13\x00\x09\x00\x69\x00\x00\x00"
                            "\x00\x00\x00\x00\xab\x00\x80\x48\x06",
         21);
  *(uint64_t*)0x20000008 = 0x15;
  syscall(__NR_writev, -1, 0x20000000, 1);
  res = syscall(__NR_socket, 0x10, 3, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000040 = 0x7ff;
  *(uint32_t*)0x20000044 = 0;
  *(uint32_t*)0x20000048 = 0;
  *(uint32_t*)0x2000004c = 0;
  syscall(__NR_setsockopt, r[0], 0x10e, 0xc, 0x20000040, 0x10);
  *(uint64_t*)0x20000240 = 0;
  *(uint32_t*)0x20000248 = 0xffffff4a;
  *(uint64_t*)0x20000250 = 0x20000200;
  *(uint64_t*)0x20000200 = 0x20000080;
  *(uint32_t*)0x20000080 = 0x18;
  *(uint16_t*)0x20000084 = 0x16;
  *(uint16_t*)0x20000086 = 0xa01;
  *(uint32_t*)0x20000088 = 0;
  *(uint32_t*)0x2000008c = 0;
  *(uint8_t*)0x20000090 = 0;
  *(uint8_t*)0x20000091 = 0;
  *(uint16_t*)0x20000092 = 0;
  *(uint64_t*)0x20000208 = 0x18;
  *(uint64_t*)0x20000258 = 1;
  *(uint64_t*)0x20000260 = 0;
  *(uint64_t*)0x20000268 = 0;
  *(uint32_t*)0x20000270 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000240, 0);
  return 0;
}
