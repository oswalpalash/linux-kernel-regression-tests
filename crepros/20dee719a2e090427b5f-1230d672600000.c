// https://syzkaller.appspot.com/bug?id=4f5b31dab4100e81b059c9975ab2d19eae51746d
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

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  syscall(__NR_socket, 0x21, 2, 0x20000000a);
  return 0;
}
