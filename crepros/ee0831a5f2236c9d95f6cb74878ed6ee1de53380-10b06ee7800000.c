// https://syzkaller.appspot.com/bug?id=ee0831a5f2236c9d95f6cb74878ed6ee1de53380
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
void loop()
{
  long res = 0;
  res = syscall(__NR_socket, 0xa, 1, 0);
  if (res != -1)
    r[0] = res;
  syscall(__NR_mmap, 0x20ffc000, 0x3000, 0, 0x2811, r[0], 0);
  res = syscall(__NR_socket, 0xa, 0x802, 0);
  if (res != -1)
    r[1] = res;
  *(uint8_t*)0x200001c0 = 0xfe;
  *(uint8_t*)0x200001c1 = 0x80;
  *(uint8_t*)0x200001c2 = 0;
  *(uint8_t*)0x200001c3 = 0;
  *(uint8_t*)0x200001c4 = 0;
  *(uint8_t*)0x200001c5 = 0;
  *(uint8_t*)0x200001c6 = 0;
  *(uint8_t*)0x200001c7 = 0;
  *(uint8_t*)0x200001c8 = 0;
  *(uint8_t*)0x200001c9 = 0;
  *(uint8_t*)0x200001ca = 0;
  *(uint8_t*)0x200001cb = 0;
  *(uint8_t*)0x200001cc = 0;
  *(uint8_t*)0x200001cd = 0;
  *(uint8_t*)0x200001ce = 0;
  *(uint8_t*)0x200001cf = 0xbb;
  *(uint32_t*)0x200001d0 = htobe32(0);
  *(uint16_t*)0x200001e0 = htobe16(0);
  *(uint16_t*)0x200001e2 = htobe16(0);
  *(uint16_t*)0x200001e4 = htobe16(0);
  *(uint16_t*)0x200001e6 = htobe16(0);
  *(uint16_t*)0x200001e8 = 0xa;
  *(uint8_t*)0x200001ea = 0;
  *(uint8_t*)0x200001eb = 0;
  *(uint8_t*)0x200001ec = 0;
  *(uint32_t*)0x200001f0 = 0;
  *(uint32_t*)0x200001f4 = 0;
  *(uint64_t*)0x200001f8 = 0;
  *(uint64_t*)0x20000200 = 0;
  *(uint64_t*)0x20000208 = 0;
  *(uint64_t*)0x20000210 = 0;
  *(uint64_t*)0x20000218 = 0;
  *(uint64_t*)0x20000220 = 0;
  *(uint64_t*)0x20000228 = 0;
  *(uint64_t*)0x20000230 = 0;
  *(uint64_t*)0x20000238 = 0;
  *(uint64_t*)0x20000240 = 0;
  *(uint64_t*)0x20000248 = 0;
  *(uint64_t*)0x20000250 = 0;
  *(uint32_t*)0x20000258 = 0;
  *(uint32_t*)0x2000025c = 0;
  *(uint8_t*)0x20000260 = 0;
  *(uint8_t*)0x20000261 = 0;
  *(uint8_t*)0x20000262 = 0;
  *(uint8_t*)0x20000263 = 0;
  *(uint8_t*)0x20000268 = 0xac;
  *(uint8_t*)0x20000269 = 0x14;
  *(uint8_t*)0x2000026a = 0x14;
  *(uint8_t*)0x2000026b = 0xaa;
  *(uint32_t*)0x20000278 = htobe32(0);
  *(uint8_t*)0x2000027c = -1;
  *(uint16_t*)0x20000280 = 0;
  *(uint8_t*)0x20000284 = -1;
  *(uint8_t*)0x20000285 = 2;
  *(uint8_t*)0x20000286 = 0;
  *(uint8_t*)0x20000287 = 0;
  *(uint8_t*)0x20000288 = 0;
  *(uint8_t*)0x20000289 = 0;
  *(uint8_t*)0x2000028a = 0;
  *(uint8_t*)0x2000028b = 0;
  *(uint8_t*)0x2000028c = 0;
  *(uint8_t*)0x2000028d = 0;
  *(uint8_t*)0x2000028e = 0;
  *(uint8_t*)0x2000028f = 0;
  *(uint8_t*)0x20000290 = 0;
  *(uint8_t*)0x20000291 = 0;
  *(uint8_t*)0x20000292 = 0;
  *(uint8_t*)0x20000293 = 1;
  *(uint32_t*)0x20000294 = 0;
  *(uint8_t*)0x20000298 = 0;
  *(uint8_t*)0x20000299 = 0;
  *(uint8_t*)0x2000029a = 0;
  *(uint32_t*)0x2000029c = 0;
  *(uint32_t*)0x200002a0 = 0;
  *(uint32_t*)0x200002a4 = 0;
  syscall(__NR_setsockopt, r[1], 0x29, 0x23, 0x200001c0, 0xe8);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
