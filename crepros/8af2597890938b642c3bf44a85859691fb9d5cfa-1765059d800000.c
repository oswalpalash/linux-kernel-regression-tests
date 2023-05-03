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

long r[2];
void test()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 0xa, 2, 0);
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
  *(uint16_t*)0x20d94fe4 = 0xa;
  *(uint16_t*)0x20d94fe6 = htobe16(0x4e20);
  *(uint32_t*)0x20d94fe8 = 4;
  *(uint8_t*)0x20d94fec = 0xfe;
  *(uint8_t*)0x20d94fed = 0x80;
  *(uint8_t*)0x20d94fee = 0;
  *(uint8_t*)0x20d94fef = 0;
  *(uint8_t*)0x20d94ff0 = 0;
  *(uint8_t*)0x20d94ff1 = 0;
  *(uint8_t*)0x20d94ff2 = 0;
  *(uint8_t*)0x20d94ff3 = 0;
  *(uint8_t*)0x20d94ff4 = 0;
  *(uint8_t*)0x20d94ff5 = 0;
  *(uint8_t*)0x20d94ff6 = 0;
  *(uint8_t*)0x20d94ff7 = 0;
  *(uint8_t*)0x20d94ff8 = 0;
  *(uint8_t*)0x20d94ff9 = 0;
  *(uint8_t*)0x20d94ffa = 0;
  *(uint8_t*)0x20d94ffb = 0xaa;
  *(uint32_t*)0x20d94ffc = 9;
  syscall(__NR_connect, r[0], 0x20d94fe4, 0x1c);
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
  memcpy((void*)0x20147fd6, "system_u:object_r:klogd_var_run_t:s0", 36);
  *(uint8_t*)0x20147ffa = 0x20;
  memcpy((void*)0x20147ffb, "root", 5);
  syscall(__NR_write, r[1], 0x20147fd6, 0x2a);
  *(uint8_t*)0x201a7000 = -1;
  *(uint8_t*)0x201a7001 = -1;
  *(uint8_t*)0x201a7002 = -1;
  *(uint8_t*)0x201a7003 = -1;
  *(uint8_t*)0x201a7004 = -1;
  *(uint8_t*)0x201a7005 = -1;
  *(uint8_t*)0x201a7006 = 0xaa;
  *(uint8_t*)0x201a7007 = 0xaa;
  *(uint8_t*)0x201a7008 = 0xaa;
  *(uint8_t*)0x201a7009 = 0xaa;
  *(uint8_t*)0x201a700a = 0;
  *(uint8_t*)0x201a700b = 0xaa;
  *(uint16_t*)0x201a700c = htobe16(0x800);
  STORE_BY_BITMASK(uint8_t, 0x201a700e, 5, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x201a700e, 4, 4, 4);
  STORE_BY_BITMASK(uint8_t, 0x201a700f, 0, 0, 2);
  STORE_BY_BITMASK(uint8_t, 0x201a700f, 0, 2, 6);
  *(uint16_t*)0x201a7010 = htobe16(0x24);
  *(uint16_t*)0x201a7012 = 0;
  *(uint16_t*)0x201a7014 = htobe16(0);
  *(uint8_t*)0x201a7016 = 0;
  *(uint8_t*)0x201a7017 = 0x29;
  *(uint16_t*)0x201a7018 = 0;
  *(uint32_t*)0x201a701a = htobe32(0);
  *(uint32_t*)0x201a701e = htobe32(-1);
  *(uint16_t*)0x201a7022 = 0;
  *(uint16_t*)0x201a7024 = 0;
  *(uint8_t*)0x201a7026 = 4;
  STORE_BY_BITMASK(uint8_t, 0x201a7027, 1, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x201a7027, 0, 4, 4);
  *(uint16_t*)0x201a7028 = 0;
  STORE_BY_BITMASK(uint8_t, 0x201a702a, 0, 0, 1);
  STORE_BY_BITMASK(uint8_t, 0x201a702a, 0, 1, 4);
  STORE_BY_BITMASK(uint8_t, 0x201a702a, 0, 5, 3);
  memcpy((void*)0x201a702b, "\x93\x40\xbf", 3);
  *(uint8_t*)0x201a702e = 0;
  memcpy((void*)0x201a702f, "\x28\xe1\xae", 3);
  *(uint32_t*)0x2049b000 = 0;
  *(uint32_t*)0x2049b004 = 1;
  *(uint32_t*)0x2049b008 = 0;
  struct csum_inet csum_1;
  csum_inet_init(&csum_1);
  csum_inet_update(&csum_1, (const uint8_t*)0x201a701a, 4);
  csum_inet_update(&csum_1, (const uint8_t*)0x201a701e, 4);
  uint16_t csum_1_chunk_2 = 0x2100;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_2, 2);
  uint16_t csum_1_chunk_3 = 0x1000;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_3, 2);
  csum_inet_update(&csum_1, (const uint8_t*)0x201a7022, 16);
  *(uint16_t*)0x201a7028 = csum_inet_digest(&csum_1);
  struct csum_inet csum_2;
  csum_inet_init(&csum_2);
  csum_inet_update(&csum_2, (const uint8_t*)0x201a700e, 20);
  *(uint16_t*)0x201a7018 = csum_inet_digest(&csum_2);
}

int main()
{
  for (;;) {
    loop();
  }
}
