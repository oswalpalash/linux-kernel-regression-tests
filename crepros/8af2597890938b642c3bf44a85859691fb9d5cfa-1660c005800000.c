// https://syzkaller.appspot.com/bug?id=8af2597890938b642c3bf44a85859691fb9d5cfa
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

static void test();

void loop()
{
  while (1) {
    test();
  }
}

#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#ifndef __NR_connect
#define __NR_connect 362
#endif
#ifndef __NR_sendmsg
#define __NR_sendmsg 370
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[2];
void test()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xe75000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 0xa, 2, 0);
  *(uint16_t*)0x20d4c000 = 0xa;
  *(uint16_t*)0x20d4c002 = htobe16(0x4e21);
  *(uint32_t*)0x20d4c004 = 7;
  *(uint64_t*)0x20d4c008 = htobe64(0);
  *(uint64_t*)0x20d4c010 = htobe64(1);
  *(uint32_t*)0x20d4c018 = 0x1c28;
  syscall(__NR_connect, r[0], 0x20d4c000, 0x1c);
  *(uint16_t*)0x20e6f000 = 0xa;
  *(uint16_t*)0x20e6f002 = htobe16(0x4e22);
  *(uint32_t*)0x20e6f004 = 0;
  *(uint8_t*)0x20e6f008 = 0;
  *(uint8_t*)0x20e6f009 = 0;
  *(uint8_t*)0x20e6f00a = 0;
  *(uint8_t*)0x20e6f00b = 0;
  *(uint8_t*)0x20e6f00c = 0;
  *(uint8_t*)0x20e6f00d = 0;
  *(uint8_t*)0x20e6f00e = 0;
  *(uint8_t*)0x20e6f00f = 0;
  *(uint8_t*)0x20e6f010 = 0;
  *(uint8_t*)0x20e6f011 = 0;
  *(uint8_t*)0x20e6f012 = -1;
  *(uint8_t*)0x20e6f013 = -1;
  *(uint8_t*)0x20e6f014 = 0xac;
  *(uint8_t*)0x20e6f015 = 0x14;
  *(uint8_t*)0x20e6f016 = 0;
  *(uint8_t*)0x20e6f017 = 0xaa;
  *(uint32_t*)0x20e6f018 = 1;
  syscall(__NR_connect, r[0], 0x20e6f000, 0x1c);
  r[1] = syscall(__NR_socket, 0x18, 1, 1);
  *(uint16_t*)0x205fafd2 = 0x18;
  *(uint32_t*)0x205fafd4 = 1;
  *(uint32_t*)0x205fafd8 = 0;
  *(uint32_t*)0x205fafdc = r[0];
  *(uint16_t*)0x205fafe0 = 2;
  *(uint16_t*)0x205fafe2 = htobe16(0x4e21);
  *(uint32_t*)0x205fafe4 = htobe32(0xe0000002);
  *(uint8_t*)0x205fafe8 = 0;
  *(uint8_t*)0x205fafe9 = 0;
  *(uint8_t*)0x205fafea = 0;
  *(uint8_t*)0x205fafeb = 0;
  *(uint8_t*)0x205fafec = 0;
  *(uint8_t*)0x205fafed = 0;
  *(uint8_t*)0x205fafee = 0;
  *(uint8_t*)0x205fafef = 0;
  *(uint32_t*)0x205faff0 = 4;
  *(uint32_t*)0x205faff4 = 0;
  *(uint32_t*)0x205faff8 = 2;
  *(uint32_t*)0x205faffc = 0;
  syscall(__NR_connect, r[1], 0x205fafd2, 0x2e);
  *(uint32_t*)0x2037ffc8 = 0x209dd000;
  *(uint32_t*)0x2037ffcc = 0xc;
  *(uint32_t*)0x2037ffd0 = 0x202ceff0;
  *(uint32_t*)0x2037ffd4 = 1;
  *(uint32_t*)0x2037ffd8 = 0;
  *(uint32_t*)0x2037ffdc = 0;
  *(uint32_t*)0x2037ffe0 = 0x8820;
  *(uint16_t*)0x209dd000 = 0x10;
  *(uint16_t*)0x209dd002 = 0;
  *(uint32_t*)0x209dd004 = 0;
  *(uint32_t*)0x209dd008 = 2;
  *(uint32_t*)0x202ceff0 = 0x2097b000;
  *(uint32_t*)0x202ceff4 = 0xfff1;
  *(uint32_t*)0x2097b000 = 0x10;
  *(uint16_t*)0x2097b004 = 0x14;
  *(uint16_t*)0x2097b006 = 0x200;
  *(uint32_t*)0x2097b008 = 0x70bd26;
  *(uint32_t*)0x2097b00c = 0x25dfdbfe;
  syscall(__NR_sendmsg, r[1], 0x2037ffc8, 0x81);
}

int main()
{
  for (;;) {
    loop();
  }
}
