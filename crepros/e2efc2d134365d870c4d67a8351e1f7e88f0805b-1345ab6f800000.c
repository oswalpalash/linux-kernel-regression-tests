// https://syzkaller.appspot.com/bug?id=e2efc2d134365d870c4d67a8351e1f7e88f0805b
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
  *(uint8_t*)0x20000040 = 0x73;
  *(uint8_t*)0x20000041 = 0x79;
  *(uint8_t*)0x20000042 = 0x7a;
  *(uint8_t*)0x20000043 = 0;
  *(uint8_t*)0x20000044 = 0;
  res = syscall(__NR_add_key, 0x20000000, 0x20000040, 0, 0, 0xfffffffd);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x200000c0, "user", 5);
  *(uint8_t*)0x20000140 = 0x73;
  *(uint8_t*)0x20000141 = 0x79;
  *(uint8_t*)0x20000142 = 0x7a;
  *(uint8_t*)0x20000143 = 0x22;
  *(uint8_t*)0x20000144 = 0;
  memcpy((void*)0x20000300, "\x50\xb2\x28\x97\x0d\x73\xba\xd7\x67\x96\x8f\x40"
                            "\x65\xda\xe3\x04\xf8\xad\x3e\xab\x7c\x56\x9a\x77"
                            "\x79\x3b\x8e",
         27);
  res = syscall(__NR_add_key, 0x200000c0, 0x20000140, 0x20000300, 0x1b, r[0]);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x20000440, "user", 5);
  *(uint8_t*)0x20000480 = 0x73;
  *(uint8_t*)0x20000481 = 0x79;
  *(uint8_t*)0x20000482 = 0x7a;
  *(uint8_t*)0x20000483 = 0x21;
  *(uint8_t*)0x20000484 = 0;
  memcpy((void*)0x20000540,
         "\xbf\x3e\xf2\x65\x19\x19\x3f\x1e\xb4\x60\xbd\x13\x99\xce\x87\x7b\xe8"
         "\xb6\xd0\x84\x74\x85\x62\x8d\xf4\x1d\x77\x3b\x94\xf4\x5d\x6a\x2f\x8c"
         "\xaa\x55\xed\x13\x74\x5c\x4d\xdb\x42\x1b\xe1\xda\x7f\x3c\xfa\x95\x89"
         "\x73\x52\x42\x18\x43\x86\x5c\xbe\x2d\x79\xf9\xa3\x41\x2e\xe8\x48\x0d"
         "\xbe\xcc\x26\x4c\x49\x80\xeb\x31\x26\xdf\xe9\xfc\xb1\xe5\x28\x33\x50"
         "\xd0\x5d\x94\xb6\x31\x0a\xe1\xf6\x6a\xd7\xb1\x7d\x67\x94\xc2\x91\x61"
         "\xa6\xc0\x73\x4a\xc7\x98\x76\x28\x3b\x25\xb3\x64\x2d\x6e\x71\xad\xd9"
         "\xf3\x69\x89\xc1\x56\x9e\x1f\x1e\x00\x15\x08\xa2\x42\x96\xcb\xc1\xbd"
         "\xf1\x87\x9f\x53\xfc\xdc\x11\xfa\xe5\xf5\x5d\xeb\x28\x8a\x25\x3d\xbe"
         "\xd0\x32\x95\xa1\x3b\x76\xed\x18\x0a\x46\x39\x28\x06\x0b\xae\xd7\x8b"
         "\xe6\x94\x2a\xad\x19\xd7\x17\xe7\xa9\x6c\x6c\x61\x54\x3b\x2e\xd4\xf2"
         "\x4f\xc4\x32\xa8\x8f\x78\x2a\x1e\xca\x55\x2a\x62\xcd\x1d\xf5\x7c\x38"
         "\x43\x55\x70\xed\x59\xb8\x69\x0f\xd8\x64\x40\xf2\x3b\x65\x14\x69\x47"
         "\xa0\x01\x21\xfd\xf3\xbf\xfc\x6d\xc7\xd2\xf3\x66\xa6\x84\xb4\xc7\xce"
         "\x8b\x5f\xdd\xf4\x1b\xde\x91\x19\xfe\xea\xa0\x0f\xe2",
         251);
  res = syscall(__NR_add_key, 0x20000440, 0x20000480, 0x20000540, 0xfb, r[0]);
  if (res != -1)
    r[2] = res;
  memcpy((void*)0x200004c0, "user", 5);
  *(uint8_t*)0x20000640 = 0x73;
  *(uint8_t*)0x20000641 = 0x79;
  *(uint8_t*)0x20000642 = 0x7a;
  *(uint8_t*)0x20000643 = 0x22;
  *(uint8_t*)0x20000644 = 0;
  memcpy((void*)0x20000680,
         "\x22\x4c\x78\x0b\x95\xc4\xd0\xab\x8b\x49\x9d\x68\x1a\xb4\x2e\x37\x84"
         "\xfa\xc7\xcb\x6f\xfb\x73\x5d\x4c\x76\x69\x1c\xe6\xb9\x4c\x73\x47\x7d"
         "\x2b\xee\x76\xd2\xbf\x30\x0b\xd9\xac\x26\x36\x0b\x42\x0b\x6c\xc4\x43"
         "\x09\x19\xe3\xf8\xcd\x37\x72\x80\xe9\x92\x0d\xcd\x7f\xd3\x46\xed\x40"
         "\x99\x41\xfd\x42\x0e\x79\xc0\x52\x81\x64\x7d\xa4\x33\xa0\xf7\xa1\x19"
         "\x73\xa1\x7d\xd4\xef\xfb\x7d\x49\x3e\x4e\xee\x2f\x87\x5b\xd0\xc1\xb7"
         "\x9f\x59\x79\x3c\xc4\x90\x66\x4b\x89\xe1\xa5\x1f\xe4\x60\xea\x34\x21"
         "\x2b\xba\xd7\x82\x0f\x25\x44\x97\x71\xca\x12\x53\xb5\x80\x74\x29\x42"
         "\x24\x93\x10\x88\x45\x0d\x5e\x3c\x40\xb7\xab\xdf\x3a\x21\x23\x7f\x80"
         "\xe4\xaf\x6b\x63\x12\xfa\x2f\x7e\x10\xc3\xf0\x15\x9f\xdf\xdc\x72\x18"
         "\x7c",
         171);
  res = syscall(__NR_add_key, 0x200004c0, 0x20000640, 0x20000680, 0xab, r[0]);
  if (res != -1)
    r[3] = res;
  *(uint32_t*)0x20000740 = r[1];
  *(uint32_t*)0x20000744 = r[2];
  *(uint32_t*)0x20000748 = r[3];
  *(uint64_t*)0x20000880 = 0x200007c0;
  memcpy((void*)0x200007c0,
         "\x72\x6d\x64\x33\x32\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         64);
  *(uint64_t*)0x20000888 = 0x20000840;
  *(uint32_t*)0x20000890 = 0;
  *(uint32_t*)0x20000894 = 0;
  *(uint32_t*)0x20000898 = 0;
  *(uint32_t*)0x2000089c = 0;
  *(uint32_t*)0x200008a0 = 0;
  *(uint32_t*)0x200008a4 = 0;
  *(uint32_t*)0x200008a8 = 0;
  *(uint32_t*)0x200008ac = 0;
  *(uint32_t*)0x200008b0 = 0;
  syscall(__NR_keyctl, 0x17, 0x20000740, 0x20000780, 0x2d, 0x20000880);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
