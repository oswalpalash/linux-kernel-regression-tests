// https://syzkaller.appspot.com/bug?id=4d9dc4ec10e0d7b004645eadc3e99bbc2af67a74
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
  res = syscall(__NR_socket, 0xa, 2, 0);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000200 = htobe64(0);
  *(uint64_t*)0x20000208 = htobe64(1);
  *(uint32_t*)0x20000210 = htobe32(0xe0000002);
  *(uint16_t*)0x20000220 = htobe16(0);
  *(uint16_t*)0x20000222 = htobe16(0);
  *(uint16_t*)0x20000224 = htobe16(0);
  *(uint16_t*)0x20000226 = htobe16(0);
  *(uint16_t*)0x20000228 = 2;
  *(uint8_t*)0x2000022a = 0;
  *(uint8_t*)0x2000022b = 0;
  *(uint8_t*)0x2000022c = 0;
  *(uint32_t*)0x20000230 = 0;
  *(uint32_t*)0x20000234 = 0;
  *(uint64_t*)0x20000238 = 0;
  *(uint64_t*)0x20000240 = 0;
  *(uint64_t*)0x20000248 = 2;
  *(uint64_t*)0x20000250 = 0;
  *(uint64_t*)0x20000258 = 0;
  *(uint64_t*)0x20000260 = 1;
  *(uint64_t*)0x20000268 = 0;
  *(uint64_t*)0x20000270 = 0;
  *(uint64_t*)0x20000278 = 2;
  *(uint64_t*)0x20000280 = 0xfffffffffffffffd;
  *(uint64_t*)0x20000288 = 0;
  *(uint64_t*)0x20000290 = 0;
  *(uint32_t*)0x20000298 = 0;
  *(uint32_t*)0x2000029c = 0;
  *(uint8_t*)0x200002a0 = 1;
  *(uint8_t*)0x200002a1 = 0;
  *(uint8_t*)0x200002a2 = 0;
  *(uint8_t*)0x200002a3 = 0;
  *(uint8_t*)0x200002a8 = 0xfe;
  *(uint8_t*)0x200002a9 = 0x80;
  *(uint8_t*)0x200002aa = 0;
  *(uint8_t*)0x200002ab = 0;
  *(uint8_t*)0x200002ac = 0;
  *(uint8_t*)0x200002ad = 0;
  *(uint8_t*)0x200002ae = 0;
  *(uint8_t*)0x200002af = 0;
  *(uint8_t*)0x200002b0 = 0;
  *(uint8_t*)0x200002b1 = 0;
  *(uint8_t*)0x200002b2 = 0;
  *(uint8_t*)0x200002b3 = 0;
  *(uint8_t*)0x200002b4 = 0;
  *(uint8_t*)0x200002b5 = 0;
  *(uint8_t*)0x200002b6 = 0;
  *(uint8_t*)0x200002b7 = 0xaa;
  *(uint32_t*)0x200002b8 = htobe32(0);
  *(uint8_t*)0x200002bc = 0x2b;
  *(uint16_t*)0x200002c0 = 0xa;
  *(uint8_t*)0x200002c4 = 0xac;
  *(uint8_t*)0x200002c5 = 0x14;
  *(uint8_t*)0x200002c6 = 0x14;
  *(uint8_t*)0x200002c7 = 0xaa;
  *(uint32_t*)0x200002d4 = 0;
  *(uint8_t*)0x200002d8 = 3;
  *(uint8_t*)0x200002d9 = 0;
  *(uint8_t*)0x200002da = 0xfe;
  *(uint32_t*)0x200002dc = 0;
  *(uint32_t*)0x200002e0 = 0;
  *(uint32_t*)0x200002e4 = 0;
  syscall(__NR_setsockopt, r[0], 0x29, 0x23, 0x20000200, 0xe8);
  *(uint64_t*)0x20002000 = 0x20000100;
  *(uint16_t*)0x20000100 = 2;
  *(uint16_t*)0x20000102 = htobe16(0xc67a);
  *(uint32_t*)0x20000104 = htobe32(0);
  *(uint8_t*)0x20000108 = 0;
  *(uint8_t*)0x20000109 = 0;
  *(uint8_t*)0x2000010a = 0;
  *(uint8_t*)0x2000010b = 0;
  *(uint8_t*)0x2000010c = 0;
  *(uint8_t*)0x2000010d = 0;
  *(uint8_t*)0x2000010e = 0;
  *(uint8_t*)0x2000010f = 0;
  *(uint32_t*)0x20002008 = 0x80;
  *(uint64_t*)0x20002010 = 0x20000240;
  *(uint64_t*)0x20002018 = 0;
  *(uint64_t*)0x20002020 = 0x20000280;
  *(uint64_t*)0x20002028 = 0;
  *(uint32_t*)0x20002030 = 0;
  *(uint32_t*)0x20002038 = 0;
  syscall(__NR_sendmmsg, r[0], 0x20002000, 1, 0);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
