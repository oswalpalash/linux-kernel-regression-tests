// https://syzkaller.appspot.com/bug?id=3020f475b83b979b1c6d36016260b0c0513364e6
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
  memcpy((void*)0x2059aff4, "/dev/ashmem", 12);
  r[0] = syscall(__NR_openat, 0xffffffffffffff9c, 0x2059aff4, 0, 0);
  syscall(__NR_ioctl, r[0], 0x40087703, 0x2a);
  syscall(__NR_mmap, 0x203f5000, 0x2000, 0, 0x1011, r[0], 0);
  syscall(__NR_ioctl, r[0], 0x7709, 0);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
