// https://syzkaller.appspot.com/bug?id=77d9a62c6781d69dd833adc06ead030abc367218
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
  res = syscall(__NR_socket, 2, 1, 0);
  if (res != -1)
    r[0] = res;
  syscall(__NR_mmap, 0x20000000, 0xf50000, 0, 0x80812, r[0], 0);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
