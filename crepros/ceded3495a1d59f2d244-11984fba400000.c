// https://syzkaller.appspot.com/bug?id=8f063539d4ecf1faf3132624b57a641e923ee25a
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x20000000, "/dev/kvm", 9);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000000, 0, 0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_ioctl, r[0], 0xae01, 0);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_ioctl, r[1], 0xae41, 0);
  if (res != -1)
    r[2] = res;
  *(uint32_t*)0x20000300 = 2;
  *(uint32_t*)0x20000304 = 0;
  *(uint64_t*)0x20000308 = 0x491;
  *(uint64_t*)0x20000310 = 0;
  *(uint64_t*)0x20000318 = 0;
  *(uint64_t*)0x20000320 = 0;
  *(uint8_t*)0x20000328 = 0;
  *(uint8_t*)0x20000329 = 0;
  *(uint8_t*)0x2000032a = 0;
  *(uint8_t*)0x2000032b = 0;
  *(uint8_t*)0x2000032c = 0;
  *(uint8_t*)0x2000032d = 0;
  *(uint8_t*)0x2000032e = 0;
  *(uint8_t*)0x2000032f = 0;
  *(uint8_t*)0x20000330 = 0;
  *(uint8_t*)0x20000331 = 0;
  *(uint8_t*)0x20000332 = 0;
  *(uint8_t*)0x20000333 = 0;
  *(uint8_t*)0x20000334 = 0;
  *(uint8_t*)0x20000335 = 0;
  *(uint8_t*)0x20000336 = 0;
  *(uint8_t*)0x20000337 = 0;
  *(uint8_t*)0x20000338 = 0;
  *(uint8_t*)0x20000339 = 0;
  *(uint8_t*)0x2000033a = 0;
  *(uint8_t*)0x2000033b = 0;
  *(uint8_t*)0x2000033c = 0;
  *(uint8_t*)0x2000033d = 0;
  *(uint8_t*)0x2000033e = 0;
  *(uint8_t*)0x2000033f = 0;
  *(uint8_t*)0x20000340 = 0;
  *(uint8_t*)0x20000341 = 0;
  *(uint8_t*)0x20000342 = 0;
  *(uint8_t*)0x20000343 = 0;
  *(uint8_t*)0x20000344 = 0;
  *(uint8_t*)0x20000345 = 0;
  *(uint8_t*)0x20000346 = 0;
  *(uint8_t*)0x20000347 = 0;
  *(uint8_t*)0x20000348 = 0;
  *(uint8_t*)0x20000349 = 0;
  *(uint8_t*)0x2000034a = 0;
  *(uint8_t*)0x2000034b = 0;
  *(uint8_t*)0x2000034c = 0;
  *(uint8_t*)0x2000034d = 0;
  *(uint8_t*)0x2000034e = 0;
  *(uint8_t*)0x2000034f = 0;
  *(uint8_t*)0x20000350 = 0;
  *(uint8_t*)0x20000351 = 0;
  *(uint8_t*)0x20000352 = 0;
  *(uint8_t*)0x20000353 = 0;
  *(uint8_t*)0x20000354 = 0;
  *(uint8_t*)0x20000355 = 0;
  *(uint8_t*)0x20000356 = 0;
  *(uint8_t*)0x20000357 = 0;
  *(uint8_t*)0x20000358 = 0;
  *(uint8_t*)0x20000359 = 0;
  *(uint8_t*)0x2000035a = 0;
  *(uint8_t*)0x2000035b = 0;
  *(uint8_t*)0x2000035c = 0;
  *(uint8_t*)0x2000035d = 0;
  *(uint8_t*)0x2000035e = 0;
  *(uint8_t*)0x2000035f = 0;
  *(uint8_t*)0x20000360 = 0;
  *(uint8_t*)0x20000361 = 0;
  *(uint8_t*)0x20000362 = 0;
  *(uint8_t*)0x20000363 = 0;
  *(uint8_t*)0x20000364 = 0;
  *(uint8_t*)0x20000365 = 0;
  *(uint8_t*)0x20000366 = 0;
  *(uint8_t*)0x20000367 = 0;
  syscall(__NR_ioctl, r[2], 0x4080aebf, 0x20000300);
  *(uint16_t*)0x2099e000 = 2;
  *(uint16_t*)0x2099e002 = htobe16(0x4e20);
  *(uint32_t*)0x2099e004 = htobe32(0xe0000001);
  *(uint8_t*)0x2099e008 = 0;
  *(uint8_t*)0x2099e009 = 0;
  *(uint8_t*)0x2099e00a = 0;
  *(uint8_t*)0x2099e00b = 0;
  *(uint8_t*)0x2099e00c = 0;
  *(uint8_t*)0x2099e00d = 0;
  *(uint8_t*)0x2099e00e = 0;
  *(uint8_t*)0x2099e00f = 0;
  syscall(__NR_bind, -1, 0x2099e000, 0x10);
  return 0;
}
