// https://syzkaller.appspot.com/bug?id=6a611b805567ae82feb9b5bb2bc8192cc5779c2c
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
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  memcpy((void*)0x2058a000, "/dev/vhost-net", 15);
  r[0] = syscall(__NR_openat, 0xffffffffffffff9c, 0x2058a000, 2, 0);
  *(uint32_t*)0x20baff98 = 1;
  *(uint64_t*)0x20baffa0 = 0x20cfd000;
  *(uint64_t*)0x20baffa8 = 0xb8;
  *(uint64_t*)0x20baffb0 = 0x20ca0000;
  *(uint8_t*)0x20baffb8 = 0;
  *(uint8_t*)0x20baffb9 = 0;
  *(uint64_t*)0x20baffc0 = 0;
  *(uint64_t*)0x20baffc8 = 0;
  *(uint64_t*)0x20baffd0 = 0;
  *(uint64_t*)0x20baffd8 = 0;
  *(uint64_t*)0x20baffe0 = 0;
  *(uint64_t*)0x20baffe8 = 0;
  *(uint64_t*)0x20bafff0 = 0;
  *(uint64_t*)0x20bafff8 = 0;
  syscall(__NR_write, r[0], 0x20baff98, 0x68);
}

int main()
{
  loop();
  return 0;
}
