// https://syzkaller.appspot.com/bug?id=76e1ca0ed8893fcd1c960eb2d322809b83c90ac7
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_openat
#define __NR_openat 295
#endif
#ifndef __NR_ioctl
#define __NR_ioctl 54
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[1];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xd000, 3, 0x32, -1, 0);
  memcpy((void*)0x20005ff7, "/dev/ion", 9);
  r[0] = syscall(__NR_openat, 0xffffff9c, 0x20005ff7, 0, 0);
  memcpy((void*)0x20003000,
         "\xf4\x7f\x76\x54\x91\xab\xfc\xa9\x51\x1d\xc7\x6c\x2e\x0c\xb4\x81\x72"
         "\x3c\x7b\xb5\x3b\x7b\x98\xd7\x53\x51\x86\x80\x18\x9e\x75\x2c\xdb\x84"
         "\x36\x3f\x60\xd7\x74\xeb\x9e\x9b\x8f\xde\x0c\x93\xe5\x3c\x8e\x0f\x94"
         "\x1b\x6f\xf5\x66\x8c\xa8\x15\xe3\x17\xc5\x86\x10\xc0\x8b\xe8\xc3\xb8"
         "\xb9\xb3\x71\xfe\x63\xf7\x6a\x74\xf0\xd8\x11\xcb\xb0\x1c\x29\x9f\x3b"
         "\xfd\xad\x04\x2e\xa0\x0d\x4f\x5d\x5f\xd8\x18\xe2\x6b\x68\x7c\x04\x5c"
         "\x15\x60\xa7\x26\xe7\x78\x39\x91\xd2\x75\xfe\x24\x46\x1b\xb0\x7d\xa4"
         "\xf1\xf6\xc8\xe4\xbf\x06\xaf\x80\xa3\x5a\xd3\x42\xbc\x7b\xec\xf5\x96"
         "\xc7\x60\x16\x61\x31\x39\x40\x5b\x2a\xb9\xd2\x21\xab\xbc\xc3\x7a\x1f"
         "\x9e\x2e\xbf\x1f\xad\x54\x0f\x7a\x1f\x94\xc5\x25\xc8\xa3\x3f\x11",
         169);
  syscall(__NR_ioctl, r[0], 0xc0184908, 0x20003000);
}

int main()
{
  loop();
  return 0;
}
