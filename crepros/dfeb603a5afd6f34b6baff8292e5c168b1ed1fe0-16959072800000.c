// https://syzkaller.appspot.com/bug?id=dfeb603a5afd6f34b6baff8292e5c168b1ed1fe0
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <sys/syscall.h>
#include <unistd.h>

#include <stdint.h>
#include <string.h>

long r[15];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xb000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  memcpy((void*)0x20002000,
         "\x63\x69\x66\x73\x2e\x73\x70\x6e\x65\x67\x6f\x00", 12);
  *(uint8_t*)0x20009ffb = (uint8_t)0x73;
  *(uint8_t*)0x20009ffc = (uint8_t)0x79;
  *(uint8_t*)0x20009ffd = (uint8_t)0x7a;
  *(uint8_t*)0x20009ffe = (uint8_t)0x20;
  *(uint8_t*)0x20009fff = (uint8_t)0x0;
  r[7] = syscall(__NR_add_key, 0x20002000ul, 0x20009ffbul, 0x20002f2dul,
                 0x0ul, 0xfffffffffffffff8ul);
  memcpy((void*)0x20009000,
         "\x70\x6b\x63\x73\x37\x5f\x74\x65\x73\x74\x00", 11);
  *(uint8_t*)0x200083d2 = (uint8_t)0x73;
  *(uint8_t*)0x200083d3 = (uint8_t)0x79;
  *(uint8_t*)0x200083d4 = (uint8_t)0x7a;
  *(uint8_t*)0x200083d5 = (uint8_t)0x20;
  *(uint8_t*)0x200083d6 = (uint8_t)0x0;
  r[14] = syscall(__NR_add_key, 0x20009000ul, 0x200083d2ul,
                  0x20000000ul, 0x0ul, r[7]);
}

int main()
{
  loop();
  return 0;
}
