// https://syzkaller.appspot.com/bug?id=2997ce21361877acf4ac2cb73a277f992f0c8264
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

#define _GNU_SOURCE

#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

long r[24];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xef4000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  r[1] = syscall(__NR_socket, 0x2ul, 0x1ul, 0x0ul);
  *(uint32_t*)0x20b4cfd0 = (uint32_t)0x1;
  *(uint32_t*)0x20b4cfd4 = (uint32_t)0x2;
  *(uint64_t*)0x20b4cfd8 = (uint64_t)0x20ef3000;
  *(uint64_t*)0x20b4cfe0 = (uint64_t)0x20b4d000;
  *(uint32_t*)0x20b4cfe8 = (uint32_t)0x1;
  *(uint32_t*)0x20b4cfec = (uint32_t)0x80;
  *(uint64_t*)0x20b4cff0 = (uint64_t)0x2000a000;
  *(uint32_t*)0x20b4cff8 = (uint32_t)0x0;
  *(uint8_t*)0x20ef3000 = (uint8_t)0x8db7;
  *(uint8_t*)0x20ef3001 = (uint8_t)0x0;
  *(uint16_t*)0x20ef3002 = (uint16_t)0x0;
  *(uint32_t*)0x20ef3004 = (uint32_t)0x0;
  *(uint8_t*)0x20ef3008 = (uint8_t)0xd395;
  *(uint8_t*)0x20ef3009 = (uint8_t)0x0;
  *(uint16_t*)0x20ef300a = (uint16_t)0x0;
  *(uint32_t*)0x20ef300c = (uint32_t)0x0;
  memcpy((void*)0x20b4d000, "\x73\x79\x73\x65\x4f\x00", 6);
  r[19] = syscall(__NR_bpf, 0x5ul, 0x20b4cfd0ul, 0x30ul);
  r[20] = syscall(__NR_socket, 0x29ul, 0x5ul, 0x0ul);
  *(uint32_t*)0x209b9000 = r[1];
  *(uint32_t*)0x209b9004 = r[19];
  r[23] = syscall(__NR_ioctl, r[20], 0x89e0ul, 0x209b9000ul);
}

int main()
{
  loop();
  return 0;
}
