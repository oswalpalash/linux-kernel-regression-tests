// https://syzkaller.appspot.com/bug?id=bea1c185923957c5339d5b43bca13e71047453da
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

static void execute_one();
extern unsigned long long procid;

void loop()
{
  while (1) {
    execute_one();
  }
}

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};
void execute_one()
{
  long res = 0;
  res = syscall(__NR_socket, 0x26, 5, 0);
  if (res != -1)
    r[0] = res;
  syscall(__NR_close, r[0]);
  res = syscall(__NR_socket, 0x26, 5, 0);
  if (res != -1)
    r[1] = res;
  *(uint16_t*)0x20001140 = 0x26;
  memcpy((void*)0x20001142,
         "\x68\x61\x73\x68\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 14);
  *(uint32_t*)0x20001150 = 0;
  *(uint32_t*)0x20001154 = 0;
  memcpy((void*)0x20001158,
         "\x73\x68\x61\x33\x2d\x35\x31\x32\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         64);
  syscall(__NR_bind, r[1], 0x20001140, 0x58);
  res = syscall(__NR_accept, r[0], 0, 0);
  if (res != -1)
    r[2] = res;
  *(uint64_t*)0x20000080 = 0x20001f40;
  memcpy((void*)0x20001f40,
         "\xc1\x75\xbb\x8e\xa7\x12\xbd\x9a\xeb\xa9\x61\xdf\x2e\x48\xdb\xa0\xd1"
         "\xd8\xe0\xdb\xeb\xa8\x6a\xe5\x0f\x6c\x08\x6b\x5a\xce\xb0\x61\xf0\x7a"
         "\x41\xf9\x0e\x29\x03\x39\x8a\xd8\x03\xec\x11\xb6\x8b\xec\x8a\xb8\x71"
         "\x32\xb6\xb5\xb5\x10\x8a\x5b\x63\x1e\x81\xe6\x34\x28\x00\xf2\x18\x10"
         "\xf1\x5b\xff\x9d\xcb\x6e\x89\x2c\x50\x28\x5f\x52\x25\x04\xd9\x80\x1b"
         "\x22\xf2\x52\x54\x80\x66\x4a\xca\x25\xfa\xe5\x39\xde\x24\x04\x12\x10"
         "\xaa\x3e\xb8\xb5\x4b\xe7\xab\xb1\x88\xdb\xf5\x56\x45\xa7\xdc\xab\xc0"
         "\xb7\xbb\xdb\xc3\xde\x87\xf3\x0f\x30\x07\x3d\x22\xde\xcc\x4d\xcd\x22"
         "\xdd\x7b\xd7\x98\x0c\x6c\xf0\xe1\x00\x4c\xcf\xce\x3e\x4b\xfd\x58\x3e"
         "\x17\xa1\xdf\x75\x81\x64\x6c\x91\x1b\x4c\x34\xfb\xe4\xf2\x12\xf3\xa6"
         "\x44\x62\xaa\xbd\x0e\x22\xda\xa0\x3d\x98\x1a\x19\xa4\xde\xf2\x15\x6f"
         "\xed\x24\xdd\x08\x19\x18\x05\x9f\xe4\xef\x70\xa2\xb6\xab\x2b\xac\x33"
         "\xc3\x3d\x19\x3a\x5a\x57\x69\xf1\x85\xfe\xbe\x64",
         216);
  *(uint64_t*)0x20000088 = 0xd8;
  syscall(__NR_writev, r[2], 0x20000080, 1);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    loop();
  }
}
