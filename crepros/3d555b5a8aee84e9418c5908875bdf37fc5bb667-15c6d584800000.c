// https://syzkaller.appspot.com/bug?id=3d555b5a8aee84e9418c5908875bdf37fc5bb667
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <sys/syscall.h>
#include <unistd.h>

#include <stdint.h>
#include <string.h>

static void test();

void loop()
{
  while (1) {
    test();
  }
}

long r[54];
void test()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  r[1] = syscall(__NR_socket, 0xaul, 0x802ul, 0x0ul);
  memcpy((void*)0x20f6e000, "\x00\x00\x00\x00\x00\x00\x0f\xa6", 8);
  r[3] = syscall(__NR_setsockopt, r[1], 0x29ul, 0x3bul, 0x20f6e000ul,
                 0x8ul);
  *(uint64_t*)0x2088c000 = (uint64_t)0x0;
  *(uint64_t*)0x2088c008 = (uint64_t)0x100000000000000;
  *(uint32_t*)0x2088c010 = (uint32_t)0x800;
  *(uint8_t*)0x2088c014 = (uint8_t)0x0;
  *(uint8_t*)0x2088c015 = (uint8_t)0x2000ff;
  *(uint16_t*)0x2088c016 = (uint16_t)0x1;
  *(uint16_t*)0x2088c018 = (uint16_t)0x0;
  *(uint16_t*)0x2088c01a = (uint16_t)0x0;
  *(uint32_t*)0x2088c01c = (uint32_t)0x0;
  r[13] = syscall(__NR_setsockopt, r[1], 0x29ul, 0x20ul, 0x2088c000ul,
                  0x20ul);
  *(uint32_t*)0x204c3ffc = (uint32_t)0x9;
  r[15] = syscall(__NR_setsockopt, r[1], 0x29ul, 0x21ul, 0x204c3ffcul,
                  0x4ul);
  memcpy((void*)0x20e72000, "\x65\x01\xd9\x03\x42\x7b\xa1\x04\x7c\x6d"
                            "\xb3\x76\xef\xaa\x5d\x50",
         16);
  r[17] = syscall(__NR_setsockopt, r[1], 0x29ul, 0x37ul, 0x20e72000ul,
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
  r[38] = syscall(__NR_sendto, r[1], 0x20fd1000ul, 0x0ul, 0x0ul,
                  0x20e0afe4ul, 0x1cul);
  r[39] = syscall(__NR_socket, 0x26ul, 0x5ul, 0x0ul);
  *(uint8_t*)0x20da7000 = (uint8_t)0x73;
  *(uint8_t*)0x20da7001 = (uint8_t)0x79;
  *(uint8_t*)0x20da7002 = (uint8_t)0x7a;
  *(uint8_t*)0x20da7003 = (uint8_t)0x30;
  *(uint8_t*)0x20da7004 = (uint8_t)0x0;
  r[45] = syscall(__NR_ioctl, r[39], 0x89a0ul, 0x20da7000ul);
  r[46] = syscall(__NR_socket, 0x11ul, 0xaul, 0x400000000000004ul);
  *(uint8_t*)0x209a0000 = (uint8_t)0x73;
  *(uint8_t*)0x209a0001 = (uint8_t)0x79;
  *(uint8_t*)0x209a0002 = (uint8_t)0x7a;
  *(uint8_t*)0x209a0003 = (uint8_t)0x30;
  *(uint8_t*)0x209a0004 = (uint8_t)0x0;
  *(uint16_t*)0x209a0010 = (uint16_t)0x4001;
  r[53] = syscall(__NR_ioctl, r[46], 0x8914ul, 0x209a0000ul);
}

int main()
{
  loop();
  return 0;
}
