// https://syzkaller.appspot.com/bug?id=26cb120b31cd24d984fc16da67f50fb375c432a7
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

long r[1];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_socket, 0xf, 3, 2);
  *(uint64_t*)0x20196fe4 = 0;
  *(uint32_t*)0x20196fec = 0;
  *(uint64_t*)0x20196ff4 = 0x208feff0;
  *(uint64_t*)0x208feff0 = 0x20327f68;
  *(uint8_t*)0x20327f68 = 2;
  *(uint8_t*)0x20327f69 = 3;
  *(uint8_t*)0x20327f6a = 0;
  *(uint8_t*)0x20327f6b = 2;
  *(uint16_t*)0x20327f6c = 0x11;
  *(uint16_t*)0x20327f6e = 0;
  *(uint32_t*)0x20327f70 = 0;
  *(uint32_t*)0x20327f74 = 0;
  *(uint16_t*)0x20327f78 = 5;
  *(uint16_t*)0x20327f7a = 6;
  *(uint8_t*)0x20327f7c = 0;
  *(uint8_t*)0x20327f7d = 0;
  *(uint16_t*)0x20327f7e = 0;
  *(uint16_t*)0x20327f80 = 0xa;
  *(uint16_t*)0x20327f82 = 0;
  *(uint32_t*)0x20327f84 = 0;
  *(uint8_t*)0x20327f88 = 0;
  *(uint8_t*)0x20327f89 = 0;
  *(uint8_t*)0x20327f8a = 0;
  *(uint8_t*)0x20327f8b = 0;
  *(uint8_t*)0x20327f8c = 0;
  *(uint8_t*)0x20327f8d = 0;
  *(uint8_t*)0x20327f8e = 0;
  *(uint8_t*)0x20327f8f = 0;
  *(uint8_t*)0x20327f90 = 0;
  *(uint8_t*)0x20327f91 = 0;
  *(uint8_t*)0x20327f92 = 0;
  *(uint8_t*)0x20327f93 = 0;
  *(uint8_t*)0x20327f94 = 0;
  *(uint8_t*)0x20327f95 = 0;
  *(uint8_t*)0x20327f96 = 0;
  *(uint8_t*)0x20327f97 = 0;
  *(uint32_t*)0x20327f98 = 0;
  *(uint16_t*)0x20327fa0 = 3;
  *(uint16_t*)0x20327fa2 = 8;
  *(uint8_t*)0x20327fa4 = 0;
  *(uint8_t*)0x20327fa5 = 0x80;
  *(uint16_t*)0x20327fa6 = 0;
  *(uint16_t*)0x20327fa8 = 2;
  *(uint16_t*)0x20327faa = 0;
  *(uint32_t*)0x20327fac = htobe32(0xe0000002);
  *(uint8_t*)0x20327fb0 = 0;
  *(uint8_t*)0x20327fb1 = 0;
  *(uint8_t*)0x20327fb2 = 0;
  *(uint8_t*)0x20327fb3 = 0;
  *(uint8_t*)0x20327fb4 = 0;
  *(uint8_t*)0x20327fb5 = 0;
  *(uint8_t*)0x20327fb6 = 0;
  *(uint8_t*)0x20327fb7 = 0;
  *(uint16_t*)0x20327fb8 = 2;
  *(uint16_t*)0x20327fba = 1;
  *(uint32_t*)0x20327fbc = 0;
  *(uint8_t*)0x20327fc0 = 0;
  *(uint8_t*)0x20327fc1 = 0;
  *(uint8_t*)0x20327fc2 = 0xfb;
  *(uint8_t*)0x20327fc3 = 0;
  *(uint32_t*)0x20327fc4 = 0;
  *(uint16_t*)0x20327fc8 = 5;
  *(uint16_t*)0x20327fca = 5;
  *(uint8_t*)0x20327fcc = 0;
  *(uint8_t*)0x20327fcd = 0;
  *(uint16_t*)0x20327fce = 0;
  *(uint16_t*)0x20327fd0 = 0xa;
  *(uint16_t*)0x20327fd2 = 0;
  *(uint32_t*)0x20327fd4 = 0;
  *(uint8_t*)0x20327fd8 = 0xfe;
  *(uint8_t*)0x20327fd9 = 0x80;
  *(uint8_t*)0x20327fda = 0;
  *(uint8_t*)0x20327fdb = 0;
  *(uint8_t*)0x20327fdc = 0;
  *(uint8_t*)0x20327fdd = 0;
  *(uint8_t*)0x20327fde = 0;
  *(uint8_t*)0x20327fdf = 0;
  *(uint8_t*)0x20327fe0 = 0;
  *(uint8_t*)0x20327fe1 = 0;
  *(uint8_t*)0x20327fe2 = 0;
  *(uint8_t*)0x20327fe3 = 0;
  *(uint8_t*)0x20327fe4 = 0;
  *(uint8_t*)0x20327fe5 = 0;
  *(uint8_t*)0x20327fe6 = 0;
  *(uint8_t*)0x20327fe7 = 0xbb;
  *(uint32_t*)0x20327fe8 = 0;
  *(uint64_t*)0x208feff8 = 0x88;
  *(uint64_t*)0x20196ffc = 1;
  *(uint64_t*)0x20197004 = 0;
  *(uint64_t*)0x2019700c = 0;
  *(uint32_t*)0x20197014 = 0;
  syscall(__NR_sendmsg, r[0], 0x20196fe4, 0);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
