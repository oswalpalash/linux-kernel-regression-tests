// https://syzkaller.appspot.com/bug?id=ff3b27c5a26a52454b64fec53964965077f9696c
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

long r[4];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xea0000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  r[1] = syscall(__NR_socket, 0x10ul, 0x802ul, 0x0ul);
  memcpy((void*)0x20006fdb, "\x24\x00\x00\x00\x1a\x00\x25\xf0\x00\x18"
                            "\x00\x04\x00\xed\xfc\x0e\x02\x0b\x00\x00"
                            "\x00\x10\xb5\x02\x02\xff\x0a\xeb\x08\x00"
                            "\x00\x01\x48\x18\x6b\xe0\x29",
         37);
  r[3] = syscall(__NR_write, r[1], 0x20006fdbul, 0x25ul);
}

int main()
{
  loop();
  return 0;
}
