// https://syzkaller.appspot.com/bug?id=ee41ab509c8c9712af5749cd659acf57c216079e
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

static uintptr_t syz_open_procfs(uintptr_t a0, uintptr_t a1)
{

  char buf[128];
  memset(buf, 0, sizeof(buf));
  if (a0 == 0) {
    snprintf(buf, sizeof(buf), "/proc/self/%s", (char*)a1);
  } else if (a0 == (uintptr_t)-1) {
    snprintf(buf, sizeof(buf), "/proc/thread-self/%s", (char*)a1);
  } else {
    snprintf(buf, sizeof(buf), "/proc/self/task/%d/%s", (int)a0,
             (char*)a1);
  }
  int fd = open(buf, O_RDWR);
  if (fd == -1)
    fd = open(buf, O_RDONLY);
  return fd;
}

long r[9];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xe000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  r[1] =
      syscall(__NR_ioctl, 0xffffffffffffff9cul, 0x540ful, 0x20009ffcul);
  if (r[1] != -1)
    r[2] = *(uint32_t*)0x20009ffc;
  memcpy((void*)0x20003000, "\x61\x74\x74\x72\x2f\x73\x6f\x63\x6b\x63"
                            "\x72\x65\x61\x74\x65\x00",
         16);
  r[4] = syz_open_procfs(r[2], 0x20003000ul);
  memcpy((void*)0x20002fff, "\x54", 1);
  r[6] = syscall(__NR_write, r[4], 0x20002ffful, 0x1ul);
  *(uint64_t*)0x20004ff8 = (uint64_t)0x0;
  r[8] = syscall(__NR_sendfile, r[4], r[4], 0x20004ff8ul, 0x1ul);
}

int main()
{
  loop();
  return 0;
}
