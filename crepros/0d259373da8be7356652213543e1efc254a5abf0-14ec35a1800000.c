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
  memcpy((void*)0x20fe5ff7, "/dev/kvm", 9);
  r[0] = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20fe5ff7ul, 0x0ul,
                 0x0ul);
  r[1] = syscall(__NR_ioctl, r[0], 0xae01ul, 0x0ul);
  r[2] = syscall(__NR_ioctl, r[1], 0xae41ul, 0x0ul);
  *(uint64_t*)0x20b18000 = (uint64_t)0x0;
  *(uint32_t*)0x20b18008 = (uint32_t)0x1;
  *(uint16_t*)0x20b1800c = (uint16_t)0xd;
  *(uint8_t*)0x20b1800e = (uint8_t)0x80000001;
  *(uint8_t*)0x20b1800f = (uint8_t)0x6;
  *(uint8_t*)0x20b18010 = (uint8_t)0x79;
  *(uint8_t*)0x20b18011 = (uint8_t)0x7;
  *(uint8_t*)0x20b18012 = (uint8_t)0x1000;
  *(uint8_t*)0x20b18013 = (uint8_t)0xfffffffffffffc28;
  *(uint8_t*)0x20b18014 = (uint8_t)0x2;
  *(uint8_t*)0x20b18015 = (uint8_t)0x9;
  *(uint8_t*)0x20b18016 = (uint8_t)0x100000001;
  *(uint8_t*)0x20b18017 = (uint8_t)0x0;
  *(uint64_t*)0x20b18018 = (uint64_t)0xf000;
  *(uint32_t*)0x20b18020 = (uint32_t)0x1004;
  *(uint16_t*)0x20b18024 = (uint16_t)0xf;
  *(uint8_t*)0x20b18026 = (uint8_t)0x1;
  *(uint8_t*)0x20b18027 = (uint8_t)0x9;
  *(uint8_t*)0x20b18028 = (uint8_t)0x7;
  *(uint8_t*)0x20b18029 = (uint8_t)0x28;
  *(uint8_t*)0x20b1802a = (uint8_t)0x101;
  *(uint8_t*)0x20b1802b = (uint8_t)0x8001;
  *(uint8_t*)0x20b1802c = (uint8_t)0xb1a;
  *(uint8_t*)0x20b1802d = (uint8_t)0x4;
  *(uint8_t*)0x20b1802e = (uint8_t)0x4;
  *(uint8_t*)0x20b1802f = (uint8_t)0x0;
  *(uint64_t*)0x20b18030 = (uint64_t)0x101000;
  *(uint32_t*)0x20b18038 = (uint32_t)0x2000;
  *(uint16_t*)0x20b1803c = (uint16_t)0x0;
  *(uint8_t*)0x20b1803e = (uint8_t)0x9;
  *(uint8_t*)0x20b1803f = (uint8_t)0x6;
  *(uint8_t*)0x20b18040 = (uint8_t)0xef;
  *(uint8_t*)0x20b18041 = (uint8_t)0xde;
  *(uint8_t*)0x20b18042 = (uint8_t)0x1000;
  *(uint8_t*)0x20b18043 = (uint8_t)0x1;
  *(uint8_t*)0x20b18044 = (uint8_t)0x3;
  *(uint8_t*)0x20b18045 = (uint8_t)0x400;
  *(uint8_t*)0x20b18046 = (uint8_t)0xb5fe;
  *(uint8_t*)0x20b18047 = (uint8_t)0x0;
  *(uint64_t*)0x20b18048 = (uint64_t)0x2000;
  *(uint32_t*)0x20b18050 = (uint32_t)0x100002;
  *(uint16_t*)0x20b18054 = (uint16_t)0x0;
  *(uint8_t*)0x20b18056 = (uint8_t)0x46;
  *(uint8_t*)0x20b18057 = (uint8_t)0x1f;
  *(uint8_t*)0x20b18058 = (uint8_t)0x55;
  *(uint8_t*)0x20b18059 = (uint8_t)0x300000000;
  *(uint8_t*)0x20b1805a = (uint8_t)0xe4;
  *(uint8_t*)0x20b1805b = (uint8_t)0x1;
  *(uint8_t*)0x20b1805c = (uint8_t)0x45db46b3;
  *(uint8_t*)0x20b1805d = (uint8_t)0x3;
  *(uint8_t*)0x20b1805e = (uint8_t)0x6;
  *(uint8_t*)0x20b1805f = (uint8_t)0x0;
  *(uint64_t*)0x20b18060 = (uint64_t)0x0;
  *(uint32_t*)0x20b18068 = (uint32_t)0x2;
  *(uint16_t*)0x20b1806c = (uint16_t)0xf;
  *(uint8_t*)0x20b1806e = (uint8_t)0x7;
  *(uint8_t*)0x20b1806f = (uint8_t)0x6;
  *(uint8_t*)0x20b18070 = (uint8_t)0xfffffffffffffffd;
  *(uint8_t*)0x20b18071 = (uint8_t)0x2;
  *(uint8_t*)0x20b18072 = (uint8_t)0xfa7;
  *(uint8_t*)0x20b18073 = (uint8_t)0x4;
  *(uint8_t*)0x20b18074 = (uint8_t)0x7f;
  *(uint8_t*)0x20b18075 = (uint8_t)0x51c;
  *(uint8_t*)0x20b18076 = (uint8_t)0x9;
  *(uint8_t*)0x20b18077 = (uint8_t)0x0;
  *(uint64_t*)0x20b18078 = (uint64_t)0x0;
  *(uint32_t*)0x20b18080 = (uint32_t)0x1;
  *(uint16_t*)0x20b18084 = (uint16_t)0x4;
  *(uint8_t*)0x20b18086 = (uint8_t)0x7;
  *(uint8_t*)0x20b18087 = (uint8_t)0x1;
  *(uint8_t*)0x20b18088 = (uint8_t)0x0;
  *(uint8_t*)0x20b18089 = (uint8_t)0x8;
  *(uint8_t*)0x20b1808a = (uint8_t)0x3;
  *(uint8_t*)0x20b1808b = (uint8_t)0xfffffffffffffffe;
  *(uint8_t*)0x20b1808c = (uint8_t)0x1;
  *(uint8_t*)0x20b1808d = (uint8_t)0x7;
  *(uint8_t*)0x20b1808e = (uint8_t)0x6;
  *(uint8_t*)0x20b1808f = (uint8_t)0x0;
  *(uint64_t*)0x20b18090 = (uint64_t)0x2000;
  *(uint32_t*)0x20b18098 = (uint32_t)0x5000;
  *(uint16_t*)0x20b1809c = (uint16_t)0xf;
  *(uint8_t*)0x20b1809e = (uint8_t)0x8;
  *(uint8_t*)0x20b1809f = (uint8_t)0x6;
  *(uint8_t*)0x20b180a0 = (uint8_t)0x400;
  *(uint8_t*)0x20b180a1 = (uint8_t)0x0;
  *(uint8_t*)0x20b180a2 = (uint8_t)0x2f;
  *(uint8_t*)0x20b180a3 = (uint8_t)0x20;
  *(uint8_t*)0x20b180a4 = (uint8_t)0x81;
  *(uint8_t*)0x20b180a5 = (uint8_t)0x0;
  *(uint8_t*)0x20b180a6 = (uint8_t)0x1;
  *(uint8_t*)0x20b180a7 = (uint8_t)0x0;
  *(uint64_t*)0x20b180a8 = (uint64_t)0x0;
  *(uint32_t*)0x20b180b0 = (uint32_t)0x5002;
  *(uint16_t*)0x20b180b4 = (uint16_t)0xb;
  *(uint8_t*)0x20b180b6 = (uint8_t)0x4;
  *(uint8_t*)0x20b180b7 = (uint8_t)0x4;
  *(uint8_t*)0x20b180b8 = (uint8_t)0x100;
  *(uint8_t*)0x20b180b9 = (uint8_t)0x9d8d;
  *(uint8_t*)0x20b180ba = (uint8_t)0x80000001;
  *(uint8_t*)0x20b180bb = (uint8_t)0x2;
  *(uint8_t*)0x20b180bc = (uint8_t)0x948;
  *(uint8_t*)0x20b180bd = (uint8_t)0x3f;
  *(uint8_t*)0x20b180be = (uint8_t)0x1;
  *(uint8_t*)0x20b180bf = (uint8_t)0x0;
  *(uint64_t*)0x20b180c0 = (uint64_t)0x4;
  *(uint16_t*)0x20b180c8 = (uint16_t)0x0;
  *(uint16_t*)0x20b180ca = (uint16_t)0x0;
  *(uint16_t*)0x20b180cc = (uint16_t)0x0;
  *(uint16_t*)0x20b180ce = (uint16_t)0x0;
  *(uint64_t*)0x20b180d0 = (uint64_t)0x10000;
  *(uint16_t*)0x20b180d8 = (uint16_t)0x2000;
  *(uint16_t*)0x20b180da = (uint16_t)0x0;
  *(uint16_t*)0x20b180dc = (uint16_t)0x0;
  *(uint16_t*)0x20b180de = (uint16_t)0x0;
  *(uint64_t*)0x20b180e0 = (uint64_t)0x1;
  *(uint64_t*)0x20b180e8 = (uint64_t)0x0;
  *(uint64_t*)0x20b180f0 = (uint64_t)0x7000;
  *(uint64_t*)0x20b180f8 = (uint64_t)0x2;
  *(uint64_t*)0x20b18100 = (uint64_t)0x1;
  *(uint64_t*)0x20b18108 = (uint64_t)0x400;
  *(uint64_t*)0x20b18110 = (uint64_t)0xd000;
  *(uint64_t*)0x20b18118 = (uint64_t)0x5;
  *(uint64_t*)0x20b18120 = (uint64_t)0x0;
  *(uint64_t*)0x20b18128 = (uint64_t)0xf3;
  *(uint64_t*)0x20b18130 = (uint64_t)0x6;
  syscall(__NR_ioctl, r[2], 0x4138ae84ul, 0x20b18000ul);
}

int main()
{
  loop();
  return 0;
}
