// https://syzkaller.appspot.com/bug?id=9fcc32fe46ce159d6a3e64b64ab3402a7ac2e9b8
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

static void test();

void loop()
{
  while (1) {
    test();
  }
}

long r[9];
void test()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  r[1] = syscall(__NR_socket, 0x26ul, 0x5ul, 0x0ul);
  *(uint16_t*)0x209c5000 = (uint16_t)0x26;
  memcpy((void*)0x209c5002,
         "\x72\x6e\x67\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         14);
  *(uint32_t*)0x209c5010 = (uint32_t)0x4;
  *(uint32_t*)0x209c5014 = (uint32_t)0x3;
  memcpy((void*)0x209c5018,
         "\x73\x74\x64\x72\x6e\x67\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xef\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00",
         64);
  r[7] = syscall(__NR_bind, r[1], 0x209c5000ul, 0x58ul);
  r[8] = syscall(__NR_setsockopt, r[1], 0x117ul, 0x1ul, 0x208fd000ul,
                 0x0ul);
}

int main()
{
  loop();
  return 0;
}
