// https://syzkaller.appspot.com/bug?id=1e14e510f51800981ad3346698877613700de374
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

long r[1];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_socket, 0x2b, 1, 0);
  syscall(__NR_listen, r[0], 0x1c000);
  syscall(__NR_shutdown, r[0], 2);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
