// https://syzkaller.appspot.com/bug?id=004b0f7b61d4901cbfecfc33de7996e8cbe0a278
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
  long res = 0;
  res = syscall(__NR_socket, 0x2b, 1, 0);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20000080 = 0x18;
  *(uint32_t*)0x20000082 = 1;
  *(uint32_t*)0x20000086 = 0;
  *(uint32_t*)0x2000008a = r[0];
  *(uint16_t*)0x2000008e = 2;
  *(uint16_t*)0x20000090 = 2;
  *(uint16_t*)0x20000092 = 0;
  *(uint16_t*)0x20000094 = 2;
  *(uint16_t*)0x20000096 = 0xa;
  *(uint16_t*)0x20000098 = htobe16(0x4e22);
  *(uint32_t*)0x2000009a = 0;
  *(uint64_t*)0x2000009e = htobe64(0);
  *(uint64_t*)0x200000a6 = htobe64(1);
  *(uint32_t*)0x200000ae = 1;
  syscall(__NR_sendto, r[0], 0x20000000, 0, 0x20000001, 0x20000080, 0x80);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
