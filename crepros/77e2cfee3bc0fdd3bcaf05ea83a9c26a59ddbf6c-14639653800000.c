// https://syzkaller.appspot.com/bug?id=77e2cfee3bc0fdd3bcaf05ea83a9c26a59ddbf6c
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

uint64_t r[1] = {0xffffffffffffffff};
void loop()
{
  long res;
  memcpy((void*)0x20000040, "/dev/infiniband/rdma_cm", 24);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000040, 2, 0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000640, "\x00\x00\x00\x00\x18\x00\x00\xfa\x00\x00\x00\x00"
                            "\x00\x00\xe3\xc1\x48\x11\xc2\x02\x56\x00\x00",
         23);
  *(uint64_t*)0x20000657 = 0x200002c0;
  *(uint32_t*)0x200002c0 = -1;
  memcpy((void*)0x2000065f,
         "\xe1\x69\x74\x5a\x5c\x34\x42\xe9\xe5\x17\xa7\x77\x9b\x19\xa2\x51\x9e"
         "\xbd\x79\x5f\x1c\xdd\x4e\xf4\xa7\xd2\x70\x5f\xa8\xe5\xd9\x2a\x86\x8d"
         "\x1e\x5c\x11\x3e\x05\xc9\x4f\xe3\xa7\xdb\xf2\x17\x0e\xce\x60\xc0\x02"
         "\xe6\x07\x44\x4d\xdb\x65\x7e\xfc\x7d\x27\xd1\x2d\x5f\xce\x8b\x87\x9a"
         "\x4b\xdc\xc0\xbc\x75\x5d\x85\x99\xd0\x14\x66\x68\xfd\xab\x39\x61\xc0"
         "\x70\xd4\x3b\xa2\x29\xd7\x1c\xad\xf3\x32\x83\x49\x4f\x47\x27\x2a\xac"
         "\x29\x4c\x89\xdf\x37\x59\x63\x15\x33\x1a\x6e\xe1\x8b\xd3\xc8\x46\x2a"
         "\xc1\x26\xa5\x3f\xb6\x76\x75\xd7\x5c\x04\xd9\x0a\x27\xfc\x49\xde\x03"
         "\x93\xfe\x52\x96\x1b\xb5\xfb\xc2\xcd\xf6\x95\x80\xd0\x8d\xb6\x43\x69"
         "\xa0\xbf\x21\xdc\x3f\xa4\x77\xd2\xd8\x7d\x04\x03\x5f\xf9\x0c\x1e",
         169);
  syscall(__NR_write, r[0], 0x20000640, 0xc8);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
