// https://syzkaller.appspot.com/bug?id=6b8d6b1847122db76e4ebd32b9d580684bac133c
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <fcntl.h>
#include <stdio.h>
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

long r[53];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  memcpy((void*)0x20fa6000, "\x2f\x64\x65\x76\x2f\x73\x67\x23\x00", 9);
  r[2] = syz_open_dev(0x20fa6000ul, 0x0ul, 0x0ul);
  *(uint32_t*)0x2084cf90 = (uint32_t)0x0;
  *(uint16_t*)0x2084cf94 = (uint16_t)0x0;
  *(uint8_t*)0x2084cf96 = (uint8_t)0x0;
  *(uint8_t*)0x2084cf97 = (uint8_t)0x0;
  *(uint8_t*)0x2084cf98 = (uint8_t)0x0;
  *(uint8_t*)0x2084cf99 = (uint8_t)0x0;
  *(uint8_t*)0x2084cf9a = (uint8_t)0xfffffffffffffffa;
  *(uint8_t*)0x2084cf9b = (uint8_t)0x0;
  *(uint8_t*)0x2084cf9c = (uint8_t)0x0;
  *(uint8_t*)0x2084cf9d = (uint8_t)0x0;
  *(uint8_t*)0x2084cf9e = (uint8_t)0x0;
  *(uint8_t*)0x2084cf9f = (uint8_t)0x0;
  *(uint64_t*)0x2084cfa0 = (uint64_t)0x0;
  *(uint32_t*)0x2084cfa8 = (uint32_t)0x0;
  *(uint16_t*)0x2084cfac = (uint16_t)0x0;
  *(uint8_t*)0x2084cfae = (uint8_t)0x0;
  *(uint8_t*)0x2084cfaf = (uint8_t)0x0;
  *(uint8_t*)0x2084cfb0 = (uint8_t)0x0;
  *(uint8_t*)0x2084cfb1 = (uint8_t)0x0;
  *(uint8_t*)0x2084cfb2 = (uint8_t)0x0;
  *(uint8_t*)0x2084cfb3 = (uint8_t)0x0;
  *(uint8_t*)0x2084cfb4 = (uint8_t)0xfffffffffffffffc;
  *(uint8_t*)0x2084cfb5 = (uint8_t)0x0;
  *(uint8_t*)0x2084cfb6 = (uint8_t)0x0;
  *(uint8_t*)0x2084cfb7 = (uint8_t)0x3f;
  *(uint64_t*)0x2084cfb8 = (uint64_t)0x4;
  *(uint32_t*)0x2084cfc0 = (uint32_t)0x0;
  *(uint16_t*)0x2084cfc4 = (uint16_t)0x0;
  *(uint8_t*)0x2084cfc6 = (uint8_t)0x0;
  *(uint8_t*)0x2084cfc7 = (uint8_t)0x8000;
  *(uint8_t*)0x2084cfc8 = (uint8_t)0x101;
  *(uint8_t*)0x2084cfc9 = (uint8_t)0x4;
  *(uint8_t*)0x2084cfca = (uint8_t)0x7;
  *(uint8_t*)0x2084cfcb = (uint8_t)0x9;
  *(uint8_t*)0x2084cfcc = (uint8_t)0x3;
  *(uint8_t*)0x2084cfcd = (uint8_t)0x8;
  *(uint8_t*)0x2084cfce = (uint8_t)0x2;
  *(uint8_t*)0x2084cfcf = (uint8_t)0x946;
  *(uint64_t*)0x2084cfd0 = (uint64_t)0x40;
  *(uint32_t*)0x2084cfd8 = (uint32_t)0x7;
  *(uint32_t*)0x2084cfdc = (uint32_t)0x0;
  *(uint32_t*)0x2084cfe0 = (uint32_t)0x0;
  *(uint32_t*)0x2084cfe4 = (uint32_t)0x0;
  *(uint32_t*)0x2084cfe8 = (uint32_t)0x0;
  *(uint32_t*)0x2084cfec = (uint32_t)0x0;
  *(uint32_t*)0x2084cff0 = (uint32_t)0x0;
  *(uint32_t*)0x2084cff4 = (uint32_t)0x0;
  *(uint32_t*)0x2084cff8 = (uint32_t)0x0;
  *(uint32_t*)0x2084cffc = (uint32_t)0x0;
  r[52] = syscall(__NR_ioctl, r[2], 0xc0481273ul, 0x2084cf90ul);
}

int main()
{
  loop();
  return 0;
}
