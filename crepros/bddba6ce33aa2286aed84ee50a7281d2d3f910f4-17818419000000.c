// https://syzkaller.appspot.com/bug?id=bddba6ce33aa2286aed84ee50a7281d2d3f910f4
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

static uintptr_t syz_open_dev(uintptr_t a0, uintptr_t a1, uintptr_t a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block",
            (uint8_t)a1, (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf));
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

static void test();

void loop()
{
  while (1) {
    test();
  }
}

long r[64];
void test()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  memcpy((void*)0x203a2000, "\x2e\x2f\x66\x69\x6c\x65\x30\x00", 8);
  *(uint64_t*)0x20000fe0 = (uint64_t)0x0;
  *(uint64_t*)0x20000fe8 = (uint64_t)0x2710;
  *(uint64_t*)0x20000ff0 = (uint64_t)0x77359400;
  *(uint64_t*)0x20000ff8 = (uint64_t)0x0;
  r[6] = syscall(__NR_utimes, 0x203a2000ul, 0x20000fe0ul);
  r[7] = syscall(__NR_getdents64, 0xfffffffffffffffful, 0x2076e000ul,
                 0x0ul);
  memcpy((void*)0x20c67ff1,
         "\x2f\x64\x65\x76\x2f\x73\x6e\x64\x2f\x74\x69\x6d\x65\x72\x00",
         15);
  r[9] = syz_open_dev(0x20c67ff1ul, 0x0ul, 0x0ul);
  *(uint32_t*)0x205a3fcc = (uint32_t)0x0;
  *(uint32_t*)0x205a3fd0 = (uint32_t)0x2;
  *(uint32_t*)0x205a3fd4 = (uint32_t)0x0;
  *(uint32_t*)0x205a3fd8 = (uint32_t)0xffffffffffffffff;
  *(uint32_t*)0x205a3fdc = (uint32_t)0x0;
  *(uint8_t*)0x205a3fe0 = (uint8_t)0x0;
  *(uint8_t*)0x205a3fe1 = (uint8_t)0x0;
  *(uint8_t*)0x205a3fe2 = (uint8_t)0x0;
  *(uint8_t*)0x205a3fe3 = (uint8_t)0x0;
  *(uint8_t*)0x205a3fe4 = (uint8_t)0x0;
  *(uint8_t*)0x205a3fe5 = (uint8_t)0x0;
  *(uint8_t*)0x205a3fe6 = (uint8_t)0x0;
  *(uint8_t*)0x205a3fe7 = (uint8_t)0x0;
  *(uint8_t*)0x205a3fe8 = (uint8_t)0x0;
  *(uint8_t*)0x205a3fe9 = (uint8_t)0x0;
  *(uint8_t*)0x205a3fea = (uint8_t)0x0;
  *(uint8_t*)0x205a3feb = (uint8_t)0x0;
  *(uint8_t*)0x205a3fec = (uint8_t)0x0;
  *(uint8_t*)0x205a3fed = (uint8_t)0x0;
  *(uint8_t*)0x205a3fee = (uint8_t)0x0;
  *(uint8_t*)0x205a3fef = (uint8_t)0x0;
  *(uint8_t*)0x205a3ff0 = (uint8_t)0x0;
  *(uint8_t*)0x205a3ff1 = (uint8_t)0x0;
  *(uint8_t*)0x205a3ff2 = (uint8_t)0x0;
  *(uint8_t*)0x205a3ff3 = (uint8_t)0x0;
  *(uint8_t*)0x205a3ff4 = (uint8_t)0x0;
  *(uint8_t*)0x205a3ff5 = (uint8_t)0x0;
  *(uint8_t*)0x205a3ff6 = (uint8_t)0x0;
  *(uint8_t*)0x205a3ff7 = (uint8_t)0x0;
  *(uint8_t*)0x205a3ff8 = (uint8_t)0x0;
  *(uint8_t*)0x205a3ff9 = (uint8_t)0x0;
  *(uint8_t*)0x205a3ffa = (uint8_t)0x0;
  *(uint8_t*)0x205a3ffb = (uint8_t)0x0;
  *(uint8_t*)0x205a3ffc = (uint8_t)0x0;
  *(uint8_t*)0x205a3ffd = (uint8_t)0x0;
  *(uint8_t*)0x205a3ffe = (uint8_t)0x0;
  *(uint8_t*)0x205a3fff = (uint8_t)0x0;
  r[47] = syscall(__NR_ioctl, r[9], 0x40345410ul, 0x205a3fccul);
  r[48] = syscall(__NR_ioctl, r[9], 0x54a2ul);
  memcpy((void*)0x20bd8000,
         "\x2f\x64\x65\x76\x2f\x73\x65\x71\x75\x65\x6e\x63\x65\x72\x32"
         "\x00",
         16);
  r[50] = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20bd8000ul,
                  0x101000ul, 0x0ul);
  *(uint32_t*)0x20d1e000 = (uint32_t)0xffffffffffffffff;
  *(uint32_t*)0x20d1e004 = (uint32_t)0x730e;
  *(uint32_t*)0x20d1e008 = (uint32_t)0x7;
  *(uint32_t*)0x20d1e00c = (uint32_t)0x3;
  *(uint8_t*)0x20d1e010 = (uint8_t)0xffffffffffffffff;
  *(uint8_t*)0x20d1e011 = (uint8_t)0x80000001;
  *(uint8_t*)0x20d1e012 = (uint8_t)0x4610;
  *(uint8_t*)0x20d1e013 = (uint8_t)0x9;
  *(uint32_t*)0x20d1e014 = (uint32_t)0x8;
  *(uint32_t*)0x20d1e018 = (uint32_t)0x9;
  *(uint32_t*)0x20d1e01c = (uint32_t)0x7;
  *(uint32_t*)0x20d1e020 = (uint32_t)0x1;
  r[63] = syscall(__NR_ioctl, r[50], 0x5402ul, 0x20d1e000ul);
}

int main()
{
  int i;
  for (i = 0; i < 8; i++) {
    if (fork() == 0) {
      loop();
      return 0;
    }
  }
  sleep(1000000);
  return 0;
}
