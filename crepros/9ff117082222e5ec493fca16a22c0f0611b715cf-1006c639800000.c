// https://syzkaller.appspot.com/bug?id=9ff117082222e5ec493fca16a22c0f0611b715cf
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

long r[2];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 2, 0x80a, 0);
  memcpy((void*)0x208a3fe0,
         "\x6c\x6f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  *(uint16_t*)0x208a3ff0 = 0xfffd;
  syscall(__NR_ioctl, r[0], 0x8914, 0x208a3fe0);
  r[1] = syscall(__NR_socket, 0xa, 1, 0);
  memcpy((void*)0x209bf000, "tls", 4);
  syscall(__NR_setsockopt, r[1], 6, 0x1f, 0x209bf000, 4);
  *(uint64_t*)0x205f8fc8 = 0x205d9000;
  *(uint32_t*)0x205f8fd0 = 0x10;
  *(uint64_t*)0x205f8fd8 = 0x20abc000;
  *(uint64_t*)0x205f8fe0 = 1;
  *(uint64_t*)0x205f8fe8 = 0x20ea0fd0;
  *(uint64_t*)0x205f8ff0 = 0;
  *(uint32_t*)0x205f8ff8 = 0x10;
  *(uint16_t*)0x205d9000 = 2;
  *(uint16_t*)0x205d9002 = htobe16(0x4e20);
  *(uint32_t*)0x205d9004 = htobe32(0);
  *(uint8_t*)0x205d9008 = 0;
  *(uint8_t*)0x205d9009 = 0;
  *(uint8_t*)0x205d900a = 0;
  *(uint8_t*)0x205d900b = 0;
  *(uint8_t*)0x205d900c = 0;
  *(uint8_t*)0x205d900d = 0;
  *(uint8_t*)0x205d900e = 0;
  *(uint8_t*)0x205d900f = 0;
  *(uint64_t*)0x20abc000 = 0x202d9000;
  *(uint64_t*)0x20abc008 = 0;
  syscall(__NR_sendmmsg, r[1], 0x205f8fc8, 1, 0x20000000);
}

int main()
{
  loop();
  return 0;
}
