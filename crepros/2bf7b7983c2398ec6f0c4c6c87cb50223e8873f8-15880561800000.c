// https://syzkaller.appspot.com/bug?id=2bf7b7983c2398ec6f0c4c6c87cb50223e8873f8
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

long r[3];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  memcpy((void*)0x20b78000, "/dev/kvm", 9);
  r[0] = syscall(__NR_openat, 0xffffffffffffff9c, 0x20b78000, 0, 0);
  r[1] = syscall(__NR_ioctl, r[0], 0xae01, 0);
  *(uint32_t*)0x20583fcc = 0;
  *(uint32_t*)0x20583fd0 = 3;
  *(uint32_t*)0x20583fd4 = 9;
  *(uint32_t*)0x20583fd8 = 3;
  *(uint32_t*)0x20583fdc = 0x8001;
  *(uint8_t*)0x20583fe0 = 0;
  *(uint8_t*)0x20583fe1 = 0;
  *(uint8_t*)0x20583fe2 = 0;
  *(uint8_t*)0x20583fe3 = 0;
  *(uint8_t*)0x20583fe4 = 0;
  *(uint8_t*)0x20583fe5 = 0;
  *(uint8_t*)0x20583fe6 = 0;
  *(uint8_t*)0x20583fe7 = 0;
  *(uint8_t*)0x20583fe8 = 0;
  *(uint8_t*)0x20583fe9 = 0;
  *(uint8_t*)0x20583fea = 0;
  *(uint8_t*)0x20583feb = 0;
  *(uint8_t*)0x20583fec = 0;
  *(uint8_t*)0x20583fed = 0;
  *(uint8_t*)0x20583fee = 0;
  *(uint8_t*)0x20583fef = 0;
  *(uint8_t*)0x20583ff0 = 0;
  *(uint8_t*)0x20583ff1 = 0;
  *(uint8_t*)0x20583ff2 = 0;
  *(uint8_t*)0x20583ff3 = 0;
  *(uint8_t*)0x20583ff4 = 0;
  *(uint8_t*)0x20583ff5 = 0;
  *(uint8_t*)0x20583ff6 = 0;
  *(uint8_t*)0x20583ff7 = 0;
  *(uint8_t*)0x20583ff8 = 0;
  *(uint8_t*)0x20583ff9 = 0;
  *(uint8_t*)0x20583ffa = 0;
  *(uint8_t*)0x20583ffb = 0;
  *(uint8_t*)0x20583ffc = 0;
  *(uint8_t*)0x20583ffd = 0;
  *(uint8_t*)0x20583ffe = 0;
  *(uint8_t*)0x20583fff = 0;
  syscall(__NR_ioctl, -1, 0x40345410, 0x20583fcc);
  syscall(__NR_ioctl, r[1], 0xae60);
  r[2] = syscall(__NR_ioctl, r[1], 0xae41, 0);
  syscall(__NR_uname, 0x20df1f0b);
  *(uint32_t*)0x2017a000 = 4;
  syscall(__NR_ioctl, r[2], 0x4004ae99, 0x2017a000);
  *(uint64_t*)0x20fc6f70 = 0;
  *(uint64_t*)0x20fc6f78 = 0;
  *(uint64_t*)0x20fc6f80 = 2;
  *(uint64_t*)0x20fc6f88 = 0x100;
  *(uint64_t*)0x20fc6f90 = 0x222;
  *(uint64_t*)0x20fc6f98 = 0;
  *(uint64_t*)0x20fc6fa0 = 0;
  *(uint64_t*)0x20fc6fa8 = 0;
  *(uint64_t*)0x20fc6fb0 = 0;
  *(uint64_t*)0x20fc6fb8 = 8;
  *(uint64_t*)0x20fc6fc0 = 0;
  *(uint64_t*)0x20fc6fc8 = 0x10000000000000;
  *(uint64_t*)0x20fc6fd0 = 0;
  *(uint64_t*)0x20fc6fd8 = 0;
  *(uint64_t*)0x20fc6fe0 = 0;
  *(uint64_t*)0x20fc6fe8 = 0;
  *(uint64_t*)0x20fc6ff0 = 0;
  *(uint64_t*)0x20fc6ff8 = 0x300;
  syscall(__NR_ioctl, r[2], 0x4090ae82, 0x20fc6f70);
  *(uint64_t*)0x207ff000 = 0x208fa000;
  *(uint64_t*)0x207ff008 = 0x4000;
  *(uint64_t*)0x207ff010 = 2;
  *(uint64_t*)0x207ff018 = 0;
  syscall(__NR_ioctl, -1, 0xc020aa00, 0x207ff000);
  syscall(__NR_setreuid, 0, 0);
  syscall(__NR_ioctl, r[2], 0xae80, 0);
}

int main()
{
  loop();
  return 0;
}
