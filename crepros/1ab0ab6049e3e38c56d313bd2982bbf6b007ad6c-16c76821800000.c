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
  memcpy((void*)0x2028bff7, "/dev/kvm", 9);
  r[0] = syscall(__NR_openat, 0xffffffffffffff9cul, 0x2028bff7ul, 0x0ul,
                 0x0ul);
  r[1] = syscall(__NR_ioctl, r[0], 0xae01ul, 0x0ul);
  *(uint16_t*)0x20e58fe0 = (uint16_t)0x10000;
  *(uint16_t*)0x20e58fe2 = (uint16_t)0xffffffffffffffc2;
  *(uint16_t*)0x20e58fe4 = (uint16_t)0x8000;
  *(uint32_t*)0x20e58fe8 = (uint32_t)0x80000001;
  *(uint32_t*)0x20e58fec = (uint32_t)0xc245;
  *(uint32_t*)0x20e58ff0 = (uint32_t)0x80;
  *(uint32_t*)0x20e58ff4 = (uint32_t)0xf8c;
  *(uint32_t*)0x20e58ff8 = (uint32_t)0x4596;
  *(uint32_t*)0x20e58ffc = (uint32_t)0x0;
  syscall(__NR_setsockopt, 0xfffffffffffffffful, 0x84ul, 0xaul,
          0x20e58fe0ul, 0x20ul);
  r[2] = syscall(__NR_ioctl, r[1], 0xae41ul, 0x201ul);
  *(uint64_t*)0x2042aec8 = (uint64_t)0x0;
  *(uint32_t*)0x2042aed0 = (uint32_t)0x0;
  *(uint16_t*)0x2042aed4 = (uint16_t)0x0;
  *(uint8_t*)0x2042aed6 = (uint8_t)0x5d3b;
  *(uint8_t*)0x2042aed7 = (uint8_t)0x4;
  *(uint8_t*)0x2042aed8 = (uint8_t)0x7;
  *(uint8_t*)0x2042aed9 = (uint8_t)0x0;
  *(uint8_t*)0x2042aeda = (uint8_t)0x40000001;
  *(uint8_t*)0x2042aedb = (uint8_t)0x1;
  *(uint8_t*)0x2042aedc = (uint8_t)0x0;
  *(uint8_t*)0x2042aedd = (uint8_t)0x0;
  *(uint8_t*)0x2042aede = (uint8_t)0x0;
  *(uint8_t*)0x2042aedf = (uint8_t)0x0;
  *(uint64_t*)0x2042aee0 = (uint64_t)0x0;
  *(uint32_t*)0x2042aee8 = (uint32_t)0x0;
  *(uint16_t*)0x2042aeec = (uint16_t)0x0;
  *(uint8_t*)0x2042aeee = (uint8_t)0x0;
  *(uint8_t*)0x2042aeef = (uint8_t)0x0;
  *(uint8_t*)0x2042aef0 = (uint8_t)0x0;
  *(uint8_t*)0x2042aef1 = (uint8_t)0x0;
  *(uint8_t*)0x2042aef2 = (uint8_t)0x0;
  *(uint8_t*)0x2042aef3 = (uint8_t)0x0;
  *(uint8_t*)0x2042aef4 = (uint8_t)0x0;
  *(uint8_t*)0x2042aef5 = (uint8_t)0x0;
  *(uint8_t*)0x2042aef6 = (uint8_t)0x0;
  *(uint8_t*)0x2042aef7 = (uint8_t)0x0;
  *(uint64_t*)0x2042aef8 = (uint64_t)0x0;
  *(uint32_t*)0x2042af00 = (uint32_t)0x0;
  *(uint16_t*)0x2042af04 = (uint16_t)0x0;
  *(uint8_t*)0x2042af06 = (uint8_t)0x0;
  *(uint8_t*)0x2042af07 = (uint8_t)0x0;
  *(uint8_t*)0x2042af08 = (uint8_t)0x600000000;
  *(uint8_t*)0x2042af09 = (uint8_t)0x0;
  *(uint8_t*)0x2042af0a = (uint8_t)0x0;
  *(uint8_t*)0x2042af0b = (uint8_t)0x0;
  *(uint8_t*)0x2042af0c = (uint8_t)0x0;
  *(uint8_t*)0x2042af0d = (uint8_t)0x0;
  *(uint8_t*)0x2042af0e = (uint8_t)0x0;
  *(uint8_t*)0x2042af0f = (uint8_t)0x0;
  *(uint64_t*)0x2042af10 = (uint64_t)0x0;
  *(uint32_t*)0x2042af18 = (uint32_t)0xfffffffffffffffc;
  *(uint16_t*)0x2042af1c = (uint16_t)0x0;
  *(uint8_t*)0x2042af1e = (uint8_t)0x0;
  *(uint8_t*)0x2042af1f = (uint8_t)0x0;
  *(uint8_t*)0x2042af20 = (uint8_t)0x0;
  *(uint8_t*)0x2042af21 = (uint8_t)0x0;
  *(uint8_t*)0x2042af22 = (uint8_t)0x0;
  *(uint8_t*)0x2042af23 = (uint8_t)0x0;
  *(uint8_t*)0x2042af24 = (uint8_t)0x0;
  *(uint8_t*)0x2042af25 = (uint8_t)0x0;
  *(uint8_t*)0x2042af26 = (uint8_t)0x400000000000000;
  *(uint8_t*)0x2042af27 = (uint8_t)0x0;
  *(uint64_t*)0x2042af28 = (uint64_t)0x0;
  *(uint32_t*)0x2042af30 = (uint32_t)0x0;
  *(uint16_t*)0x2042af34 = (uint16_t)0x0;
  *(uint8_t*)0x2042af36 = (uint8_t)0x0;
  *(uint8_t*)0x2042af37 = (uint8_t)0x0;
  *(uint8_t*)0x2042af38 = (uint8_t)0x0;
  *(uint8_t*)0x2042af39 = (uint8_t)0x0;
  *(uint8_t*)0x2042af3a = (uint8_t)0x0;
  *(uint8_t*)0x2042af3b = (uint8_t)0x0;
  *(uint8_t*)0x2042af3c = (uint8_t)0x0;
  *(uint8_t*)0x2042af3d = (uint8_t)0x0;
  *(uint8_t*)0x2042af3e = (uint8_t)0x0;
  *(uint8_t*)0x2042af3f = (uint8_t)0x0;
  *(uint64_t*)0x2042af40 = (uint64_t)0x0;
  *(uint32_t*)0x2042af48 = (uint32_t)0x0;
  *(uint16_t*)0x2042af4c = (uint16_t)0x0;
  *(uint8_t*)0x2042af4e = (uint8_t)0x0;
  *(uint8_t*)0x2042af4f = (uint8_t)0x0;
  *(uint8_t*)0x2042af50 = (uint8_t)0x0;
  *(uint8_t*)0x2042af51 = (uint8_t)0x0;
  *(uint8_t*)0x2042af52 = (uint8_t)0x0;
  *(uint8_t*)0x2042af53 = (uint8_t)0x0;
  *(uint8_t*)0x2042af54 = (uint8_t)0x0;
  *(uint8_t*)0x2042af55 = (uint8_t)0x0;
  *(uint8_t*)0x2042af56 = (uint8_t)0x0;
  *(uint8_t*)0x2042af57 = (uint8_t)0x0;
  *(uint64_t*)0x2042af58 = (uint64_t)0x0;
  *(uint32_t*)0x2042af60 = (uint32_t)0x0;
  *(uint16_t*)0x2042af64 = (uint16_t)0x0;
  *(uint8_t*)0x2042af66 = (uint8_t)0x0;
  *(uint8_t*)0x2042af67 = (uint8_t)0x0;
  *(uint8_t*)0x2042af68 = (uint8_t)0x0;
  *(uint8_t*)0x2042af69 = (uint8_t)0x0;
  *(uint8_t*)0x2042af6a = (uint8_t)0x0;
  *(uint8_t*)0x2042af6b = (uint8_t)0x0;
  *(uint8_t*)0x2042af6c = (uint8_t)0x0;
  *(uint8_t*)0x2042af6d = (uint8_t)0x0;
  *(uint8_t*)0x2042af6e = (uint8_t)0x0;
  *(uint8_t*)0x2042af6f = (uint8_t)0x0;
  *(uint64_t*)0x2042af70 = (uint64_t)0x0;
  *(uint32_t*)0x2042af78 = (uint32_t)0x0;
  *(uint16_t*)0x2042af7c = (uint16_t)0x0;
  *(uint8_t*)0x2042af7e = (uint8_t)0x0;
  *(uint8_t*)0x2042af7f = (uint8_t)0x0;
  *(uint8_t*)0x2042af80 = (uint8_t)0x0;
  *(uint8_t*)0x2042af81 = (uint8_t)0x0;
  *(uint8_t*)0x2042af82 = (uint8_t)0x0;
  *(uint8_t*)0x2042af83 = (uint8_t)0x0;
  *(uint8_t*)0x2042af84 = (uint8_t)0x0;
  *(uint8_t*)0x2042af85 = (uint8_t)0x0;
  *(uint8_t*)0x2042af86 = (uint8_t)0x0;
  *(uint8_t*)0x2042af87 = (uint8_t)0x0;
  *(uint64_t*)0x2042af88 = (uint64_t)0x0;
  *(uint16_t*)0x2042af90 = (uint16_t)0x0;
  *(uint16_t*)0x2042af92 = (uint16_t)0x0;
  *(uint16_t*)0x2042af94 = (uint16_t)0x0;
  *(uint16_t*)0x2042af96 = (uint16_t)0x0;
  *(uint64_t*)0x2042af98 = (uint64_t)0x0;
  *(uint16_t*)0x2042afa0 = (uint16_t)0x0;
  *(uint16_t*)0x2042afa2 = (uint16_t)0x0;
  *(uint16_t*)0x2042afa4 = (uint16_t)0x0;
  *(uint16_t*)0x2042afa6 = (uint16_t)0x0;
  *(uint64_t*)0x2042afa8 = (uint64_t)0x1;
  *(uint64_t*)0x2042afb0 = (uint64_t)0x0;
  *(uint64_t*)0x2042afb8 = (uint64_t)0x0;
  *(uint64_t*)0x2042afc0 = (uint64_t)0x0;
  *(uint64_t*)0x2042afc8 = (uint64_t)0x0;
  *(uint64_t*)0x2042afd0 = (uint64_t)0x0;
  *(uint64_t*)0x2042afd8 = (uint64_t)0x12800;
  *(uint64_t*)0x2042afe0 = (uint64_t)0x0;
  *(uint64_t*)0x2042afe8 = (uint64_t)0x0;
  *(uint64_t*)0x2042aff0 = (uint64_t)0x0;
  *(uint64_t*)0x2042aff8 = (uint64_t)0x7;
  syscall(__NR_ioctl, r[2], 0x4138ae84ul, 0x2042aec8ul);
}

int main()
{
  loop();
  return 0;
}
