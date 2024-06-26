// https://syzkaller.appspot.com/bug?id=f82ab89451323208e343f4a8632014ef12b1252d
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

#ifndef __NR_ioctl
#define __NR_ioctl 54
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_openat
#define __NR_openat 295
#endif
#ifndef __NR_write
#define __NR_write 4
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000, 0x1000, 0, 0x32, -1, 0);
  syscall(__NR_mmap, 0x20000000, 0x1000000, 7, 0x32, -1, 0);
  syscall(__NR_mmap, 0x21000000, 0x1000, 0, 0x32, -1, 0);
  intptr_t res = 0;
  memcpy((void*)0x20000180, "/dev/fb0\000", 9);
  res = syscall(__NR_openat, 0xffffff9c, 0x20000180, 0, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x2000000c = 0;
  *(uint32_t*)0x20000010 = 0;
  *(uint32_t*)0x20000014 = 0;
  *(uint32_t*)0x20000018 = 4;
  *(uint32_t*)0x2000001c = 3;
  *(uint32_t*)0x20000020 = 0;
  *(uint32_t*)0x20000024 = 0;
  *(uint32_t*)0x20000028 = 0;
  *(uint32_t*)0x2000002c = 0;
  *(uint32_t*)0x20000030 = 0;
  *(uint32_t*)0x20000034 = 0;
  *(uint32_t*)0x20000038 = 0;
  *(uint32_t*)0x2000003c = 0;
  *(uint32_t*)0x20000040 = 0;
  *(uint32_t*)0x20000044 = 0;
  *(uint32_t*)0x20000048 = 0;
  *(uint32_t*)0x2000004c = 0;
  *(uint32_t*)0x20000050 = 0;
  *(uint32_t*)0x20000054 = 0;
  *(uint32_t*)0x20000058 = 0;
  *(uint32_t*)0x2000005c = 0;
  *(uint32_t*)0x20000060 = 0;
  *(uint32_t*)0x20000064 = 0;
  *(uint32_t*)0x20000068 = 0;
  *(uint32_t*)0x2000006c = 0;
  *(uint32_t*)0x20000070 = 0;
  *(uint32_t*)0x20000074 = 0;
  *(uint32_t*)0x20000078 = 0;
  *(uint32_t*)0x2000007c = 0;
  *(uint32_t*)0x20000080 = 0;
  *(uint32_t*)0x20000084 = 0;
  *(uint32_t*)0x20000088 = 0;
  *(uint32_t*)0x2000008c = 0;
  *(uint32_t*)0x20000090 = 0;
  *(uint32_t*)0x20000094 = 0;
  *(uint32_t*)0x20000098 = 0;
  *(uint32_t*)0x2000009c = 0;
  syscall(__NR_ioctl, (intptr_t)r[0], 0x4601, 0x20000000);
  res = syz_open_dev(0xc, 4, 0x15);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x20000240 = 8;
  memcpy(
      (void*)0x20000244,
      "\xb9\x83\x62\xc6\xa3\x77\x87\xd0\x27\x9b\xc9\xf7\x89\x02\x74\x4a\x81\xbb"
      "\xe2\x85\xb4\xf3\x5a\x80\x79\x0d\x33\x35\xf1\x60\xcb\xb1\x58\xbd\x3e\x8b"
      "\x08\x0d\x12\xf9\x23\x3d\xb5\x61\xe6\xff\x18\x83\xa0\xd2\x8a\x5b\x57\xe9"
      "\xab\x0e\x47\x83\xd8\xbc\x8b\x20\x55\x2f\x8a\x25\x2b\xaf\x20\xdb\x6b\x77"
      "\x7e\x69\xc7\x2d\xe8\x31\x8a\xb6\x70\x66\xbc\x54\xbb\x2c\x60\xbc\x55\x64"
      "\x1b\xdd\x56\xd8\xa3\x43\x48\xd8\xe9\xe2\x71\x9c\x75\x27\xde\x77\xf8\x74"
      "\x08\xdf\x66\xc6\xcc\xaa\x37\x95\x30\xdc\xe7\xf3\x7a\x49\x2a\x1f\x47\x63"
      "\x53\x88\xff\x79\xc3\xb2\x2c\x24\x18\x20\x67\xff\xa4\xa3\x7e\x12\x69\xb4"
      "\xd0\x1d\xbd\x62\xc3\x62\x1d\x8e\xfb\xcf\x53\x0b\x73\x74\xd0\x6e\x04\x56"
      "\x29\xdf\x88\x3d\xb0\x1e\x1e\xe6\x03\x60\x03\xf7\xcb\xb4\x27\x46\x80\x44"
      "\xe9\x27\x49\x9a\x5e\x3f\x43\x17\x0e\xd7\xba\x38\xe4\x8c\x8a\xae\xc5\x46"
      "\xa4\xb7\x88\x2e\xd5\xe3\x7f\xb8\xfc\x97\xf5\xf1\x63\x6b\x5d\x1d\x52\xad"
      "\x3c\x87\x56\x93\xda\x5e\x41\x56\xb1\xfd\xac\x50\x8e\xf1\xc3\x1a\x58\x12"
      "\x61\x8e\x17\x81\x59\xbe\x8e\x0f\xb4\x2e\x6a\x39\x3a\x67\x89\xa9\xfb\x9f"
      "\x6f\x32\x96\x9d\xda\xe8\x43\x06\xa5\xad\x5b\xb8\x00\xd2\x15\x41\xf7\x72"
      "\x9d\x66\xb1\x51\x3d\x30\x5f\xdb\x3e\x00\xf9\x37\x37\xdb\x46\x33\xbd\xbe"
      "\x1b\x38\xde\x2b\xeb\xd7\xee\xb8\x51\x20\xf6\x9b\x55\x58\xbd\x24\x26\x7f"
      "\xdb\xeb\xcd\x23\x81\x39\xf9\x6c\x42\xb3\x75\x7e\xc1\x96\x14\x21\x9e\x40"
      "\x4f\xb9\x13\xed\x43\x74\x0e\xed\x4d\xd7\xe7\x37\x0c\x97\xfb\xde\x2c\x57"
      "\x8b\x8c\xb5\x23\x4e\xad\x9e\x30\xbf\xeb\x3b\xb8\x5c\xb9\x03\xd4\xcc\xc3"
      "\x86\xc0\xef\x59\x11\x20\x13\xe8\x4c\xff\xdd\x74\xf6\x8c\xbd\xd3\x1a\x1a"
      "\x1f\x3c\x94\xfe\x4f\xee\x36\x64\xae\x80\x5c\xa7\x65\x9d\x11\x20\x16\xc4"
      "\x4e\x8d\xa1\x40\x95\x6b\xd7\x30\xc8\xef\xaf\x68\xcf\x1a\xd1\x4c\x57\xaf"
      "\xee\xc3\x35\x72\x6d\x42\x09\x37\xf0\xa5\x56\x94\x1d\xdf\x97\x45\xcc\xab"
      "\xaf\x80\xb2\xda\x62\xf5\x2a\x38\x25\xa9\x5f\xf4\x50\x02\x9e\xb5\x98\x09"
      "\xc6\xd9\xab\x19\x0c\x4a\xbc\xb0\x4f\x41\x97\x33\xaa\x32\x2f\xe3\x13\xd3"
      "\xa1\x5a\xbb\x23\x9a\xcc\x18\x59\x8e\xb4\x39\xfb\x41\x6a\x12\x7d\x22\xd9"
      "\x07\x52\xc4\xd0\xfd\xb3\x17\xaf\x3b\x32\x42\xb1\x4b\xe3\x46\xae\xfb\xc8"
      "\xa6\x79\x32\x29\x9a\x95\x07\x4d\x54\xc6\x77\x28\x4a\x15\xfa\x2a\x71\x98"
      "\xe5\x3b\x89\x74\xa0\x5e\x2e\x26\xd3\x7f\xf7\x56\xb3\x0c\xb4\x08\x76\xad"
      "\xf6\xf0\x73\x7d\xc8\x78\x58\x10\xb1\xd9\x3e\xd8\x3a\xed\xff\x5b\x29\x20"
      "\x12\x6b\x0b\x9e\x3b\xfd\x3a\x37\x4a\xdd\xe3\xa1\x03\xb8\x32\xfb\x84\xea"
      "\xd8\x2d\x21\x05\x18\x35\xef\x2d\x8e\x36\x4d\x48\xe6\xa1\x1d\x35\x4b\x4d"
      "\x0e\x35\x4b\xcf\x6e\x50\xb3\x5d\x54\xd6\xef\x8d\x1a\xa0\xba\x6f\x5e\x9e"
      "\x55\xb3\x27\xb3\x27\x7e\x1a\xd8\xb0\x13\x7a\x41\xa7\x1d\x3d\x11\xad\x5d"
      "\x55\x45\xd0\xb9\xf5\xe1\xba\xdc\xd1\x6c\xa5\x8a\x8b\xe6\x82\x09\x86\xe1"
      "\x11\xd0\x7c\x05\x3c\xac\x81\xef\xc1\xc0\xbf\x06\x6a\x07\xb6\xac\x21\xea"
      "\xdc\xa2\xe5\xa3\xbd\x2c\x66\xc1\x1b\xf7\x0c\x33\x67\x2e\xd1\x96\x47\xba"
      "\x7d\xe2\x65\x5f\x29\xf2\xc7\x29\x6f\x6a\xf3\x45\x0a\x72\x72\x74\xb8\x3a"
      "\x89\x15\x83\x98\x62\x24\x76\xd7\x1c\x09\xaf\x1e\x1e\x68\xd4\xb7\xc8\x6a"
      "\x5f\xcf\x9e\xf7\xb6\x93\x0d\x87\x94\xe7\xd0\x96\xa5\xea\xbb\xa9\xac\xf3"
      "\x95\x40\xf8\xba\x7b\x23\x23\x3f\x70\x96\x6b\xf8\x69\x09\x18\xff\xcd\xd5"
      "\xe0\x3f\x67\x02\x21\x18\xdb\x70\x28\xe8\x1d\x40\x88\x6c\x91\xd4\x21\xf6"
      "\xcc\x19\xaf\xaa\xd9\x36\x21\x66\x49\x2e\x93\xd4\x0a\x61\x4f\x94\xaf\x59"
      "\x6a\xb5\xaf\xf3\x13\x43\xac\x85\xeb\xb8\xd4\x98\xc6\x9b\x0d\x3b\x3b\x8a"
      "\xec\xf5\x65\xf2\x87\x69\x6e\x2b\xe3\xb5\x2e\x04\x87\x7b\x60\x90\x80\xd9"
      "\xb9\x6a\x77\xc3\x68\xb7\xee\xc9\xef\xfe\x52\xa2\xf0\x44\x30\x1a\x3a\x8b"
      "\xb9\x12\xfa\x8d\x5b\x8c\x3e\x52\x9c\x79\xd7\x89\xd6\xbc\x7a\x7d\x3e\xf7"
      "\x1e\x8f\x54\xb4\xe2\xa7\x88\x6f\x22\xc8\x9f\x4c\x48\x8d\xf3\xcb\xf2\xf7"
      "\x9f\xbe\x79\x93\xb4\x21\xea\x5f\x5a\x61\x0b\x6d\xd1\x96\x7a\x85\x49\x28"
      "\xe8\x9c\x16\xa6\x65\xc7\x55\x4b\x39\x78\x01\xf8\xec\x81\x76\x59\x09\x03"
      "\x43\x7f\x4d\x53\x83\xf2\x83\xfe\x43\x96\x10\x60\x01\x0e\xbf\x7d\xf5\xfa"
      "\x21\x2f\xd7\x29\x7a\xcc\xdb\xca\xc4\x61\x9f\x47\x4f\x98\xbf\x1c\x52\x1f"
      "\x44\xe6\xea\x3c\xce\xa1\x17\x85\xac\xca\xfd\x3b\x2e\x7a\x05\xf9\xcc\x83"
      "\x94\xcc\x23\xd5\x14\x61\x0d\x59\xb0\x34\xab\x37\x23\xce\x5a\xb2\x66\x23"
      "\xa6\xff\xce\x0e\x50\xfa\x9f\x1b\x31\x1e\xa7\xaa\xac\x1e\x24\x47\x2a\x51"
      "\x14\x83\x23\x5d\x2f\xa8\x22\x4a\x3d\xf5\x8b\xc7\x45\x59\xbb\x9e\x7c\x85"
      "\x41\xd7\x0d\xd0\xe5\xbe\x97\x3f\xf8\x73\xb6\xf4\x94\xcd\xc0\x3b\x47\xf3"
      "\xc9\x4b\x2e\xf2\xc3\x0e\x9f\x2a\x38\xbf\x1b\xe1\x38\x5a\xae\xb6\x7b\x5c"
      "\xdf\x6c\x0a\xb9\xf6\x46\x26\xbd\x76\xdc\x68\xff\x8c\x76\xf3\x82\xbf\xa8"
      "\x08\x53\xe3\x7b\xa9\x9e\xee\xc9\xd4\x81\xa8\x06\x8f\x36\x56\x4c\x4b\x5d"
      "\x44\x98\x88\xf5\x9d\xba\x75\xb4\x9b\x35\x77\xea\xc7\xf9\x1b\x22\x6d\xca"
      "\xaf\x41\xd8\x0e\xd8\x72\xb5\xa6\xa1\x39\x7d\x0e\x12\xbc\xc5\x5f\x31\x1b"
      "\xdc\x6d\x1d\xf1\x0f\x4b\x0a\x96\xb4\x3c\x78\x98\x75\x95\x3c\xbc\x0a\xbb"
      "\x43\x4a\x08\x50\x98\xb7\x6c\xee\x0f\xe9\xba\x84\x82\x1d\xd9\xdd\x34\xb5"
      "\x82\x84\x5f\x32\x79\xf9\x48\x69\x80\x87\x46\xbd\x62\xda\x25\x0d\x87\x8e"
      "\x8e\xf8\x31\xd1\xb6\x9b\x9b\x6a\x93\xa0\x33\x8e\xe4\x1f\x96\x4d\xde\x2d"
      "\xcd\xa8\x57\x76\x05\xc7\xb6\x94\x6d\x95\x39\x23\x00\xf4\xfb\x76\x4b\xe7"
      "\x9b\x6c\xf8\xf4\xfa\x64\x7e\x75\xbf\x54\x96\x59\x28\x53\x81\xfe\x33\xfb"
      "\xda\x86\x60\x66\x38\xc9\x60\xa0\x47\x8f\x63\x36\x15\xbc\x07\xa0\xd4\x97"
      "\x5e\xe8\x13\x67\xc0\x8d\x14\xba\x02\x56\xc7\x52\x09\x83\x4d\x91\xc2\xe2"
      "\x73\x91\x12\x30\x7b\x30\x91\x97\xd6\xf3\xe1\x85\xaf\x08\xaa\xab\xb9\xe4"
      "\x90\x43\xae\xb1\x42\x01\x15\xef\x31\x09\xb3\xb0\xae\x3f\x0a\x17\x66\x23"
      "\x5f\xbe\xcb\xe6\x54\xde\x10\xe1\x76\x73\xe1\x00\x29\x7a\xb1\xd6\x51\x64"
      "\x49\x88\xe3\x73\x53\xda\x50\x9c\x31\xe2\x67\x23\xcf\xfd\x3f\x84\x31\x6e"
      "\xa5\xe3\xe7\x78\xe9\x4e\x73\x13\xa8\x06\xc8\xa2\x57\x89\x46\xf6\x38\x78"
      "\x64\x26\xce\x6c\x80\xa6\x36\x26\x27\x06\x88\xbc\x2b\x43\x04\x76\xa3\xf5"
      "\x58\x8e\x30\xf9\x70\x6c\xe3\xe3\xd3\x05\x3b\x51\x93\xe7\xc3\xc1\xb0\xd9"
      "\xa3\xde\xa0\xc7\x26\xf2\x89\xd5\xe7\x18\xd1\xf0\xc7\x64\x8e\xda\x88\xc3"
      "\x25\x79\xe7\x27\x00\x37\x54\x1f\x32\xcc\x7f\x8f\xc3\x74\xee\xf9\xe1\x42"
      "\x94\x23\x7f\x72\x82\xb1\xd5\x69\x3e\x78\x84\xba\x13\x00\xd0\x5d\x67\x81"
      "\x99\xc6\xe1\x2b\x72\x08\x5a\x8c\x58\x16\xe7\xd2\x98\x51\x1e\x39\xd3\x79"
      "\xf5\x66\x9d\xf0\x70\xf6\xc2\x4e\x55\xaa\x5f\x60\x8d\xd8\xe1\x79\x87\xaf"
      "\x98\x36\x47\xf6\xce\x04\x62\xcc\xfa\xf8\xfa\x08\x7d\x7e\xf5\x3e\xfa\xe7"
      "\xc6\x89\x46\x13\xfe\x61\x5e\x24\x4b\xd6\xf9\x50\x93\xa6\xb6\x63\x0f\x8b"
      "\x92\x58\x95\xa1\x7e\x24\xc7\xc2\x9c\xd3\x37\xf8\x50\xe8\x82\xe1\xad\xc7"
      "\x5e\x9d\xac\x38\xf2\xb2\x77\xca\xc9\x50\xb3\x28\x13\x25\x6c\x99\x45\x78"
      "\x4c\xd0\x6a\x3d\x0e\xac\xec\x70\x7b\x1b\xe8\xfd\x8c\x48\x7c\xd1\x15\x1f"
      "\x79\x74\x7d\x10\xf2\x77\x1d\x78\xad\x6b\x93\xdd\x97\x87\x00\xf8\x89\x01"
      "\xd1\xf0\x7f\x7b\xa7\x8a\x1e\xff\xf9\x75\xe3\x5c\x34\x7c\xa6\xf3\x5b\x6f"
      "\x2c\x53\xb8\xe3\xd8\x72\x40\x2f\xa8\x2e\xb1\x25\x8c\x0a\x79\x64\x59\x2b"
      "\xb5\xd0\xb7\x5c\x9a\x85\x61\x14\x74\xf6\x61\x6e\xfa\x52\x07\x5a\x36\xc7"
      "\x84\x42\x20\x91\x4e\x4c\xda\xa8\xc6\x91\x99\x4f\x62\x1b\xbd\x9d\xa1\xba"
      "\x08\xd7\x96\x1a\xba\x08\xc7\xb2\x69\x7d\x57\xa7\xc0\xda\x44\xaa\xdf\x38"
      "\x79\x76\x8d\xf9\xfb\x90\x2e\x8f\x6d\xab\x6b\x7e\x2a\xc3\x28\x4b\x6a\x8c"
      "\xb4\x56\xf6\x2f\x58\xf6\x5d\x49\xaa\xf1\x8c\xac\x43\x34\xfb\xec\x0e\xa9"
      "\xcf\x1d\x28\x3b\xff\x5f\x39\x84\xc5\x20\xec\x89\x31\xc3\x3f\xb5\x42\x2d"
      "\x84\xde\x0f\xaa\xa3\x15\x88\xe6\x8c\xff\x0a\x5f\x66\xa3\xab\x96\x29\x7a"
      "\x31\xf4\x9d\xa7\x1f\x02\x7a\x50\x21\x3b\xa9\x2f\xae\x7d\x8d\xdc\xb1\x6f"
      "\x38\x42\x50\x46\x11\x8c\xd1\x61\x3b\x84\x32\xd2\x65\x88\x49\xc4\x77\x6c"
      "\xfe\xab\xf5\x25\xf9\xc9\x30\x63\x43\x5d\x0f\x7a\x01\x05\xa0\xa7\x5e\x87"
      "\xa9\x63\x5b\xcb\x33\xe6\x5b\xf2\xa7\x72\x41\x14\x38\x4f\x95\x0b\x1a\x66"
      "\xfe\xc3\xed\x0b\x25\x01\x96\x36\x00\x6e\x6f\xaf\x16\xc2\x83\xec\xaf\xb5"
      "\x9a\x76\xbf\xc0\x03\xe0\x46\x08\x33\x25\x18\x81\x02\xf2\xc5\xaa\xef\xc0"
      "\x86\x9d\xbb\xd4\x46\x46\xce\xb0\x0d\x6b\x91\xbc\x57\x57\xed\x47\xac\x22"
      "\x23\xec\x07\x03\x46\x99\x68\x87\x7a\xea\xda\x61\x13\x42\x69\x3b\xbb\xbc"
      "\x66\x6e\x1a\xe6\xd9\x6b\x0d\x48\xc5\x6b\x2c\xdb\x1a\xc0\x2d\xc5\x38\x5c"
      "\x38\x46\xa8\xfb\xb6\x02\x91\x77\xc6\x8b\x7f\x17\xf5\xe3\xd7\x45\x8e\x10"
      "\x78\xf2\x56\x3a\xba\xfe\x3d\x89\x06\x07\x37\x9a\xeb\xfd\x8a\xa8\x53\x2e"
      "\xf9\x53\x4b\x22\x7d\x55\x9c\x52\x19\x74\x86\xa6\x23\xd0\xa5\xf8\x2f\xa8"
      "\xf2\xa0\xb2\xdb\x5c\x7f\xec\x84\x3a\x61\xc0\x4d\xe7\xe5\x3a\x74\x2d\x6c"
      "\x58\xf2\x8c\xa3\x23\x64\x8c\x93\x47\xfd\x46\x76\xf9\x8f\x68\xd1\x8b\x66"
      "\xa8\xcc\x02\x6c\xf6\xe0\x9b\x6a\x79\x17\xc1\x14\xc3\x2f\xd9\x4d\xcb\xab"
      "\xcd\x7e\x99\x64\xc0\x1d\x37\xe3\x76\x68\x49\x03\x7d\x3e\x52\x2d\xef\x0f"
      "\x8e\x87\xd8\x76\xbb\xd4\x00\x24\x8d\xa1\xbf\xa3\x06\xcb\x98\x47\xb3\xbc"
      "\xcd\xd2\xf9\x57\xc1\xdc\xf8\x91\x1a\x4e\x9c\x17\xd7\x05\xe0\x9f\xf5\x11"
      "\x9e\x21\xd5\xfe\x0e\xbb\x2b\xc1\xbe\x3d\x6c\x2f\x21\xd9\x4b\x7f\x45\x63"
      "\x0f\x7d\x32\xf9\xbc\xe7\x14\xa3\x80\x8c\x74\xfa\x55\x35\x38\x5c\xd6\x82"
      "\x4e\x0e\xd9\xe6\x4d\xb9\x11\x97\x87\xfd\x6b\x43\xb0\xab\xd9\x98\x93\x44"
      "\xce\xb6\x82\x22\x70\xf3\xcc\x97\x2f\x50\x91\xba\x33\xef\x59\x04\x90\xe0"
      "\xf9\x23\xec\xec\x3d\xba\x2c\x59\xbf\xd4\x21\xc0\x63\x26\x32\x7e\x7f\x77"
      "\xe8\xa8\x2c\xe1\x89\x03\xe3\x66\xb4\x89\x19\xda\x8a\xa5\x4f\x97\xd1\x1f"
      "\x3d\x56\xd5\xe6\xb1\x3b\xd8\x4d\x89\x92\x70\x4f\x8c\x51\x08\x74\x5f\x7e"
      "\xcf\xac\x0f\x71\x67\x3a\x96\xee\x7a\xe4\x7c\x62\x77\x64\xed\xa4\xe0\x63"
      "\x1a\x93\x5d\xa1\x81\xfd\x47\x09\x5f\x15\xb7\x72\x9f\x3c\x77\xda\x2d\x3f"
      "\x7e\x90\xbd\x67\x97\x14\x4a\x56\xd2\xf3\xde\x70\x28\x44\x90\x07\x76\x88"
      "\xc0\x07\x15\x1f\x83\x26\xf2\x85\x7f\x49\x30\xe8\xb7\x4e\x4d\x90\xaa\x22"
      "\xda\x53\x19\x12\x4a\x5a\x14\xb7\xbb\xe0\x14\x59\x4f\x3b\x8b\x50\x29\xd4"
      "\xde\xf9\x3b\x3b\x29\x05\xe2\xea\xe0\x5f\x2e\xe1\xf4\xfe\x59\x74\x6e\xab"
      "\x24\x4c\x59\x39\x78\x65\x22\xfe\x08\xfc\x11\x9b\x4b\x8e\xc0\x2f\x07\x98"
      "\xa6\xed\x40\x93\xf2\xe5\xe0\xf2\xa0\x38\x47\x5d\x7b\x21\xf6\xe6\xb0\x83"
      "\xb4\x7e\xf8\x2a\x20\xdf\x5e\x74\x5f\xd6\x57\xe8\xb7\x9d\x23\xbf\x80\x6a"
      "\x81\xbf\xb4\x31\xb8\xdb\x29\x6a\x65\xa1\x7f\x31\xfb\xad\x3d\xa6\x7b\x49"
      "\xe9\xa8\xe4\x8a\x93\x0b\xb8\xb2\x41\x5a\x5b\xfe\x6a\x6a\xef\xa5\xd1\xd3"
      "\x61\xc4\xa7\x12\x14\x92\x9c\x72\xf9\x6d\x3e\x8d\xd7\x3b\x1e\x8f\xbb\x8d"
      "\xea\xec\xd3\xa1\x96\x2e\x01\x76\x09\xc2\x65\xda\x3a\x20\x83\x50\x32\x58"
      "\x7f\x16\xad\xe8\x16\x9b\x91\xa0\xc6\x67\x53\x63\x3e\xb4\x51\x49\x66\x53"
      "\x78\x17\x28\x70\x7b\xee\x67\xb4\x2e\x9e\xf8\x79\x52\xf6\xed\x48\x26\x4f"
      "\xbb\x2d\x2f\xdd\x18\xa1\x2d\xbf\x92\x84\x35\x26\x06\xa0\x4e\x7e\x68\x34"
      "\x3e\x48\xeb\x95\x7e\x38\x03\x95\x3f\xb0\x45\x1b\xd1\x1d\xe1\xba\x22\x50"
      "\x45\x3a\x87\x43\xfc\xea\x42\xb2\xd5\x77\x1c\x0a\x50\x17\xb7\xd1\x40\x46"
      "\x0a\x08\x00\x87\x93\x95\x36\x78\x09\xca\xca\x0e\x59\xa8\xdb\x0e\x89\x07"
      "\x8e\xe7\xac\xe0\x5a\x51\xac\x35\x58\x9f\x5f\x0e\x0e\x0d\xac\x9b\x5d\x08"
      "\xa5\xc5\x26\xf7\x6d\xc1\xe1\x4c\xd6\x87\xe3\x10\x07\x21\x82\xa7\xd9\xb6"
      "\xf4\x5a\x2f\x5f\xae\x1a\x86\x60\x4b\x5d\x72\x36\x68\x69\x8e\xd7\x32\xe3"
      "\x94\x93\x2c\xed\xeb\x87\x09\xeb\x75\x49\xc4\x2c\x9e\xef\x2a\x7e\x8e\xec"
      "\xc7\x4b\xee\x2e\x95\xb9\x3f\xe3\x0f\x37\xc1\x2a\x82\x25\x66\x51\x02\xe4"
      "\xdc\xb7\x4d\xf8\x27\xfe\x34\x36\x1a\x7c\x97\x54\xb1\x0e\xb9\xd4\x23\xf8"
      "\x7b\x7b\x6e\x76\x4a\xa3\x2d\x99\xd7\x51\xf9\xf5\x17\x28\xd0\x63\xc1\xb0"
      "\x82\xc9\x64\x7c\x12\x44\x2a\xec\x97\x7c\x11\xee\x7e\xdd\xaf\xad\x39\x2d"
      "\x4a\xe7\x4f\xa9\x2a\x3f\xd0\xb8\x07\x4c\xf6\x4e\x26\x74\x57\x6d\x1a\x3f"
      "\x38\x46\xfb\xff\xa2\x22\xe3\x6c\xd1\xc9\xfc\x9c\xea\xa1\x72\x18\x9a\x9e"
      "\x03\x77\xea\xff\x15\x93\xb4\x2c\x99\x41\x4d\xc1\xe2\xc7\x30\xd4\x18\x5e"
      "\x3c\xf3\xe6\xea\x7d\xc1\x98\x4b\x90\x87\xc3\x22\x64\x2e\x4a\xf3\x8d\x8c"
      "\x38\x95\x8e\xfe\x1d\x0c\x55\xe2\x5e\x36\x13\xbf\x28\x2d\xc3\xfe\xff\x6b"
      "\xbd\x06\xb7\xfa\xc4\xa1\x1e\x86\x8f\x91\x49\xa6\x97\x59\x61\x25\xfe\x69"
      "\x0d\x19\x94\x94\x9a\x70\xd7\x20\xe7\x3f\xed\x69\x49\x9a\xbb\x69\x1a\x43"
      "\x49\x1f\xf5\x08\xcc\x29\x42\xd1\xb5\x9a\xda\x38\x02\x7a\xa2\x6c\xaf\x2e"
      "\x98\xa4\x12\x05\xdb\xde\x6a\x0c\xfd\x58\x26\x03\xc7\x61\xa0\xc5\x88\x1a"
      "\xbb\x24\x0c\xc6\x4d\x0e\xcf\xd6\x5e\x17\x68\xc2\xe6\x33\x3d\x7e\x88\x11"
      "\xfe\x24\x69\x13\xdd\x5c\xd6\xa2\xd6\x56\x7b\x1f\x6b\x9e\x7f\xb8\x02\x74"
      "\xd9\xa2\xa3\x82\x2b\x02\x88\xe5\xf4\xca\x33\x39\x0c\x9f\x23\xbd\x60\x8f"
      "\x57\xe5\xcf\xbd\x16\x7c\xda\x1c\xfc\x8d\xab\x88\x27\xbc\xa1\x0b\x47\x31"
      "\x39\x46\xf7\x84\x77\x7f\x12\x95\x32\xee\xc9\xde\xea\xa0\xe3\x57\xbb\xc3"
      "\x48\xa2\xa3\x23\x01\xac\xda\x8e\x65\x20\xd2\x47\x00\xc6\x6d\x76\x08\xcf"
      "\x8d\x97\xa1\xa9\xbe\xa6\x91\xe1\xdd\xf3\x77\x88\xb7\x77\x0d\xda\xa6\x10"
      "\x49\x43\xaa\x9d\xe8\x7b\x31\x9d\xd0\x21\x12\x4c\x13\xe6\x45\xcf\x2c\x05"
      "\x24\x53\x51\x45\xd2\x45\xfe\x7f\xfe\xa3\xd2\x6a\x15\xaf\x86\x89\xda\x44"
      "\x66\xb4\xd0\x35\x60\x25\x78\x55\xfc\xb7\xda\x4b\x14\xad\x0e\x37\xcf\xe0"
      "\x21\x8e\x02\x42\x57\xf3\xd8\x58\x2d\xd9\x32\x91\x8f\x92\xd6\xd5\x16\xd9"
      "\x9a\x90\x0b\x83\x6e\x0a\x5d\x02\x55\x6f\xd0\xf6\xe4\xc2\xbd\xbf\xca\xdd"
      "\x13\x65\x1d\x3b\xb7\x8c\x0d\xed\x66\xc4\x68\x0d\x8e\xa8\x96\x78\x78\xd3"
      "\x0a\x47\x02\xdb\xa3\x2a\x2f\xd3\xca\x91\x4f\x1e\xcf\x18\x81\xed\x0f\x21"
      "\xa4\xb3\xc3\xd8\x55\x58\x20\xad\xfb\x4c\xc6\x20\xa9\x6c\x3d\x4e\xa5\x9d"
      "\xaf\xcb\xcc\x90\x9b\x29\xd1\x9e\x26\x87\xa8\x9f\x5d\x72\xcb\xe6\xba\xd0"
      "\x13\xbf\x8a\x08\xee\x33\x22\x37\x05\xb2\xb1\x5f\x5e\x09\x07\x18\x7f\xda"
      "\x09\xae\xaa\x31\x41\x03\x91\x3a\x10\x34\x81\x52\xd3\xd8\xff\x32\x97\x90"
      "\x7f\x42\xdb\x2d\x6c\xb2\xcd\x1e\x9e\x5c\xc4\xd6\x43\x9f\xe2\x56\x1b\x4f"
      "\x25\xce\x91\x85\xa3\x69\xba\xe0\xe9\xa0\x6d\x64\xd2\x5d\x72\x16\x8e\xce"
      "\xdf\xc5\xae\xbe\xeb\x68\x50\xa0\x9c\x04\x0d\xf3\x0d\x99\x93\xef\xf4\x03"
      "\x30\x47\xb2\x9d\xcf\x63\xd4\x40\xcc\xf7\x33\xdf\xd8\x54\x40\x05\x70\x35"
      "\xd8\xda\x47\x63\xbf\x42\xf2\x99\x8a\xea\x72\xf3\x19\x8d\xc8\x81\x5d\x8f"
      "\x73\x59\x9e\xfc\xa4\x4b\x3a\xe9\x6e\xcf\xe2\xc0\x53\x54\x33\x79\x2b\x99"
      "\xcd\x04\x31\xfe\x31\xe2\x23\xdf\x03\x27\x02\x35\xe4\x25\x32\x63\x56\x38"
      "\x77\xa0\x98\x29\xdb\xf8\x91\x7e\x68\x30\xc5\x47\x19\x6b\xd8\x8b\x92\x78"
      "\xf8\xf9\x45\x6b\x5a\x08\x20\x58\xac\xcf\x1b\xf0\x0e\xd3\x6e\xce\xc2\xd3"
      "\x53\x7e\x09\xac\xa7\x6b\xfd\x99\x54\x3d\xc1\x7e\x23\xa5\x01\x75\x92\x13"
      "\x39\x11\x1f\x9b\xce\xc5\xfd\x08\x59\x79\xc6\x57\x1e\x88\xb3\x5d\x17\x9e"
      "\xfa\x65\x03\x03\xe9\xd7\x98\x7d\x85\xe3\xac\xfb\xb7\x84\x26\x61\x66\x01"
      "\xef\xc3\x24\x3f\x5c\x16\x36\x32\x83\x32\xc3\x24\xf0\xb5\xb8\xf1\x06\x66"
      "\x57\x91\xf0\xfb\xfe\x77\x71\x37\xce\xdd\xa0\x1f\xf5\xfa\x7e\x54\x12\xd3"
      "\x42\xc2\xab\xbe\x49\x85\x69\x44\x7d\x33\xa6\xc0\x63\x19\xf7\xe5\xde\xda"
      "\x10\x02\x9d\x7d\x9b\x02\x30\x34\x14\x6e\xe3\x27\x77\x7f\xfa\x50\x66\x2f"
      "\x65\xea\x26\x84\xe5\xf9\x47\x8a\x9c\xf7\x5e\x72\xb2\x6b\x89\x3b\x71\x4f"
      "\x73\x64\x71\xb5\x1b\x29\xdd\x57\x07\xcf\x58\x9e\x79\xe7\x96\xb1\x1c\xbe"
      "\xf2\x7d\xe9\x25\x11\xe6\xd6\xdb\x03\xf7\xfe\x2d\x87\x8b\xc4\xa7\x11\x57"
      "\x82\x4b\x52\xd5\x68\xf2\xeb\xe6\xfe\x94\xa4\xd1\x1c\xac\x3a\x11\x11\x3f"
      "\x00\x70\x24\xa5\x48\x68\xdb\x8e\xeb\x1e\x41\x2a\x92\x2b\x63\xf7\x77\x35"
      "\x26\x06\x28\x33\x25\x03\x45\x32\xa8\x91\x56\x0b\xf6\x90\xcd\x86\xcf\xf1"
      "\x0a\xe7\x1a\x54\x94\x10\xb2\x92\x70\x9f\x58\x0e\x41\x0f\x8a\xc3\x08\xc0"
      "\x07\xc3\x16\xc8\x1b\x58\xb8\x6d\xee\x65\x27\xd8\xd5\x40\x43\x5f\x18\x40"
      "\x14\xc0\x52\x51\x21\xdb\x17\xbc\x1a\x83\x23\x2e\x82\x28\x33\xf3\xda\x61"
      "\x9b\x0b\x69\x36\xc8\xfa\xe7\xe3\x73\x0c\x40\x29\x5a\xe8\xe6\x20\xca\x0b"
      "\x51\xf0\xf6\x27\x3f\x36\x95\x78\x6a\x6e\x53\x9b\xbd\x46\x8b\xc2\x07\x94"
      "\x6c\xc4\x04\x45\x32\xf4\x35\x41\xd8\xf4\xf1\x4e\x10\xd7\x8a\x0e\xf0\x6f"
      "\x7e\x13\x53\xbe\x56\x59\x09\x8d\x1f\x07\xb0\x46\xb0\xab\x67\xf7\xd5\x22"
      "\x7f\x41\xa7\x0c\x54\xfb\xdd\x18\x9e\xeb\x31\x8d\x54\x4b\x73\x27\xf2\x4c"
      "\x3d\xfe\x88\x0c\x4e\x64\x9d\x45\x6f\x6e\x82\xba\x0b\x92\xf2\x43\x10\x95"
      "\x7a\x38\xcd\x8c\x72\xce\x23\x43\x6c\x12\x70\x57\xc8\x72\x31\x3c\x73\x77"
      "\x6f\x93\x86\x78\x17\x23\x14\x7a\x33\xe4\x70\x86\xaa\x61\x4c\x24\x18\xd8"
      "\xb4\xad\xf9\x02\xb2\x6a\x77\xe5\xbf\xd6\x0d\x9a\x88\x7d\x70\x63\xb9\x31"
      "\x5a\xdf\x76\x20\xa7\x16\xab\xac\x92\xcd\x3d\x19\xe3\xc9\x9f\xad\x31\xde"
      "\x7a\xcb\x20\xe9\x11\x91\x0a\x1a\xc5\x4d\xa5\x79\xed\x20\xef\x40\xe9\x4f"
      "\x5f\xc7\xd5\xc2\x0b\x35\x27\x32\x9d\xd9\x21\x9d\x94\x86\x52\xa9\x2e\x9c"
      "\xe9\xa3\x31\xd7\xdf\x4e\x34\x6c\x65\xf2\x98\xe1\xd4\x31\xe9\x88\x72\x4a"
      "\x06\xfb\x5a\x91\xae\x89\xbc\x35\xe5\x80\xef\x4c\xf4\x9b\x32\x73\x3c\xe8"
      "\xaa\x53\x2a\x83\x8f\x35\x12\x75\x30\x4a\x36\x46\x1c\xf1\x33\xb3\x26\x64"
      "\xd5\xfe\x41\x9e\x32\x6c\x6c\xad\x1f\x1a\x47\xc9\x45\xb7\xca\xf3\x12\xc1"
      "\x19\x5c\x89\xb9\x20\xbc\x6f\x58\xa1\x6d\x04\xfc\xe7\x4c\x95\x03\xb2\xe6"
      "\xcb\x27\xed\x19\x94\x20\x36\x41\xc8\xa3\x10\x9a\xc7\x44\xcd\x19\x84\x31"
      "\x79\xee\x09\xd3\x50\x15\x3d\x79\xfe\x0a\x03\xb2\x1b\xea\x57\xd0\x47\x35"
      "\xbd\xd3\x2b\xd8\xbb\x6e\x63\x72\x83\x90\x76\xb5\x89\x2c\xf2\x58\x4f\x5c"
      "\xfe\x8b\xca\xc3\xe2\x34\x07\x05\xe8\xc4\xa6\xd5\x6f\x72\x7d\x2f\xd1\x4e"
      "\x59\x46\xc8\xfe\x6b\x91\xb0\x62\x84\xdb\x35\x72\x79\xc8\xe8\xb9\x6e\x30"
      "\xb3\xe4\xaf\x3d\x70\x27\x98\x0c\x1d\x93\xce\x46\xb8\xb2\x49\x56\x20\x96"
      "\xa9\x82\x1c\x31\xe1\xa0\xc2\x15\x84\x6e\xcb\x9d\xb0\x1e\x9c\x26\xfb\x0b"
      "\x65\xe9\x65\x6f\x4d\x59\x48\x1e\x96\xa5\xfd\xf9\xfd\xa3\x04\x0a\x4d\x85"
      "\xa7\x78\x68\x17\xc3\x7e\x5c\xc3\xfd\x93",
      4096);
  *(uint16_t*)0x20001244 = 0x1000;
  syscall(__NR_write, (intptr_t)r[1], 0x20000240, 0x1006);
  return 0;
}
