// https://syzkaller.appspot.com/bug?id=a332f9b717df8cd78df6bcd8306a2a9f89712817
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

#ifndef __NR_mmap
#define __NR_mmap 222
#endif
#ifndef __NR_prctl
#define __NR_prctl 167
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
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  intptr_t res = 0;
  memcpy((void*)0x20000080, "/dev/bus/usb/00#/00#\000", 21);
  res = -1;
  res = syz_open_dev(/*dev=*/0x20000080, /*id=*/0x200, /*flags=*/0);
  if (res != -1)
    r[0] = res;
  syscall(__NR_prctl, /*option=*/0x41ul, /*opt=*/3ul, /*arg=*/0ul, /*len=*/0ul,
          0);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x400000ul,
          /*prot=PROT_GROWSUP|PROT_WRITE|PROT_READ|PROT_EXEC*/ 0x2000007ul,
          /*flags=MAP_FIXED|MAP_PRIVATE*/ 0x12ul, /*fd=*/r[0], /*offset=*/0ul);
  return 0;
}
