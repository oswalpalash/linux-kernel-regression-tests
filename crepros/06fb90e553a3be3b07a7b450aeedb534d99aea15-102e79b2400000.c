// https://syzkaller.appspot.com/bug?id=06fb90e553a3be3b07a7b450aeedb534d99aea15
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
void loop()
{
  long res = 0;
  res = syscall(__NR_socket, 2, 1, 0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_socket, 0xa, 1, 0);
  if (res != -1)
    r[1] = res;
  syscall(__NR_mmap, 0x201ed000, 0x4000, 0, 0x4012, r[1], 0);
  *(uint16_t*)0x20942000 = 2;
  *(uint16_t*)0x20942002 = htobe16(0x4e20);
  *(uint32_t*)0x20942004 = htobe32(0xe0000001);
  *(uint8_t*)0x20942008 = 0;
  *(uint8_t*)0x20942009 = 0;
  *(uint8_t*)0x2094200a = 0;
  *(uint8_t*)0x2094200b = 0;
  *(uint8_t*)0x2094200c = 0;
  *(uint8_t*)0x2094200d = 0;
  *(uint8_t*)0x2094200e = 0;
  *(uint8_t*)0x2094200f = 0;
  syscall(__NR_bind, r[0], 0x20942000, 0x10);
  *(uint16_t*)0x20000100 = 2;
  *(uint16_t*)0x20000102 = htobe16(0x4e20);
  *(uint32_t*)0x20000104 = htobe32(0x7f000001);
  *(uint8_t*)0x20000108 = 0;
  *(uint8_t*)0x20000109 = 0;
  *(uint8_t*)0x2000010a = 0;
  *(uint8_t*)0x2000010b = 0;
  *(uint8_t*)0x2000010c = 0;
  *(uint8_t*)0x2000010d = 0;
  *(uint8_t*)0x2000010e = 0;
  *(uint8_t*)0x2000010f = 0;
  syscall(__NR_connect, r[0], 0x20000100, 0x10);
  memcpy(
      (void*)0x20004140,
      "\x8f\xc7\xbb\xa0\x72\x8f\x8e\x6a\xaa\x0a\xb7\xf5\xf3\x3d\x8b\x87\xed\xe7"
      "\xf4\xe4\x7c\xd2\x33\x0f\x88\x97\x40\x4c\x2a\x62\x5b\xef\xcc\xba\xed\x50"
      "\xa7\x43\x6c\x34\x4e\x60\xa4\x17\x2c\x08\xd0\x26\x06\xd3\xdb\x82\x27\xf0"
      "\x6f\x65\x50\x22\xe9\xed\xef\x9a\xe9\x63\xa0\xa4\xbc\x93\xe7\xd7\xa5\x21"
      "\xfc\xc1\x00\xfe\xfd\x5b\x2a\xd1\x6a\xde\x94\x21\x62\xdc\xa7\xad\x7a\x48"
      "\xcb\x44\xce\x31\x5c\x47\x53\x95\xb0\x9c\x06\xaa\xf0\x4d\x2c\xc6\x30\x92"
      "\x21\x46\x6d\xd6\x06\xb0\x11\x02\xf7\x32\x5e\xf3\x71\xf7\x8a\xbf\x4d\x74"
      "\x7f\x33\xba\x4f\xca\x91\x39\x06\x84\xb9\xe3\x9c\xce\x78\x49\xa1\xc3\x2c"
      "\x1d\x2b\xb1\x3a\x1d\x6d\x6c\xce\xcb\x8e\xde\xe6\x8a\xce\x50\x54\xc2\x0b"
      "\xcf\x41\x11\xf2\xf8\x44\xbd\xe3\x56\x11\x3a\xcd\x7d\xcb\x72\x6f\xf9\x4f"
      "\x7b\x7a\xd8\xe1\xa2\xbd\xb7\xf5\x90\x20\x34\xfb\x6a\x6e\x18\xf0\x94\x47"
      "\xec\xaa\x82\x09\x22\x6a\x10\xef\xd0\x27\xc4\x29\x37\xc9\x21\x4a\x17\x76"
      "\xa4\x8c\x3b\x69\x1c\x2d\x9a\xec\xa4\x2e\x37\xd1\x7c\x7f\x00\xcd\x80\x5d"
      "\xa0\x49\xa0\x89\x67\xb8\xe0\xe1\x03\xc2\x4f\xb4\x1f\xbf\x06\x6f\x0a\x62"
      "\xcc\x3f\x1a\x31\xc3\xb6\x10\x08\x63\xc5\x72\xef\x4d\x38\x5f\x13\x2a\x54"
      "\xa9\x1c\x7f\xf3\x1d\x71\x2c\xb8\x0f\xed\x24\xf3\xfc\x90\x09\xb4\x00\x83"
      "\x33\x9a\x89\x57\xf4\x6d\x43\x92\x65\x7c\x7f\xd5\xe3\xb6\x9e\xd1\x75\x02"
      "\x52\x09\x80\x6f\x8c\x04\xec\xad\x6f\x06\x63\xb6\x29\x83\x02\x2f\xc4\xba"
      "\xb8\x14\x27\xb3\x7c\x96\xa3\x1c\x97\xba\xc4\xfd\x9f\x62\x45\x82\x3d\xb7"
      "\x93\xaa\x67\xd6\xee\x16\x77\x5f\xce\xcb\x7a\x4d\xe6\x14\xf2\x0f\x74\x0c"
      "\xce\x27\xbd\x76\x7e\xda\xe5\x04\xf9\x99\x63\x00\xbe\xb0\x7c\xc6\x3a\x23"
      "\xb6\x95\x5c\x7d\x50\x56\x19\xe3\x6d\x7c\xe3\xfe\x53\x4c\xd2\xb9\x8c\xb1"
      "\x2f\x20\x6b\x60\xb7\x11\x17\x45\x74\x75\x1d\x70\x02\x27\xa2\x3b\xe7\x02"
      "\xbd\x16\xc8\xc3\xd7\xd3\x2b\x18\x41\x4b\x3a\xa3\x35\xe2\x53\x57\xf6\xf9"
      "\xa2\xf6\x2b\x7f\x4c\xae\x38\x82\x32\x85\x3d\x8d\xa0\x5d\xe5\x54\x40\x48"
      "\xa5\x72\x68\x9f\x12\xb3\xfd\xc8\xba\xf9\x9c\x7a\x66\xc3\x33\xbe\x4b\x36"
      "\x99\x69\x4b\xca\x1b\x3f\xcb\x25\xc5\xce\x19\x2c\x0a\xe3\x1b\x02\x6d\xd5"
      "\x92\x4b\x33\x6a\x41\x3e\x33\xe3\xe5\x31\x6f\x9c\xc3\x69\x8f\x24\x10\x7e"
      "\x11\x80\xc6\xe9\x5c\xbb\x9f\xa4\xf2\xa1\x6c\x1e\xd1\xf7\xca\xb9\x97\x05"
      "\x1b\xbc\x8e\x33\x79\x23\xab\xe3\x66\xdf\xec\xe3\xfd\x26\xea\xe2\x1c\xd4"
      "\xf2\x10\xd7\xa4\x03\xc5\xc7\x89\xb1\x01\x74\xa5\x37\xa0\x5d\x6e\x94\xe7"
      "\x52\x88\xe6\xe2\x4f\x70\x26\x05\x94\xd2\x45\xce\x28\xdc\x56\x48\xc8\x4a"
      "\x29\xf8\x0c\xd7\x4d\x30\x11\x86\xc3\x5a\x1e\xdd\xdd\x86\xef\xb5\xd3\x31"
      "\xf1\x68\x46\xf8\x04\xff\x0a\x82\x19\xa5\x33\x6e\x16\x53\xc4\x5a\x02\xb8"
      "\x12\xcc\x22\x3e\xb4\x1f\xcb\xe1\x54\x02\xe7\xbc\xb2\x6f\x32\x63\xe7\x26"
      "\x1e\xfc\xa4\x0f\x73\xe8\xd9\xda\xca\xb2\xbe\xe7\xea\x0a\x43\x2b\xe5\xcb"
      "\x26\xdf\x3b\xe3\x9d\x2d\xa4\x57\x2a\xf5\x66\x95\x6c\x53\x64\xa3\x6f\xbc"
      "\xa7\x7b\x4b\x18\x84\x0c\x05\x73\xa5\xff\x1d\x19\x96\x1a\x31\x02\x26\xa0"
      "\x97\x11\xd3\x28\xf5\x5d\xe4\x39\xaf\xd2\xc6\xbb\x93\x05\xe8\x03\x13\xaf"
      "\x9f\x4a\x8f\x53\x92\x31\x2a\x3c\xfc\x64\x48\x62\x17\x5e\xd3\x0e\x34\xec"
      "\x05\x51\x4a\x9d\xa4\xc7\x93\x84\xaa\x87\x87\x08\x6e\x1c\xad\x21\x13\xfd"
      "\x4f\x85\x3d\x1d\x23\x2e\x67\xe9\x50\xcc\x0d\x84\x37\xa7\x3d\x3f\xdd\xd4"
      "\x5c\x87\x68\xcd\x62\x67\x92\x01\xb7\xd9\x87\x12\x14\xec\xf0\xe8\x98\xaa"
      "\xd8\x6c\x76\x4f\xfa\xab\xdc\xc2\x2e\xe3\x92\xc4\xd3\xfa\x27\xd8\x8e\x80"
      "\x30\x0c\x4d\x2d\xc4\xc3\x37\x13\x9c\x34\xa1\xf0\x7c\x13\xf1\x17\x5a\x9c"
      "\xce\x3b\x8f\xe9\xd3\xc8\x78\x19\x31\x72\x0a\x79\x9b\x2e\xd4\xf7\xb6\x1e"
      "\xb3\xcb\xf0\x26\xd3\x15\x2f\xe5\x17\xc9\x65\xbe\x01\xa9\x2c\x2c\xba\x5c"
      "\x8b\x6c\xef\x17\xf5\xce\xbd\xea\x1d\xf8\xb7\xb2\x6c\xe4\x39\x55\x2d\xb9"
      "\xe7\x52\x38\x69\x95\x43\xf9\xdc\x8a\x96\x46\xe2\xcc\x2f\x70\x22\xb9\x46"
      "\x32\xb0\xaf\xd4\x95\x55\x86\x1a\x56\x3c\xc6\x27\xd0\x2a\xb8\x84\x78\x23"
      "\xe4\x56\x9e\x77\xab\x3f\xb4\x11\xf0\x6b\x79\xcf\x2f\xd1\x78\x96\xe5\x9f"
      "\x99\xc6\xb0\xa6\x86\xef\xaf\x1e\xa9\x93\x06\xe0\x39\x12\xd9\x3e\x35\x0f"
      "\x16\x23\x6e\x4b\xf6\xe2\xb7\xb1\xc4\xfe\xf0\x1f\x19\x9d\xff\x65\xdc\x4d"
      "\x76\xdb\x72\xd7\x49\x23\xdf\x94\xe2\x51\x7c\x67\xde\xf8\xb3\x98\xe4\xf3"
      "\x53\x51\xe1\xa7\x02\x36\x88\xfe\xf8\x74\xdc\x23\x64\x4e\xd0\x72\xcd\x2d"
      "\x9b\xf8\x13\x66\x3e\x36\x38\xb6\x28\xd9\x7c\x65\x33\x2b\x43\xc4\xfb\x5a"
      "\x8e\x26\x55\x0a\x25\x9f\x16\x85\xa0\xc3\xfa\x2f\x8f\x49\x52\xb1\x25\xdf"
      "\x01\x0f\xf5\xa4\x23\x03\x7e\xda\x9e\x68\xbf\x29\x82\x94\x0a\x9b\x52\xa0"
      "\xf4\x32\x5e\x9d\x2e\xc7\x5a\xb9\x5e\x58\x88\xab\x5d\xd6\x34\x30\x48\x97"
      "\x25\x43\x86\x62\xa1\xde\xfa\x6d\x64\x72\xa3\xa9\xbf\xfb\x32\x13\xfb\xde"
      "\xef\x41\x7f\xea\x3d\xfd\xbe\x66\x14\xac\x83\x73\xb1\x3d\x5c\x81\x60\x44"
      "\x93\x7a\x87\x43\xf0\x00\xa5\xe4\x96\xf5\xce\x73\x84\x05\x63\xf4\x4a\x98"
      "\x4a\x8c\x54\x55\x25\x87\x1c\x10\x8a\x4b\x1d\x96\x1c\x63\x95\xd2\xc3\x26"
      "\x38\x2b\xe0\xd2\x34\x55\x3e\x87\x53\xd1\x81\x8d\x5b\x0e\x6b\x7d\x31\xe5"
      "\xc7\xf1\xe7\x8f\x1e\x43\xbc\xe8\xde\x6d\xfc\xa6\xad\xec\x2d\x65\xfc\xfe"
      "\x52\xa0\xff\x3a\x9f\x66\xc2\xd4\x37\xc1\xe9\x3e\x9b\x8b\x55\xf2\x2c\x97"
      "\x74\xc3\x25\x88\x7c\x83\xd7\x77\x57\xf6\x16\xae\x93\x11\x62\x53\xb7\x0c"
      "\xad\x4c\x63\xdb\x7d\x71\x03\x08\xae\xd9\xf0\x78\x1d\xbc\x01\x6e\xa0\xe6"
      "\x21\x8e\xbc\xa9\x3e\x59\xfd\x68\x20\x84\xa3\x55\x39\x55\x41\xa8\xab\x72"
      "\x80\xce\xfe\x97\x7c\xc6\x72\x29\x16\xc4\xe2\x96\xa1\x80\x7b\x8f\x43\x36"
      "\x07\x3a\x81\x00\x9c\x83\x23\xe5\xb2\x84\xcf\xac\xe5\x82\xd8\xd8\x2a\x61"
      "\xdd\x91\x75\xbf\x29\x7b\xaa\xbd\xb1\xea\xad\x95\x21\xde\x69\x3e\x1c\x18"
      "\x6c\xb9\xe5\xea\x12\x08\x53\xc2\x4e\x0d\x2e\x30\x9a\xe9\x66\x7e\xf8\x52"
      "\xfe\x69\xfe\xc7\x40\x93\x89\x33\x19\xa4\x56\x03\x20\x1b\x30\x08\x5e\xbc"
      "\x38\xb7\xb3\xd3\x65\x9a\x60\xf1\x63\x37\x8b\x87\xa1\x0b\x93\x77\x10\xfa"
      "\x9c\xf6\x15\x47\xfe\x9d\x23\xad\xd7\x63\xc4\xee\xbf\xd4\x3f\xbf\xc2\x94"
      "\x29\x9e\x83\x70\x4e\xe3\xcb\x59\xcd\x07\x4b\x28\xf6\x85\x4d\x0b\x18\xe3"
      "\x36\x2c\xc2\x9c\xd6\x17\xbc\x67\x0e\x4c\x79\xae\x27\x69\xcc\xea\xff\xc6"
      "\xab\x10\xde\x82\x82\x03\x4b\x8b\x6d\x4f\xc8\xea\xf8\x3f\x46\xa9\xfe\xf9"
      "\x64\xf1\x09\xba\x72\x91\xdf\x95\x8b\xb0\x72\x9d\x13\x00\x7b\x2c\xe3\x69"
      "\x69\x65\xf9\xa1\x10\xee\x00\xf2\x1e\x74\xaa\x8b\xa2\x4b\xc0\xea\x81\x91"
      "\xbc\xdd\x3b\xe8\x68\x11\xcd\x85\xb0\xfe\x00\xcc\x7a\xf8\x97\xb1\xe9\x37"
      "\xe2\xaa\xbd\x49\xa0\x73\x2a\x68\x20\x22\x29\xcc\x91\x97\xc3\x27\x01\x78"
      "\xcb\x83\xd0\x0f\x84\xb6\xec\x98\xa7\x88\xac\xd8\xcd\xec\xe7\x5f\x83\x70"
      "\x3a\x77\x66\xab\x98\xeb\xbf\xfc\x63\x7b\x5d\x51\x26\x02\x7e\xf3\x54\xb1"
      "\xce\x87\xe1\xd5\x95\xfc\x24\xe8\x93\xe5\xcf\x54\xcd\xcf\xba\xe1\x12\xa2"
      "\x90\x66\xdf\x42\x19\xaa\x23\x8d\x38\x24\x70\x93\xb6\xb2\x62\xfb\xac\x4e"
      "\x0a\x61\x52\xf1\x01\x07\x5a\x32\x01\x81\x81\x7c\x90\x69\xb5\x80\x51\xf7"
      "\xdd\xdb\xeb\xa5\xe1\x59\xa6\xe7\xd2\xf2\xfb\x34\x81\x6e\x3e\x96\x72\x61"
      "\x90\x30\x38\x98\xc5\xa3\xb5\x5e\x92\x19\xad\xf6\x16\xb9\x17\x27\x7f\x3c"
      "\xba\x0f\x76\x8d\xe7\x75\x5e\xfc\x42\xc6\xfd\xb4\x78\x63\xc3\x40\x6f\xc7"
      "\x06\x43\x9a\xd1\x77\xd5\xf4\x20\xa7\x40\x5b\x55\x23\x7b\xd3\x73\x31\x3f"
      "\x93\x08\x8e\x94\x03\xf7\xcd\x1f\xaf\xe2\xaa\x95\xae\x1b\x8c\x21\xd7\x30"
      "\x5d\xa3\x70\x6b\x0c\x10\x95\x7e\xe1\x3b\xcd\xed\x39\x06\xe7\xb6\xa5\x5a"
      "\x55\x81\x6d\x16\x2f\x66\x3c\xd3\xd5\x73\x0f\x13\xf3\x62\x9e\xb9\xe5\x12"
      "\x22\xb2\xdd\xee\x6b\x5c\x95\x5a\x86\x4b\x7f\x72\xca\xd0\x12\xbf\x5b\x4b"
      "\x35\x9f\x27\xd8\x40\xd1\xbb\x2b\x38\x05\xb1\x90\xd0\x35\x21\xb9\x7f\xeb"
      "\x83\x6a\xc4\x3d\xb8\x28\xc7\xd1\x58\xf1\x59\x07\xb2\x48\x10\x4d\x7f\xa1"
      "\x94\xfd\x1f\x78\x70\xfd\x75\x80\xb5\x3f\x39\xa7\xc3\x4c\x3f\x8f\xc1\x1f"
      "\x21\x6d\xae\xd6\x02\xbc\x9e\xc5\x82\x33\xbf\x89\x04\xd9\xd5\xfe\x79\xd6"
      "\xe9\x6b\x5f\x62\x98\x44\x29\x4d\x09\xd1\x34\xf6\xc0\x4d\xd0\xd1\xd7\x69"
      "\xbb\x03\x1b\xad\x22\xe0\xb0\xba\x90\x45\x1c\xf5\xcd\xaf\xed\xb4\x1a\x82"
      "\x7f\x50\xa4\x44\xb6\x93\x9a\xac\x7a\xdb\x01\xff\x19\xfe\x43\x02\x93\xcb"
      "\x45\x33\x8d\x7f\x5b\x66\x34\x24\x8f\xae\x21\x53\xf3\x97\xb7\xa5\x54\xf1"
      "\x1b\x41\x0d\x76\xb2\x11\x70\xf0\x04\x93\xf0\x07\xd9\xd6\x72\xc0\x76\xc8"
      "\x12\x2d\xd1\xdc\xee\xdf\xcc\x03\x8d\xca\xb6\x27\xb0\x10\xbc\xcc\x25\x11"
      "\x4d\x9d\xf7\xa0\x2c\x01\x4f\x70\x27\xf3\x8a\xa6\x07\x9b\xe9\xac\xaf\xec"
      "\x18\x87\x42\xb4\x24\xbe\x9c\x61\xb3\x06\xd6\x63\xb6\x0f\x2f\x0e\x0d\xf0"
      "\x59\x62\xb8\xfe\xaf\x03\xd7\x7e\xef\xd5\xe3\x1d\x0a\x31\x75\xf7\x82\xcb"
      "\x8b\x5c\xb4\xbf\x1a\x69\x48\x14\x55\xac\xd5\xf1\xfa\x65\x71\x59\xe3\x5d"
      "\xf6\xbf\xbb\x6c\xc0\x31\x3a\x19\x38\x2a\xfa\x84\xd3\x76\x95\x07\x2e\xb5"
      "\x9f\x8d\x6d\xe9\xd6\x18\xb2\x44\xeb\xa1\x86\x68\x3b\x88\x16\x37\xcc\x5c"
      "\xa0\xaf\xb7\xa4\xbc\xcb\x76\xf0\xb9\x8d\x28\xb0\x67\x0d\x7a\x56\xeb\xbd"
      "\xf9\xbb\x03\x83\xde\x5f\x36\x20\xae\x91\x15\x84\xfe\xf0\xad\x99\xa8\xa2"
      "\x0d\xbc\xed\x4f\x7d\xc1\x11\x3f\xe1\x59\x61\xa5\x07\x5f\xaf\x04\xc4\x7c"
      "\x67\x3f\xc2\x3d\x10\x47\x5b\x99\x6a\x5f\x2e\x13\x3c\xc0\xff\xf2\x09\x17"
      "\xa0\x7e\x97\x66\x44\x01\xd1\xdb\x25\x44\x7a\xb3\x77\x74\xe1\x91\xbd\xe8"
      "\x78\x9e\xbe\x71\x6a\x07\x1a\x27\xd8\x51\x1d\x83\x80\x82\x0c\xfe\x4b\x3b"
      "\x77\x80\xaa\xdb\xee\x52\xc7\xa8\xaf\xea\xd0\x82\x68\xb3\x17\xe4\x5f\xf0"
      "\xdb\xd4\x6d\x4c\xa1\x79\xd9\xae\x27\xdd\xcb\x80\xe0\x2b\xd1\x17\x35\x80"
      "\x6d\xb9\x19\x8c\x6b\x99\x56\x4a\x51\x42\x70\xa4\x91\x2e\xff\x3a\x19\xc0"
      "\x89\x03\x02\xbe\xd4\x80\x43\x4f\x1f\x79\x1d\xbd\x6a\x05\xf1\x79\xea\x68"
      "\x81\xf3\x41\xca\x05\x55\xd3\xea\x7a\x3d\xe8\xa5\x2d\xdb\xf8\x25\x67\x67"
      "\x93\xee\xa8\xf4\xb1\xc2\x4e\xec\x7c\x66\xf5\xc5\xbc\xfb\x44\x8a\x6e\x30"
      "\x07\xf9\x58\x72\xd1\x13\xbb\x5c\x2f\x46\xc8\xce\x10\x99\x31\x6a\x2d\x78"
      "\x9c\x7e\x2e\x6e\x94\xb0\x8c\x68\x71\x95\x6f\x5b\xa6\xcd\xf7\x04\xb5\x29"
      "\x0f\xbd\x49\xa2\x84\x3a\x08\x86\xfa\x5d\xd9\x99\xbc\x13\x89\x12\xb9\x73"
      "\xdb\x3c\xc8\xad\xbf\x4c\x22\x9e\x06\x43\xde\xf3\x94\x10\x67\x0d\x68\xd0"
      "\xd8\x58\xd4\x29\x21\x3f\x57\xb4\xd3\x45\x33\xa2\xf4\xb4\x3d\xe3\x03\xcf"
      "\x4b\x86\x91\x79\x18\x82\xaa\x04\x0c\x90\xb1\xac\x2a\x74\x64\x63\xab\xe5"
      "\xa3\xec\x54\xa7\x10\x7d\x69\x48\x4c\x0c\x2a\x3b\x94\x50\xcc\xbc\xab\x82"
      "\x5d\xcd\x54\x59\xa6\x5e\x37\x9d\xf3\x2a\xe2\xd7\x8d\x8b\x01\xf7\x15\x76"
      "\x8f\xdd\x60\xe9\x9c\x33\xf2\x20\x46\x54\xcf\x9b\xde\xd7\x25\xfa\x39\x47"
      "\x75\x03\x12\x7a\x3c\x48\xa1\x70\x86\x87\x32\xb3\xc4\xa6\x81\xe2\x89\x01"
      "\x50\xbf\x52\x74\x19\x96\x9a\xab\x4b\x0e\xa5\xe7\x2a\x90\x51\xb8\x94\x8c"
      "\x1a\x50\x2d\x90\x9a\xca\x46\xfb\xdf\x73\x3f\x1f\xf1\x5d\x58\xac\xb1\x7f"
      "\x8a\xbd\x05\x3f\x96\x9c\x6b\x4e\x5b\x44\x4f\x38\x77\x60\xc0\xfd\xd1\x36"
      "\xac\x77\xe7\xdd\x5a\x76\xef\x95\x03\xf5\x3e\x6e\xd5\x9f\x4a\x82\xcc\x13"
      "\x67\x75\xde\x46\x00\x18\xe4\xb6\x56\x3f\xf4\x55\x3c\x68\x86\x33\xc3\x56"
      "\xb7\x3f\x0c\xf5\xd9\x25\x75\x5f\xdd\x72\x84\xe0\x56\x85\x9a\x35\xaa\x7f"
      "\x07\x24\x18\xc9\x7f\x7e\xdf\xbc\x0f\x8c\xf7\xdd\x33\x7c\xe6\x03\x37\x40"
      "\xdf\xe9\xb0\x6d\x08\xa8\xc9\x03\xe3\x74\x71\x72\x8a\x3f\xac\xca\xca\x7d"
      "\x98\x95\x89\x0e\x69\x50\x48\xcf\x82\x77\xb0\xe4\x2e\x05\xaa\x1a\x1c\xa5"
      "\xc8\x0d\xb7\xa3\x29\x61\xaf\x20\xfe\x14\x3d\xe9\x84\xb0\x8b\xfd\xeb\xf8"
      "\x3f\x0b\x3b\x46\x51\x3a\x97\x5e\x3b\x7c\x5d\x29\x8f\x8d\xdb\x0f\x4c\x05"
      "\x80\x80\x18\x69\xca\xea\x4c\xf4\x89\x87\x22\x73\x17\x5b\x7d\x0a\x0e\x6c"
      "\xb2\xf4\xdf\xe8\xdc\x93\x90\x0e\xe0\x57\x4c\x96\x3d\x6e\x76\x6e\x9a\x24"
      "\x9f\xb9\xef\xf7\xdd\x57\x8a\xab\x90\x03\xb3\x6c\x4d\x54\x74\x1f\x44\xa8"
      "\x50\x74\x4b\x03\x99\xf2\x86\xe4\x91\x29\xe4\x9b\xba\x1f\x81\x88\x55\x57"
      "\x9a\x9d\x54\x25\x46\x65\xc1\xf7\x96\xb9\x65\x68\x40\x95\xa8\x44\x12\x3e"
      "\x28\x8d\x9d\x5b\xb8\xbd\xb0\xcf\xc3\xb3\xcc\xa7\x63\x0f\x22\xc3\x17\x1e"
      "\x77\x4c\xa7\x80\xc6\xb8\x21\x3c\xae\x50\x99\xdf\x5b\x62\xb8\x5f\xc3\x29"
      "\x07\x79\x63\xe5\x1a\x43\x42\xec\xec\xfc\xa5\xc1\x8e\x52\xa9\xc7\xe1\x43"
      "\xb6\xce\x8b\xbf\x62\x0f\x56\xf4\xf1\xcd\x1a\xa1\x09\x82\x7c\x9a\x6b\x53"
      "\x74\x99\xf7\x32\x0d\xe3\x59\x44\x6e\xb6\xee\xcf\x26\x9e\x36\x87\xea\x84"
      "\xd1\x35\x8e\x7e\x91\x6c\x73\x23\x0d\xab\x8d\xfa\x07\x34\xc6\xd9\xa9\xc1"
      "\xfb\x5d\xa8\x47\x7d\x9f\x15\xfd\x59\x8c\x52\xcf\xf0\xd7\x67\x4f\xfc\xa7"
      "\x0f\x54\xec\xc8\x9c\x83\x67\xdb\x22\x5b\x8b\xde\xa6\xf7\xad\xfc\x15\x43"
      "\x4c\x86\xd2\x40\xaa\x35\xd1\x98\xf6\x60\xac\x67\x85\xd3\x58\xe0\x56\x58"
      "\xf4\xf0\xad\x69\x1b\xf8\x90\xf3\xbf\xd2\x67\xc1\x77\xbb\x03\x29\x4a\xef"
      "\x99\x5e\xbf\xf4\x5b\x54\x32\x78\xa9\x98\x17\x27\x8e\x65\x51\x0a\xf0\x8d"
      "\x72\x1c\xd4\x83\xcc\x37\xbd\x05\xb4\xcf\x66\x94\x2a\x91\xc9\xe0\x62\x2a"
      "\xab\x7e\x27\x6f\x05\x75\x88\x56\x70\x0d\x55\x95\x10\x44\x1e\xfa\x7e\xe9"
      "\x84\xa8\x53\x36\x18\x37\x58\x41\x03\x04\x4d\xda\x76\xf1\xc5\xf5\xb4\xd1"
      "\x55\x12\x03\x1c\xfb\x22\x7a\xa0\x0c\x20\x2f\x34\xad\x90\x4b\x17\x22\x33"
      "\x45\x0a\x54\x9e\x1c\x7a\xbb\xa1\x59\xe3\xc1\xe3\xfa\x20\x76\xff\xac\x7e"
      "\xd6\x59\xbf\xa5\xb7\xa1\x93\x41\xf4\x6a\xdd\x1f\xd7\x20\x11\x0e\x26\x5e"
      "\xe0\xa6\xb2\x09\xc9\x23\x38\x8f\x5c\xd9\x87\x97\x47\xb4\x67\x76\x7d\x3a"
      "\x19\x99\xd7\x43\xe8\xdc\xed\xf2\xa9\x20\x3b\xec\xac\x8d\xe0\x28\x74\xda"
      "\xf3\x17\x04\xf4\x24\xb5\x38\x4d\x32\x1e\x5e\x16\x96\x47\xe7\x55\x07\xe2"
      "\x06\x65\x6e\xc7\xe9\x12\xa3\xaf\x83\x38\x75\x0a\x65\x24\x2c\x8e\x0e\xc5"
      "\x1d\x25\xc4\xa0\x2d\xe2\x10\xe2\x91\xc4\x46\x14\xcf\xd5\x30\x6b\x66\x0c"
      "\x2c\xd4\x92\x5c\x8c\x51\x81\x39\xfd\x63\x87\x4e\x33\x97\x4b\xed\xcc\x8e"
      "\xf7\x59\x73\xb4\x50\x78\xb2\x04\xc3\x76\xff\x05\xb0\xe8\xac\x12\x03\x7c"
      "\xa5\x47\x4f\x74\x76\xf7\xf6\x12\x46\xab\x28\x28\x0c\xd1\x14\xf2\x7d\xd2"
      "\x30\x66\xe5\x94\xde\x42\x74\x16\xaf\xbd\x18\x74\xba\x5a\x38\x5c\xf4\x93"
      "\x85\x67\xfa\x3d\x67\x20\xd1\x75\x2f\xa1\xe1\x34\xf3\x42\x12\x4a\xaa\x92"
      "\x65\x58\x8a\x25\x13\x53\x5b\x06\x95\xa0\x4c\x8a\x6f\x34\x16\x47\xc0\x25"
      "\xb1\x5f\xa7\xc5\x34\x51\x4f\x09\xce\x5a\x8f\x5a\x90\xdf\x00\xc5\xde\x9a"
      "\x25\x8f\x12\x7e\xf7\xab\x1b\x61\xa0\x34\x5a\xe8\x32\x0f\xf3\x08\x61\x9d"
      "\xe9\x17\x54\x57\x37\x2a\x67\x5d\x73\x11\x05\xf6\x07\x61\x39\x13\x2e\x94"
      "\x22\x24\xab\x01\xac\x42\x72\xd0\x97\xa4\x84\x42\xb9\x18\xb2\x31\x21\x35"
      "\x9c\xd9\xae\xdc\x3f\x31\xb0\x53\x3d\xab\x28\x71\x95\x36\xbc\x5e\x63\x7a"
      "\x49\xe9\x98\xe5\x24\xa9\xb6\xcb\xd1\x9b\x88\xc4\x5b\x11\x92\x13\x27\x09"
      "\xb9\xb3\xe4\x16\x38\xd3\xf6\x38\xee\xe7\x3e\x69\x1d\xcd\x8a\x12\x1a\x1c"
      "\x12\x69\x53\xb8\x4c\x52\x11\x28\x34\xd0\x54\x6f\xcf\x8b\x82\x71\x98\xde"
      "\x6e\x51\x26\x56\x6a\xc4\xc6\xfd\x85\x07\x16\xae\x51\x2c\xc7\xe0\xc6\x1b"
      "\x31\x30\x71\x6b\xa7\x77\x06\x01\xdb\x9b\xb4\x95\x8f\xfb\x9f\xd4\x49\xe8"
      "\x92\x41\x07\x6b\x78\xd4\xf6\x57\xa0\xfd\x84\xda\x0d\x29\x1e\x38\xf3\x54"
      "\xba\x8d\x28\xc7\x3e\x09\x2b\xa1\x44\x4d\xc3\x08\xcd\x35\x91\x86\x51\xc9"
      "\x02\x78\x59\x9b\xdb\x96\x7c\x94\xe7\xd3\x3a\x7c\x15\xf7\x52\xaa\x6d\x58"
      "\x47\x6f\x4b\x17\x06\x8c\x38\x72\xf7\x11\x18\x38\xb0\x6a\x7c\x6c\x48\x4c"
      "\xb4\xfe\xbc\x44\x1b\x63\xf3\xe8\xae\x81\x9c\x6c\x63\xd0\x10\x23\xea\xae"
      "\xc8\xc2\xb6\xe4\x8c\x08\x89\x98\xdd\x06\x28\x8b\xe1\x1d\x6e\x84\x1d\x37"
      "\x61\x64\xf5\xc4\x36\x2c\x5f\x83\xa8\xb9\x8c\x32\xfe\x3b\x4b\x5c\xbb\x71"
      "\x2f\xd0\xa2\x5e\x9f\xaa\xc7\x44\x80\xfc\x6e\xb7\x30\xff\x75\xf0\xe5\xe2"
      "\x67\x64\xac\x5a\x7d\xec\x05\xb1\x90\x4c\x47\x88\x6a\x79\xf6\x8a\x6d\xc4"
      "\x34\x3e\x31\xbe\x53\x4e\xe9\x5f\x60\xe4\x00\xcd\xf6\x40\x94\xc9\x7c\xc6"
      "\xb2\x02\x96\xc3\xa4\x3a\x3f\x07\x6b\x9a\xc5\xf1\x11\x88\x70\xda\xe8\xeb"
      "\xd3\xcc\xc1\x5b\x46\x26\x0b\xa2\xb0\x16\x0f\x0f\xf9\x4d\x8f\x7c\xae\x31"
      "\xc5\x23\xe6\x35\xa1\xe3\x15\x26\xd9\xa9\x70\xe8\x38\xcb\xed\x0a\xa5\x1b"
      "\x49\xdc\xc7\x4a\x3f\x19\xff\x8a\xff\xb3\xaa\x13\xe8\x3e\x22\x2c\x1f\xdf"
      "\xe5\xba\x40\xe6\x68\xbf\xe6\x9c\x27\xc3\x12\x0e\xba\x12\xab\x26\x42\x87"
      "\x2a\x51\xca\x99\xa7\x75\x33\x61\xe7\x99\x06\x23\x36\xf9\xe4\x73\x4e\x4b"
      "\xc6\x02\x1a\xd8\xb3\xed\x68\x6c\xeb\x3b\x83\xd6\x11\x0a\x1c\x9d\x0b\xc2"
      "\xf1\xb8\xd5\x2e\x4d\xec\xf8\x67\xe8\xfe\xce\xe4\x36\xc1\xcb\x48\x8e\x9e"
      "\x97\x3a\xe6\x3e\xc9\xbf\xcb\x57\x04\xea\x77\xba\x88\x17\x3d\x61\x52\x28"
      "\x7e\x4b\xc8\x59\x46\x0b\xb9\x92\x5a\xea\xfb\xf5\x2d\xc4\x95\x6f\xa2\x24"
      "\x15\x68\x81\xf9\x29\x9c\x78\x6b\x4b\xfe\xaf\x8d\xc3\x41\xff\x9c\x77\xa7"
      "\xe1\x12\xca\xe7\xcc\xf0\x01\x9d\x4d\x31\xf8\xd7\x5b\x6d\x67\xf1\x7d\x5e"
      "\xa7\x66\x04\x95\x28\x2c\x84\xb3\xfe\x22\x4a\x15\x9d\xba\x42\x9f\x33\x8b"
      "\x37\xa5\x07\x18\xc4\x54\x20\xb1\x3d\x55\xba\x5e\xa6\xd6\xe3\xfb\x59\xf7"
      "\x50\x4c\x5e\x5a\xce\x43\x3d\x4a\xf9\x04\xec\x42\x0e\x8c\x46\x1d\xeb\x47"
      "\x0b\xa1\x28\x53\x36\xf1\xac\xd7\x4a\x32\x0f\xcc\x37\xae\x08\x66\xdf\xa7"
      "\x04\x4f\x49\x46\xde\xc7\x51\x4e\x87\x44\xb6\xdf\xaa\x32\x34\xa4\x4e\x3c"
      "\xbe\x20\x01\x5c\xc8\x6a\x78\x5b\xfb\xc7\x58\xfa\xdf\x0d\xc1\xe6\xa7\x2c"
      "\x3c\x55\x6f\x86\x87\xf0\xbc\xd4\x50\xbc\xe9\x63\x2d\xf4\xfc\x81\x24\x46"
      "\xd5\xd6\xaa\xb6\x55\xc9\xaa\xc3\xd0\x78\xb0\x4e\xcd\xcb\xc2\x9f\x28\xe3"
      "\xf9\x39\xbf\x19\x38\x08\x02\xc4\x43\xbe\x74\x01\xb6\x19\x8c\x89\xb2\x31"
      "\xc5\x5f\x74\xfa\xd3\x6f\x47\x83\x79\xea\xdd\x84\x1a\xf2\x27\x37\x60\x87"
      "\x61\xd1\xc0\x77\xbf\xec\xdd\x9e\x19\xbf\x47\xae\x19\xff\x14\xa8\x2d\x72"
      "\xc0\x94\x9f\x73\xd5\x9a\xd4\x59\xba\x6b\x2c\x4d\xd2\x09\xfb\x90\x83\xb5"
      "\x35\x49\x7f\x1f\x49\x4d\x33\x53\x35\xd7\xca\x4d\x3f\xb6\x82\x56\x8c\xd4"
      "\x79\xcd\xc5\x5a\xb5\x44\x02\xf7\xd1\x0a\xf1\x9d\xc7\xb1\x05\x1d\xdc\xda"
      "\x32\xf4\x3c\xc9\xe6\x04\x64\xe2\xe8\x54\x47\xba\x89\x3f\x47\xa6\x84\x6e"
      "\x7d\xb5\xd1\x9e\x78\x1c\xce\xd9\x96\x00\x1f\x46\x6f\xff\x14\x59\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x33\x29\x7a\x8f\xb0\x10\x71"
      "\x11\xc3\xe1\xec\xf7\xc8\x6e\xc6\x31\x39",
      4096);
  *(uint16_t*)0x20000200 = 0x1f;
  *(uint16_t*)0x20000202 = 0;
  *(uint8_t*)0x20000204 = 0;
  *(uint8_t*)0x20000205 = 0;
  *(uint8_t*)0x20000206 = 0;
  *(uint8_t*)0x20000207 = 0;
  *(uint8_t*)0x20000208 = 0;
  *(uint8_t*)0x20000209 = 0;
  *(uint16_t*)0x2000020a = 0;
  *(uint8_t*)0x2000020c = 0;
  syscall(__NR_sendto, r[0], 0x20004140, 0x1000, 0, 0x20000200, 0x80);
  *(uint16_t*)0x20000180 = 0;
  *(uint16_t*)0x20000182 = 0;
  *(uint32_t*)0x20000184 = 0;
  *(uint32_t*)0x20000188 = 0;
  syscall(__NR_connect, r[0], 0x20000180, 0x80);
  *(uint64_t*)0x20000000 = 0x201f0000;
  *(uint32_t*)0x20000008 = 0x3000;
  *(uint32_t*)0x2000000c = 0;
  *(uint32_t*)0x20000040 = 0x10;
  syscall(__NR_getsockopt, r[0], 6, 0x23, 0x20000000, 0x20000040);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
