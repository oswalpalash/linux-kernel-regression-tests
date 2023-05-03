// https://syzkaller.appspot.com/bug?id=4fbc4bf3d1300cf7629a7f63686b5d9cf94ed429
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

uint64_t r[4] = {0x0, 0x0, 0x0, 0x0};
void loop()
{
  long res = 0;
  memcpy((void*)0x20000000, "keyring", 8);
  *(uint8_t*)0x20000080 = 0x73;
  *(uint8_t*)0x20000081 = 0x79;
  *(uint8_t*)0x20000082 = 0x7a;
  *(uint8_t*)0x20000083 = 0;
  *(uint8_t*)0x20000084 = 0;
  res = syscall(__NR_add_key, 0x20000000, 0x20000080, 0, 0, -1);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x200000c0, "keyring", 8);
  *(uint8_t*)0x20000180 = 0x73;
  *(uint8_t*)0x20000181 = 0x79;
  *(uint8_t*)0x20000182 = 0x7a;
  *(uint8_t*)0x20000183 = 0;
  *(uint8_t*)0x20000184 = 0;
  res = syscall(__NR_add_key, 0x200000c0, 0x20000180, 0, 0, r[0]);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x203bd000, "user", 5);
  *(uint8_t*)0x20000040 = 0x73;
  *(uint8_t*)0x20000041 = 0x79;
  *(uint8_t*)0x20000042 = 0x7a;
  *(uint8_t*)0x20000043 = 0;
  *(uint8_t*)0x20000044 = 0;
  memcpy((void*)0x20facfff, "\x06", 1);
  res = syscall(__NR_add_key, 0x203bd000, 0x20000040, 0x20facfff, 1, r[1]);
  if (res != -1)
    r[2] = res;
  memcpy((void*)0x20000500, "user", 5);
  *(uint8_t*)0x20752ffb = 0x73;
  *(uint8_t*)0x20752ffc = 0x79;
  *(uint8_t*)0x20752ffd = 0x7a;
  *(uint8_t*)0x20752ffe = 0x22;
  *(uint8_t*)0x20752fff = 0;
  memcpy((void*)0x20000580, "\xb3", 1);
  res = syscall(__NR_add_key, 0x20000500, 0x20752ffb, 0x20000580, 1, r[1]);
  if (res != -1)
    r[3] = res;
  memcpy((void*)0x20000340,
         "\xdf\x02\x75\x46\x77\x21\x2d\xfc\x3e\x2a\xbc\xb2\x44\xe2\x27\x73\xe8"
         "\xe1\xcf\xef\x09\x74\x22\x58\xcb\xae\x7b\x26\xfd\xc3\xff\xa6\x15\xd9"
         "\xda\x49\x41\x37\xe1\x75\xe9\xf2\x78\x0a\xc5\xe2\xa0\x9f\x43\xa1\xfc"
         "\xeb\xf2\x72\xa5\xa1\x35\xde\x92\xbf\x4a\x90\x33\x93\x38\x24\xf6\xe6"
         "\xaa\x02\x38\x01\x4e\x32\x93\x53\x5d\x1c\x00\x66\xd2\x8e\x0f\x27\x51"
         "\x88\xb4\xb4\xc1\x87\xe1\x87\x74\xfc\x22\x27\xcb\xb6\x61\x0f\xc6\x69"
         "\x7f\x53\x37\x72\x61\x64\xc8\xfb\xe1\x18\x1e\x6d\x50\x98\x6c\xd9\x8a"
         "\x5c\x44\xac\x0e\xc3\x75\xde\xb2\x7e\xaf\xcf\x7d\x06\x43\x8f\x25\x25"
         "\x10\x04\x00\x00\x00\x03\xe7\xdc\x2f\x8f\xd8\x5a\xdc\xd8\x8c\xa3\x0e"
         "\x75\xb1\xe1\x43\x26\x2d\x94\xde\xfa\x18\x7e\x48\x2a\x9b\x9a\x9a\x38"
         "\x73\x53\xdf\x7a\x21\xaf\xfc\xe4\xe9\x14\x9e\x04\x00\x00\x00\x00\x00"
         "\x00\x00\x65\xaf\xd9\x79",
         193);
  syscall(__NR_keyctl, 2, r[3], 0x20000340, 0xc1);
  *(uint32_t*)0x20000100 = r[2];
  *(uint32_t*)0x20000104 = r[3];
  *(uint32_t*)0x20000108 = r[2];
  *(uint64_t*)0x20c61fc8 = 0x20a3dffa;
  memcpy((void*)0x20a3dffa,
         "\x73\x68\x61\x31\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         64);
  *(uint64_t*)0x20c61fd0 = 0;
  *(uint32_t*)0x20c61fd8 = 0;
  *(uint32_t*)0x20c61fdc = 0;
  *(uint32_t*)0x20c61fe0 = 0;
  *(uint32_t*)0x20c61fe4 = 0;
  *(uint32_t*)0x20c61fe8 = 0;
  *(uint32_t*)0x20c61fec = 0;
  *(uint32_t*)0x20c61ff0 = 0;
  *(uint32_t*)0x20c61ff4 = 0;
  *(uint32_t*)0x20c61ff8 = 0;
  syscall(__NR_keyctl, 0x17, 0x20000100, 0x20a53ffb, 5, 0x20c61fc8);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
