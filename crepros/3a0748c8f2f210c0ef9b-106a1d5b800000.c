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
  *(uint64_t*)0x20000840 = 0x200000c0;
  *(uint16_t*)0x200000c0 = 1;
  *(uint8_t*)0x200000c2 = 0;
  *(uint32_t*)0x200000c4 = 0x4e23;
  *(uint32_t*)0x20000848 = 0x6e;
  *(uint64_t*)0x20000850 = 0x20000340;
  *(uint64_t*)0x20000340 = 0x20000140;
  *(uint64_t*)0x20000348 = 0;
  *(uint64_t*)0x20000350 = 0x20000200;
  *(uint64_t*)0x20000358 = 0;
  *(uint64_t*)0x20000360 = 0x200002c0;
  *(uint64_t*)0x20000368 = 0;
  *(uint64_t*)0x20000370 = 0x20000300;
  *(uint64_t*)0x20000378 = 0;
  *(uint64_t*)0x20000858 = 4;
  *(uint64_t*)0x20000860 = 0;
  *(uint64_t*)0x20000868 = 0;
  *(uint32_t*)0x20000870 = 0x4040014;
  *(uint64_t*)0x20000878 = 0x20000380;
  *(uint16_t*)0x20000380 = 1;
  *(uint8_t*)0x20000382 = 0;
  *(uint32_t*)0x20000384 = 0x4e24;
  *(uint32_t*)0x20000880 = 0x6e;
  *(uint64_t*)0x20000888 = 0x20000580;
  *(uint64_t*)0x20000580 = 0x20000400;
  *(uint64_t*)0x20000588 = 0;
  *(uint64_t*)0x20000590 = 0x20000500;
  *(uint64_t*)0x20000598 = 0;
  *(uint64_t*)0x20000890 = 2;
  *(uint64_t*)0x20000898 = 0x200007c0;
  *(uint32_t*)0x200007c0 = -1;
  *(uint32_t*)0x200007c4 = -1;
  *(uint32_t*)0x200007c8 = -1;
  *(uint32_t*)0x200007cc = r[0];
  *(uint32_t*)0x200007d0 = r[0];
  *(uint32_t*)0x200007d4 = r[0];
  *(uint32_t*)0x200007d8 = r[0];
  *(uint32_t*)0x200007dc = r[0];
  *(uint32_t*)0x200007e0 = r[0];
  *(uint64_t*)0x200008a0 = 0x24;
  *(uint32_t*)0x200008a8 = 0;
  syscall(__NR_sendmmsg, r[0], 0x20000840, 2, 0x2004c080);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
