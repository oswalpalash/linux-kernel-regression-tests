// https://syzkaller.appspot.com/bug?id=26cb120b31cd24d984fc16da67f50fb375c432a7
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

uint64_t r[1] = {0xffffffffffffffff};
void loop()
{
  long res = 0;
  res = syscall(__NR_socket, 0xf, 3, 2);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20196fe4 = 0;
  *(uint32_t*)0x20196fec = 0;
  *(uint64_t*)0x20196ff4 = 0x200001c0;
  *(uint64_t*)0x200001c0 = 0x20327f68;
  memcpy((void*)0x20327f68,
         "\x02\x03\x00\x00\x11\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x05"
         "\x00\x06\x00\x00\x00\x00\x00\x0a\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x03\x00\x08\x00\xf9\xff\x00\x00\x02\x00\x00\x00"
         "\xe0\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x01\x00\x00"
         "\x00\x00\x00\x00\x00\xfb\x00\x00\x00\x00\x00\x05\x00\x05\x00\x00\x00"
         "\x00\x00\x0a\x00\x00\x00\x00\x00\x00\x00\xfe\x80\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\xff\x00\x00\x00\x00\x00\x00\x00\x00",
         136);
  *(uint64_t*)0x200001c8 = 0x88;
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
