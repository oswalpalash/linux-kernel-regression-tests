// https://syzkaller.appspot.com/bug?id=5f31b30406d84a86d4b7a42c103f6eda07eed137
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

#ifndef __NR_ioctl
#define __NR_ioctl 54
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x20000000, "/dev/video#", 12);
  res = syz_open_dev(0x20000000, 0, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000500 = 5;
  *(uint32_t*)0x20000504 = 1;
  *(uint32_t*)0x20000508 = 1;
  *(uint32_t*)0x2000050c = 0;
  *(uint32_t*)0x20000510 = 0;
  syscall(__NR_ioctl, (long)r[0], 0xc0145608, 0x20000500);
  *(uint32_t*)0x20000940 = 0;
  *(uint32_t*)0x20000944 = 9;
  *(uint32_t*)0x20000948 = 4;
  *(uint32_t*)0x2000094c = 1;
  memcpy((void*)0x20000950,
         "\x9d\x0d\x1d\x4e\xf5\xfd\xaa\x6a\x0c\x18\x1e\x7b\x99\x16\x17\xa6\x4b"
         "\x2c\xd0\x75\xf8\xc6\x7d\xd6\x03\x25\xc6\x02\x76\x89\x19\xef\x53\x0b"
         "\x4b\x48\xfd\x4b\x83\xb9\xe3\x9e\x82\x49\x5f\x73\xb4\x4d\x72\x63\x8b"
         "\x57\x83\xf9\x44\x03\x78\x22\xd6\x7e\x4c\x29\xe1\xc7\x55\x7f\x21\x3d"
         "\xe4\x55\x4d\x9f\x1f\x03\xe1\x17\x1f\x15\xf1\xf0\x99\x96\x21\xba\x86"
         "\xe7\x8e\x2f\x33\x54\x8c\xb0\xa5\x64\x63\xd9\x37\x6f\x6c\xe5\xe0\x85"
         "\xa1\xcc\x7b\x2c\x5c\x1a\x46\x44\x02\x2c\x2b\xbe\xc2\x42\xe6\x79\x0c"
         "\xcc\x54\xe8\x77\xce\x77\xf4\x4d\x55\x07\x46\x00\x48\x2f\x99\xd8\x60"
         "\x98\x36\xd3\x3f\xee\xd7\x95\x93\x43\x8e\x60\xd2\x54\x42\x91\x7c\x45"
         "\x22\x53\x51\x05\xd2\x8f\x52\x40\x45\xe1\x48\x83\xde\x43\xac\x9c\xca"
         "\x44\xce\x68\x40\xa4\xa3\xc0\xce\x2e\x40\x9b\xef\xc0\x85\x6b\x56\x71"
         "\x1a\xd5\x78\x88\x27\x18\x17\x2b\x8a\x79\xcd\xb7\xd8",
         200);
  *(uint32_t*)0x20000a18 = 0;
  *(uint32_t*)0x20000a1c = 0;
  *(uint32_t*)0x20000a20 = 0;
  *(uint32_t*)0x20000a24 = 0;
  *(uint32_t*)0x20000a28 = 0;
  *(uint32_t*)0x20000a2c = 0;
  *(uint32_t*)0x20000a30 = 0;
  *(uint32_t*)0x20000a34 = 0;
  syscall(__NR_ioctl, (long)r[0], 0xc100565c, 0x20000940);
  return 0;
}
