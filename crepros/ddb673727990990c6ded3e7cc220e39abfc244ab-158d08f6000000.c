// https://syzkaller.appspot.com/bug?id=ddb673727990990c6ded3e7cc220e39abfc244ab
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

long r[8];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfcd000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  memcpy((void*)0x20124ff3,
         "\x2f\x64\x65\x76\x2f\x6e\x65\x74\x2f\x74\x75\x6e\x00", 13);
  r[2] = syz_open_dev(0x20124ff3ul, 0x0ul, 0x0ul);
  *(uint32_t*)0x200caffc = (uint32_t)0x1;
  r[4] = syscall(__NR_ioctl, r[2], 0x400454daul, 0x200caffcul);
  memcpy((void*)0x20511fd8, "\x30\x60\x94\x02\x00\x00\x00\x00\x00\x00"
                            "\x06\x7d\x08\xce\xbd\x5c",
         16);
  memcpy((void*)0x20511fe8, "\xc2\x00\x07\x11\x5f\x09\x00\x07\x00\x00"
                            "\x00\xc2\x83\x8a\x1c\x96",
         16);
  r[7] = syscall(__NR_ioctl, r[2], 0x400454caul, 0x20511fd8ul);
}

int main()
{
  loop();
  return 0;
}
