// https://syzkaller.appspot.com/bug?id=ee0831a5f2236c9d95f6cb74878ed6ee1de53380
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
  syscall(__NR_mmap, 0x20ff0000, 0x10000, 0, 0x12, r[0], 0);
  res = syscall(__NR_socket, 0xa, 0x80001, 0);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x20fca000 = 0;
  *(uint16_t*)0x20fca008 = 0xa;
  *(uint16_t*)0x20fca00a = htobe16(0);
  *(uint32_t*)0x20fca00c = 0;
  *(uint8_t*)0x20fca010 = -1;
  *(uint8_t*)0x20fca011 = 1;
  *(uint8_t*)0x20fca012 = 0;
  *(uint8_t*)0x20fca013 = 0;
  *(uint8_t*)0x20fca014 = 0;
  *(uint8_t*)0x20fca015 = 0;
  *(uint8_t*)0x20fca016 = 0;
  *(uint8_t*)0x20fca017 = 0;
  *(uint8_t*)0x20fca018 = 0;
  *(uint8_t*)0x20fca019 = 0;
  *(uint8_t*)0x20fca01a = 0;
  *(uint8_t*)0x20fca01b = 0;
  *(uint8_t*)0x20fca01c = 0;
  *(uint8_t*)0x20fca01d = 0;
  *(uint8_t*)0x20fca01e = 0;
  *(uint8_t*)0x20fca01f = 1;
  *(uint32_t*)0x20fca020 = 0;
  syscall(__NR_setsockopt, r[1], 0x29, 0x2a, 0x20fca000, 0x88);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
