// https://syzkaller.appspot.com/bug?id=3b0a690969d5dd390506158e006d5bfdf9861f1e
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

long r[2];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 0xa, 3, 6);
  *(uint16_t*)0x20a10df3 = 0xa;
  *(uint16_t*)0x20a10df5 = 0;
  *(uint32_t*)0x20a10df7 = 0;
  *(uint8_t*)0x20a10dfb = 0xfe;
  *(uint8_t*)0x20a10dfc = 0x80;
  *(uint8_t*)0x20a10dfd = 0;
  *(uint8_t*)0x20a10dfe = 0;
  *(uint8_t*)0x20a10dff = 0;
  *(uint8_t*)0x20a10e00 = 0;
  *(uint8_t*)0x20a10e01 = 0;
  *(uint8_t*)0x20a10e02 = 0;
  *(uint8_t*)0x20a10e03 = 0;
  *(uint8_t*)0x20a10e04 = 0;
  *(uint8_t*)0x20a10e05 = 0;
  *(uint8_t*)0x20a10e06 = 0;
  *(uint8_t*)0x20a10e07 = 0;
  *(uint8_t*)0x20a10e08 = 0;
  *(uint8_t*)0x20a10e09 = 0;
  *(uint8_t*)0x20a10e0a = 0;
  *(uint32_t*)0x20a10e0b = 9;
  syscall(__NR_connect, r[0], 0x20a10df3, 0x1c);
  r[1] = syscall(__NR_socket, 0xa, 2, 0);
  memcpy((void*)0x20fa2000, "\x6e\x61\x74\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20fa2020 = 0x1b;
  *(uint32_t*)0x20fa2024 = 5;
  *(uint32_t*)0x20fa2028 = 0x4d8;
  *(uint32_t*)0x20fa202c = 0xf0;
  *(uint32_t*)0x20fa2030 = 0x318;
  *(uint32_t*)0x20fa2034 = -1;
  *(uint32_t*)0x20fa2038 = 0;
  *(uint32_t*)0x20fa203c = 0x318;
  *(uint32_t*)0x20fa2040 = 0x408;
  *(uint32_t*)0x20fa2044 = 0x408;
  *(uint32_t*)0x20fa2048 = -1;
  *(uint32_t*)0x20fa204c = 0x408;
  *(uint32_t*)0x20fa2050 = 0x408;
  *(uint32_t*)0x20fa2054 = 5;
  *(uint64_t*)0x20fa2058 = 0x201affb0;
  *(uint8_t*)0x20fa2060 = -1;
  *(uint8_t*)0x20fa2061 = 1;
  *(uint8_t*)0x20fa2062 = 0;
  *(uint8_t*)0x20fa2063 = 0;
  *(uint8_t*)0x20fa2064 = 0;
  *(uint8_t*)0x20fa2065 = 0;
  *(uint8_t*)0x20fa2066 = 0;
  *(uint8_t*)0x20fa2067 = 0;
  *(uint8_t*)0x20fa2068 = 0;
  *(uint8_t*)0x20fa2069 = 0;
  *(uint8_t*)0x20fa206a = 0;
  *(uint8_t*)0x20fa206b = 0;
  *(uint8_t*)0x20fa206c = 0;
  *(uint8_t*)0x20fa206d = 0;
  *(uint8_t*)0x20fa206e = 0;
  *(uint8_t*)0x20fa206f = 1;
  *(uint8_t*)0x20fa2070 = 0xfe;
  *(uint8_t*)0x20fa2071 = 0x80;
  *(uint8_t*)0x20fa2072 = 0;
  *(uint8_t*)0x20fa2073 = 0;
  *(uint8_t*)0x20fa2074 = 0;
  *(uint8_t*)0x20fa2075 = 0;
  *(uint8_t*)0x20fa2076 = 0;
  *(uint8_t*)0x20fa2077 = 0;
  *(uint8_t*)0x20fa2078 = 0;
  *(uint8_t*)0x20fa2079 = 0;
  *(uint8_t*)0x20fa207a = 0;
  *(uint8_t*)0x20fa207b = 0;
  *(uint8_t*)0x20fa207c = 0;
  *(uint8_t*)0x20fa207d = 0;
  *(uint8_t*)0x20fa207e = 0;
  *(uint8_t*)0x20fa207f = 0xaa;
  *(uint32_t*)0x20fa2080 = htobe32(0);
  *(uint32_t*)0x20fa2084 = htobe32(0);
  *(uint32_t*)0x20fa2088 = htobe32(0);
  *(uint32_t*)0x20fa208c = htobe32(0);
  *(uint32_t*)0x20fa2090 = htobe32(0);
  *(uint32_t*)0x20fa2094 = htobe32(0);
  *(uint32_t*)0x20fa2098 = htobe32(0);
  *(uint32_t*)0x20fa209c = htobe32(0);
  memcpy((void*)0x20fa20a0,
         "\x69\x70\x36\x67\x72\x65\x74\x61\x70\x30\x00\x00\x00\x00\x00\x00",
         16);
  *(uint8_t*)0x20fa20b0 = 0x73;
  *(uint8_t*)0x20fa20b1 = 0x79;
  *(uint8_t*)0x20fa20b2 = 0x7a;
  *(uint8_t*)0x20fa20b3 = 0;
  *(uint8_t*)0x20fa20b4 = 0;
  *(uint8_t*)0x20fa20c0 = 0;
  *(uint8_t*)0x20fa20c1 = 0;
  *(uint8_t*)0x20fa20c2 = 0;
  *(uint8_t*)0x20fa20c3 = 0;
  *(uint8_t*)0x20fa20c4 = 0;
  *(uint8_t*)0x20fa20c5 = 0;
  *(uint8_t*)0x20fa20c6 = 0;
  *(uint8_t*)0x20fa20c7 = 0;
  *(uint8_t*)0x20fa20c8 = 0;
  *(uint8_t*)0x20fa20c9 = 0;
  *(uint8_t*)0x20fa20ca = 0;
  *(uint8_t*)0x20fa20cb = 0;
  *(uint8_t*)0x20fa20cc = 0;
  *(uint8_t*)0x20fa20cd = 0;
  *(uint8_t*)0x20fa20ce = 0;
  *(uint8_t*)0x20fa20cf = 0;
  *(uint8_t*)0x20fa20d0 = 0;
  *(uint8_t*)0x20fa20d1 = 0;
  *(uint8_t*)0x20fa20d2 = 0;
  *(uint8_t*)0x20fa20d3 = 0;
  *(uint8_t*)0x20fa20d4 = 0;
  *(uint8_t*)0x20fa20d5 = 0;
  *(uint8_t*)0x20fa20d6 = 0;
  *(uint8_t*)0x20fa20d7 = 0;
  *(uint8_t*)0x20fa20d8 = 0;
  *(uint8_t*)0x20fa20d9 = 0;
  *(uint8_t*)0x20fa20da = 0;
  *(uint8_t*)0x20fa20db = 0;
  *(uint8_t*)0x20fa20dc = 0;
  *(uint8_t*)0x20fa20dd = 0;
  *(uint8_t*)0x20fa20de = 0;
  *(uint8_t*)0x20fa20df = 0;
  *(uint16_t*)0x20fa20e0 = 0;
  *(uint8_t*)0x20fa20e2 = 0;
  *(uint8_t*)0x20fa20e3 = 0;
  *(uint8_t*)0x20fa20e4 = 0;
  *(uint32_t*)0x20fa20e8 = 0;
  *(uint16_t*)0x20fa20ec = 0xa8;
  *(uint16_t*)0x20fa20ee = 0xf0;
  *(uint32_t*)0x20fa20f0 = 0;
  *(uint64_t*)0x20fa20f8 = 0;
  *(uint64_t*)0x20fa2100 = 0;
  *(uint16_t*)0x20fa2108 = 0x48;
  memcpy((void*)0x20fa210a, "\x4e\x45\x54\x4d\x41\x50\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20fa2127 = 0;
  *(uint32_t*)0x20fa2128 = 3;
  *(uint8_t*)0x20fa212c = 0;
  *(uint8_t*)0x20fa212d = 0;
  *(uint8_t*)0x20fa212e = 0;
  *(uint8_t*)0x20fa212f = 0;
  *(uint8_t*)0x20fa2130 = 0;
  *(uint8_t*)0x20fa2131 = 0;
  *(uint8_t*)0x20fa2132 = 0;
  *(uint8_t*)0x20fa2133 = 0;
  *(uint8_t*)0x20fa2134 = 0;
  *(uint8_t*)0x20fa2135 = 0;
  *(uint8_t*)0x20fa2136 = -1;
  *(uint8_t*)0x20fa2137 = -1;
  *(uint32_t*)0x20fa2138 = htobe32(0xe0000001);
  *(uint8_t*)0x20fa213c = 0xfe;
  *(uint8_t*)0x20fa213d = 0x80;
  *(uint8_t*)0x20fa213e = 0;
  *(uint8_t*)0x20fa213f = 0;
  *(uint8_t*)0x20fa2140 = 0;
  *(uint8_t*)0x20fa2141 = 0;
  *(uint8_t*)0x20fa2142 = 0;
  *(uint8_t*)0x20fa2143 = 0;
  *(uint8_t*)0x20fa2144 = 0;
  *(uint8_t*)0x20fa2145 = 0;
  *(uint8_t*)0x20fa2146 = 0;
  *(uint8_t*)0x20fa2147 = 0;
  *(uint8_t*)0x20fa2148 = 0;
  *(uint8_t*)0x20fa2149 = 0;
  *(uint8_t*)0x20fa214a = 0;
  *(uint8_t*)0x20fa214b = 0xbb;
  *(uint16_t*)0x20fa214c = 0;
  *(uint16_t*)0x20fa214e = 0;
  *(uint8_t*)0x20fa2150 = 0;
  *(uint8_t*)0x20fa2151 = 0;
  *(uint8_t*)0x20fa2152 = 0;
  *(uint8_t*)0x20fa2153 = 0;
  *(uint8_t*)0x20fa2154 = 0;
  *(uint8_t*)0x20fa2155 = 0;
  *(uint8_t*)0x20fa2156 = 0;
  *(uint8_t*)0x20fa2157 = 0;
  *(uint8_t*)0x20fa2158 = 0;
  *(uint8_t*)0x20fa2159 = 0;
  *(uint8_t*)0x20fa215a = 0;
  *(uint8_t*)0x20fa215b = 0;
  *(uint8_t*)0x20fa215c = 0;
  *(uint8_t*)0x20fa215d = 0;
  *(uint8_t*)0x20fa215e = 0;
  *(uint8_t*)0x20fa215f = 0;
  *(uint8_t*)0x20fa2160 = 0;
  *(uint8_t*)0x20fa2161 = 0;
  *(uint8_t*)0x20fa2162 = 0;
  *(uint8_t*)0x20fa2163 = 0;
  *(uint8_t*)0x20fa2164 = 0;
  *(uint8_t*)0x20fa2165 = 0;
  *(uint8_t*)0x20fa2166 = 0;
  *(uint8_t*)0x20fa2167 = 0;
  *(uint8_t*)0x20fa2168 = 0;
  *(uint8_t*)0x20fa2169 = 0;
  *(uint8_t*)0x20fa216a = 0;
  *(uint8_t*)0x20fa216b = 0;
  *(uint8_t*)0x20fa216c = 0;
  *(uint8_t*)0x20fa216d = 0;
  *(uint8_t*)0x20fa216e = 0;
  *(uint8_t*)0x20fa216f = 0;
  *(uint8_t*)0x20fa2170 = 0;
  *(uint8_t*)0x20fa2171 = 0;
  *(uint8_t*)0x20fa2172 = 0;
  *(uint8_t*)0x20fa2173 = 0;
  *(uint8_t*)0x20fa2174 = 0;
  *(uint8_t*)0x20fa2175 = 0;
  *(uint8_t*)0x20fa2176 = 0;
  *(uint8_t*)0x20fa2177 = 0;
  *(uint8_t*)0x20fa2178 = 0;
  *(uint8_t*)0x20fa2179 = 0;
  *(uint8_t*)0x20fa217a = 0;
  *(uint8_t*)0x20fa217b = 0;
  *(uint8_t*)0x20fa217c = 0;
  *(uint8_t*)0x20fa217d = 0;
  *(uint8_t*)0x20fa217e = 0;
  *(uint8_t*)0x20fa217f = 0;
  *(uint8_t*)0x20fa2180 = 0;
  *(uint8_t*)0x20fa2181 = 0;
  *(uint8_t*)0x20fa2182 = 0;
  *(uint8_t*)0x20fa2183 = 0;
  *(uint8_t*)0x20fa2184 = 0;
  *(uint8_t*)0x20fa2185 = 0;
  *(uint8_t*)0x20fa2186 = 0;
  *(uint8_t*)0x20fa2187 = 0;
  *(uint8_t*)0x20fa2188 = 0;
  *(uint8_t*)0x20fa2189 = 0;
  *(uint8_t*)0x20fa218a = 0;
  *(uint8_t*)0x20fa218b = 0;
  *(uint8_t*)0x20fa218c = 0;
  *(uint8_t*)0x20fa218d = 0;
  *(uint8_t*)0x20fa218e = 0;
  *(uint8_t*)0x20fa218f = 0;
  *(uint8_t*)0x20fa2190 = 0;
  *(uint8_t*)0x20fa2191 = 0;
  *(uint8_t*)0x20fa2192 = 0;
  *(uint8_t*)0x20fa2193 = 0;
  *(uint8_t*)0x20fa2194 = 0;
  *(uint8_t*)0x20fa2195 = 0;
  *(uint8_t*)0x20fa2196 = 0;
  *(uint8_t*)0x20fa2197 = 0;
  *(uint8_t*)0x20fa2198 = 0;
  *(uint8_t*)0x20fa2199 = 0;
  *(uint8_t*)0x20fa219a = 0;
  *(uint8_t*)0x20fa219b = 0;
  *(uint8_t*)0x20fa219c = 0;
  *(uint8_t*)0x20fa219d = 0;
  *(uint8_t*)0x20fa219e = 0;
  *(uint8_t*)0x20fa219f = 0;
  *(uint8_t*)0x20fa21a0 = 0;
  *(uint8_t*)0x20fa21a1 = 0;
  *(uint8_t*)0x20fa21a2 = 0;
  *(uint8_t*)0x20fa21a3 = 0;
  *(uint8_t*)0x20fa21a4 = 0;
  *(uint8_t*)0x20fa21a5 = 0;
  *(uint8_t*)0x20fa21a6 = 0;
  *(uint8_t*)0x20fa21a7 = 0;
  *(uint8_t*)0x20fa21a8 = 0;
  *(uint8_t*)0x20fa21a9 = 0;
  *(uint8_t*)0x20fa21aa = 0;
  *(uint8_t*)0x20fa21ab = 0;
  *(uint8_t*)0x20fa21ac = 0;
  *(uint8_t*)0x20fa21ad = 0;
  *(uint8_t*)0x20fa21ae = 0;
  *(uint8_t*)0x20fa21af = 0;
  *(uint8_t*)0x20fa21b0 = 0;
  *(uint8_t*)0x20fa21b1 = 0;
  *(uint8_t*)0x20fa21b2 = 0;
  *(uint8_t*)0x20fa21b3 = 0;
  *(uint8_t*)0x20fa21b4 = 0;
  *(uint8_t*)0x20fa21b5 = 0;
  *(uint8_t*)0x20fa21b6 = 0;
  *(uint8_t*)0x20fa21b7 = 0;
  *(uint8_t*)0x20fa21b8 = 0;
  *(uint8_t*)0x20fa21b9 = 0;
  *(uint8_t*)0x20fa21ba = 0;
  *(uint8_t*)0x20fa21bb = 0;
  *(uint8_t*)0x20fa21bc = 0;
  *(uint8_t*)0x20fa21bd = 0;
  *(uint8_t*)0x20fa21be = 0;
  *(uint8_t*)0x20fa21bf = 0;
  *(uint8_t*)0x20fa21c0 = 0;
  *(uint8_t*)0x20fa21c1 = 0;
  *(uint8_t*)0x20fa21c2 = 0;
  *(uint8_t*)0x20fa21c3 = 0;
  *(uint8_t*)0x20fa21c4 = 0;
  *(uint8_t*)0x20fa21c5 = 0;
  *(uint8_t*)0x20fa21c6 = 0;
  *(uint8_t*)0x20fa21c7 = 0;
  *(uint8_t*)0x20fa21c8 = 0;
  *(uint8_t*)0x20fa21c9 = 0;
  *(uint8_t*)0x20fa21ca = 0;
  *(uint8_t*)0x20fa21cb = 0;
  *(uint8_t*)0x20fa21cc = 0;
  *(uint8_t*)0x20fa21cd = 0;
  *(uint8_t*)0x20fa21ce = 0;
  *(uint8_t*)0x20fa21cf = 0;
  *(uint8_t*)0x20fa21d0 = 0;
  *(uint8_t*)0x20fa21d1 = 0;
  *(uint8_t*)0x20fa21d2 = 0;
  *(uint8_t*)0x20fa21d3 = 0;
  *(uint8_t*)0x20fa21d4 = 0;
  *(uint8_t*)0x20fa21d5 = 0;
  *(uint8_t*)0x20fa21d6 = 0;
  *(uint8_t*)0x20fa21d7 = 0;
  *(uint32_t*)0x20fa21d8 = 0;
  *(uint16_t*)0x20fa21dc = 0xf0;
  *(uint16_t*)0x20fa21de = 0x138;
  *(uint32_t*)0x20fa21e0 = 0;
  *(uint64_t*)0x20fa21e8 = 0;
  *(uint64_t*)0x20fa21f0 = 0;
  *(uint16_t*)0x20fa21f8 = 0x48;
  memcpy((void*)0x20fa21fa, "\x64\x73\x74\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20fa2217 = 0;
  *(uint32_t*)0x20fa2218 = 0;
  *(uint8_t*)0x20fa221c = 0;
  *(uint8_t*)0x20fa221d = 0;
  *(uint16_t*)0x20fa221e = 0;
  *(uint16_t*)0x20fa2220 = 0;
  *(uint16_t*)0x20fa2222 = 0;
  *(uint16_t*)0x20fa2224 = 0;
  *(uint16_t*)0x20fa2226 = 0;
  *(uint16_t*)0x20fa2228 = 0;
  *(uint16_t*)0x20fa222a = 0;
  *(uint16_t*)0x20fa222c = 0;
  *(uint16_t*)0x20fa222e = 0;
  *(uint16_t*)0x20fa2230 = 0;
  *(uint16_t*)0x20fa2232 = 0;
  *(uint16_t*)0x20fa2234 = 0;
  *(uint16_t*)0x20fa2236 = 0;
  *(uint16_t*)0x20fa2238 = 0;
  *(uint16_t*)0x20fa223a = 0;
  *(uint16_t*)0x20fa223c = 0;
  *(uint8_t*)0x20fa223e = 0;
  *(uint16_t*)0x20fa2240 = 0x48;
  memcpy((void*)0x20fa2242, "\x44\x4e\x41\x54\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20fa225f = 1;
  *(uint32_t*)0x20fa2260 = 0;
  *(uint8_t*)0x20fa2264 = 0xfe;
  *(uint8_t*)0x20fa2265 = 0x80;
  *(uint8_t*)0x20fa2266 = 0;
  *(uint8_t*)0x20fa2267 = 0;
  *(uint8_t*)0x20fa2268 = 0;
  *(uint8_t*)0x20fa2269 = 0;
  *(uint8_t*)0x20fa226a = 0;
  *(uint8_t*)0x20fa226b = 0;
  *(uint8_t*)0x20fa226c = 0;
  *(uint8_t*)0x20fa226d = 0;
  *(uint8_t*)0x20fa226e = 0;
  *(uint8_t*)0x20fa226f = 0;
  *(uint8_t*)0x20fa2270 = 0;
  *(uint8_t*)0x20fa2271 = 0;
  *(uint8_t*)0x20fa2272 = 0;
  *(uint8_t*)0x20fa2273 = 0xbb;
  *(uint8_t*)0x20fa2274 = 0xac;
  *(uint8_t*)0x20fa2275 = 0x14;
  *(uint8_t*)0x20fa2276 = 0;
  *(uint8_t*)0x20fa2277 = 0;
  *(uint16_t*)0x20fa2284 = 0;
  *(uint16_t*)0x20fa2286 = 0;
  *(uint8_t*)0x20fa2288 = 0;
  *(uint8_t*)0x20fa2289 = 0;
  *(uint8_t*)0x20fa228a = 0;
  *(uint8_t*)0x20fa228b = 0;
  *(uint8_t*)0x20fa228c = 0;
  *(uint8_t*)0x20fa228d = 0;
  *(uint8_t*)0x20fa228e = 0;
  *(uint8_t*)0x20fa228f = 0;
  *(uint8_t*)0x20fa2290 = 0;
  *(uint8_t*)0x20fa2291 = 0;
  *(uint8_t*)0x20fa2292 = 0;
  *(uint8_t*)0x20fa2293 = 0;
  *(uint8_t*)0x20fa2294 = 0;
  *(uint8_t*)0x20fa2295 = 0;
  *(uint8_t*)0x20fa2296 = 0;
  *(uint8_t*)0x20fa2297 = 0;
  *(uint8_t*)0x20fa2298 = 0;
  *(uint8_t*)0x20fa2299 = 0;
  *(uint8_t*)0x20fa229a = 0;
  *(uint8_t*)0x20fa229b = 0;
  *(uint8_t*)0x20fa229c = 0;
  *(uint8_t*)0x20fa229d = 0;
  *(uint8_t*)0x20fa229e = 0;
  *(uint8_t*)0x20fa229f = 0;
  *(uint8_t*)0x20fa22a0 = 0;
  *(uint8_t*)0x20fa22a1 = 0;
  *(uint8_t*)0x20fa22a2 = 0;
  *(uint8_t*)0x20fa22a3 = 0;
  *(uint8_t*)0x20fa22a4 = 0;
  *(uint8_t*)0x20fa22a5 = 0;
  *(uint8_t*)0x20fa22a6 = 0;
  *(uint8_t*)0x20fa22a7 = 0;
  *(uint8_t*)0x20fa22a8 = 0;
  *(uint8_t*)0x20fa22a9 = 0;
  *(uint8_t*)0x20fa22aa = 0;
  *(uint8_t*)0x20fa22ab = 0;
  *(uint8_t*)0x20fa22ac = 0;
  *(uint8_t*)0x20fa22ad = 0;
  *(uint8_t*)0x20fa22ae = 0;
  *(uint8_t*)0x20fa22af = 0;
  *(uint8_t*)0x20fa22b0 = 0;
  *(uint8_t*)0x20fa22b1 = 0;
  *(uint8_t*)0x20fa22b2 = 0;
  *(uint8_t*)0x20fa22b3 = 0;
  *(uint8_t*)0x20fa22b4 = 0;
  *(uint8_t*)0x20fa22b5 = 0;
  *(uint8_t*)0x20fa22b6 = 0;
  *(uint8_t*)0x20fa22b7 = 0;
  *(uint8_t*)0x20fa22b8 = 0;
  *(uint8_t*)0x20fa22b9 = 0;
  *(uint8_t*)0x20fa22ba = 0;
  *(uint8_t*)0x20fa22bb = 0;
  *(uint8_t*)0x20fa22bc = 0;
  *(uint8_t*)0x20fa22bd = 0;
  *(uint8_t*)0x20fa22be = 0;
  *(uint8_t*)0x20fa22bf = 0;
  *(uint8_t*)0x20fa22c0 = 0;
  *(uint8_t*)0x20fa22c1 = 0;
  *(uint8_t*)0x20fa22c2 = 0;
  *(uint8_t*)0x20fa22c3 = 0;
  *(uint8_t*)0x20fa22c4 = 0;
  *(uint8_t*)0x20fa22c5 = 0;
  *(uint8_t*)0x20fa22c6 = 0;
  *(uint8_t*)0x20fa22c7 = 0;
  *(uint8_t*)0x20fa22c8 = 0;
  *(uint8_t*)0x20fa22c9 = 0;
  *(uint8_t*)0x20fa22ca = 0;
  *(uint8_t*)0x20fa22cb = 0;
  *(uint8_t*)0x20fa22cc = 0;
  *(uint8_t*)0x20fa22cd = 0;
  *(uint8_t*)0x20fa22ce = 0;
  *(uint8_t*)0x20fa22cf = 0;
  *(uint8_t*)0x20fa22d0 = 0;
  *(uint8_t*)0x20fa22d1 = 0;
  *(uint8_t*)0x20fa22d2 = 0;
  *(uint8_t*)0x20fa22d3 = 0;
  *(uint8_t*)0x20fa22d4 = 0;
  *(uint8_t*)0x20fa22d5 = 0;
  *(uint8_t*)0x20fa22d6 = 0;
  *(uint8_t*)0x20fa22d7 = 0;
  *(uint8_t*)0x20fa22d8 = 0;
  *(uint8_t*)0x20fa22d9 = 0;
  *(uint8_t*)0x20fa22da = 0;
  *(uint8_t*)0x20fa22db = 0;
  *(uint8_t*)0x20fa22dc = 0;
  *(uint8_t*)0x20fa22dd = 0;
  *(uint8_t*)0x20fa22de = 0;
  *(uint8_t*)0x20fa22df = 0;
  *(uint8_t*)0x20fa22e0 = 0;
  *(uint8_t*)0x20fa22e1 = 0;
  *(uint8_t*)0x20fa22e2 = 0;
  *(uint8_t*)0x20fa22e3 = 0;
  *(uint8_t*)0x20fa22e4 = 0;
  *(uint8_t*)0x20fa22e5 = 0;
  *(uint8_t*)0x20fa22e6 = 0;
  *(uint8_t*)0x20fa22e7 = 0;
  *(uint8_t*)0x20fa22e8 = 0;
  *(uint8_t*)0x20fa22e9 = 0;
  *(uint8_t*)0x20fa22ea = 0;
  *(uint8_t*)0x20fa22eb = 0;
  *(uint8_t*)0x20fa22ec = 0;
  *(uint8_t*)0x20fa22ed = 0;
  *(uint8_t*)0x20fa22ee = 0;
  *(uint8_t*)0x20fa22ef = 0;
  *(uint8_t*)0x20fa22f0 = 0;
  *(uint8_t*)0x20fa22f1 = 0;
  *(uint8_t*)0x20fa22f2 = 0;
  *(uint8_t*)0x20fa22f3 = 0;
  *(uint8_t*)0x20fa22f4 = 0;
  *(uint8_t*)0x20fa22f5 = 0;
  *(uint8_t*)0x20fa22f6 = 0;
  *(uint8_t*)0x20fa22f7 = 0;
  *(uint8_t*)0x20fa22f8 = 0;
  *(uint8_t*)0x20fa22f9 = 0;
  *(uint8_t*)0x20fa22fa = 0;
  *(uint8_t*)0x20fa22fb = 0;
  *(uint8_t*)0x20fa22fc = 0;
  *(uint8_t*)0x20fa22fd = 0;
  *(uint8_t*)0x20fa22fe = 0;
  *(uint8_t*)0x20fa22ff = 0;
  *(uint8_t*)0x20fa2300 = 0;
  *(uint8_t*)0x20fa2301 = 0;
  *(uint8_t*)0x20fa2302 = 0;
  *(uint8_t*)0x20fa2303 = 0;
  *(uint8_t*)0x20fa2304 = 0;
  *(uint8_t*)0x20fa2305 = 0;
  *(uint8_t*)0x20fa2306 = 0;
  *(uint8_t*)0x20fa2307 = 0;
  *(uint8_t*)0x20fa2308 = 0;
  *(uint8_t*)0x20fa2309 = 0;
  *(uint8_t*)0x20fa230a = 0;
  *(uint8_t*)0x20fa230b = 0;
  *(uint8_t*)0x20fa230c = 0;
  *(uint8_t*)0x20fa230d = 0;
  *(uint8_t*)0x20fa230e = 0;
  *(uint8_t*)0x20fa230f = 0;
  *(uint32_t*)0x20fa2310 = 0;
  *(uint16_t*)0x20fa2314 = 0xa8;
  *(uint16_t*)0x20fa2316 = 0xf0;
  *(uint32_t*)0x20fa2318 = 0;
  *(uint64_t*)0x20fa2320 = 0;
  *(uint64_t*)0x20fa2328 = 0;
  *(uint16_t*)0x20fa2330 = 0x48;
  memcpy((void*)0x20fa2332, "\x52\x45\x44\x49\x52\x45\x43\x54\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20fa234f = 0;
  *(uint32_t*)0x20fa2350 = 0;
  *(uint64_t*)0x20fa2354 = htobe64(0);
  *(uint64_t*)0x20fa235c = htobe64(1);
  *(uint32_t*)0x20fa2364 = htobe32(0);
  *(uint16_t*)0x20fa2374 = 0;
  *(uint16_t*)0x20fa2376 = 0;
  *(uint8_t*)0x20fa2378 = 0;
  *(uint8_t*)0x20fa2379 = 0;
  *(uint8_t*)0x20fa237a = 0;
  *(uint8_t*)0x20fa237b = 0;
  *(uint8_t*)0x20fa237c = 0;
  *(uint8_t*)0x20fa237d = 0;
  *(uint8_t*)0x20fa237e = 0;
  *(uint8_t*)0x20fa237f = 0;
  *(uint8_t*)0x20fa2380 = 0;
  *(uint8_t*)0x20fa2381 = 0;
  *(uint8_t*)0x20fa2382 = 0;
  *(uint8_t*)0x20fa2383 = 0;
  *(uint8_t*)0x20fa2384 = 0;
  *(uint8_t*)0x20fa2385 = 0;
  *(uint8_t*)0x20fa2386 = 0;
  *(uint8_t*)0x20fa2387 = 0;
  *(uint8_t*)0x20fa2388 = 0;
  *(uint8_t*)0x20fa2389 = 0;
  *(uint8_t*)0x20fa238a = 0;
  *(uint8_t*)0x20fa238b = 0;
  *(uint8_t*)0x20fa238c = 0;
  *(uint8_t*)0x20fa238d = 0;
  *(uint8_t*)0x20fa238e = 0;
  *(uint8_t*)0x20fa238f = 0;
  *(uint8_t*)0x20fa2390 = 0;
  *(uint8_t*)0x20fa2391 = 0;
  *(uint8_t*)0x20fa2392 = 0;
  *(uint8_t*)0x20fa2393 = 0;
  *(uint8_t*)0x20fa2394 = 0;
  *(uint8_t*)0x20fa2395 = 0;
  *(uint8_t*)0x20fa2396 = 0;
  *(uint8_t*)0x20fa2397 = 0;
  *(uint8_t*)0x20fa2398 = 0;
  *(uint8_t*)0x20fa2399 = 0;
  *(uint8_t*)0x20fa239a = 0;
  *(uint8_t*)0x20fa239b = 0;
  *(uint8_t*)0x20fa239c = 0;
  *(uint8_t*)0x20fa239d = 0;
  *(uint8_t*)0x20fa239e = 0;
  *(uint8_t*)0x20fa239f = 0;
  *(uint8_t*)0x20fa23a0 = 0;
  *(uint8_t*)0x20fa23a1 = 0;
  *(uint8_t*)0x20fa23a2 = 0;
  *(uint8_t*)0x20fa23a3 = 0;
  *(uint8_t*)0x20fa23a4 = 0;
  *(uint8_t*)0x20fa23a5 = 0;
  *(uint8_t*)0x20fa23a6 = 0;
  *(uint8_t*)0x20fa23a7 = 0;
  *(uint8_t*)0x20fa23a8 = 0;
  *(uint8_t*)0x20fa23a9 = 0;
  *(uint8_t*)0x20fa23aa = 0;
  *(uint8_t*)0x20fa23ab = 0;
  *(uint8_t*)0x20fa23ac = 0;
  *(uint8_t*)0x20fa23ad = 0;
  *(uint8_t*)0x20fa23ae = 0;
  *(uint8_t*)0x20fa23af = 0;
  *(uint8_t*)0x20fa23b0 = 0;
  *(uint8_t*)0x20fa23b1 = 0;
  *(uint8_t*)0x20fa23b2 = 0;
  *(uint8_t*)0x20fa23b3 = 0;
  *(uint8_t*)0x20fa23b4 = 0;
  *(uint8_t*)0x20fa23b5 = 0;
  *(uint8_t*)0x20fa23b6 = 0;
  *(uint8_t*)0x20fa23b7 = 0;
  *(uint8_t*)0x20fa23b8 = 0;
  *(uint8_t*)0x20fa23b9 = 0;
  *(uint8_t*)0x20fa23ba = 0;
  *(uint8_t*)0x20fa23bb = 0;
  *(uint8_t*)0x20fa23bc = 0;
  *(uint8_t*)0x20fa23bd = 0;
  *(uint8_t*)0x20fa23be = 0;
  *(uint8_t*)0x20fa23bf = 0;
  *(uint8_t*)0x20fa23c0 = 0;
  *(uint8_t*)0x20fa23c1 = 0;
  *(uint8_t*)0x20fa23c2 = 0;
  *(uint8_t*)0x20fa23c3 = 0;
  *(uint8_t*)0x20fa23c4 = 0;
  *(uint8_t*)0x20fa23c5 = 0;
  *(uint8_t*)0x20fa23c6 = 0;
  *(uint8_t*)0x20fa23c7 = 0;
  *(uint8_t*)0x20fa23c8 = 0;
  *(uint8_t*)0x20fa23c9 = 0;
  *(uint8_t*)0x20fa23ca = 0;
  *(uint8_t*)0x20fa23cb = 0;
  *(uint8_t*)0x20fa23cc = 0;
  *(uint8_t*)0x20fa23cd = 0;
  *(uint8_t*)0x20fa23ce = 0;
  *(uint8_t*)0x20fa23cf = 0;
  *(uint8_t*)0x20fa23d0 = 0;
  *(uint8_t*)0x20fa23d1 = 0;
  *(uint8_t*)0x20fa23d2 = 0;
  *(uint8_t*)0x20fa23d3 = 0;
  *(uint8_t*)0x20fa23d4 = 0;
  *(uint8_t*)0x20fa23d5 = 0;
  *(uint8_t*)0x20fa23d6 = 0;
  *(uint8_t*)0x20fa23d7 = 0;
  *(uint8_t*)0x20fa23d8 = 0;
  *(uint8_t*)0x20fa23d9 = 0;
  *(uint8_t*)0x20fa23da = 0;
  *(uint8_t*)0x20fa23db = 0;
  *(uint8_t*)0x20fa23dc = 0;
  *(uint8_t*)0x20fa23dd = 0;
  *(uint8_t*)0x20fa23de = 0;
  *(uint8_t*)0x20fa23df = 0;
  *(uint8_t*)0x20fa23e0 = 0;
  *(uint8_t*)0x20fa23e1 = 0;
  *(uint8_t*)0x20fa23e2 = 0;
  *(uint8_t*)0x20fa23e3 = 0;
  *(uint8_t*)0x20fa23e4 = 0;
  *(uint8_t*)0x20fa23e5 = 0;
  *(uint8_t*)0x20fa23e6 = 0;
  *(uint8_t*)0x20fa23e7 = 0;
  *(uint8_t*)0x20fa23e8 = 0;
  *(uint8_t*)0x20fa23e9 = 0;
  *(uint8_t*)0x20fa23ea = 0;
  *(uint8_t*)0x20fa23eb = 0;
  *(uint8_t*)0x20fa23ec = 0;
  *(uint8_t*)0x20fa23ed = 0;
  *(uint8_t*)0x20fa23ee = 0;
  *(uint8_t*)0x20fa23ef = 0;
  *(uint8_t*)0x20fa23f0 = 0;
  *(uint8_t*)0x20fa23f1 = 0;
  *(uint8_t*)0x20fa23f2 = 0;
  *(uint8_t*)0x20fa23f3 = 0;
  *(uint8_t*)0x20fa23f4 = 0;
  *(uint8_t*)0x20fa23f5 = 0;
  *(uint8_t*)0x20fa23f6 = 0;
  *(uint8_t*)0x20fa23f7 = 0;
  *(uint8_t*)0x20fa23f8 = 0;
  *(uint8_t*)0x20fa23f9 = 0;
  *(uint8_t*)0x20fa23fa = 0;
  *(uint8_t*)0x20fa23fb = 0;
  *(uint8_t*)0x20fa23fc = 0;
  *(uint8_t*)0x20fa23fd = 0;
  *(uint8_t*)0x20fa23fe = 0;
  *(uint8_t*)0x20fa23ff = 0;
  *(uint32_t*)0x20fa2400 = 0;
  *(uint16_t*)0x20fa2404 = 0xa8;
  *(uint16_t*)0x20fa2406 = 0xf0;
  *(uint32_t*)0x20fa2408 = 0;
  *(uint64_t*)0x20fa2410 = 0;
  *(uint64_t*)0x20fa2418 = 0;
  *(uint16_t*)0x20fa2420 = 0x48;
  memcpy((void*)0x20fa2422, "\x4e\x45\x54\x4d\x41\x50\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20fa243f = 0;
  *(uint32_t*)0x20fa2440 = 1;
  *(uint32_t*)0x20fa2444 = htobe32(-1);
  *(uint8_t*)0x20fa2454 = 0xfe;
  *(uint8_t*)0x20fa2455 = 0x80;
  *(uint8_t*)0x20fa2456 = 0;
  *(uint8_t*)0x20fa2457 = 0;
  *(uint8_t*)0x20fa2458 = 0;
  *(uint8_t*)0x20fa2459 = 0;
  *(uint8_t*)0x20fa245a = 0;
  *(uint8_t*)0x20fa245b = 0;
  *(uint8_t*)0x20fa245c = 0;
  *(uint8_t*)0x20fa245d = 0;
  *(uint8_t*)0x20fa245e = 0;
  *(uint8_t*)0x20fa245f = 0;
  *(uint8_t*)0x20fa2460 = 0;
  *(uint8_t*)0x20fa2461 = 0;
  *(uint8_t*)0x20fa2462 = 0;
  *(uint8_t*)0x20fa2463 = 0xbb;
  *(uint16_t*)0x20fa2464 = 0;
  *(uint16_t*)0x20fa2466 = 0;
  *(uint8_t*)0x20fa2468 = 0;
  *(uint8_t*)0x20fa2469 = 0;
  *(uint8_t*)0x20fa246a = 0;
  *(uint8_t*)0x20fa246b = 0;
  *(uint8_t*)0x20fa246c = 0;
  *(uint8_t*)0x20fa246d = 0;
  *(uint8_t*)0x20fa246e = 0;
  *(uint8_t*)0x20fa246f = 0;
  *(uint8_t*)0x20fa2470 = 0;
  *(uint8_t*)0x20fa2471 = 0;
  *(uint8_t*)0x20fa2472 = 0;
  *(uint8_t*)0x20fa2473 = 0;
  *(uint8_t*)0x20fa2474 = 0;
  *(uint8_t*)0x20fa2475 = 0;
  *(uint8_t*)0x20fa2476 = 0;
  *(uint8_t*)0x20fa2477 = 0;
  *(uint8_t*)0x20fa2478 = 0;
  *(uint8_t*)0x20fa2479 = 0;
  *(uint8_t*)0x20fa247a = 0;
  *(uint8_t*)0x20fa247b = 0;
  *(uint8_t*)0x20fa247c = 0;
  *(uint8_t*)0x20fa247d = 0;
  *(uint8_t*)0x20fa247e = 0;
  *(uint8_t*)0x20fa247f = 0;
  *(uint8_t*)0x20fa2480 = 0;
  *(uint8_t*)0x20fa2481 = 0;
  *(uint8_t*)0x20fa2482 = 0;
  *(uint8_t*)0x20fa2483 = 0;
  *(uint8_t*)0x20fa2484 = 0;
  *(uint8_t*)0x20fa2485 = 0;
  *(uint8_t*)0x20fa2486 = 0;
  *(uint8_t*)0x20fa2487 = 0;
  *(uint8_t*)0x20fa2488 = 0;
  *(uint8_t*)0x20fa2489 = 0;
  *(uint8_t*)0x20fa248a = 0;
  *(uint8_t*)0x20fa248b = 0;
  *(uint8_t*)0x20fa248c = 0;
  *(uint8_t*)0x20fa248d = 0;
  *(uint8_t*)0x20fa248e = 0;
  *(uint8_t*)0x20fa248f = 0;
  *(uint8_t*)0x20fa2490 = 0;
  *(uint8_t*)0x20fa2491 = 0;
  *(uint8_t*)0x20fa2492 = 0;
  *(uint8_t*)0x20fa2493 = 0;
  *(uint8_t*)0x20fa2494 = 0;
  *(uint8_t*)0x20fa2495 = 0;
  *(uint8_t*)0x20fa2496 = 0;
  *(uint8_t*)0x20fa2497 = 0;
  *(uint8_t*)0x20fa2498 = 0;
  *(uint8_t*)0x20fa2499 = 0;
  *(uint8_t*)0x20fa249a = 0;
  *(uint8_t*)0x20fa249b = 0;
  *(uint8_t*)0x20fa249c = 0;
  *(uint8_t*)0x20fa249d = 0;
  *(uint8_t*)0x20fa249e = 0;
  *(uint8_t*)0x20fa249f = 0;
  *(uint8_t*)0x20fa24a0 = 0;
  *(uint8_t*)0x20fa24a1 = 0;
  *(uint8_t*)0x20fa24a2 = 0;
  *(uint8_t*)0x20fa24a3 = 0;
  *(uint8_t*)0x20fa24a4 = 0;
  *(uint8_t*)0x20fa24a5 = 0;
  *(uint8_t*)0x20fa24a6 = 0;
  *(uint8_t*)0x20fa24a7 = 0;
  *(uint8_t*)0x20fa24a8 = 0;
  *(uint8_t*)0x20fa24a9 = 0;
  *(uint8_t*)0x20fa24aa = 0;
  *(uint8_t*)0x20fa24ab = 0;
  *(uint8_t*)0x20fa24ac = 0;
  *(uint8_t*)0x20fa24ad = 0;
  *(uint8_t*)0x20fa24ae = 0;
  *(uint8_t*)0x20fa24af = 0;
  *(uint8_t*)0x20fa24b0 = 0;
  *(uint8_t*)0x20fa24b1 = 0;
  *(uint8_t*)0x20fa24b2 = 0;
  *(uint8_t*)0x20fa24b3 = 0;
  *(uint8_t*)0x20fa24b4 = 0;
  *(uint8_t*)0x20fa24b5 = 0;
  *(uint8_t*)0x20fa24b6 = 0;
  *(uint8_t*)0x20fa24b7 = 0;
  *(uint8_t*)0x20fa24b8 = 0;
  *(uint8_t*)0x20fa24b9 = 0;
  *(uint8_t*)0x20fa24ba = 0;
  *(uint8_t*)0x20fa24bb = 0;
  *(uint8_t*)0x20fa24bc = 0;
  *(uint8_t*)0x20fa24bd = 0;
  *(uint8_t*)0x20fa24be = 0;
  *(uint8_t*)0x20fa24bf = 0;
  *(uint8_t*)0x20fa24c0 = 0;
  *(uint8_t*)0x20fa24c1 = 0;
  *(uint8_t*)0x20fa24c2 = 0;
  *(uint8_t*)0x20fa24c3 = 0;
  *(uint8_t*)0x20fa24c4 = 0;
  *(uint8_t*)0x20fa24c5 = 0;
  *(uint8_t*)0x20fa24c6 = 0;
  *(uint8_t*)0x20fa24c7 = 0;
  *(uint8_t*)0x20fa24c8 = 0;
  *(uint8_t*)0x20fa24c9 = 0;
  *(uint8_t*)0x20fa24ca = 0;
  *(uint8_t*)0x20fa24cb = 0;
  *(uint8_t*)0x20fa24cc = 0;
  *(uint8_t*)0x20fa24cd = 0;
  *(uint8_t*)0x20fa24ce = 0;
  *(uint8_t*)0x20fa24cf = 0;
  *(uint8_t*)0x20fa24d0 = 0;
  *(uint8_t*)0x20fa24d1 = 0;
  *(uint8_t*)0x20fa24d2 = 0;
  *(uint8_t*)0x20fa24d3 = 0;
  *(uint8_t*)0x20fa24d4 = 0;
  *(uint8_t*)0x20fa24d5 = 0;
  *(uint8_t*)0x20fa24d6 = 0;
  *(uint8_t*)0x20fa24d7 = 0;
  *(uint8_t*)0x20fa24d8 = 0;
  *(uint8_t*)0x20fa24d9 = 0;
  *(uint8_t*)0x20fa24da = 0;
  *(uint8_t*)0x20fa24db = 0;
  *(uint8_t*)0x20fa24dc = 0;
  *(uint8_t*)0x20fa24dd = 0;
  *(uint8_t*)0x20fa24de = 0;
  *(uint8_t*)0x20fa24df = 0;
  *(uint8_t*)0x20fa24e0 = 0;
  *(uint8_t*)0x20fa24e1 = 0;
  *(uint8_t*)0x20fa24e2 = 0;
  *(uint8_t*)0x20fa24e3 = 0;
  *(uint8_t*)0x20fa24e4 = 0;
  *(uint8_t*)0x20fa24e5 = 0;
  *(uint8_t*)0x20fa24e6 = 0;
  *(uint8_t*)0x20fa24e7 = 0;
  *(uint8_t*)0x20fa24e8 = 0;
  *(uint8_t*)0x20fa24e9 = 0;
  *(uint8_t*)0x20fa24ea = 0;
  *(uint8_t*)0x20fa24eb = 0;
  *(uint8_t*)0x20fa24ec = 0;
  *(uint8_t*)0x20fa24ed = 0;
  *(uint8_t*)0x20fa24ee = 0;
  *(uint8_t*)0x20fa24ef = 0;
  *(uint32_t*)0x20fa24f0 = 0;
  *(uint16_t*)0x20fa24f4 = 0xa8;
  *(uint16_t*)0x20fa24f6 = 0xd0;
  *(uint32_t*)0x20fa24f8 = 0;
  *(uint64_t*)0x20fa2500 = 0;
  *(uint64_t*)0x20fa2508 = 0;
  *(uint16_t*)0x20fa2510 = 0x28;
  memcpy((void*)0x20fa2512, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20fa252f = 0;
  *(uint32_t*)0x20fa2530 = 0xfffffffe;
  syscall(__NR_setsockopt, r[1], 0x29, 0x40, 0x20fa2000, 0x538);
  *(uint8_t*)0x20005f25 = 0xaa;
  *(uint8_t*)0x20005f26 = 0xaa;
  *(uint8_t*)0x20005f27 = 0xaa;
  *(uint8_t*)0x20005f28 = 0xaa;
  *(uint8_t*)0x20005f29 = 0;
  *(uint8_t*)0x20005f2a = 0xaa;
  *(uint8_t*)0x20005f2b = 1;
  *(uint8_t*)0x20005f2c = 0x80;
  *(uint8_t*)0x20005f2d = 0xc2;
  *(uint8_t*)0x20005f2e = 0;
  *(uint8_t*)0x20005f2f = 0;
  *(uint8_t*)0x20005f30 = 0;
  *(uint16_t*)0x20005f31 = htobe16(0x86dd);
  STORE_BY_BITMASK(uint8_t, 0x20005f33, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x20005f33, 6, 4, 4);
  memcpy((void*)0x20005f34, "\x01\x1f\x20", 3);
  *(uint16_t*)0x20005f37 = htobe16(8);
  *(uint8_t*)0x20005f39 = 0x11;
  *(uint8_t*)0x20005f3a = 0;
  *(uint8_t*)0x20005f3b = 0xfe;
  *(uint8_t*)0x20005f3c = 0x80;
  *(uint8_t*)0x20005f3d = 0;
  *(uint8_t*)0x20005f3e = 0;
  *(uint8_t*)0x20005f3f = 0;
  *(uint8_t*)0x20005f40 = 0;
  *(uint8_t*)0x20005f41 = 0;
  *(uint8_t*)0x20005f42 = 0;
  *(uint8_t*)0x20005f43 = 0;
  *(uint8_t*)0x20005f44 = 0;
  *(uint8_t*)0x20005f45 = 0;
  *(uint8_t*)0x20005f46 = 0;
  *(uint8_t*)0x20005f47 = 0;
  *(uint8_t*)0x20005f48 = 0;
  *(uint8_t*)0x20005f49 = 0;
  *(uint8_t*)0x20005f4a = 0xbb;
  *(uint8_t*)0x20005f4b = -1;
  *(uint8_t*)0x20005f4c = 2;
  *(uint8_t*)0x20005f4d = 0;
  *(uint8_t*)0x20005f4e = 0;
  *(uint8_t*)0x20005f4f = 0;
  *(uint8_t*)0x20005f50 = 0;
  *(uint8_t*)0x20005f51 = 0;
  *(uint8_t*)0x20005f52 = 0;
  *(uint8_t*)0x20005f53 = 0;
  *(uint8_t*)0x20005f54 = 0;
  *(uint8_t*)0x20005f55 = 0;
  *(uint8_t*)0x20005f56 = 0;
  *(uint8_t*)0x20005f57 = 0;
  *(uint8_t*)0x20005f58 = 0;
  *(uint8_t*)0x20005f59 = 0;
  *(uint8_t*)0x20005f5a = 1;
  *(uint8_t*)0x20005f5b = 0x81;
  *(uint8_t*)0x20005f5c = 0;
  *(uint16_t*)0x20005f5d = 0;
  *(uint16_t*)0x20005f5f = htobe16(8);
  *(uint16_t*)0x20005f61 = htobe16(0);
  *(uint32_t*)0x20775000 = 0;
  *(uint32_t*)0x20775004 = 0;
  struct csum_inet csum_1;
  csum_inet_init(&csum_1);
  csum_inet_update(&csum_1, (const uint8_t*)0x20005f3b, 16);
  csum_inet_update(&csum_1, (const uint8_t*)0x20005f4b, 16);
  uint32_t csum_1_chunk_2 = 0x8000000;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_2, 4);
  uint32_t csum_1_chunk_3 = 0x3a000000;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_3, 4);
  csum_inet_update(&csum_1, (const uint8_t*)0x20005f5b, 8);
  *(uint16_t*)0x20005f5d = csum_inet_digest(&csum_1);
  syz_emit_ethernet(0x3e, 0x20005f25, 0x20775000);
  *(uint64_t*)0x20009fc8 = 0x20000000;
  *(uint32_t*)0x20009fd0 = 0;
  *(uint64_t*)0x20009fd8 = 0x2001d000;
  *(uint64_t*)0x20009fe0 = 0;
  *(uint64_t*)0x20009fe8 = 0x20011000;
  *(uint64_t*)0x20009ff0 = 0;
  *(uint32_t*)0x20009ff8 = 0;
  syscall(__NR_sendmsg, r[0], 0x20009fc8, 0xc100);
  *(uint64_t*)0x209f2fc8 = 0x20000000;
  *(uint32_t*)0x209f2fd0 = 0;
  *(uint64_t*)0x209f2fd8 = 0x207f4fe0;
  *(uint64_t*)0x209f2fe0 = 1;
  *(uint64_t*)0x209f2fe8 = 0x2016ff78;
  *(uint64_t*)0x209f2ff0 = 0;
  *(uint32_t*)0x209f2ff8 = 0;
  *(uint64_t*)0x207f4fe0 = 0x20304000;
  *(uint64_t*)0x207f4fe8 = 0x1c;
  memcpy((void*)0x20304000, "\x9f\xaa\x7e\x40\x28\x29\x83\xaf\x5a\xde\xfd\xaf"
                            "\x79\xe2\x1b\x2a\xf3\xe6\xe7\x48\xde\x8a\x1b\x6b"
                            "\x08\x0b\x88\xac",
         28);
  syscall(__NR_sendmsg, r[0], 0x209f2fc8, 0);
}

int main()
{
  setup_tun(0, true);
  loop();
  return 0;
}
