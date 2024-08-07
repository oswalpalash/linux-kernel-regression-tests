// https://syzkaller.appspot.com/bug?id=425ba1ad5aae5458b5cdfc8c4227dfb988a28775
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

long r[1];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  memcpy((void*)0x20159000, "/dev/loop#", 11);
  r[0] = syz_open_dev(0x20159000, 0, 0);
  *(uint32_t*)0x20beaf68 = 0;
  *(uint32_t*)0x20beaf6c = 0;
  *(uint32_t*)0x20beaf70 = 0;
  *(uint32_t*)0x20beaf74 = 0;
  *(uint32_t*)0x20beaf78 = 0;
  *(uint32_t*)0x20beaf7c = 0;
  *(uint32_t*)0x20beaf80 = 0;
  *(uint32_t*)0x20beaf84 = 0;
  memcpy((void*)0x20beaf88,
         "\x00\x00\x00\x00\x01\x00\x00\x00\x01\x00\x1b\x16\x00\x00\x01\x00\x00"
         "\x65\x00\x00\x00\x01\x00\x00\x00\x7d\xb0\xe6\xf1\x0e\xfb\xf9\xa2\x19"
         "\xd8\xf6\xaa\x6b\xd5\x8d\x1c\x43\x47\x31\x00\xe8\x50\x26\xe7\xff\x40"
         "\xf9\xb5\x5b\xd1\xb3\x33\x5d\x5b\xff\xff\x00\x01\xf3",
         64);
  memcpy((void*)0x20beafc8, "\xcf\xa4\x00\x05\x00\x00\x00\xf7\xff\xff\xff\xff"
                            "\x00\x00\x00\x00\x00\x00\x00\xff\xb8\x33\x22\x01"
                            "\x82\xab\x86\x7d\x00\x00\x00\x00",
         32);
  *(uint64_t*)0x20beafe8 = 0;
  *(uint64_t*)0x20beaff0 = 0;
  *(uint32_t*)0x20beaff8 = 0;
  syscall(__NR_ioctl, r[0], 0xc0481273, 0x20beaf68);
}

int main()
{
  loop();
  return 0;
}
