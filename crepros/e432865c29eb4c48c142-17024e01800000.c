// https://syzkaller.appspot.com/bug?id=a3cc92fe3fe3bf47b6ab9f5c840d43a2aa24ccc9
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

#ifndef __NR_write
#define __NR_write 4
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[2];
void test()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
          0xfffffffffffffffful, 0x0ul);
  r[0] = syscall(__NR_socket, 0x10ul, 0x2ul, 0x0ul);
  memcpy((void*)0x20151000, "\x1f\x00\x00\x00\x12\x00\x03\xf2\x05\x02"
                            "\x00\x03\x00\x0e\xff\x8d\x78\x21\x00\x41"
                            "\x08\x00\x2e\x00\x00\x00\x00\x00\xb4\x2a"
                            "\x81",
         31);
  syscall(__NR_write, r[0], 0x20151000ul, 0x1ful);
  r[1] = syscall(__NR_socket, 0x10ul, 0x3ul, 0x0ul);
  memcpy((void*)0x20001fdb, "\x24\x00\x00\x00\x58\x00\x1f\x00\x14\xf9"
                            "\xf4\x07\x00\x23\x04\x00\x0a\xff\xf5\x11"
                            "\x08\x00\x01\x00\xfe\xbf\xff\xff\x08\x00"
                            "\x02\x00\x01\x00\x00\x00",
         36);
  syscall(__NR_write, r[1], 0x20001fdbul, 0x24ul);
}

int main()
{
  loop();
  return 0;
}
