// https://syzkaller.appspot.com/bug?id=d969589dcca96808c8c22e856cc5045148895eee
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
  *(uint32_t*)0x20000440 = 0xe8;
  syscall(__NR_getsockopt, -1, 0x29, 0x22, 0x20005b40, 0x20000440);
  res = syscall(__NR_socket, 0x2b, 1, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000040 = r[0];
  *(uint16_t*)0x20000044 = 0;
  *(uint16_t*)0x20000046 = 0;
  syscall(__NR_poll, 0x20000040, 1, 3);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
