// https://syzkaller.appspot.com/bug?id=b58aca05eb0565e3962ecd906d03664a1edc8dd5
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <sys/syscall.h>
#include <unistd.h>

#include <stdint.h>
#include <string.h>

#ifndef __NR_mmap
#define __NR_mmap 90
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#ifndef __NR_setsockopt
#define __NR_setsockopt 366
#endif
#ifndef __NR_sendto
#define __NR_sendto 369
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[59];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xf01000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  r[1] = syscall(__NR_socket, 0xaul, 0x2ul, 0x0ul);
  *(uint64_t*)0x206c3f18 = (uint64_t)0x0;
  *(uint64_t*)0x206c3f20 = (uint64_t)0x100000000000000;
  *(uint64_t*)0x206c3f28 = (uint64_t)0x0;
  *(uint64_t*)0x206c3f30 = (uint64_t)0x100000000000000;
  *(uint16_t*)0x206c3f38 = (uint16_t)0x204e;
  *(uint16_t*)0x206c3f3a = (uint16_t)0x0;
  *(uint16_t*)0x206c3f3c = (uint16_t)0x204e;
  *(uint16_t*)0x206c3f3e = (uint16_t)0x0;
  *(uint16_t*)0x206c3f40 = (uint16_t)0xa;
  *(uint8_t*)0x206c3f42 = (uint8_t)0x0;
  *(uint8_t*)0x206c3f43 = (uint8_t)0x0;
  *(uint8_t*)0x206c3f44 = (uint8_t)0x0;
  *(uint32_t*)0x206c3f48 = (uint32_t)0x0;
  *(uint32_t*)0x206c3f4c = (uint32_t)0x0;
  *(uint64_t*)0x206c3f50 = (uint64_t)0x0;
  *(uint64_t*)0x206c3f58 = (uint64_t)0x0;
  *(uint64_t*)0x206c3f60 = (uint64_t)0x0;
  *(uint64_t*)0x206c3f68 = (uint64_t)0x0;
  *(uint64_t*)0x206c3f70 = (uint64_t)0x0;
  *(uint64_t*)0x206c3f78 = (uint64_t)0x0;
  *(uint64_t*)0x206c3f80 = (uint64_t)0x0;
  *(uint64_t*)0x206c3f88 = (uint64_t)0x0;
  *(uint64_t*)0x206c3f90 = (uint64_t)0x0;
  *(uint64_t*)0x206c3f98 = (uint64_t)0x0;
  *(uint64_t*)0x206c3fa0 = (uint64_t)0x0;
  *(uint64_t*)0x206c3fa8 = (uint64_t)0x0;
  *(uint32_t*)0x206c3fb0 = (uint32_t)0x0;
  *(uint32_t*)0x206c3fb4 = (uint32_t)0x0;
  *(uint8_t*)0x206c3fb8 = (uint8_t)0x1;
  *(uint8_t*)0x206c3fb9 = (uint8_t)0x0;
  *(uint8_t*)0x206c3fba = (uint8_t)0x0;
  *(uint8_t*)0x206c3fbb = (uint8_t)0x0;
  *(uint32_t*)0x206c3fc0 = (uint32_t)0x100007f;
  *(uint32_t*)0x206c3fd0 = (uint32_t)0x0;
  *(uint8_t*)0x206c3fd4 = (uint8_t)0x0;
  *(uint16_t*)0x206c3fd8 = (uint16_t)0x0;
  *(uint32_t*)0x206c3fdc = (uint32_t)0xffffffff;
  *(uint32_t*)0x206c3fec = (uint32_t)0x0;
  *(uint8_t*)0x206c3ff0 = (uint8_t)0x0;
  *(uint8_t*)0x206c3ff1 = (uint8_t)0x0;
  *(uint8_t*)0x206c3ff2 = (uint8_t)0x0;
  *(uint32_t*)0x206c3ff4 = (uint32_t)0x4;
  *(uint32_t*)0x206c3ff8 = (uint32_t)0x0;
  *(uint32_t*)0x206c3ffc = (uint32_t)0x0;
  r[46] = syscall(__NR_setsockopt, r[1], 0x29ul, 0x23ul, 0x206c3f18ul,
                  0xe8ul);
  *(uint16_t*)0x20efc000 = (uint16_t)0x2;
  *(uint16_t*)0x20efc002 = (uint16_t)0x224e;
  *(uint32_t*)0x20efc004 = (uint32_t)0x0;
  *(uint8_t*)0x20efc008 = (uint8_t)0x0;
  *(uint8_t*)0x20efc009 = (uint8_t)0x0;
  *(uint8_t*)0x20efc00a = (uint8_t)0x0;
  *(uint8_t*)0x20efc00b = (uint8_t)0x0;
  *(uint8_t*)0x20efc00c = (uint8_t)0x0;
  *(uint8_t*)0x20efc00d = (uint8_t)0x0;
  *(uint8_t*)0x20efc00e = (uint8_t)0x0;
  *(uint8_t*)0x20efc00f = (uint8_t)0x0;
  r[58] = syscall(__NR_sendto, r[1], 0x20efcf90ul, 0x0ul, 0x4090ul,
                  0x20efc000ul, 0x10ul);
}

int main()
{
  loop();
  return 0;
}
