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
  *(uint64_t*)0x200001c0 = 0x200000c0;
  *(uint16_t*)0x200000c0 = 0x10;
  *(uint16_t*)0x200000c2 = 0;
  *(uint32_t*)0x200000c4 = 0;
  *(uint32_t*)0x200000c8 = 0x10008;
  *(uint32_t*)0x200001c8 = 0xc;
  *(uint64_t*)0x200001d0 = 0x20000180;
  *(uint64_t*)0x20000180 = 0x20000140;
  *(uint32_t*)0x20000140 = 0x40;
  *(uint16_t*)0x20000144 = 0;
  *(uint16_t*)0x20000146 = 0x800;
  *(uint32_t*)0x20000148 = 0x70bd2b;
  *(uint32_t*)0x2000014c = 0x25dfdbfe;
  *(uint8_t*)0x20000150 = 4;
  *(uint8_t*)0x20000151 = 0;
  *(uint16_t*)0x20000152 = 0;
  *(uint16_t*)0x20000154 = 0x2c;
  *(uint16_t*)0x20000156 = 2;
  *(uint16_t*)0x20000158 = 8;
  *(uint16_t*)0x2000015a = 9;
  *(uint32_t*)0x2000015c = 0xed;
  *(uint16_t*)0x20000160 = 8;
  *(uint16_t*)0x20000162 = 3;
  *(uint16_t*)0x20000164 = 4;
  *(uint16_t*)0x20000168 = 8;
  *(uint16_t*)0x2000016a = 9;
  *(uint32_t*)0x2000016c = 1;
  *(uint16_t*)0x20000170 = 8;
  *(uint16_t*)0x20000172 = 7;
  *(uint32_t*)0x20000174 = 8;
  *(uint16_t*)0x20000178 = 8;
  *(uint16_t*)0x2000017a = 4;
  *(uint32_t*)0x2000017c = 2;
  *(uint64_t*)0x20000188 = 0x40;
  *(uint64_t*)0x200001d8 = 1;
  *(uint64_t*)0x200001e0 = 0;
  *(uint64_t*)0x200001e8 = 0;
  *(uint32_t*)0x200001f0 = 0;
  syscall(__NR_sendmsg, r[0], 0x200001c0, 0x20000000);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
