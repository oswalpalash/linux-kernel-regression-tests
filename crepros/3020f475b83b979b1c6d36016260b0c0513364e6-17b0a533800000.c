// https://syzkaller.appspot.com/bug?id=3020f475b83b979b1c6d36016260b0c0513364e6
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
  long res;
  memcpy((void*)0x200000c0, "/dev/ashmem", 12);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x200000c0, 0, 0);
  if (res != -1)
    r[0] = res;
  syscall(__NR_ioctl, r[0], 0x40087703, 0x400);
  syscall(__NR_mmap, 0x20ffe000, 0x1000, 0, 0x1011, r[0], 0);
  syscall(__NR_ioctl, r[0], 0x7709, 0);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
