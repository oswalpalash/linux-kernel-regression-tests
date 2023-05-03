// https://syzkaller.appspot.com/bug?id=3b95065b0a91c5c706a6ccaa354d847e253dc418
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

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

long r[80];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  memcpy((void*)0x20539000,
         "\x2f\x64\x65\x76\x2f\x73\x6e\x64\x2f\x73\x65\x71\x00", 13);
  r[2] = syz_open_dev(0x20539000ul, 0x0ul, 0x0ul);
  *(uint8_t*)0x20ad3000 = (uint8_t)0xb40e;
  *(uint8_t*)0x20ad3001 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3002 = (uint8_t)0xe;
  *(uint8_t*)0x20ad3003 = (uint8_t)0x0;
  *(uint32_t*)0x20ad3004 = (uint32_t)0x0;
  *(uint32_t*)0x20ad3008 = (uint32_t)0x10003fe;
  *(uint8_t*)0x20ad300c = (uint8_t)0xfffffffffffffffd;
  *(uint8_t*)0x20ad300d = (uint8_t)0x0;
  *(uint8_t*)0x20ad300e = (uint8_t)0x0;
  *(uint8_t*)0x20ad300f = (uint8_t)0x0;
  *(uint8_t*)0x20ad3010 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3011 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3012 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3013 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3014 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3015 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3016 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3017 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3018 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3019 = (uint8_t)0x0;
  *(uint8_t*)0x20ad301a = (uint8_t)0x0;
  *(uint8_t*)0x20ad301b = (uint8_t)0x0;
  *(uint8_t*)0x20ad301c = (uint8_t)0x0;
  *(uint8_t*)0x20ad301d = (uint8_t)0x0;
  *(uint8_t*)0x20ad301e = (uint8_t)0x0;
  *(uint8_t*)0x20ad301f = (uint8_t)0x0;
  *(uint8_t*)0x20ad3020 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3021 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3022 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3023 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3024 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3025 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3026 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3027 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3028 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3029 = (uint8_t)0x0;
  *(uint8_t*)0x20ad302a = (uint8_t)0x0;
  *(uint8_t*)0x20ad302b = (uint8_t)0x0;
  *(uint8_t*)0x20ad302c = (uint8_t)0x0;
  *(uint8_t*)0x20ad302d = (uint8_t)0x0;
  *(uint8_t*)0x20ad302e = (uint8_t)0x0;
  *(uint8_t*)0x20ad302f = (uint8_t)0x0;
  *(uint8_t*)0x20ad3030 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3031 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3032 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3033 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3034 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3035 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3036 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3037 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3038 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3039 = (uint8_t)0x0;
  *(uint8_t*)0x20ad303a = (uint8_t)0x0;
  *(uint8_t*)0x20ad303b = (uint8_t)0x0;
  *(uint8_t*)0x20ad303c = (uint8_t)0x0;
  *(uint8_t*)0x20ad303d = (uint8_t)0x0;
  *(uint8_t*)0x20ad303e = (uint8_t)0x0;
  *(uint8_t*)0x20ad303f = (uint8_t)0x0;
  *(uint8_t*)0x20ad3040 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3041 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3042 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3043 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3044 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3045 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3046 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3047 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3048 = (uint8_t)0x0;
  *(uint8_t*)0x20ad3049 = (uint8_t)0x0;
  *(uint8_t*)0x20ad304a = (uint8_t)0x0;
  *(uint8_t*)0x20ad304b = (uint8_t)0x0;
  *(uint8_t*)0x20ad304c = (uint8_t)0x0;
  *(uint8_t*)0x20ad304d = (uint8_t)0x0;
  *(uint8_t*)0x20ad304e = (uint8_t)0x0;
  *(uint8_t*)0x20ad304f = (uint8_t)0x0;
  r[77] = syscall(__NR_ioctl, r[2], 0x40505330ul, 0x20ad3000ul);
  memcpy((void*)0x2003b000,
         "\x2f\x64\x65\x76\x2f\x73\x65\x71\x75\x65\x6e\x63\x65\x72\x32"
         "\x00",
         16);
  r[79] = syscall(__NR_openat, 0xffffffffffffff9cul, 0x2003b000ul,
                  0x8402ul, 0x0ul);
}

int main()
{
  loop();
  return 0;
}
