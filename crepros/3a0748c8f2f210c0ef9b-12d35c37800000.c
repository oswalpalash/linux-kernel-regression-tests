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
  memcpy((void*)0x200001c0,
         "\xf4\xd7\x6b\xd3\xca\x1d\x66\xa1\x8e\x4d\x89\xc5\x0b\x70\xfd\x2c",
         16);
  syscall(__NR_setsockopt, r[0], 6, 0x21, 0x200001c0, 0x10);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
