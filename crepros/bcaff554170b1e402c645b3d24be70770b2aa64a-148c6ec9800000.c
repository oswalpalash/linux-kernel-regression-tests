// https://syzkaller.appspot.com/bug?id=bcaff554170b1e402c645b3d24be70770b2aa64a
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#ifndef __NR_sendmsg
#define __NR_sendmsg 370
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[1];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 0x10, 3, 0x10);
  *(uint32_t*)0x20b3dfc8 = 0x20d49ff4;
  *(uint32_t*)0x20b3dfcc = 0xc;
  *(uint32_t*)0x20b3dfd0 = 0x20007000;
  *(uint32_t*)0x20b3dfd4 = 1;
  *(uint32_t*)0x20b3dfd8 = 0;
  *(uint32_t*)0x20b3dfdc = 0;
  *(uint32_t*)0x20b3dfe0 = 0;
  *(uint16_t*)0x20d49ff4 = 0x10;
  *(uint16_t*)0x20d49ff6 = 0;
  *(uint32_t*)0x20d49ff8 = 0;
  *(uint32_t*)0x20d49ffc = 0;
  *(uint32_t*)0x20007000 = 0x20713000;
  *(uint32_t*)0x20007004 = 0x1018;
  *(uint32_t*)0x20713000 = 0x1018;
  *(uint16_t*)0x20713004 = 0x1c;
  *(uint16_t*)0x20713006 = 0x109;
  *(uint32_t*)0x20713008 = 0;
  *(uint32_t*)0x2071300c = 0;
  *(uint8_t*)0x20713010 = 4;
  *(uint8_t*)0x20713011 = 0;
  *(uint16_t*)0x20713012 = 0;
  *(uint16_t*)0x20713014 = 0x1004;
  *(uint16_t*)0x20713016 = 0x11;
  memcpy(
      (void*)0x20713018,
      "\x94\xe4\xb2\x03\x64\xd3\xdd\x37\x50\x6d\x1d\x45\x33\x16\x4e\xd4\x4c\xfd"
      "\x58\xf2\xe0\x20\xc2\x90\x0a\x11\x58\xec\x76\xcc\x62\x5f\xe2\x81\x93\xbb"
      "\x1b\x95\x3e\x8a\xd9\xec\xd3\xa2\x99\x45\x22\xe4\x5d\x5c\xa1\x46\x4e\x3e"
      "\x41\x2e\x3f\x57\x22\xbc\x01\x96\x52\x3b\x35\x57\x7e\xd5\x97\x4b\x82\x78"
      "\x66\xa6\x3a\xce\x2b\x6a\x5b\x6c\xea\x03\xf9\x20\x73\x63\x24\xf3\x85\xf5"
      "\x6b\xf2\x02\xa2\xae\xf6\x37\x48\xdb\x67\xc0\xb4\x36\x37\x38\xa3\xde\xab"
      "\xcf\xb2\xbe\x28\x81\x88\xf1\x29\x16\x7c\xe4\x69\xeb\xd4\x1b\x89\x8b\xf8"
      "\xae\x39\x9a\x84\xd8\xa3\xe3\x30\x46\x42\x5d\x77\x42\x70\x33\x61\xcc\xe9"
      "\x79\x89\x21\xb5\x95\x2e\x7c\xbd\xfe\xf4\x4b\xdf\x3d\x46\xfa\x7a\xd5\xc8"
      "\x27\x1d\xc7\xa2\x6e\x8f\xa1\x6d\x6c\x96\x6b\x56\x50\x3a\x8e\x4b\x6e\xe8"
      "\x09\xed\x5c\x71\x1e\xca\xb7\x95\x99\x5d\x99\xce\xfa\xb3\x40\x92\x46\x71"
      "\xca\xa6\xf6\xdd\x01\x34\x8a\xec\xaf\x74\x88\xba\x49\x50\xb6\x23\x32\xa9"
      "\x10\x7a\x62\x81\xe4\xce\x41\x0e\xff\x8a\x18\x2d\xbd\xad\x24\x06\x86\x8e"
      "\xee\xfd\xc0\x59\x6e\x63\xfb\xe6\x24\xa3\xc7\x73\x33\x8a\xb9\x04\x0d\xbb"
      "\x58\xe0\x5a\x65\x68\x0a\xed\xda\x7a\x69\x60\x5d\xe5\x22\x3a\x8c\x7e\x42"
      "\x30\x08\xd1\x04\x9f\x2d\xc3\x54\x44\x0a\x78\x0b\x5f\xa0\x47\x49\x86\x70"
      "\x25\xd1\x3d\x35\x5f\x46\xcd\xbd\xde\x00\xe1\xd4\xe8\xb5\xff\xfd\xf8\x88"
      "\xa4\xbc\x94\x0b\xa1\x19\x47\xfe\xa8\xab\x01\xf3\x57\x44\xb8\x7f\xd1\x1d"
      "\x3d\xba\x45\x48\x41\x84\x83\x83\x6f\xfd\x56\xbf\xf6\x94\xfb\xd8\x87\x6b"
      "\x14\x66\x1a\xda\x37\xfe\x92\x34\x54\x84\xd9\xdb\x5d\x71\xbe\xb9\x76\x05"
      "\x1b\x00\x4e\xbb\xc6\xcd\x5f\x6c\xaa\xd3\x67\x5c\x6e\xd6\x73\x83\x0b\x50"
      "\xc0\x65\xf0\x6f\x2c\xdc\xab\xf7\x83\xa8\x21\x1b\x1e\x61\x65\xfc\x0e\x51"
      "\x54\xec\x39\x47\x9b\xde\xb5\x46\x0b\x03\xdc\xd7\xfb\x67\x0b\x03\xef\x65"
      "\x7b\x65\x80\xe5\x83\x6f\xd1\xe2\x6d\xa9\x28\xe2\xad\x14\x31\xce\x6e\x3c"
      "\x2b\x72\x65\x8e\x7a\xba\x6a\xee\xec\x95\x82\x51\x5f\x0f\x47\x12\x5f\xc9"
      "\x12\x2a\x46\x19\x91\x9d\xb5\x36\xd0\xb9\x68\xb1\xd8\xe7\x39\x2e\x60\xf5"
      "\xe4\x04\x5c\xc3\xff\x22\x33\x94\x3b\x46\x24\x59\x8c\xa1\x10\xb9\xc5\x0d"
      "\x7f\x7c\xa9\x52\x11\x13\xbf\x8d\xbc\xe0\x79\x7c\xd5\x6c\xc7\xbe\xcd\x40"
      "\x0d\x2e\x52\xd0\xe5\xa8\x70\xd0\x5e\xd1\xfc\x0c\x82\xf4\x38\x92\xca\x02"
      "\x32\x09\xe0\x34\x52\x8a\x74\x3b\x7a\xf1\x5c\xcf\xb0\xa4\xe4\xf6\x54\xfb"
      "\xca\x77\xa6\x9f\xd9\x3a\xff\x24\xd8\xad\xa3\x01\x3d\xc7\x9d\x1d\xaf\x55"
      "\xd3\x14\x6c\xb6\x83\x13\xf6\xa5\x66\x0f\x76\x9b\x5b\xf6\xdd\xb1\x62\x4c"
      "\x62\x35\xc3\xe5\xa8\x21\x7f\xe4\x8b\xb1\x5a\xc5\x00\xbe\xd7\xf6\x58\x90"
      "\x47\xd5\xca\xed\xb9\x4c\x28\xc4\xcf\xf0\x0a\xd6\x1f\xea\xa3\xa9\x98\x5f"
      "\xac\x65\xa2\x65\xd4\x38\x20\x6a\x14\xe0\x09\xa1\x81\x31\xce\x3e\x92\x81"
      "\x49\x80\x16\x74\xed\xc5\x87\x29\x2c\x41\xee\x3d\x8f\x83\xf1\xad\x60\x4c"
      "\x6f\xdf\xf2\x59\xaa\x12\x54\x8a\x38\x4d\x3e\xd1\xe7\x63\x49\x5d\xb8\x87"
      "\xb8\x10\xe7\xa7\x27\x6f\x9e\x1a\xb3\x4c\x91\x8e\x7c\x42\x7e\x35\x35\x36"
      "\x6d\x4b\xd2\x66\x69\x79\x05\xf7\xd2\xf5\xd6\xf2\x9e\x93\xd5\xb0\xb6\xdf"
      "\xc5\xa5\x84\x07\xda\x25\x42\xb0\xf2\xa3\x92\xca\x40\x51\x04\x1e\x38\xc8"
      "\x9b\x51\xf3\x29\xa7\x2a\x83\x7c\x72\xd1\xc0\x5c\xb1\x17\xea\x73\x6b\x12"
      "\xd2\x44\x25\x2a\x46\xba\x3a\x42\x93\x67\xf6\x08\x30\x9d\xe2\xae\x57\xa5"
      "\xe8\x42\xca\x7d\x70\xf4\x32\x4e\x59\xba\x08\x92\xa8\xa0\x99\x9d\x0f\x35"
      "\x4e\x6f\xae\x79\xc4\xbe\x26\x36\xe2\xfe\x1b\xcd\xb9\x05\xf9\xde\x65\x56"
      "\x5c\x81\x98\xcc\x66\x79\x03\x4d\x3e\xb4\xce\xc0\x63\x27\x75\xa6\x82\xe5"
      "\x2b\xdf\xe8\x79\xeb\x2e\x5a\x23\x8b\xb6\x47\x0a\x57\xb3\x7c\xa0\x48\x82"
      "\xec\xf0\x63\xd2\x9f\x84\xe4\x41\xc1\x1a\xab\xed\x9d\x64\x99\x1c\x4f\x4c"
      "\x15\x5e\x33\x87\xd9\x8b\x58\xdc\xab\x38\xe9\x98\x6c\xb3\xa5\x5b\x23\x3f"
      "\x01\x45\x6b\xc1\x3e\x2a\x4b\x9e\x6e\x63\x50\x4e\x33\x3a\xdf\xf0\xee\x43"
      "\xd6\x5c\xb4\xf0\x6f\x1a\x70\x34\xd8\x67\xb3\x9b\x50\x2e\x73\xbe\x67\x2d"
      "\x84\x60\x3e\x28\x80\xf8\x48\xd5\x7a\x87\x26\x25\xca\x92\xfa\x23\x8f\x93"
      "\xc7\xa8\x4b\x17\x8b\xf3\x4d\x01\x57\x0d\x09\x09\x6b\x7b\x8c\x2c\xab\x8b"
      "\xe2\x3c\x0a\x27\x0d\x3d\x66\x1e\x04\x9a\xf3\x3b\xb7\xa6\xf4\x97\x1d\xb5"
      "\x34\xc7\x6b\xc2\xcf\xab\x42\xc7\xeb\xc4\xb0\x49\xfb\xf1\xf3\xed\x4f\x9b"
      "\x7d\x16\x4c\x6f\x79\xec\x64\x49\xfb\xb5\x33\xfd\xf3\x53\x30\x31\xc1\x30"
      "\xfa\xe4\xc0\x1e\xf9\x14\x44\xc5\x8c\xe9\x78\x81\x23\x14\x48\xeb\x5e\x86"
      "\x52\x21\x58\xdd\x48\x01\x01\x42\x21\x45\x45\x82\xcc\x71\x6d\xb1\x83\x66"
      "\x1d\x28\x87\x55\xec\xf2\x5f\x1e\x08\x63\x1d\xc8\xf7\x34\x05\x0d\x2e\x14"
      "\x57\x7a\x23\x3a\x04\x30\xcb\x23\xcd\x6d\x48\x42\xc8\x2e\xf8\x00\x1f\x8b"
      "\x7c\x1b\x8d\x2d\x3b\x00\x7d\x3f\x61\x3c\x7c\x9e\xa9\xb2\xc5\xbf\xea\x75"
      "\xc9\xb5\x7a\x67\x26\x0c\xd2\x78\xd0\x6a\xdd\x50\xc3\xc6\x87\x00\x06\x8d"
      "\xb3\x08\x0f\xa9\xd0\xdc\x4f\x75\xf1\xab\x00\x8f\x5b\xb9\x1b\xf0\x34\x7f"
      "\xc4\xc0\x1c\x74\x2f\x2f\x93\x5b\x7f\x91\x6e\x75\x3a\x1c\xa1\x4d\xe7\xf8"
      "\xe6\x98\x7b\x7b\x36\xc7\x91\xef\xce\x2f\x95\xc3\x5f\x1b\xc5\xd9\xda\xb9"
      "\x2b\xee\x67\xaf\x09\x57\xf2\xad\xe2\x0e\x52\x87\xbc\x87\xaf\xbc\xe5\xf6"
      "\xd2\x5d\xd3\xeb\x6f\xb5\xc3\xdc\xee\x26\x87\xdd\xd4\x89\x5b\x8e\x77\xa2"
      "\x44\x0e\x78\x52\x59\xa6\xdd\x8c\xb1\x24\xf8\xaf\x9e\x50\xde\x35\x68\x45"
      "\x78\xd4\x50\xb2\x2c\x14\xfe\x2d\x28\x09\xc7\x9c\xf9\x9f\xa0\x5a\x0b\xc9"
      "\xcf\x3d\x1f\x49\xa7\x74\x63\x51\xdd\x10\x05\x7e\x45\xfa\xca\x12\x66\xe1"
      "\xb3\xd7\xc5\xaa\x58\x3a\xad\x09\x70\x55\xf6\x5c\x02\xa3\xdd\xe5\xad\xda"
      "\x44\x16\xba\x33\x31\xfe\xf0\xe0\x77\x8e\x23\x44\x71\x31\x91\x0a\xbb\x70"
      "\x5a\xfd\xa1\x05\x6d\xea\x6c\xc4\x43\x7f\x77\x56\x7c\xb3\xcd\x9f\xd8\xd7"
      "\x2b\x88\x5e\xf0\x1e\x5e\x48\x8b\x86\x34\xce\xbf\x43\xe2\x6e\x7b\x3c\xdf"
      "\x33\x71\xc4\x4a\x1d\xb6\x05\x44\x82\xd0\x42\x06\x36\xe4\x93\xbb\x57\x9f"
      "\x2b\xa3\x08\xee\xed\x4d\x89\xed\x17\x68\xb3\xcc\x4f\x4e\xe3\xcf\xdd\x0c"
      "\x79\x25\xb7\x5c\x1d\x70\x6f\x57\xaa\xb5\x26\x27\x10\xc5\x93\x8e\x44\xf7"
      "\xf5\x39\xdc\x0d\xc3\x18\x3c\x78\x7d\xe9\x12\x00\xe0\x4e\xfe\x09\x33\xdf"
      "\xae\x5a\xb5\xa7\xbe\x37\x41\xe9\x35\x8d\x3a\xe5\xe7\xca\xf6\xd2\xe8\x08"
      "\xc7\xeb\x46\x24\x4c\x4a\x3c\x99\xef\x06\x95\x50\x15\xc1\xa2\x47\x94\xb1"
      "\x07\x34\xdc\xb6\xa5\xaf\xba\x61\x0b\x6f\x4d\x2b\x90\x3e\x77\x1f\x5a\x4e"
      "\x40\xca\x38\x94\x77\x7e\xd8\x0c\x19\x5d\xf8\x3e\x72\x9c\x3c\x07\x11\x34"
      "\x19\xd0\x7e\xf0\x46\x62\xeb\xc6\x59\x82\xe8\xd5\x63\xf9\xf4\x09\x12\x1d"
      "\x5f\xb3\xe4\xe7\x37\xa0\x00\x5f\x4f\xde\x1e\x17\x4d\x9e\xf8\xae\x90\x0f"
      "\x74\x7c\xfd\xa7\xa4\xd3\xe4\xf8\xa3\xd6\x10\x44\x3e\x8a\xde\x84\x22\x4b"
      "\x0b\xaf\xb7\x2d\x5f\xeb\xcf\x22\xcc\xa7\x2d\xff\xe9\xbd\x05\x4d\xba\x68"
      "\xd9\xfa\xdc\x9b\x23\xc4\x3d\x9c\x7a\x7d\x0d\x23\x19\x06\x80\x7b\x59\x9f"
      "\x48\x21\x8e\x5e\x97\x95\x40\x24\x90\x61\x5b\x60\x35\xab\x1d\xe7\x75\x9d"
      "\x4a\x1a\x9e\x16\x4d\x0f\x3a\x19\x50\x3c\x78\xd5\xd0\x4c\xd5\x90\x52\xe7"
      "\x68\xa0\x29\x2f\x26\xf8\x47\x10\x09\x35\x76\x44\x00\x3c\xea\xf7\x03\x8c"
      "\xd0\x9b\x65\xb7\x8c\x80\x6f\x10\x61\xd2\xd8\x75\xef\x05\xed\xa7\x2c\x2b"
      "\xd0\xdf\x4e\x9b\x6b\xfd\xdf\xe9\xe2\xdd\x08\x2d\x1c\x44\x4f\x98\x1b\x7f"
      "\x80\xa4\x16\xfe\x9d\xf9\xd6\xba\x4c\x41\x86\xac\xfd\x6a\x7a\x04\x89\x03"
      "\x3a\xff\x03\x7c\xc8\xfe\xe9\x87\x66\x3a\xea\x55\x19\xb6\x00\x74\xf8\x32"
      "\xd3\x66\x48\x28\x28\xc2\xc3\xf6\xfc\x01\x58\xd9\x7c\x5f\x06\x8f\x29\xc9"
      "\x53\x67\x29\xd8\x9f\xb9\x07\x69\x76\x41\x61\x12\xfd\x01\x33\xa0\xe7\x48"
      "\xfc\xa9\xc3\x31\x75\x38\x24\xd2\x37\xf4\xfd\x4d\xdc\xbc\xfe\x8f\x10\x6b"
      "\x37\xa5\x90\x99\x61\xec\x65\x81\x7e\x8d\x14\x41\x7b\xce\x6d\xde\x74\x2a"
      "\xd1\xa3\x0d\x2e\xe3\x7a\xaa\x96\xbb\xc6\x39\x29\x97\x4a\x05\xb5\x4d\x02"
      "\x05\x83\xbd\x57\x8a\x4f\x2e\x60\x5a\x93\xb4\x88\xcd\xe3\xa2\x11\x76\x4d"
      "\x39\xe6\x27\xf9\xd5\x3f\x0b\x33\x75\x2b\xb0\x44\x49\x72\x0d\x62\x10\x5e"
      "\xf3\x98\xf4\x8b\x1e\xd4\x4a\xc1\x15\x1c\xd1\xe2\xc3\x86\xf7\x69\xb8\xad"
      "\x90\x57\xcc\x70\x2b\x56\xfb\x91\x34\xe1\x7f\x9c\x2a\x2a\x9d\x54\xf5\xc5"
      "\x5c\xd9\xee\x28\x2b\xa4\x95\x92\x0a\x04\x2c\xb6\x1d\x2c\xac\x95\xf6\x2d"
      "\xca\xff\xf1\xd8\xf6\x20\x8b\x28\x9c\x25\xf6\xb6\x10\xba\x5a\xbb\x70\x1b"
      "\xf6\xf3\x6d\x48\x29\x89\xfc\xfd\xb1\x01\x1b\x1d\x42\x33\x73\x90\x0b\x5b"
      "\x92\x98\xef\x14\x5e\xc6\xf1\xe7\xb0\xad\x88\xba\xb4\x34\xb6\x7a\x34\x6e"
      "\xf6\x63\x0a\x07\xd9\xe5\xbd\xe1\xdb\x44\x36\x89\x21\xb9\x1d\xc2\xb4\x97"
      "\xba\x8f\x48\xbc\xcf\x45\x82\xff\xe2\x7d\x43\xbd\xd9\x15\xf9\x21\xa4\xed"
      "\xc2\x82\x1f\xea\xce\xfb\xf5\x77\x11\xa6\xc6\xda\x0e\xd2\xff\x33\xbb\x23"
      "\x00\x98\x20\xfd\x1c\x4f\xff\xaf\x7f\x9b\xf7\xbb\x38\x57\x4b\xd9\x0f\xf6"
      "\xb8\x23\xc1\x7a\x92\xd6\x32\x56\x87\x22\xdc\x6a\xd4\xab\xa8\x92\x64\xbd"
      "\xc6\x52\x55\x6d\x33\xca\x7e\x49\xb7\x5c\x54\x66\x95\xdb\x2c\xc1\xf3\xba"
      "\x28\xb1\xc3\xc0\xaa\xa2\x7f\xf8\x3c\x4d\xdd\x50\x77\x40\x5e\x74\x01\xab"
      "\x65\x07\x63\x11\xf2\x0a\x82\x1b\x8e\xa6\x80\xb5\xe1\xf5\x53\x05\x77\x19"
      "\x90\x4f\x5a\x53\x44\x1f\x38\x48\xdc\x47\x3b\x86\x5f\xca\x14\xc8\x62\x54"
      "\xc4\xfb\x25\x4c\x1b\xdc\x61\x82\xcd\x3e\xca\x27\x39\x31\x96\xa7\x6e\x2f"
      "\x0a\xb9\x0d\xe0\x36\xeb\x87\x79\xf5\xf3\x3f\xc5\xf1\xf4\x18\x28\xd2\x42"
      "\x28\xd9\xbd\xff\xaf\xff\x6d\x69\x2e\x73\x17\x2d\x4f\x37\x66\xd5\x59\x0c"
      "\x81\xc6\x50\x97\xa6\x1e\xd7\xdf\xde\x91\x75\x57\x8e\xd0\xb0\xec\x55\x7d"
      "\x80\xad\xb8\x70\x70\x7a\x6b\x43\x8f\x7d\x79\x0b\xa0\x63\x3f\xd1\x92\x0a"
      "\x25\x4c\x80\x22\xfa\x4a\x74\x94\x72\x0f\xcf\x26\x4a\xd3\xd4\x52\x4b\x03"
      "\x8a\x6e\x87\x46\x9f\x83\xdc\xa9\x31\x93\x71\x2e\x2c\x9f\x29\x4a\xe8\x43"
      "\xbb\xfc\xa8\xc4\xc4\xbe\x00\xd0\x82\x6d\xf6\xe5\x09\x13\x58\x3f\xcd\x85"
      "\xa2\x14\xfe\x24\x54\xf7\x60\xbd\x00\xe3\x1f\x3e\xd1\x96\xb9\x5c\x6d\x6a"
      "\x30\x9a\x0d\xe2\x76\x29\x2b\x38\x94\x4f\x48\x9a\x1b\x78\x5f\x4f\x55\xe9"
      "\xd2\xe5\xb1\x05\x65\x47\x26\x8f\x40\xad\x5b\x11\x26\x16\x6a\xd7\x28\x71"
      "\x91\x4e\x45\x2c\x8d\x3f\xc3\x4c\x80\x7e\xbe\xaf\x8e\x27\xcc\xdd\x04\x54"
      "\x84\xb7\xb1\x25\xb2\x1c\x9a\x26\x02\x85\x88\x65\x16\x31\xb7\xa0\x18\xfd"
      "\x2e\x09\x15\xdb\x49\xc5\x24\xef\x0c\xb5\xbb\x76\xee\x7d\x5e\xa5\x27\xc8"
      "\x3e\x9c\xec\xc8\x43\x57\xa1\x21\xdc\x37\xd7\xed\xb3\x73\x2b\xf0\x63\x76"
      "\xb4\xc8\xd5\x42\x88\x39\x8e\x96\xab\xce\x60\x42\x68\x9c\xca\x46\x5f\x79"
      "\x22\x19\xc6\xc6\x7f\x46\x0a\xcd\x35\x8d\x2e\x42\xf6\x45\x48\xae\x4b\xa3"
      "\x78\x00\xa3\x83\x6d\x71\x88\x7d\xe2\x0f\x7b\xa4\xc0\x51\x13\x77\x31\x3c"
      "\x15\x65\x3f\xf6\xf1\x01\xdb\xf0\x90\xe5\x42\xee\xf0\x69\xef\x83\x5b\xcf"
      "\xc1\x9f\x3f\x95\xc9\x89\x53\x33\x00\x63\xad\x48\x2a\x0a\x63\x6d\xd4\x6a"
      "\x28\x8d\x9c\x5a\x2d\xff\xbf\x24\x75\x12\xe8\x67\xd4\x2b\x00\x39\xfc\xff"
      "\xf6\xe0\x96\x94\x73\xb4\xfd\xbd\xaf\x89\x3f\x27\xc0\x3b\x2d\x1d\x77\x20"
      "\x6a\x74\x6c\x57\x21\x9f\x7b\xd5\x78\x5b\x8c\xf2\x89\xed\xd9\x77\x5b\x82"
      "\xc8\xc9\x83\x54\x15\x19\x41\x97\x2b\xd4\x4e\x0b\x3e\xb1\xd7\xc1\xaa\x18"
      "\x23\x5f\x52\xe1\x48\x96\x9d\x2a\x5d\xe1\xb3\xc8\xbd\x3e\xfd\x26\x7f\x38"
      "\x34\x12\xca\xae\x2a\xe7\x8e\x87\x00\x43\x52\xf5\x45\x16\xa0\x6e\xd2\x39"
      "\x1d\xaa\x70\x74\xcb\x88\xea\x66\x54\xbc\x6d\x25\x6c\x80\x31\xae\x12\x9b"
      "\xd9\xfa\xcb\x96\x5d\x77\xff\xc1\xef\x99\x59\xbd\xa9\x88\xfe\x36\x2f\xd4"
      "\xf0\x18\xad\xad\xa4\x0d\x96\x82\x1c\x70\xc9\x1b\xac\x74\x99\xaf\xf8\xf1"
      "\x7b\x5a\x1f\xc6\xbe\xc8\x32\x17\x03\x49\xb3\x41\xf4\x9e\x40\x08\xe0\x5e"
      "\xba\xe1\xbf\x6b\x24\x29\xa9\x1d\x15\xba\x58\x9e\x89\x89\xf3\x2c\x5c\x74"
      "\xe9\xde\x72\x5e\x66\xa3\x17\xf7\x87\x68\x42\x36\x39\x93\x4a\x59\x57\x3b"
      "\xb9\xcd\xbe\x06\xa6\x23\x38\x4f\x67\xdb\x83\x5a\x29\x0b\xf3\x1a\xa4\x58"
      "\x97\x6a\x84\xf1\xac\x7b\x7d\x51\xcb\xf0\xb8\xf3\x39\x06\xe9\x15\xb5\x0a"
      "\x5f\xbc\x51\x8e\xde\x0a\xbd\x03\xbb\xea\xdb\x6e\x92\xb7\x02\x37\xeb\x99"
      "\x58\x5b\xe3\x1d\x81\xdb\x20\x1e\xf3\xd7\x4d\x47\x54\x10\x40\x91\x2b\xaf"
      "\x38\x69\x69\x25\x53\xad\xc1\xc3\xeb\x63\x93\x09\x95\xc1\xed\x10\xd3\xd4"
      "\x3b\x4f\x00\x48\x05\xe2\xae\xaf\x6f\x2c\xc1\xce\xd8\x0c\x9f\x41\x5d\x4a"
      "\xf1\xc6\xc6\x58\x47\xfd\x5f\xb6\x7d\x80\x69\xbd\xde\x73\x58\xbe\xd4\xd1"
      "\x0b\x1f\xac\x95\x6e\x22\x08\x92\x6a\x71\x7d\xa8\xe4\x3e\x37\x45\x84\x9d"
      "\xa9\x5c\xe2\x65\xa5\x3b\xdc\x56\x86\xb8\xd1\xe9\x87\x0d\xf2\xed\xda\x65"
      "\x2c\xd3\x09\xa0\x18\x66\x5a\xd5\x9c\x42\xa6\x22\x41\x10\x39\x6b\xa4\x85"
      "\xa6\x73\x59\xd9\xdd\xea\xd5\x2f\x0e\xb9\x1e\x5d\x04\x8a\x58\xb6\x6b\x74"
      "\x60\x5a\x0c\x84\x68\xf6\x48\xbf\x86\xf6\xd1\x71\x59\x9c\x01\x5e\x7a\xff"
      "\xf7\x1e\x81\xbc\x07\x7a\xd8\xe4\xde\x9d\xef\x2b\xba\x44\x90\x3a\xcb\xcd"
      "\x47\xa3\xaa\x3a\x60\x7d\x1a\xb3\x50\x76\xfb\x19\x0e\xdd\x56\x3a\xaf\xa6"
      "\x49\xf6\x9a\xeb\x64\xfa\xc4\x7d\xd5\xce\x98\xa0\x66\x70\x3f\x4e\x6f\x93"
      "\x1f\x7b\x2b\xc5\x83\x79\x7c\x95\xa1\x97\xf4\x8e\xa4\xc1\xf6\x15\xd3\x37"
      "\x72\x45\x79\x74\xa1\x5c\xde\x5f\xef\xa8\x41\x2f\x67\x6c\xbb\x0e\x49\x71"
      "\x4f\x44\xa1\x0b\x8b\xb2\xd1\x47\xa6\x88\x49\x1a\x63\xfe\x87\x58\x7e\x81"
      "\x7e\xf5\x66\x8d\xe9\x12\x60\x77\x21\x68\xcd\x3e\x7a\xc5\x47\xf7\x94\xae"
      "\x02\xb3\x2d\x1b\x79\xe5\xca\x13\xfe\xd5\x3a\x1d\x9f\xa8\x6e\xd5\xb8\xdf"
      "\x11\xae\xc4\xcc\x34\x42\x37\xf5\x4b\x9d\x33\xca\xfb\x82\x6e\x18\xa2\x68"
      "\xcb\xb1\x7e\xf2\x2a\x73\xa1\x71\x07\xc2\x23\x51\xeb\x91\x4a\x7b\xce\x85"
      "\xbe\x93\xf6\x81\x32\x95\xfc\x42\x4d\x08\xcc\x66\x71\x74\x88\x7a\xd2\xab"
      "\xfa\x82\x5b\x59\xe6\x22\x8d\x66\x77\x0a\x74\xfe\xd8\x3c\x5d\xf4\x64\x14"
      "\x8c\x55\x3a\xa5\x2e\x95\x92\xa1\x53\x33\x4c\xf7\xc5\x6f\xec\xc5\x64\x4f"
      "\x85\xe5\x1d\xd1\x47\xa7\xe5\x1c\x9c\xac\xf1\xb8\xd1\x41\xbc\x57\x6c\xa0"
      "\x4d\x08\x9f\xbd\x3b\xc0\x4d\x26\x34\x46\xe5\x06\x2f\x05\x84\x24\x4a\x2b"
      "\x45\x09\x86\xf7\x62\xd3\xb1\x4c\x90\x78\x53\xb1\xd6\x0c\xd6\x2b\xb8\x5b"
      "\xc2\xb5\x34\x9c\x60\x18\x60\xcd\xfe\x4c\x62\x62\x12\xd4\x14\xda\x73\xb4"
      "\xb2\xaf\x68\x39\x8f\xf4\xa9\x0e\x83\xc3\xb4\x04\xa2\x89\xb8\x14\x1d\xe4"
      "\x5c\x06\x16\x3c\xeb\xf4\xe1\x30\xee\x16\x45\x99\x88\x6f\xe8\xc3\x88\xb5"
      "\xde\x06\x4e\x75\x8f\x2c\xb6\x95\xfa\x1f\x68\xcf\x07\xfc\x11\x26\xd8\xe0"
      "\x37\xf3\x8f\x64\xfe\xf9\xaa\x14\x3b\xc5\x74\x02\xfd\x33\xf9\xd6\x5d\xee"
      "\xfc\x95\x79\x05\x5d\xa8\xc4\x22\xa5\xaa\xd1\x78\xa3\x86\x0c\x39\xcc\xee"
      "\x1b\xcc\xfd\x6a\x8f\xb0\x75\xa2\xf1\xbe\x2e\xa3\x2d\xd2\x87\x23\x99\x1b"
      "\x51\x35\x31\xdd\x9f\x7c\xbf\x6e\x48\x20\x0b\x01\xa8\x93\x87\x4c\x11\x4c"
      "\x22\x60\x6b\x86\x9b\x59\x61\xbc\x2a\xa3\xa9\xf2\x5b\x7c\xad\xce\xae\xcb"
      "\x57\xee\xa4\xb1\x6b\x43\xc8\x87\xa6\x3d\xd3\x79\xb8\x05\xd0\xdb\x99\xdb"
      "\x28\x94\x6e\xe6\xe5\xee\x63\x7c\xe3\x7b\x77\x4a\xde\xb0\x4e\xd8\x85\x63"
      "\xd6\x12\xc4\xfc\x81\xfb\x73\x07\xa2\x32\xe3\xb1\x7d\x3e\xbe\x8b\xc7\x33"
      "\xc2\xc3\x06\x02\x66\x86\x8e\x26\x1a\x3d\xdb\x70\x7e\xfc\xd8\x7d\xd6\x2b"
      "\x79\x75\x33\xb7\x7a\x1d\x65\x20\xfe\x85\x6c\xb9\xe3\x8f\x62\x11\x6b\x2a"
      "\x5f\x27\x82\xcc\xc1\x58\xe1\xea\xe0\xa3\x3a\x6b\xde\x7f\x84\xdf\xad\xee"
      "\x92\x12\x31\x5e\xf8\xda\xbf\xa1\x28\x99\x39\xa2\xd1\x08\xd6\x6b\xae\xf1"
      "\xc5\x48\xe3\xc0\x68\x86\x96\x94\x7f\x02\xc5\xee\x37\x14\x5e\xaf\x0e\x21"
      "\x0a\x66\xa4\xe2\xfc\xe7\xad\x45\x90\xe2\x8e\x68\x15\x3c\xb8\x0a\x2c\xf0"
      "\x3d\x60\x9a\xbc\xa3\x3d\xa0\x4a\x71\x46\xd0\xc5\xec\xb0\x73\xb1\x36\x72"
      "\xa4\xf3\x94\x88\x6a\x8a\x7e\x0d\xd6\x00\x4f\xeb\x6b\xbe\xf8\x98\x1f\x5e"
      "\xd0\x21\x6b\xd1\x63\x11\xa5\xc4\x19\x04\x71\x15\x3e\xf0\xf4\xce\x4b\x90"
      "\xb2\x49\x84\xbd\x63\xf2\xf8\x33\x72\x6c\xb7\xc4\xf5\xa0\xe8\xa6\x46\x28"
      "\x7f\x78\x7b\x40\xdd\x72\x0c\x06\xe9\x7a\x7c\x92\x91\x5a\x45\x49\x9e\x12"
      "\xfe\xe3\x93\xc9\xe8\xb5\xa1\xfe\x58\xe9\x15\xa9\x25\x27\xe7\xc2\x0a\x97"
      "\xe1\x0e\x42\x78\x57\x71\xf0\xe6\x09\x10\x7e\x3a\xeb\xd6\x2c\xe5\x20\x49"
      "\xc0\x64\xef\x26\x69\xa9\x03\x84\x96\xd1\x52\x9b\x5a\x89\xf9\xa2\x7c\x82"
      "\x1a\xbc\x58\x22\xb0\xf7\xc6\x7d\x7e\xe1\x1f\x45\x9a\xae\x62\xfc\x30\xe8"
      "\xff\x68\xa4\xc9\x85\x2a\x54\x51\x17\xd8\xa1\xe9\xc3\x99\x04\x75\x7e\x0a"
      "\x12\xd2\x6d\xc7\x7e\x00\x1e\xec\x91\xed\x9a\xd1\x30\x66\x3b\x3f\xe8\x49"
      "\x33\xf8\x90\xd7\x32\xf6\xcb\x9a\x94\x3e\xde\x45\xb7\xc7\xc0\x18\x24\xb3"
      "\x75\x3f\x90\xe6\x46\x7d\x56\xbe\x49\xab\x66\x8c\xe9\xbe\x88\xee\xd9\x1d"
      "\x12\x94\x0e\x61\xf5\xd4\x79\x39\xc7\x53\xeb\xec\xa2\x12\xf7\xd7\xd1\x9a"
      "\x70\x5b\x47\x3e\xac\x8a\xe8\x2c\xed\x32\x69\xdb\x0e\xd4\x71\x9b\x34\x1b"
      "\xdd\xdd\x3c\xce\xd9\xed\x6e\x7a\x4a\x77\x75\x2a\xa1\x92\x7b\x20\x4d\xd3"
      "\xca\x97\x3f\xa2\x3f\xf6\xeb\x7c\x5f\x92\xe8\x4a\xb4\x02\x9a\xd3\xf7\x88"
      "\x8c\xc9\x6c\x54\xe4\x20\xd5\x06\x9a\x5e\xd8\x37\xe8\xce\x1b\x54\x2c\xe0"
      "\xc3\x2d\xc1\xa1\x5f\xca\x57\xec\x61\xe1\xf6\xfb\x21\x96\xcb\x44\x16\x78"
      "\xd3\xfc\xbe\x7a\x97\xe3\x10\xe4\x6f\x1d\x2d\xc2\x5b\xb4\x85\x21\xbe\x44"
      "\x43\x48\x27\x3a\xa8\x07\xcb\xe5\xe9\x9a\xd5\xb5\xd7\x11\x3e\x9e\x2c\x45"
      "\x6b\xcf\xba\x87\xb9\x13\xf1\xfa\xf7\xfd\x1f\xd5\xff\x9a\x06\xe8\x40\xbb"
      "\x96\x96\xeb\xb1\x22\x1d\xca\x97\x7b\x8b\xa8\x30\xed\xa3\xd0\x8c\x66\x98"
      "\x1f\x3c\xf0\x63\xdd\x63\x23\x59\x8a\x4d\xc9\x90\x2d\x42\xa5\x06\x35\xfe"
      "\x56\x91\xa5\x29\x0c\x4e\xbc\xe1\xa6\x02\x20\xc5\xa4\xf8\xd7\x67\x4b\x6f"
      "\xb9\x46\x91\xb8\x0f\x41\x40\x36\x05\xee\xc8\x2b\xc9\xc3\x65\x9e\x4c\x20"
      "\x2d\x4b\xc5\x16\x74\x07\x22\x10\xa6\x19\x73\x1a\xa1\xb5\x58\x7b\xc3\x42"
      "\x14\x97\xba\x17\x72\x74\x97\xc4\xbc\x15\x49\x11\xde\x29\x50\xef\xc0\xee"
      "\x75\x67\x43\x2d\xd1\xea\xd2\x74\xd1\x51\xb6\x9e\x7e\x64\x61\xd6\xca\x72"
      "\xf4\x37\xba\x6f\xe1\xb3\xf8\x90\x19\x4b\x3d\x59\x5c\xc9\x03\x90\xa1\x33"
      "\xf3\x15\x05\xbe\x81\x77\x39\xb7\x6c\xf3\x59\xf7\x66\x07\x3d\xd3\xf1\x74"
      "\xfb\x0c\x75\xf3\x31\x79\x45\x11\xfe\xc1\xe7\x0e\x84\xad\x92\x98\xc2\x02"
      "\x5a\x8e\x1c\x0c\xcb\x31\x43\x1c\x34\x34\xb6\xa8\x7f\xe2\x9d\x8d\xb0\x18"
      "\x3c\xda\x25\x54\x10\x4f\x09\x75\x06\x10\xf6\x8e\xd2\x34\x5b\xbb\x53\x6f"
      "\x82\xb7\xc2\x45\x43\x8e\x40\x83\x93\x84",
      4096);
  syscall(__NR_sendmsg, r[0], 0x20b3dfc8, 0);
}

int main()
{
  loop();
  return 0;
}
