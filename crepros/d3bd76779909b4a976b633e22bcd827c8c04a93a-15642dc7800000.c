// https://syzkaller.appspot.com/bug?id=d3bd76779909b4a976b633e22bcd827c8c04a93a
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
  syscall(__NR_mmap, 0x20ffa000, 0x3000, 8, 0x4051, r[0], 0);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
