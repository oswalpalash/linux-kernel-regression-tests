// https://syzkaller.appspot.com/bug?id=faec16609edc192550ccb47ce6a60d5eca007e78
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

#ifndef __NR_ioctl
#define __NR_ioctl 54
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_openat
#define __NR_openat 295
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#ifndef __NR_write
#define __NR_write 4
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  intptr_t res = 0;
  memcpy((void*)0x20000080, "/dev/net/tun\000", 13);
  res = syscall(__NR_openat, 0xffffff9c, 0x20000080, 0x88002, 0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000040, "syzkaller1\000\000\000\000\000\000", 16);
  *(uint16_t*)0x20000050 = 0x5001;
  syscall(__NR_ioctl, (intptr_t)r[0], 0x400454ca, 0x20000040);
  res = syscall(__NR_socket, 0x10, 3, 0);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x20000140, "syzkaller1\000\000\000\000\000\000", 16);
  *(uint16_t*)0x20000150 = 7;
  *(uint16_t*)0x20000152 = htobe16(0);
  *(uint32_t*)0x20000154 = htobe32(0);
  syscall(__NR_ioctl, (intptr_t)r[1], 0x8914, 0x20000140);
  memcpy(
      (void*)0x20002580,
      "\x07\x03\x00\x00\x00\x04\x00\x00\x00\x00\x60\x2d\x2a\xe1\x0f\x98\x2f\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\x02"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x88\x3e"
      "\x00\x00\x02\x00\x00\x00\x00\x00\x68\x00\x00\x00\x00\xcf\x00\x00\x00\x00"
      "\x00\x00\x00\xd1\x14\xb1\x39\x16\x5a\xf3\x20\x91\xba\x89\x0f\x11\x43\x69"
      "\x7d\x60\xff\x6f\xbf\x67\x4d\xa1\x99\x5e\x0c\xd4\x90\x05\x05\xf2\xce\xb4"
      "\x4d\xcd\x98\x20\x9d\x1d\x99\x47\xfc\x39\x6b\xfb\x08\x71\xe6\x49\x13\x13"
      "\xcf\xe0\x78\x8a\x50\x74\xe8\x9e\x8a\xe3\xba\x8d\xf7\xe6\x87\x7d\x0b\x40"
      "\xee\xec\x0e\x36\x2d\x17\xeb\x2f\x71\x80\x2b\xb6\x0e\xa9\x7b\x5a\xfc\x81"
      "\x0d\x37\xd1\x54\xe4\x7b\x04\x4a\xb8\x0e\x72\x38\x58\x8f\xbf\x37\xde\xc7"
      "\x82\xb8\xb1\xbc\xc0\x95\x01\x73\x79\x14\x56\x95\x9f\x80\x3d\x61\xb0\x56"
      "\x31\xef\xbb\xfc\xce\xbe\xaf\xef\x57\xbd\x95\x63\xc2\x7e\xc7\xe5\x0d\xdb"
      "\x67\x18\xb4\xa2\xac\xa7\x20\xfd\x86\x62\xc6\x71\x3d\x50\x7f\x08\xb8\xed"
      "\x95\x62\x2e\xbe\xbe\x3a\x02\xb9\x76\x44\xe2\x40\xf3\x04\x23\xfb\xaa\x33"
      "\x9c\x43\x3d\x6b\x41\xf6\xdb\x76\x70\xa5\xaf\x4b\x2f\x17\x2e\x64\xc2\xe5"
      "\xb4\x0b\x8f\x6b\x37\xc2\xe6\x44\x21\xd0\x33\x1a\xff\x52\xb9\xc9\x10\xff"
      "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x01\x00"
      "\x00\x01\x00\x00\x8c\xb3\x7d\x45\x18\xd6\xae\x13\xad\xdd\x27\x1c\xa3\x9d"
      "\x04\x8e\xd6\xd8\x30\xfc\x13\x40\x62\x36\xe5\xb6\x7f\x4c\xda\xd0\x86\x7d"
      "\xff\x7a\x4d\x54\x4a\x31\xa8\x95\xf0\xe9\x67\xa2\xaa\x3a\xaa\x21\x95\xcd"
      "\x11\x81\x1a\x8e\x0c\x3b\x78\xa3\x01\xb1\xd4\xcf\xcf\xf4\xa6\x9f\xac\x54"
      "\x6d\xb9\xf8\x17\x07\xb4\x2c\x40\x98\xb6\x79\x46\x9e\xfe\x5b\x8e\xe7\x1c"
      "\x87\x96\xbb\x12\xe3\xe2\x64\x1e\xf4\x0d\xa6\x63\xe4\x02\xc9\x62\xf7\xe1"
      "\xd9\x95\x3e\x53\x64\x5f\x6f\xfd\xa6\x98\x7d\xb8\x4b\x9a\x80\x6c\x7c\x2f"
      "\xe0\xa6\x9d\x4d\xb3\xfd\x31\x4b\x37\x41\x0f\xa0\xe6\x5c\x56\x14\x9b\xde"
      "\x13\xbd\xf8\x92\x42\x41\x1d\xc1\x97\xea\xfa\xfd\x76\xa0\x7e\x83\xf3\xc8"
      "\xb2\xab\x12\x61\xcc\x75\x63\x16\x5e\x38\x0b\xc3\x9c\xc4\xe8\x8d\x70\xb8"
      "\x63\xce\x00\x7e\x71\x6d\x86\x09\x2f\xd1\xec\x17\xdb\xd2\x41\x81\xa6\x4e"
      "\x28\x32\xc8\x39\x07\x49\xf2\x7f\x89\x89\xa9\x4a\x94\xb4\xd6\x9d\xd3\xbd"
      "\xb0\xd3\x96\xc7\xd5\x8f\x79\x77\xe1\xc0\x1c\xbd\xfb\xe7\xad\xf0\x7a\x80"
      "\xaf\x96\x22\x5c\xff\xad\x1d\xcb\xd2\xc6\x17\x21\x6b\xf8\x9b\x8a\xe7\xcd"
      "\x0e\x41\x63\x83\x0a\xcc\x2f\x1c\x95\x4c\x75\x44\x8d\x27\xa8\x91\x89\xf4"
      "\x91\xf6\x69\x7c\xed\xed\xb9\x5d\xe0\xf0\x23\x1c\x57\x31\x7f\x61\x0a\x52"
      "\xfe\xd4\x4e\xbb\xa1\x1b\xe4\x0e\x36\x56\x89\x50\xb4\x7c\x18\x7c\x86\xdb"
      "\x3e\xba\x66\xe3\x21\xb5\xa9\x0d\x0f\xa5\xc4\x48\x4a\xca\x6e\x66\xd9\xc0"
      "\xa8\x9f\x4a\x70\xc0\xe9\x2a\x24\x64\x5a\x89\x67\xd6\x88\xdc\xcd\x14\x8d"
      "\x79\x43\x65\xae\x67\x2f\x69\xfe\xf7\xf4\x2c\x97\x4d\x44\x46\x43\xdf\xa5"
      "\xbf\x84\x2a\xbf\x60\x80\x5c\xcf\x13\xd6\x0f\x93\x88\x5e\x9e\x36\xea\xbc"
      "\xaf\x12\x97\xa9\x34\x3e\x4f\x7a\x31\x84\xae\xdf\xae\xd8\xab\x6c\x2c\x3a"
      "\xf6\x8c\x40\x23\x10\x1a\xae\xb3\x6c\xf1\xb2\x85\xe3\xa8\xfe\xf3\x31\xe6"
      "\x19\xa1\xd0\xe2\x71\x45\x5f\xda\x62\xba\xde\x30\x32\xf7\x36\xe1\x9c\x86"
      "\xf4\xd9\x12\xc6\xbf\xe2\x41\x45\xff\xb4\x8a\x46\xd6\x95\xa7\x57\x52\xdf"
      "\x73\xf7\x36\x6e\x0f\xef\x6b\x7f\xa4\xb4\x21\x1b\x00\x5d\x97\x07\x89\x4f"
      "\x2b\x22\xa8\x25\xb3\xd6\x71\xd9\x9e\xd7\xfb\x8d\xd3\x61\x58\x07\xec\x2e"
      "\x2d\x92\xf4\x8f\x9a\x19\x7f\x86\x4c\x39\x0d\xe2\x34\x96\xd1\x51\x7d\x4d"
      "\x71\xb7\xe1\x5f\xec\x54\x4c\xba\xaa\xeb\x89\x81\xb6\x6f\xde\x00\x5c\x5d"
      "\x09\xaf\xb2\x1b\x1c\xcb\x9c\xbe\x6f\xdc\xc4\xb5\x49\xfb\xa7\x54\x99\xe6"
      "\xe1\x08\xfd\x4a\xe4\xfe\xdf\x1b\xab\x77\x98\xe2\xa9\x99\x61\x3a\x72\x73"
      "\x65\x2e\x4e\xa9\x07\xeb\xff\xdf\x87\x25\x04\x12\xcd\xfa\xf6\x63\xc7\x47"
      "\x93\xb4\x40\x01\x9f\x1b\xef\xb1\x21\xe1\xcf\xf5\xe9\xe6\x18\x7d\xe4\x6d"
      "\xb8\x92\x20\x0f\xc2\xaf\x9e\xe2\x36\xfc\xee\xfc\xdb\x1a\x61\xc6\x56\x51"
      "\x80\x91\xd3\x91\x1a\x14\xc1\x15\x0d\x91\x1b\x9b\x92\x31\x26\x24\x1d\x77"
      "\x3f\xf2\xe6\x22\xbb\x10\x76\xdf\xcc\x37\xa0\x99\x54\x0f\x0d\xd6\xb3\xe9"
      "\x68\x92\x51\xa0\xfb\xfe\x4c\xde\x79\xf9\x9b\x3a\xac\xc6\x51\xc0\x61\xc9"
      "\x66\xb7\xa7\xa5\xde\xf7\x3e\x14\xf2\xdf\x0e\x54\x52\x18\x46\x20\x3e\xdb"
      "\xc0\x06\xa7\x5d\x27\xb2\x57\x4a\x28\x0f\x44\xbb\x68\x4a\xd7\x66\xc6\xee"
      "\x09\x1a\xc4\x00\xa5\xe9\xe7\xb8\x2a\xa3\xe4\x0a\xa8\xf6\x95\x63\xdc\x14"
      "\xe9\x0c\xb7\xc2\xdd\x1a\x8f\xb3\x26\x5a\x89\xeb\xec\x21\xe3\x01\xc2\x9d"
      "\xab\x4b\x04\xba\x9a\x62\x63\xb8\xc5\x9a\x32\xc7\xed\x3a\xb6\x0e\x4a\xcb"
      "\xf4\x9b\x48\x09\xef\xc4\x7d\xef\x8c\x11\xba\xfe\x0a\x3f\x02\xc0\x8d\x46"
      "\xf7\x9e\xc6\xe0\x63\xcf\xef\xed\x23\x89\x18\x34\x48\x1a\xa4\xec\x50\xb7"
      "\xee\x50\x6e\xe3\x67\x39\x80\x9f\x33\xf8\x9d\xdd\x40\x15\x2a\xec\xe7\x12"
      "\x35\xae\x9d\x8b\x65\x6b\x92\xa6\x94\x79\xb3\xb5\xb9\x7f\x35\x49\xb5\x40"
      "\x61\x36\xca\xeb\xeb\x46\xee\x6a\x66\x03\xd3\xac\xd0\x01\xa6\xb3\xa9\xef"
      "\x56\xf5\x11\xe5\x12\x66\x7a\x57\x22\x0a\x13\x8a\x3c\x09\xae\xdd\x37\x4d"
      "\x69\x22\x4a\x14\x8a\x4d\xdf\x95\x53\x4f\xd6\x46\x73\xc0\xf1\x80\xf3\x44"
      "\x96\x4f\xb3\x0f\xae\x60\x82\xf6\xae\x71\x46\x41\x76\x17\xe3\x19\x6f\xe1"
      "\xec\xc7\x40\x02\x85\x76\x49\x6f\x73\x6f\x8c\x8e\x5c\x52\x13\xdf\x54\xba"
      "\xf2\x2f\x8b\x5d\xf6\x77\x37\x16\xe8\x46\xc9\xbb\x44\x52\xba\x44\x65\x9c"
      "\x30\x22\xb3\x91\xdc\x0c\xff\x68\x5f\x15\x9c\xcf\xac\x22\xe2\xcc\x5c\xac"
      "\xce\xa6\x0b\x29\x05\x9f\x52\x54\x49\xa5\x8f\x6f\x53\x24\x61\x28\xcf\x01"
      "\x26\x26\x6e\x66\xea\xd7\xd7\xb5\x7c\x70\x17\xc8\xda\xcf\xc7\x58\x09\x2c"
      "\x0d\x10\x52\x39\x86\xe8\x46\x8b\xfa\x3e\x8c\xd1\xc6\xb5\x26\xe2\xad\x23"
      "\x25\xd2\xb7\x59\xfe\x70\xab\x4d\xe7\x46\x61\xc4\x8d\x64\xae\x07\xdb\x86"
      "\xf0\xbc\x11\x53\x75\xab\xc4\xce\xf4\xaf\xe6\xc3\xf2\x57\x84\xff\x19\x15"
      "\x86\x4c\x28\x39\xe4\xc6\x94\xed\x16\xc5\x83\xca\xe1\x6b\x14\xbb\x11\xda"
      "\x5a\x5f\xa6\x59\xcf\x12\x98\x89\x54\x81\xfc\xca\xff\x26\x2d\x6b\x38\xd3"
      "\x12\x9b\xe8\xa6\x88\x3b\x63\x60\xf8\x65\x08\xed\x80\x9f\xe3\xe6\x11\x2a"
      "\xd8\x50\x59\xe0\xbd\x16\x24\xf9\x28\x22\x00\x38\x55\x51\xfa\x41\xb9\x88"
      "\x13\x96\x44\xa2\xb0\xbb\xd0\x9b\xe7\x30\xc6\x01\x0a\x70\xe8\x46\xb8\x61"
      "\xf9\x53\x18\x06\xe1\xba\xf0\x4e\x03\x29\x91\xfb\xa3\x6f\x2c\x55\xc9\xee"
      "\xca\xd7\x43\x75\x6a\x5f\x87\x46\x42\x2b\x90\x0f\xac\xb8\xb4\xdb\x46\x32"
      "\xc2\x77\xcd\xec\xa7\xc7\x28\x8a\x88\x16\x69\x6a\xd3\x54\xd5\xf0\x07\x53"
      "\xe0\xf3\xd6\xe1\xe1\xad\x3e\xc6\x6d\x21\xfc\x01\xc4\xae\x29\xfd\xe5\xdb"
      "\x53\x27\x4c\xcf\x04\xea\x8d\xca\x09\xdb\x82\xc1\xdb\xf3\x20\x2c\xb4\xb6"
      "\x5d\x76\xc0\x8f\xbb\x18\xba\x20\x3b\xe4\x85\x6f\xc9\xac\xe6\x92\x16\x47"
      "\x4b\x8a\x12\x9c\x06\x3b\x9d\x8a\x71\x2b\xc4\x89\x46\xcb\xb2\x43\x62\x19"
      "\xf9\x05\x2a\x5a\xee\xcb\x33\x22\xae\x98\x4c\x5b\x07\xf2\x89\x6e\x5a\x05"
      "\xda\xeb\x38\x18\xb2\xc6\xae\x51\x89\xf1\x6d\x26\x22\x21\xcb\x35\xc2\xd6"
      "\xb9\x33\xd1\x19\xcc\x55\x0f\xde\xf6\x9c\x34\xbb\x9d\xf0\xdd\xb7\x57\x2a"
      "\xe8\x01\xce\xb1\xf0\x0c\x44\x8d\xbc\x2a\x51\xf2\xda\x6c\xd6\x99\x2d\xf1"
      "\xd6\xa1\x8e\x81\xba\x2f\x44\xbb\x33\x7b\x8a\x16\x1f\xa8\x16\x04\xe1\xc8"
      "\xce\x9b\x8c\xd8\x35\x1b\x6e\x75\x75\x19\x6f\x43\x31\xfc\x88\xc9\xa9\xd3"
      "\xab\x8f\x1a\x64\x61\x14\xdf\xc7\x55\x27\xb9\xb6\x69\xf8\xd6\x1c\x32\x5d"
      "\x11\xa8\x75\x37\x20\x5c\xd7\xf8\xd6\x75\x88\x37\xe7\xb4\x51\x79\x53\x4f"
      "\x7f\x38\x00\x2c\x4c\xb5\xe3\x22\xd0\x1b\x3b\xeb\x3b\xc6\xaa\x41\xfb\x91"
      "\xc0\x5c\x94\x5f\x50\xf6\x2a\x23\xde\xbd\x8f\xf8\x6b\xf2\x45\x76\xa3\xa0"
      "\x21\x35\x9d\x28\x7b\xa1\x99\x77\xc9\xce\x3d\x31\x05\xc0\x5f\xe4\x39\xe8"
      "\x56\x82\xaa\x03\xfd\xea\x6b\x1d\xb6\xed\xa4\xa4\x6e\x66\x24\x0e\xc2\xb3"
      "\x4b\xc4\x71\xa7\x77\x23\x15\x78\xaa\x7e\x4b\xfc\x1f\x18\x10\x8f\x03\xcf"
      "\x8c\x36\xac\x0b\xe8\x06\xd0\x31\x30\x9e\x39\xab\xc6\x72\x37\x0a\xae\x6c"
      "\x09\x60\xb6\xbe\xf9\x9e\x2a\x62\x8e\xb1\x69\xc5\xe6\xb6\xb8\xd2\x28\xa9"
      "\xba\x9a\x2f\xe1\x4c\xf3\x15\x33\x1e\xee\x67\x14\x35\x7d\x7e\xbb\x84\x66"
      "\x7b\x5a\x6d\x39\x00\xd4\xe7\x5e\xe8\x91\x1d\xe5\x9d\x3b\xcc\x2d\x4b\xd0"
      "\xd5\x14\x2a\x02\xd8\x60\x88\x1f\x7a\x71\x06\xc5\x9e\x5a\xe1\x0c\x56\xc8"
      "\x8e\x2f\xdf\x64\x71\xe5\x79\xcb\xdc\x0b\xe5\xc6\x56\x5e\x02\x2c\xa6\x5d"
      "\x81\x73\x7c\xd0\xdf\x1a\x53\x81\xe5\x6a\x9e\xe0\x9f\xaf\xe9\xd6\x02\x15"
      "\x74\x8c\x15\xb5\xe2\xed\x9c\xc3\x7f\x1c\x7d\x51\xa0\x5f\x00\x43\x50\xfe"
      "\x74\x6d\xa1\x9f\x3f\x01\x27\x54\x6b\x63\xc6\x6f\x3c\x40\x03\x91\x32\xbb"
      "\xe2\xc7\x4b\x28\x6e\xf8\x01\x01\xc8\xeb\xb4\xe8\x4f\xe3\x97\x8e\x0e\x20"
      "\x8b\xeb\xd8\xd3\xb9\xfd\xf7\x4b\xdc\x10\x8d\x08\xef\x4d\x86\x23\x6d\x32"
      "\x7c\x5f\xf1\xb9\xc2\x9d\x4c\xd0\x95\x0e\x09\xf0\xe8\x68\x48\xc5\xd3\x02"
      "\x8d\xc7\x99\x4a\x58\x9e\x60\xd0\xf2\x93\xb9\x82\x56\x51\xc8\x49\x62\x10"
      "\xa0\x0a\x84\x3d\xb3\x71\x04\x08\x5a\x53\x64\x6e\x4f\x9e\xf8\x87\x6c\x06"
      "\xc9\xb3\x01\x4e\xed\x02\xa2\x1a\x93\x41\xce\x74\x6c\x15\xd3\x63\x8e\x7a"
      "\xee\x8a\x16\xcf\x62\x3d\x1a\x94\xbe\x85\x0d\x77\xd0\xfc\x22\x93\x5e\xde"
      "\xd1\xd6\xf6\x91\x76\x20\x29\xcd\xf3\x00\x53\x95\x04\x4a\xb8\xbc\x98\x9a"
      "\x7f\x67\x6f\x28\xbc\x2c\xe3\x10\xa1\x3e\x30\x03\xc9\xba\x3b\x81\x4c\x82"
      "\xa5\x11\x31\x84\x35\xf0\xfa\xdc\xc7\x08\x8b\x9d\x47\x7b\x5b\x5e\xcb\x88"
      "\x45\x05\xf0\x10\xf9\xb0\x72\xa2\x0d\x6f\xb2\x91\xb8\x52\x4f\xb0\xf6\xe8"
      "\x47\xf4\x52\xff\x5b\xa3\xee\xb8\xb1\x2c\x0e\x3e\x25\xde\xcc\x81\xb8\xf1"
      "\xf8\xd9\x80\x30\x0c\xc9\x2c\xe7\x0c\xdc\xf0\x3f\x70\x78\xcc\xd7\xc7\x9b"
      "\x9d\xb1\x67\xfe\x7e\x82\xdc\x04\x0e\x68\xf9\x41\xf3\x36\x84\x72\x79\xcc"
      "\x99\x60\xdc\x6b\x0a\xd6\x5a\xdc\xf1\x05\x0b\xc2\x8e\xaf\x70\x0a\xf1\xf3"
      "\x9e\x76\x48\x9f\xf1\x9c\x1a\x48\x65\xd8\x9b\xf5\xdc\x8a\x89\xdd\x64\x51"
      "\xbd\x52\xdc\xbf\x67\x63\xea\xd8\xa6\x0e\x4f\xbd\xd8\x5c\xa3\x7b\x25\xbc"
      "\x3e\xdc\xa8\xf2\x78\x60\xfd\xf3\x9a\xd5\x99\x3e\xe1\xb4\x7f\x02\xcc\xaa"
      "\xd9\xb2\xca\xe3\xef\x53\x00\xc1\x96\x37\xec\x24\xf0\xcb\x2a\xcc\x88\xe8"
      "\xa6\x39\x08\x77\x7f\x0c\x75\x9b\x26\x55\x81\xae\xa3\xe3\xa7\x4f\xcb\x44"
      "\x3d\x4f\x28\x99\xcf\x3c\x3a\x28\xc9\x4d\x5b\x8e\xd2\x8f\xd0\x01\x74\x24"
      "\x59\xb9\x9d\xcd\x48\xa6\xc7\xcd\x30\x60\x0b\x61\x26\x2e\xac\x6d\x24\x9e"
      "\x34\xf7\x56\x9a\xa5\x34\xc3\x7b\xe0\x88\xb1\x2c\xe8\xa9\x17\xaf\x4d\xb6"
      "\xa6\x03\xb7\x35\x54\x14\xcc\x2a\x92\x07\x1f\x64\xf1\x7b\xc7\x3a\x62\xb2"
      "\xa4\x61\xfb\x4f\x90\x7c\x81\x55\x09\x0a\x6c\xca\xd7\x18\xd3\xf4\x86\x44"
      "\x65\x9c\xba\x76\xa4\x2a\xbb\xbf\x22\xa9\xac\xbc\xdc\xbb\xf1\x6f\x11\x49"
      "\x4f\x21\x4b\xd6\xa5\xbc\xc5\x41\x5a\xa7\x8a\xd9\xd5\xc9\xd9\x55\xc0\x18"
      "\x74\xce\x24\x16\xc7\xe7\x44\x41\x46\xaa\xa2\x91\x43\x6b\x9a\xdd\xb2\xcd"
      "\xac\xaf\xfa\x85\x42\x84\xe3\x1c\xbe\xa9\x31\x5a\x39\xcf\x56\x1d\x39\x5c"
      "\xd2\x02\xeb\x13\xd1\xe3\x30\xa8\x30\x3c\xe9\x48\xcd\x9a\xf2\x6d\x14\xe2"
      "\xf0\x6b\xbf\x7a\x82\x2e\x65\x47\xfb\x1d\x75\x0b\x36\x1c\x68\x59\x5b\x68"
      "\xf7\xb7\xd3\xa2\xbd\x7a\xa0\x07\x13\xd4\xaf\xa8\x23\xc4\x4a\xee\x12\x85"
      "\x68\xc2\x80\xbe\xf9\x25\x43\x19\xc0\xfd\x37\xea\xd3\x0e\xac\x83\x54\xbe"
      "\x03\xf4\xff\x44\x16\x90\x52\x2b\x94\x03\x53\xc9\xf3\xcf\xeb\x3c\xf3\x99"
      "\xc5\x8a\xea\x6e\x25\xbc\x5f\xce\xef\x0c\x29\x34\x76\x0d\x9a\x7c\xb7\x61"
      "\x4d\xc3\x90\x19\x60\xb6\xa1\xbe\x8a\xc1\x4c\x36\x7f\xd8\xee\xc4\x48\x92"
      "\x9f\xc1\xf5\xef\xc8\xa7\xbd\x30\x0e\x10\x92\x03\x62\x98\x29\xbf\x79\x05"
      "\x0b\x85\x6f\x26\x8a\x10\x1c\xfb\x85\x8b\xc5\x43\x98\x52\x36\x6c\xe7\xca"
      "\x20\x51\xbf\x01\x57\x1c\xa4\xa3\xb2\xc5\xb2\xfe\x0d\xe8\x18\xf3\xca\xb4"
      "\x3d\x05\xcd\x38\xa2\xca\xc4\xfd\x92\x55\xb8\x97\x85\x5b\x55\xee\xe8\x35"
      "\x32\xa0\xd8\x24\x21\xc7\xed\x74\x26\xde\x53\x81\x88\x7d\x92\xd2\x4c\x67"
      "\x8f\x3a\x16\x42\x21\x9e\x6b\xaa\x06\x7e\xfe\xf2\x41\x0a\x40\x45\xf1\x95"
      "\x81\xa3\x28\x16\x0c\x55\x19\x91\xa2\x80\xc2\x92\x2d\x47\xa2\x74\xf0\xd0"
      "\xce\xac\xa9\x74\xb7\x86\x23\x8a\xff\x3f\x7b\x89\x31\x77\x5f\x2a\xdf\x50"
      "\xce\xac\xfe\xdf\x71\x9d\xa4\x89\x35\x80\x59\x27\x25\x31\x3c\x20\x9a\x4d"
      "\x11\x02\x9a\xa1\x98\xcd\xb9\xa8\x0d\x31\xc8\xb4\x0e\x6d\xe5\xb7\x4d\x75"
      "\x22\x87\x50\x04\x94\xc3\xe2\x2d\xf3\x6c\xdf\xdb\x6f\x4d\x5a\x8a\xc2\xc8"
      "\x71\xee\x89\x77\x2a\x7a\x00\xec\x5c\xc7\x8b\x39\xc9\xd0\x4b\xb0\x79\xb7"
      "\x59\xaf\x9b\xf5\xf9\x0a\x07\xa4\x3f\xc2\x36\x86\xf7\xf1\x3f\x3c\x6e\xdc"
      "\xd8\x0e\xe1\x2a\x63\x85\x64\x01\x47\xe8\xac\x54\x5e\xe6\x87\x53\xcc\x01"
      "\xdf\xa2\x3a\xf4\xd1\xe0\xd6\x39\x12\x45\x3a\xaf\x7d\x8b\x12\x27\xca\x67"
      "\x25\x45\x86\x39\x97\x8e\x2e\xa9\x5c\x3d\x32\xdb\xba\x6b\xb5\x2c\x7b\x19"
      "\x13\x79\xc6\xa2\xbd\x5c\x4b\x7b\xdc\x47\xc7\x0e\x73\x6c\x7d\xa1\xc9\xb6"
      "\x79\x5c\x65\x47\xa7\x41\xaf\xfb\xff\x4b\xf3\x7d\x09\xef\x4d\x36\x82\xe7"
      "\x30\x04\xdd\x06\x52\x29\xc6\x75\x59\x94\xff\x06\x79\x12\x51\x23\x28\x9d"
      "\x94\xaf\x03\x62\x01\xe4\x85\x47\xf5\x99\xe2\x4b\x7d\x87\x78\xde\x18\x38"
      "\x07\x10\xe3\x70\xc3\xc8\x6f\x0a\x48\x83\x59\x54\x34\xa5\xf6\xf9\x3f\x95"
      "\x5a\xbd\xc0\x13\xab\x48\x7a\xfe\xd0\xe1\xcb\x05\x32\x67\xf3\x29\x97\x33"
      "\xf1\x71\x25\xd7\xb7\x15\x3f\x8f\x9e\xf7\x7f\x86\x83\xe0\x26\xeb\xed\x04"
      "\x54\x9b\x0f\xf1\x55\xab\x9b\x4d\x59\x95\xf9\x46\xc9\xa8\x48\x70\x07\xe2"
      "\xdd\x5f\x8f\x6b\xaa\x25\x7b\x6d\x96\x93\xfd\xcd\x4a\x3d\x82\x33\x84\xac"
      "\x9e\x92\xb6\x68\x31\xf2\x95\xd7\x2f\x7c\x0b\xa5\x39\x8b\x8a\x6a\xd3\x9d"
      "\x0f\xa6\xe5\x30\x97\xbb\xd1\x63\x49\x57\x0a\x96\x24\x5e\x3b\xb5\xe4\x1c"
      "\x93\xaa\x60\x89\xec\x85\x3d\xc7\xa4\xf9\x11\x53\xa5\x26\xf1\xb6\x6c\xc2"
      "\x6a\xd4\x24\xd3\x2b\x57\x36\x9a\xa2\xcd\x5a\xa2\x51\xa4\x0b\x31\x89\x44"
      "\xcd\x6c\x09\xf2\x76\xea\x9c\xf5\xa4\x21\x27\x24\xcb\xa3\x71\xfe\x49\x0b"
      "\x4d\x0e\x80\xe6\x43\xbd\x23\x6a\x9d\xdb\x2d\xb1\xfc\x36\xcf\xb9\xb0\xcc"
      "\xbd\xce\x44\xa7\x7d\x1a\x6b\xb0\x2b\x37\xb7\xcf\xe0\x8c\x99\x5d\xdb\xa2"
      "\x5e\x29\x84\x69\x69\x8e\x7b\x1e\x4f\x49\x77\xb7\xac\x41\x40\x9a\x08\xae"
      "\xec\x23\xa6\xba\xc5\x29\x3d\xd2\x02\xb5\x3a\xa2\x27\x9e\xc8\x6e\x02\x19"
      "\xca\xe6\xce\xa3\xc0\x96\x66\x63\x7c\xaf\x07\x6e\x9e\xed\xd0\xec\xae\x03"
      "\xef\x34\x33\x8d\x9b\xa9\xb3\x5c\x1b\xa6\x77\x2f\x4e\x7a\xa6\xf4\xa6\x4d"
      "\x15\xe3\x39\x0c\x82\x72\x12\xb9\x31\x92\xf1\x9a\x96\x09\x0a\x00\x9f\x96"
      "\xf3\x5b\x31\x83\xc6\xde\x6f\x3d\x70\x94\x7d\x86\x2d\xb1\x27\x5c\x04\x47"
      "\xb2\xda\x78\x7b\x7a\x63\x72\xde\x28\x2e\x20\xa6\xfc\x88\x80\xe0\xea\x6a"
      "\x0f\x73\x8c\xe4\xdd\x8e\x5d\xeb\x2f\xbc\xb3\x87\x28\x7f\x71\x73\x53\x8e"
      "\x89\x9e\x3d\x40\x12\x79\xfa\x5f\x3a\x38\x4c\x1f\x4c\xd9\x51\xe3\x00\x42"
      "\xf0\xf6\x27\x0f\xe6\x60\xba\xc9\x32\x31\x8e\x29\xa7\xd8\x42\x23\x3d\xad"
      "\x53\x88\xa6\xc8\x2a\xb9\xb1\x11\xb6\x7a\xe7\x3f\x47\x22\x56\x3f\xc8\x89"
      "\x83\x25\x6a\x77\x06\xed\x88\x3a\xc4\x1b\x51\xef\x42\x7c\x2a\xa1\xa8\xa0"
      "\xf5\x6b\x72\xc0\xd9\xe8\xce\xe6\x37\xf7\x7f\x67\xe9\x99\x13\x55\x1a\x9c"
      "\x78\x49\xf8\xa9\xd5\x0b\x20\x0c\x8e\x6e\x1b\x16\x92\x86\x60\x98\x2b\xc8"
      "\xc4\xb7\x70\xfd\x90\x7a\x2d\x74\x0b\xab\xec\x99\x13\x90\xdd\xc2\xb7\xfc"
      "\xe2\x3f\xa3\xa6\x11\x31\xbd\xc8\x75\x02\x71\x1b\x4b\x57\x59\xe6\x35\x12"
      "\x7a\xdf\xe7\xfe\x69\xe0\x6a\x43\xb4\x50\xf9\xcc\x1c\x60\xe7\xae\xd7\x5f"
      "\xfd\x30\xc3\x27\x24\x01\x7e\x7a\x02\x1c\x90\xf8\xc8\x1a\xe8\x65\x90\x8d"
      "\x48\xfa\x7f\xcb\x7c\xfb\x9b\x0f\x15\x30\xfd\xe5\x5c\xf0\x8c\xb3\xc1\x4d"
      "\x36\x56\x49\x15\x2f\x2c\x8a\x8c\x61\x5f\xef\xb3\x7a\x21\x9f\x73\x0f\x5d"
      "\x2a\x66\x0c\x4c\xf0\x30\x8a\x40\xf0\xc0\x60\x99\x32\x11\x36\x30\x14\x28"
      "\x40\x8a\x8f\xa8\x73\x5b\xc6\xf7\xea\xeb\xbd\x40\x67\x28\xb6\xe0\xf9\x43"
      "\xa8\x59\x75\x57\xf6\xe5\x38\x12\x33\x53\xe5\x6e\x52\x0d\x45\xe9\x93\x1d"
      "\xbb\xce\x8c\x5a\x7f\x09\x46\x45\xe1\xed\x88\xd1\x6c\x3d\x91\x72\xb1\x09"
      "\xf9\x3f\xd1\x8f\xed\xa6\x95\x69\x41\xe0\xb4\x1f\x35\xe2\xc5\xb7\xc9\x97"
      "\xce\xd9\xfb\xac\x1e\x9f\x0f\xf3\x61\xfa\x81\xbe\x5c\xc5\x9e\xd7\x8e\x00"
      "\x64\x44\xf7\x1a\xb8\x68\x45\x50\x2a\x0a\x6e\x49\xc8\xef\x81\xf8\xe4\x5c"
      "\x94\x3e\x78\x24\xe8\x7e\x65\x13\x74\x58\x50\x0a\xb1\x8f\x0e\xcf\xc9\xaa"
      "\x10\xec\x00\xc2\xa1\x02\x3f\xf9\x69\x5c\xc2\x72\x8b\xae\xc1\xf2\x10\x4c"
      "\x1d\x69\x90\x69\xb1\x82\x51\x35\xa1\x1f\x2b\x77\x6d\x57\x6e\x38\x64\x09"
      "\x19\xed\x96\x85\x0c\xfd\x48\x63\xd5\xc9\xfa\xfe\x47\x3e\x88\x54\x4a\xd0"
      "\xd1\xd2\x07\x1e\x00\x0f\x23\x34\x83\xdb\x41\xfe\x5c\x46\xda\x9c\x47\x2b"
      "\x6c\xfb\xca\x9a\x0b\xf0\xd8\x20\x37\x4d\x34\xb2\xe8\x4e\xc6\x0d\xc2\x65"
      "\x7f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x67\x0d\xb5\x04\x6f\x39\x00\xc1"
      "\x8c\xf0\xa1\x2f\x17\xc4\xb1\xbe\xa9\xd0\xb3\x66\x7c\x76\x15\x94\xcb\x2e"
      "\x4c\x0c\xb0\xca\xc5\x43\x43\x7b\x6d\x75\x78\x0d\xb9\x19\x76\x3f\x59\x21"
      "\x2d\x9c\x4d\xae\x0e\xb9\xe7\x30\xd2\x38\xf4\x0f\xf2\x6b\x34\x3a\x3e\x19"
      "\x06\x55\x48\xa4\x10\x6c\x98\xfa\x5e\x5c\x45\xbf\x20\x55\x1f\x7b\x90\x87"
      "\x2b\x5a\x88\xd3\x3a\x85\xfe\xe6\x08\xf3\x8f\x90\xff\xfd\xa8\x8b\xc7\xd3"
      "\xf7\x0f\xdc\xda\x4c\x9e\xe6\xd6\x24\x0e\xb9\x7c\x76\x6d\xfb\xd4\xec\x13"
      "\x89\x4b\x4f\x19\x1d\xe6\x83\xc5\xf9\xae\xfc\xc5",
      3918);
  *(uint32_t*)0x200034ce = 0x41424344;
  *(uint32_t*)0x200034d2 = 0x41424344;
  memcpy(
      (void*)0x200034d6,
      "\x90\x00\x00\xc5\x90\x78\x00\x00\x02\x04\x00\x00\x22\x09\xbd\xe8\x87\x85"
      "\x10\xdc\x91\x00\x00\x00\x50\x3e\x71\x42\x6c\x3e\x1b\xb0\x8f\xb8\x2c\x3b"
      "\x5a\xec\x00\x52\x16\x40\x47\xd9\x15\x02\x27\x0a\xc2\x00\x4c\xa1\xdd\x33"
      "\x8f\x9c\x92\x4a\xf6\xec\x3c\x82\x5c\x4c\xc8\x28\x86\x80\x02\x9a\x34\x5d"
      "\x22\x24\x5d\x02\xae\x3e\x20\xea\x97\x37\xdc\xa3\xfc\xd0\x7a\x1b\x74\x81"
      "\xeb\x0f\x0e\xb4\x04\x3b\x46\x1a\x7a\x11\x2a\x0a\x2b\x9f\x62\x16\x20\x2f"
      "\xcb\x32\x89\x3f\x1b\xe0\x12\x9b\x17\x56\x58\x23\xcd\xdc\xd3\x4d\xe7\x24"
      "\x9b\x29\x62\xa2\x0d\xfe\x81\x3a\x38\xb5\xd3\xd2\xad\xaf\x52\x56\x76\x9f"
      "\x84\x70\xbe\xf2\x9a\xc8\x90\x9d\x53\x59\xab\x65\x91\x3a\xa7\xa9\x45\x45"
      "\x14\x3c\x3e\xcf\xed\xd9\x2b\xf8\x0a\xc9\x19\x4e\xde\xcd\xc9\xc5\x88\x4a"
      "\x6a\xd4\xa2\xc3\xce\xf5\x59\x7c\x94\xab\xfa\x7f\xd7\xe8\x25\xd2\xec\xce"
      "\xab\x5f\x37\x69\x70\x76\x68\x4e\xc0\x36\x16\x5d\x42\xe7\x11\x0f\x01\x53"
      "\x72\x08\x03\x7b\x6e\xc5\xac\x75\x7a\x3b\xc5\x17\x06\xbc\x2b\xaf\x36\x38"
      "\xe7\xbe\x21\x9d\xc0\x5c\xfc\x10\x31\xe3\x7f\xee\xb1\x39\xcd\x7c\x3b\xde"
      "\x13\x7b\xa7\x1d\x9f\x39\x19\x94\x47\xa0\x24\x54\xb3\x91\x5c\x1b\xd7\x97"
      "\x70\x4e\x71\xf7\xda\xd1\xa4\x25\xc3\x29\x5d\xd1\x52\x49\x8c\x36\x08\xf2"
      "\x7b\xa0\x4f\x85\x3b\xfe\xbf\xd1\xd5\xf1\xe7\xe8\xe0\x58\xa6\xc3\x2b\x78"
      "\xb6\xd9\x74\xd0\xec\x21\x3d\xaf\x0e\xd9\x96\x9e\xa4\xa0\x83\xd5\xc6\x5f"
      "\x4b\xc5\x22\xa0\x86\x92\x3d\x6b\x4b\x74\x6f\xf6\x81\x2a\x18\x43\xe5\x87"
      "\x40\xab\x15\x9f\x5a\x6c\xfc\x43\x26\xda\xf8\xc2\x70\xdf\xa7\x90\x8d\x46"
      "\x9c\xed\xfc\x15\x89\x3d\x81\x0b\xc1\xd7\x03\x61\x5b\x56\x39\xde\xcd\xf0"
      "\xde\x63\x6e\xf4\x6f\xa1\xba\x52\x35\x35\x19\xa1\x66\x9f\xeb\x44\x1e\x2a"
      "\x2a\x72\xee\x73\xa3\xea\xb0\x1f\xed\x86\x59\x7b\xef\x30\x72\x96\xd4\xf5"
      "\xb5\xd5\x4c\x43\x9b\xbe\x27\xc3\x97\x95\x10\x9c\xcd\x66\x97\x50\xa4\x6a"
      "\x3a\x81\xdf\xde\x93\x41\xdd\x33\x06\x03\x2e\x5e\xb1\x99\xd8\xf0\xa2\xf9"
      "\x88\x77\x55\xb8\xcc\x87\x3b\x93\x39\x6a\x22\xfa\xed\xfd\x67\x42\x2e\x1e"
      "\xc0\x52\xbc\xfd\xa7\x1d\xa4\xde\xb6\x70\x14\x5a\xe4\x3b\x44\x25\xe4\xe0"
      "\x4d\x24\x3d\xa7\xd2\x29\xbf\x11\xc1\xc4\x7b\xdf\x0b\x30\x2a\xa7\x9d\xf2"
      "\x3a\xe9\xba\x6b\x8e\x89\xb0\x4c\x36\xc9\x55\xca\x7e\x94\xd3\xc9\x2c\xfd"
      "\xc9\x80\x4b\x0b\x42\x63\x9b\x95\x37\x2b\x9e\xf7\x1c\xd0\x4b\x01\xab\xbe"
      "\x16\x88\x42\xb1\x9e\xb3\xd7\x6e\xdf\xa4\x6c\x6c\x2b\xd4\x12\xe0\xf7\xe9"
      "\x43\x63\x82\x49\xab\x7f\x3b\xa9\x87\x2c\x86\xd4\x55\x18\xc8\x56\x60\xb9"
      "\xd1\x16\xc7\x80\x81\x5d\x46\x31\xdb\x99\xbd\x08\x39\xe7\x83\x9a\x94\x53"
      "\x58\xa2\xeb\x11\xcb\x32\xcd\x99\x42\xf6\xce\x7c\x74\x32\x7f\x91\xae\x8e"
      "\xcd\x99\x27\x34\x6f\xee\xba\xbc\xf0\x91\xdc\x5c\xb3\xa9\xde\xe4\xd5\x67"
      "\x3d\xfa\xc3\x8d\xf1\x3c\x6f\xd4\x6e\x43\x67\xbd\xce\x3a\x7f\x5b\x2d\x03"
      "\xf1\x1e\x2c\xef\xd4\x03\x17\x84\xd1\x5f\x50\x44\xb2\x51\xdf\x88\x9f\xf6"
      "\x89\x3e\x76\x6d\xb9\xd4\x37\x15\x80\x7e\x91\x20\x0a\xee\xf3\x0f\xc6\x4e"
      "\x4c\x4c\x38\x1d\x89\x5a\xe6\x46\x31\x97\xbc\x6f\x16\x68\x86\xd3\xe0\x76"
      "\xd4\x5f\xae\xf0\x0e\x82\x95\x2d\xbd\xe2\xac\xc0\xaf\x34\xf5\x6d\xcd\x2e"
      "\x3e\x25\x34\x56\x89\xe6\x67\xcb\xd4\x7c\xf8\x11\xbb\x5b\x43",
      735);
  syscall(__NR_write, (intptr_t)r[0], 0x20002580, 0xfca);
  return 0;
}
