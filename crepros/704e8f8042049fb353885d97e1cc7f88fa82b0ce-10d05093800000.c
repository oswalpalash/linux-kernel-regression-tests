// https://syzkaller.appspot.com/bug?id=704e8f8042049fb353885d97e1cc7f88fa82b0ce
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
  long res;
  memcpy((void*)0x20000280, "/dev/infiniband/rdma_cm", 24);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000280, 2, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000580 = 0;
  *(uint16_t*)0x20000584 = 0x18;
  *(uint16_t*)0x20000586 = 0xfa00;
  *(uint64_t*)0x20000588 = 0;
  *(uint64_t*)0x20000590 = 0x20000540;
  *(uint16_t*)0x20000598 = 0x13f;
  *(uint8_t*)0x2000059a = 0;
  *(uint8_t*)0x2000059b = 0;
  *(uint8_t*)0x2000059c = 0;
  *(uint8_t*)0x2000059d = 0;
  *(uint8_t*)0x2000059e = 0;
  *(uint8_t*)0x2000059f = 0;
  res = syscall(__NR_write, r[0], 0x20000580, 0x20);
  if (res != -1)
    r[1] = *(uint32_t*)0x20000540;
  memcpy((void*)0x20000040, "\x0e\x00\x00\x00\x18\x00\x00\xfa", 8);
  *(uint64_t*)0x20000048 = 0x20000080;
  *(uint32_t*)0x20000050 = r[1];
  memcpy((void*)0x20000054, "\x00\xd4\x57\x95\xb6\xed\x18\x85\xc2\xf8\x3b\xb0"
                            "\x03\x00\x04\x00\x00\x00",
         18);
  syscall(__NR_write, r[0], 0x20000040, 0x26);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
