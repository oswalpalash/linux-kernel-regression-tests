// https://syzkaller.appspot.com/bug?id=e594c81bfd24c3ef5206d993c986d1e0c41aed19
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
  syscall(__NR_mmap, 0x20000000, 0xc000, 3, 0x32, -1, 0);
  memcpy((void*)0x20009000, "/dev/snd/pcmC#D#c", 18);
  r[0] = syz_open_dev(0x20009000, 0, 0);
  *(uint32_t*)0x20002000 = 0;
  *(uint32_t*)0x20002004 = 0x80;
  *(uint32_t*)0x20005000 = 8;
  syscall(__NR_getsockopt, -1, 0x84, 0x75, 0x20002000, 0x20005000);
  memcpy((void*)0x20001f64,
         "\xac\x7d\x34\xf6\xf2\x99\x5a\x5f\xb9\xca\xe8\x95\x9c", 13);
  syscall(__NR_ioctl, r[0], 0xc1004110, 0x20001f64);
}

int main()
{
  loop();
  return 0;
}
