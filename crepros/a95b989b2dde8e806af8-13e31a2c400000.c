// https://syzkaller.appspot.com/bug?id=7f89ee40d35beb6d8f3fc3d122e43287cc3825cd
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
void loop()
{
  long res = 0;
  memcpy((void*)0x200005c0, "./file0", 8);
  syscall(__NR_mkdir, 0x200005c0, 0xfffffffffffffffc);
  res = syscall(__NR_pipe2, 0x20000240, 0);
  if (res != -1) {
    r[0] = *(uint32_t*)0x20000240;
    r[1] = *(uint32_t*)0x20000244;
  }
  memcpy((void*)0x20000280, "./file0", 8);
  memcpy((void*)0x200002c0, "9p", 3);
  memcpy((void*)0x20000340, "trans=fd,", 9);
  memcpy((void*)0x20000349, "rfdno", 5);
  *(uint8_t*)0x2000034e = 0x3d;
  sprintf((char*)0x2000034f, "0x%016llx", (long long)r[0]);
  *(uint8_t*)0x20000361 = 0x2c;
  memcpy((void*)0x20000362, "wfdno", 5);
  *(uint8_t*)0x20000367 = 0x3d;
  sprintf((char*)0x20000368, "0x%016llx", (long long)r[1]);
  *(uint8_t*)0x2000037a = 0x2c;
  memcpy((void*)0x2000037b, "fscache", 7);
  *(uint8_t*)0x20000382 = 0x2c;
  memcpy((void*)0x20000383, "version=9p2000.L", 16);
  *(uint8_t*)0x20000393 = 0x2c;
  memcpy((void*)0x20000394, "cache=mmap", 10);
  *(uint8_t*)0x2000039e = 0x2c;
  memcpy((void*)0x2000039f, "access=user", 11);
  *(uint8_t*)0x200003aa = 0x2c;
  memcpy((void*)0x200003ab, "loose", 5);
  *(uint8_t*)0x200003b0 = 0x2c;
  memcpy((void*)0x200003b1, "cachetag", 8);
  *(uint8_t*)0x200003b9 = 0x3d;
  memcpy((void*)0x200003ba, "nodevmap", 8);
  *(uint8_t*)0x200003c2 = 0x2c;
  memcpy((void*)0x200003c3, "posixacl", 8);
  *(uint8_t*)0x200003cb = 0x2c;
  memcpy((void*)0x200003cc, "afid", 4);
  *(uint8_t*)0x200003d0 = 0x3d;
  sprintf((char*)0x200003d1, "0x%016llx", (long long)0x7ff);
  *(uint8_t*)0x200003e3 = 0x2c;
  memcpy((void*)0x200003e4, "cachetag", 8);
  *(uint8_t*)0x200003ec = 0x3d;
  memcpy((void*)0x200003ed, "\x40\x28\x77\x6c\x61\x6e\x30\x27\x6c\x6f\x73\x65"
                            "\x6c\x66\x70\x6f\x73\x69\x78\x5f\x61\x63\x6c\x5f"
                            "\x61\x63\x63\x65\x73\x73\xad\x70\x6f\x73\x69\x78"
                            "\x5f\x61\x63\x6c\x5f\x61\x63\x63\x65\x73\x73\x77"
                            "\x6c\x61\x6e\x31\x2d\x2f\x2d",
         55);
  *(uint8_t*)0x20000424 = 0x2c;
  *(uint8_t*)0x20000425 = 0;
  syscall(__NR_mount, 0, 0x20000280, 0x200002c0, 0x800000, 0x20000340);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
