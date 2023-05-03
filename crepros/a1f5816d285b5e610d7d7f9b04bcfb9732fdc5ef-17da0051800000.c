// https://syzkaller.appspot.com/bug?id=a1f5816d285b5e610d7d7f9b04bcfb9732fdc5ef
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

#define BITMASK_LEN(type, bf_len) (type)((1ull << (bf_len)) - 1)

#define BITMASK_LEN_OFF(type, bf_off, bf_len)                                  \
  (type)(BITMASK_LEN(type, (bf_len)) << (bf_off))

#define STORE_BY_BITMASK(type, addr, val, bf_off, bf_len)                      \
  if ((bf_off) == 0 && (bf_len) == 0) {                                        \
    *(type*)(addr) = (type)(val);                                              \
  } else {                                                                     \
    type new_val = *(type*)(addr);                                             \
    new_val &= ~BITMASK_LEN_OFF(type, (bf_off), (bf_len));                     \
    new_val |= ((type)(val)&BITMASK_LEN(type, (bf_len))) << (bf_off);          \
    *(type*)(addr) = new_val;                                                  \
  }

struct csum_inet {
  uint32_t acc;
};

static void csum_inet_init(struct csum_inet* csum)
{
  csum->acc = 0;
}

static void csum_inet_update(struct csum_inet* csum, const uint8_t* data,
                             size_t length)
{
  if (length == 0)
    return;

  size_t i;
  for (i = 0; i < length - 1; i += 2)
    csum->acc += *(uint16_t*)&data[i];

  if (length & 1)
    csum->acc += (uint16_t)data[length - 1];

  while (csum->acc > 0xffff)
    csum->acc = (csum->acc & 0xffff) + (csum->acc >> 16);
}

static uint16_t csum_inet_digest(struct csum_inet* csum)
{
  return ~csum->acc;
}

static uintptr_t syz_open_dev(uintptr_t a0, uintptr_t a1, uintptr_t a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf));
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

#ifndef __NR_dup2
#define __NR_dup2 63
#endif
#ifndef __NR_ioctl
#define __NR_ioctl 54
#endif
#ifndef __NR_bind
#define __NR_bind 361
#endif
#ifndef __NR_fcntl
#define __NR_fcntl 55
#endif
#ifndef __NR_setsockopt
#define __NR_setsockopt 366
#endif
#ifndef __NR_write
#define __NR_write 4
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[5];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 2, 1, 0);
  memcpy((void*)0x20641ff3, "/dev/net/tun", 13);
  r[1] = syz_open_dev(0x20641ff3, 0, 0);
  r[2] = syscall(__NR_fcntl, r[1], 0, r[1]);
  r[3] = syscall(__NR_socket, 0x11, 0x80003, 8);
  *(uint8_t*)0x20927fd8 = 0x73;
  *(uint8_t*)0x20927fd9 = 0x79;
  *(uint8_t*)0x20927fda = 0x7a;
  *(uint8_t*)0x20927fdb = 0x30;
  *(uint8_t*)0x20927fdc = 0;
  *(uint32_t*)0x20927fe8 = 5;
  *(uint32_t*)0x20927fec = 0;
  *(uint32_t*)0x20927ff0 = 0x20c15000;
  *(uint32_t*)0x20c15000 = 0;
  *(uint32_t*)0x20c15004 = 0;
  *(uint16_t*)0x20c15008 = 0;
  syscall(__NR_ioctl, r[1], 0x400454ca, 0x20927fd8);
  *(uint8_t*)0x2062ffe0 = 0x73;
  *(uint8_t*)0x2062ffe1 = 0x79;
  *(uint8_t*)0x2062ffe2 = 0x7a;
  *(uint8_t*)0x2062ffe3 = 0x30;
  *(uint8_t*)0x2062ffe4 = 0;
  *(uint16_t*)0x2062fff0 = 0x301;
  syscall(__NR_ioctl, r[0], 0x8914, 0x2062ffe0);
  *(uint32_t*)0x20113000 = 0x1000;
  syscall(__NR_setsockopt, r[3], 0x107, 0xf, 0x20113000, 4);
  *(uint8_t*)0x2086d000 = 0x73;
  *(uint8_t*)0x2086d001 = 0x79;
  *(uint8_t*)0x2086d002 = 0x7a;
  *(uint8_t*)0x2086d003 = 0x30;
  *(uint8_t*)0x2086d004 = 0;
  *(uint32_t*)0x2086d010 = 0;
  *(uint8_t*)0x2086d014 = 0;
  *(uint8_t*)0x2086d015 = 0;
  *(uint8_t*)0x2086d016 = 0;
  *(uint8_t*)0x2086d017 = 0;
  *(uint8_t*)0x2086d018 = 0;
  *(uint8_t*)0x2086d019 = 0;
  *(uint8_t*)0x2086d01a = 0;
  *(uint8_t*)0x2086d01b = 0;
  *(uint8_t*)0x2086d01c = 0;
  *(uint8_t*)0x2086d01d = 0;
  *(uint8_t*)0x2086d01e = 0;
  *(uint8_t*)0x2086d01f = 0;
  *(uint8_t*)0x2086d020 = 0;
  *(uint8_t*)0x2086d021 = 0;
  *(uint8_t*)0x2086d022 = 0;
  *(uint8_t*)0x2086d023 = 0;
  *(uint8_t*)0x2086d024 = 0;
  *(uint8_t*)0x2086d025 = 0;
  *(uint8_t*)0x2086d026 = 0;
  *(uint8_t*)0x2086d027 = 0;
  if (syscall(__NR_ioctl, r[3], 0x8933, 0x2086d000) != -1)
    r[4] = *(uint32_t*)0x2086d010;
  *(uint16_t*)0x20c85000 = 0x11;
  *(uint16_t*)0x20c85002 = htobe16(0);
  *(uint32_t*)0x20c85004 = r[4];
  *(uint16_t*)0x20c85008 = 1;
  *(uint8_t*)0x20c8500a = 0;
  *(uint8_t*)0x20c8500b = 6;
  *(uint8_t*)0x20c8500c = 0xaa;
  *(uint8_t*)0x20c8500d = 0xaa;
  *(uint8_t*)0x20c8500e = 0xaa;
  *(uint8_t*)0x20c8500f = 0xaa;
  *(uint8_t*)0x20c85010 = 0xaa;
  *(uint8_t*)0x20c85011 = 0;
  *(uint8_t*)0x20c85012 = 0;
  *(uint8_t*)0x20c85013 = 0;
  syscall(__NR_bind, r[3], 0x20c85000, 0x14);
  syscall(__NR_dup2, r[3], r[2]);
  *(uint8_t*)0x20384000 = 0;
  *(uint8_t*)0x20384001 = 1;
  *(uint16_t*)0x20384002 = 0;
  *(uint16_t*)0x20384004 = 8;
  *(uint16_t*)0x20384006 = 0;
  *(uint16_t*)0x20384008 = 0;
  STORE_BY_BITMASK(uint8_t, 0x2038400a, 5, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x2038400a, 4, 4, 4);
  STORE_BY_BITMASK(uint8_t, 0x2038400b, 0, 0, 2);
  STORE_BY_BITMASK(uint8_t, 0x2038400b, 0, 2, 6);
  *(uint16_t*)0x2038400c = htobe16(0x1c);
  *(uint16_t*)0x2038400e = htobe16(0x64);
  *(uint16_t*)0x20384010 = htobe16(0);
  *(uint8_t*)0x20384012 = 0;
  *(uint8_t*)0x20384013 = 0x29;
  *(uint16_t*)0x20384014 = 0;
  *(uint32_t*)0x20384016 = htobe32(0xe0000001);
  *(uint8_t*)0x2038401a = 0xac;
  *(uint8_t*)0x2038401b = 0x14;
  *(uint8_t*)0x2038401c = 0;
  *(uint8_t*)0x2038401d = 0xaa;
  *(uint8_t*)0x2038401e = 0x15;
  *(uint8_t*)0x2038401f = 0x9e;
  *(uint16_t*)0x20384020 = 0;
  *(uint32_t*)0x20384022 = htobe32(0xe0000001);
  struct csum_inet csum_1;
  csum_inet_init(&csum_1);
  csum_inet_update(&csum_1, (const uint8_t*)0x2038401e, 8);
  *(uint16_t*)0x20384020 = csum_inet_digest(&csum_1);
  struct csum_inet csum_2;
  csum_inet_init(&csum_2);
  csum_inet_update(&csum_2, (const uint8_t*)0x2038400a, 20);
  *(uint16_t*)0x20384014 = csum_inet_digest(&csum_2);
  syscall(__NR_write, r[2], 0x20384000, 0x26);
}

int main()
{
  loop();
  return 0;
}
