// https://syzkaller.appspot.com/bug?id=8af2597890938b642c3bf44a85859691fb9d5cfa
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
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

static void test();

void loop()
{
  while (1) {
    test();
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
void test()
{
  long res;
  res = syscall(__NR_socket, 0xa, 2, 0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_socket, 0x18, 1, 1);
  if (res != -1)
    r[1] = res;
  *(uint16_t*)0x205fafd2 = 0x18;
  *(uint32_t*)0x205fafd4 = 1;
  *(uint32_t*)0x205fafd8 = 0;
  *(uint32_t*)0x205fafdc = r[0];
  *(uint16_t*)0x205fafe0 = 2;
  *(uint16_t*)0x205fafe2 = htobe16(0x4e21);
  *(uint8_t*)0x205fafe4 = 0xac;
  *(uint8_t*)0x205fafe5 = 0x14;
  *(uint8_t*)0x205fafe6 = 0;
  *(uint8_t*)0x205fafe7 = 0xbb;
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
  *(uint16_t*)0x200002c0 = 0xa;
  *(uint16_t*)0x200002c2 = htobe16(0x4e21);
  *(uint32_t*)0x200002c4 = 0x7fff;
  *(uint8_t*)0x200002c8 = 0;
  *(uint8_t*)0x200002c9 = 0;
  *(uint8_t*)0x200002ca = 0;
  *(uint8_t*)0x200002cb = 0;
  *(uint8_t*)0x200002cc = 0;
  *(uint8_t*)0x200002cd = 0;
  *(uint8_t*)0x200002ce = 0;
  *(uint8_t*)0x200002cf = 0;
  *(uint8_t*)0x200002d0 = 0;
  *(uint8_t*)0x200002d1 = 0;
  *(uint8_t*)0x200002d2 = -1;
  *(uint8_t*)0x200002d3 = -1;
  *(uint8_t*)0x200002d4 = 0xac;
  *(uint8_t*)0x200002d5 = 0x14;
  *(uint8_t*)0x200002d6 = 0x14;
  *(uint8_t*)0x200002d7 = 0xaa;
  *(uint32_t*)0x200002d8 = 3;
  syscall(__NR_connect, r[0], 0x200002c0, 0x1c);
  *(uint16_t*)0x200021c0 = 0x18;
  *(uint32_t*)0x200021c2 = 0;
  *(uint16_t*)0x200021c6 = 0;
  *(uint8_t*)0x200021c8 = 0;
  *(uint8_t*)0x200021c9 = 0;
  *(uint8_t*)0x200021ca = 0;
  *(uint8_t*)0x200021cb = 0;
  *(uint8_t*)0x200021cc = 0;
  *(uint8_t*)0x200021cd = 0;
  memcpy((void*)0x200021ce,
         "\x69\x66\x62\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  syscall(__NR_sendto, r[1], 0x20001180, 0, 0x40001, 0x200021c0, 0x80);
  *(uint8_t*)0x20694ffe = -1;
  *(uint8_t*)0x20694fff = -1;
  *(uint8_t*)0x20695000 = -1;
  *(uint8_t*)0x20695001 = -1;
  *(uint8_t*)0x20695002 = -1;
  *(uint8_t*)0x20695003 = -1;
  *(uint8_t*)0x20695004 = 1;
  *(uint8_t*)0x20695005 = 0x80;
  *(uint8_t*)0x20695006 = 0xc2;
  *(uint8_t*)0x20695007 = 0;
  *(uint8_t*)0x20695008 = 0;
  *(uint8_t*)0x20695009 = 0;
  *(uint16_t*)0x2069500a = htobe16(0x86dd);
  STORE_BY_BITMASK(uint8_t, 0x2069500c, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x2069500c, 6, 4, 4);
  memcpy((void*)0x2069500d, "\x06\xf5\x26", 3);
  *(uint16_t*)0x20695010 = htobe16(8);
  *(uint8_t*)0x20695012 = 0x11;
  *(uint8_t*)0x20695013 = 0;
  *(uint8_t*)0x20695014 = 0;
  *(uint8_t*)0x20695015 = 0;
  *(uint8_t*)0x20695016 = 0;
  *(uint8_t*)0x20695017 = 0;
  *(uint8_t*)0x20695018 = 0;
  *(uint8_t*)0x20695019 = 0;
  *(uint8_t*)0x2069501a = 0;
  *(uint8_t*)0x2069501b = 0;
  *(uint8_t*)0x2069501c = 0;
  *(uint8_t*)0x2069501d = 0;
  *(uint8_t*)0x2069501e = 0;
  *(uint8_t*)0x2069501f = 0;
  *(uint8_t*)0x20695020 = 0;
  *(uint8_t*)0x20695021 = 0;
  *(uint8_t*)0x20695022 = 0;
  *(uint8_t*)0x20695023 = 0;
  *(uint8_t*)0x20695024 = -1;
  *(uint8_t*)0x20695025 = 2;
  *(uint8_t*)0x20695026 = 0;
  *(uint8_t*)0x20695027 = 0;
  *(uint8_t*)0x20695028 = 0;
  *(uint8_t*)0x20695029 = 0;
  *(uint8_t*)0x2069502a = 0;
  *(uint8_t*)0x2069502b = 0;
  *(uint8_t*)0x2069502c = 0;
  *(uint8_t*)0x2069502d = 0;
  *(uint8_t*)0x2069502e = 0;
  *(uint8_t*)0x2069502f = 0;
  *(uint8_t*)0x20695030 = 0;
  *(uint8_t*)0x20695031 = 0;
  *(uint8_t*)0x20695032 = 0;
  *(uint8_t*)0x20695033 = 1;
  *(uint16_t*)0x20695034 = htobe16(0);
  *(uint16_t*)0x20695036 = htobe16(0x4e22);
  *(uint16_t*)0x20695038 = htobe16(8);
  *(uint16_t*)0x2069503a = 0;
  *(uint32_t*)0x20775000 = 0;
  *(uint32_t*)0x20775004 = 0;
  *(uint32_t*)0x20775008 = 0;
  *(uint32_t*)0x2077500c = 0;
  *(uint32_t*)0x20775010 = 0;
  *(uint32_t*)0x20775014 = 0;
  struct csum_inet csum_1;
  csum_inet_init(&csum_1);
  csum_inet_update(&csum_1, (const uint8_t*)0x20695014, 16);
  csum_inet_update(&csum_1, (const uint8_t*)0x20695024, 16);
  uint32_t csum_1_chunk_2 = 0x8000000;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_2, 4);
  uint32_t csum_1_chunk_3 = 0x11000000;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_3, 4);
  csum_inet_update(&csum_1, (const uint8_t*)0x20695034, 8);
  *(uint16_t*)0x2069503a = csum_inet_digest(&csum_1);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    loop();
  }
}
