// https://syzkaller.appspot.com/bug?id=e826b4d012da9963d581b16d665db3811588ffbe
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
  r[0] = syscall(__NR_socket, 0x40000000015, 5, 0);
  *(uint16_t*)0x208a5ff0 = 2;
  *(uint16_t*)0x208a5ff2 = 0;
  *(uint32_t*)0x208a5ff4 = htobe32(0x7f000001);
  *(uint8_t*)0x208a5ff8 = 0;
  *(uint8_t*)0x208a5ff9 = 0;
  *(uint8_t*)0x208a5ffa = 0;
  *(uint8_t*)0x208a5ffb = 0;
  *(uint8_t*)0x208a5ffc = 0;
  *(uint8_t*)0x208a5ffd = 0;
  *(uint8_t*)0x208a5ffe = 0;
  *(uint8_t*)0x208a5fff = 0;
  syscall(__NR_bind, r[0], 0x208a5ff0, 0x10);
  *(uint16_t*)0x2069affb = 2;
  *(uint16_t*)0x2069affd = 0;
  *(uint8_t*)0x2069afff = 0xac;
  *(uint8_t*)0x2069b000 = 0x14;
  *(uint8_t*)0x2069b001 = 0;
  *(uint8_t*)0x2069b002 = 0xbb;
  *(uint8_t*)0x2069b003 = 0;
  *(uint8_t*)0x2069b004 = 0;
  *(uint8_t*)0x2069b005 = 0;
  *(uint8_t*)0x2069b006 = 0;
  *(uint8_t*)0x2069b007 = 0;
  *(uint8_t*)0x2069b008 = 0;
  *(uint8_t*)0x2069b009 = 0;
  *(uint8_t*)0x2069b00a = 0;
  syscall(__NR_sendto, r[0], 0x20141fb9, 0, 0, 0x2069affb, 0x10);
}

int main()
{
  loop();
  return 0;
}
