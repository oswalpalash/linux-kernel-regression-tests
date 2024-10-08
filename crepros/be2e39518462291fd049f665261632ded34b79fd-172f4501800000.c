// https://syzkaller.appspot.com/bug?id=be2e39518462291fd049f665261632ded34b79fd
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <stdint.h>
#include <string.h>

static void test();

void loop()
{
  while (1) {
    test();
  }
}

long r[1];
uint64_t procid;
void test()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 0x3, 0x32, 0xffffffff, 0x0);
  r[0] = syscall(__NR_socket, 0x2, 0x1, 0x0);
  *(uint64_t*)0x20001000 = 0x0;
  *(uint16_t*)0x20001008 = 0x2;
  *(uint16_t*)0x2000100a = htobe16(0x4e20 + procid * 0x4ul);
  *(uint32_t*)0x2000100c = htobe32(0xe0000001);
  *(uint8_t*)0x20001010 = 0x0;
  *(uint8_t*)0x20001011 = 0x0;
  *(uint8_t*)0x20001012 = 0x0;
  *(uint8_t*)0x20001013 = 0x0;
  *(uint8_t*)0x20001014 = 0x0;
  *(uint8_t*)0x20001015 = 0x0;
  *(uint8_t*)0x20001016 = 0x0;
  *(uint8_t*)0x20001017 = 0x0;
  *(uint16_t*)0x20001018 = 0x2;
  *(uint16_t*)0x2000101a = htobe16(0x4e20 + procid * 0x4ul);
  *(uint8_t*)0x2000101c = 0xac;
  *(uint8_t*)0x2000101d = 0x14;
  *(uint8_t*)0x2000101e = 0x0 + procid * 0x1ul;
  *(uint8_t*)0x2000101f = 0xaa;
  *(uint8_t*)0x20001020 = 0x0;
  *(uint8_t*)0x20001021 = 0x0;
  *(uint8_t*)0x20001022 = 0x0;
  *(uint8_t*)0x20001023 = 0x0;
  *(uint8_t*)0x20001024 = 0x0;
  *(uint8_t*)0x20001025 = 0x0;
  *(uint8_t*)0x20001026 = 0x0;
  *(uint8_t*)0x20001027 = 0x0;
  *(uint16_t*)0x20001028 = 0x2;
  *(uint16_t*)0x2000102a = htobe16(0x4e20 + procid * 0x4ul);
  *(uint32_t*)0x2000102c = htobe32(0xe0000002);
  *(uint8_t*)0x20001030 = 0x0;
  *(uint8_t*)0x20001031 = 0x0;
  *(uint8_t*)0x20001032 = 0x0;
  *(uint8_t*)0x20001033 = 0x0;
  *(uint8_t*)0x20001034 = 0x0;
  *(uint8_t*)0x20001035 = 0x0;
  *(uint8_t*)0x20001036 = 0x0;
  *(uint8_t*)0x20001037 = 0x0;
  *(uint16_t*)0x20001038 = 0x0;
  *(uint16_t*)0x2000103a = 0x0;
  *(uint64_t*)0x20001040 = 0x0;
  *(uint64_t*)0x20001048 = 0x0;
  *(uint16_t*)0x20001050 = 0x2;
  *(uint64_t*)0x20001058 = 0x20000000;
  *(uint64_t*)0x20001060 = 0x0;
  *(uint64_t*)0x20001068 = 0x0;
  *(uint16_t*)0x20001070 = 0x0;
  syscall(__NR_ioctl, 0xffffffff, 0x890b, 0x20001000);
  *(uint32_t*)0x20001000 = 0x1;
  *(uint32_t*)0x20001004 = 0xffffff44;
  *(uint32_t*)0x20001008 = 0x0;
  *(uint32_t*)0x2000100c = 0x0;
  *(uint32_t*)0x20001010 = 0x0;
  *(uint32_t*)0x20001014 = 0x0;
  *(uint32_t*)0x20001018 = 0x0;
  *(uint32_t*)0x2000101c = 0x0;
  *(uint32_t*)0x20001020 = 0x100000;
  *(uint32_t*)0x20001024 = 0xfffffffb;
  syscall(__NR_setsockopt, r[0], 0x6, 0x16, 0x20001000, 0x5);
  *(uint32_t*)0x20000ffc = 0x0;
  syscall(__NR_setsockopt, r[0], 0x0, 0x10000000000040, 0x20000ffc,
          0x4);
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
