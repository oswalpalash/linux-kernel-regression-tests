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
  memcpy((void*)0x2000fff7, "/dev/kvm", 9);
  r[0] = syscall(__NR_openat, 0xffffffffffffff9cul, 0x2000fff7ul, 0x0ul,
                 0x0ul);
  r[1] = syscall(__NR_ioctl, r[0], 0xae01ul, 0x0ul);
  r[2] = syscall(__NR_ioctl, r[1], 0xae41ul, 0x0ul);
  *(uint64_t*)0x20011000 = (uint64_t)0x0;
  *(uint32_t*)0x20011008 = (uint32_t)0x0;
  *(uint16_t*)0x2001100c = (uint16_t)0x0;
  *(uint8_t*)0x2001100e = (uint8_t)0x0;
  *(uint8_t*)0x2001100f = (uint8_t)0x0;
  *(uint8_t*)0x20011010 = (uint8_t)0x0;
  *(uint8_t*)0x20011011 = (uint8_t)0x0;
  *(uint8_t*)0x20011012 = (uint8_t)0x0;
  *(uint8_t*)0x20011013 = (uint8_t)0x0;
  *(uint8_t*)0x20011014 = (uint8_t)0x0;
  *(uint8_t*)0x20011015 = (uint8_t)0x0;
  *(uint8_t*)0x20011016 = (uint8_t)0x0;
  *(uint8_t*)0x20011017 = (uint8_t)0x0;
  *(uint64_t*)0x20011018 = (uint64_t)0x0;
  *(uint32_t*)0x20011020 = (uint32_t)0x0;
  *(uint16_t*)0x20011024 = (uint16_t)0x0;
  *(uint8_t*)0x20011026 = (uint8_t)0x0;
  *(uint8_t*)0x20011027 = (uint8_t)0x0;
  *(uint8_t*)0x20011028 = (uint8_t)0x0;
  *(uint8_t*)0x20011029 = (uint8_t)0x0;
  *(uint8_t*)0x2001102a = (uint8_t)0x0;
  *(uint8_t*)0x2001102b = (uint8_t)0x0;
  *(uint8_t*)0x2001102c = (uint8_t)0x0;
  *(uint8_t*)0x2001102d = (uint8_t)0x0;
  *(uint8_t*)0x2001102e = (uint8_t)0x0;
  *(uint8_t*)0x2001102f = (uint8_t)0x0;
  *(uint64_t*)0x20011030 = (uint64_t)0x0;
  *(uint32_t*)0x20011038 = (uint32_t)0x0;
  *(uint16_t*)0x2001103c = (uint16_t)0x0;
  *(uint8_t*)0x2001103e = (uint8_t)0x0;
  *(uint8_t*)0x2001103f = (uint8_t)0x0;
  *(uint8_t*)0x20011040 = (uint8_t)0x0;
  *(uint8_t*)0x20011041 = (uint8_t)0x0;
  *(uint8_t*)0x20011042 = (uint8_t)0x0;
  *(uint8_t*)0x20011043 = (uint8_t)0x0;
  *(uint8_t*)0x20011044 = (uint8_t)0x0;
  *(uint8_t*)0x20011045 = (uint8_t)0x0;
  *(uint8_t*)0x20011046 = (uint8_t)0x0;
  *(uint8_t*)0x20011047 = (uint8_t)0x0;
  *(uint64_t*)0x20011048 = (uint64_t)0x0;
  *(uint32_t*)0x20011050 = (uint32_t)0x0;
  *(uint16_t*)0x20011054 = (uint16_t)0x0;
  *(uint8_t*)0x20011056 = (uint8_t)0x0;
  *(uint8_t*)0x20011057 = (uint8_t)0x0;
  *(uint8_t*)0x20011058 = (uint8_t)0x0;
  *(uint8_t*)0x20011059 = (uint8_t)0x0;
  *(uint8_t*)0x2001105a = (uint8_t)0x0;
  *(uint8_t*)0x2001105b = (uint8_t)0x0;
  *(uint8_t*)0x2001105c = (uint8_t)0x0;
  *(uint8_t*)0x2001105d = (uint8_t)0x0;
  *(uint8_t*)0x2001105e = (uint8_t)0x0;
  *(uint8_t*)0x2001105f = (uint8_t)0x0;
  *(uint64_t*)0x20011060 = (uint64_t)0x0;
  *(uint32_t*)0x20011068 = (uint32_t)0x0;
  *(uint16_t*)0x2001106c = (uint16_t)0x0;
  *(uint8_t*)0x2001106e = (uint8_t)0x0;
  *(uint8_t*)0x2001106f = (uint8_t)0x0;
  *(uint8_t*)0x20011070 = (uint8_t)0x0;
  *(uint8_t*)0x20011071 = (uint8_t)0x0;
  *(uint8_t*)0x20011072 = (uint8_t)0x0;
  *(uint8_t*)0x20011073 = (uint8_t)0x0;
  *(uint8_t*)0x20011074 = (uint8_t)0x0;
  *(uint8_t*)0x20011075 = (uint8_t)0x0;
  *(uint8_t*)0x20011076 = (uint8_t)0x0;
  *(uint8_t*)0x20011077 = (uint8_t)0x0;
  *(uint64_t*)0x20011078 = (uint64_t)0x0;
  *(uint32_t*)0x20011080 = (uint32_t)0x0;
  *(uint16_t*)0x20011084 = (uint16_t)0xf;
  *(uint8_t*)0x20011086 = (uint8_t)0x0;
  *(uint8_t*)0x20011087 = (uint8_t)0x0;
  *(uint8_t*)0x20011088 = (uint8_t)0x0;
  *(uint8_t*)0x20011089 = (uint8_t)0x0;
  *(uint8_t*)0x2001108a = (uint8_t)0x0;
  *(uint8_t*)0x2001108b = (uint8_t)0x0;
  *(uint8_t*)0x2001108c = (uint8_t)0x0;
  *(uint8_t*)0x2001108d = (uint8_t)0x0;
  *(uint8_t*)0x2001108e = (uint8_t)0x0;
  *(uint8_t*)0x2001108f = (uint8_t)0x0;
  *(uint64_t*)0x20011090 = (uint64_t)0x0;
  *(uint32_t*)0x20011098 = (uint32_t)0x0;
  *(uint16_t*)0x2001109c = (uint16_t)0x0;
  *(uint8_t*)0x2001109e = (uint8_t)0x0;
  *(uint8_t*)0x2001109f = (uint8_t)0x0;
  *(uint8_t*)0x200110a0 = (uint8_t)0x0;
  *(uint8_t*)0x200110a1 = (uint8_t)0x0;
  *(uint8_t*)0x200110a2 = (uint8_t)0x0;
  *(uint8_t*)0x200110a3 = (uint8_t)0x0;
  *(uint8_t*)0x200110a4 = (uint8_t)0x0;
  *(uint8_t*)0x200110a5 = (uint8_t)0x0;
  *(uint8_t*)0x200110a6 = (uint8_t)0x0;
  *(uint8_t*)0x200110a7 = (uint8_t)0x0;
  *(uint64_t*)0x200110a8 = (uint64_t)0x0;
  *(uint32_t*)0x200110b0 = (uint32_t)0x0;
  *(uint16_t*)0x200110b4 = (uint16_t)0x0;
  *(uint8_t*)0x200110b6 = (uint8_t)0x0;
  *(uint8_t*)0x200110b7 = (uint8_t)0x0;
  *(uint8_t*)0x200110b8 = (uint8_t)0x0;
  *(uint8_t*)0x200110b9 = (uint8_t)0x0;
  *(uint8_t*)0x200110ba = (uint8_t)0x0;
  *(uint8_t*)0x200110bb = (uint8_t)0x0;
  *(uint8_t*)0x200110bc = (uint8_t)0x0;
  *(uint8_t*)0x200110bd = (uint8_t)0x0;
  *(uint8_t*)0x200110be = (uint8_t)0xf46;
  *(uint8_t*)0x200110bf = (uint8_t)0x0;
  *(uint64_t*)0x200110c0 = (uint64_t)0x0;
  *(uint16_t*)0x200110c8 = (uint16_t)0x0;
  *(uint16_t*)0x200110ca = (uint16_t)0x0;
  *(uint16_t*)0x200110cc = (uint16_t)0x0;
  *(uint16_t*)0x200110ce = (uint16_t)0x0;
  *(uint64_t*)0x200110d0 = (uint64_t)0x0;
  *(uint16_t*)0x200110d8 = (uint16_t)0x0;
  *(uint16_t*)0x200110da = (uint16_t)0x0;
  *(uint16_t*)0x200110dc = (uint16_t)0x0;
  *(uint16_t*)0x200110de = (uint16_t)0x0;
  *(uint64_t*)0x200110e0 = (uint64_t)0x80000001;
  *(uint64_t*)0x200110e8 = (uint64_t)0x0;
  *(uint64_t*)0x200110f0 = (uint64_t)0x0;
  *(uint64_t*)0x200110f8 = (uint64_t)0x200000;
  *(uint64_t*)0x20011100 = (uint64_t)0x0;
  *(uint64_t*)0x20011108 = (uint64_t)0x4101;
  *(uint64_t*)0x20011110 = (uint64_t)0x0;
  *(uint64_t*)0x20011118 = (uint64_t)0x0;
  *(uint64_t*)0x20011120 = (uint64_t)0x0;
  *(uint64_t*)0x20011128 = (uint64_t)0x0;
  *(uint64_t*)0x20011130 = (uint64_t)0x9a;
  syscall(__NR_ioctl, r[2], 0x4138ae84ul, 0x20011000ul);
}

int main()
{
  loop();
  return 0;
}
