// https://syzkaller.appspot.com/bug?id=004b0f7b61d4901cbfecfc33de7996e8cbe0a278
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
  res = syscall(__NR_socket, 0x2b, 1, 0);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000580 = 0x20000180;
  *(uint16_t*)0x20000180 = 0x18;
  *(uint32_t*)0x20000182 = 1;
  *(uint32_t*)0x20000186 = 0;
  *(uint32_t*)0x2000018a = r[0];
  *(uint16_t*)0x2000018e = 2;
  *(uint16_t*)0x20000190 = htobe16(0x4e24);
  *(uint8_t*)0x20000192 = 0xac;
  *(uint8_t*)0x20000193 = 0x14;
  *(uint8_t*)0x20000194 = 0x14;
  *(uint8_t*)0x20000195 = 0x18;
  *(uint8_t*)0x20000196 = 0;
  *(uint8_t*)0x20000197 = 0;
  *(uint8_t*)0x20000198 = 0;
  *(uint8_t*)0x20000199 = 0;
  *(uint8_t*)0x2000019a = 0;
  *(uint8_t*)0x2000019b = 0;
  *(uint8_t*)0x2000019c = 0;
  *(uint8_t*)0x2000019d = 0;
  *(uint32_t*)0x2000019e = 2;
  *(uint32_t*)0x200001a2 = 0;
  *(uint32_t*)0x200001a6 = 0;
  *(uint32_t*)0x200001aa = 3;
  *(uint32_t*)0x20000588 = 0x80;
  *(uint64_t*)0x20000590 = 0x20000500;
  *(uint64_t*)0x20000500 = 0x20000280;
  *(uint64_t*)0x20000508 = 0;
  *(uint64_t*)0x20000510 = 0x20000340;
  *(uint64_t*)0x20000518 = 0;
  *(uint64_t*)0x20000520 = 0x20000440;
  *(uint64_t*)0x20000528 = 0;
  *(uint64_t*)0x20000530 = 0x20000640;
  *(uint64_t*)0x20000538 = 0;
  *(uint64_t*)0x20000540 = 0x20000200;
  *(uint64_t*)0x20000548 = 0;
  *(uint64_t*)0x20000550 = 0x200004c0;
  *(uint64_t*)0x20000558 = 0;
  *(uint64_t*)0x20000598 = 6;
  *(uint64_t*)0x200005a0 = 0x20001640;
  *(uint64_t*)0x20001640 = 0x10;
  *(uint32_t*)0x20001648 = 0x10c;
  *(uint32_t*)0x2000164c = 5;
  *(uint64_t*)0x20001650 = 0x10;
  *(uint32_t*)0x20001658 = 0x117;
  *(uint32_t*)0x2000165c = 0x80000001;
  *(uint64_t*)0x200005a8 = 0x20;
  *(uint32_t*)0x200005b0 = 0x20000000;
  syscall(__NR_sendmsg, r[0], 0x20000580, 0x24000804);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
