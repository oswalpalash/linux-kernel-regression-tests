// https://syzkaller.appspot.com/bug?id=1456dbc1f353ec4d523630bdfff662681675ab15
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_setsockopt
#define __NR_setsockopt 366
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000, 0x1000, 0, 0x32, -1, 0);
  syscall(__NR_mmap, 0x20000000, 0x1000000, 7, 0x32, -1, 0);
  syscall(__NR_mmap, 0x21000000, 0x1000, 0, 0x32, -1, 0);
  intptr_t res = 0;
  res = syscall(__NR_socket, 2, 3, 5);
  if (res != -1)
    r[0] = res;
  syscall(__NR_setsockopt, (intptr_t)r[0], 0, 8, -1, 1);
  return 0;
}
