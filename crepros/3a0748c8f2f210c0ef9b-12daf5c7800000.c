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
  *(uint64_t*)0x20001600 = 0x20000100;
  *(uint16_t*)0x20000100 = 0x28;
  *(uint16_t*)0x20000102 = 0;
  *(uint32_t*)0x20000104 = -1;
  *(uint32_t*)0x20000108 = 0x4d2;
  *(uint32_t*)0x2000010c = 0;
  *(uint32_t*)0x20001608 = 0x80;
  *(uint64_t*)0x20001610 = 0x200003c0;
  *(uint64_t*)0x200003c0 = 0x20000180;
  *(uint64_t*)0x200003c8 = 0;
  *(uint64_t*)0x200003d0 = 0x20000280;
  *(uint64_t*)0x200003d8 = 0;
  *(uint64_t*)0x200003e0 = 0x20000300;
  *(uint64_t*)0x200003e8 = 0;
  *(uint64_t*)0x20001618 = 3;
  *(uint64_t*)0x20001620 = 0x20000400;
  *(uint64_t*)0x20000400 = 0x10;
  *(uint32_t*)0x20000408 = 0x114;
  *(uint32_t*)0x2000040c = 0x200;
  *(uint64_t*)0x20000410 = 0x10;
  *(uint32_t*)0x20000418 = 0x1ff;
  *(uint32_t*)0x2000041c = 3;
  *(uint64_t*)0x20000420 = 0x10;
  *(uint32_t*)0x20000428 = 0x11a;
  *(uint32_t*)0x2000042c = 3;
  *(uint64_t*)0x20000430 = 0x10;
  *(uint32_t*)0x20000438 = 0x10d;
  *(uint32_t*)0x2000043c = 4;
  *(uint64_t*)0x20001628 = 0x40;
  *(uint32_t*)0x20001630 = 0x44090;
  syscall(__NR_sendmsg, r[0], 0x20001600, 0x20000000);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
