// https://syzkaller.appspot.com/bug?id=3d555b5a8aee84e9418c5908875bdf37fc5bb667
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

long r[40];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  r[2] = syscall(__NR_socket, 0xaul, 0x802ul, 0x0ul);
  memcpy((void*)0x20f6e000, "\x00\x00\x00\x00\x00\x00\x0f\xa6", 8);
  r[4] = syscall(__NR_setsockopt, r[2], 0x29ul, 0x3bul, 0x20f6e000ul,
                 0x8ul);
  *(uint64_t*)0x20f5ffe0 = (uint64_t)0x0;
  *(uint64_t*)0x20f5ffe8 = (uint64_t)0x100000000000000;
  *(uint32_t*)0x20f5fff0 = (uint32_t)0x800;
  *(uint8_t*)0x20f5fff4 = (uint8_t)0x0;
  *(uint8_t*)0x20f5fff5 = (uint8_t)0xff;
  *(uint16_t*)0x20f5fff6 = (uint16_t)0x1;
  *(uint16_t*)0x20f5fff8 = (uint16_t)0x0;
  *(uint16_t*)0x20f5fffa = (uint16_t)0x0;
  *(uint32_t*)0x20f5fffc = (uint32_t)0x0;
  r[14] = syscall(__NR_setsockopt, r[2], 0x29ul, 0x20ul, 0x20f5ffe0ul,
                  0x20ul);
  *(uint32_t*)0x2089b000 = (uint32_t)0xffffffffffffffff;
  r[16] = syscall(__NR_setsockopt, r[2], 0x29ul, 0x21ul, 0x2089b000ul,
                  0x4ul);
  memcpy((void*)0x20e72000, "\x65\x01\xd9\x03\x42\x7b\xa1\x04\x7c\x6d"
                            "\xb3\x76\xef\xaa\x5d\x50",
         16);
  r[18] = syscall(__NR_setsockopt, r[2], 0x29ul, 0x37ul, 0x20e72000ul,
                  0x10ul);
  *(uint16_t*)0x20e0afe4 = (uint16_t)0xa;
  *(uint16_t*)0x20e0afe6 = (uint16_t)0x214e;
  *(uint32_t*)0x20e0afe8 = (uint32_t)0x800;
  *(uint8_t*)0x20e0afec = (uint8_t)0xfe;
  *(uint8_t*)0x20e0afed = (uint8_t)0x80;
  *(uint8_t*)0x20e0afee = (uint8_t)0x0;
  *(uint8_t*)0x20e0afef = (uint8_t)0x0;
  *(uint8_t*)0x20e0aff0 = (uint8_t)0x0;
  *(uint8_t*)0x20e0aff1 = (uint8_t)0x0;
  *(uint8_t*)0x20e0aff2 = (uint8_t)0x0;
  *(uint8_t*)0x20e0aff3 = (uint8_t)0x0;
  *(uint8_t*)0x20e0aff4 = (uint8_t)0x0;
  *(uint8_t*)0x20e0aff5 = (uint8_t)0x0;
  *(uint8_t*)0x20e0aff6 = (uint8_t)0x0;
  *(uint8_t*)0x20e0aff7 = (uint8_t)0x0;
  *(uint8_t*)0x20e0aff8 = (uint8_t)0x0;
  *(uint8_t*)0x20e0aff9 = (uint8_t)0x0;
  *(uint8_t*)0x20e0affa = (uint8_t)0x0;
  *(uint8_t*)0x20e0affb = (uint8_t)0xaa;
  *(uint32_t*)0x20e0affc = (uint32_t)0x0;
  r[39] = syscall(__NR_sendto, r[2], 0x2052bfccul, 0x0ul, 0x0ul,
                  0x20e0afe4ul, 0x1cul);
}

int main()
{
  loop();
  return 0;
}
