// https://syzkaller.appspot.com/bug?id=1f0cb39636774fcacd954ab0bd24a0ebf086e4e0
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

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  res = syscall(__NR_socket, 2ul, 2ul, 0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000040,
         "lo\000\000\000\000\000\000\000\000\000\000\000\000\000\000", 16);
  *(uint32_t*)0x20000050 = 0;
  res = syscall(__NR_ioctl, r[0], 0x8933, 0x20000040ul);
  if (res != -1)
    r[1] = *(uint32_t*)0x20000050;
  *(uint64_t*)0x20000240 = 0;
  *(uint32_t*)0x20000248 = 0;
  *(uint64_t*)0x20000250 = 0x20000080;
  *(uint64_t*)0x20000080 = 0x20001200;
  memcpy((void*)0x20001200, "\x70\x08\x00\x00\x24\x00\x07\x05\x08\x00\x01\x00"
                            "\x00\x4b\x3b\xeb\xff\x16\x00\x79",
         20);
  *(uint32_t*)0x20001214 = r[1];
  memcpy(
      (void*)0x20001218,
      "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x08\x00\x01\x00\x74\x62"
      "\x66\x00\x44\x08\x02\x00\x08\x00\x0e\x00\x00\x00\x00\x00\x6b\x00\x01\x00"
      "\x00\xc2\xd6\x81\xad\x4a\x2a\x48\x66\xa2\x12\x04\x00\x00\x00\x00\x00\x00"
      "\xda\xa5\x00\x00\xeb\xff\x00\x00\x00\x00\x4c\x1f\x1e\xd4\x02\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x7e\x30\x39\xc2\xa1\xda\xec\xa8\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf5\xff\xff\xff\xff"
      "\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x1f\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x9b\x66\xe8\xfa\xaa\x90\x79\x36\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x8f\x7e\x8b\xc4\x66"
      "\xac\x1e\xd8\xa9\x19\xb3\x53\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x20\x00\x00\x5f\x5d\x3f\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x04\x8a\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10"
      "\x00\x81\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10\xde\x01"
      "\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x04\x04\x02\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xd3\x44\xb3\xac\x98\xbd\x2f"
      "\x3e\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x96\x55\x5c\xa9"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x2a\x75\x14\x19\x00\x03\x50\xf4\xbe\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x50\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x3c\xab\x58\x2f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x40\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\xf6\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x04\x00\x19\xf8\x9b\xb9\x73\x65\xba\x6a\xe3\x2a"
      "\xa8\xe7\x9b\x28\x3a\xbe\x61\x4b\x4f\x74\xa5\xe2\x07\xd1\x75\x7c\xee\xb7"
      "\x77\xfc\x4a\x93\x97\x5c\x05\x45\x56\xf9\x36\xca\xf3\xbc\x4b\x3a\x24\x45"
      "\xe1\xa5\x3f\x57\x8d\xa8\x43\x69\x72\xf2\xd4\x05\x51\x56\x8b\x38\x4f\x9d"
      "\xbd\x3c\xa1\x84\x80\x6b\xf5\xe1\x2e\x95\x08\x00\x00\x00\xa0\xd3\x9a\xee"
      "\x9a\x0d\xea\xd8\x0f\xe2\x8f\xe5\x1b\xbb\x87\xa5\x35\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x73\xfa\x6a\x67\xf6\xf5\xd9\x4b\x87\xb0\xfb\xa9\x80\xaf\x7d"
      "\x53\x24\x21\x9f\x98\x95\x2f\x6a\x3a\x38\xb4\x86\x80\x13\xd2\x82\xe1\x9f"
      "\xe9\x53\x88\xd8\xf5\x6a\x19\x38\x1d\x19\xf1\x99\xe0\x00\xaf\x63\x68\x1a"
      "\x1b\x22\x68\x82\xc5\xf4\x2d\x45\xa7\xc3\x80\x41\x88\x82\x1b\x9f\xd2\x29"
      "\xdf\x47\x16\x77\x9f\x1a\xf3\x30\xc0\x55\x83\x1e\x6b\xa3\x56\xc9\x71\x6d"
      "\x95\xd1\x70\xd3\x1f\x29\x98\xb4\x04\xf9\x5b\xd1\x3a\xbf\x94\x5b\x39\x39"
      "\x1b\x51\xe1\xa3\x20\x0a\x84\x7b\x9d\x51\x92\x0b\x2b\x18\x63\xb0\x95\x2f"
      "\xb4\x5b\xec\x60\x7a\x02\x32\x90\xce\x85\x6c\xf5\x9e\x04\xd7\x04\x00\x00"
      "\x00\x31\xa9\x79\x1f\x7e\x57\x2e\x5b\xcc\x21\xcd\xbc\xa8\xfa\xa3\x46\x26"
      "\x28\x76\xf1\x88\xd9\xbc\x55\x5f\xb4\x6d\x8f\xc5\x16\x46\x3f\x10\xf6\x54"
      "\x3c\xca\xa1\x36\x63\x43\x77\x62\xbe\x87\x3a\x01\x5e\x96\x93\x42\x52\xfe"
      "\x8c\xa3\x36\x21\x28\xc2\x28\x06\x58\x5a\xf9\xca\xee\x09\x7f\xfc\x42\xa2"
      "\xd5\xc1\x02\x00\xa2\x80\x48\xff\x28\x60\x64\xc6\x2e\xb9\x81\x23\x91\xe7"
      "\xc8\xd7\x34\x48\x37\x81\x1c\x92\x31\x22\x01\xb2\x93\xbd\x45\xf7\xd5\x0f"
      "\xe4\xd7\x15\x59\x63\x2f\x5c\x12\x18\x11\xcd\x97\xd4\x78\x4d\x7f\x87\x54"
      "\x1c\xb2\x77\x81\xf0\x31\x32\xdb\x02\xa2\xbf\x82\xac\xa2\x62\xa0\x71\x97"
      "\x7b\x59\xd3\x7c\x63\x4d\x97\x50\x8a\x31\xae\xa1\xad\xc1\x7f\x14\x6c\x57"
      "\xa2\x86\x63\xe9\x17\x5b\x81\x2d\x4d\x00\x3e\x6a\x4a\xba\x6b\x0f\x20\x48"
      "\x8e\x2b\x3d\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x9d\x23\x02\x00\x00"
      "\x00\x82\x3a\x06\x11\x26\xc1\x6e\x12\x18\x3b\x7a\x92\xf1\x5b\xd7\xfc\x6c"
      "\x57\x70\xc4\xed\x8d\x80\x9d\xa0\x4a\xbf\xa7\xcb\xce\xd6\xf9\xbb\x85\x60"
      "\xe6\xe0\x68\xa9\x1d\x88\xed\x9d\xf1\x7c\xbc\x5e\xcc\x53\x96\x73\x9d\x49"
      "\x10\xef\x76\x7b\x21\x09\xc6\x29\x9e\x32\xdd\xa6\xbe\x8d\x91\xf3\x89\xf2"
      "\x4f\xb3\xd8\x44\xed\x8b\xb6\x66\x3a\x5f\x34\xb2\x16\xbd\xc4\x15\xbd\xe2"
      "\x35\xd3\xfd\x1e\x4f\x34\xb5\x5b\xe0\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\xa7\x5b\x63\x0e\xda\xd2\x00\xf8\x81\x51\x56\x8e\x0e\xc2\x2e\x61\x1f\x29"
      "\x86\x97\x13\xba\x7f\x40\x6d\x73\xc1\xb2\x00\xee\xaa\x38\x36\x3d\xf4\xb1"
      "\x78\xa8\x86\x6e\x91\xec\x11\x72\xeb\x22\x1f\x8f\x7b\x44\x1c\x01\xe9\xa7"
      "\xec\xcc\xd0\xe2\xb5\x85\x00\x80\x23\xba\x68\xc6\x51\x4d\xce\xbc\x47\xea"
      "\x68\x23\x92\x55\xcd\xc4\x04\xaa\xe0\x09\x58\x33\x98\x9e\xac\xe6\x6b\xac"
      "\x75\x86\x35\x21\xa1\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xc7"
      "\xb3\x0e\xa1\x3b\xd5\xe3\xf5\x95\xe4\xc9\xdd\xa8\xff\x88\x4f\xb7\xc3\x29"
      "\xf9\xb8\x3a\x51\x1b\x5b\xd4\x6b\x1d\xc1\x09\x08\xd6\x83\x96\x95\x0f\x2c"
      "\xd9\x3c\xe9\x3b\x3a\xf1\x26\x83\x22\x79\xbe\x50\x57\x03\xf0\xf0\x62\x03"
      "\x14\x7c\xe1\xb1\x39\x07\xc6\xc4\x6b\x84\xfb\x70\xfe\xd7\x5f\x29\xf8\x11"
      "\x6f\x5a\x92\x5e\xf8\xfa\xa9\x3a\x46\x91\x2c\x31\x64\xd7\x84\xe6\x1f\xa9"
      "\x05\x66\xed\x50\x09\x57\x82\x04\x1e\xc4\xc0\x6f\x45\x28\x86\x4e\x75\x64"
      "\xce\x81\x8a\x0d\x99\x9c\xfa\x1c\x08\x5b\x0a\xe6\xae\x2e\x23\x24\x5f\x42"
      "\xf2\xe5\x00\x00\x00\x00\x00\x00\x00\x07\x73\x3a\xab\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
      2950);
  *(uint64_t*)0x20000088 = 0x870;
  *(uint64_t*)0x20000258 = 1;
  *(uint64_t*)0x20000260 = 0;
  *(uint64_t*)0x20000268 = 0;
  *(uint32_t*)0x20000270 = 0;
  syscall(__NR_sendmsg, -1, 0x20000240ul, 0ul);
  res = syscall(__NR_socket, 0x1000000010ul, 0x80002ul, 0);
  if (res != -1)
    r[2] = res;
  syscall(__NR_sendmmsg, r[2], 0x20000200ul, 0x4924924924926d3ul, 0ul);
  res = syscall(__NR_socket, 0x11ul, 0x800000003ul, 0);
  if (res != -1)
    r[3] = res;
  memcpy((void*)0x20000000, "\xa2\xe6\xfa\x9a", 4);
  syscall(__NR_setsockopt, r[3], 0x107, 0xf, 0x20000000ul, 0x31cul);
  *(uint16_t*)0x20000080 = 0x11;
  memcpy((void*)0x20000082,
         "\x00\x00\x01\x00\x00\x00\x00\x00\x08\x00\x44\x94\x4e\xeb\xa7\x1a\x49"
         "\x76\xe2\x52\x92\x2c\xb1\x8f\x6e\x2e\x2a\xba\x00\x00\x00\x01\x2e\x0b"
         "\x38\x36\x00\x54\x04\xb0\xe0\x30\x1a\x4c\xe8\x75\xf2\xe3\xff\x5f\x16"
         "\x3e\xe3\x40\xb7\x67\x95\x00\x80\x00\x00\x00\x00\x00\x00\x01\x01\x01"
         "\x3c\x58\x11\x03\x9e\x15\x77\x50\x27\xec\xce\x66\xfd\x79\x2b\xbf\x0e"
         "\x5b\xf5\xff\x1b\x08\x16\xf3\xf6\xdb\x1c\x00\x01\x00\x00\x00\x00\x00"
         "\x00\x00\x49\x74\x00\x00\x00\x00\x00\x00\x00\x06\xad\x8e\x5e\xcc\x32"
         "\x6d\x3a\x09\xff\xc2\xc6\x54",
         126);
  syscall(__NR_bind, r[3], 0x20000080ul, 0x80ul);
  memcpy((void*)0x20000180,
         "\xce\x04\x07\x00\x8b\x33\x50\xec\x00\x91\x1e\xfc\x0b\xb3\x5c\x02\x63"
         "\x0d\xff\xff\xa3\x28\xb4\x7c\xa8\xa8\x8a\x37\x87\x7b\x2b\x34\xe9\xff"
         "\x00\x00\x99\xdb\xe5\x47\xf4\x81\x70\x59\x24\xfa\xc3\x00\x00\x00\x00"
         "\x30\x96\x62\xbd\x84\x50\x11\x39\x9e\x97\x08\x00\xd8\x2b\x33\x0a\x7d"
         "\xa6\xd0\xed\xc5\x42\xcf\xf0\xc2\xd1\xe3\x27\xfb\x13\x28\x80\xf7\x0f"
         "\x9e\xa0\xee\xd8\x61\xc0\x35\x97\x19\x77\x1a\xaf\x54\xcf\x13\x2c\x4a"
         "\x68\x4a\x66\x9b\x62\x00\x00\x00\x00\x3c\xb4\xf1\x0e\x6f\xb6\xe9\x31"
         "\x41\x28\x76\x55\x1a\x46\xb4\xa0\xbd\x9d\x70\x73\x8b\x72\xdb\xc7\xdb"
         "\xae\xbf\xf1\xe0\xbb\xfd\x5f\xd1\x59\xc5\x49\xb5\xd3\x29\x84\x04\xb0"
         "\x6a\xb5\x99\xd9\x48\xfa\x87\x1b\xde\x41\x38\xdd\xfc\x42\x56\xdf\xa3"
         "\x67\x43\xb3\xc5\x08\x63\x2b\xef\x45\x31\x34\x6d\x82\xa2\x8b\xa3\x61"
         "\x2a\x9e\x26\x09\x5a\x14\x9b\xdb\xc9\xa8\x13\x6b\xc1\x41\xec\x1e\xb7"
         "\x93\x8d\x6d\xb2\xe8\x1e\xe8\xd6\x5c\x2c\xe5\x25\xbd\xf3\x96\x33\xfa"
         "\xca\x0a\x5f\x5a\x1e\xcc\x6e\x86\x23\x01\x4f\xe4\xc5\xbe\x6b\xd7\xdc"
         "\xbb",
         239);
  syscall(__NR_sendto, r[3], 0x20000180ul, 0x4e60ul, 0x810ul, 0ul,
          0xfffffffffffffe5dul);
  return 0;
}
