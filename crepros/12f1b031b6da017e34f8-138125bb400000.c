// https://syzkaller.appspot.com/bug?id=a5954455fcfa51c29ca2ab55b203076337e1c770
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

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x20000200, "/dev/dri/card#\x00", 15);
  res = syz_open_dev(0x20000200, 0xc72f, 0);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000180 = 0;
  *(uint32_t*)0x20000188 = 0;
  *(uint32_t*)0x2000018c = 0x1f;
  *(uint32_t*)0x20000190 = 0;
  *(uint32_t*)0x20000194 = 0;
  *(uint32_t*)0x20000198 = 0;
  *(uint32_t*)0x2000019c = 0;
  *(uint32_t*)0x200001a0 = 0;
  *(uint16_t*)0x200001a4 = 0;
  *(uint16_t*)0x200001a6 = 0;
  *(uint16_t*)0x200001a8 = 0;
  *(uint16_t*)0x200001aa = 0;
  *(uint16_t*)0x200001ac = 0;
  *(uint16_t*)0x200001ae = 0;
  *(uint16_t*)0x200001b0 = 0;
  *(uint16_t*)0x200001b2 = 0;
  *(uint16_t*)0x200001b4 = 0;
  *(uint16_t*)0x200001b6 = 0;
  *(uint16_t*)0x200001b8 = 0;
  *(uint32_t*)0x200001bc = 0;
  *(uint32_t*)0x200001c0 = 0;
  memcpy((void*)0x200001c4, "\x77\x10\x0b\x6a\x1c\xc0\x76\xa1\x5a\xd9\xf0\x55"
                            "\x2d\x5b\x12\xbf\xc3\x38\xe5\x26\x9b\x12\xac\x2a"
                            "\x4e\xe4\x20\x2c\x64\x12\x2b\x0a",
         32);
  syscall(__NR_ioctl, r[0], 0xc06864a2, 0x20000180);
  return 0;
}
