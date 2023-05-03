// https://syzkaller.appspot.com/bug?id=8b456d9f4b011cb98abfcc9264b55420ad6f9ab0
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

static void test();

void loop()
{
  while (1) {
    test();
  }
}

#ifndef __NR_memfd_create
#define __NR_memfd_create 319
#endif

long r[22];
void test()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  memcpy((void*)0x20000000,
         "\x2f\x64\x65\x76\x2f\x6c\x6f\x6f\x70\x23\x00", 11);
  r[2] = syz_open_dev(0x20000000ul, 0x0ul, 0x2ul);
  r[3] = syscall(__NR_fallocate, r[2], 0x11ul, 0x0ul, 0x44400004ul);
  memcpy((void*)0x20fb6ff5,
         "\x2f\x64\x65\x76\x2f\x6c\x6f\x6f\x70\x23\x00", 11);
  r[5] = syz_open_dev(0x20fb6ff5ul, 0x0ul, 0x2ul);
  memcpy((void*)0x20346000,
         "\x2f\x64\x65\x76\x2f\x72\x66\x6b\x69\x6c\x6c\x00", 12);
  r[7] = syscall(__NR_memfd_create, 0x20346000ul, 0x2ul);
  *(uint64_t*)0x20fe6fb0 = (uint64_t)0x20e3e000;
  *(uint64_t*)0x20fe6fb8 = (uint64_t)0x0;
  *(uint64_t*)0x20fe6fc0 = (uint64_t)0x20fa2000;
  *(uint64_t*)0x20fe6fc8 = (uint64_t)0x92;
  *(uint64_t*)0x20fe6fd0 = (uint64_t)0x20c0c000;
  *(uint64_t*)0x20fe6fd8 = (uint64_t)0x0;
  *(uint64_t*)0x20fe6fe0 = (uint64_t)0x20adbfdd;
  *(uint64_t*)0x20fe6fe8 = (uint64_t)0x0;
  *(uint64_t*)0x20fe6ff0 = (uint64_t)0x2010279f;
  *(uint64_t*)0x20fe6ff8 = (uint64_t)0xac;
  memcpy((void*)0x20fa2000,
         "\xe5\x1d\x66\xe0\x35\x97\x8e\x51\xb9\x88\xb6\x56\x62\xee\x35"
         "\x8d\xeb\x77\x85\xb6\x68\xba\x77\xec\x71\x71\x7a\xac\x2f\x67"
         "\xc6\x41\x77\x2d\x3a\x4b\x00\xd1\xdf\x92\xe2\xdd\x49\xb6\x22"
         "\xcb\x9f\xf5\x3e\x66\x71\x7c\xf2\xe5\xb8\x28\xcb\x70\xe3\xeb"
         "\x72\xff\x51\xb6\xfc\xcc\x1b\x10\x92\x40\x20\x52\xab\x2a\xa2"
         "\x56\x29\x72\x96\xda\x9f\x1e\x9a\x34\xc7\x73\x1a\xbb\x0e\xde"
         "\x9f\x54\x6e\x6c\x05\x0f\x93\x15\x05\xb1\x00\xa1\x9f\x44\x3e"
         "\x90\xe9\xd1\x6c\xab\x2b\x9c\xb1\x71\x02\x3d\x6b\x65\x94\x5d"
         "\x52\x18\xe4\xb6\x00\xf9\x28\x99\x15\x8a\x41\x40\x51\x21\xdd"
         "\x90\xd2\x0b\xd1\x06\x3e\xe4\xc1\xa7\x07\x25",
         146);
  memcpy((void*)0x2010279f,
         "\x31\x81\x06\x07\x95\x90\xd3\x9b\x8f\x09\x65\x04\x2c\x8e\xca"
         "\x0d\xc8\x74\x0d\x85\xf6\xf0\x03\xcc\x40\xfc\x35\x0e\x53\xc3"
         "\x84\x81\x6d\x03\x28\xd6\xcb\xae\x79\x6b\x57\x7b\x4e\x51\xc3"
         "\xaf\xf8\x74\x34\xf1\x95\xcd\xf4\x84\xd9\x29\xfa\xc1\x77\xfd"
         "\xf6\xbc\x5a\x06\x37\x75\x98\xaf\x2f\xea\x4c\xa6\x3a\x53\x6a"
         "\x5c\x32\x2c\x96\xcd\xb1\x56\x9e\x14\xd2\xb5\xbb\x43\x99\xa4"
         "\x26\x15\x79\x94\xeb\x9a\x64\x95\x9f\xf8\x2f\x33\xb7\x4f\x2e"
         "\x1a\x4d\xa7\x52\x87\xc8\xf2\x24\x01\x01\xc0\x30\x0b\x1a\x0b"
         "\x25\x1b\x26\x04\x9a\x47\xbc\x8b\x9b\xc4\xc6\xd7\xde\x11\x7d"
         "\xd6\x35\x7a\x0a\x30\xab\xc4\x95\x61\x3b\xd0\x58\xb9\x7a\x5b"
         "\x52\xc0\x1a\x12\x16\xe4\x1c\x9d\xe1\xed\xe2\x9f\xb9\x52\x06"
         "\xa5\x3e\x5b\x91\xf1\xdb\x21",
         172);
  r[20] = syscall(__NR_pwritev, r[7], 0x20fe6fb0ul, 0x5ul, 0xfcul);
  r[21] = syscall(__NR_ioctl, r[5], 0x4c00ul, r[7]);
}

int main()
{
  loop();
  return 0;
}
