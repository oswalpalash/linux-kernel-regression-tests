// https://syzkaller.appspot.com/bug?id=45251caf911e74c2d20cbd4b39091fdead9d54fd
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
  res = syscall(__NR_socket, 0x10, 0x200000000002, 0xc);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000040, "\x1f\x00\x00\x00\x00\x07\xfd\x02\x01\x0c\x00\xf6"
                            "\x00\x00\xff\x00\x81\x09\xf8\x00\x09\x00\x01\x80"
                            "\x06\x08\x14\xec\x00\x00\x00",
         31);
  syscall(__NR_write, r[0], 0x20000040, 0x1f);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
