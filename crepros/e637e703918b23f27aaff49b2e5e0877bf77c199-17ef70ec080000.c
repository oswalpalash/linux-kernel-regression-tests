// https://syzkaller.appspot.com/bug?id=e637e703918b23f27aaff49b2e5e0877bf77c199
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 2ul, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000080 = 1;
  *(uint32_t*)0x20000084 = 5;
  *(uint64_t*)0x20000088 = 0x200003c0;
  memcpy(
      (void*)0x200003c0,
      "\xbf\x16\x00\x00\x00\x00\x00\x00\xb7\x07\x00\x00\x01\x00\xf0\xff\x40\x70"
      "\x00\x00\x00\x00\x00\x00\x48\x00\x00\x00\x00\x00\xe1\xff\x95\x00\x00\x00"
      "\x00\x00\x00\x00\x2b\xa7\xe1\xd3\x0c\xb5\x99\xe8\x3f\x04\x00\x00\xf3\x00"
      "\x00\x00\x00\xbd\x01\x21\x2f\xb5\x6f\x04\x00\x26\xfb\xfe\xfc\x41\x05\x6b"
      "\xd8\x17\x4b\x79\xed\x31\x71\x42\xfa\x9e\xa4\x15\x81\x23\x75\x1c\x5c\x65"
      "\x2f\xbc\x16\x26\xcc\xa2\xa2\xad\x75\x80\x61\x50\xae\x02\x09\xe6\x2f\x51"
      "\xee\x98\x8e\x6e\x06\xc8\x20\x6a\xc6\x93\x9f\xc4\x04\x00\x00\x00\xc7\x88"
      "\xb2\x77\xbe\x1c\xb7\x9b\x0a\x4d\xcf\x23\xd4\x10\xf6\xac\xcd\x36\x41\x11"
      "\x0b\xec\x4e\x90\xa6\x34\x19\x9e\x07\xf8\xf6\xeb\x96\x8f\x20\x0e\x01\x1e"
      "\xa6\x65\xc4\x5a\x34\x49\xab\xe8\x02\xf5\xab\x3e\x89\xcf\x6c\xfd\xff\xff"
      "\xff\xb8\x58\x02\x18\xce\x74\x00\x68\x72\x00\x00\x07\x4e\x8b\x17\x15\x80"
      "\x7e\xa0\xca\x46\x9e\x46\x8e\xea\x3f\xd2\xf7\x39\x02\xeb\xcf\xcf\x49\x82"
      "\x27\x75\x98\x5b\xf3\x13\x40\x5b\x36\x7e\x81\xc7\x00\x00\x00\x40\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x53\x35\x00\x00\x00\x14\x3e\xa7\x0c\x2a"
      "\xb4\x0c\x7c\xb7\x0c\x94\x3a\x6d\x60\xd7\xc4\x90\x02\x82\xe1\x47\xd0\x8e"
      "\x0a\xf4\xb2\x9d\xf8\x14\xf5\x69\x1d\xb4\x3a\x5c\x00\x00\x00\x04\x00\x00"
      "\x00\x00\x00\x89\xfa\xff\x01\x21\x0c\xce\x39\xbf\x40\x5f\x1e\x84\x6c\x12"
      "\x42\x3a\x16\x4a\x33\xe6\x80\x84\x6f\x26\xad\x03\xdd\x65\x87\x3d\x9f\x87"
      "\x46\x3a\xd6\xf7\xc2\xe8\xee\x1a\x39\x24\x49\x60\xb3\x18\x77\x8f\x2a\x04"
      "\x7f\x6d\x5b\xc2\x4f\xef\x5d\x7d\x61\x55\x10\x2b\x1c\xed\x1e\x80\x19\xe6"
      "\x3c\x85\x0a\xf8\x95\xab\xba\x14\xf6\xfb\xd7\xfb\x5e\x2a\x43\x1a\xb9\x14"
      "\x2f\x3a\x06\xd5\x00\x00\x01\x00\x92\xc9\xf4\x60\x96\x46\xb6\xc5\xc2\x96"
      "\x47\xd2\xf9\x50\xa9\x59\xcf\x99\x38\xd6\xdf\xcb\x8e\xd2\xcb\xdc\x2b\xa9"
      "\xd5\x80\x60\x9e\x31\xc3\xfa\x90\xe7\xe5\x7a\x79\xd6\xfc\xe4\x24\xc2\x20"
      "\x0a\xf6\xc7\x78\x4a\x19\x75\xfa\x80\x7d\xe3\x8a\x3a\x61\xe4\x4a\x9e\xca"
      "\xb1\x9b\xdf\xb1\x5a\x32\xa4\xfd\x67\xce\x44\x6a\xdb\x43\x1d\x07\xdb\x79"
      "\x24\x0a\xca\xf0\x91\x23\x1b\x98\x6e\x77\xd0\x5d\x98\x8d\x6e\xdc\x71\xdf"
      "\x48\xdc\xa0\x21\x13\xa3\x83\x00\xca\xbf\x2b\x55\x43\xff\xc1\x66\x95\x57"
      "\x09\x00\x00\x00\x00\x00\x00\x00\x61\x62\x9d\x18\x22\xf7\x20\xec\x23\x81"
      "\x27\x70\xd7\x2c\x70\x0a\x44\xe1\x13\xd1\x70\x88\xfd\xd0\x06\x00\x00\x0f"
      "\x78\x89\xb8\xc7\x04\x4f\x56\x3a\x1f\x68\xd4\xf0\x01\x1a\x44\x6c\xd4\x97"
      "\x04\x00\xca\xc6\xf4\x5a\x69\x22\xde\xd2\xe2\x95\x14\xaf\x46\x3f\x74\x7c"
      "\x08\xf4\x01\x05\x86\x90\x35\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xbe"
      "\x34\xcf\x65\x2e\x28\xe7\x00\x00\x00\x00\x00\x00\x00\xb2\x44\x78\xa7\x8a"
      "\x0f\x9d\x64\x0d\xd7\x82\xac\x0c\xbc\x46\x90\x32\x43\xd0\xd0\xf4\xbc\x7f"
      "\x25\x3d\x05\x00\x00\x00\x32\xda\xaf\x28\x72\x3a\x1c\x4a\xbd\x39\x79\x9d"
      "\xd3\xb9\xde\x45\x0e\x64\xc3\x3a\xac\x8f\xf7\xe7\xd1\xc9\x4c\x45\x05\xa9"
      "\x83\x96\x88\xb0\x08\xc3\x70\x49\x4f\x67\x34\xb7\x71\x54\x6d\x95\x52\xd3"
      "\xbb\x2d\xa0\xd0\x00\x00\x00\x00\x00\x00\x00\x00\x91\x25\xc9\x7f\x00\x00"
      "\xf5\xe1\x67\x1b\xc5\xeb\x77\x39\xda\xa7\x82\x0a\x91\xcb\x0e\x73\x2d\xf2"
      "\xae\x1d\x39\xc7\x47\xe0\x0a\x4f\xbf\xe8\x94\x2f\xa8\x59\xcd\x28\xbd\xaa"
      "\x15\x09\x30\x99\x26\xc7\x7f\xbc\xb1\x5e\xc5\x8b\x42\xb4\xcb\xaf\x5a\x6b"
      "\x64\x9d\xd5\xf1\x3c\xd7\x76\xe6\xc7\xc4\xb5\xc4\xb0\xde\x30\xb6\x2c\x36"
      "\x36\x4e\x65\x05\x99\x22\x09\xbd\xbc\x62\x03\xda\x7a\x37\x97\x24\x6a\x6a"
      "\xde\xf0\x71\x10\x2f\x0a\xa2\xc4\x00\x95\xdd\xd0\x51\x76\xf5\xcb\x8b\xd9"
      "\x9e\x1b\xa0\xf9\x56\x8f\x3e\x38\x76\xbb\xa7\xbf\x97\x33\x34\xe7\x91\x9a"
      "\xb0\xaf\xfe\xd6\x22\xcd\xa6\xff\x04\xfb\x99\x6a\xd9\x19\xf7\xe9\x67\x2c"
      "\xe1\x07\x00\x00\x00\xad\x88\x2f\x2a\xea\xd1\x66\xc9\xe9\xdf\x81\xec\xaf"
      "\x5f\x74\x4f\x22\xf2\xe4\x5a\xfe\x2c\x9e\x86\x32\x27\x6c\xff\xe5\xf1\xfc"
      "\x21\x5c\x07\x97\xd0\x24\x4c\xf1\xce\x26\x9d\x10\x52\x57\x45\xca\xaa\x3f"
      "\x77\xd1\xb8\x01\x16\xcb\x9a\x38\x40\x02\x42\x1d\x89\x89\x13\xc4\x5a\x9a"
      "\xc0\x91\xa0\x11\x6f\x46\x93\x13\x31\x38\x58\x3d\xa5\xe1\x0b\x43\x46\x97"
      "\xb0\x44\x3b\x7b\x4d\xdf\xb3\xac\xe2\x9e\x16\xe5\xa8\x81\x33\x6a\xad\x09"
      "\x74\x26\x9a\x10\x25\xe2\x8b\xa1\x35\xc0\x45\xa1\x7e\x9a\x61\xc3\xb0\x64"
      "\xe6\x79\x50\x8a\xf1\xae\xc2\x92\x66\x27\xb4\x3b\xba\x12\x29\xa7\x46\x6b"
      "\xdc\xa6\x4f\x51\x4b\x79\x11\x45\x8d\xa0\x9f\xe8\x68\x19\x16\xd4\x08\xd7"
      "\x53\x22\x6a\x83\xae\x24\x34\xcc\xd3\xfc\x50\x82\x16\xae\xa8\x68\x33\x03"
      "\x0f\x56\x9d\x61\xdc\x99\x86\x20\xfc\xf4\xee\xb9\x2e\x7b\xc5\x11\xdf\x63"
      "\xc5\x3b\xb8\xf3\xc7\x4f\x44\xba\x18\x4d\x40\xe8\x76\x12\x02\x4d\xa1\xa1"
      "\xeb\xe3\x16\x92\x38\x65\xf0\x37\xc0\x1d\x71\xb5\xde\x81\x12\x10\x46\x1c"
      "\x4b\x18\xac\xb5\xcb\xea\x7e\xec\xad\x9b\x6d\xd4\x6e\xd8\x35\x15\xcd\x91"
      "\x1e\x0e\x5f\x00\x17\x9b\xe2\x5b\x59\x10\xa3\x19\x3e\x90\xbe\x23\x1a\x05"
      "\xfd\x82\xe6\x00\x39\x69\xc3\xf0\x81\xff\x1d\x0e\xb5\x0a\x04\xd1\x46\x44"
      "\x23\x48\x28\xcb\xb5\xaa\xa0\xec\xe7\x02\xab\xdd\x42\x5f\xa2\x5a\xe0\x4a"
      "\x23\x15\xc8\x90\x64\xdf\x63\x37\x00\x00\x00\x00\xd9\xe5\x95\x3e\xa6\x73"
      "\x10\x99\x3d\x01\x00\x00\x00\x00\x00\x00\x00\x3a\xc7\x53\x35\x87\x91\xb1"
      "\x49\x02\x73\xca\x53\x5e\x05\xb1\x1d\x81\x52\x37\x84\x6a\x19\xa3\x02\x93"
      "\xd2\x63\xc7\x11\x74\x3a\x5b\x79\xad\x45\xde\x2a\x3c\x91\x25\x7f\x02\xc2"
      "\xf3\x0f\x55\x13\x66\x28\x09\x07\x37\x10\x93\x7e\xd0\x05\x5b\x23\x8f\x46"
      "\x6e\x14\x42\xf8\xec\x7a\x5b\x39\x42\x28\x03\x50\x39\xce\xeb\x45\x2d\xca"
      "\x75\xf9\xff\x53\x32\xb4\xc4\x77\x7a\x41\xa0\xaa\x9a\x82\x16\x67\xc6\x85"
      "\x49\xe9\xda\x89\xad\x42\x18\xce\xa7\x44\xb3\x32\xab\x23\x2a\x09\xcf\x1e"
      "\xc3\x75\x62\x70\x74\xce\x2d\x3d\x76\x19\x93\x67\x68\xa8\x4a\x14\x65\xff"
      "\xf4\xee\xdb\xa5\x59\x55\x43\x4f\x13\x2a\xb7\xb8\x84\x05\x58\xb3\xf9\x18"
      "\xd6\x75\xa7\x99\x07\xa7\x2a\x82\x52\xcd\x3f\xba\xea\x5d\x30\x06\xa0\x35"
      "\x07\x83\x82\x31\xa3\x35\xae\x75\x9e\xd2\x55\x34\xf2\xe9\x0a\x7d\xef\x4b"
      "\x3d\x4a\xf7\xfd\x47\xab\x1a\x70\x1e\x4b\x7a\x7d\xfc\x1d\x58\x80\xaa\x76"
      "\x7e\x68\x19\x6c\x7a\xa5\xac\x11\x57\x24\xb6\xcb\x8f\xce\xbb\x67\x71\x9e"
      "\xcc\xd8\x7b\x06\xb3\x85\x66\xcf\x61\xad\x2f\x30\x7a\x79\xd2\xce\x98\x01"
      "\x83\x7b\xf0\xbd\x3a\xf0\x27\x1d\xe7\x00\xee\xf2\x79\x5d\x28\xcb\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\xe0\x52\xd9\x31\x94\x12\x1b\x77\x4d"
      "\x21\xa0\x31\x7d\x03\x46\x07\x84\x00\x00\x46\x52\xc7\x69\xfd\x3d\x1a\x2f"
      "\xb5\x11\x16\x4f\x15\x02\xab\x2a\xc4\xeb\x3f\x19\xc0\x42\x16\x3e\x0b\xdb"
      "\x88\xb8\x2d\xe3\x84\xa8\x05\x5e\x8b\x1e\x24\x29\x4b\x05\x46\xcc\xe4\x81"
      "\xff\x56\x18\xb7\xb9\x58\x5d\xbb\xbf\x22\xf3\xd6\x4f\xe8\x2e\x46\x6e\xa6"
      "\xf2\x78\x59\x94\x6e\x72\xf8\x0b\xb1\xc9\xcf\xcd\xe5\x7b\x79\x62\x5e\x29"
      "\x79\xfe\x68\x9a\x5a\x24\x6c\xbb\xdf\x6a\xd4\x88\xf4\x3f\x46\xb2\x53\x6f"
      "\x17\x5f\x46\xdf\xb2\x7d\x52\x29\x46\x72\x70\x24\xde\x0c\x59\xca\x33\x05"
      "\xe6\x68\x25\x71\x5e\x5e\x4c\xd5\xb5\x4c\x1b\x05\xc0\x9f\x04\x33\x7a\x76"
      "\xa3\x03\x73\xba\xac\x3e\xce\xc9\x1f\xd5\xc6\xeb\x7c\x32\xdb\xec\xb1\x8a"
      "\x30\x8a\x5a\xe2\xbe\x94\xdf\xab\x28\xc2\xa5\x1d\xc8\x56\xdf\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\xdf\xa4\xc1\x22\x54\xf0\x41\x80\x4f\x7f\x70"
      "\x74\x35\x67\x89\xb1\xd4\xdd\x55\xf3\xe0\x45\xa4\x82\x41\xa4\xce\x04\xd0"
      "\x6a\xcb\x2c\xf1\x1e\xab\x75\x9b\xa7\x8d\xa5\xda\x0f\x26\x12\x6d\x4c\xf2"
      "\xc7\x3e\x5f\x94\x85\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
      1708);
  *(uint64_t*)0x20000090 = 0x20000140;
  memcpy((void*)0x20000140, "GPL\000", 4);
  *(uint32_t*)0x20000098 = 0;
  *(uint32_t*)0x2000009c = 0;
  *(uint64_t*)0x200000a0 = 0;
  *(uint32_t*)0x200000a8 = 0;
  *(uint32_t*)0x200000ac = 0;
  memset((void*)0x200000b0, 0, 16);
  *(uint32_t*)0x200000c0 = 0;
  *(uint32_t*)0x200000c4 = 0;
  *(uint32_t*)0x200000c8 = -1;
  *(uint32_t*)0x200000cc = 8;
  *(uint64_t*)0x200000d0 = 0x20000000;
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x200000d8 = 0;
  *(uint32_t*)0x200000dc = 0x10;
  *(uint64_t*)0x200000e0 = 0x20000000;
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x2000000c = 0;
  *(uint32_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000ec = 0;
  *(uint32_t*)0x200000f0 = -1;
  *(uint32_t*)0x200000f4 = 0;
  *(uint64_t*)0x200000f8 = 0;
  res = syscall(__NR_bpf, 5ul, 0x20000080ul, 0x48ul);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x20002100 = r[1];
  syscall(__NR_setsockopt, r[0], 1, 0x32, 0x20002100ul, 4ul);
  *(uint64_t*)0x20000000 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint64_t*)0x20000010 = 0x20003cc0;
  *(uint64_t*)0x20003cc0 = 0x20003d00;
  memcpy((void*)0x20003d00,
         "\xd8\x00\x00\x00\x18\x00\x81\x06\x4e\x81\xf7\x82\xdb\x4c\xb9\x04\x02"
         "\x1d\x08\x00\x06\x2f\x7c\x09\xe8\xfe\x55\xa1\x0a\x00\x15\x00\x09\x00"
         "\x14\x26\x03\x60\x0e\x12\x08\x00\x1e\x00\x04\x00\x20\x01\xa8\x02\x08"
         "\x00\xa4\x00\x00\xc0\xe5\x58\xf0\x30\x03\x5c\x3b\x61\xc1\xd6\x7f\x6f"
         "\x94\x00\x71\x34\xcf\x6e\xfb\x80\x00\xa0\x07\xa2\x90\x45\x7f\x01\x89"
         "\xb3\x16\x27\x7c\xe0\x6b\xba\xce\x80\x17\xcb\xec\x4c\x2e\xe5\xa7\xce"
         "\xf4\x09\x00\x00\x00\x1f",
         108);
  *(uint64_t*)0x20003cc8 = 0x6c;
  *(uint64_t*)0x20003cd0 = 0x20003bc0;
  memcpy((void*)0x20003bc0,
         "\xbb\xdd\x48\x3a\x74\x1f\xa6\x5f\x9e\x24\xe8\x50\x57\x72\xbc\xe5\xd1"
         "\x5a\x74\x52\x58\x6b\x22\x56\x18\x6c\x0e\xd8\x62\xd3\xc6\x8e\x2f\xca"
         "\x9d\x16\xab\xf4\x00\xbe\x6a\xf5\x78\x77\x3b\xa1\x60\x0a\xbb\x69\x22"
         "\xe1\xd6\xb3\x46\x77\x2c\x82\x0d\x8e\xa0\x88\x80\x67\xb9\xb2\x4b\x61"
         "\x17\x53\xdf\xc9\x5b\xe0\x6a\x14\x7b\xad\x37\x50\x82\xd5\x39\x1d\x37"
         "\x5e\x1e\xfe\xae\xb4\xd5\x74\x41\xc1\x2e\xc9\x98\x26\xfe\x06\x3f\x96"
         "\x29\x71\x9c\xee\x63\xc7",
         108);
  *(uint64_t*)0x20003cd8 = 0x6c;
  *(uint64_t*)0x20000018 = 2;
  *(uint64_t*)0x20000020 = 0;
  *(uint64_t*)0x20000028 = 0;
  *(uint32_t*)0x20000030 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000000ul, 0ul);
  return 0;
}
