// https://syzkaller.appspot.com/bug?id=d204aaca3ac260c553e053c566b529f350ea6454
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
  res = syscall(__NR_socket, 2, 0x4000000000000001, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000b80 = 0x80;
  syscall(__NR_accept4, r[0], 0x20000b00, 0x20000b80, 0x80800);
  *(uint32_t*)0x20000040 = 2;
  memcpy((void*)0x20000044,
         "\x62\x72\x69\x64\x67\x65\x5f\x73\x6c\x61\x76\x65\x5f\x30\x00\x00",
         16);
  *(uint32_t*)0x20000054 = 0;
  syscall(__NR_setsockopt, r[0], 0, 0x48b, 0x20000040, 0x18);
  *(uint32_t*)0x20000000 = 2;
  memcpy((void*)0x20000004,
         "\x69\x66\x62\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  *(uint32_t*)0x20000014 = 0;
  syscall(__NR_setsockopt, r[0], 0, 0x48c, 0x20000000, 0x18);
  *(uint16_t*)0x200000c0 = 0x28;
  *(uint16_t*)0x200000c2 = 0;
  *(uint32_t*)0x200000c4 = 0;
  *(uint32_t*)0x200000c8 = 0;
  *(uint32_t*)0x200000cc = 0;
  syscall(__NR_connect, -1, 0x200000c0, 0x10);
  res = syscall(__NR_pipe, 0x20000100);
  if (res != -1)
    r[1] = *(uint32_t*)0x20000100;
  *(uint32_t*)0x20000180 = 5;
  *(uint32_t*)0x20000140 = 4;
  syscall(__NR_getsockopt, r[1], 0x103, 3, 0x20000180, 0x20000140);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    loop();
  }
}
