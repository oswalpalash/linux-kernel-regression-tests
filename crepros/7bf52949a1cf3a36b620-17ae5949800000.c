// https://syzkaller.appspot.com/bug?id=7c17e17da1e764a4db88b33fb408531edef3c5a0
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifndef __NR_open
#define __NR_open 5
#endif
#ifndef __NR_fsetxattr
#define __NR_fsetxattr 228
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_mkdir
#define __NR_mkdir 39
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[1];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  memcpy((void*)0x2000dff6, "./control", 10);
  syscall(__NR_mkdir, 0x2000dff6, 0);
  memcpy((void*)0x20741000, "./control", 10);
  r[0] = syscall(__NR_open, 0x20741000, 0, 0);
  memcpy((void*)0x20f4c000, "security.capability", 20);
  memcpy((void*)0x208c4fe9, "", 1);
  syscall(__NR_fsetxattr, r[0], 0x20f4c000, 0x208c4fe9, 1, 1);
}

int main()
{
  loop();
  return 0;
}
