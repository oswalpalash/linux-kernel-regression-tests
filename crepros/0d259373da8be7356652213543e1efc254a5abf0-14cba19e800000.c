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
  memcpy((void*)0x20508ff7, "/dev/kvm", 9);
  r[0] = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20508ff7ul, 0x0ul,
                 0x0ul);
  r[1] = syscall(__NR_ioctl, r[0], 0xae01ul, 0x0ul);
  r[2] = syscall(__NR_ioctl, r[1], 0xae41ul, 0x2ul);
  *(uint64_t*)0x20002000 = (uint64_t)0x0;
  *(uint32_t*)0x20002008 = (uint32_t)0xffffffffffffffff;
  *(uint16_t*)0x2000200c = (uint16_t)0x0;
  *(uint8_t*)0x2000200e = (uint8_t)0x0;
  *(uint8_t*)0x2000200f = (uint8_t)0x0;
  *(uint8_t*)0x20002010 = (uint8_t)0x5;
  *(uint8_t*)0x20002011 = (uint8_t)0xfffffffffffffffd;
  *(uint8_t*)0x20002012 = (uint8_t)0x0;
  *(uint8_t*)0x20002013 = (uint8_t)0x0;
  *(uint8_t*)0x20002014 = (uint8_t)0x0;
  *(uint8_t*)0x20002015 = (uint8_t)0x0;
  *(uint8_t*)0x20002016 = (uint8_t)0x0;
  *(uint8_t*)0x20002017 = (uint8_t)0x0;
  *(uint64_t*)0x20002018 = (uint64_t)0x0;
  *(uint32_t*)0x20002020 = (uint32_t)0x0;
  *(uint16_t*)0x20002024 = (uint16_t)0x0;
  *(uint8_t*)0x20002026 = (uint8_t)0x3;
  *(uint8_t*)0x20002027 = (uint8_t)0x0;
  *(uint8_t*)0x20002028 = (uint8_t)0x0;
  *(uint8_t*)0x20002029 = (uint8_t)0x0;
  *(uint8_t*)0x2000202a = (uint8_t)0x400000;
  *(uint8_t*)0x2000202b = (uint8_t)0x0;
  *(uint8_t*)0x2000202c = (uint8_t)0x0;
  *(uint8_t*)0x2000202d = (uint8_t)0x0;
  *(uint8_t*)0x2000202e = (uint8_t)0x0;
  *(uint8_t*)0x2000202f = (uint8_t)0x0;
  *(uint64_t*)0x20002030 = (uint64_t)0x0;
  *(uint32_t*)0x20002038 = (uint32_t)0xf000;
  *(uint16_t*)0x2000203c = (uint16_t)0x0;
  *(uint8_t*)0x2000203e = (uint8_t)0x0;
  *(uint8_t*)0x2000203f = (uint8_t)0x0;
  *(uint8_t*)0x20002040 = (uint8_t)0x0;
  *(uint8_t*)0x20002041 = (uint8_t)0x100000001;
  *(uint8_t*)0x20002042 = (uint8_t)0x0;
  *(uint8_t*)0x20002043 = (uint8_t)0x100000000;
  *(uint8_t*)0x20002044 = (uint8_t)0x0;
  *(uint8_t*)0x20002045 = (uint8_t)0x0;
  *(uint8_t*)0x20002046 = (uint8_t)0x0;
  *(uint8_t*)0x20002047 = (uint8_t)0x0;
  *(uint64_t*)0x20002048 = (uint64_t)0x0;
  *(uint32_t*)0x20002050 = (uint32_t)0x0;
  *(uint16_t*)0x20002054 = (uint16_t)0x0;
  *(uint8_t*)0x20002056 = (uint8_t)0x0;
  *(uint8_t*)0x20002057 = (uint8_t)0x0;
  *(uint8_t*)0x20002058 = (uint8_t)0x3;
  *(uint8_t*)0x20002059 = (uint8_t)0x0;
  *(uint8_t*)0x2000205a = (uint8_t)0x0;
  *(uint8_t*)0x2000205b = (uint8_t)0x0;
  *(uint8_t*)0x2000205c = (uint8_t)0x0;
  *(uint8_t*)0x2000205d = (uint8_t)0x0;
  *(uint8_t*)0x2000205e = (uint8_t)0x0;
  *(uint8_t*)0x2000205f = (uint8_t)0x0;
  *(uint64_t*)0x20002060 = (uint64_t)0x0;
  *(uint32_t*)0x20002068 = (uint32_t)0x0;
  *(uint16_t*)0x2000206c = (uint16_t)0x0;
  *(uint8_t*)0x2000206e = (uint8_t)0x0;
  *(uint8_t*)0x2000206f = (uint8_t)0x0;
  *(uint8_t*)0x20002070 = (uint8_t)0x0;
  *(uint8_t*)0x20002071 = (uint8_t)0x0;
  *(uint8_t*)0x20002072 = (uint8_t)0xfffffffffffffffe;
  *(uint8_t*)0x20002073 = (uint8_t)0x0;
  *(uint8_t*)0x20002074 = (uint8_t)0x3;
  *(uint8_t*)0x20002075 = (uint8_t)0x0;
  *(uint8_t*)0x20002076 = (uint8_t)0x0;
  *(uint8_t*)0x20002077 = (uint8_t)0x0;
  *(uint64_t*)0x20002078 = (uint64_t)0x0;
  *(uint32_t*)0x20002080 = (uint32_t)0x0;
  *(uint16_t*)0x20002084 = (uint16_t)0x8;
  *(uint8_t*)0x20002086 = (uint8_t)0x0;
  *(uint8_t*)0x20002087 = (uint8_t)0x0;
  *(uint8_t*)0x20002088 = (uint8_t)0x0;
  *(uint8_t*)0x20002089 = (uint8_t)0x0;
  *(uint8_t*)0x2000208a = (uint8_t)0x0;
  *(uint8_t*)0x2000208b = (uint8_t)0x0;
  *(uint8_t*)0x2000208c = (uint8_t)0x0;
  *(uint8_t*)0x2000208d = (uint8_t)0x0;
  *(uint8_t*)0x2000208e = (uint8_t)0x0;
  *(uint8_t*)0x2000208f = (uint8_t)0x0;
  *(uint64_t*)0x20002090 = (uint64_t)0x0;
  *(uint32_t*)0x20002098 = (uint32_t)0x14000;
  *(uint16_t*)0x2000209c = (uint16_t)0x0;
  *(uint8_t*)0x2000209e = (uint8_t)0x0;
  *(uint8_t*)0x2000209f = (uint8_t)0x0;
  *(uint8_t*)0x200020a0 = (uint8_t)0x0;
  *(uint8_t*)0x200020a1 = (uint8_t)0x0;
  *(uint8_t*)0x200020a2 = (uint8_t)0x0;
  *(uint8_t*)0x200020a3 = (uint8_t)0x0;
  *(uint8_t*)0x200020a4 = (uint8_t)0x0;
  *(uint8_t*)0x200020a5 = (uint8_t)0x0;
  *(uint8_t*)0x200020a6 = (uint8_t)0x0;
  *(uint8_t*)0x200020a7 = (uint8_t)0x0;
  *(uint64_t*)0x200020a8 = (uint64_t)0x0;
  *(uint32_t*)0x200020b0 = (uint32_t)0x0;
  *(uint16_t*)0x200020b4 = (uint16_t)0x0;
  *(uint8_t*)0x200020b6 = (uint8_t)0x8000000000;
  *(uint8_t*)0x200020b7 = (uint8_t)0x0;
  *(uint8_t*)0x200020b8 = (uint8_t)0x0;
  *(uint8_t*)0x200020b9 = (uint8_t)0x0;
  *(uint8_t*)0x200020ba = (uint8_t)0x0;
  *(uint8_t*)0x200020bb = (uint8_t)0x0;
  *(uint8_t*)0x200020bc = (uint8_t)0x0;
  *(uint8_t*)0x200020bd = (uint8_t)0x0;
  *(uint8_t*)0x200020be = (uint8_t)0xfffffffffffffffd;
  *(uint8_t*)0x200020bf = (uint8_t)0x0;
  *(uint64_t*)0x200020c0 = (uint64_t)0x0;
  *(uint16_t*)0x200020c8 = (uint16_t)0x0;
  *(uint16_t*)0x200020ca = (uint16_t)0x0;
  *(uint16_t*)0x200020cc = (uint16_t)0x0;
  *(uint16_t*)0x200020ce = (uint16_t)0x0;
  *(uint64_t*)0x200020d0 = (uint64_t)0x0;
  *(uint16_t*)0x200020d8 = (uint16_t)0x0;
  *(uint16_t*)0x200020da = (uint16_t)0x0;
  *(uint16_t*)0x200020dc = (uint16_t)0x0;
  *(uint16_t*)0x200020de = (uint16_t)0x0;
  *(uint64_t*)0x200020e0 = (uint64_t)0x80000015;
  *(uint64_t*)0x200020e8 = (uint64_t)0x0;
  *(uint64_t*)0x200020f0 = (uint64_t)0x0;
  *(uint64_t*)0x200020f8 = (uint64_t)0x600000;
  *(uint64_t*)0x20002100 = (uint64_t)0x6;
  *(uint64_t*)0x20002108 = (uint64_t)0xfffffffffffffffd;
  *(uint64_t*)0x20002110 = (uint64_t)0x0;
  *(uint64_t*)0x20002118 = (uint64_t)0x5;
  *(uint64_t*)0x20002120 = (uint64_t)0x0;
  *(uint64_t*)0x20002128 = (uint64_t)0x0;
  *(uint64_t*)0x20002130 = (uint64_t)0x0;
  syscall(__NR_ioctl, r[2], 0x4138ae84ul, 0x20002000ul);
}

int main()
{
  loop();
  return 0;
}
