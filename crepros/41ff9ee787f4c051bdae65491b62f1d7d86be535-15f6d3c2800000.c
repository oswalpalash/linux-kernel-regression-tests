// https://syzkaller.appspot.com/bug?id=41ff9ee787f4c051bdae65491b62f1d7d86be535
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <sys/syscall.h>
#include <unistd.h>

#include <stdint.h>
#include <string.h>

long r[4];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  r[1] = syscall(__NR_socket, 0xaul, 0x8000080000000001ul, 0x84ul);
  *(uint32_t*)0x20290000 = (uint32_t)0xb;
  r[3] = syscall(__NR_getsockopt, r[1], 0x84ul, 0xbul, 0x2099aff5ul,
                 0x20290000ul);
}

int main()
{
  loop();
  return 0;
}
