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
  *(uint64_t*)0x20001900 = 0x20000040;
  *(uint16_t*)0x20000040 = 0x18;
  *(uint32_t*)0x20000042 = 1;
  *(uint32_t*)0x20000046 = 0;
  *(uint32_t*)0x2000004a = r[0];
  *(uint16_t*)0x2000004e = 1;
  *(uint16_t*)0x20000050 = 1;
  *(uint16_t*)0x20000052 = 2;
  *(uint16_t*)0x20000054 = 3;
  *(uint16_t*)0x20000056 = 0xa;
  *(uint16_t*)0x20000058 = htobe16(0x4e22);
  *(uint32_t*)0x2000005a = 5;
  *(uint8_t*)0x2000005e = 0xfe;
  *(uint8_t*)0x2000005f = 0x80;
  *(uint8_t*)0x20000060 = 0;
  *(uint8_t*)0x20000061 = 0;
  *(uint8_t*)0x20000062 = 0;
  *(uint8_t*)0x20000063 = 0;
  *(uint8_t*)0x20000064 = 0;
  *(uint8_t*)0x20000065 = 0;
  *(uint8_t*)0x20000066 = 0;
  *(uint8_t*)0x20000067 = 0;
  *(uint8_t*)0x20000068 = 0;
  *(uint8_t*)0x20000069 = 0;
  *(uint8_t*)0x2000006a = 0;
  *(uint8_t*)0x2000006b = 0;
  *(uint8_t*)0x2000006c = 0;
  *(uint8_t*)0x2000006d = 0xbb;
  *(uint32_t*)0x2000006e = 4;
  *(uint32_t*)0x20001908 = 0x80;
  *(uint64_t*)0x20001910 = 0x20001600;
  *(uint64_t*)0x20001600 = 0x200000c0;
  *(uint64_t*)0x20001608 = 0;
  *(uint64_t*)0x20001610 = 0x20000100;
  *(uint64_t*)0x20001618 = 0;
  *(uint64_t*)0x20001620 = 0x20000140;
  *(uint64_t*)0x20001628 = 0;
  *(uint64_t*)0x20001630 = 0x20000280;
  *(uint64_t*)0x20001638 = 0;
  *(uint64_t*)0x20001640 = 0x20000380;
  *(uint64_t*)0x20001648 = 0;
  *(uint64_t*)0x20001650 = 0x200003c0;
  *(uint64_t*)0x20001658 = 0;
  *(uint64_t*)0x20001660 = 0x20000480;
  *(uint64_t*)0x20001668 = 0;
  *(uint64_t*)0x20001670 = 0x20001480;
  *(uint64_t*)0x20001678 = 0;
  *(uint64_t*)0x20001680 = 0x20001540;
  *(uint64_t*)0x20001688 = 0;
  *(uint64_t*)0x20001690 = 0x20001580;
  *(uint64_t*)0x20001698 = 0;
  *(uint64_t*)0x20001918 = 0xa;
  *(uint64_t*)0x20001920 = 0x200016c0;
  *(uint64_t*)0x200016c0 = 0x10;
  *(uint32_t*)0x200016c8 = 0x3a;
  *(uint32_t*)0x200016cc = 6;
  *(uint64_t*)0x200016d0 = 0x10;
  *(uint32_t*)0x200016d8 = 0;
  *(uint32_t*)0x200016dc = 0x10001;
  *(uint64_t*)0x200016e0 = 0x10;
  *(uint32_t*)0x200016e8 = 6;
  *(uint32_t*)0x200016ec = 0x400;
  *(uint64_t*)0x20001928 = 0x30;
  *(uint32_t*)0x20001930 = 0x20000000;
  syscall(__NR_sendmsg, r[0], 0x20001900, 0x20000040);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
