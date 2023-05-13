// https://syzkaller.appspot.com/bug?id=be2e39518462291fd049f665261632ded34b79fd
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

#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_bpf
#define __NR_bpf 357
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t procid;
void test()
{
  syscall(__NR_mmap, 0x20000000, 0xeb3000, 3, 0x32, -1, 0);
  *(uint32_t*)0x203dc000 = 6;
  *(uint32_t*)0x203dc004 = 4;
  *(uint32_t*)0x203dc008 = 0x338d;
  *(uint32_t*)0x203dc00c = 0x20004;
  *(uint32_t*)0x203dc010 = 0;
  *(uint32_t*)0x203dc014 = 0;
  *(uint32_t*)0x203dc018 = 0;
  *(uint8_t*)0x203dc01c = 0;
  *(uint8_t*)0x203dc01d = 0;
  *(uint8_t*)0x203dc01e = 0;
  *(uint8_t*)0x203dc01f = 0;
  *(uint8_t*)0x203dc020 = 0;
  *(uint8_t*)0x203dc021 = 0;
  *(uint8_t*)0x203dc022 = 0;
  *(uint8_t*)0x203dc023 = 0;
  *(uint8_t*)0x203dc024 = 0;
  *(uint8_t*)0x203dc025 = 0;
  *(uint8_t*)0x203dc026 = 0;
  *(uint8_t*)0x203dc027 = 0;
  *(uint8_t*)0x203dc028 = 0;
  *(uint8_t*)0x203dc029 = 0;
  *(uint8_t*)0x203dc02a = 0;
  *(uint8_t*)0x203dc02b = 0;
  syscall(__NR_bpf, 0, 0x203dc000, 0x2c);
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
