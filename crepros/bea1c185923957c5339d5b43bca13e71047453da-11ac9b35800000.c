// https://syzkaller.appspot.com/bug?id=bea1c185923957c5339d5b43bca13e71047453da
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
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

long r[3];
uint64_t procid;
void test()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 0x26, 5, 0);
  syscall(__NR_close, r[0]);
  r[1] = syscall(__NR_socket, 0x26, 5, 0);
  *(uint16_t*)0x20219fa8 = 0x26;
  memcpy((void*)0x20219faa,
         "\x73\x6b\x63\x69\x70\x68\x65\x72\x00\x00\x00\x00\x00\x00", 14);
  *(uint32_t*)0x20219fb8 = 0;
  *(uint32_t*)0x20219fbc = 0;
  memcpy((void*)0x20219fc0,
         "\x73\x61\x6c\x73\x61\x32\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         64);
  syscall(__NR_bind, r[0], 0x20219fa8, 0x58);
  memcpy((void*)0x20001f3a,
         "\xad\x56\xb6\xc5\x82\x0f\xae\xb9\x95\x29\x89\x92\xea\x54\xc7\xbe",
         16);
  syscall(__NR_setsockopt, r[1], 0x117, 1, 0x20001f3a, 0x10);
  r[2] = syscall(__NR_accept, r[1], 0, 0);
  *(uint64_t*)0x20791000 = 0;
  *(uint32_t*)0x20791008 = 0;
  *(uint64_t*)0x20791010 = 0x20cf6000;
  *(uint64_t*)0x20791018 = 1;
  *(uint64_t*)0x20791020 = 0x20000000;
  *(uint64_t*)0x20791028 = 0;
  *(uint32_t*)0x20791030 = 0;
  *(uint64_t*)0x20cf6000 = 0x20087000;
  *(uint64_t*)0x20cf6008 = 0x80;
  memcpy((void*)0x20087000,
         "\x51\xc8\xfb\x29\x29\x98\x9d\x20\xaf\xe7\x3d\xca\xc1\x12\x91\xb7\x20"
         "\xbd\x7b\x71\xdd\xeb\x91\x61\x96\x6d\x49\x86\xbc\x69\x33\x5f\xf6\x3b"
         "\x71\x1f\x36\x65\x3d\x33\xc3\xaf\x96\xb4\x27\x39\x38\x69\x50\x91\x95"
         "\xdb\xe3\xbb\xd7\x2d\x89\x61\x05\xf0\x20\x41\x6b\xbc\xa5\xf9\xc9\x6a"
         "\x03\x1e\xde\x84\xd7\x22\xa8\xca\x57\x49\x65\xa5\x35\x85\x02\x15\x33"
         "\x5d\x66\xef\xc5\xe1\x24\xd0\x25\xa7\x62\xf7\x7b\x51\xe2\xad\x1a\xe8"
         "\x86\x5d\xa4\xed\x5d\xde\x15\xbe\x35\xbe\x78\xfc\xdb\x66\x7a\x12\xe2"
         "\x45\x2c\x8b\x20\x14\x61\xf9\x62\xad",
         128);
  syscall(__NR_sendmmsg, r[2], 0x20791000, 1, 0);
  *(uint64_t*)0x20b2f000 = 0x208e8000;
  *(uint32_t*)0x20b2f008 = 0x10;
  *(uint64_t*)0x20b2f010 = 0x204f3f73;
  *(uint64_t*)0x20b2f018 = 2;
  *(uint64_t*)0x20b2f020 = 0x20590000;
  *(uint64_t*)0x20b2f028 = 0;
  *(uint32_t*)0x20b2f030 = 0;
  *(uint64_t*)0x204f3f73 = 0x20284f81;
  *(uint64_t*)0x204f3f7b = 0x7f;
  *(uint64_t*)0x204f3f83 = 0x203bc000;
  *(uint64_t*)0x204f3f8b = 0xc6;
  syscall(__NR_recvmsg, r[2], 0x20b2f000, 0);
}

int main()
{
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      for (;;) {
        loop();
      }
    }
  }
  sleep(1000000);
  return 0;
}
