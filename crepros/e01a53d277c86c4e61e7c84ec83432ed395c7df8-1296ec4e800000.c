// https://syzkaller.appspot.com/bug?id=e2dc9aea0465d1eea101bb24cb463e2a7efe7d17
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

long r[50];
void test()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0x5ab000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  r[1] = syscall(__NR_ioctl, 0xfffffffffffffffful, 0x5607ul);
  r[2] = syscall(__NR_socket, 0x26ul, 0x5ul, 0x0ul);
  *(uint16_t*)0x20216fa8 = (uint16_t)0x26;
  memcpy((void*)0x20216faa,
         "\x73\x6b\x63\x69\x70\x68\x65\x72\x00\x00\x00\x00\x00\x00",
         14);
  *(uint32_t*)0x20216fb8 = (uint32_t)0x0;
  *(uint32_t*)0x20216fbc = (uint32_t)0x0;
  memcpy((void*)0x20216fc0,
         "\x6c\x72\x77\x28\x74\x77\x6f\x66\x69\x73\x68\x29\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00",
         64);
  r[8] = syscall(__NR_bind, r[2], 0x20216fa8ul, 0x58ul);
  r[9] = syscall(__NR_accept, r[2], 0x0ul, 0x0ul);
  memcpy((void*)0x20330d27, "\xd3\xab\x33\x19\x1a\x01\x00\x23\x5f\xba"
                            "\x60\x2d\xff\x18\x00\x0b\xfe\xf9\xf3\x09"
                            "\x00\x00\x00\x00\x00\x00\x00\xff\x00\x40"
                            "\xfe\x00\x02\x25\x07\x00\x97\xc1\x1e\xd4"
                            "\xc2\xc4\xdc\x42\xff\xa8\x6e\xb9",
         48);
  r[11] = syscall(__NR_setsockopt, r[2], 0x117ul, 0x1ul, 0x20330d27ul,
                  0x30ul);
  *(uint64_t*)0x205a9fc8 = (uint64_t)0x0;
  *(uint32_t*)0x205a9fd0 = (uint32_t)0x0;
  *(uint64_t*)0x205a9fd8 = (uint64_t)0x205aa000;
  *(uint64_t*)0x205a9fe0 = (uint64_t)0x3;
  *(uint64_t*)0x205a9fe8 = (uint64_t)0x20000000;
  *(uint64_t*)0x205a9ff0 = (uint64_t)0x0;
  *(uint32_t*)0x205a9ff8 = (uint32_t)0x1000000000;
  *(uint64_t*)0x205aa000 = (uint64_t)0x201daff3;
  *(uint64_t*)0x205aa008 = (uint64_t)0xd;
  *(uint64_t*)0x205aa010 = (uint64_t)0x205a9000;
  *(uint64_t*)0x205aa018 = (uint64_t)0xe8;
  *(uint64_t*)0x205aa020 = (uint64_t)0x2027b000;
  *(uint64_t*)0x205aa028 = (uint64_t)0x0;
  memcpy((void*)0x201daff3,
         "\xb6\x47\x8e\x2b\xe2\xd8\x6a\xbc\xed\x7b\xc7\x4f\x64", 13);
  memcpy((void*)0x205a9000,
         "\x77\x4a\x6f\x55\xec\x37\x36\x46\x5c\xfd\x3c\xbe\xa3\x9e\xa3"
         "\x45\xef\x5c\x2c\x50\x6a\xe5\x33\x79\x25\x0a\x64\x5f\x47\xd2"
         "\x81\xe1\x91\xab\xa8\xc6\x48\x09\xe6\x47\xfb\xe5\x57\x6f\xc0"
         "\xbc\x61\xa7\x6f\x65\xa8\x13\x2f\xb5\x56\x12\x0d\x80\x92\x05"
         "\x69\x8b\x06\xbf\x87\x1f\xba\x65\x5a\x2b\x32\x14\x46\x97\x94"
         "\x3a\x0d\x61\x57\xc8\xab\x76\xb4\xbf\x8e\x91\xad\x8f\x68\xaf"
         "\x2e\x92\x2e\xd1\x3c\xed\x82\xf2\x8a\x56\xad\x7f\x48\x15\xf7"
         "\x7d\x6b\x70\x36\x57\x37\x47\x4b\x56\x32\x17\x57\xeb\xf2\x7c"
         "\x98\x99\xdc\x74\x7f\xe6\xb0\x1c\x24\x30\xab\x83\x20\xfa\xed"
         "\xab\x6a\x07\x29\x1f\x58\x84\x94\x5e\xd8\xc8\x8f\x86\xfb\xab"
         "\x94\xa7\x5e\x34\x22\xed\x79\x33\x8a\xd2\xe8\x66\x5c\x5a\x68"
         "\x85\xa5\x68\x2f\x60\xa5\x88\xcb\xdd\xa7\xb6\xf4\xc5\xb3\x08"
         "\xe5\xff\x2a\x64\xca\x5c\x45\x59\x88\x66\xe7\xbd\x65\x30\x7c"
         "\x47\x78\x9f\x84\x04\xbb\xd9\xdc\x8b\x71\x11\x22\xa0\x1b\xdd"
         "\x3a\xd6\x18\xb4\x56\xed\xe9\xbc\x1d\x8b\xa6\x9c\x3e\xde\x53"
         "\xe3\xc2\xd0\x07\x3d\x7a\x63",
         232);
  r[27] = syscall(__NR_sendmsg, r[9], 0x205a9fc8ul, 0x0ul);
  *(uint64_t*)0x20596000 = (uint64_t)0x2006efa0;
  *(uint32_t*)0x20596008 = (uint32_t)0x60;
  *(uint64_t*)0x20596010 = (uint64_t)0x2059f000;
  *(uint64_t*)0x20596018 = (uint64_t)0x7;
  *(uint64_t*)0x20596020 = (uint64_t)0x203f6f43;
  *(uint64_t*)0x20596028 = (uint64_t)0x0;
  *(uint32_t*)0x20596030 = (uint32_t)0x0;
  *(uint64_t*)0x2059f000 = (uint64_t)0x2059c000;
  *(uint64_t*)0x2059f008 = (uint64_t)0x0;
  *(uint64_t*)0x2059f010 = (uint64_t)0x2059d000;
  *(uint64_t*)0x2059f018 = (uint64_t)0x0;
  *(uint64_t*)0x2059f020 = (uint64_t)0x20035f51;
  *(uint64_t*)0x2059f028 = (uint64_t)0x0;
  *(uint64_t*)0x2059f030 = (uint64_t)0x2059ff73;
  *(uint64_t*)0x2059f038 = (uint64_t)0x8d;
  *(uint64_t*)0x2059f040 = (uint64_t)0x2044af44;
  *(uint64_t*)0x2059f048 = (uint64_t)0x0;
  *(uint64_t*)0x2059f050 = (uint64_t)0x20185000;
  *(uint64_t*)0x2059f058 = (uint64_t)0x0;
  *(uint64_t*)0x2059f060 = (uint64_t)0x2059ef24;
  *(uint64_t*)0x2059f068 = (uint64_t)0xdc;
  r[49] = syscall(__NR_recvmsg, r[9], 0x20596000ul, 0x0ul);
}

int main()
{
  int i;
  for (i = 0; i < 8; i++) {
    if (fork() == 0) {
      loop();
      return 0;
    }
  }
  sleep(1000000);
  return 0;
}
