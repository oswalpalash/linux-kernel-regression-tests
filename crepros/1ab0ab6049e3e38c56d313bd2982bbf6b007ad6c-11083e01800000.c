// https://syzkaller.appspot.com/bug?id=0d259373da8be7356652213543e1efc254a5abf0
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <sys/syscall.h>
#include <unistd.h>

#include <stdint.h>
#include <string.h>

long r[3];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
          0xfffffffffffffffful, 0x0ul);
  memcpy((void*)0x20e37000, "/dev/kvm", 9);
  r[0] = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20e37000ul, 0x0ul,
                 0x0ul);
  r[1] = syscall(__NR_ioctl, r[0], 0xae01ul, 0x0ul);
  r[2] = syscall(__NR_ioctl, r[1], 0xae41ul, 0x0ul);
  *(uint64_t*)0x2099afd0 = (uint64_t)0x2000;
  *(uint32_t*)0x2099afd8 = (uint32_t)0x6000;
  *(uint16_t*)0x2099afdc = (uint16_t)0xa;
  *(uint8_t*)0x2099afde = (uint8_t)0x11;
  *(uint8_t*)0x2099afdf = (uint8_t)0xff;
  *(uint8_t*)0x2099afe0 = (uint8_t)0x5e3fc9f4;
  *(uint8_t*)0x2099afe1 = (uint8_t)0x1;
  *(uint8_t*)0x2099afe2 = (uint8_t)0x81;
  *(uint8_t*)0x2099afe3 = (uint8_t)0x0;
  *(uint8_t*)0x2099afe4 = (uint8_t)0x6;
  *(uint8_t*)0x2099afe5 = (uint8_t)0x4;
  *(uint8_t*)0x2099afe6 = (uint8_t)0x101;
  *(uint8_t*)0x2099afe7 = (uint8_t)0x0;
  *(uint64_t*)0x2099afe8 = (uint64_t)0xf000;
  *(uint32_t*)0x2099aff0 = (uint32_t)0xd000;
  *(uint16_t*)0x2099aff4 = (uint16_t)0x0;
  *(uint8_t*)0x2099aff6 = (uint8_t)0x7;
  *(uint8_t*)0x2099aff7 = (uint8_t)0x1000;
  *(uint8_t*)0x2099aff8 = (uint8_t)0x1205;
  *(uint8_t*)0x2099aff9 = (uint8_t)0x3;
  *(uint8_t*)0x2099affa = (uint8_t)0x80000000;
  *(uint8_t*)0x2099affb = (uint8_t)0xb27;
  *(uint8_t*)0x2099affc = (uint8_t)0x5;
  *(uint8_t*)0x2099affd = (uint8_t)0x9;
  *(uint8_t*)0x2099affe = (uint8_t)0x0;
  *(uint8_t*)0x2099afff = (uint8_t)0x0;
  *(uint64_t*)0x2099b000 = (uint64_t)0x3000;
  *(uint32_t*)0x2099b008 = (uint32_t)0x0;
  *(uint16_t*)0x2099b00c = (uint16_t)0xf;
  *(uint8_t*)0x2099b00e = (uint8_t)0x80000000;
  *(uint8_t*)0x2099b00f = (uint8_t)0x1ff;
  *(uint8_t*)0x2099b010 = (uint8_t)0x7ff;
  *(uint8_t*)0x2099b011 = (uint8_t)0x1;
  *(uint8_t*)0x2099b012 = (uint8_t)0x7;
  *(uint8_t*)0x2099b013 = (uint8_t)0xf84;
  *(uint8_t*)0x2099b014 = (uint8_t)0x1ff;
  *(uint8_t*)0x2099b015 = (uint8_t)0x2;
  *(uint8_t*)0x2099b016 = (uint8_t)0x1000;
  *(uint8_t*)0x2099b017 = (uint8_t)0x0;
  *(uint64_t*)0x2099b018 = (uint64_t)0x3000;
  *(uint32_t*)0x2099b020 = (uint32_t)0x10000;
  *(uint16_t*)0x2099b024 = (uint16_t)0x8;
  *(uint8_t*)0x2099b026 = (uint8_t)0x1;
  *(uint8_t*)0x2099b027 = (uint8_t)0x7;
  *(uint8_t*)0x2099b028 = (uint8_t)0x9de1;
  *(uint8_t*)0x2099b029 = (uint8_t)0xffffffffffffffc1;
  *(uint8_t*)0x2099b02a = (uint8_t)0x7;
  *(uint8_t*)0x2099b02b = (uint8_t)0x7fff;
  *(uint8_t*)0x2099b02c = (uint8_t)0x9;
  *(uint8_t*)0x2099b02d = (uint8_t)0x1;
  *(uint8_t*)0x2099b02e = (uint8_t)0x89fd;
  *(uint8_t*)0x2099b02f = (uint8_t)0x0;
  *(uint64_t*)0x2099b030 = (uint64_t)0x2;
  *(uint32_t*)0x2099b038 = (uint32_t)0x1000;
  *(uint16_t*)0x2099b03c = (uint16_t)0x86ef34c8e8fc3ca8;
  *(uint8_t*)0x2099b03e = (uint8_t)0xdc33;
  *(uint8_t*)0x2099b03f = (uint8_t)0xffffffff;
  *(uint8_t*)0x2099b040 = (uint8_t)0xae13;
  *(uint8_t*)0x2099b041 = (uint8_t)0x100000000;
  *(uint8_t*)0x2099b042 = (uint8_t)0x10000;
  *(uint8_t*)0x2099b043 = (uint8_t)0x5;
  *(uint8_t*)0x2099b044 = (uint8_t)0xfff;
  *(uint8_t*)0x2099b045 = (uint8_t)0x4;
  *(uint8_t*)0x2099b046 = (uint8_t)0x7;
  *(uint8_t*)0x2099b047 = (uint8_t)0x0;
  *(uint64_t*)0x2099b048 = (uint64_t)0x100000;
  *(uint32_t*)0x2099b050 = (uint32_t)0xd004;
  *(uint16_t*)0x2099b054 = (uint16_t)0x4;
  *(uint8_t*)0x2099b056 = (uint8_t)0x3;
  *(uint8_t*)0x2099b057 = (uint8_t)0x4;
  *(uint8_t*)0x2099b058 = (uint8_t)0x1;
  *(uint8_t*)0x2099b059 = (uint8_t)0xfffffffffffffb53;
  *(uint8_t*)0x2099b05a = (uint8_t)0x9b;
  *(uint8_t*)0x2099b05b = (uint8_t)0x1;
  *(uint8_t*)0x2099b05c = (uint8_t)0x2;
  *(uint8_t*)0x2099b05d = (uint8_t)0x3;
  *(uint8_t*)0x2099b05e = (uint8_t)0x2;
  *(uint8_t*)0x2099b05f = (uint8_t)0x0;
  *(uint64_t*)0x2099b060 = (uint64_t)0x6001;
  *(uint32_t*)0x2099b068 = (uint32_t)0xd000;
  *(uint16_t*)0x2099b06c = (uint16_t)0x0;
  *(uint8_t*)0x2099b06e = (uint8_t)0x64d;
  *(uint8_t*)0x2099b06f = (uint8_t)0x9;
  *(uint8_t*)0x2099b070 = (uint8_t)0xba70;
  *(uint8_t*)0x2099b071 = (uint8_t)0xfff;
  *(uint8_t*)0x2099b072 = (uint8_t)0x1000;
  *(uint8_t*)0x2099b073 = (uint8_t)0x22;
  *(uint8_t*)0x2099b074 = (uint8_t)0x88e;
  *(uint8_t*)0x2099b075 = (uint8_t)0x91;
  *(uint8_t*)0x2099b076 = (uint8_t)0x8;
  *(uint8_t*)0x2099b077 = (uint8_t)0x0;
  *(uint64_t*)0x2099b078 = (uint64_t)0x0;
  *(uint32_t*)0x2099b080 = (uint32_t)0xf002;
  *(uint16_t*)0x2099b084 = (uint16_t)0x19;
  *(uint8_t*)0x2099b086 = (uint8_t)0x4;
  *(uint8_t*)0x2099b087 = (uint8_t)0x5;
  *(uint8_t*)0x2099b088 = (uint8_t)0x3;
  *(uint8_t*)0x2099b089 = (uint8_t)0x0;
  *(uint8_t*)0x2099b08a = (uint8_t)0x7;
  *(uint8_t*)0x2099b08b = (uint8_t)0x8;
  *(uint8_t*)0x2099b08c = (uint8_t)0x8;
  *(uint8_t*)0x2099b08d = (uint8_t)0x9;
  *(uint8_t*)0x2099b08e = (uint8_t)0x100;
  *(uint8_t*)0x2099b08f = (uint8_t)0x0;
  *(uint64_t*)0x2099b090 = (uint64_t)0x3000;
  *(uint16_t*)0x2099b098 = (uint16_t)0xf000;
  *(uint16_t*)0x2099b09a = (uint16_t)0x0;
  *(uint16_t*)0x2099b09c = (uint16_t)0x0;
  *(uint16_t*)0x2099b09e = (uint16_t)0x0;
  *(uint64_t*)0x2099b0a0 = (uint64_t)0x2000;
  *(uint16_t*)0x2099b0a8 = (uint16_t)0x3000;
  *(uint16_t*)0x2099b0aa = (uint16_t)0x0;
  *(uint16_t*)0x2099b0ac = (uint16_t)0x0;
  *(uint16_t*)0x2099b0ae = (uint16_t)0x0;
  *(uint64_t*)0x2099b0b0 = (uint64_t)0x1;
  *(uint64_t*)0x2099b0b8 = (uint64_t)0x0;
  *(uint64_t*)0x2099b0c0 = (uint64_t)0x10d000;
  *(uint64_t*)0x2099b0c8 = (uint64_t)0x100000;
  *(uint64_t*)0x2099b0d0 = (uint64_t)0x2;
  *(uint64_t*)0x2099b0d8 = (uint64_t)0x100;
  *(uint64_t*)0x2099b0e0 = (uint64_t)0x7002;
  *(uint64_t*)0x2099b0e8 = (uint64_t)0x2;
  *(uint64_t*)0x2099b0f0 = (uint64_t)0x8;
  *(uint64_t*)0x2099b0f8 = (uint64_t)0x2;
  *(uint64_t*)0x2099b100 = (uint64_t)0x80000001;
  syscall(__NR_ioctl, r[2], 0x4138ae84ul, 0x2099afd0ul);
}

int main()
{
  loop();
  return 0;
}
