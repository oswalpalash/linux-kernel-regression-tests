// https://syzkaller.appspot.com/bug?id=004b0f7b61d4901cbfecfc33de7996e8cbe0a278
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
  long res = 0;
  res = syscall(__NR_socket, 0x2b, 1, 0);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000cc0 = 0x200006c0;
  *(uint16_t*)0x200006c0 = 0x26;
  memcpy((void*)0x200006c2,
         "\x61\x65\x61\x64\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 14);
  *(uint32_t*)0x200006d0 = 0;
  *(uint32_t*)0x200006d4 = 0;
  memcpy((void*)0x200006d8,
         "\x67\x63\x6d\x5f\x62\x61\x73\x65\x28\x63\x74\x72\x28\x61\x65\x73\x2d"
         "\x61\x65\x73\x6e\x69\x29\x2c\x67\x68\x61\x73\x68\x2d\x67\x65\x6e\x65"
         "\x72\x69\x63\x29\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         64);
  *(uint32_t*)0x20000cc8 = 0x80;
  *(uint64_t*)0x20000cd0 = 0x20000b40;
  *(uint64_t*)0x20000b40 = 0x20000740;
  *(uint64_t*)0x20000b48 = 0;
  *(uint64_t*)0x20000b50 = 0x200007c0;
  *(uint64_t*)0x20000b58 = 0;
  *(uint64_t*)0x20000b60 = 0x20000140;
  *(uint64_t*)0x20000b68 = 0;
  *(uint64_t*)0x20000b70 = 0x20000840;
  *(uint64_t*)0x20000b78 = 0;
  *(uint64_t*)0x20000b80 = 0x20000940;
  *(uint64_t*)0x20000b88 = 0;
  *(uint64_t*)0x20000b90 = 0x20000a40;
  *(uint64_t*)0x20000b98 = 0;
  *(uint64_t*)0x20000ba0 = 0x20001980;
  *(uint64_t*)0x20000ba8 = 0;
  *(uint64_t*)0x20000bb0 = 0x20000b00;
  *(uint64_t*)0x20000bb8 = 0;
  *(uint64_t*)0x20000cd8 = 8;
  *(uint64_t*)0x20000ce0 = 0x20000bc0;
  *(uint64_t*)0x20000bc0 = 0x10;
  *(uint32_t*)0x20000bc8 = 0x10a;
  *(uint32_t*)0x20000bcc = 0;
  *(uint64_t*)0x20000ce8 = 0x10;
  *(uint32_t*)0x20000cf0 = 0x4000000;
  syscall(__NR_sendmsg, r[0], 0x20000cc0, 0x24000010);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
