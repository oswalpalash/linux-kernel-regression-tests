// https://syzkaller.appspot.com/bug?id=77d9a62c6781d69dd833adc06ead030abc367218
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
void loop()
{
  long res = 0;
  res = syscall(__NR_socket, 0xa, 1, 0);
  if (res != -1)
    r[0] = res;
  syscall(__NR_mmap, 0x20ffc000, 0x3000, 0, 0x11, r[0], 0);
  res = syscall(__NR_socket, 0xa, 2, 0);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x209ecf78 = 0;
  *(uint16_t*)0x209ecf80 = 0xa;
  *(uint16_t*)0x209ecf82 = htobe16(0);
  *(uint32_t*)0x209ecf84 = 0;
  *(uint8_t*)0x209ecf88 = -1;
  *(uint8_t*)0x209ecf89 = 2;
  *(uint8_t*)0x209ecf8a = 0;
  *(uint8_t*)0x209ecf8b = 0;
  *(uint8_t*)0x209ecf8c = 0;
  *(uint8_t*)0x209ecf8d = 0;
  *(uint8_t*)0x209ecf8e = 0;
  *(uint8_t*)0x209ecf8f = 0;
  *(uint8_t*)0x209ecf90 = 0;
  *(uint8_t*)0x209ecf91 = 0;
  *(uint8_t*)0x209ecf92 = 0;
  *(uint8_t*)0x209ecf93 = 0;
  *(uint8_t*)0x209ecf94 = 0;
  *(uint8_t*)0x209ecf95 = 0;
  *(uint8_t*)0x209ecf96 = 0;
  *(uint8_t*)0x209ecf97 = 1;
  *(uint32_t*)0x209ecf98 = 0;
  syscall(__NR_setsockopt, r[1], 0x29, 0x2a, 0x209ecf78, 0x2ef);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
