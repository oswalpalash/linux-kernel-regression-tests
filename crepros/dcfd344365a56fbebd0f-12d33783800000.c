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
  memcpy((void*)0x20000140, "/dev/infiniband/rdma_cm", 24);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000140, 2, 0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000240, "\x00\x00\x00\x00\x18\x00\x00\x45\xe6\x17\x5e\x5f"
                            "\x2d\x03\x43\x81\x50\xf9\x0e\x78\x79\x49\xb0\xfa"
                            "\x02\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint64_t*)0x20000260 = 0x200032c0;
  *(uint32_t*)0x200032c0 = -1;
  syscall(__NR_write, r[0], 0x20000240, 0x28);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
