// https://syzkaller.appspot.com/bug?id=9853146f1bf4c6496218371ab4847d895cfa444f
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

static long syz_open_procfs(volatile long a0, volatile long a1)
{
  char buf[128];
  memset(buf, 0, sizeof(buf));
  if (a0 == 0) {
    snprintf(buf, sizeof(buf), "/proc/self/%s", (char*)a1);
  } else if (a0 == -1) {
    snprintf(buf, sizeof(buf), "/proc/thread-self/%s", (char*)a1);
  } else {
    snprintf(buf, sizeof(buf), "/proc/self/task/%d/%s", (int)a0, (char*)a1);
  }
  int fd = open(buf, O_RDWR);
  if (fd == -1)
    fd = open(buf, O_RDONLY);
  return fd;
}

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);

  memcpy((void*)0x20000080, "mountinfo\000", 10);
  syz_open_procfs(0, 0x20000080);
  memcpy((void*)0x200001c0, "./file0\000", 8);
  syscall(__NR_mkdir, 0x200001c0, 0);
  memcpy((void*)0x200000c0, "./file0\000", 8);
  memcpy((void*)0x20000080, "fuse.", 5);
  memcpy((void*)0x200002c0, "\x66\x64\x2c\xe4\x3a\xe7\xe3\xdd\xfa\x46\xe4\x87"
                            "\x75\x91\x1d\xfb\xa0\x19\x2f\xf1\xba\x2f\x48\xda",
         24);
  sprintf((char*)0x200002d8, "0x%016llx", (long long)-1);
  sprintf((char*)0x200002ea, "%020llu", (long long)0);
  sprintf((char*)0x200002fe, "%020llu", (long long)0);
  syscall(__NR_mount, 0, 0x200000c0, 0x20000080, 0, 0x200002c0);
  return 0;
}
