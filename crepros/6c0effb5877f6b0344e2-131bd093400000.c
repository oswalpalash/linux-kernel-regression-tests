// https://syzkaller.appspot.com/bug?id=10cd2ff2ccd320618b127ec50ea6e5a55461cd76
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

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  *(uint32_t*)0x200000c0 = 0;
  *(uint32_t*)0x200000c4 = 0;
  *(uint32_t*)0x200000c8 = 0xfffffe85;
  *(uint32_t*)0x200000cc = 0;
  *(uint64_t*)0x200000d0 = 0;
  *(uint64_t*)0x200000d8 = 0x7530;
  *(uint32_t*)0x200000e0 = 0;
  *(uint32_t*)0x200000e4 = 0;
  *(uint8_t*)0x200000e8 = 0;
  *(uint8_t*)0x200000e9 = 0;
  *(uint8_t*)0x200000ea = 0;
  *(uint8_t*)0x200000eb = 0;
  memcpy((void*)0x200000ec, "\x54\x6a\x7c\x16", 4);
  *(uint32_t*)0x200000f0 = 0;
  *(uint32_t*)0x200000f4 = 0;
  *(uint32_t*)0x200000f8 = 0xffffff9c;
  *(uint32_t*)0x20000100 = 4;
  *(uint32_t*)0x20000104 = 0;
  *(uint32_t*)0x20000108 = 0;
  syscall(__NR_ioctl, -1, 0xc058560f, 0x200000c0);
  memcpy((void*)0x20000040, "/dev/video#", 12);
  res = syz_open_dev(0x20000040, 0x6287, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x200002c0 = 3;
  *(uint32_t*)0x200002c8 = 0;
  *(uint32_t*)0x200002cc = 0;
  *(uint32_t*)0x200002d0 = 0;
  *(uint32_t*)0x200002d4 = 0;
  *(uint32_t*)0x200002d8 = 0;
  *(uint32_t*)0x200002dc = 0;
  *(uint64_t*)0x200002e0 = 0x200000c0;
  *(uint32_t*)0x200000c0 = 0;
  *(uint32_t*)0x200000c4 = 0;
  *(uint32_t*)0x200000c8 = 0;
  *(uint32_t*)0x200000cc = 0;
  *(uint64_t*)0x200000d0 = 0x20000080;
  *(uint32_t*)0x20000080 = 0;
  *(uint32_t*)0x20000084 = 0;
  *(uint32_t*)0x20000088 = 0;
  *(uint32_t*)0x2000008c = 0;
  *(uint64_t*)0x20000090 = 0x20000000;
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x2000000c = 0;
  *(uint64_t*)0x20000010 = 0;
  *(uint32_t*)0x200002e8 = 2;
  *(uint64_t*)0x200002f0 = 0x20000100;
  *(uint8_t*)0x200002f8 = 0;
  syscall(__NR_ioctl, r[0], 0xc0d05605, 0x200002c0);
  memcpy((void*)0x20000200, "/dev/video#", 12);
  res = syz_open_dev(0x20000200, 3, 0);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x200001c0 = 1;
  syscall(__NR_ioctl, r[1], 0xc0045627, 0x200001c0);
  memcpy((void*)0x20000040, "/dev/snd/pcmC#D#c", 18);
  res = syz_open_dev(0x20000040, 1, 0x509000);
  if (res != -1)
    r[2] = res;
  *(uint16_t*)0x20000140 = 0x101;
  *(uint8_t*)0x20000142 = 1;
  *(uint8_t*)0x20000143 = 6;
  *(uint16_t*)0x20000144 = 9;
  *(uint32_t*)0x20000148 = 6;
  syscall(__NR_setsockopt, r[2], 0x29, 0xca, 0x20000140, 0xc);
  *(uint64_t*)0x200001c0 = 0x80000001;
  syscall(__NR_mbind, 0x20012000, 0xc00000, 2, 0x200001c0, 0x100, 0);
  syscall(__NR_get_mempolicy, 0x200005c0, 0x203e8000, 0x401, 0x208d3000, 3);
  syscall(__NR_ioctl, -1, 0x127b, 0x20000080);
  syscall(__NR_listen, -1, 0);
  syscall(__NR_ioctl, -1, 0x20000000008912, 0x200001c0);
  *(uint32_t*)0x20000040 = 0xfffffff9;
  syscall(__NR_setsockopt, -1, 0x84, 0x1e, 0x20000040, 4);
  memcpy((void*)0x20000200, "/dev/video#", 12);
  res = syz_open_dev(0x20000200, 3, 0);
  if (res != -1)
    r[3] = res;
  memcpy((void*)0x20000040, "/proc/sys/net/ipv4/vs/sync_threshold", 37);
  syscall(__NR_openat, 0xffffffffffffff9c, 0x20000040, 2, 0);
  *(uint32_t*)0x20000000 = 9;
  *(uint32_t*)0x20000004 = 0x100;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x2000000c = 0;
  *(uint32_t*)0x20000010 = 0xfdfd;
  *(uint32_t*)0x20000014 = 0;
  *(uint32_t*)0x20000018 = 0;
  *(uint32_t*)0x2000001c = 0;
  *(uint32_t*)0x20000020 = 0;
  *(uint32_t*)0x20000024 = 0;
  *(uint32_t*)0x20000028 = 0;
  *(uint32_t*)0x2000002c = 0;
  *(uint32_t*)0x20000030 = 0;
  *(uint32_t*)0x20000034 = 0;
  *(uint32_t*)0x20000038 = 0;
  *(uint32_t*)0x2000003c = 0;
  syscall(__NR_ioctl, r[3], 0xc040565f, 0x20000000);
  return 0;
}
