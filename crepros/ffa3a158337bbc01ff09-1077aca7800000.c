// https://syzkaller.appspot.com/bug?id=bea1c185923957c5339d5b43bca13e71047453da
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
void execute_one()
{
  long res = 0;
  res = syscall(__NR_socket, 0x26, 5, 0);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20001280 = 0x26;
  memcpy((void*)0x20001282,
         "\x68\x61\x73\x68\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 14);
  *(uint32_t*)0x20001290 = 0;
  *(uint32_t*)0x20001294 = 0;
  memcpy((void*)0x20001298,
         "\x73\x68\x61\x33\x2d\x35\x31\x32\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         64);
  syscall(__NR_bind, r[0], 0x20001280, 0x58);
  *(uint32_t*)0x20000300 = 0xab2539e6;
  res = syscall(__NR_accept, r[0], 0, 0x20000300);
  if (res != -1)
    r[1] = res;
  *(uint16_t*)0x20000240 = 2;
  *(uint16_t*)0x20000242 = htobe16(0xfffe);
  *(uint32_t*)0x20000244 = htobe32(0xe0000002);
  *(uint8_t*)0x20000248 = 0;
  *(uint8_t*)0x20000249 = 0;
  *(uint8_t*)0x2000024a = 0;
  *(uint8_t*)0x2000024b = 0;
  *(uint8_t*)0x2000024c = 0;
  *(uint8_t*)0x2000024d = 0;
  *(uint8_t*)0x2000024e = 0;
  *(uint8_t*)0x2000024f = 0;
  syscall(__NR_sendto, r[1], 0x20000000, 0, 0, 0x20000240, 0x10);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    loop();
  }
}
