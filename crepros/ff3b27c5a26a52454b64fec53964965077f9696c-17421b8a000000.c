// https://syzkaller.appspot.com/bug?id=ff3b27c5a26a52454b64fec53964965077f9696c
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

long r[6];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xf96000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  r[1] = syscall(__NR_socket, 0x10ul, 0x3ul, 0x2000000000000ul);
  *(uint64_t*)0x20b9fff0 = (uint64_t)0x20ab3fd5;
  *(uint64_t*)0x20b9fff8 = (uint64_t)0x2b;
  memcpy((void*)0x20ab3fd5,
         "\x29\x00\x00\x00\x1a\x00\x61\x00\x00\x00\x00\x00\x00\x00\x97"
         "\xe8\x02\x0c\x00\x00\x00\x00\x02\xff\x02\xf7\x00\x09\x00\x02"
         "\x00\x00\x00\xf1\xff\xff\xff\xf0\x04\x01\x10\x7c\xb0",
         43);
  r[5] = syscall(__NR_writev, r[1], 0x20b9fff0ul, 0x1ul);
}

int main()
{
  loop();
  return 0;
}
