// https://syzkaller.appspot.com/bug?id=fadb9ffad365c212f0396cf8965f12f7c35b1886
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

uint64_t r[1] = {0xffffffffffffffff};
void loop()
{
  long res = 0;
  res = syscall(__NR_socket, 0x10, 3, 0);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20005000 = 0x20000040;
  *(uint16_t*)0x20000040 = 0x10;
  *(uint16_t*)0x20000042 = 0;
  *(uint32_t*)0x20000044 = 0;
  *(uint32_t*)0x20000048 = 0;
  *(uint32_t*)0x20005008 = 0xc;
  *(uint64_t*)0x20005010 = 0x209ecff0;
  *(uint64_t*)0x209ecff0 = 0x20000080;
  *(uint32_t*)0x20000080 = 0x20;
  *(uint16_t*)0x20000084 = 0x1e;
  *(uint16_t*)0x20000086 = 0x2ff;
  *(uint32_t*)0x20000088 = 0;
  *(uint32_t*)0x2000008c = 0;
  *(uint8_t*)0x20000090 = 0;
  *(uint8_t*)0x20000091 = 0;
  *(uint16_t*)0x20000092 = 0;
  *(uint16_t*)0x20000094 = 8;
  *(uint16_t*)0x20000096 = 0;
  *(uint32_t*)0x20000098 = 0;
  *(uint16_t*)0x2000009c = 4;
  *(uint16_t*)0x2000009e = 9;
  *(uint64_t*)0x209ecff8 = 0x20;
  *(uint64_t*)0x20005018 = 1;
  *(uint64_t*)0x20005020 = 0;
  *(uint64_t*)0x20005028 = 0;
  *(uint32_t*)0x20005030 = 0;
  syscall(__NR_sendmsg, r[0], 0x20005000, 0);
  *(uint8_t*)0x20000240 = -1;
  *(uint8_t*)0x20000241 = -1;
  *(uint8_t*)0x20000242 = -1;
  *(uint8_t*)0x20000243 = -1;
  *(uint8_t*)0x20000244 = -1;
  *(uint8_t*)0x20000245 = -1;
  *(uint8_t*)0x20000246 = 0xaa;
  *(uint8_t*)0x20000247 = 0xaa;
  *(uint8_t*)0x20000248 = 0xaa;
  *(uint8_t*)0x20000249 = 0xaa;
  *(uint8_t*)0x2000024a = 0xaa;
  *(uint8_t*)0x2000024b = 0;
  *(uint16_t*)0x2000024c = htobe16(0x806);
  *(uint16_t*)0x2000024e = htobe16(1);
  *(uint16_t*)0x20000250 = htobe16(0x800);
  *(uint8_t*)0x20000252 = 6;
  *(uint8_t*)0x20000253 = 4;
  *(uint16_t*)0x20000254 = htobe16(0);
  memcpy((void*)0x20000256, "\xbb\xbf\xed\xd1\x03\xc2", 6);
  *(uint32_t*)0x2000025c = htobe32(0xe0000002);
  memcpy((void*)0x20000260, "\x84\x9b\x7f\x7f\x30\x53", 6);
  *(uint32_t*)0x20000266 = htobe32(0xe0000001);
  *(uint32_t*)0x20000280 = 0;
  *(uint32_t*)0x20000284 = 0;
  *(uint32_t*)0x20000288 = 0;
  *(uint32_t*)0x2000028c = 0;
  *(uint32_t*)0x20000290 = 0;
  *(uint32_t*)0x20000294 = 0;
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
