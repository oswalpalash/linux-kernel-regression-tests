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
  *(uint64_t*)0x200014c0 = 0x20000040;
  *(uint16_t*)0x20000040 = 0x1f;
  *(uint32_t*)0x20000044 = 0;
  *(uint16_t*)0x20000048 = 3;
  *(uint32_t*)0x200014c8 = 0x80;
  *(uint64_t*)0x200014d0 = 0x20001440;
  *(uint64_t*)0x20001440 = 0x20000100;
  *(uint64_t*)0x20001448 = 0;
  *(uint64_t*)0x20001450 = 0x20000140;
  *(uint64_t*)0x20001458 = 0;
  *(uint64_t*)0x20001460 = 0x20000240;
  *(uint64_t*)0x20001468 = 0;
  *(uint64_t*)0x20001470 = 0x20001240;
  *(uint64_t*)0x20001478 = 0;
  *(uint64_t*)0x20001480 = 0x20001280;
  *(uint64_t*)0x20001488 = 0;
  *(uint64_t*)0x20001490 = 0x20001380;
  *(uint64_t*)0x20001498 = 0;
  *(uint64_t*)0x200014d8 = 6;
  *(uint64_t*)0x200014e0 = 0;
  *(uint64_t*)0x200014e8 = 0;
  *(uint32_t*)0x200014f0 = 0x20000000;
  syscall(__NR_sendmsg, r[0], 0x200014c0, 0x20000000);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
