// https://syzkaller.appspot.com/bug?id=9ed44c22919e3cd2c71907b963fb048ac1115073
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
  res = -1;
  res = syz_open_dev(/*dev=*/0xc, /*major=*/4, /*minor=*/0x14);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000040 = 0x15;
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0x5423, /*arg=*/0x20000040ul);
  *(uint64_t*)0x20000400 = 0x20000180;
  memcpy((void*)0x20000180,
         "\x83\xfc\x8b\x84\x1d\x07\x3a\x98\x05\x5c\x26\x9b\x9c\x7f\x8b\xf7\xe3"
         "\xf0\x9b\xd3\xb7\x2f\x4d\x52\x93\x78\x48\xfd\x38\x66\x19\xe9\x16\x18"
         "\x6d\x47\xd6\xe4\x2d\x86\x2c\x63\x5f\xd6\x54\x7e\xbc\x2b\x11\xd4\xa8"
         "\xd3\xc0\xe6\x60\x0d\x32\x0e\xdc\xd1\xac\x61\x73\xb2\x5e\x7f\xac\xda"
         "\x94\x3a\x59\x62\x94\xc8\x38\x24\x5f\xd7\x8c\x9e\x9e\x40\xe1\x03\x7f"
         "\x4f\xd1\xc7\xde\xa8\x75\x24\xf1\x75\xfd\x0c\x6f\xcb\x75\x91\xa7\x57"
         "\xbc\x2c\x37\x41\xfd\xf2\xb7\x9b\x44\x68\x27\x61\xb4\x1d\x97\x3c\x05"
         "\xf2\x1b\xf6\xbc\x15\x24\x55\x01\x93\x54\x83\xdc\x5a\x81\x01\xf9\xf9"
         "\xf8\x8f\x76\x71\xa3\xd1\xfe\x07\x87\x82",
         146);
  *(uint64_t*)0x20000408 = 0x92;
  *(uint64_t*)0x20000410 = 0;
  *(uint64_t*)0x20000418 = 0;
  *(uint64_t*)0x20000420 = 0;
  *(uint64_t*)0x20000428 = 0;
  *(uint64_t*)0x20000430 = 0;
  *(uint64_t*)0x20000438 = 0;
  syscall(__NR_writev, /*fd=*/r[0], /*vec=*/0x20000400ul, /*vlen=*/4ul);
  return 0;
}
