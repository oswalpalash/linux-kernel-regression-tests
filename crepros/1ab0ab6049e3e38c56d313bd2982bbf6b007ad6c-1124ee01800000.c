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
  syscall(__NR_mmap, 0x20000000ul, 0xafb000ul, 0x3ul, 0x32ul,
          0xfffffffffffffffful, 0x0ul);
  memcpy((void*)0x202e9ff7, "/dev/kvm", 9);
  r[0] = syscall(__NR_openat, 0xffffffffffffff9cul, 0x202e9ff7ul, 0x0ul,
                 0x0ul);
  r[1] = syscall(__NR_ioctl, r[0], 0xae01ul, 0x0ul);
  r[2] = syscall(__NR_ioctl, r[1], 0xae41ul, 0x0ul);
  *(uint64_t*)0x203de000 = (uint64_t)0x2000;
  *(uint32_t*)0x203de008 = (uint32_t)0x3000;
  *(uint16_t*)0x203de00c = (uint16_t)0xf;
  *(uint8_t*)0x203de00e = (uint8_t)0x2;
  *(uint8_t*)0x203de00f = (uint8_t)0x9cc;
  *(uint8_t*)0x203de010 = (uint8_t)0x0;
  *(uint8_t*)0x203de011 = (uint8_t)0x9;
  *(uint8_t*)0x203de012 = (uint8_t)0x101;
  *(uint8_t*)0x203de013 = (uint8_t)0x8;
  *(uint8_t*)0x203de014 = (uint8_t)0x0;
  *(uint8_t*)0x203de015 = (uint8_t)0x20;
  *(uint8_t*)0x203de016 = (uint8_t)0x4;
  *(uint8_t*)0x203de017 = (uint8_t)0x0;
  *(uint64_t*)0x203de018 = (uint64_t)0x5000;
  *(uint32_t*)0x203de020 = (uint32_t)0x1;
  *(uint16_t*)0x203de024 = (uint16_t)0x3;
  *(uint8_t*)0x203de026 = (uint8_t)0x7;
  *(uint8_t*)0x203de027 = (uint8_t)0x7;
  *(uint8_t*)0x203de028 = (uint8_t)0x96f;
  *(uint8_t*)0x203de029 = (uint8_t)0x94;
  *(uint8_t*)0x203de02a = (uint8_t)0xe3b0000;
  *(uint8_t*)0x203de02b = (uint8_t)0x7;
  *(uint8_t*)0x203de02c = (uint8_t)0x800;
  *(uint8_t*)0x203de02d = (uint8_t)0x696;
  *(uint8_t*)0x203de02e = (uint8_t)0x1365;
  *(uint8_t*)0x203de02f = (uint8_t)0x0;
  *(uint64_t*)0x203de030 = (uint64_t)0x1000;
  *(uint32_t*)0x203de038 = (uint32_t)0x7003;
  *(uint16_t*)0x203de03c = (uint16_t)0xccb4f35bac6e5ef7;
  *(uint8_t*)0x203de03e = (uint8_t)0x3;
  *(uint8_t*)0x203de03f = (uint8_t)0x7f;
  *(uint8_t*)0x203de040 = (uint8_t)0x80000001;
  *(uint8_t*)0x203de041 = (uint8_t)0x0;
  *(uint8_t*)0x203de042 = (uint8_t)0x9;
  *(uint8_t*)0x203de043 = (uint8_t)0xb47;
  *(uint8_t*)0x203de044 = (uint8_t)0x8;
  *(uint8_t*)0x203de045 = (uint8_t)0x6;
  *(uint8_t*)0x203de046 = (uint8_t)0xd3de;
  *(uint8_t*)0x203de047 = (uint8_t)0x0;
  *(uint64_t*)0x203de048 = (uint64_t)0x4000;
  *(uint32_t*)0x203de050 = (uint32_t)0x13004;
  *(uint16_t*)0x203de054 = (uint16_t)0x10;
  *(uint8_t*)0x203de056 = (uint8_t)0x0;
  *(uint8_t*)0x203de057 = (uint8_t)0x7;
  *(uint8_t*)0x203de058 = (uint8_t)0x0;
  *(uint8_t*)0x203de059 = (uint8_t)0x300000000000;
  *(uint8_t*)0x203de05a = (uint8_t)0x9da;
  *(uint8_t*)0x203de05b = (uint8_t)0xfff;
  *(uint8_t*)0x203de05c = (uint8_t)0x4;
  *(uint8_t*)0x203de05d = (uint8_t)0xdce1;
  *(uint8_t*)0x203de05e = (uint8_t)0x5;
  *(uint8_t*)0x203de05f = (uint8_t)0x0;
  *(uint64_t*)0x203de060 = (uint64_t)0x2001;
  *(uint32_t*)0x203de068 = (uint32_t)0x3000;
  *(uint16_t*)0x203de06c = (uint16_t)0xf;
  *(uint8_t*)0x203de06e = (uint8_t)0x3f;
  *(uint8_t*)0x203de06f = (uint8_t)0x7;
  *(uint8_t*)0x203de070 = (uint8_t)0x2;
  *(uint8_t*)0x203de071 = (uint8_t)0xe6e8;
  *(uint8_t*)0x203de072 = (uint8_t)0x3;
  *(uint8_t*)0x203de073 = (uint8_t)0x80000000;
  *(uint8_t*)0x203de074 = (uint8_t)0x80000000;
  *(uint8_t*)0x203de075 = (uint8_t)0x2f;
  *(uint8_t*)0x203de076 = (uint8_t)0x5;
  *(uint8_t*)0x203de077 = (uint8_t)0x0;
  *(uint64_t*)0x203de078 = (uint64_t)0x5000;
  *(uint32_t*)0x203de080 = (uint32_t)0x3004;
  *(uint16_t*)0x203de084 = (uint16_t)0xf;
  *(uint8_t*)0x203de086 = (uint8_t)0x3;
  *(uint8_t*)0x203de087 = (uint8_t)0xe61c;
  *(uint8_t*)0x203de088 = (uint8_t)0x48000000;
  *(uint8_t*)0x203de089 = (uint8_t)0x70;
  *(uint8_t*)0x203de08a = (uint8_t)0x401;
  *(uint8_t*)0x203de08b = (uint8_t)0xfffffffffffffffa;
  *(uint8_t*)0x203de08c = (uint8_t)0x3c3e;
  *(uint8_t*)0x203de08d = (uint8_t)0x0;
  *(uint8_t*)0x203de08e = (uint8_t)0x10001;
  *(uint8_t*)0x203de08f = (uint8_t)0x0;
  *(uint64_t*)0x203de090 = (uint64_t)0x6000;
  *(uint32_t*)0x203de098 = (uint32_t)0x100000;
  *(uint16_t*)0x203de09c = (uint16_t)0xa;
  *(uint8_t*)0x203de09e = (uint8_t)0x1;
  *(uint8_t*)0x203de09f = (uint8_t)0x6;
  *(uint8_t*)0x203de0a0 = (uint8_t)0xfffffffffffffff7;
  *(uint8_t*)0x203de0a1 = (uint8_t)0x401;
  *(uint8_t*)0x203de0a2 = (uint8_t)0x1;
  *(uint8_t*)0x203de0a3 = (uint8_t)0x101;
  *(uint8_t*)0x203de0a4 = (uint8_t)0x4;
  *(uint8_t*)0x203de0a5 = (uint8_t)0xff;
  *(uint8_t*)0x203de0a6 = (uint8_t)0x384;
  *(uint8_t*)0x203de0a7 = (uint8_t)0x0;
  *(uint64_t*)0x203de0a8 = (uint64_t)0x10000;
  *(uint32_t*)0x203de0b0 = (uint32_t)0x11f000;
  *(uint16_t*)0x203de0b4 = (uint16_t)0xf;
  *(uint8_t*)0x203de0b6 = (uint8_t)0x40;
  *(uint8_t*)0x203de0b7 = (uint8_t)0x6f;
  *(uint8_t*)0x203de0b8 = (uint8_t)0x1f;
  *(uint8_t*)0x203de0b9 = (uint8_t)0x0;
  *(uint8_t*)0x203de0ba = (uint8_t)0x2;
  *(uint8_t*)0x203de0bb = (uint8_t)0x81;
  *(uint8_t*)0x203de0bc = (uint8_t)0x5;
  *(uint8_t*)0x203de0bd = (uint8_t)0x401;
  *(uint8_t*)0x203de0be = (uint8_t)0xa3d;
  *(uint8_t*)0x203de0bf = (uint8_t)0x0;
  *(uint64_t*)0x203de0c0 = (uint64_t)0x100000;
  *(uint16_t*)0x203de0c8 = (uint16_t)0x5000;
  *(uint16_t*)0x203de0ca = (uint16_t)0x0;
  *(uint16_t*)0x203de0cc = (uint16_t)0x0;
  *(uint16_t*)0x203de0ce = (uint16_t)0x0;
  *(uint64_t*)0x203de0d0 = (uint64_t)0x5000;
  *(uint16_t*)0x203de0d8 = (uint16_t)0x1000;
  *(uint16_t*)0x203de0da = (uint16_t)0x0;
  *(uint16_t*)0x203de0dc = (uint16_t)0x0;
  *(uint16_t*)0x203de0de = (uint16_t)0x0;
  *(uint64_t*)0x203de0e0 = (uint64_t)0x10001;
  *(uint64_t*)0x203de0e8 = (uint64_t)0x0;
  *(uint64_t*)0x203de0f0 = (uint64_t)0x2000;
  *(uint64_t*)0x203de0f8 = (uint64_t)0x20;
  *(uint64_t*)0x203de100 = (uint64_t)0x5;
  *(uint64_t*)0x203de108 = (uint64_t)0x800;
  *(uint64_t*)0x203de110 = (uint64_t)0x4;
  *(uint64_t*)0x203de118 = (uint64_t)0x7fff;
  *(uint64_t*)0x203de120 = (uint64_t)0x7f;
  *(uint64_t*)0x203de128 = (uint64_t)0xffffffffffff7a04;
  *(uint64_t*)0x203de130 = (uint64_t)0x8000;
  syscall(__NR_ioctl, r[2], 0x4138ae84ul, 0x203de000ul);
}

int main()
{
  loop();
  return 0;
}
