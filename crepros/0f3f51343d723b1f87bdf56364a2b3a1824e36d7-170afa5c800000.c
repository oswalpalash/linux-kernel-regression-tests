// https://syzkaller.appspot.com/bug?id=0f3f51343d723b1f87bdf56364a2b3a1824e36d7
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <sys/syscall.h>
#include <unistd.h>

#include <stdint.h>
#include <string.h>

long r[12];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  r[1] = syscall(__NR_socket, 0x1eul, 0x5ul, 0x0ul);
  *(uint64_t*)0x203bbfc8 = (uint64_t)0x20fdbf80;
  *(uint32_t*)0x203bbfd0 = (uint32_t)0x80;
  *(uint64_t*)0x203bbfd8 = (uint64_t)0x20a72fd0;
  *(uint64_t*)0x203bbfe0 = (uint64_t)0x0;
  *(uint64_t*)0x203bbfe8 = (uint64_t)0x2012e000;
  *(uint64_t*)0x203bbff0 = (uint64_t)0x0;
  *(uint32_t*)0x203bbff8 = (uint32_t)0x0;
  *(uint16_t*)0x20fdbf80 = (uint16_t)0x10000000001e;
  memcpy((void*)0x20fdbf82,
         "\x01\x01\x01\x00\x00\x00\x0d\x00\x00\x00\x06\x00\x00\x00\x07"
         "\xe7\x7f\x5b\xf8\x6c\x48\x02\x00\x02\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x06\x00\x75\xe6\xa5\x00\x00\x00\x01\x03\x00\x00"
         "\x00\x00\xe4\xff\x06\x4b\x3f\x01\x3a\x00\x00\x00\x08\x00\x00"
         "\x00\x8f\x00\x00\x00\x00\xac\x50\x49\xfe\x32\xc4\x00\xe9\xff"
         "\xff\x7e\xff\xff\x00\x00\xff\xff\x56\xed\xb9\xa6\x34\x1c\x1f"
         "\xd4\x56\x24\x28\x1e\x00\x07\x0e\xcd\xdd\x02\x06\xc3\x97\x50"
         "\xc4\x00\x00\xfd\x00\x00\x09\x00\x00\x00\x00\x00\x0b\x00\x00"
         "\xdb\x00\x00\x04\xdb\x36",
         126);
  r[11] = syscall(__NR_sendmsg, r[1], 0x203bbfc8ul, 0x4ul);
}

int main()
{
  loop();
  return 0;
}
