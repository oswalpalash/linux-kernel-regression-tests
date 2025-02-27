// https://syzkaller.appspot.com/bug?id=319d600149b51a6469bb9813513414e4a983e63e
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/if_arp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/uio.h>
#include <unistd.h>

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}
#include <stdint.h>
#include <string.h>

const int kFailStatus = 67;
const int kRetryStatus = 69;

static void fail(const char* msg, ...)
{
  int e = errno;
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit((e == ENOMEM || e == EAGAIN) ? kRetryStatus : kFailStatus);
}

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

static void vsnprintf_check(char* str, size_t size, const char* format,
                            va_list args)
{
  int rv;

  rv = vsnprintf(str, size, format, args);
  if (rv < 0)
    fail("tun: snprintf failed");
  if ((size_t)rv >= size)
    fail("tun: string '%s...' doesn't fit into buffer", str);
}

static void snprintf_check(char* str, size_t size, const char* format, ...)
{
  va_list args;

  va_start(args, format);
  vsnprintf_check(str, size, format, args);
  va_end(args);
}

#define COMMAND_MAX_LEN 128
#define PATH_PREFIX                                                            \
  "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin "
#define PATH_PREFIX_LEN (sizeof(PATH_PREFIX) - 1)

static void execute_command(bool panic, const char* format, ...)
{
  va_list args;
  char command[PATH_PREFIX_LEN + COMMAND_MAX_LEN];
  int rv;

  va_start(args, format);
  memcpy(command, PATH_PREFIX, PATH_PREFIX_LEN);
  vsnprintf_check(command + PATH_PREFIX_LEN, COMMAND_MAX_LEN, format, args);
  rv = system(command);
  if (panic && rv != 0)
    fail("tun: command \"%s\" failed with code %d", &command[0], rv);

  va_end(args);
}

static int tunfd = -1;
static int tun_frags_enabled;

#define SYZ_TUN_MAX_PACKET_SIZE 1000

#define MAX_PIDS 32
#define ADDR_MAX_LEN 32

#define LOCAL_MAC "aa:aa:aa:aa:%02hx:aa"
#define REMOTE_MAC "aa:aa:aa:aa:%02hx:bb"

#define LOCAL_IPV4 "172.20.%d.170"
#define REMOTE_IPV4 "172.20.%d.187"

#define LOCAL_IPV6 "fe80::%02hx:aa"
#define REMOTE_IPV6 "fe80::%02hx:bb"

#define IFF_NAPI 0x0010
#define IFF_NAPI_FRAGS 0x0020

static void initialize_tun(int id)
{
  if (id >= MAX_PIDS)
    fail("tun: no more than %d executors", MAX_PIDS);

  tunfd = open("/dev/net/tun", O_RDWR | O_NONBLOCK);
  if (tunfd == -1) {
    printf("tun: can't open /dev/net/tun: please enable CONFIG_TUN=y\n");
    printf("otherwise fuzzing or reproducing might not work as intended\n");
    return;
  }

  char iface[IFNAMSIZ];
  snprintf_check(iface, sizeof(iface), "syz%d", id);

  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, iface, IFNAMSIZ);
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI | IFF_NAPI | IFF_NAPI_FRAGS;
  if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0) {
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0)
      fail("tun: ioctl(TUNSETIFF) failed");
  }
  if (ioctl(tunfd, TUNGETIFF, (void*)&ifr) < 0)
    fail("tun: ioctl(TUNGETIFF) failed");
  tun_frags_enabled = (ifr.ifr_flags & IFF_NAPI_FRAGS) != 0;

  char local_mac[ADDR_MAX_LEN];
  snprintf_check(local_mac, sizeof(local_mac), LOCAL_MAC, id);
  char remote_mac[ADDR_MAX_LEN];
  snprintf_check(remote_mac, sizeof(remote_mac), REMOTE_MAC, id);

  char local_ipv4[ADDR_MAX_LEN];
  snprintf_check(local_ipv4, sizeof(local_ipv4), LOCAL_IPV4, id);
  char remote_ipv4[ADDR_MAX_LEN];
  snprintf_check(remote_ipv4, sizeof(remote_ipv4), REMOTE_IPV4, id);

  char local_ipv6[ADDR_MAX_LEN];
  snprintf_check(local_ipv6, sizeof(local_ipv6), LOCAL_IPV6, id);
  char remote_ipv6[ADDR_MAX_LEN];
  snprintf_check(remote_ipv6, sizeof(remote_ipv6), REMOTE_IPV6, id);

  execute_command(1, "sysctl -w net.ipv6.conf.%s.accept_dad=0", iface);

  execute_command(1, "sysctl -w net.ipv6.conf.%s.router_solicitations=0",
                  iface);

  execute_command(1, "ip link set dev %s address %s", iface, local_mac);
  execute_command(1, "ip addr add %s/24 dev %s", local_ipv4, iface);
  execute_command(1, "ip -6 addr add %s/120 dev %s", local_ipv6, iface);
  execute_command(1, "ip neigh add %s lladdr %s dev %s nud permanent",
                  remote_ipv4, remote_mac, iface);
  execute_command(1, "ip -6 neigh add %s lladdr %s dev %s nud permanent",
                  remote_ipv6, remote_mac, iface);
  execute_command(1, "ip link set dev %s up", iface);
}

#define DEV_IPV4 "172.20.%d.%d"
#define DEV_IPV6 "fe80::%02hx:%02hx"
#define DEV_MAC "aa:aa:aa:aa:%02hx:%02hx"

static void initialize_netdevices(int id)
{
  unsigned i;
  const char* devtypes[] = {"ip6gretap", "bridge", "vcan"};
  const char* devnames[] = {"lo",       "sit0",    "bridge0", "vcan0",
                            "tunl0",    "gre0",    "gretap0", "ip_vti0",
                            "ip6_vti0", "ip6tnl0", "ip6gre0", "ip6gretap0",
                            "erspan0"};

  for (i = 0; i < sizeof(devtypes) / (sizeof(devtypes[0])); i++)
    execute_command(0, "ip link add dev %s0 type %s", devtypes[i], devtypes[i]);
  for (i = 0; i < sizeof(devnames) / (sizeof(devnames[0])); i++) {
    char addr[ADDR_MAX_LEN];
    snprintf_check(addr, sizeof(addr), DEV_IPV4, id, id + 10);
    execute_command(0, "ip -4 addr add %s/24 dev %s", addr, devnames[i]);
    snprintf_check(addr, sizeof(addr), DEV_IPV6, id, id + 10);
    execute_command(0, "ip -6 addr add %s/120 dev %s", addr, devnames[i]);
    snprintf_check(addr, sizeof(addr), DEV_MAC, id, id + 10);
    execute_command(0, "ip link set dev %s address %s", devnames[i], addr);
    execute_command(0, "ip link set dev %s up", devnames[i]);
  }
}

static void setup_tun(uint64_t pid, bool enable_tun)
{
  if (enable_tun) {
    initialize_tun(pid);
    initialize_netdevices(pid);
  }
}

#define MAX_FRAGS 4
struct vnet_fragmentation {
  uint32_t full;
  uint32_t count;
  uint32_t frags[MAX_FRAGS];
};

static uintptr_t syz_emit_ethernet(uintptr_t a0, uintptr_t a1, uintptr_t a2)
{
  if (tunfd < 0)
    return (uintptr_t)-1;

  uint32_t length = a0;
  char* data = (char*)a1;

  struct vnet_fragmentation* frags = (struct vnet_fragmentation*)a2;
  struct iovec vecs[MAX_FRAGS + 1];
  uint32_t nfrags = 0;
  if (!tun_frags_enabled || frags == NULL) {
    vecs[nfrags].iov_base = data;
    vecs[nfrags].iov_len = length;
    nfrags++;
  } else {
    bool full = true;
    uint32_t i, count = 0;
    full = frags->full;
    count = frags->count;
    if (count > MAX_FRAGS)
      count = MAX_FRAGS;
    for (i = 0; i < count && length != 0; i++) {
      uint32_t size = 0;
      size = frags->frags[i];
      if (size > length)
        size = length;
      vecs[nfrags].iov_base = data;
      vecs[nfrags].iov_len = size;
      nfrags++;
      data += size;
      length -= size;
    }
    if (length != 0 && (full || nfrags == 0)) {
      vecs[nfrags].iov_base = data;
      vecs[nfrags].iov_len = length;
      nfrags++;
    }
  }
  return writev(tunfd, vecs, nfrags);
}

static void test();

void loop()
{
  while (1) {
    test();
  }
}

long r[1];
void test()
{
  memset(r, -1, sizeof(r));
  *(uint8_t*)0x202d0ff1 = 1;
  *(uint8_t*)0x202d0ff2 = 0x80;
  *(uint8_t*)0x202d0ff3 = 0xc2;
  *(uint8_t*)0x202d0ff4 = 0;
  *(uint8_t*)0x202d0ff5 = 0;
  *(uint8_t*)0x202d0ff6 = 0;
  *(uint8_t*)0x202d0ff7 = 1;
  *(uint8_t*)0x202d0ff8 = 0x80;
  *(uint8_t*)0x202d0ff9 = 0xc2;
  *(uint8_t*)0x202d0ffa = 0;
  *(uint8_t*)0x202d0ffb = 0;
  *(uint8_t*)0x202d0ffc = 0;
  *(uint16_t*)0x202d0ffd = htobe16(0x800);
  STORE_BY_BITMASK(uint8_t, 0x202d0fff, 5, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x202d0fff, 4, 4, 4);
  STORE_BY_BITMASK(uint8_t, 0x202d1000, 0, 0, 2);
  STORE_BY_BITMASK(uint8_t, 0x202d1000, 0, 2, 6);
  *(uint16_t*)0x202d1001 = htobe16(0x28);
  *(uint16_t*)0x202d1003 = 0;
  *(uint16_t*)0x202d1005 = htobe16(0);
  *(uint8_t*)0x202d1007 = 0;
  *(uint8_t*)0x202d1008 = 0;
  *(uint16_t*)0x202d1009 = 0;
  *(uint32_t*)0x202d100b = htobe32(0);
  *(uint32_t*)0x202d100f = htobe32(0xe0000001);
  *(uint8_t*)0x202d1013 = 0xd;
  *(uint8_t*)0x202d1014 = 0;
  *(uint16_t*)0x202d1015 = 0;
  *(uint16_t*)0x202d1017 = htobe16(0);
  *(uint16_t*)0x202d1019 = htobe16(0);
  *(uint32_t*)0x202d101b = htobe32(0);
  *(uint32_t*)0x202d101f = htobe32(0);
  *(uint32_t*)0x202d1023 = htobe32(0);
  *(uint32_t*)0x20ea3000 = 0;
  *(uint32_t*)0x20ea3004 = 0;
  struct csum_inet csum_1;
  csum_inet_init(&csum_1);
  csum_inet_update(&csum_1, (const uint8_t*)0x202d1013, 20);
  *(uint16_t*)0x202d1015 = csum_inet_digest(&csum_1);
  struct csum_inet csum_2;
  csum_inet_init(&csum_2);
  csum_inet_update(&csum_2, (const uint8_t*)0x202d0fff, 20);
  *(uint16_t*)0x202d1009 = csum_inet_digest(&csum_2);
  syz_emit_ethernet(0x36, 0x202d0ff1, 0x20ea3000);
  r[0] = syscall(__NR_socket, 0xa, 2, 0);
  memcpy((void*)0x20000000, "\x66\x69\x6c\x74\x65\x72\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20000020 = 0xe;
  *(uint32_t*)0x20000024 = 4;
  *(uint32_t*)0x20000028 = 0x510;
  *(uint32_t*)0x2000002c = -1;
  *(uint32_t*)0x20000030 = 0xd0;
  *(uint32_t*)0x20000034 = 0xd0;
  *(uint32_t*)0x20000038 = 0xd0;
  *(uint32_t*)0x2000003c = -1;
  *(uint32_t*)0x20000040 = -1;
  *(uint32_t*)0x20000044 = 0x440;
  *(uint32_t*)0x20000048 = 0x440;
  *(uint32_t*)0x2000004c = 0x440;
  *(uint32_t*)0x20000050 = -1;
  *(uint32_t*)0x20000054 = 4;
  *(uint64_t*)0x20000058 = 0x20c6a000;
  *(uint8_t*)0x20000060 = 0xfe;
  *(uint8_t*)0x20000061 = 0x80;
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
  *(uint8_t*)0x2000006d = 0;
  *(uint8_t*)0x2000006e = 0;
  *(uint8_t*)0x2000006f = 0xaa;
  *(uint8_t*)0x20000070 = 0;
  *(uint8_t*)0x20000071 = 0;
  *(uint8_t*)0x20000072 = 0;
  *(uint8_t*)0x20000073 = 0;
  *(uint8_t*)0x20000074 = 0;
  *(uint8_t*)0x20000075 = 0;
  *(uint8_t*)0x20000076 = 0;
  *(uint8_t*)0x20000077 = 0;
  *(uint8_t*)0x20000078 = 0;
  *(uint8_t*)0x20000079 = 0;
  *(uint8_t*)0x2000007a = -1;
  *(uint8_t*)0x2000007b = -1;
  *(uint32_t*)0x2000007c = htobe32(0);
  *(uint32_t*)0x20000080 = htobe32(0);
  *(uint32_t*)0x20000084 = htobe32(0);
  *(uint32_t*)0x20000088 = htobe32(0);
  *(uint32_t*)0x2000008c = htobe32(0);
  *(uint32_t*)0x20000090 = htobe32(0);
  *(uint32_t*)0x20000094 = htobe32(0);
  *(uint32_t*)0x20000098 = htobe32(0);
  *(uint32_t*)0x2000009c = htobe32(0);
  memcpy((void*)0x200000a0,
         "\x65\x72\x73\x70\x61\x6e\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  *(uint8_t*)0x200000b0 = 0x73;
  *(uint8_t*)0x200000b1 = 0x79;
  *(uint8_t*)0x200000b2 = 0x7a;
  *(uint8_t*)0x200000b3 = 0;
  *(uint8_t*)0x200000b4 = 0;
  *(uint8_t*)0x200000c0 = 0;
  *(uint8_t*)0x200000c1 = 0;
  *(uint8_t*)0x200000c2 = 0;
  *(uint8_t*)0x200000c3 = 0;
  *(uint8_t*)0x200000c4 = 0;
  *(uint8_t*)0x200000c5 = 0;
  *(uint8_t*)0x200000c6 = 0;
  *(uint8_t*)0x200000c7 = 0;
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
  *(uint8_t*)0x200000d2 = 0;
  *(uint8_t*)0x200000d3 = 0;
  *(uint8_t*)0x200000d4 = 0;
  *(uint8_t*)0x200000d5 = 0;
  *(uint8_t*)0x200000d6 = 0;
  *(uint8_t*)0x200000d7 = 0;
  *(uint8_t*)0x200000d8 = 0;
  *(uint8_t*)0x200000d9 = 0;
  *(uint8_t*)0x200000da = 0;
  *(uint8_t*)0x200000db = 0;
  *(uint8_t*)0x200000dc = 0;
  *(uint8_t*)0x200000dd = 0;
  *(uint8_t*)0x200000de = 0;
  *(uint8_t*)0x200000df = 0;
  *(uint16_t*)0x200000e0 = 0;
  *(uint8_t*)0x200000e2 = 0;
  *(uint8_t*)0x200000e3 = 0;
  *(uint8_t*)0x200000e4 = 0;
  *(uint32_t*)0x200000e8 = 0;
  *(uint16_t*)0x200000ec = 0xa8;
  *(uint16_t*)0x200000ee = 0xd0;
  *(uint32_t*)0x200000f0 = 0;
  *(uint64_t*)0x200000f8 = 0;
  *(uint64_t*)0x20000100 = 0;
  *(uint16_t*)0x20000108 = 0x28;
  memcpy((void*)0x2000010a, "\x52\x45\x4a\x45\x43\x54\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20000127 = 0;
  *(uint32_t*)0x20000128 = 0;
  *(uint8_t*)0x20000130 = 0;
  *(uint8_t*)0x20000131 = 0;
  *(uint8_t*)0x20000132 = 0;
  *(uint8_t*)0x20000133 = 0;
  *(uint8_t*)0x20000134 = 0;
  *(uint8_t*)0x20000135 = 0;
  *(uint8_t*)0x20000136 = 0;
  *(uint8_t*)0x20000137 = 0;
  *(uint8_t*)0x20000138 = 0;
  *(uint8_t*)0x20000139 = 0;
  *(uint8_t*)0x2000013a = 0;
  *(uint8_t*)0x2000013b = 0;
  *(uint8_t*)0x2000013c = 0;
  *(uint8_t*)0x2000013d = 0;
  *(uint8_t*)0x2000013e = 0;
  *(uint8_t*)0x2000013f = 0;
  *(uint8_t*)0x20000140 = 0;
  *(uint8_t*)0x20000141 = 0;
  *(uint8_t*)0x20000142 = 0;
  *(uint8_t*)0x20000143 = 0;
  *(uint8_t*)0x20000144 = 0;
  *(uint8_t*)0x20000145 = 0;
  *(uint8_t*)0x20000146 = 0;
  *(uint8_t*)0x20000147 = 0;
  *(uint8_t*)0x20000148 = 0;
  *(uint8_t*)0x20000149 = 0;
  *(uint8_t*)0x2000014a = 0;
  *(uint8_t*)0x2000014b = 0;
  *(uint8_t*)0x2000014c = 0;
  *(uint8_t*)0x2000014d = 0;
  *(uint8_t*)0x2000014e = 0;
  *(uint8_t*)0x2000014f = 0;
  *(uint8_t*)0x20000150 = 0;
  *(uint8_t*)0x20000151 = 0;
  *(uint8_t*)0x20000152 = 0;
  *(uint8_t*)0x20000153 = 0;
  *(uint8_t*)0x20000154 = 0;
  *(uint8_t*)0x20000155 = 0;
  *(uint8_t*)0x20000156 = 0;
  *(uint8_t*)0x20000157 = 0;
  *(uint8_t*)0x20000158 = 0;
  *(uint8_t*)0x20000159 = 0;
  *(uint8_t*)0x2000015a = 0;
  *(uint8_t*)0x2000015b = 0;
  *(uint8_t*)0x2000015c = 0;
  *(uint8_t*)0x2000015d = 0;
  *(uint8_t*)0x2000015e = 0;
  *(uint8_t*)0x2000015f = 0;
  *(uint8_t*)0x20000160 = 0;
  *(uint8_t*)0x20000161 = 0;
  *(uint8_t*)0x20000162 = 0;
  *(uint8_t*)0x20000163 = 0;
  *(uint8_t*)0x20000164 = 0;
  *(uint8_t*)0x20000165 = 0;
  *(uint8_t*)0x20000166 = 0;
  *(uint8_t*)0x20000167 = 0;
  *(uint8_t*)0x20000168 = 0;
  *(uint8_t*)0x20000169 = 0;
  *(uint8_t*)0x2000016a = 0;
  *(uint8_t*)0x2000016b = 0;
  *(uint8_t*)0x2000016c = 0;
  *(uint8_t*)0x2000016d = 0;
  *(uint8_t*)0x2000016e = 0;
  *(uint8_t*)0x2000016f = 0;
  *(uint8_t*)0x20000170 = 0;
  *(uint8_t*)0x20000171 = 0;
  *(uint8_t*)0x20000172 = 0;
  *(uint8_t*)0x20000173 = 0;
  *(uint8_t*)0x20000174 = 0;
  *(uint8_t*)0x20000175 = 0;
  *(uint8_t*)0x20000176 = 0;
  *(uint8_t*)0x20000177 = 0;
  *(uint8_t*)0x20000178 = 0;
  *(uint8_t*)0x20000179 = 0;
  *(uint8_t*)0x2000017a = 0;
  *(uint8_t*)0x2000017b = 0;
  *(uint8_t*)0x2000017c = 0;
  *(uint8_t*)0x2000017d = 0;
  *(uint8_t*)0x2000017e = 0;
  *(uint8_t*)0x2000017f = 0;
  *(uint8_t*)0x20000180 = 0;
  *(uint8_t*)0x20000181 = 0;
  *(uint8_t*)0x20000182 = 0;
  *(uint8_t*)0x20000183 = 0;
  *(uint8_t*)0x20000184 = 0;
  *(uint8_t*)0x20000185 = 0;
  *(uint8_t*)0x20000186 = 0;
  *(uint8_t*)0x20000187 = 0;
  *(uint8_t*)0x20000188 = 0;
  *(uint8_t*)0x20000189 = 0;
  *(uint8_t*)0x2000018a = 0;
  *(uint8_t*)0x2000018b = 0;
  *(uint8_t*)0x2000018c = 0;
  *(uint8_t*)0x2000018d = 0;
  *(uint8_t*)0x2000018e = 0;
  *(uint8_t*)0x2000018f = 0;
  *(uint8_t*)0x20000190 = 0;
  *(uint8_t*)0x20000191 = 0;
  *(uint8_t*)0x20000192 = 0;
  *(uint8_t*)0x20000193 = 0;
  *(uint8_t*)0x20000194 = 0;
  *(uint8_t*)0x20000195 = 0;
  *(uint8_t*)0x20000196 = 0;
  *(uint8_t*)0x20000197 = 0;
  *(uint8_t*)0x20000198 = 0;
  *(uint8_t*)0x20000199 = 0;
  *(uint8_t*)0x2000019a = 0;
  *(uint8_t*)0x2000019b = 0;
  *(uint8_t*)0x2000019c = 0;
  *(uint8_t*)0x2000019d = 0;
  *(uint8_t*)0x2000019e = 0;
  *(uint8_t*)0x2000019f = 0;
  *(uint8_t*)0x200001a0 = 0;
  *(uint8_t*)0x200001a1 = 0;
  *(uint8_t*)0x200001a2 = 0;
  *(uint8_t*)0x200001a3 = 0;
  *(uint8_t*)0x200001a4 = 0;
  *(uint8_t*)0x200001a5 = 0;
  *(uint8_t*)0x200001a6 = 0;
  *(uint8_t*)0x200001a7 = 0;
  *(uint8_t*)0x200001a8 = 0;
  *(uint8_t*)0x200001a9 = 0;
  *(uint8_t*)0x200001aa = 0;
  *(uint8_t*)0x200001ab = 0;
  *(uint8_t*)0x200001ac = 0;
  *(uint8_t*)0x200001ad = 0;
  *(uint8_t*)0x200001ae = 0;
  *(uint8_t*)0x200001af = 0;
  *(uint8_t*)0x200001b0 = 0;
  *(uint8_t*)0x200001b1 = 0;
  *(uint8_t*)0x200001b2 = 0;
  *(uint8_t*)0x200001b3 = 0;
  *(uint8_t*)0x200001b4 = 0;
  *(uint8_t*)0x200001b5 = 0;
  *(uint8_t*)0x200001b6 = 0;
  *(uint8_t*)0x200001b7 = 0;
  *(uint32_t*)0x200001b8 = 0;
  *(uint16_t*)0x200001bc = 0x240;
  *(uint16_t*)0x200001be = 0x268;
  *(uint32_t*)0x200001c0 = 0;
  *(uint64_t*)0x200001c8 = 0;
  *(uint64_t*)0x200001d0 = 0;
  *(uint16_t*)0x200001d8 = 0x158;
  memcpy((void*)0x200001da, "\x68\x61\x73\x68\x6c\x69\x6d\x69\x74\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x200001f7 = 3;
  memcpy((void*)0x200001f8,
         "\x64\x75\x6d\x6d\x79\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         255);
  *(uint64_t*)0x200002f8 = 6;
  *(uint64_t*)0x20000300 = 9;
  *(uint32_t*)0x20000308 = 0x62;
  *(uint32_t*)0x2000030c = 0;
  *(uint32_t*)0x20000310 = 0;
  *(uint32_t*)0x20000314 = 0x80000001;
  *(uint32_t*)0x20000318 = 0x10000;
  *(uint32_t*)0x2000031c = 0x10000;
  *(uint8_t*)0x20000320 = 0;
  *(uint8_t*)0x20000321 = 0;
  *(uint64_t*)0x20000328 = 0;
  *(uint16_t*)0x20000330 = 0x40;
  memcpy((void*)0x20000332, "\x63\x6f\x6e\x6e\x6c\x69\x6d\x69\x74\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x2000034f = 1;
  *(uint32_t*)0x20000350 = htobe32(0);
  *(uint32_t*)0x20000354 = htobe32(0);
  *(uint32_t*)0x20000358 = htobe32(0);
  *(uint32_t*)0x2000035c = htobe32(0);
  *(uint32_t*)0x20000360 = 0;
  *(uint32_t*)0x20000364 = 1;
  *(uint64_t*)0x20000368 = 0;
  *(uint16_t*)0x20000370 = 0x28;
  memcpy((void*)0x20000372, "\x52\x45\x4a\x45\x43\x54\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x2000038f = 0;
  *(uint32_t*)0x20000390 = 0;
  *(uint8_t*)0x20000398 = 0;
  *(uint8_t*)0x20000399 = 0;
  *(uint8_t*)0x2000039a = 0;
  *(uint8_t*)0x2000039b = 0;
  *(uint8_t*)0x2000039c = 0;
  *(uint8_t*)0x2000039d = 0;
  *(uint8_t*)0x2000039e = 0;
  *(uint8_t*)0x2000039f = 0;
  *(uint8_t*)0x200003a0 = 0;
  *(uint8_t*)0x200003a1 = 0;
  *(uint8_t*)0x200003a2 = 0;
  *(uint8_t*)0x200003a3 = 0;
  *(uint8_t*)0x200003a4 = 0;
  *(uint8_t*)0x200003a5 = 0;
  *(uint8_t*)0x200003a6 = 0;
  *(uint8_t*)0x200003a7 = 0;
  *(uint8_t*)0x200003a8 = 0;
  *(uint8_t*)0x200003a9 = 0;
  *(uint8_t*)0x200003aa = 0;
  *(uint8_t*)0x200003ab = 0;
  *(uint8_t*)0x200003ac = 0;
  *(uint8_t*)0x200003ad = 0;
  *(uint8_t*)0x200003ae = 0;
  *(uint8_t*)0x200003af = 0;
  *(uint8_t*)0x200003b0 = 0;
  *(uint8_t*)0x200003b1 = 0;
  *(uint8_t*)0x200003b2 = 0;
  *(uint8_t*)0x200003b3 = 0;
  *(uint8_t*)0x200003b4 = 0;
  *(uint8_t*)0x200003b5 = 0;
  *(uint8_t*)0x200003b6 = 0;
  *(uint8_t*)0x200003b7 = 0;
  *(uint8_t*)0x200003b8 = 0;
  *(uint8_t*)0x200003b9 = 0;
  *(uint8_t*)0x200003ba = 0;
  *(uint8_t*)0x200003bb = 0;
  *(uint8_t*)0x200003bc = 0;
  *(uint8_t*)0x200003bd = 0;
  *(uint8_t*)0x200003be = 0;
  *(uint8_t*)0x200003bf = 0;
  *(uint8_t*)0x200003c0 = 0;
  *(uint8_t*)0x200003c1 = 0;
  *(uint8_t*)0x200003c2 = 0;
  *(uint8_t*)0x200003c3 = 0;
  *(uint8_t*)0x200003c4 = 0;
  *(uint8_t*)0x200003c5 = 0;
  *(uint8_t*)0x200003c6 = 0;
  *(uint8_t*)0x200003c7 = 0;
  *(uint8_t*)0x200003c8 = 0;
  *(uint8_t*)0x200003c9 = 0;
  *(uint8_t*)0x200003ca = 0;
  *(uint8_t*)0x200003cb = 0;
  *(uint8_t*)0x200003cc = 0;
  *(uint8_t*)0x200003cd = 0;
  *(uint8_t*)0x200003ce = 0;
  *(uint8_t*)0x200003cf = 0;
  *(uint8_t*)0x200003d0 = 0;
  *(uint8_t*)0x200003d1 = 0;
  *(uint8_t*)0x200003d2 = 0;
  *(uint8_t*)0x200003d3 = 0;
  *(uint8_t*)0x200003d4 = 0;
  *(uint8_t*)0x200003d5 = 0;
  *(uint8_t*)0x200003d6 = 0;
  *(uint8_t*)0x200003d7 = 0;
  *(uint8_t*)0x200003d8 = 0;
  *(uint8_t*)0x200003d9 = 0;
  *(uint8_t*)0x200003da = 0;
  *(uint8_t*)0x200003db = 0;
  *(uint8_t*)0x200003dc = 0;
  *(uint8_t*)0x200003dd = 0;
  *(uint8_t*)0x200003de = 0;
  *(uint8_t*)0x200003df = 0;
  *(uint8_t*)0x200003e0 = 0;
  *(uint8_t*)0x200003e1 = 0;
  *(uint8_t*)0x200003e2 = 0;
  *(uint8_t*)0x200003e3 = 0;
  *(uint8_t*)0x200003e4 = 0;
  *(uint8_t*)0x200003e5 = 0;
  *(uint8_t*)0x200003e6 = 0;
  *(uint8_t*)0x200003e7 = 0;
  *(uint8_t*)0x200003e8 = 0;
  *(uint8_t*)0x200003e9 = 0;
  *(uint8_t*)0x200003ea = 0;
  *(uint8_t*)0x200003eb = 0;
  *(uint8_t*)0x200003ec = 0;
  *(uint8_t*)0x200003ed = 0;
  *(uint8_t*)0x200003ee = 0;
  *(uint8_t*)0x200003ef = 0;
  *(uint8_t*)0x200003f0 = 0;
  *(uint8_t*)0x200003f1 = 0;
  *(uint8_t*)0x200003f2 = 0;
  *(uint8_t*)0x200003f3 = 0;
  *(uint8_t*)0x200003f4 = 0;
  *(uint8_t*)0x200003f5 = 0;
  *(uint8_t*)0x200003f6 = 0;
  *(uint8_t*)0x200003f7 = 0;
  *(uint8_t*)0x200003f8 = 0;
  *(uint8_t*)0x200003f9 = 0;
  *(uint8_t*)0x200003fa = 0;
  *(uint8_t*)0x200003fb = 0;
  *(uint8_t*)0x200003fc = 0;
  *(uint8_t*)0x200003fd = 0;
  *(uint8_t*)0x200003fe = 0;
  *(uint8_t*)0x200003ff = 0;
  *(uint8_t*)0x20000400 = 0;
  *(uint8_t*)0x20000401 = 0;
  *(uint8_t*)0x20000402 = 0;
  *(uint8_t*)0x20000403 = 0;
  *(uint8_t*)0x20000404 = 0;
  *(uint8_t*)0x20000405 = 0;
  *(uint8_t*)0x20000406 = 0;
  *(uint8_t*)0x20000407 = 0;
  *(uint8_t*)0x20000408 = 0;
  *(uint8_t*)0x20000409 = 0;
  *(uint8_t*)0x2000040a = 0;
  *(uint8_t*)0x2000040b = 0;
  *(uint8_t*)0x2000040c = 0;
  *(uint8_t*)0x2000040d = 0;
  *(uint8_t*)0x2000040e = 0;
  *(uint8_t*)0x2000040f = 0;
  *(uint8_t*)0x20000410 = 0;
  *(uint8_t*)0x20000411 = 0;
  *(uint8_t*)0x20000412 = 0;
  *(uint8_t*)0x20000413 = 0;
  *(uint8_t*)0x20000414 = 0;
  *(uint8_t*)0x20000415 = 0;
  *(uint8_t*)0x20000416 = 0;
  *(uint8_t*)0x20000417 = 0;
  *(uint8_t*)0x20000418 = 0;
  *(uint8_t*)0x20000419 = 0;
  *(uint8_t*)0x2000041a = 0;
  *(uint8_t*)0x2000041b = 0;
  *(uint8_t*)0x2000041c = 0;
  *(uint8_t*)0x2000041d = 0;
  *(uint8_t*)0x2000041e = 0;
  *(uint8_t*)0x2000041f = 0;
  *(uint32_t*)0x20000420 = 0;
  *(uint16_t*)0x20000424 = 0xe0;
  *(uint16_t*)0x20000426 = 0x108;
  *(uint32_t*)0x20000428 = 0;
  *(uint64_t*)0x20000430 = 0;
  *(uint64_t*)0x20000438 = 0;
  *(uint16_t*)0x20000440 = 0x38;
  memcpy((void*)0x20000442, "\x63\x6f\x6e\x6e\x62\x79\x74\x65\x73\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x2000045f = 0;
  *(uint64_t*)0x20000460 = 0;
  *(uint64_t*)0x20000468 = 0;
  *(uint8_t*)0x20000470 = 0;
  *(uint8_t*)0x20000471 = 0;
  *(uint16_t*)0x20000478 = 0x28;
  memcpy((void*)0x2000047a, "\x52\x45\x4a\x45\x43\x54\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20000497 = 0;
  *(uint32_t*)0x20000498 = 0;
  *(uint8_t*)0x200004a0 = 0;
  *(uint8_t*)0x200004a1 = 0;
  *(uint8_t*)0x200004a2 = 0;
  *(uint8_t*)0x200004a3 = 0;
  *(uint8_t*)0x200004a4 = 0;
  *(uint8_t*)0x200004a5 = 0;
  *(uint8_t*)0x200004a6 = 0;
  *(uint8_t*)0x200004a7 = 0;
  *(uint8_t*)0x200004a8 = 0;
  *(uint8_t*)0x200004a9 = 0;
  *(uint8_t*)0x200004aa = 0;
  *(uint8_t*)0x200004ab = 0;
  *(uint8_t*)0x200004ac = 0;
  *(uint8_t*)0x200004ad = 0;
  *(uint8_t*)0x200004ae = 0;
  *(uint8_t*)0x200004af = 0;
  *(uint8_t*)0x200004b0 = 0;
  *(uint8_t*)0x200004b1 = 0;
  *(uint8_t*)0x200004b2 = 0;
  *(uint8_t*)0x200004b3 = 0;
  *(uint8_t*)0x200004b4 = 0;
  *(uint8_t*)0x200004b5 = 0;
  *(uint8_t*)0x200004b6 = 0;
  *(uint8_t*)0x200004b7 = 0;
  *(uint8_t*)0x200004b8 = 0;
  *(uint8_t*)0x200004b9 = 0;
  *(uint8_t*)0x200004ba = 0;
  *(uint8_t*)0x200004bb = 0;
  *(uint8_t*)0x200004bc = 0;
  *(uint8_t*)0x200004bd = 0;
  *(uint8_t*)0x200004be = 0;
  *(uint8_t*)0x200004bf = 0;
  *(uint8_t*)0x200004c0 = 0;
  *(uint8_t*)0x200004c1 = 0;
  *(uint8_t*)0x200004c2 = 0;
  *(uint8_t*)0x200004c3 = 0;
  *(uint8_t*)0x200004c4 = 0;
  *(uint8_t*)0x200004c5 = 0;
  *(uint8_t*)0x200004c6 = 0;
  *(uint8_t*)0x200004c7 = 0;
  *(uint8_t*)0x200004c8 = 0;
  *(uint8_t*)0x200004c9 = 0;
  *(uint8_t*)0x200004ca = 0;
  *(uint8_t*)0x200004cb = 0;
  *(uint8_t*)0x200004cc = 0;
  *(uint8_t*)0x200004cd = 0;
  *(uint8_t*)0x200004ce = 0;
  *(uint8_t*)0x200004cf = 0;
  *(uint8_t*)0x200004d0 = 0;
  *(uint8_t*)0x200004d1 = 0;
  *(uint8_t*)0x200004d2 = 0;
  *(uint8_t*)0x200004d3 = 0;
  *(uint8_t*)0x200004d4 = 0;
  *(uint8_t*)0x200004d5 = 0;
  *(uint8_t*)0x200004d6 = 0;
  *(uint8_t*)0x200004d7 = 0;
  *(uint8_t*)0x200004d8 = 0;
  *(uint8_t*)0x200004d9 = 0;
  *(uint8_t*)0x200004da = 0;
  *(uint8_t*)0x200004db = 0;
  *(uint8_t*)0x200004dc = 0;
  *(uint8_t*)0x200004dd = 0;
  *(uint8_t*)0x200004de = 0;
  *(uint8_t*)0x200004df = 0;
  *(uint8_t*)0x200004e0 = 0;
  *(uint8_t*)0x200004e1 = 0;
  *(uint8_t*)0x200004e2 = 0;
  *(uint8_t*)0x200004e3 = 0;
  *(uint8_t*)0x200004e4 = 0;
  *(uint8_t*)0x200004e5 = 0;
  *(uint8_t*)0x200004e6 = 0;
  *(uint8_t*)0x200004e7 = 0;
  *(uint8_t*)0x200004e8 = 0;
  *(uint8_t*)0x200004e9 = 0;
  *(uint8_t*)0x200004ea = 0;
  *(uint8_t*)0x200004eb = 0;
  *(uint8_t*)0x200004ec = 0;
  *(uint8_t*)0x200004ed = 0;
  *(uint8_t*)0x200004ee = 0;
  *(uint8_t*)0x200004ef = 0;
  *(uint8_t*)0x200004f0 = 0;
  *(uint8_t*)0x200004f1 = 0;
  *(uint8_t*)0x200004f2 = 0;
  *(uint8_t*)0x200004f3 = 0;
  *(uint8_t*)0x200004f4 = 0;
  *(uint8_t*)0x200004f5 = 0;
  *(uint8_t*)0x200004f6 = 0;
  *(uint8_t*)0x200004f7 = 0;
  *(uint8_t*)0x200004f8 = 0;
  *(uint8_t*)0x200004f9 = 0;
  *(uint8_t*)0x200004fa = 0;
  *(uint8_t*)0x200004fb = 0;
  *(uint8_t*)0x200004fc = 0;
  *(uint8_t*)0x200004fd = 0;
  *(uint8_t*)0x200004fe = 0;
  *(uint8_t*)0x200004ff = 0;
  *(uint8_t*)0x20000500 = 0;
  *(uint8_t*)0x20000501 = 0;
  *(uint8_t*)0x20000502 = 0;
  *(uint8_t*)0x20000503 = 0;
  *(uint8_t*)0x20000504 = 0;
  *(uint8_t*)0x20000505 = 0;
  *(uint8_t*)0x20000506 = 0;
  *(uint8_t*)0x20000507 = 0;
  *(uint8_t*)0x20000508 = 0;
  *(uint8_t*)0x20000509 = 0;
  *(uint8_t*)0x2000050a = 0;
  *(uint8_t*)0x2000050b = 0;
  *(uint8_t*)0x2000050c = 0;
  *(uint8_t*)0x2000050d = 0;
  *(uint8_t*)0x2000050e = 0;
  *(uint8_t*)0x2000050f = 0;
  *(uint8_t*)0x20000510 = 0;
  *(uint8_t*)0x20000511 = 0;
  *(uint8_t*)0x20000512 = 0;
  *(uint8_t*)0x20000513 = 0;
  *(uint8_t*)0x20000514 = 0;
  *(uint8_t*)0x20000515 = 0;
  *(uint8_t*)0x20000516 = 0;
  *(uint8_t*)0x20000517 = 0;
  *(uint8_t*)0x20000518 = 0;
  *(uint8_t*)0x20000519 = 0;
  *(uint8_t*)0x2000051a = 0;
  *(uint8_t*)0x2000051b = 0;
  *(uint8_t*)0x2000051c = 0;
  *(uint8_t*)0x2000051d = 0;
  *(uint8_t*)0x2000051e = 0;
  *(uint8_t*)0x2000051f = 0;
  *(uint8_t*)0x20000520 = 0;
  *(uint8_t*)0x20000521 = 0;
  *(uint8_t*)0x20000522 = 0;
  *(uint8_t*)0x20000523 = 0;
  *(uint8_t*)0x20000524 = 0;
  *(uint8_t*)0x20000525 = 0;
  *(uint8_t*)0x20000526 = 0;
  *(uint8_t*)0x20000527 = 0;
  *(uint32_t*)0x20000528 = 0;
  *(uint16_t*)0x2000052c = 0xa8;
  *(uint16_t*)0x2000052e = 0xd0;
  *(uint32_t*)0x20000530 = 0;
  *(uint64_t*)0x20000538 = 0;
  *(uint64_t*)0x20000540 = 0;
  *(uint16_t*)0x20000548 = 0x28;
  memcpy((void*)0x2000054a, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20000567 = 0;
  *(uint32_t*)0x20000568 = 0xfffffffe;
  syscall(__NR_setsockopt, r[0], 0x29, 0x40, 0x20000000, 0x570);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    setup_tun(0, true);
    loop();
  }
}
