// https://syzkaller.appspot.com/bug?id=660cabc1b8bdb5a0a68e37cc37ed4c9229199e9e
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

static void execute_one();
extern unsigned long long procid;

void loop()
{
  while (1) {
    execute_one();
  }
}

void execute_one()
{
  memcpy((void*)0x20001000, "./file0", 8);
  syscall(__NR_mkdir, 0x20001000, 0);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    loop();
  }
}
