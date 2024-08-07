// https://syzkaller.appspot.com/bug?id=d7f1b035775ccddd99af76c0fbf75137e3974890
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
#ifndef __NR_readv
#define __NR_readv 145
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
  memcpy((void*)0x20004ff6, "/dev/dsp#", 10);
  r[0] = syz_open_dev(0x20004ff6, 1, 0);
  *(uint32_t*)0x2000bff0 = 0x20008000;
  *(uint32_t*)0x2000bff4 = 0xa5;
  *(uint32_t*)0x2000bff8 = 0x20007000;
  *(uint32_t*)0x2000bffc = 0xd6;
  syscall(__NR_readv, r[0], 0x2000bff0, 2);
  *(uint64_t*)0x20006ff8 = 3;
  syscall(__NR_ioctl, r[0], 0xc004500a, 0x20006ff8);
}

int main()
{
  loop();
  return 0;
}
