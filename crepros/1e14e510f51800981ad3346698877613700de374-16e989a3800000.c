// https://syzkaller.appspot.com/bug?id=1e14e510f51800981ad3346698877613700de374
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#ifndef __NR_listen
#define __NR_listen 363
#endif
#ifndef __NR_shutdown
#define __NR_shutdown 373
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[1] = {0xffffffffffffffff};
void loop()
{
  long res;
  res = syscall(__NR_socket, 0x2b, 0x801, 0);
  if (res != -1)
    r[0] = res;
  syscall(__NR_listen, r[0], 0);
  syscall(__NR_shutdown, r[0], 0);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
