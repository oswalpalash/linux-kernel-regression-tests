// https://syzkaller.appspot.com/bug?id=bf154052f0eea4bc7712499e4569505907d15889
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

static long syz_open_dev(long a0, long a1, long a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

#ifndef __NR_close
#define __NR_close 6
#endif
#ifndef __NR_ioctl
#define __NR_ioctl 54
#endif
#ifndef __NR_memfd_create
#define __NR_memfd_create 356
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_open
#define __NR_open 5
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x2000fffa, "./bus", 6);
  res = syscall(__NR_open, 0x2000fffa, 0x141042, 0);
  if (res != -1)
    r[0] = res;
  syscall(__NR_close, (long)r[0]);
  memcpy((void*)0x20ca9ff5, "/dev/loop#", 11);
  res = syz_open_dev(0x20ca9ff5, 0, 0);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x20000140,
         "\x00\x00\x00\x00\x8c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 15);
  res = syscall(__NR_memfd_create, 0x20000140, 0);
  if (res != -1)
    r[2] = res;
  syscall(__NR_ioctl, (long)r[1], 0x4c00, (long)r[2]);
  *(uint32_t*)0x20000300 = 0;
  *(uint32_t*)0x20000304 = 0;
  *(uint32_t*)0x20000308 = 0;
  *(uint32_t*)0x2000030c = 0;
  *(uint32_t*)0x20000310 = 0;
  *(uint32_t*)0x20000314 = 0;
  *(uint32_t*)0x20000318 = 0;
  *(uint32_t*)0x2000031c = 0x18;
  memcpy((void*)0x20000320,
         "\x75\xec\xe2\xee\xd4\x35\x8f\xcb\xd6\x39\xc2\xdc\xcb\x34\x27\x27\xa7"
         "\x37\xee\xfb\xae\x39\xe0\xab\xbb\x50\xc3\x50\x1b\xab\x32\xf6\xb2\x92"
         "\x8d\x67\xc7\x8f\xb7\x0a\x46\xc1\x4b\x47\xeb\x59\xfc\x9a\x27\x8e\x97"
         "\x4a\x5e\x0b\x95\x32\x52\xf2\x46\xc1\xc4\x36\xc6\xc6",
         64);
  memcpy((void*)0x20000360, "\x2b\x86\x54\xd9\xa5\x0e\x2b\xff\xd2\x83\xff\x62"
                            "\x26\x28\x6b\x6e\xc2\x16\x5d\x30\xa4\xf6\xf2\xee"
                            "\xdc\x6f\x77\x84\x55\xef\xc6\x87",
         32);
  *(uint32_t*)0x20000380 = -1;
  *(uint32_t*)0x20000384 = 0;
  *(uint32_t*)0x20000388 = 0;
  syscall(__NR_ioctl, (long)r[0], 0x4c02, 0x20000300);
  return 0;
}
