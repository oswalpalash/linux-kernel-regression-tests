// https://syzkaller.appspot.com/bug?id=bb69f52bec2663e65a89d5851cd05b88906a31a7
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <sys/syscall.h>
#include <unistd.h>

#include <stdint.h>
#include <string.h>

#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#ifndef __NR_bind
#define __NR_bind 361
#endif
#ifndef __NR_setsockopt
#define __NR_setsockopt 366
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[10];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  r[1] = syscall(__NR_socket, 0x26ul, 0x5ul, 0x0ul);
  *(uint16_t*)0x20944000 = (uint16_t)0x26;
  memcpy((void*)0x20944002,
         "\x68\x61\x73\x68\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         14);
  *(uint32_t*)0x20944010 = (uint32_t)0x0;
  *(uint32_t*)0x20944014 = (uint32_t)0x20f;
  memcpy((void*)0x20944018,
         "\x68\x6d\x61\x63\x28\x68\x6d\x61\x63\x28\x73\x68\x61\x33\x2d"
         "\x35\x31\x32\x2d\x67\x65\x6e\x65\x72\x69\x63\x29\x29\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00",
         64);
  r[7] = syscall(__NR_bind, r[1], 0x20944000ul, 0x58ul);
  memcpy(
      (void*)0x20ea5000,
      "\x61\xb7\x52\xcf\xb6\xb2\xf8\x68\xf1\xd5\xd6\xf4\x3a\x15\x0e\x73"
      "\xa5\xd5\x8d\xa9\x62\x0b\x60\x72\x98\x40\x43\x8e\x3b\x6b\x56\x00"
      "\xb3\x7a\x9f\x59\xd3\x9d\x45\x28\xb0\x3c\x26\x4a\x12\xe9\x80\x29"
      "\x63\xca\x8b\x1a\x68\x62\x8d\xb8\x53\xce\x97\xec\x2e\x3c\x92\x67"
      "\x8b\xdd\xeb\x5d\x6d\xd9\x76\x4e\x73\xd6\x94\x23\x6f\x5f\xa7\x85"
      "\xb0\x12\x6c\x9b\x9d\x78\xac\x69\x80\x77\x3f\xce\x85\xa1\x53\x45"
      "\xc0\x03\x67\xab\xdd\x1c\xaf\x1c\x7b\xa5\x01\x58\xaf\xf1\x08\xde"
      "\x71\x32\x11\xd8\x5c\x11\xc6\xe5\x78\x57\x07\x82\xb9\x5d\x0b\x28"
      "\x38\x91\xd3\x5c\x40\xc7\x3f\xf0\xce\xf0\xc7\x30\x7f\x12\x90\x0d"
      "\xdd\x69\xd6\xcd\xa7\x13\xa1\x0c\x0c\xe5\xdf\x32\x87\x2f\xed\x7e"
      "\x7e\x08\x86\x15\xab\xc7\x11\xdb\x46\xc0\x4c\xd5\xf6\x30\xdf\xaa"
      "\xc0\x76\x38\xb2\x8a\x34\x11\x3f\xa3\xc1\x1d\x20\x2f\xff\x8a\x94"
      "\x7b\xc9\x58\x4a\x2c\x38\x43\x93\xe4\x7b\x00\xed\x3e\x28\x5c\xba"
      "\xf6\x23\xdb\xe6\xde\x4c\x19\x7e\x67\x27\x94\x87\x94\xb2\x57\x59"
      "\xb4\x1c\x3e\xeb\x19\xcb\xe1\x03\x6d\x54\x1b\xac\x43\x6a\x46\x83"
      "\x1f\xfd\x55\xbc\x54\x80\x0e\x94\xb5\x26\x7e\xfa\x7f\x9e\xda\xf5"
      "\xbd\xdf\x89\x5f\x26\x4a\x5d\x8a\x8c\x58\xa0\x00\xf6\x43\x10\x67"
      "\x29\x44\x5f\x53\x98\x1e\xf7\xf8\x48\xe4\x8d\x22\xea\x1e\x60\x8a"
      "\x3e\xa0\x0e\x6c\x1a\x7f\x9b\xd6\x17\x36\xe2\x1d\x1b\xd9\xab\x64"
      "\xaf\xea\x60\xbc\xbb\x6a\x42\xe9\xe6\x01\x6d\xcd\x39\x3b\x61\xb0"
      "\x12\xbf\xe2\x8c\x21\xe4\xf6\x79\x27\x2f\xc8\x90\x28\x07\x09\x7b"
      "\x86\xe8\x82\x74\xc9\x77\xcd\x83\x8a\xc7\xd5\xae\xaf\x59\xa3\x55"
      "\x18\xf2\x5a\x0a\xfe\x7a\x9f\xee\x2a\xa8\x0d\x55\xa6\xc7\x6a\x44"
      "\xb3\xfe\x49\x6e\xf6\x6e\x83\x71\x21\xe1\xec\xcf\xa4\x44\xa2\x4b"
      "\xc5\x85\x01\x69\xbc\x9b\x1a\x96\xdc\x00\x8a\xb9\x8d\x72\x5d\x51"
      "\x4e\x20\x82\x24\x01\x5c\xd3\xb9\x18\x49\x46\xc6\x6d\x81\x2e\x4f"
      "\x5c\x62\x9f\x78\xd0\xf9\x88\x5d\x95\xfe\xd4\x5d\x42\xd8\xfa\xd9"
      "\xf8\xe2\x09\x7a\xe1\x5b\x08\xe2\x6b\x8a\xbe\x2a\x6b\x04\xd3\xcc"
      "\xd2\xb7\x83\x8d\x29\x84\x18\x21\x14\x26\xdf\x3b\xd7\xa7\x09\xe0"
      "\x11\x1f\x6b\x9c\x98\xd5\x1c\x17\xd0\x30\xdc\x45\x26\xc7\x8e\xe1"
      "\xdf\x52\x39\x2e\x7d\xa5\x3a\xcf\xa7\x2b\x97\xc7\x04\x3e\x8b\x70"
      "\x61\x43\xde\xd3\x1a\x9d\xbd\xb6\x80\x8a\xba\xd3\x8d\x5d\xe5\xc2"
      "\x01\x62\x85\xc6\xe9\x7e\xc1\xe1\x53\x1e\xc7\x99\xed\x0f\x6c\x47"
      "\xaf\x27\x1f\x6c\x5b\x66\x92\x70\xfe\x83\x0a\xbe\xca\x27\xaa\xec"
      "\x9d\x7b\xe3\xac\x97\xdd\x9f\x5a\x16\x5a\x63\xf2\x05\x46\xd9\x99"
      "\x30\x6a\xb2\xc6\x96\x4f\x80\x37\xa5\x22\x35\x57\x69\xcc\x12\x40"
      "\x6d\xdc\x72\x2b\x79\xdb\xd0\x46\x9d\x67\x64\xb2\x9d\x83\x35\xa2"
      "\x57\xce\x74\xf0\x74\x37\x9c\x21\x73\x18\x86\x28\xc9\xce\xef\x6a"
      "\x32\xd9\xb7\x38\x26\xc2\xdf\xbf\x59\x61\xbb\x0f\xc4\xbe\xcc\x48"
      "\x2b\xa5\x9b\xbd\x15\x5f\xed\x38\x2e\x6d\x37\x49\xc0\xfd\xc8\x1d"
      "\xbe\x53\x57\x1c\x0c\x3c\x96\x34\xad\x48\x2c\xff\xb8\x24\x6d\x06"
      "\x16\x51\xa0\x69\xcc\x4a\x54\x6e\x65\x74\xed\xb9\xf5\x1f\xd9\xe9"
      "\x2f\x39\x7e\xef\xfe\x78\xaa\x46\x8f\xad\x6c\xb5\x26\x15\x06\xdf"
      "\x62\xc8\xfd\x3d\xf1\x7d\x62\x08\xd5\xd4\xcd\x59\xfe\x96\x1d\xba"
      "\xdd\x60\x8d\xe6\x97\x90\x78\x84\x88\x6d\xe2\x6e\x1b\x74\xe2\x44"
      "\xff\xe4\x8f\xdf\x9e\xef\x6a\x91\x78\x1c\xa9\x99\x99\xe3\xd0\x42"
      "\x72\xe5\x7c\xab\x64\xd8\xc5\xeb\x07\x98\x61\x5c\x58\xb7\xd5\x6c"
      "\x4a\xb6\x1f\xe0\x41\x1e\xb1\x47\xb4\x87\x5e\x1e\xf7\xf1\xbd\x0a"
      "\x79\xe8\x59\x8a\x9a\x2c\x78\x03\x76\xcd\x5f\xb0\x04\x6d\x07\xa6"
      "\x39\x3a\x14\x0f\xfb\xfa\xbe\x64\xa2\x70\x3e\xf6\x99\x78\xa3\xe8"
      "\x11\x07\x71\x4c\xa5\x9f\xf9\x76\xeb\xff\x71\x49\xe7\x26\xe1\xa1"
      "\x10\x78\x62\xc2\xf9\xb0\x5b\x06\x82\xaa\xe1\xf7\xa4\x5d\x9f\x8d"
      "\xb6\x6e\x56\xcc\x91\x77\x52\xee\x8b\xc0\xcf\x79\xc2\xb0\x50\x32"
      "\x3d\xc2\x98\xc6\xab\x88\x6f\x86\x6d\x1b\x62\xe3\x50\x77\x78\xf3"
      "\x82\xe0\x2d\x9b\x3a\xd1\xc2\x28\x7a\x14\x18\x7c\x6b\xd9\x35\x19"
      "\xaf\x92\x2d\x4f\x56\x1d\xba\x27\xa4\xe6\x83\xa5\x2e\x24\x49\x0a"
      "\xe9\x3a\x12\xb6\x3d\xb1\x35\x04\x6e\x10\x8b\x52\x22\x6c\x62\x77"
      "\x07\x1f\x0f\x12\x87\x4e\x26\x7c\x35\x1a\x15\x65\x3b\x1a\xd5\x80"
      "\xc5\xf8\xde\xc2\x4e\xc2\xc3\x7b\x9d\x2d\x05\x15\xc0\xff\x9b\x60"
      "\xd1\xb1\xae\x47\x67\xcb\xfc\x35\x0d\x64\x5c\x1c\x0e\xc9\x6a\x1e"
      "\xd2\xbc\xd2\xfc\xb7\x91\x6b\x8d\xb0\xe2\x29\x73\x21\x68\xda\xd1"
      "\xb8\x26\xce\x84\x55\x1a\x01\x9a\x65\x75\x3c\x87\xb2\xbe\xc0\xa7"
      "\xca\x00\x4e\x4c\x7b\xb9\x86\x46\x91\x03\xd5\x36\x0c\xf6\x35\x1e"
      "\x63\xf4\x09\x7e\x08\x37\x91\xdd\x34\x4d\x5e\xce\xdd\x87\x06\x32"
      "\xce\xea\x2e\xf0\xda\xc9\x7c\x47\x46\x89\xe4\xa0\xf7\xf1\xcb\xb5"
      "\xec\x82\x62\x3a\xe5\xa3\x0e\xe3\x8e\x35\xf7\xcb\xb4\xd7\x39\xda"
      "\x5e\xbc\xc7\x3e\x94\x14\x1c\xd2\x8b\x82\xab\x15\x9e\xa2\x59\xb4"
      "\x07\x7c\xa4\x0a\x21\xc2\xb6\xc9\xa7\xc7\x44\x97\x10\x76\x08\xc1"
      "\xf3\x49\xa6\xe0\x68\x33\x80\x03\x32\x19\x71\xe5\x36\x30\xce\x01"
      "\xd1\x95\x42\x71\x69\x4f\x2d\xd3\x78\x45\x94\x5c\xd6\xa7\x72\x51"
      "\x95\xc2\x89\x82\x45\x6c\x69\xc2\xa4\x35\xb4\x50\xf2\xbd\xbf\x28"
      "\x1a\xa1\x4d\x89\x01\xcc\x42\xc1\xac\x85\xd2\xe2\xa3\x16\xc6\xd2"
      "\x95\x92\x3e\xc4\x2d\xf5\x77\xd0\x82\x07\xd8\x89\xeb\x07\xbe\x79"
      "\x75\x9b\x00\xa8\x06\xbd\xe8\xda\x27\xa3\x67\xf5\x1e\x35\xac\xcf"
      "\x54\x7a\x4a\x89\xed\x57\x95\xc3\xe4\x99\xcc\xb3\xe9\x71\xbc\x2c"
      "\x4e\x39\xe8\x52\xfa\xea\xe4\x37\x93\x23\x56\xce\xe3\xb0\x6a\xcd"
      "\x39\x88\xe9\xd4\x0a\x46\x8b\x55\x96\xc9\x74\x20\xda\xda\x11\x95"
      "\xc4\x6d\x7a\xe8\x5c\xa4\xc9\x99\xfa\xe7\x2b\xea\x70\xe9\x85\x99"
      "\x24\x0f\x43\xdb\xc1\xeb\x4e\xaf\xe2\x92\x74\xf5\xab\x20\xc4\xd6"
      "\xf9\x6b\x6a\x1a\x96\xcf\x27\xeb\xbb\x7e\xbc\xcb\x55\xe5\xdf\x64"
      "\x58\xe1\xe4\x07\xf7\xb2\x7e\x20\x8a\xa1\x03\x84\xe3\xe4\x67\x68"
      "\xfa\xcc\x27\x20\x6d\x3c\xd2\x52\x1b\x13\xe2\xe3\x36\x15\xab\xa8"
      "\x15\x7a\x69\xf9\xcb\x5d\x27\x97\x51\x71\xa4\x13\x3f\x97\x1c\x38"
      "\xbe\xed\x9c\x65\x2e\xff\xa6\x95\xee\x34\x1c\x44\xf9\x6e\xe4\xcb"
      "\x99\x84\x27\x09\xe2\xe6\xad\xbd\x61\x09\x23\x9d\x4e\xe6\xe2\x53"
      "\x77\x12\x2a\xe8\xff\x19\x7b\x35\x8b\x90\x37\x68\xd7\xf3\x3a\x47"
      "\xe9\x2a\x1f\xdc\x9d\xe1\x53\xa4\xa0\xd8\xf6\x98\xca\xde\x4d\xac"
      "\x8c\xaa\xd2\x85\x7d\x03\xd1\x0c\x33\xe3\x2b\x4e\xc8\x96\xa5\xd0"
      "\x2f\x8b\xed\xf3\x28\xfb\x43\x2d\xc0\x1d\xce\xb9\x09\x85\xb7\xd6"
      "\x4a\xf7\x55\x38\x75\x89\xfc\xfa\x7a\x64\xfe\x7d\xe6\x8a\x95\x74"
      "\xe8\xcc\x67\x92\x1f\x58\xab\x74\x9a\xf6\x1d\x2a\xa5\x6e\x67\xea"
      "\x91\xed\x41\xf6\x11\x5d\xde\x03\xde\x47\x7d\x1d\x2b\x62\x64\x3e"
      "\x0d\x7c\xd4\x23\x24\xbc\x12\x5e\x4c\xab\x9f\xc2\xa7\x18\xca\x19"
      "\x60\x66\xd8\x40\xf1\xc4\x6e\x2f\x3b\x1a\x6e\xa3\x45\x23\xe5\xfa"
      "\xcf\xd6\x24\xb7\x99\x91\xa9\x37\xaa\x86\x3a\x4e\x57\xd2\x61\x8d"
      "\xb8\x34\x22\xca\x29\xe7\x68\x84\xc9\xf8\xf8\xb0\x0f\x81\xa1\x8a"
      "\xcf\xd7\x2d\xb8\xdd\x5a\x4a\xc4\x19\xc0\x94\x7d\xca\xf1\x45\xb1"
      "\x15\x07\x29\x4b\xd3\xf3\xe8\x8c\x85\xe6\xbd\x12\x4d\x4e\x02\x39"
      "\x25\xcd\xc9\xdf\xb1\xd8\xca\x83\x35\x45\x86\x04\x4c\x11\xe6\x63"
      "\xce\xe4\xff\xbe\x14\xcc\xf7\x8c\x0c\x8d\xf8\x57\xd2\xff\x3e\xe7"
      "\x39\x0c\x3a\x17\x1e\xb4\x45\x37\x98\xc4\x6a\x28\x3b\x2e\x8d\x47"
      "\xc0\xa3\x03\x2f\x41\x73\xc3\xe1\x0b\xc0\xd8\x00\x25\x55\x18\xb0"
      "\x3d\xa6\x03\x04\xb2\x7b\x60\xc1\xbe\x70\x31\x2e\x59\xa2\xa9\x27"
      "\xca\x4c\x64\x05\x80\xac\xc5\xe1\xd9\x98\xa3\xab\x70\x04\xb8\xac"
      "\x68\x53\xe8\x67\x99\x99\xd6\x5c\x8f\xdf\xf9\xc9\xf0\x21\x4a\xad"
      "\x4c\xd4\x96\x0f\xb5\xde\xc5\xeb\xf4\x05\xb4\xdf\x2f\x11\x47\xaa"
      "\x20\xf1\xd8\xbd\x6b\xf5\x8d\x2e\x13\xec\x49\x34\xae\xb5\x8b\x66"
      "\x09\x2a\x3e\xd1\xe1\x4d\x2c\x24\xb9\x2f\x53\xf3\xd1\x18\xc0\xfc"
      "\xad\x1a\x6f\x86\x62\x7b\x1f\x82\x38\x93\x97\x6f\xda\xea\x8a\x7e"
      "\xe0\x5b\x05\xd6\xf0\x66\x7e\xbf\xe4\x1f\x8e\x43\xd8\x6d\xeb\x5f"
      "\xac\x9f\x2a\x06\x35\x69\x95\xf0\x36\x14\xa3\x5a\xab\x03\x9d\x5b"
      "\x96\xcf\x7e\x2a\x75\x82\x6c\xd0\xdb\x12\x7b\x58\x83\x04\xa2\xb5"
      "\x08\x03\xbd\x10\x22\xbf\xaa\x4d\x1d\x96\xab\xe1\x21\xcc\xad\xf0"
      "\xc4\xb8\x2a\x28\xa3\xc2\x92\xdd\xe8\x70\x8e\x78\x2e\x34\xbe\x7f"
      "\x3a\xff\xdb\x33\x92\x5f\x48\x7e\x55\xfa\x06\x98\xec\x5f\xe3\xc6"
      "\xa7\xa0\xb6\x25\x84\x7a\x72\xc4\x6b\x61\x30\xf7\xed\xd2\xaa\x0b"
      "\x73\x12\xbf\xc0\xaa\x63\x7c\xda\x1f\x81\x83\xa4\x77\x97\xd5\x37"
      "\x53\xa4\x27\xd1\xe6\x46\xa0\x00\xb6\x55\x8f\xc0\x0e\x1b\x92\xe6"
      "\x29\xa4\xab\xfa\xe2\xbf\x94\x4e\x82\x0e\x44\xbb\xe4\x8c\xbd\x13"
      "\x7c\x65\xc6\x70\x41\x84\xce\x7c\xd4\xa0\x53\x68\x43\x05\x4b\xbe"
      "\xa2\x33\x45\x2f\x62\xa7\x43\x16\x52\x7d\xac\xe2\x42\x8d\x3d\x77"
      "\x99\x04\xb8\x16\x66\x3f\x2e\xc4\x90\xac\x23\xf2\x38\xd4\xec\xe4"
      "\x32\x74\xdd\x78\xf5\x62\x9c\x26\xf4\x18\xa3\x03\xc5\x11\x65\xbf"
      "\xbd\x4b\xd9\xd7\xd7\xe7\x2a\x18\xbf\x4a\x55\x21\x28\xe9\xfc\x75"
      "\x91\x29\x07\xdb\xb2\x34\xf1\xcd\xfd\xaf\xc7\x1d\xd1\x1f\xc4\x4c"
      "\xa6\x80\x9f\xec\x8a\x34\x88\x53\x66\x63\xe2\x5f\x06\x7f\xba\xba"
      "\x0a\xce\x4e\x2e\xd2\x33\x94\x3f\xdc\xb2\xd8\x98\xc9\x90\x44\x35"
      "\x57\x01\xd7\xa0\x8f\x31\x64\x0f\x54\x35\x91\x8a\xe4\x40\x78\x85"
      "\x28\xfe\x62\x9e\x83\x21\xf6\x07\xc6\xf8\x3c\x21\xc7\xeb\xef\x33"
      "\xa2\x49\xf9\xcf\x6d\x00\xc2\x89\x94\x9a\xb3\xd6\x44\x85\x0b\xbb"
      "\x6c\x99\xdb\x4e\xe5\xb6\x21\xdf\x89\x88\x32\x36\x90\xf8\x4d\x1f"
      "\xe5\x12\x3f\xcd\xf8\x00\x8e\x84\x98\xc4\x9d\x4c\xbc\xd2\x59\xb3"
      "\x9d\xcb\x57\x4f\xb6\x2d\xb1\x1c\x4b\xa5\xaf\x88\xca\x4f\x5f\x63"
      "\x3f\x95\xdc\xe9\xa7\x12\xe7\x45\xd2\x18\x31\x92\x25\x19\x21\xa5"
      "\x0e\x80\x5d\x05\x2c\x31\x0a\xba\xc0\x4b\xd2\x5a\xb1\x54\xcc\xa8"
      "\xed\x52\xf0\x7e\xee\xca\x4c\x1a\x35\x40\x20\x91\x0a\xe7\xc5\xe3"
      "\xe1\x45\x44\x78\x49\x0b\x07\x03\xc2\x5b\xa5\xa0\x79\x2a\x68\x1a"
      "\xa3\xa1\xf2\x59\xfc\x5a\x34\x4f\xa4\x4b\xd5\x7e\xe2\x81\x74\x76"
      "\x83\xd4\x1c\xd7\xca\xac\x77\xc7\x89\xfa\xd4\x5f\x20\xc5\x1b\xad"
      "\x87\x35\xa3\x1e\xec\xb9\x44\xa7\xe7\xcc\x19\xc8\x58\xde\xc2\xa2"
      "\x66\xc1\x50\x8b\x53\x38\xa8\xe9\x88\xf0\x79\xaf\xc4\xd1\x10\xc6"
      "\xc1\xf6\x2e\x87\x88\x78\x16\xcb\x65\xa0\x5b\x38\x21\x8a\xcb\x2d"
      "\xa0\x8f\xe7\x39\x86\x60\x91\x17\x32\x5f\x2d\xd2\x81\x50\x2d\x06"
      "\x8e\xa0\xdd\x03\x0c\x97\x1f\x88\x44\x90\x91\x08\xaa\x10\x3b\x65"
      "\x26\x2f\xbf\x43\x95\x1c\x60\x77\xfc\x9e\x6a\x40\x3f\x70\xa7\x82"
      "\xaf\x66\xcf\x43\xeb\xaf\x46\xaf\x36\x1a\xeb\x0d\x3a\x88\xe8\x9c"
      "\x4e\x12\xcf\x5c\x4a\x8d\x83\xfa\x1d\x2b\x0a\x8d\xc1\xce\x04\xeb"
      "\x16\x72\x70\xcb\xb5\x63\x75\x05\xfe\x18\xcb\x0a\xa4\x7c\x57\xfa"
      "\xbe\x10\x18\x75\x37\x8d\xf1\xc5\x98\x3c\x1e\x49\x50\x59\x2e\xf0"
      "\x83\x1b\xf5\xcd\xd1\x22\xfe\x8c\x2e\xc7\xfe\x33\x8c\x6c\x05\x67"
      "\x67\xfd\xa5\xfe\xa8\x52\x60\x05\xdc\x7e\xfa\x52\x5e\xd3\x09\xe6"
      "\x5d\x38\xa9\x00\xc7\xbf\x0e\xf3\x2a\x4f\xc3\x01\x8e\x70\xd3\x07"
      "\xa8\xc9\x86\x82\xdd\x37\x49\xa1\x14\x3c\xd8\x0d\xf0\xed\xae\x6d"
      "\x83\x97\x6a\xc8\xf2\xbb\xd0\x72\x65\xf2\x53\x92\x72\x23\x35\xdb"
      "\x2c\x63\x61\x9c\xbc\x79\x35\x53\xd0\x27\xf0\x4d\x15\xb7\xb9\xc8"
      "\x1b\xab\xa5\x03\x1d\x6b\xad\x2e\x1c\x18\xbb\xe3\x7a\xb3\xac\x44"
      "\x51\x32\x30\x86\xde\x80\x08\xc7\xb6\x2b\x88\xfe\x54\x16\xf2\x3f"
      "\x08\x2d\x32\x92\x0f\x21\xb5\x7d\x8a\xdc\x58\xbd\xb4\x0f\x90\xb2"
      "\xfa\x07\x26\x91\x2e\xcb\x8f\x69\x49\x11\x83\x9d\xc5\x5e\x01\x65"
      "\x32\x59\xa5\x4e\x79\xb4\x25\x21\xc3\x55\x51\x65\xa6\x6e\x34\x5f"
      "\x7b\x6c\x19\xd2\x65\xa8\xbe\x14\xa1\xe7\x94\x57\x1a\x0f\xd0\xf9"
      "\xb0\xff\x06\x97\x2a\xc0\x35\x20\xc2\x52\xbf\x92\x29\x3b\xe6\x38"
      "\xac\xb6\x42\x82\x78\xdd\x7c\x43\x5f\xfd\x37\xbd\x2d\xfe\xd5\x8f"
      "\x1c\xed\xfe\x5a\x91\xd2\xe0\xe9\x5e\xff\x8b\x10\x0d\x7b\x89\x0e"
      "\xb9\x64\xa0\x42\xff\x49\xa2\x51\xb7\xd7\xb0\x95\x9b\xe8\xb1\xaf"
      "\xfb\x96\xb1\xa8\xa6\x9b\x9d\x46\xfa\x2a\xc6\x03\x89\xc8\xde\x56"
      "\xee\xe5\xba\x79\xee\xc8\x2a\xc4\x85\x6a\xe8\xc6\x1f\x7b\xa0\xa7"
      "\xb1\x61\xcc\x13\xff\x6b\xdc\xf4\x9f\x8f\x55\x3e\xd8\x69\x8e\x76"
      "\x7f\x39\x87\x8e\xb6\xe8\x9e\x98\x7b\x5d\x85\x54\xf9\xdc\x8f\xb9"
      "\x19\xe8\x97\x7e\x83\x6c\xbb\xe2\x65\xb2\x88\x95\xe0\x4a\xa0\x4b"
      "\x38\xdb\xe7\x21\xf6\xa3\xd8\x1c\x18\x14\x7b\x1d\xe2\xd2\x87\x75"
      "\x71\x08\x4c\x09\xaf\xa4\xbe\x07\x6b\x01\xba\x16\x66\x37\x5d\xc7"
      "\x48\x78\xb0\xd2\x34\x28\x26\x6e\x7a\xef\x72\x6c\x37\x47\x5b\x9a"
      "\x97\x41\xb1\x1e\x51\x2a\x9a\x3d\x07\xd7\xbd\x92\xe5\x1a\xf9\xd3"
      "\x85\x4c\xa5\x7b\x9e\x17\x25\x77\xd9\x2f\x7d\xd2\xe8\x1d\x29\x34"
      "\x54\xec\x60\x22\xeb\x62\x1a\x33\xb4\x1e\x8f\x65\x1e\x3a\xd5\x85"
      "\x3a\x4f\x30\x70\xfa\xf1\x7c\xae\x1c\x98\xb5\x72\x2b\xc2\x1b\xbb"
      "\x95\x1f\x8a\x05\x8c\xa9\x50\xa5\xe1\xba\xb0\xed\x35\xdd\xb1\x36"
      "\xba\x00\x8b\x59\xd0\xd2\xaa\xbe\x10\x3b\x01\x84\x64\x3a\x02\xa7"
      "\x2e\xda\x3e\xca\xfa\x8f\xa8\xe2\xe4\xba\x36\x59\x48\xf9\x2c\x75"
      "\x1e\x04\x27\x85\xba\x22\xea\x12\x48\x8c\xd2\x0c\x61\x3b\x82\x18"
      "\xaf\x96\x67\xad\xd9\x1f\xd2\xe7\xe2\x48\xe0\x5b\xa6\xd6\x4e\xd8"
      "\xde\xbb\xd9\x47\xc5\x07\x13\x99\xe4\x64\x6a\x06\x73\x31\x4b\x8e"
      "\xbd\xbd\x29\xa5\x28\x35\x78\x60\xde\x70\x78\x22\x4e\x07\x18\xee"
      "\xb9\x81\xbe\x17\xa0\x82\x66\xfa\x20\xaa\x1d\x16\x0d\xdc\x55\xa4"
      "\xe8\xef\x04\x3d\xf1\x2c\xed\x9d\xb1\x47\x67\x2a\xd0\x51\xd6\x4e"
      "\xca\x32\x5d\xce\x0b\x74\x31\x53\x1e\x84\xfb\xd8\xee\x0f\x3a\x23"
      "\x67\x2e\xa8\xb8\x44\xf4\x30\x06\xe3\x16\x48\x07\x8a\xe7\x3a\xee"
      "\x72\x16\x07\xf5\x5c\x0f\xa9\x9b\x86\x65\x54\x18\xe9\x6f\x1d\xba"
      "\x00\xf1\x79\x77\x91\x43\x1b\x51\xbd\x3d\xf0\x01\xdc\x0b\x4e\x04"
      "\x63\x1e\xc4\x8f\x55\x9a\xcc\xaf\x79\xe0\xe5\xad\x80\x94\x88\x7c"
      "\x6a\xf0\x15\x97\x37\x6e\x8a\x8a\x6a\x2c\xc2\xef\xf5\x1d\xf9\x55"
      "\x97\xb6\x89\x1d\x9b\x21\xb6\xeb\x5a\xa5\x6b\xa5\x1d\xd7\x02\xf1"
      "\x30\xdf\x52\x4e\xdb\xa8\x20\x98\xc7\xd7\x7c\x30\x25\xd8\x07\x6c"
      "\xee\xc5\xa5\x68\x85\x94\x71\x07\xd0\x6a\xb8\x04\x03\x27\x1c\xba"
      "\x63\xec\x93\x9b\x7a\x85\x50\x34\x9b\x69\xf9\xb6\x93\xf5\xed\xfc"
      "\xae\x54\x6b\x6c\x95\x89\xf8\xf1\xf4\x67\x58\xfb\xb8\x49\x5e\x5f"
      "\xce\xc3\x53\xa0\x39\xe0\x44\xae\x68\x9a\xf1\x79\x7a\x95\x9b\xfd"
      "\x84\xc8\x69\xcc\x55\x98\x33\x73\xcf\x76\x9f\xfb\x6a\x76\xcf\xfe"
      "\xd8\xe5\x52\x04\x08\xfa\x27\xd5\xeb\xa4\xf7\x93\x42\x5a\x6a\xf7"
      "\x45\x00\x3f\xa1\x83\xa5\x78\xd7\x5e\x16\x2e\xa6\x21\x24\x96\x35"
      "\xdb\xb8\xdb\x4f\xea\x81\x6b\xc5\xf9\xab\xbb\x2b\x92\x96\xeb\x0e"
      "\x09\x89\x31\x04\x87\x94\x7d\xa4\x74\x62\xdc\x03\x12\x82\x00\xd2"
      "\xe4\x7b\xdb\x03\x6a\xa9\xbb\x2c\x7e\xe5\x87\x85\xa1\xcd\x66\x28"
      "\x09\x21\xa7\x97\x81\x6e\xb9\xb5\x71\xcc\xdd\x67\x83\x2a\x8e\xeb"
      "\x4f\x3e\x2a\x06\x61\x38\x04\x49\x2a\xdc\xa4\x2c\x82\x78\xe6\x9c"
      "\xf3\x88\x80\x30\x60\x30\xdc\x97\xae\x43\x92\xe9\xde\x6b\xc5\x68"
      "\xec\xfa\x9c\x47\xfb\x11\x7c\x31\x61\x02\x75\xc6\xf8\x21\xe0\xc0"
      "\x57\x52\x11\x0c\x3b\x0f\x8b\xa2\x9f\xe5\x4a\xa4\xc1\x7c\x92\x6a"
      "\x5e\xe4\xbf\x39\xc2\x5a\x29\x22\xed\x15\xbb\xd1\xdc\x7a\x9f\x4b"
      "\x69\xe1\xb2\x86\x77\x02\x16\x9a\x87\x3a\x44\xb4\x78\x81\x72\x9f"
      "\xae\x9f\xe3\xf6\x93\x20\xd2\x78\xde\x8d\xfd\x9e\x1f\xfc\xfb\xa1"
      "\x06\x09\x68\x31\xb2\xa0\xfc\x29\x63\x18\x3b\xe1\xd3\xbd\x29\x41"
      "\xe8\x0f\x77\x57\xcf\xb0\xbd\xad\x10\x84\xcd\x77\x36\x8e\xcf\x98"
      "\xae\xff\x33\x4d\x91\xb5\x8e\x60\x47\x1d\x0c\x1d\x3b\x95\x51\x30"
      "\x1c\x0f\xcd\xf6\x76\xa6\x3d\x15\x2d\x7f\x63\xd5\xd8\x1c\xad\x89"
      "\xff\x63\x3e\x22\xd7\xb2\xb8\x38\x11\xed\x29\x5b\xce\x44\x01\x98"
      "\xd2\x83\xca\x3a\x30\x2b\x87\x5c\x92\x94\x70\x76\x9f\x4e\x99\xb5"
      "\x3a\x02\x3d\x96\x72\xd5\x93\xb4\x51\xcb\xb7\x1d\x66\x14\xae\x9a"
      "\xa5\x70\x36\x0b\x23\x59\x7b\x60\xed\x54\xbd\x83\xd9\x60\x5b\xdf"
      "\x8d\xbe\xc3\xa0\x0b\x7d\x42\xa0\x91\xe8\xeb\xfb\xb6\xec\x5b\x23"
      "\xe6\x32\xc7\xd0\xcc\xf9\xce\x98\x26\xea\xee\x2b\x6f\x9c\x18\x48"
      "\x6c\x0d\x56\xca\x89\xc7\x32\x24\xf4\x48\xf2\xb1\x09\xd7\x1d\xce"
      "\x8c\xf8\xc6\x71\x5c\x4d\xcd\x8d\xde\x16\x6d\x63\x3b\xb4\xba\x90"
      "\x9b\x49\xc9\xa8\xbc\x2d\x64\x9b\xb6\x5d\xda\x64\xd5\x33\x45\x47"
      "\xef\x00\x0b\x56\x01\x6c\x16\x6e\xb9\xe7\xed\xd4\x8b\xfc\x6b\x04"
      "\x81\x66\xd9\x1c\x49\x6b\xf6\xaf\x88\x41\x2d\xa5\x41\xf9\x35\xa0"
      "\x3d\xe3\x59\x43\xc9\x93\x53\xd4\xd8\x78\x3c\x20\xbf\xc6\x86\xd6"
      "\xbd\x72\x2e\x39\x90\x5f\xd0\x4c\x59\xc4\x3b\x7d\x33\xa7\x79\x66"
      "\x91\x1c\xa1\x9f\xa0\x3c\x2a\x67\x55\x25\x07\x31\xff\xcd\x12\xc1"
      "\xf7\x72\x2d\x13\x78\x85\x9c\x18\x52\x7b\x29\xea\x4f\x39\x7f\x00"
      "\x78\x00\x4d\x0f\xf6\xb2\x4e\x56\x38\xc3\x4e\x32\xf5\x43\x02\x40"
      "\xf5\xc2\x78\x53\x8b\xe7\xad\x0e\x7f\xe6\x83\x67\xfc\xf4\xd6\x11"
      "\x76\x92\xfe\xb9\xa8\x27\xe6\x53\xa1\x0a\xa3\xbc\x50\xbb\x40\xb0"
      "\xed\x3e\xef\x60\xc4\x29\x1d\x7c\xc4\xeb\x90\x28\xc0\xfe\xad\x68"
      "\x78\x8d\xa3\x77\x76\xb6\x83\xfc\x4e\x43\x44\x32\xb3\x01\x12\x32"
      "\x63\x0f\x3d\x8c\xa3\x57\xcd\x4a\xd7\x8d\x4f\x27\xf9\xa9\x5f\x33"
      "\xc4\xe0\xc2\xcb\x9a\x32\x52\x2e\x45\x88\x7c\x2e\x55\x20\xe4\xa0"
      "\x24\x26\xd3\x4a\x75\x44\xd1\x2d\xaf\x55\x91\x10\x50\x05\xd1\xf4"
      "\x01\x48\x5d\x40\x4b\xf4\x8c\x2c\xcf\xb0\x73\x84\x47\xf4\xe6\x26"
      "\x58\xbd\xfc\x4d\x06\xd9\xf2\x41\xef\xbb\x19\x80\xe5\x3b\x86\x9f"
      "\x0c\x76\x4d\x33\x51\x77\x59\xe3\x80\x3d\xe2\x7b\x31\x3b\x5b\x4e"
      "\x69\x97\x97\x8c\xc4\xbb\x22\x88\x3a\x96\x4f\x71\xfe\x1a\xe7\xba"
      "\x43\xab\x6c\x3a\x54\x4d\xb5\x20\x8b\x2c\x6e\xb0\x1d\x27\x96\x29"
      "\x29\xbd\x13\xb9\x73\xa5\x2a\x02\x18\x6a\x62\x1a\x8c\x85\x2f\x72"
      "\xd0\x5a\xba\x2a\x4a\x0d\x91\x31\x4e\xcf\x8a\x34\x9f\x13\x68\xd1"
      "\xb8\xa3\x28\xdc\xaf\x2b\xdf\xc9\x4c\x9f\xc9\xc9\xae\x91\x38\x71"
      "\xa5\xb7\xab\xef\x6e\xa1\xa7\x89\x2d\x0d\x4d\x06\x65\xef\x84\xe9"
      "\xe1\x2b\x87\xc4\x12\x0c\x4c\xd6\x90\x7c\x77\xc6\xb4\x67\xe3\xb0"
      "\xdf\xd9\xb8\x0b\x74\x5d\x60\x8c\x9b\x7c\x85\xa3\x64\xf8\x95\xdd"
      "\x1a\xde\xc4\xb1\x59\xa5\x50\x49\x6c\xdd\xec\x77\x79\x7a\xfd\x29"
      "\xc0\x65\x95\xbe\x11\x81\xd6\x2e\x00\x91\x62\xc6\xb6\x8f\x97\x5c"
      "\x67\xac\xda\x34\x2d\x04\x17\xca\x1a\x5a\x60\x80\xc8\x05\x32\x2f"
      "\x41\x31\x05\xb3\x32\xe0\x6c\xfb\xc9\x3a\x62\x27\x43\xa6\x33\x5f"
      "\x97\x03\x03\x9a\x7d\x53\x6a\x64\x1c\x6e\xe4\x4a\x72\x57\xb9\xb6"
      "\x23\xee\x60\x04\x76\xfd\xa1\x63\xc1\x1f\xcc\x30\x5e\xe9\xbb"
      "\x0b",
      4096);
  r[9] = syscall(__NR_setsockopt, r[1], 0x117ul, 0x1ul, 0x20ea5000ul,
                 0x1000ul);
}

int main()
{
  loop();
  return 0;
}
