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
  syscall(__NR_mmap, 0x20001000, 0xf4f000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 0x40000000015, 0x805, 0);
  *(uint16_t*)0x200a9000 = 2;
  *(uint16_t*)0x200a9002 = 0;
  *(uint32_t*)0x200a9004 = htobe32(0x7f000001);
  *(uint8_t*)0x200a9008 = 0;
  *(uint8_t*)0x200a9009 = 0;
  *(uint8_t*)0x200a900a = 0;
  *(uint8_t*)0x200a900b = 0;
  *(uint8_t*)0x200a900c = 0;
  *(uint8_t*)0x200a900d = 0;
  *(uint8_t*)0x200a900e = 0;
  *(uint8_t*)0x200a900f = 0;
  syscall(__NR_bind, r[0], 0x200a9000, 0x10);
  *(uint16_t*)0x20af3ff0 = 2;
  *(uint16_t*)0x20af3ff2 = 0;
  *(uint32_t*)0x20af3ff4 = htobe32(0x7f000001);
  *(uint8_t*)0x20af3ff8 = 0;
  *(uint8_t*)0x20af3ff9 = 0;
  *(uint8_t*)0x20af3ffa = 0;
  *(uint8_t*)0x20af3ffb = 0;
  *(uint8_t*)0x20af3ffc = 0;
  *(uint8_t*)0x20af3ffd = 0;
  *(uint8_t*)0x20af3ffe = 0;
  *(uint8_t*)0x20af3fff = 0;
  syscall(__NR_sendto, r[0], 0x20218000, 0, 0, 0x20af3ff0, 0x10);
}

int main()
{
  loop();
  return 0;
}
