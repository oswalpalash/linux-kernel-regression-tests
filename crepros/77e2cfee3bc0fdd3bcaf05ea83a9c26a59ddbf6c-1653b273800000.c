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
  memcpy((void*)0x20000280, "/dev/infiniband/rdma_cm", 24);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000280, 2, 0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000580,
         "\x00\x00\x00\x00\x18\x00\x00\xfa\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  *(uint64_t*)0x20000590 = -1;
  memcpy((void*)0x20000598, "\x3f\x01\x00\x67\x27\x00\x80\x00", 8);
  syscall(__NR_write, r[0], 0x20000580, 0x20);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
