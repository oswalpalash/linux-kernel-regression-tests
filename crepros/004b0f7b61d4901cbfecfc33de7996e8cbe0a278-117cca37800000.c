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
  *(uint64_t*)0x20000580 = 0x20000080;
  *(uint16_t*)0x20000080 = 0x18;
  *(uint32_t*)0x20000082 = 1;
  *(uint32_t*)0x20000086 = 0;
  *(uint32_t*)0x2000008a = r[0];
  *(uint16_t*)0x2000008e = 4;
  *(uint16_t*)0x20000090 = 4;
  *(uint16_t*)0x20000092 = 1;
  *(uint16_t*)0x20000094 = 1;
  *(uint16_t*)0x20000096 = 0xa;
  *(uint16_t*)0x20000098 = htobe16(0x4e23);
  *(uint32_t*)0x2000009a = 1;
  *(uint8_t*)0x2000009e = 0xfe;
  *(uint8_t*)0x2000009f = 0x80;
  *(uint8_t*)0x200000a0 = 0;
  *(uint8_t*)0x200000a1 = 0;
  *(uint8_t*)0x200000a2 = 0;
  *(uint8_t*)0x200000a3 = 0;
  *(uint8_t*)0x200000a4 = 0;
  *(uint8_t*)0x200000a5 = 0;
  *(uint8_t*)0x200000a6 = 0;
  *(uint8_t*)0x200000a7 = 0;
  *(uint8_t*)0x200000a8 = 0;
  *(uint8_t*)0x200000a9 = 0;
  *(uint8_t*)0x200000aa = 0;
  *(uint8_t*)0x200000ab = 0;
  *(uint8_t*)0x200000ac = 0;
  *(uint8_t*)0x200000ad = 0xaa;
  *(uint32_t*)0x200000ae = 6;
  *(uint32_t*)0x20000588 = 0x80;
  *(uint64_t*)0x20000590 = 0x20000380;
  *(uint64_t*)0x20000380 = 0x20000100;
  *(uint64_t*)0x20000388 = 0;
  *(uint64_t*)0x20000390 = 0x20000140;
  *(uint64_t*)0x20000398 = 0;
  *(uint64_t*)0x200003a0 = 0x20000200;
  *(uint64_t*)0x200003a8 = 0;
  *(uint64_t*)0x200003b0 = 0x20000280;
  *(uint64_t*)0x200003b8 = 0;
  *(uint64_t*)0x20000598 = 4;
  *(uint64_t*)0x200005a0 = 0x200003c0;
  *(uint64_t*)0x200003c0 = 0x10;
  *(uint32_t*)0x200003c8 = 0x109;
  *(uint32_t*)0x200003cc = 0xe4c3;
  *(uint64_t*)0x200003d0 = 0x10;
  *(uint32_t*)0x200003d8 = 0x116;
  *(uint32_t*)0x200003dc = 7;
  *(uint64_t*)0x200003e0 = 0x10;
  *(uint32_t*)0x200003e8 = 0x103;
  *(uint32_t*)0x200003ec = 6;
  *(uint64_t*)0x200005a8 = 0x30;
  *(uint32_t*)0x200005b0 = 0x4001;
  syscall(__NR_sendmsg, r[0], 0x20000580, 0x20040000);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
