// https://syzkaller.appspot.com/bug?id=8b456d9f4b011cb98abfcc9264b55420ad6f9ab0
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#ifndef __NR_memfd_create
#define __NR_memfd_create 319
#endif

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

long r[15];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  memcpy((void*)0x20ee1000,
         "\x2f\x64\x65\x76\x2f\x6c\x6f\x6f\x70\x23\x00", 11);
  r[2] = syz_open_dev(0x20ee1000ul, 0x0ul, 0x2ul);
  memcpy((void*)0x20000000, "\xff\xf8", 2);
  r[4] = syscall(__NR_memfd_create, 0x20000000ul, 0x2ul);
  *(uint64_t*)0x20fe6fb0 = (uint64_t)0x2064d000;
  *(uint64_t*)0x20fe6fb8 = (uint64_t)0x0;
  *(uint64_t*)0x20fe6fc0 = (uint64_t)0x20e04f4f;
  *(uint64_t*)0x20fe6fc8 = (uint64_t)0xb1;
  memcpy((void*)0x20e04f4f,
         "\x69\x7d\x06\x0c\xf9\xd8\xbc\x6d\xf3\xc8\x51\x00\x11\x43\x8b"
         "\xa2\x4e\xa2\xb2\x26\x10\xaa\xb4\xd2\xb6\xdc\xe4\xf2\xaa\x43"
         "\xff\xa8\xa9\xfa\x8a\x9e\x94\x2b\x2d\x65\x90\xea\xf4\x9c\xf0"
         "\xe3\xfd\x23\x85\x81\x42\xe0\x29\x53\xdf\xc6\x5f\xde\x3b\xb8"
         "\x56\x5c\xcf\x35\xc5\x5f\xc2\x48\xd0\xad\x6c\x47\x3f\x76\x63"
         "\x55\xd2\x2a\x9d\x2f\x43\xf4\x49\x3c\xd5\x07\xf9\x19\xab\x87"
         "\x3a\xa3\xc9\xfb\x28\x8f\x4a\xe3\xa1\x34\xda\x84\x40\xae\xee"
         "\x57\xd6\x4a\x2a\x47\x14\x2d\x56\x92\x64\xdd\x9e\xc1\xfa\x49"
         "\x25\x33\xe8\x14\x53\xf1\xad\x1d\xae\x61\xd9\xbe\x13\xe0\x70"
         "\xac\x9d\x36\xf4\x33\xc0\xf1\x2a\xe8\xd5\x6a\xf7\xfb\x38\x01"
         "\xa8\x29\xa6\x26\xa8\x7b\x14\x23\x5f\x44\xc2\x86\x51\xbf\x99"
         "\xa7\x80\x82\x94\x96\x57\xf7\x4b\x04\x8a\xe1\xca",
         177);
  r[10] = syscall(__NR_pwritev, r[4], 0x20fe6fb0ul, 0x2ul, 0x80fcul);
  r[11] = syscall(__NR_ioctl, r[2], 0x4c00ul, r[4]);
  memcpy((void*)0x2034f000,
         "\x2f\x64\x65\x76\x2f\x6c\x6f\x6f\x70\x23\x00", 11);
  r[13] = syz_open_dev(0x2034f000ul, 0x0ul, 0x5ul);
  r[14] = syscall(__NR_fallocate, r[13], 0x11ul, 0x800ul, 0x44400004ul);
}

int main()
{
  loop();
  return 0;
}
