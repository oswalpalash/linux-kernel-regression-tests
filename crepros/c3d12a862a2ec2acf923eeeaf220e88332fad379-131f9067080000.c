// https://syzkaller.appspot.com/bug?id=c3d12a862a2ec2acf923eeeaf220e88332fad379
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

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[5] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x29ul, 5ul, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000100 = 1;
  *(uint32_t*)0x20000104 = 3;
  *(uint64_t*)0x20000108 = 0x20000400;
  memcpy(
      (void*)0x20000400,
      "\x18\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x26\xd0\x00\x00\x95\x00"
      "\x2b\x00\x00\x00\x00\x00\x93\xad\xff\xa8\x7d\x22\x55\xf6\x74\x41\x2d\x02"
      "\x00\x00\x00\x00\x00\x00\x5a\xb5\x27\xee\x36\x97\xf1\xec\x44\x36\xdd\x11"
      "\x64\xb9\xb3\xf4\x27\xf6\xba\x6b\x34\xf9\x81\x25\xf3\x0e\x69\x26\x99\x6a"
      "\x3c\xfe\xe3\x30\x25\xa3\x0b\x27\x36\x83\x62\x6e\x00\xdc\x25\x4d\x57\x0d"
      "\xca\x6b\x78\xad\x83\x34\x88\xcf\xe4\x10\x9e\xaf\x00\x9e\xdd\x3e\x69\x61"
      "\x3d\x3c\xd6\xaa\xa3\x00\x00\x6e\xee\x85\x01\x00\x00\x00\x52\x0a\x00\x00"
      "\x15\x1d\x01\x00\x00\x00\x01\x00\xbf\x00\x00\x00\x00\xcc\x58\x74\x24\x36"
      "\x3d\xc6\xad\x7f\x3b\xbd\x42\x4c\x6e\x6c\xaf\xbe\x93\x09\xab\xa2\x18\xa5"
      "\x20\x01\xa3\xcd\x00\x00\x3e\xee\x30\x5c\x78\x24\x21\xa0\xb3\x01\x8e\xf8"
      "\x9c\x6a\xfd\x41\xf0\xdb\x74\x59\x6f\xd7\x2c\x00\x2a\x60\xc1\xbc\x7d\xc8"
      "\xc3\x8b\x7d\x2e\x13\xc5\x04\x24\xb9\xdd\x11\x45\xd0\x3f\xf4\x5f\x70\x68"
      "\x5c\x6b\xd9\xff\x41\xc6\x9b\x7d\xe4\x75\x8c\x10\x96\xa1\xdc\x52\xf2\x9e"
      "\x47\x0a\x00\x05\x17\xeb\xc4\x06\xe8\x9d\xcb\xb7\x67\x7e\x65\x28\xb0\x85"
      "\x6e\x31\xed\x94\x74\xac\x24\xcf\x60\x90\x68\xf6\x45\xce\x97\x1f\xc0\x48"
      "\x07\x37\xa5\x5e\xbb\x0b\xd7\x01\xf7\xff\x21\xe8\x8b\x3c\xfc\x22\xdf\x01"
      "\xe4\xba\xc9\xd9\x73\x28\xfa\x2a\x82\xb5\xe8\x74\x1e\x02\x05\x6d\x93\x3b"
      "\xed\x75\x9f\xf2\x32\xce\xbc\x68\xb9\x1a\xf5\x04\x79\x38\x74\x67\x82\x42"
      "\x62\x85\x2c\x79\x39\xdb\x56\x72\xd0\x7c\xdb\xe8\xe1\x4a\xbf\x56\x49\x7e"
      "\x5d\x56\xd0\x6c\x75\x51\xb8\x70\xb2\x85\x1c\x3f\x0a\x1a\xab\x71\x58\x7a"
      "\x21\xc8\xf1\xb3\x36\xa6\xcc\xdd\x01\xb0\xf0\x4e\xdb\x25\x6c\x60\x4f\x06"
      "\x87\x73\xf6\xff\x00\x00\x00\x00\x00\x00\x00\x6f\xfb\xfe\x5c\xa3\x21\x42"
      "\xb0\x19\x55\x31\x45\x8b\x7d\x1e\x34\x1c\x6f\x86\x4f\x98\x3d\x74\x5f\x58"
      "\x65\xaa\xd4\x1d\x29\x15\xaa\xe7\x60\x2a\x2d\x6c\xd4\x15\xe8\x35\x1e\xbc"
      "\x42\x23\xf5\x4d\x6b\xec\x66\x47\x09\xff\x03\xf1\xaa\x3d\xc7\xf1\x58\x0a"
      "\xce\x9b\xf2\xaf\xd2\x8d\x71\x57\xe6\x7f\xb9\x8d\x12\x1a\xd6\xeb\x37\x27"
      "\x13\x25\x50\x12\xe0\x28\xcb\x26\x54\xd4\x93\xa0\xb4\xb3\x5f\xaa\xe1\x76"
      "\xc8\x9b\x74\x5e\xda\x29\x67\x19\x9c\xc9\x36\x85\x9a\x53\x7e\x8e\x48\x71"
      "\xd4\xac\xf3\xe3\xdc\x10\xe1\x3e\xf2\x27\xf6\x27\xa4\x00\x00\xad\x1f\xa2"
      "\x53\xd3\x3f\xa7\x4f\x17\x2d\x34\x07\xae\x4e\x1e\x34\x7c\x0c\xff\x28\x23"
      "\x5a\x3c\xbb\x5d\x33\xb0\x9b\xc3\x0c\xf2\x88\x0c\x58\x62\x72\xc3\xf4\xd7"
      "\x9b\xc3\x63\x05\x74\x5c\xb1\xcb\x38\x5e\x6a\xdd\x14\x65\x20\x03\xc7\xcd"
      "\xd3\x32\x4f\x07\xd1\x34\xd3\xed\x07\xf1\xc1\x09\x00\x00\x00\x09\xdd\x87"
      "\x2e\xc6\x6e\xa6\xc7\x18\xbb\xd1\xaa\x59\x11\x40\x00\xf0\xbe\x4c\x6f\x8d"
      "\xf0\x84\xc5\xe9\x73\x4a\xe3\x0a\xa9\xaf\xdc\x71\x9b\xf0\x1a\xb0\x3a\x9b"
      "\x10\x74\x40\x71\x36\xb4\x50\x60\x00\xf0\x91\x6a\x39\xd3\x05\x7d\x50\x18"
      "\x36\x12\xb3\x9e\x73\xae\xeb\x6e\xaf\x14\x65\x2d\xda\x68\xe9\x8e\xf9\x38"
      "\xe6\x51\x5a\x94\x51\xd9\xb7\xeb\x89\xa1\x31\x85\xf3\xf2\xd5\xae\x2c\x51"
      "\x94\x4d\xa8\xd7\x39\x2a\x6d\x6b\x97\x41\x9a\x3b\x76\x60\xdf\x4c\x51\x24"
      "\xca\x42\x5d\x37\x4b\x37\x18\x67\xa7\x9b\x31\xc6\x61\x7f\xc3\x32\x71\x91"
      "\xfb\xf5\x14\x57\x3f\x0e\x30\xd1\xd6\x0b\xe2\x16\x8f\xe6\xc2\xf3\xdc\xcd"
      "\x59\x9a\x2c\xb7\x7f\x12\x4e\x22\xf8\x76\x73\x67\x58\x05\x49\x4d\xb8\x21"
      "\xf3\x2d\x66\x25\xa9\xd5\xfd\x8c\x6b\x2a\x3a\x32\x4c\x25\x7b\x84\x00\x00"
      "\x00\xb7\x49\xcc\xd7\x40\x89\xed\x6b\x86\xf8\x1c\xa3\xd2\x47\xd8\xf7\x1d"
      "\x29\x0e\xd1\xb1\xa1\x1f\x7a\x67\x12\x51\x70\xc8\x8c\x3b\x6a\x50\x69\x63"
      "\x32\x22\x64\x01\xb1\x10\xda\x9c\x78\x6e\xec\xa2\x2d\xeb\xc9\x93\x35\x58"
      "\x3b\x54\xc1\x3c\x31\x30\x97\x8f\xa0\x69\xaf\x82\x23\xb3\x8c\xed\x73\x5c"
      "\x2d\x90\x5f\x51\xca\x85\xff\xa4\xad\xd5\x64\x74\x89\xb3\x96\x01\x27\x69"
      "\x6c\xf2\xf1\x66\x25\xc0\xc1\x02\x00\x00\x00\x00\x00\x00\x00\x9e\xf5\x21"
      "\x34\x84\x2e\x64\x17\x1f\x39\x63\x84\x10\x86\xe3\x79\x7a\x48\x25\xd0\x81"
      "\xf2\xd9\x87\xf0\x5c\x53\x41\x87\x73\x86\xec\x55\xd7\xdc\x95\x8f\xd2\x35"
      "\xd6\x07\x16\x19\xa6\x5d\x4b\x82\xd9\xc1\x62\xf3\x55\x60\x76\xb8\x05\x50"
      "\xd9\x61\xca\x74\xf1\xff\xda\xcc\xf0\xea\x5f\x02\xe0\xfc\xa8\xb2\x7f\xf3"
      "\x98\x3a\xb7\x4f\xd3\xd5\x60\x70\x0a\x1f\xbb\x44\xe7\x7e\x31\x2b\x3b\x12"
      "\x9e\x00\x03\x02\xd6\x13\x91\x6c\x9b\xcf\x9f\x00\x00\xfa\xc7\x3a\x5b\x6b"
      "\xfb\x27\xf8\x8d\xba\x81\x60\x20\xbe\x76\x0f\x7b\x45\xe0\x01\xef\xad\xa8"
      "\x00\x00\x00\x00\x00\x00\x00\xfd\xaf\x46\x60\x40\x2f\x7b\x76\x21\x89\x3b"
      "\xaa\xe6\x26\x6f\xf8\xb8\x78\xbd\x64\x3b\x79\xa4\x33\xe0\x80\x74\xea\x24"
      "\x62\x97\x4a\xb2\xcb\xd2\x47\xeb\x1c\xfa\x26\x38\xf5\x6d\xae\xe5\x7e\xd1"
      "\x4b\xc7\x4d\xe0\xfd\x87\xa9\xce\x63\x81\x90\xf3\x57\x0e\x0b\x4c\x80\xef"
      "\x68\x2d\xf2\x22\x37\x27\x09\x55\xaf\xb6\x00\x88\x46\x55\x7e\xe3\xbc\x09"
      "\xfd\xa6\xdb\xb6\x54\x2e\x59\x73\x00\xeb\x82\xa1\x84\xc9\x6f\xfd\xe5\xa3"
      "\x0e\x54\x33\xd8\x66\x66\xcb\x04\x5b\xdd\x02\xc8\x04\xc2\x2f\xf2\x63\x5c"
      "\x7b\xfb\xf5\xc0\xd5\x86\xcd\xa5\xe1\xe8\x8a\x4d\x41\xde\xe7\xcc\x74\xf8"
      "\x22\x27\x8d\x12\x46\x38\xfe\xc5\x8f\xae\xb4\x8a\xfe\x32\x43\x69\xcc\x51"
      "\x20\x41\x58\xbb\x44\x0d\xf2\xa6\x94\xf4\xcd\xca\xa4\xf6\x5c\x22\xf0\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xd5\x03\xd7\x99\x06\x95"
      "\x81\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x1f\xff\xf0\xef\x89\xb2"
      "\xa6\x35\xed\xb2\xdd\x16\x3e\x86\x33\x15\xe8\x44\x98\xdf\xb5\x2b\x7f\x54"
      "\xda\x63\x98\xcb\xed\xaa\x42\xcc\x17\xc4\x56\x3c\x85\x96\x56\xa3\x57\x77"
      "\x02\x89\xa6\x1f\xaa\x95\xa8\x2b\xf1\xcf\xb7\xf2\xfd\x72\x52\xe9\x32\x2a"
      "\xbe\x28\x2c\x33\x44\xfc\x67\x38\xb4\x46\x78\x93\xb9\xbf\x0d\x1c\x81\x30"
      "\xae\x6b\x22\x69\x00\x11\x06\x35\x37\x64\x13\xc2\x9f\x7c\x6f\x7b\x7e\x29"
      "\xb9\xf4\xbd\xdd\x5e\x32\x86\x61\xf4\x04\x6e\x01\xf7\xd7\xdc\x22\x17\x4e"
      "\x5e\x62\x7a\x6f\x60\x8a\xd5\x3a\x41\x68\xd4\xd8\xf7\xfb\xc7\x11\x04\x51"
      "\x2e\xfe\x8e\x5d\x7d\x93\x4a\xa2\x89\xb4\xdb\x2b\x87\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x9b\x77"
      "\x78\x83\xa0\x2f\x05\x93\xdf\xc4\xcb\x41\x14\xb9\xf9\xcf\x4a\xd1\x55\x11"
      "\x0c\xd3\xac\xe2\xb3\x22\xac\x31\xbf\xa2\x78\x47\xc7\x99\xc8\xa6\x9a\x1e"
      "\xa5\xb9\x8e\x52\x5e\x63\x83\xad\x7f\xd9\x79\x51\x70\xe7\xb1\x1e\x4f\xa9"
      "\x90\xb9\x38\x69\x10\xa6\xa1\xa6\x6a\x70\xea\xff\x01\x24\x76\x03\xc2\xff"
      "\x49\xd3\x97\x96\x76\xbf\xfb\x30\x49\x16\x6a\xb8\x4a\x0f\x06\x19\x91\xbd"
      "\x57\xc2\x56\x6c\x10\xc2\x82\x35\x2a\x51\x05\xb6\x16\x4e\x3f\x24\x91\xe4"
      "\x79\x3e\x59\x0d\xcc\x71\xde\x10\xda\x96\xfd\xff\x40\xdd\x44\xa2\xc9\x88"
      "\x2d\x3a\xa0\xf8\xa7\x97\xb8\xfe\xa6\xef\xcf\xb5\x27\x6b\x76\x79\xf1\x55"
      "\x59\xcd\xaa\x97\x75\x04\xcc\x0b\x2f\x77\x7a\xcb\x90\x7e\xbf\x5f\xc1\x4a"
      "\xdd\x71\xd0\xbc\xa3\x74\x05\xde\xd6\x9b\x77\xab\x4a\x3d\x74\x87\xfd\x50"
      "\xc5\xe2\x2a\xde\x17\x55\x6a\xbb\x72\x2d\x9c\x08\x5b\x18\x9b\x5f\xd1\xf3"
      "\x0e\x8d\xc8\x13\xf6\x08\x70\xfd\xe1\xf8\x8d\x83\x0b\x11\x00\x21\x35\xe8"
      "\xe7\x26\x2f\x29\x9e\xd7\x92\x3b\xfb\xe0\x0a\xd8\x8b\xe1\x79\xe5\x6b\x41"
      "\xff\x37\x92\xce\xe2\xfc\x37\xee\xe7\x39\xc3\xe3\xaf\x92\x3e\x87\x38\xd9"
      "\x3d\x58\x3a\x9c\xf0\x0b\x94\x69\x60\xfc\x38\xcf\x85\xaa\xe7\xcf\x70\x8f"
      "\x9a\x9d\x16\x6f\x2e\x35\x2a\x06\xd9\x9b\x8b\xe4\x76\xd1\xcc\x2a\x53\xa8"
      "\x59\xae\x4f\xda\xb2\xa9\x87\x92\x5d\x12\x42\x24\x74\xac\x04\x4f\xfe\x9f"
      "\xe2\xbf\x9b\xf9\xbb\xdf\x36\xc4\xca\x89\xc5\x16\x64\x75\x42\xac\x45\x54"
      "\x53\x37\x82\x9d\xa7\x03\x9d\x15\x5e\xbd\xa4\x2d\x4c\x14\xf4\xca\x7f\x8b"
      "\x5d\x58\x42\x65\x8c\x62\xd0\xa0\x30\x92\xb9\x4f\xa1\xb1\x9f\x19\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x9e\x75\xa3\x2b\x9f"
      "\xaf\xef\xfd\x89\x0f\x27\x59\xb0\xfe\x3a\xdd\x33\x5d\x43\xa4\xc3\x99\x54"
      "\x58\xf8\x6a\x92\x6a\xd5\x6b\x23\x57\x1c\x46\x72\x8c\x03\x9c\xd3\xb4\xbb"
      "\x7d\x69\xdf\xa2\x77\x82\xb9\x53\xa7\xb8\x1c\xc1\x61\x91\x2b\x3e\x57\x16"
      "\x36\x06\x86\xe1\x26\x31\x1a\x7e\x21\xbf\xa2\xef\xd0\xf5\x7b\x90\xc2\x03"
      "\x52\x8c\x8f\x62\x0d\x3c\x7b\x31\xc7\xab\xcf\xfa\xe3\x82\xf5\x35\x00\xf7"
      "\xcd\x5d\x00\x15\x9e\x5f\x74\x1d\x3e\x2d\x2c\xbd\x1a\x04\xb3\xf3\x9b\x51"
      "\xa4\x68\x3d\xaa\x7d\x11\x7b\x7f\x4a\x14\x9c\x95\x4d\x69\xd8\xab\x00\x13"
      "\x39\xe4\x64\xc8\xeb\x5f\x0c\x63\x89\x90\x10\x75\x7c\x9a\x3b\x69\xf4\x92"
      "\x05\x31\xb8\x3f\x71\xd5\xa3\x4e\xf9\x40\x58\x19\xaf\xee\x15\xb7\x7c\x01"
      "\x5e\xa7\x55\xc9\x51\x27\xff\x22\x74\xbb\x9a\x84\x63\xce\x4b\x8c\x08\xad"
      "\x70\x59\x6a\xd2\xb2\xb0\x44\xe6\x60\xed\x14\x4b\x9d\xce\x37\x24\x50\xea"
      "\x69\xd2\x5d\xa2\xb6\xde\xed\x67\xfa\xc2\x6e\x76\x5a\xa7\xd5\x53\x2b\xa1"
      "\x04\x4f\x62\xdb\x04\x94\x86\xac\xde\x22\x94\x12\x7c\xb7\x67\xc2\x3d\xa7"
      "\xd8\xf9\x84\x4d\x3b\xe5\xb6\xaa\x83\xee\x4c\xe1\x87\x6a\xf5\x13\x0e\xfe"
      "\x1b\x64\xcc\xb6\xbb\xd3\x49\xbc\xc0\xe8\xde\xec\x8a\xb3\xbd\x1b\x35\xbb"
      "\xc8\xab\x8a\x15\x27\x71\x74\x4b\xaa\x57\x6b\x92\x23\xd2\x6b\x56\x03\xa7"
      "\xf0\x91\xbe\x12\x64\xca\xba\xf6\x61\xfe\x2d\xbe\x79\x90\xa6\x1f\x71\x0f"
      "\x92\x3f\x23\x37\x81\x8a\x39\x83\xd0\x6c\x11\xa6\xbe\xe7\xfc\xcb\x78\xa5"
      "\x3c\x56\xdb\x5c\x18\xf9\x20\xd2\x19\x43\x74\xdb\x66\x5d\xca\xdf\x53\xb8"
      "\xd0\x01\x4e\x68\x2e\xc7\x21\xd6\x7a\x7a\xb6\xc8\x17\xfe\x53\xc8\x6f\x89"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x60\xb7\xb8"
      "\x27\xc5\x6e\x97\x3a\x2a\xb5\xbc\x5c\x06\x00\x00\x00\x00\x00\x00\x00\xf5"
      "\x95\x7b\x20\xb9\x19\xaf\x5d\x53\xc8\x7d\xe0\x56\xa3\x97\xbd\xcb\x61\x4c"
      "\x34\x76\x1e\x2c\x81\x56\x98\xe1\xf9\xf5\x52\x1a\x38\x5c\x29\x10\x85\x09"
      "\x29\x04\x0a\x4e\xba\x57\x3e\x91\xca\x21\xfc\x85\x53\x58\x12\x0e\xcd\x79"
      "\xa5\xd7\x00\x76\x93\xef\x3f\xf9\xd2\xb9\x93\xd1\x14\x44\x3d\x53\xc5\x30"
      "\x94\xe5\x16\xf6\x75\xb2\xa7\x07\x45\x84\x71\x4e\x7a\x20\x15\xe0\x5e\x50"
      "\x78\x11\xb4\xca\x89\xc3\x92\x81\xc9\xad\xa5\xf5\x8c\xeb\x55\x89\x3c\xca"
      "\x78\x3a\xb0\x9c\x9a\x19\x83\x6a\x3a\x2c\x71\x5b\x10\x43\x6a\x57\x31\x54"
      "\x9e\x36\x46\x79\xec\xd8\x46\x1a\x68\x43\x3a\xb5\x2b\x11\x08\x83\x1e\xdb"
      "\x96\x54\xdc\x60\x21\x83\xc1\x17\x0d\x68\x81\x64\x7f\x6d\xca\x15\xd5",
      2321);
  *(uint64_t*)0x20000110 = 0x202bf000;
  memcpy((void*)0x202bf000, "syzkaller\000", 10);
  *(uint32_t*)0x20000118 = 4;
  *(uint32_t*)0x2000011c = 0x436;
  *(uint64_t*)0x20000120 = 0x20000040;
  *(uint32_t*)0x20000128 = 0;
  *(uint32_t*)0x2000012c = 0;
  memset((void*)0x20000130, 0, 16);
  *(uint32_t*)0x20000140 = 0;
  *(uint32_t*)0x20000144 = 0;
  *(uint32_t*)0x20000148 = -1;
  *(uint32_t*)0x2000014c = 8;
  *(uint64_t*)0x20000150 = 0x20000000;
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000158 = 7;
  *(uint32_t*)0x2000015c = 0x10;
  *(uint64_t*)0x20000160 = 0x20000000;
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x2000000c = 0;
  *(uint32_t*)0x20000168 = 0;
  *(uint32_t*)0x2000016c = 0;
  *(uint32_t*)0x20000170 = -1;
  *(uint32_t*)0x20000174 = 0;
  *(uint64_t*)0x20000178 = 0;
  res = syscall(__NR_bpf, 5ul, 0x20000100ul, 0x48ul);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_socket, 0x29ul, 5ul, 0);
  if (res != -1)
    r[2] = res;
  res = syscall(__NR_socket, 0xaul, 0x803ul, 6);
  if (res != -1)
    r[3] = res;
  *(uint16_t*)0x200003c0 = 0xa;
  *(uint16_t*)0x200003c2 = htobe16(0);
  *(uint32_t*)0x200003c4 = htobe32(0);
  memset((void*)0x200003c8, 0, 16);
  *(uint32_t*)0x200003d8 = 0;
  syscall(__NR_connect, r[3], 0x200003c0ul, 0x1cul);
  *(uint32_t*)0x20000180 = r[3];
  *(uint32_t*)0x20000184 = r[1];
  syscall(__NR_ioctl, r[2], 0x89e0, 0x20000180ul);
  *(uint32_t*)0x20000100 = 1;
  *(uint32_t*)0x20000104 = 3;
  *(uint64_t*)0x20000108 = 0x20000e80;
  memcpy(
      (void*)0x20000e80,
      "\x18\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x26\xd0\x00\x00\x95\x00"
      "\x2b\x00\x00\x00\x00\x00\x93\xad\xff\xa8\x7d\x22\x55\xf6\x74\x41\x2d\x02"
      "\x00\x00\x00\x00\x00\x00\x5a\xb5\x27\xee\x36\x97\xf1\xec\x44\x36\xdd\x11"
      "\x64\xb9\xb3\xf4\x27\xf6\xba\x6b\x34\xf9\x81\x25\xf3\x0e\x69\x26\x99\x6a"
      "\x3c\xfe\xe3\x30\x25\xa3\x0b\x27\x36\x83\x62\x6e\x00\xdc\x25\x4d\x57\x0d"
      "\xca\x6b\x78\xad\x83\x34\x88\xcf\xe4\x10\x9e\xaf\x00\x9e\xdd\x3e\x69\x61"
      "\x3d\x3c\xd6\xaa\xa3\x00\x00\x6e\xee\x85\x01\x00\x00\x00\x52\x0a\x00\x00"
      "\x15\x1d\x01\x00\x00\x00\x01\x00\xbf\x00\x00\x00\x00\xcc\x58\x74\x24\x36"
      "\x3d\xc6\xad\x7f\x3b\xbd\x42\x4c\x6e\x6c\xaf\xbe\x93\x09\xab\xa2\x18\xa5"
      "\x20\x01\xa3\xcd\x00\x00\x3e\xee\x30\x5c\x78\x24\x21\xa0\xb3\x01\x8e\xf8"
      "\x9c\x6a\xfd\x41\xf0\xdb\x74\x59\x6f\xd7\x2c\x00\x2a\x60\xc1\xbc\x7d\xc8"
      "\xc3\x8b\x7d\x2e\x13\xc5\x04\x24\xb9\xdd\x11\x45\xd0\x3f\xf4\x5f\x70\x68"
      "\x5c\x6b\xd9\xff\x41\xc6\x9b\x7d\xe4\x75\x8c\x10\x96\xa1\xdc\x52\xf2\x9e"
      "\x47\x0a\x00\x05\x17\xeb\xc4\x06\xe8\x9d\xcb\xb7\x67\x7e\x65\x28\xb0\x85"
      "\x6e\x31\xed\x94\x74\xac\x24\xcf\x60\x90\x68\xf6\x45\xce\x97\x1f\xc0\x48"
      "\x07\x37\xa5\x5e\xbb\x0b\xd7\x01\xf7\xff\x21\xe8\x8b\x3c\xfc\x22\xdf\x01"
      "\xe4\xba\xc9\xd9\x73\x28\xfa\x2a\x82\xb5\xe8\x74\x1e\x02\x05\x6d\x93\x3b"
      "\xed\x75\x9f\xf2\x32\xce\xbc\x68\xb9\x1a\xf5\x04\x79\x38\x74\x67\x82\x42"
      "\x62\x85\x2c\x79\x39\xdb\x56\x72\xd0\x7c\xdb\xe8\xe1\x4a\xbf\x56\x49\x7e"
      "\x5d\x56\xd0\x6c\x75\x51\xb8\x70\xb2\x85\x1c\x3f\x0a\x1a\xab\x71\x58\x7a"
      "\x21\xc8\xf1\xb3\x36\xa6\xcc\xdd\x01\xb0\xf0\x4e\xdb\x25\x6c\x60\x4f\x06"
      "\x87\x73\xf6\xff\x00\x00\x00\x00\x00\x00\x00\x6f\xfb\xfe\x5c\xa3\x21\x42"
      "\xb0\x19\x55\x31\x45\x8b\x7d\x1e\x34\x1c\x6f\x86\x4f\x98\x3d\x74\x5f\x58"
      "\x65\xaa\xd4\x1d\x29\x15\xaa\xe7\x60\x2a\x2d\x6c\xd4\x15\xe8\x35\x1e\xbc"
      "\x42\x23\xf5\x4d\x6b\xec\x66\x47\x09\xff\x03\xf1\xaa\x3d\xc7\xf1\x58\x0a"
      "\xce\x9b\xf2\xaf\xd2\x8d\x71\x57\xe6\x7f\xb9\x8d\x12\x1a\xd6\xeb\x37\x27"
      "\x13\x25\x50\x12\xe0\x28\xcb\x26\x54\xd4\x93\xa0\xb4\xb3\x5f\xaa\xe1\x76"
      "\xc8\x9b\x74\x5e\xda\x29\x67\x19\x9c\xc9\x36\x85\x9a\x53\x7e\x8e\x48\x71"
      "\xd4\xac\xf3\xe3\xdc\x10\xe1\x3e\xf2\x27\xf6\x27\xa4\x00\x00\xad\x1f\xa2"
      "\x53\xd3\x3f\xa7\x4f\x17\x2d\x34\x07\xae\x4e\x1e\x34\x7c\x0c\xff\x28\x23"
      "\x5a\x3c\xbb\x5d\x33\xb0\x9b\xc3\x0c\xf2\x88\x0c\x58\x62\x72\xc3\xf4\xd7"
      "\x9b\xc3\x63\x05\x74\x5c\xb1\xcb\x38\x5e\x6a\xdd\x14\x65\x20\x03\xc7\xcd"
      "\xd3\x32\x4f\x07\xd1\x34\xd3\xed\x07\xf1\xc1\x09\x00\x00\x00\x09\xdd\x87"
      "\x2e\xc6\x6e\xa6\xc7\x18\xbb\xd1\xaa\x59\x11\x40\x00\xf0\xbe\x4c\x6f\x8d"
      "\xf0\x84\xc5\xe9\x73\x4a\xe3\x0a\xa9\xaf\xdc\x71\x9b\xf0\x1a\xb0\x3a\x9b"
      "\x10\x74\x40\x71\x36\xb4\x50\x60\x00\xf0\x91\x6a\x39\xd3\x05\x7d\x50\x18"
      "\x36\x12\xb3\x9e\x73\xae\xeb\x6e\xaf\x14\x65\x2d\xda\x68\xe9\x8e\xf9\x38"
      "\xe6\x51\x5a\x94\x51\xd9\xb7\xeb\x85\xf3\xf2\xd5\xae\x2c\x51\x94\x4d\xa8"
      "\xd7\x39\x1d\x6d\x6b\x97\x41\x9a\x3b\x76\x60\xdf\x4c\x51\x24\xca\x42\x5d"
      "\x37\x4b\x37\x18\x67\xa7\x9b\x31\xc6\x61\x7f\xc3\x32\x71\x91\xfb\xf5\x14"
      "\x57\x3f\x0e\x30\xd1\xd6\x0b\xe2\x16\x8f\xe6\xc2\xf3\xdc\xcd\x59\x9a\x2c"
      "\xb7\x7f\x12\x4e\x22\xf8\x76\x73\x67\x58\x05\x49\x4d\xb8\x21\xf3\x9b\x50"
      "\xd9\x38\xd5\xfd\x8c\x6b\x2a\x3a\x32\x4c\x25\x7b\x84\x00\x00\x00\xb7\x49"
      "\xcc\xd7\x40\x89\xed\x6b\x86\xf8\x1c\xa3\xd2\x47\xd8\xf7\x1d\x29\x0e\xd1"
      "\xb1\xa1\x1f\x7a\x67\x12\x51\x70\xc8\x8c\x3b\x6a\x50\x69\x63\x32\x22\x64"
      "\x01\xb1\x10\xda\x9c\x78\x6e\xec\xa2\x2d\xeb\xc9\x93\x35\x58\x3b\x54\xc1"
      "\x3c\x31\x30\x97\x8f\xa0\x69\xaf\x82\x23\xb3\x8c\xed\x73\x5c\x2d\x90\x5f"
      "\x51\xca\x85\xff\xa4\xad\xd5\x64\x74\x89\xb3\x96\x01\x27\x69\x6c\xf2\xf1"
      "\x66\x25\xc0\xc1\x02\x00\x00\x00\x00\x00\x00\x00\x9e\xf5\x21\x34\x84\x2e"
      "\x64\x17\x1f\x39\x63\x84\x10\x86\xe3\x79\x7a\x48\x25\xd0\x81\xf2\xd9\x87"
      "\xf0\x5c\x53\x41\x87\x73\x86\xec\x55\xd7\xdc\x95\x8f\xd2\x35\xd6\x07\x16"
      "\x19\xa6\x5d\x4b\x82\xd9\xc1\x62\xf3\x55\x60\x76\xb8\x05\x50\xd9\x61\xca"
      "\x74\xf1\xff\xda\xcc\xf0\xea\x5f\x02\xe0\xfc\xa8\xb2\x7f\xf3\x98\x3a\xb7"
      "\x4f\xd3\xd5\x60\x70\x0a\x1f\xbb\x44\xe7\x7e\x31\x2b\x3b\x12\x9e\x00\x03"
      "\x02\xd6\x13\x91\x6c\x9b\xcf\x9f\x00\x00\xfa\xc7\x3a\x5b\x6b\xfb\x27\xf8"
      "\x8d\xba\x81\x60\x20\xbe\x76\x0f\x7b\x45\xe0\x01\xef\xad\xa8\x00\x00\x00"
      "\x00\x00\x00\x00\xfd\xaf\x46\x60\x40\x2f\x7b\x3b\x79\xa4\x33\xe0\x80\x74"
      "\xea\x24\x62\x97\x4a\xb2\xcb\xd2\x47\xeb\x1c\xfa\x26\x38\xf5\x6d\xae\xe5"
      "\x7e\xd1\x4b\xc7\x4d\xe0\xfd\x87\xa9\xce\x63\x81\x90\xf3\x57\x0e\x0b\x4c"
      "\x80\xef\x68\x2d\xf2\x22\x37\x27\x09\x55\xaf\xb6\x00\x88\x46\x55\x7e\xe3"
      "\xbc\x09\xfd\xa6\xdb\xb6\x54\x2e\x59\x73\x00\xeb\x82\xa1\x84\xc9\x6f\xfd"
      "\xe5\xa3\x0e\x54\x33\xd8\x66\x66\xcb\x04\x5b\xdd\x02\xc8\x04\xc2\x2f\xf2"
      "\x63\x5c\x7b\xfb\xf5\xc0\xd5\x86\xcd\xa5\xe1\xe8\x8a\x4d\x41\xde\xe7\xcc"
      "\x74\xf8\x22\x27\x8d\x12\x46\x38\xfe\xc5\x8f\xae\xb4\x8a\xfe\x32\x43\x69"
      "\xcc\x51\x20\x41\x58\xbb\x44\x0d\xf2\xa6\x94\xf4\xcd\xca\xa4\xf6\x5c\x22"
      "\xf0\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xd5\x03\xd7\x99"
      "\x06\x95\x81\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x1f\xff\xf0\xef"
      "\x89\xb2\xa6\x35\xed\xb2\xdd\x16\x3e\x86\x33\x15\xe8\x44\x98\xdf\xb5\x2b"
      "\x7f\x54\xda\x63\x98\xcb\xed\xaa\x42\xcc\x17\xc4\x56\x3c\x85\x96\x56\xa3"
      "\x57\x77\x02\x89\xa6\x1f\xaa\x95\xa8\x2b\xf1\xcf\xb7\xf2\xfd\x72\x52\xe9"
      "\x32\x2a\xbe\x28\x2c\x33\x44\xfc\x67\x38\xb4\x46\x78\x93\xb9\xbf\x0d\x1c"
      "\x81\x30\xae\x6b\x22\x69\x00\x11\x06\x35\x37\x64\x13\xc2\x9f\x7c\x6f\x7b"
      "\x7e\x29\xb9\xf4\xbd\xdd\x5e\x32\x86\x61\xf4\x04\x6e\x01\xf7\xd7\xdc\x22"
      "\x17\x4e\x5e\x62\x7a\x6f\x60\x8a\xd5\x3a\x41\x68\xd4\xd8\xf7\xfb\xc7\x11"
      "\x04\x51\x2e\xfe\x8e\x5d\x7d\x93\x4a\xa2\x89\xb4\xdb\x2b\x87\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x9b\x77\x78\x83\xa0\x2f\x05\x93\xdf\xc4\xcb\x41\x14\xb9\xf9\xcf\x4a\xd1"
      "\x55\x11\x0c\xd3\xac\xe2\xb3\x22\xac\x31\xbf\xa2\x78\x47\xc7\x99\xc8\xa6"
      "\x9a\x1e\xa5\xb9\x8e\x52\x5e\x63\x83\xad\x7f\xd9\x79\x51\x70\xe7\xb1\x1e"
      "\x4f\xa9\x90\xb9\x38\x69\x10\xa6\xa1\xa6\x6a\x70\xea\xff\x01\x24\x76\x03"
      "\xc2\xff\x49\xd3\x97\x96\x76\xbf\xfb\x30\x49\x16\x6a\xb8\x4a\x0f\x06\x19"
      "\x91\xbd\x57\xc2\x56\x6c\x10\xc2\x82\x35\x2a\x51\x05\xb6\x16\x4e\x3f\x24"
      "\x91\xe4\x79\x3e\x59\x0d\xcc\x71\xde\x10\xda\x96\xfd\xff\x40\xdd\x44\xa2"
      "\xc9\x88\x2d\x3a\xa0\xf8\xa7\x97\xb8\xfe\xa6\xef\xcf\xb5\x27\x6b\x76\x79"
      "\xf1\x55\x59\xcd\xaa\x97\x75\x04\xcc\x0b\x2f\x77\x7a\xcb\x90\x7e\xbf\x5f"
      "\xc1\x4a\xdd\x71\xd0\xbc\xa3\x74\x05\xde\xd6\x9b\x77\xab\x4a\x3d\x74\x87"
      "\xfd\x50\xc5\xe2\x2a\xde\x17\x55\x6a\xbb\x72\x2d\x9c\x08\x5b\x18\x9b\x5f"
      "\xd1\xf3\x0e\x8d\xc8\x13\xf6\x08\x70\xfd\xe1\xf8\x8d\x83\x0b\x11\x00\x21"
      "\x35\xe8\xe7\x26\x2f\x29\x9e\xd7\x92\x3b\xfb\xe0\x0a\xd8\x8b\xe1\x79\xe5"
      "\x6b\x41\xff\x37\x92\xce\xe2\xfc\x37\xee\xe7\x39\xc3\xe3\xaf\x92\x3e\x87"
      "\x38\xd9\x3d\x58\x3a\x9c\xf0\x0b\x94\x69\x60\xfc\x38\xcf\x85\xaa\xe7\xcf"
      "\x70\x8f\x9a\x9d\x16\x6f\x2e\x35\x2a\x06\xd9\x9b\x8b\xe4\x76\xd1\xcc\x2a"
      "\x53\xa8\x59\xae\x4f\xda\xb2\xa9\x87\x92\x5d\x12\x42\x24\x74\xac\x04\x4f"
      "\xfe\x9f\xe2\xbf\x9b\xf9\xbb\xdf\x36\xc4\xca\x89\xc5\x16\x64\x75\x42\xac"
      "\x45\x54\x53\x37\x82\x9d\xa7\x03\x9d\x15\x5e\xbd\xa4\x2d\x4c\x14\xf4\xca"
      "\x7f\x8b\x5d\x58\x42\x65\x8c\x62\xd0\xa0\x30\x92\xb9\x4f\xa1\xb1\x9f\x19"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x9e\x75\xa3"
      "\x2b\x9f\xaf\xef\xfd\x89\x0f\x27\x59\xb0\xfe\x3a\xdd\x33\x5d\x43\xa4\xc3"
      "\x99\x54\x58\xf8\x6a\x92\x6a\xd5\x6b\x23\x57\x1c\x46\x72\x8c\x03\x9c\xd3"
      "\xb4\xbb\x7d\x69\xdf\xa2\x77\x82\xb9\x53\xa7\xb8\x1c\xc1\x61\x91\x2b\x3e"
      "\x57\x16\x36\x06\x86\xe1\x26\x31\x1a\x7e\x21\xbf\xa2\xef\xd0\xf5\x7b\x90"
      "\xc2\x03\x52\x8c\x8f\x62\x0d\x3c\x7b\x31\xc7\xab\xcf\xfa\xe3\x82\xf5\x35"
      "\x00\xf7\xcd\x5d\x00\x15\x9e\x5f\x74\x1d\x3e\x2d\x2c\xbd\x1a\x04\xb3\xf3"
      "\x9b\x51\xa4\x68\x3d\xaa\x7d\x11\x7b\x7f\x4a\x14\x9c\x95\x4d\x69\xd8\xab"
      "\x00\x13\x39\xe4\x64\xc8\xeb\x5f\x0c\x63\x89\x90\x10\x75\x7c\x9a\x3b\x69"
      "\xf4\x92\x05\x31\xb8\x3f\x71\xd5\xa3\x4e\xf9\x40\x58\x19\xaf\xee\x15\xb7"
      "\x7c\x01\x5e\xa7\x55\xc9\x51\x27\xff\x22\x74\xbb\x9a\x84\x63\xce\x4b\x8c"
      "\x08\xad\x70\x59\x6a\xd2\xb2\xb0\x44\xe6\x60\xed\x14\x4b\x9d\xce\x37\x24"
      "\x50\xea\x69\xd2\x5d\xa2\xb6\xde\xed\x67\xfa\xc2\x6e\x76\x5a\xa7\xd5\x53"
      "\x2b\xa1\x04\x4f\x62\xdb\x04\x94\x86\xac\xde\x22\x94\x12\x7c\xb7\x67\xc2"
      "\x3d\xa7\xd8\xf9\x84\x4d\x3b\xe5\xb6\xaa\x83\xee\x4c\xe1\x87\x6a\xf5\x13"
      "\x0e\xfe\x1b\x64\xcc\xb6\xbb\xd3\x49\xbc\xc0\xe8\xde\xec\x8a\xb3\xbd\x1b"
      "\x35\xbb\xc8\xab\x8a\x15\x27\x71\x74\x4b\xaa\x57\x6b\x92\x23\xd2\x6b\x56"
      "\x03\xa7\xf0\x91\xbe\x12\x64\xca\xba\xf6\x61\xfe\x2d\xbe\x79\x90\xa6\x1f"
      "\x71\x0f\x92\x3f\x23\x37\x81\x8a\x39\x83\xd0\x6c\x11\xa6\xbe\xe7\xfc\xcb"
      "\x78\xa5\x3c\x56\xdb\x5c\x18\xf9\x20\xd2\x19\x43\x74\xdb\x66\x5d\xca\xdf"
      "\x53\xb8\xd0\x01\x4e\x68\x2e\xc7\x21\xd6\x7a\x7a\xb6\xc8\x17\xfe\x53\xc8"
      "\x6f\x89\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x60"
      "\xb7\xb8\x27\xc5\x6e\x97\x3a\x2a\xb5\xbc\x5c\x06\x00\x00\x00\x00\x00\x00"
      "\x00\xf5\x95\x7b\x20\xb9\x19\xaf\x5d\x53\xc8\x7d\xe0\x56\xa3\x97\xbd\xcb"
      "\x61\x4c\x34\x76\x1e\x2c\x81\x56\x98\xe1\xf9\xf5\x52\x1a\x38\x5c\x29\x10"
      "\x85\x09\x29\x04\x0a\x4e\xba\x57\x3e\x91\xca\x21\xfc\x85\x53\x58\x12\x0e"
      "\xcd\x79\xa5\xd7\x00\x76\x93\xef\x3f\xf9\xd2\xb9\x93\xd1\x14\x44\x3d\x53"
      "\xc5\x30\x94\xe5\x16\xf6\x75\xb2\xa7\x07\x45\x84\x71\x4e\x7a\x20\x15\xe0"
      "\x5e\x50\x78\x11\xb4\xca\x89\xc3\x92\x81\xc9\xad\xa5\xf5\x8c\xeb\x55\x89"
      "\x3c\xca\x78\x3a\xb0\x9c\x9a\x19\x83\x6a\x3a\x2c\x71\x5b\x10\x43\x6a\x57"
      "\x31\x54\x9e\x36\x46\x79\xec\xd8\x46\x1a\x68\x43\x3a\xb5\x2b\x11\x08\x83"
      "\x1e\xdb\x96\x54\xdc\x60\x21\x83\xc1\x17\x0d\x68\x81\x64\x7f\x6d\xca\x15"
      "\xd5\x7f\xb7\x63\x57\xd8\x15\xc5\xf1\x00\x00\x00\x00\x00",
      2318);
  *(uint64_t*)0x20000110 = 0x202bf000;
  memcpy((void*)0x202bf000, "syzkaller\000", 10);
  *(uint32_t*)0x20000118 = 4;
  *(uint32_t*)0x2000011c = 0x436;
  *(uint64_t*)0x20000120 = 0x20000040;
  *(uint32_t*)0x20000128 = 0;
  *(uint32_t*)0x2000012c = 0;
  memset((void*)0x20000130, 0, 16);
  *(uint32_t*)0x20000140 = 0;
  *(uint32_t*)0x20000144 = 0;
  *(uint32_t*)0x20000148 = -1;
  *(uint32_t*)0x2000014c = 8;
  *(uint64_t*)0x20000150 = 0x20000000;
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000158 = 7;
  *(uint32_t*)0x2000015c = 0x10;
  *(uint64_t*)0x20000160 = 0x20000000;
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x2000000c = 0;
  *(uint32_t*)0x20000168 = 0;
  *(uint32_t*)0x2000016c = 0;
  *(uint32_t*)0x20000170 = -1;
  *(uint32_t*)0x20000174 = 0;
  *(uint64_t*)0x20000178 = 0;
  res = syscall(__NR_bpf, 5ul, 0x20000100ul, 0x48ul);
  if (res != -1)
    r[4] = res;
  *(uint32_t*)0x200000c0 = r[3];
  *(uint32_t*)0x200000c4 = r[4];
  syscall(__NR_ioctl, r[0], 0x89e0, 0x200000c0ul);
  return 0;
}
