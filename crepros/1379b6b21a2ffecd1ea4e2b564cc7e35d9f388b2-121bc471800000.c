// https://syzkaller.appspot.com/bug?id=1379b6b21a2ffecd1ea4e2b564cc7e35d9f388b2
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

static uintptr_t syz_open_dev(uintptr_t a0, uintptr_t a1, uintptr_t a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf));
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_epoll_create1
#define __NR_epoll_create1 329
#endif
#ifndef __NR_epoll_ctl
#define __NR_epoll_ctl 255
#endif
#ifndef __NR_ioctl
#define __NR_ioctl 54
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[2];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_epoll_create1, 0);
  memcpy((void*)0x20002000, "/dev/binder#", 13);
  r[1] = syz_open_dev(0x20002000, 0, 0);
  *(uint32_t*)0x20cf7ff4 = 0;
  *(uint64_t*)0x20cf7ff8 = 0;
  syscall(__NR_epoll_ctl, r[0], 1, r[1], 0x20cf7ff4);
  syscall(__NR_ioctl, r[1], 0x40046208, 0);
}

int main()
{
  loop();
  return 0;
}
