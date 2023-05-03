// https://syzkaller.appspot.com/bug?id=4694bd1c1c0019f067af5b6e14e8ef02431b6b34
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

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

static long syz_open_dev(volatile long a0, volatile long a1, volatile long a2)
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
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  *(uint64_t*)0x20000240 = 0;
  *(uint32_t*)0x20000248 = -1;
  syscall(__NR_bpf, 0x11ul, 0x20000240ul, 0x10ul);
  memcpy((void*)0x20000180, "/dev/sg#\000", 9);
  res = -1;
  res = syz_open_dev(0x20000180, 0, 0x805);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000040 = 0x200001c0;
  memcpy((void*)0x200001c0,
         "\xae\x09\x09\x00\x00\x20\x00\x00\x00\x90\xf5\x7f\x07\x70\x30\xef\xf0"
         "\xf6\x4e\xbb\xee\xd9\x0c\xf4\x1b\xdd\x2a\xc8\xbb\x8c\x43\xb4\x60\xe4"
         "\x62\x92\xd2\x51\x33\x27\x0b\x44\x53\x93\xec\x19",
         46);
  *(uint64_t*)0x20000048 = 0x2e;
  *(uint64_t*)0x20000050 = 0x200000c0;
  memcpy((void*)0x200000c0,
         "\x53\x00\x00\x00\xfc\xff\xff\xff\x1b\x04\x48\x00\x2d\x14\x07\x00\x20"
         "\x02\x00\x20\x00\x00\x00\x00\x00\x00\x67\x20\x00\x00\x00\x00\x00\x00"
         "\x00\x00",
         36);
  *(uint64_t*)0x20000058 = 0x200000e4;
  syscall(__NR_writev, r[0], 0x20000040ul, 2ul);
  return 0;
}
