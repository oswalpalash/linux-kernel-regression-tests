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
  syscall(__NR_mmap, 0x20001000, 0x4000, 0, 0x2c871, -1, 0);
  memcpy((void*)0x20000180, "/dev/infiniband/rdma_cm", 24);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000180, 2, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000040 = 0;
  *(uint16_t*)0x20000044 = 0x18;
  *(uint16_t*)0x20000046 = 0xfa00;
  *(uint64_t*)0x20000048 = 3;
  *(uint64_t*)0x20000050 = 0x20002f00;
  *(uint16_t*)0x20000058 = 0x13f;
  *(uint8_t*)0x2000005a = 5;
  *(uint8_t*)0x2000005b = 0;
  *(uint8_t*)0x2000005c = 0;
  *(uint8_t*)0x2000005d = 0;
  *(uint8_t*)0x2000005e = 0;
  *(uint8_t*)0x2000005f = 0;
  syscall(__NR_write, r[0], 0x20000040, 0x20);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
