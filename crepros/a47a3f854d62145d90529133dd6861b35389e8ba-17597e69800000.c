// https://syzkaller.appspot.com/bug?id=a47a3f854d62145d90529133dd6861b35389e8ba
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
  syscall(__NR_mmap, 0x20000000, 0x6000, 3, 0x32, -1, 0);
  memcpy((void*)0x20000000, "/dev/ion", 9);
  r[0] = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000000, 0, 0);
  memcpy((void*)0x20001eff,
         "\x97\xcf\xfb\x5d\xa6\x71\x1f\x50\x37\xf8\xab\xdd\x3a\xde\xd8\x8d\xb0"
         "\xea\xf6\x31\x3c\xc6\xde\x0d\x5d\xde\x07\x38\xa2\x26\xc2\x3c\x62\x49"
         "\x06\xb5\xfc\x2a\x9f\x95\xac\xb7\xbd\x5e\x47\x8f\xba\x82\xa5\x9d\xf9"
         "\x57\x19\xb6\xd2\x7f\x41\xcc\x82\xd1\x19\xff\x2a\x40\x32\xbc\x91\x6d"
         "\xe1\xfa\xb6\x27\x6f\xcf\xa6\x52\xae\xed\x17\x87\xf5\xd6\xc5\xf7\x1e"
         "\x87\xfe\x65\x0b\x9f\xbe\xc3\x4e\x37\xf1\x81\x8d\x8e\xdc\x6d\x25\x1c"
         "\x52\x19\x7f\x17\x74\x00\x01\x00\x01\x40\x98\xd2\x81\x79\x83\x4e\x31"
         "\x05\xa0\xe0\xdd\xc7\x06\x5d\x3c\x14\xbf\xae\xb3\x04\x7d\x53\x43\xbf"
         "\x9e\x2f\x01\xf9\x8f\x5b\x65\x6e\x8f\xca\x7c\xe6\x36\x4c\x78\x4b\x6d"
         "\x6c\x73\xc6\xd6\xf8\x0d\xfb\xc9\x18\x0c\xe0\xed\xf6\x7e\x8e\x28\xc4"
         "\xbf\x22\x63\x4b\x8b\x29\x6c\x6a\x25\xc7\x19\x7c\xb3\xab\x32\xae\x2a"
         "\x20\x01\x53\xbf\x03\x5e\x77\xbd\xcf\x95\x00\x00\x00\x00\x00\x00\x00"
         "\x01\xbd\x2f\x0a\x44\x39\x5d\xb0\xcc\x9e\xe9\xb8\x95\x62\x13\xc7\xe7"
         "\x77\x98\xcb\xd5\xbf\x11\x1e\x0a\xe8\xd6\x20\x8a\x16\x6b\x9a\xa8\x2c"
         "\xbe\x50\x9e\x41\xf4\x70\xa4\xb7\x4a\xcf\xb7\xe9\x34\xe4\x1d\x0d\x0c"
         "\x5c\x8e",
         257);
  syscall(__NR_ioctl, r[0], 0xc0184900, 0x20001eff);
}

int main()
{
  loop();
  return 0;
}
