// https://syzkaller.appspot.com/bug?id=9fcc32fe46ce159d6a3e64b64ab3402a7ac2e9b8
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#ifndef __NR_finit_module
#define __NR_finit_module 313
#endif

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

long r[40];
void test()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  *(uint32_t*)0x208a7f88 = (uint32_t)0x4000000002;
  *(uint32_t*)0x208a7f8c = (uint32_t)0x78;
  *(uint8_t*)0x208a7f90 = (uint8_t)0xdc;
  *(uint8_t*)0x208a7f91 = (uint8_t)0x0;
  *(uint8_t*)0x208a7f92 = (uint8_t)0x0;
  *(uint8_t*)0x208a7f93 = (uint8_t)0x0;
  *(uint32_t*)0x208a7f94 = (uint32_t)0x0;
  *(uint64_t*)0x208a7f98 = (uint64_t)0x0;
  *(uint64_t*)0x208a7fa0 = (uint64_t)0x0;
  *(uint64_t*)0x208a7fa8 = (uint64_t)0x0;
  *(uint8_t*)0x208a7fb0 = (uint8_t)0xfc;
  *(uint8_t*)0x208a7fb1 = (uint8_t)0x0;
  *(uint8_t*)0x208a7fb2 = (uint8_t)0x0;
  *(uint8_t*)0x208a7fb3 = (uint8_t)0x0;
  *(uint32_t*)0x208a7fb4 = (uint32_t)0x0;
  *(uint32_t*)0x208a7fb8 = (uint32_t)0x0;
  *(uint32_t*)0x208a7fbc = (uint32_t)0x0;
  *(uint64_t*)0x208a7fc0 = (uint64_t)0x0;
  *(uint64_t*)0x208a7fc8 = (uint64_t)0x0;
  *(uint64_t*)0x208a7fd0 = (uint64_t)0x0;
  *(uint64_t*)0x208a7fd8 = (uint64_t)0x0;
  *(uint64_t*)0x208a7fe0 = (uint64_t)0x0;
  *(uint32_t*)0x208a7fe8 = (uint32_t)0x0;
  *(uint64_t*)0x208a7ff0 = (uint64_t)0x0;
  *(uint32_t*)0x208a7ff8 = (uint32_t)0x0;
  *(uint16_t*)0x208a7ffc = (uint16_t)0x0;
  *(uint16_t*)0x208a7ffe = (uint16_t)0x0;
  r[28] = syscall(__NR_perf_event_open, 0x208a7f88ul, 0x0ul,
                  0xfffffffffffffffful, 0xfffffffffffffffful, 0x0ul);
  r[29] = syscall(__NR_socket, 0x26ul, 0x5ul, 0x0ul);
  *(uint16_t*)0x20013000 = (uint16_t)0x26;
  memcpy((void*)0x20013002,
         "\x72\x6e\x67\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         14);
  *(uint32_t*)0x20013010 = (uint32_t)0x0;
  *(uint32_t*)0x20013014 = (uint32_t)0x1;
  memcpy((void*)0x20013018,
         "\x73\x74\x64\x72\x6e\x67\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00",
         64);
  r[35] = syscall(__NR_bind, r[29], 0x20013000ul, 0x58ul);
  r[36] = syscall(__NR_setsockopt, r[29], 0x117ul, 0x1ul, 0x20271000ul,
                  0x0ul);
  memcpy((void*)0x2058eff2,
         "\x29\x73\x65\x63\x75\x72\x69\x74\x79\x2c\x47\x50\x4c\x00",
         14);
  r[38] = syscall(__NR_finit_module, 0xfffffffffffffffful, 0x2058eff2ul,
                  0x1ul);
  r[39] = syscall(__NR_mmap, 0x20000000ul, 0xf62000ul, 0x3ul, 0x32ul,
                  0xfffffffffffffffful, 0x0ul);
}

int main()
{
  loop();
  return 0;
}
