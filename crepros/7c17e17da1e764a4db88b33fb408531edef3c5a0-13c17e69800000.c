// https://syzkaller.appspot.com/bug?id=7c17e17da1e764a4db88b33fb408531edef3c5a0
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
  memcpy((void*)0x20022ff8, "./file0", 8);
  r[0] = syscall(__NR_creat, 0x20022ff8, 0);
  memcpy((void*)0x20d4bfe8, "security.capability", 20);
  memcpy((void*)0x209b8000, "k", 1);
  syscall(__NR_fsetxattr, r[0], 0x20d4bfe8, 0x209b8000, 1, 0);
}

int main()
{
  loop();
  return 0;
}
