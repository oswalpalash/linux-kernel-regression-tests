// https://syzkaller.appspot.com/bug?id=4c915663aead5e3e4aa09f44247a0bed3808e569
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

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

long r[1];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
          0xfffffffffffffffful, 0x0ul);
  memcpy((void*)0x20016ff5, "/dev/loop#", 11);
  r[0] = syz_open_dev(0x20016ff5ul, 0xfffful, 0x0ul);
  *(uint32_t*)0x20ed6000 = (uint32_t)0x0;
  *(uint32_t*)0x20ed6004 = (uint32_t)0x0;
  *(uint32_t*)0x20ed6008 = (uint32_t)0x0;
  *(uint32_t*)0x20ed600c = (uint32_t)0x0;
  *(uint32_t*)0x20ed6010 = (uint32_t)0x0;
  *(uint32_t*)0x20ed6014 = (uint32_t)0x0;
  *(uint32_t*)0x20ed6018 = (uint32_t)0x0;
  *(uint32_t*)0x20ed601c = (uint32_t)0x0;
  memcpy((void*)0x20ed6020,
         "\x39\x00\xea\x63\x1d\x00\x00\x00\x00\x02\x00\x01\x04\x00\x00"
         "\x00\x00\x00\x00\x00\x23\xf7\xb7\xd6\x5f\x90\xb0\xe6\x33\xff"
         "\xff\xff\xf8\x19\xd8\xf6\xaa\x6b\xd5\x8d\x14\x43\x47\x46\x82"
         "\xe8\x50\x40\xfb\x49\x47\xeb\xb5\x5b\xd1\x9f\x33\x5b\x5b\xff"
         "\x04\x00\x01\xf3",
         64);
  memcpy((void*)0x20ed6060, "\xcf\xa4\x30\x74\x5a\x54\x0d\xc1\xc1\x49"
                            "\xb7\xb8\x15\x79\xf6\xa4\x1c\x51\xf7\xd5"
                            "\x19\x33\x22\x3e\x82\xab\x86\x7d\xac\x76"
                            "\x1f\xaf",
         32);
  *(uint64_t*)0x20ed6080 = (uint64_t)0x0;
  *(uint64_t*)0x20ed6088 = (uint64_t)0x0;
  *(uint32_t*)0x20ed6090 = (uint32_t)0x0;
  syscall(__NR_ioctl, r[0], 0xc0481273ul, 0x20ed6000ul);
}

int main()
{
  loop();
  return 0;
}
