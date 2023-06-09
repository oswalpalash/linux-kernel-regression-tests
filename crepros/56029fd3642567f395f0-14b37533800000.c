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
  *(uint16_t*)0x200000c0 = 0xa;
  *(uint16_t*)0x200000c2 = htobe16(0x4e23);
  *(uint32_t*)0x200000c4 = 0x7fff;
  *(uint8_t*)0x200000c8 = 0;
  *(uint8_t*)0x200000c9 = 0;
  *(uint8_t*)0x200000ca = 0;
  *(uint8_t*)0x200000cb = 0;
  *(uint8_t*)0x200000cc = 0;
  *(uint8_t*)0x200000cd = 0;
  *(uint8_t*)0x200000ce = 0;
  *(uint8_t*)0x200000cf = 0;
  *(uint8_t*)0x200000d0 = 0;
  *(uint8_t*)0x200000d1 = 0;
  *(uint8_t*)0x200000d2 = -1;
  *(uint8_t*)0x200000d3 = -1;
  *(uint8_t*)0x200000d4 = 0xac;
  *(uint8_t*)0x200000d5 = 0x14;
  *(uint8_t*)0x200000d6 = 0x14;
  *(uint8_t*)0x200000d7 = 0xaa;
  *(uint32_t*)0x200000d8 = 3;
  syscall(__NR_connect, r[0], 0x200000c0, 0x1c);
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
  *(uint8_t*)0x20000000 = 1;
  *(uint8_t*)0x20000001 = 0x80;
  *(uint8_t*)0x20000002 = 0xc2;
  *(uint8_t*)0x20000003 = 0;
  *(uint8_t*)0x20000004 = 0;
  *(uint8_t*)0x20000005 = 0;
  *(uint8_t*)0x20000006 = 0;
  *(uint8_t*)0x20000007 = 0;
  *(uint8_t*)0x20000008 = 0;
  *(uint8_t*)0x20000009 = 0;
  *(uint8_t*)0x2000000a = 0;
  *(uint8_t*)0x2000000b = 0;
  *(uint16_t*)0x2000000c = htobe16(0x800);
  STORE_BY_BITMASK(uint8_t, 0x2000000e, 5, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x2000000e, 4, 4, 4);
  STORE_BY_BITMASK(uint8_t, 0x2000000f, 0, 0, 2);
  STORE_BY_BITMASK(uint8_t, 0x2000000f, 0, 2, 6);
  *(uint16_t*)0x20000010 = htobe16(0x30);
  *(uint16_t*)0x20000012 = htobe16(0);
  *(uint16_t*)0x20000014 = htobe16(0);
  *(uint8_t*)0x20000016 = 0;
  *(uint8_t*)0x20000017 = 0x67;
  *(uint16_t*)0x20000018 = 0;
  *(uint32_t*)0x2000001a = htobe32(0);
  *(uint32_t*)0x2000001e = htobe32(-1);
  *(uint8_t*)0x20000022 = 0x21;
  *(uint8_t*)0x20000023 = 0;
  *(uint16_t*)0x20000024 = 0;
  *(uint8_t*)0x20000026 = 0;
  *(uint8_t*)0x20000027 = 0;
  *(uint16_t*)0x20000028 = 0;
  STORE_BY_BITMASK(uint8_t, 0x2000002a, 5, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x2000002a, 4, 4, 4);
  STORE_BY_BITMASK(uint8_t, 0x2000002b, 0, 0, 2);
  STORE_BY_BITMASK(uint8_t, 0x2000002b, 0, 2, 6);
  *(uint16_t*)0x2000002c = htobe16(3);
  *(uint16_t*)0x2000002e = htobe16(0);
  *(uint16_t*)0x20000030 = htobe16(0);
  *(uint8_t*)0x20000032 = 0;
  *(uint8_t*)0x20000033 = 0;
  *(uint16_t*)0x20000034 = htobe16(0);
  *(uint8_t*)0x20000036 = 0xac;
  *(uint8_t*)0x20000037 = 0x14;
  *(uint8_t*)0x20000038 = -1;
  *(uint8_t*)0x20000039 = 0xbb;
  *(uint32_t*)0x2000003a = htobe32(0xe0000001);
  *(uint32_t*)0x20000040 = 0;
  *(uint32_t*)0x20000044 = 0;
  *(uint32_t*)0x20000048 = 0;
  *(uint32_t*)0x2000004c = 0;
  *(uint32_t*)0x20000050 = 0;
  *(uint32_t*)0x20000054 = 0;
  struct csum_inet csum_1;
  csum_inet_init(&csum_1);
  csum_inet_update(&csum_1, (const uint8_t*)0x20000022, 28);
  *(uint16_t*)0x20000024 = csum_inet_digest(&csum_1);
  struct csum_inet csum_2;
  csum_inet_init(&csum_2);
  csum_inet_update(&csum_2, (const uint8_t*)0x2000000e, 20);
  *(uint16_t*)0x20000018 = csum_inet_digest(&csum_2);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    loop();
  }
}
