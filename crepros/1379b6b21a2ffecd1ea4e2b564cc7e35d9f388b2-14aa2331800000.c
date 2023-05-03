// https://syzkaller.appspot.com/bug?id=1379b6b21a2ffecd1ea4e2b564cc7e35d9f388b2
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <stdint.h>
#include <string.h>

static uintptr_t syz_open_dev(uintptr_t a0, uintptr_t a1, uintptr_t a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block",
            (uint8_t)a1, (uint8_t)a2);
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

long r[3];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0x10000, 0x3, 0x32, 0xffffffff, 0x0);
  memcpy((void*)0x20008000, "/dev/binder#", 13);
  r[0] = syz_open_dev(0x20008000, 0x0, 0x0);
  syscall(__NR_close, r[0]);
  memcpy((void*)0x20008ff3, "/dev/binder#", 13);
  r[1] = syz_open_dev(0x20008ff3, 0x0, 0x0);
  r[2] = syscall(__NR_epoll_create, 0x8000);
  *(uint32_t*)0x2000c000 = 0x0;
  *(uint64_t*)0x2000c004 = 0x0;
  syscall(__NR_epoll_ctl, r[2], 0x1, r[0], 0x2000c000);
  *(uint64_t*)0x20007000 = 0x14;
  *(uint64_t*)0x20007008 = 0x0;
  *(uint64_t*)0x20007010 = 0x2000c000;
  *(uint64_t*)0x20007018 = 0x0;
  *(uint64_t*)0x20007020 = 0x0;
  *(uint64_t*)0x20007028 = 0x20001fff;
  *(uint32_t*)0x2000c000 = 0x40486311;
  *(uint64_t*)0x2000c004 = 0x0;
  *(uint64_t*)0x2000c00c = 0x0;
  syscall(__NR_ioctl, r[0], 0xc0306201, 0x20007000);
  *(uint64_t*)0x2000afd0 = 0x44;
  *(uint64_t*)0x2000afd8 = 0x0;
  *(uint64_t*)0x2000afe0 = 0x20009f84;
  *(uint64_t*)0x2000afe8 = 0x0;
  *(uint64_t*)0x2000aff0 = 0x0;
  *(uint64_t*)0x2000aff8 = 0x20003fb3;
  *(uint32_t*)0x20009f84 = 0x40486311;
  *(uint32_t*)0x20009f88 = 0x0;
  *(uint32_t*)0x20009f8c = 0x0;
  *(uint64_t*)0x20009f90 = 0x0;
  *(uint32_t*)0x20009f98 = 0x0;
  *(uint32_t*)0x20009f9c = 0x0;
  *(uint32_t*)0x20009fa0 = 0x0;
  *(uint32_t*)0x20009fa4 = 0x0;
  *(uint64_t*)0x20009fa8 = 0x0;
  *(uint64_t*)0x20009fb0 = 0x0;
  *(uint64_t*)0x20009fb8 = 0x2000a000;
  *(uint64_t*)0x20009fc0 = 0x20003000;
  syscall(__NR_ioctl, r[0], 0xc0306201, 0x2000afd0);
  syscall(__NR_ioctl, r[1], 0x40046208, 0x0);
}

int main()
{
  loop();
  return 0;
}
