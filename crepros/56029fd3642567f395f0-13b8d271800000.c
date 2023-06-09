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

long r[1];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  *(uint8_t*)0x20ee3000 = 1;
  *(uint8_t*)0x20ee3001 = 0x80;
  *(uint8_t*)0x20ee3002 = 0xc2;
  *(uint8_t*)0x20ee3003 = 0;
  *(uint8_t*)0x20ee3004 = 0;
  *(uint8_t*)0x20ee3005 = 0;
  *(uint8_t*)0x20ee3006 = 0xaa;
  *(uint8_t*)0x20ee3007 = 0xaa;
  *(uint8_t*)0x20ee3008 = 0xaa;
  *(uint8_t*)0x20ee3009 = 0xaa;
  *(uint8_t*)0x20ee300a = 0;
  *(uint8_t*)0x20ee300b = 0xbb;
  *(uint16_t*)0x20ee300c = htobe16(0x800);
  STORE_BY_BITMASK(uint8_t, 0x20ee300e, 6, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x20ee300e, 4, 4, 4);
  STORE_BY_BITMASK(uint8_t, 0x20ee300f, 0, 0, 2);
  STORE_BY_BITMASK(uint8_t, 0x20ee300f, 0, 2, 6);
  *(uint16_t*)0x20ee3010 = htobe16(0x28);
  *(uint16_t*)0x20ee3012 = 0;
  *(uint16_t*)0x20ee3014 = htobe16(0);
  *(uint8_t*)0x20ee3016 = 0;
  *(uint8_t*)0x20ee3017 = 0;
  *(uint16_t*)0x20ee3018 = 0;
  *(uint8_t*)0x20ee301a = 0xac;
  *(uint8_t*)0x20ee301b = 0x14;
  *(uint8_t*)0x20ee301c = 0;
  *(uint8_t*)0x20ee301d = 0;
  *(uint32_t*)0x20ee301e = htobe32(-1);
  *(uint8_t*)0x20ee3022 = 0x94;
  *(uint8_t*)0x20ee3023 = 2;
  *(uint8_t*)0x20ee3024 = 0x89;
  *(uint8_t*)0x20ee3025 = 2;
  STORE_BY_BITMASK(uint16_t, 0x20ee3026, 0, 0, 1);
  STORE_BY_BITMASK(uint16_t, 0x20ee3026, 0, 1, 1);
  STORE_BY_BITMASK(uint16_t, 0x20ee3026, 1, 2, 1);
  STORE_BY_BITMASK(uint16_t, 0x20ee3026, 0, 3, 1);
  STORE_BY_BITMASK(uint16_t, 0x20ee3026, 0, 4, 4);
  STORE_BY_BITMASK(uint16_t, 0x20ee3026, 0, 8, 1);
  STORE_BY_BITMASK(uint16_t, 0x20ee3026, 0, 9, 4);
  STORE_BY_BITMASK(uint16_t, 0x20ee3026, 1, 13, 3);
  *(uint16_t*)0x20ee3028 = htobe16(0x880b);
  *(uint16_t*)0x20ee302a = htobe16(0);
  *(uint16_t*)0x20ee302c = htobe16(0);
  STORE_BY_BITMASK(uint16_t, 0x20ee302e, 0, 0, 1);
  STORE_BY_BITMASK(uint16_t, 0x20ee302e, 0, 1, 1);
  STORE_BY_BITMASK(uint16_t, 0x20ee302e, 0, 2, 1);
  STORE_BY_BITMASK(uint16_t, 0x20ee302e, 0, 3, 1);
  STORE_BY_BITMASK(uint16_t, 0x20ee302e, 0, 4, 9);
  STORE_BY_BITMASK(uint16_t, 0x20ee302e, 0, 13, 3);
  *(uint16_t*)0x20ee3030 = htobe16(0);
  STORE_BY_BITMASK(uint16_t, 0x20ee3032, 0, 0, 1);
  STORE_BY_BITMASK(uint16_t, 0x20ee3032, 0, 1, 1);
  STORE_BY_BITMASK(uint16_t, 0x20ee3032, 0, 2, 1);
  STORE_BY_BITMASK(uint16_t, 0x20ee3032, 0, 3, 1);
  STORE_BY_BITMASK(uint16_t, 0x20ee3032, 0, 4, 9);
  STORE_BY_BITMASK(uint16_t, 0x20ee3032, 0, 13, 3);
  *(uint16_t*)0x20ee3034 = htobe16(0);
  *(uint32_t*)0x20aaafec = 0;
  *(uint32_t*)0x20aaaff0 = 0;
  struct csum_inet csum_1;
  csum_inet_init(&csum_1);
  csum_inet_update(&csum_1, (const uint8_t*)0x20ee300e, 24);
  *(uint16_t*)0x20ee3018 = csum_inet_digest(&csum_1);
  r[0] = syscall(__NR_socket, 0xa, 1, 0);
  memcpy((void*)0x20000000, "tls", 4);
  syscall(__NR_setsockopt, r[0], 6, 0x1f, 0x20000000, 4);
  *(uint64_t*)0x2088b000 = 0x20210000;
  *(uint32_t*)0x2088b008 = 0x10;
  *(uint64_t*)0x2088b010 = 0x206f5fa0;
  *(uint64_t*)0x2088b018 = 6;
  *(uint64_t*)0x2088b020 = 0x2072bfd0;
  *(uint64_t*)0x2088b028 = 0x30;
  *(uint32_t*)0x2088b030 = 0x48080;
  *(uint16_t*)0x20210000 = 2;
  *(uint16_t*)0x20210002 = htobe16(0x4e22);
  *(uint32_t*)0x20210004 = htobe32(0);
  *(uint8_t*)0x20210008 = 0;
  *(uint8_t*)0x20210009 = 0;
  *(uint8_t*)0x2021000a = 0;
  *(uint8_t*)0x2021000b = 0;
  *(uint8_t*)0x2021000c = 0;
  *(uint8_t*)0x2021000d = 0;
  *(uint8_t*)0x2021000e = 0;
  *(uint8_t*)0x2021000f = 0;
  *(uint64_t*)0x206f5fa0 = 0x20aa2fc6;
  *(uint64_t*)0x206f5fa8 = 0;
  *(uint64_t*)0x206f5fb0 = 0x20aa2fba;
  *(uint64_t*)0x206f5fb8 = 0;
  *(uint64_t*)0x206f5fc0 = 0x20aa2000;
  *(uint64_t*)0x206f5fc8 = 0;
  *(uint64_t*)0x206f5fd0 = 0x20aa2000;
  *(uint64_t*)0x206f5fd8 = 0;
  *(uint64_t*)0x206f5fe0 = 0x20aa2000;
  *(uint64_t*)0x206f5fe8 = 0;
  *(uint64_t*)0x206f5ff0 = 0x20aa2f06;
  *(uint64_t*)0x206f5ff8 = 0;
  *(uint64_t*)0x2072bfd0 = 0x20;
  *(uint32_t*)0x2072bfd8 = 0x84;
  *(uint32_t*)0x2072bfdc = 2;
  *(uint16_t*)0x2072bfe0 = 0x1294;
  *(uint16_t*)0x2072bfe2 = 0x200;
  *(uint32_t*)0x2072bfe4 = 0x54;
  *(uint32_t*)0x2072bfe8 = 0x1bb;
  *(uint32_t*)0x2072bfec = 0;
  syscall(__NR_sendmsg, r[0], 0x2088b000, 0x20004000);
}

int main()
{
  loop();
  return 0;
}
