// https://syzkaller.appspot.com/bug?id=e0640a911365faa499433155c3d5d5b674b36a83
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

#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_sendmsg
#define __NR_sendmsg 370
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  res = syscall(__NR_socket, 0x10, 3, 0xc);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20dddfc8 = 0x20000080;
  *(uint16_t*)0x20000080 = 0x10;
  *(uint16_t*)0x20000082 = 0;
  *(uint32_t*)0x20000084 = 0;
  *(uint32_t*)0x20000088 = 0;
  *(uint32_t*)0x20dddfcc = 0xc;
  *(uint32_t*)0x20dddfd0 = 0x208a7000;
  *(uint32_t*)0x208a7000 = 0x20000000;
  memcpy((void*)0x20000000, "\x2c\x00\x00\x00\x04\x08\x05\x01\xff\x00\x80\xff"
                            "\xfd\xff\xfe\x2e\x0a\x00\x00\x00\x0c\x00\x03\x00"
                            "\x88\xff\xff\xff\x7d\x0a\x00\x01\x0c\x00\x02\x00"
                            "\x17\x00\x22\xff\x02\xf1\x00\x00",
         44);
  *(uint32_t*)0x208a7004 = 0x2c;
  *(uint32_t*)0x20dddfd4 = 1;
  *(uint32_t*)0x20dddfd8 = 0;
  *(uint32_t*)0x20dddfdc = 0;
  *(uint32_t*)0x20dddfe0 = 0;
  syscall(__NR_sendmsg, (long)r[0], 0x20dddfc8, 0);
  return 0;
}
