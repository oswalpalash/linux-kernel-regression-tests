// https://syzkaller.appspot.com/bug?id=1665547180092fef185a46d5a4ba5b382d77848e
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <sys/syscall.h>
#include <unistd.h>

#include <stdint.h>
#include <string.h>

#ifndef __NR_mmap
#define __NR_mmap 90
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#ifndef __NR_sendto
#define __NR_sendto 369
#endif
#ifndef __NR_listen
#define __NR_listen 363
#endif
#ifndef __NR_accept4
#define __NR_accept4 364
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[27];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  r[1] = syscall(__NR_socket, 0xaul, 0x801ul, 0x84ul);
  memcpy((void*)0x2026dfff, "\x78", 1);
  *(uint16_t*)0x20a8cfe4 = (uint16_t)0xa;
  *(uint16_t*)0x20a8cfe6 = (uint16_t)0x204e;
  *(uint32_t*)0x20a8cfe8 = (uint32_t)0xfffffffffffffffd;
  *(uint8_t*)0x20a8cfec = (uint8_t)0xfe;
  *(uint8_t*)0x20a8cfed = (uint8_t)0x80;
  *(uint8_t*)0x20a8cfee = (uint8_t)0x0;
  *(uint8_t*)0x20a8cfef = (uint8_t)0x0;
  *(uint8_t*)0x20a8cff0 = (uint8_t)0x0;
  *(uint8_t*)0x20a8cff1 = (uint8_t)0x0;
  *(uint8_t*)0x20a8cff2 = (uint8_t)0x0;
  *(uint8_t*)0x20a8cff3 = (uint8_t)0x0;
  *(uint8_t*)0x20a8cff4 = (uint8_t)0x0;
  *(uint8_t*)0x20a8cff5 = (uint8_t)0x0;
  *(uint8_t*)0x20a8cff6 = (uint8_t)0x0;
  *(uint8_t*)0x20a8cff7 = (uint8_t)0x0;
  *(uint8_t*)0x20a8cff8 = (uint8_t)0x0;
  *(uint8_t*)0x20a8cff9 = (uint8_t)0x0;
  *(uint8_t*)0x20a8cffa = (uint8_t)0x0;
  *(uint8_t*)0x20a8cffb = (uint8_t)0xbb;
  *(uint32_t*)0x20a8cffc = (uint32_t)0x800000001;
  r[23] = syscall(__NR_sendto, r[1], 0x2026dffful, 0x1ul, 0x8800ul,
                  0x20a8cfe4ul, 0x1cul);
  r[24] = syscall(__NR_listen, r[1], 0x1ful);
  *(uint32_t*)0x20281000 = (uint32_t)0x10;
  r[26] =
      syscall(__NR_accept4, r[1], 0x20aa5ff0ul, 0x20281000ul, 0x0ul);
}

int main()
{
  loop();
  return 0;
}
