// https://syzkaller.appspot.com/bug?id=14177cd007e876c70445b10912cfab974e43657c
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
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  *(uint8_t*)0x207a1000 = 0xaa;
  *(uint8_t*)0x207a1001 = 0xaa;
  *(uint8_t*)0x207a1002 = 0xaa;
  *(uint8_t*)0x207a1003 = 0xaa;
  *(uint8_t*)0x207a1004 = 0;
  *(uint8_t*)0x207a1005 = 0xaa;
  *(uint8_t*)0x207a1006 = 0xaa;
  *(uint8_t*)0x207a1007 = 0xaa;
  *(uint8_t*)0x207a1008 = 0xaa;
  *(uint8_t*)0x207a1009 = 0xaa;
  *(uint8_t*)0x207a100a = 0;
  *(uint8_t*)0x207a100b = 0;
  *(uint16_t*)0x207a100c = htobe16(0x86dd);
  STORE_BY_BITMASK(uint8_t, 0x207a100e, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x207a100e, 6, 4, 4);
  memcpy((void*)0x207a100f, "\x01\xf7\xe3", 3);
  *(uint16_t*)0x207a1012 = htobe16(0x4d);
  *(uint8_t*)0x207a1014 = 6;
  *(uint8_t*)0x207a1015 = 0;
  *(uint8_t*)0x207a1016 = 0xfe;
  *(uint8_t*)0x207a1017 = 0x80;
  *(uint8_t*)0x207a1018 = 0;
  *(uint8_t*)0x207a1019 = 0;
  *(uint8_t*)0x207a101a = 0;
  *(uint8_t*)0x207a101b = 0;
  *(uint8_t*)0x207a101c = 0;
  *(uint8_t*)0x207a101d = 0;
  *(uint8_t*)0x207a101e = 0;
  *(uint8_t*)0x207a101f = 0;
  *(uint8_t*)0x207a1020 = 0;
  *(uint8_t*)0x207a1021 = 0;
  *(uint8_t*)0x207a1022 = 0;
  *(uint8_t*)0x207a1023 = 0;
  *(uint8_t*)0x207a1024 = 0;
  *(uint8_t*)0x207a1025 = 0xbb;
  *(uint8_t*)0x207a1026 = -1;
  *(uint8_t*)0x207a1027 = 2;
  *(uint8_t*)0x207a1028 = 0;
  *(uint8_t*)0x207a1029 = 0;
  *(uint8_t*)0x207a102a = 0;
  *(uint8_t*)0x207a102b = 0;
  *(uint8_t*)0x207a102c = 0;
  *(uint8_t*)0x207a102d = 0;
  *(uint8_t*)0x207a102e = 0;
  *(uint8_t*)0x207a102f = 0;
  *(uint8_t*)0x207a1030 = 0;
  *(uint8_t*)0x207a1031 = 0;
  *(uint8_t*)0x207a1032 = 0;
  *(uint8_t*)0x207a1033 = 0;
  *(uint8_t*)0x207a1034 = 0;
  *(uint8_t*)0x207a1035 = 1;
  *(uint16_t*)0x207a1036 = 0;
  *(uint16_t*)0x207a1038 = htobe16(0x4e22);
  *(uint8_t*)0x207a103a = 4;
  STORE_BY_BITMASK(uint8_t, 0x207a103b, 1, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x207a103b, 0, 4, 4);
  *(uint16_t*)0x207a103c = 0;
  STORE_BY_BITMASK(uint8_t, 0x207a103e, 0, 0, 1);
  STORE_BY_BITMASK(uint8_t, 0x207a103e, 0, 1, 4);
  STORE_BY_BITMASK(uint8_t, 0x207a103e, 0, 5, 3);
  memcpy((void*)0x207a103f, "\x7a\x15\xcb", 3);
  *(uint8_t*)0x207a1042 = 0xfb;
  memcpy((void*)0x207a1043, "\xa9\xa8\xbd", 3);
  memcpy((void*)0x207a1046,
         "\xbf\xc1\xa2\x22\xe1\x39\x6f\x74\xa3\x96\x5a\x48\x4e\x14\xab\x23\x36"
         "\x9e\xe2\xcb\x5e\xe7\x76\x7a\x97\xba\x97\x72\xee\xd9\xa4\x77\x9d\xa0"
         "\xce\xf5\x20\x31\xf8\x20\x98\x45\xed\x6b\x2a\xda\x35\xfa\x39\xaf\x3f"
         "\x76\xac\x13\x95\xc3\xe1\x48\xfd\x95\x8c",
         61);
  struct csum_inet csum_1;
  csum_inet_init(&csum_1);
  csum_inet_update(&csum_1, (const uint8_t*)0x207a1016, 16);
  csum_inet_update(&csum_1, (const uint8_t*)0x207a1026, 16);
  uint32_t csum_1_chunk_2 = 0x10000000;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_2, 4);
  uint32_t csum_1_chunk_3 = 0x21000000;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_3, 4);
  csum_inet_update(&csum_1, (const uint8_t*)0x207a1036, 16);
  *(uint16_t*)0x207a103c = csum_inet_digest(&csum_1);
  syz_emit_ethernet(0x83, 0x207a1000, 0x20000000);
  r[0] = syscall(__NR_socket, 0xa, 3, 0xd);
  memcpy((void*)0x20001000, "\x73\x65\x63\x75\x72\x69\x74\x79\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20001020 = 0xe;
  *(uint32_t*)0x20001024 = 4;
  *(uint32_t*)0x20001028 = 0x378;
  *(uint32_t*)0x2000102c = -1;
  *(uint32_t*)0x20001030 = 0xd0;
  *(uint32_t*)0x20001034 = 0;
  *(uint32_t*)0x20001038 = 0;
  *(uint32_t*)0x2000103c = -1;
  *(uint32_t*)0x20001040 = -1;
  *(uint32_t*)0x20001044 = 0;
  *(uint32_t*)0x20001048 = 0;
  *(uint32_t*)0x2000104c = 0;
  *(uint32_t*)0x20001050 = -1;
  *(uint32_t*)0x20001054 = 4;
  *(uint64_t*)0x20001058 = 0x20000fc0;
  *(uint8_t*)0x20001060 = 0;
  *(uint8_t*)0x20001061 = 0;
  *(uint8_t*)0x20001062 = 0;
  *(uint8_t*)0x20001063 = 0;
  *(uint8_t*)0x20001064 = 0;
  *(uint8_t*)0x20001065 = 0;
  *(uint8_t*)0x20001066 = 0;
  *(uint8_t*)0x20001067 = 0;
  *(uint8_t*)0x20001068 = 0;
  *(uint8_t*)0x20001069 = 0;
  *(uint8_t*)0x2000106a = 0;
  *(uint8_t*)0x2000106b = 0;
  *(uint8_t*)0x2000106c = 0;
  *(uint8_t*)0x2000106d = 0;
  *(uint8_t*)0x2000106e = 0;
  *(uint8_t*)0x2000106f = 0;
  *(uint8_t*)0x20001070 = 0;
  *(uint8_t*)0x20001071 = 0;
  *(uint8_t*)0x20001072 = 0;
  *(uint8_t*)0x20001073 = 0;
  *(uint8_t*)0x20001074 = 0;
  *(uint8_t*)0x20001075 = 0;
  *(uint8_t*)0x20001076 = 0;
  *(uint8_t*)0x20001077 = 0;
  *(uint8_t*)0x20001078 = 0;
  *(uint8_t*)0x20001079 = 0;
  *(uint8_t*)0x2000107a = 0;
  *(uint8_t*)0x2000107b = 0;
  *(uint8_t*)0x2000107c = 0;
  *(uint8_t*)0x2000107d = 0;
  *(uint8_t*)0x2000107e = 0;
  *(uint8_t*)0x2000107f = 0;
  *(uint8_t*)0x20001080 = 0;
  *(uint8_t*)0x20001081 = 0;
  *(uint8_t*)0x20001082 = 0;
  *(uint8_t*)0x20001083 = 0;
  *(uint8_t*)0x20001084 = 0;
  *(uint8_t*)0x20001085 = 0;
  *(uint8_t*)0x20001086 = 0;
  *(uint8_t*)0x20001087 = 0;
  *(uint8_t*)0x20001088 = 0;
  *(uint8_t*)0x20001089 = 0;
  *(uint8_t*)0x2000108a = 0;
  *(uint8_t*)0x2000108b = 0;
  *(uint8_t*)0x2000108c = 0;
  *(uint8_t*)0x2000108d = 0;
  *(uint8_t*)0x2000108e = 0;
  *(uint8_t*)0x2000108f = 0;
  *(uint8_t*)0x20001090 = 0;
  *(uint8_t*)0x20001091 = 0;
  *(uint8_t*)0x20001092 = 0;
  *(uint8_t*)0x20001093 = 0;
  *(uint8_t*)0x20001094 = 0;
  *(uint8_t*)0x20001095 = 0;
  *(uint8_t*)0x20001096 = 0;
  *(uint8_t*)0x20001097 = 0;
  *(uint8_t*)0x20001098 = 0;
  *(uint8_t*)0x20001099 = 0;
  *(uint8_t*)0x2000109a = 0;
  *(uint8_t*)0x2000109b = 0;
  *(uint8_t*)0x2000109c = 0;
  *(uint8_t*)0x2000109d = 0;
  *(uint8_t*)0x2000109e = 0;
  *(uint8_t*)0x2000109f = 0;
  *(uint8_t*)0x200010a0 = 0;
  *(uint8_t*)0x200010a1 = 0;
  *(uint8_t*)0x200010a2 = 0;
  *(uint8_t*)0x200010a3 = 0;
  *(uint8_t*)0x200010a4 = 0;
  *(uint8_t*)0x200010a5 = 0;
  *(uint8_t*)0x200010a6 = 0;
  *(uint8_t*)0x200010a7 = 0;
  *(uint8_t*)0x200010a8 = 0;
  *(uint8_t*)0x200010a9 = 0;
  *(uint8_t*)0x200010aa = 0;
  *(uint8_t*)0x200010ab = 0;
  *(uint8_t*)0x200010ac = 0;
  *(uint8_t*)0x200010ad = 0;
  *(uint8_t*)0x200010ae = 0;
  *(uint8_t*)0x200010af = 0;
  *(uint8_t*)0x200010b0 = 0;
  *(uint8_t*)0x200010b1 = 0;
  *(uint8_t*)0x200010b2 = 0;
  *(uint8_t*)0x200010b3 = 0;
  *(uint8_t*)0x200010b4 = 0;
  *(uint8_t*)0x200010b5 = 0;
  *(uint8_t*)0x200010b6 = 0;
  *(uint8_t*)0x200010b7 = 0;
  *(uint8_t*)0x200010b8 = 0;
  *(uint8_t*)0x200010b9 = 0;
  *(uint8_t*)0x200010ba = 0;
  *(uint8_t*)0x200010bb = 0;
  *(uint8_t*)0x200010bc = 0;
  *(uint8_t*)0x200010bd = 0;
  *(uint8_t*)0x200010be = 0;
  *(uint8_t*)0x200010bf = 0;
  *(uint8_t*)0x200010c0 = 0;
  *(uint8_t*)0x200010c1 = 0;
  *(uint8_t*)0x200010c2 = 0;
  *(uint8_t*)0x200010c3 = 0;
  *(uint8_t*)0x200010c4 = 0;
  *(uint8_t*)0x200010c5 = 0;
  *(uint8_t*)0x200010c6 = 0;
  *(uint8_t*)0x200010c7 = 0;
  *(uint8_t*)0x200010c8 = 0;
  *(uint8_t*)0x200010c9 = 0;
  *(uint8_t*)0x200010ca = 0;
  *(uint8_t*)0x200010cb = 0;
  *(uint8_t*)0x200010cc = 0;
  *(uint8_t*)0x200010cd = 0;
  *(uint8_t*)0x200010ce = 0;
  *(uint8_t*)0x200010cf = 0;
  *(uint8_t*)0x200010d0 = 0;
  *(uint8_t*)0x200010d1 = 0;
  *(uint8_t*)0x200010d2 = 0;
  *(uint8_t*)0x200010d3 = 0;
  *(uint8_t*)0x200010d4 = 0;
  *(uint8_t*)0x200010d5 = 0;
  *(uint8_t*)0x200010d6 = 0;
  *(uint8_t*)0x200010d7 = 0;
  *(uint8_t*)0x200010d8 = 0;
  *(uint8_t*)0x200010d9 = 0;
  *(uint8_t*)0x200010da = 0;
  *(uint8_t*)0x200010db = 0;
  *(uint8_t*)0x200010dc = 0;
  *(uint8_t*)0x200010dd = 0;
  *(uint8_t*)0x200010de = 0;
  *(uint8_t*)0x200010df = 0;
  *(uint8_t*)0x200010e0 = 0;
  *(uint8_t*)0x200010e1 = 0;
  *(uint8_t*)0x200010e2 = 0;
  *(uint8_t*)0x200010e3 = 0;
  *(uint8_t*)0x200010e4 = 0;
  *(uint8_t*)0x200010e5 = 0;
  *(uint8_t*)0x200010e6 = 0;
  *(uint8_t*)0x200010e7 = 0;
  *(uint32_t*)0x200010e8 = 0;
  *(uint16_t*)0x200010ec = 0xa8;
  *(uint16_t*)0x200010ee = 0xd0;
  *(uint32_t*)0x200010f0 = 0;
  *(uint64_t*)0x200010f8 = 0;
  *(uint64_t*)0x20001100 = 0;
  *(uint16_t*)0x20001108 = 0x28;
  memcpy((void*)0x2000110a, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20001127 = 0;
  *(uint32_t*)0x20001128 = 0xfffffffe;
  *(uint8_t*)0x20001130 = 0;
  *(uint8_t*)0x20001131 = 0;
  *(uint8_t*)0x20001132 = 0;
  *(uint8_t*)0x20001133 = 0;
  *(uint8_t*)0x20001134 = 0;
  *(uint8_t*)0x20001135 = 0;
  *(uint8_t*)0x20001136 = 0;
  *(uint8_t*)0x20001137 = 0;
  *(uint8_t*)0x20001138 = 0;
  *(uint8_t*)0x20001139 = 0;
  *(uint8_t*)0x2000113a = 0;
  *(uint8_t*)0x2000113b = 0;
  *(uint8_t*)0x2000113c = 0;
  *(uint8_t*)0x2000113d = 0;
  *(uint8_t*)0x2000113e = 0;
  *(uint8_t*)0x2000113f = 0;
  *(uint8_t*)0x20001140 = 0;
  *(uint8_t*)0x20001141 = 0;
  *(uint8_t*)0x20001142 = 0;
  *(uint8_t*)0x20001143 = 0;
  *(uint8_t*)0x20001144 = 0;
  *(uint8_t*)0x20001145 = 0;
  *(uint8_t*)0x20001146 = 0;
  *(uint8_t*)0x20001147 = 0;
  *(uint8_t*)0x20001148 = 0;
  *(uint8_t*)0x20001149 = 0;
  *(uint8_t*)0x2000114a = 0;
  *(uint8_t*)0x2000114b = 0;
  *(uint8_t*)0x2000114c = 0;
  *(uint8_t*)0x2000114d = 0;
  *(uint8_t*)0x2000114e = 0;
  *(uint8_t*)0x2000114f = 0;
  *(uint8_t*)0x20001150 = 0;
  *(uint8_t*)0x20001151 = 0;
  *(uint8_t*)0x20001152 = 0;
  *(uint8_t*)0x20001153 = 0;
  *(uint8_t*)0x20001154 = 0;
  *(uint8_t*)0x20001155 = 0;
  *(uint8_t*)0x20001156 = 0;
  *(uint8_t*)0x20001157 = 0;
  *(uint8_t*)0x20001158 = 0;
  *(uint8_t*)0x20001159 = 0;
  *(uint8_t*)0x2000115a = 0;
  *(uint8_t*)0x2000115b = 0;
  *(uint8_t*)0x2000115c = 0;
  *(uint8_t*)0x2000115d = 0;
  *(uint8_t*)0x2000115e = 0;
  *(uint8_t*)0x2000115f = 0;
  *(uint8_t*)0x20001160 = 0;
  *(uint8_t*)0x20001161 = 0;
  *(uint8_t*)0x20001162 = 0;
  *(uint8_t*)0x20001163 = 0;
  *(uint8_t*)0x20001164 = 0;
  *(uint8_t*)0x20001165 = 0;
  *(uint8_t*)0x20001166 = 0;
  *(uint8_t*)0x20001167 = 0;
  *(uint8_t*)0x20001168 = 0;
  *(uint8_t*)0x20001169 = 0;
  *(uint8_t*)0x2000116a = 0;
  *(uint8_t*)0x2000116b = 0;
  *(uint8_t*)0x2000116c = 0;
  *(uint8_t*)0x2000116d = 0;
  *(uint8_t*)0x2000116e = 0;
  *(uint8_t*)0x2000116f = 0;
  *(uint8_t*)0x20001170 = 0;
  *(uint8_t*)0x20001171 = 0;
  *(uint8_t*)0x20001172 = 0;
  *(uint8_t*)0x20001173 = 0;
  *(uint8_t*)0x20001174 = 0;
  *(uint8_t*)0x20001175 = 0;
  *(uint8_t*)0x20001176 = 0;
  *(uint8_t*)0x20001177 = 0;
  *(uint8_t*)0x20001178 = 0;
  *(uint8_t*)0x20001179 = 0;
  *(uint8_t*)0x2000117a = 0;
  *(uint8_t*)0x2000117b = 0;
  *(uint8_t*)0x2000117c = 0;
  *(uint8_t*)0x2000117d = 0;
  *(uint8_t*)0x2000117e = 0;
  *(uint8_t*)0x2000117f = 0;
  *(uint8_t*)0x20001180 = 0;
  *(uint8_t*)0x20001181 = 0;
  *(uint8_t*)0x20001182 = 0;
  *(uint8_t*)0x20001183 = 0;
  *(uint8_t*)0x20001184 = 0;
  *(uint8_t*)0x20001185 = 0;
  *(uint8_t*)0x20001186 = 0;
  *(uint8_t*)0x20001187 = 0;
  *(uint8_t*)0x20001188 = 0;
  *(uint8_t*)0x20001189 = 0;
  *(uint8_t*)0x2000118a = 0;
  *(uint8_t*)0x2000118b = 0;
  *(uint8_t*)0x2000118c = 0;
  *(uint8_t*)0x2000118d = 0;
  *(uint8_t*)0x2000118e = 0;
  *(uint8_t*)0x2000118f = 0;
  *(uint8_t*)0x20001190 = 0;
  *(uint8_t*)0x20001191 = 0;
  *(uint8_t*)0x20001192 = 0;
  *(uint8_t*)0x20001193 = 0;
  *(uint8_t*)0x20001194 = 0;
  *(uint8_t*)0x20001195 = 0;
  *(uint8_t*)0x20001196 = 0;
  *(uint8_t*)0x20001197 = 0;
  *(uint8_t*)0x20001198 = 0;
  *(uint8_t*)0x20001199 = 0;
  *(uint8_t*)0x2000119a = 0;
  *(uint8_t*)0x2000119b = 0;
  *(uint8_t*)0x2000119c = 0;
  *(uint8_t*)0x2000119d = 0;
  *(uint8_t*)0x2000119e = 0;
  *(uint8_t*)0x2000119f = 0;
  *(uint8_t*)0x200011a0 = 0;
  *(uint8_t*)0x200011a1 = 0;
  *(uint8_t*)0x200011a2 = 0;
  *(uint8_t*)0x200011a3 = 0;
  *(uint8_t*)0x200011a4 = 0;
  *(uint8_t*)0x200011a5 = 0;
  *(uint8_t*)0x200011a6 = 0;
  *(uint8_t*)0x200011a7 = 0;
  *(uint8_t*)0x200011a8 = 0;
  *(uint8_t*)0x200011a9 = 0;
  *(uint8_t*)0x200011aa = 0;
  *(uint8_t*)0x200011ab = 0;
  *(uint8_t*)0x200011ac = 0;
  *(uint8_t*)0x200011ad = 0;
  *(uint8_t*)0x200011ae = 0;
  *(uint8_t*)0x200011af = 0;
  *(uint8_t*)0x200011b0 = 0;
  *(uint8_t*)0x200011b1 = 0;
  *(uint8_t*)0x200011b2 = 0;
  *(uint8_t*)0x200011b3 = 0;
  *(uint8_t*)0x200011b4 = 0;
  *(uint8_t*)0x200011b5 = 0;
  *(uint8_t*)0x200011b6 = 0;
  *(uint8_t*)0x200011b7 = 0;
  *(uint32_t*)0x200011b8 = 0;
  *(uint16_t*)0x200011bc = 0xa8;
  *(uint16_t*)0x200011be = 0xd0;
  *(uint32_t*)0x200011c0 = 0;
  *(uint64_t*)0x200011c4 = 0;
  *(uint64_t*)0x200011cc = 0;
  *(uint16_t*)0x200011d8 = 0x28;
  memcpy((void*)0x200011da, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x200011f7 = 0;
  *(uint32_t*)0x200011f8 = 0;
  *(uint8_t*)0x20001200 = 0;
  *(uint8_t*)0x20001201 = 0;
  *(uint8_t*)0x20001202 = 0;
  *(uint8_t*)0x20001203 = 0;
  *(uint8_t*)0x20001204 = 0;
  *(uint8_t*)0x20001205 = 0;
  *(uint8_t*)0x20001206 = 0;
  *(uint8_t*)0x20001207 = 0;
  *(uint8_t*)0x20001208 = 0;
  *(uint8_t*)0x20001209 = 0;
  *(uint8_t*)0x2000120a = 0;
  *(uint8_t*)0x2000120b = 0;
  *(uint8_t*)0x2000120c = 0;
  *(uint8_t*)0x2000120d = 0;
  *(uint8_t*)0x2000120e = 0;
  *(uint8_t*)0x2000120f = 0;
  *(uint8_t*)0x20001210 = 0;
  *(uint8_t*)0x20001211 = 0;
  *(uint8_t*)0x20001212 = 0;
  *(uint8_t*)0x20001213 = 0;
  *(uint8_t*)0x20001214 = 0;
  *(uint8_t*)0x20001215 = 0;
  *(uint8_t*)0x20001216 = 0;
  *(uint8_t*)0x20001217 = 0;
  *(uint8_t*)0x20001218 = 0;
  *(uint8_t*)0x20001219 = 0;
  *(uint8_t*)0x2000121a = 0;
  *(uint8_t*)0x2000121b = 0;
  *(uint8_t*)0x2000121c = 0;
  *(uint8_t*)0x2000121d = 0;
  *(uint8_t*)0x2000121e = 0;
  *(uint8_t*)0x2000121f = 0;
  *(uint8_t*)0x20001220 = 0;
  *(uint8_t*)0x20001221 = 0;
  *(uint8_t*)0x20001222 = 0;
  *(uint8_t*)0x20001223 = 0;
  *(uint8_t*)0x20001224 = 0;
  *(uint8_t*)0x20001225 = 0;
  *(uint8_t*)0x20001226 = 0;
  *(uint8_t*)0x20001227 = 0;
  *(uint8_t*)0x20001228 = 0;
  *(uint8_t*)0x20001229 = 0;
  *(uint8_t*)0x2000122a = 0;
  *(uint8_t*)0x2000122b = 0;
  *(uint8_t*)0x2000122c = 0;
  *(uint8_t*)0x2000122d = 0;
  *(uint8_t*)0x2000122e = 0;
  *(uint8_t*)0x2000122f = 0;
  *(uint8_t*)0x20001230 = 0;
  *(uint8_t*)0x20001231 = 0;
  *(uint8_t*)0x20001232 = 0;
  *(uint8_t*)0x20001233 = 0;
  *(uint8_t*)0x20001234 = 0;
  *(uint8_t*)0x20001235 = 0;
  *(uint8_t*)0x20001236 = 0;
  *(uint8_t*)0x20001237 = 0;
  *(uint8_t*)0x20001238 = 0;
  *(uint8_t*)0x20001239 = 0;
  *(uint8_t*)0x2000123a = 0;
  *(uint8_t*)0x2000123b = 0;
  *(uint8_t*)0x2000123c = 0;
  *(uint8_t*)0x2000123d = 0;
  *(uint8_t*)0x2000123e = 0;
  *(uint8_t*)0x2000123f = 0;
  *(uint8_t*)0x20001240 = 0;
  *(uint8_t*)0x20001241 = 0;
  *(uint8_t*)0x20001242 = 0;
  *(uint8_t*)0x20001243 = 0;
  *(uint8_t*)0x20001244 = 0;
  *(uint8_t*)0x20001245 = 0;
  *(uint8_t*)0x20001246 = 0;
  *(uint8_t*)0x20001247 = 0;
  *(uint8_t*)0x20001248 = 0;
  *(uint8_t*)0x20001249 = 0;
  *(uint8_t*)0x2000124a = 0;
  *(uint8_t*)0x2000124b = 0;
  *(uint8_t*)0x2000124c = 0;
  *(uint8_t*)0x2000124d = 0;
  *(uint8_t*)0x2000124e = 0;
  *(uint8_t*)0x2000124f = 0;
  *(uint8_t*)0x20001250 = 0;
  *(uint8_t*)0x20001251 = 0;
  *(uint8_t*)0x20001252 = 0;
  *(uint8_t*)0x20001253 = 0;
  *(uint8_t*)0x20001254 = 0;
  *(uint8_t*)0x20001255 = 0;
  *(uint8_t*)0x20001256 = 0;
  *(uint8_t*)0x20001257 = 0;
  *(uint8_t*)0x20001258 = 0;
  *(uint8_t*)0x20001259 = 0;
  *(uint8_t*)0x2000125a = 0;
  *(uint8_t*)0x2000125b = 0;
  *(uint8_t*)0x2000125c = 0;
  *(uint8_t*)0x2000125d = 0;
  *(uint8_t*)0x2000125e = 0;
  *(uint8_t*)0x2000125f = 0;
  *(uint8_t*)0x20001260 = 0;
  *(uint8_t*)0x20001261 = 0;
  *(uint8_t*)0x20001262 = 0;
  *(uint8_t*)0x20001263 = 0;
  *(uint8_t*)0x20001264 = 0;
  *(uint8_t*)0x20001265 = 0;
  *(uint8_t*)0x20001266 = 0;
  *(uint8_t*)0x20001267 = 0;
  *(uint8_t*)0x20001268 = 0;
  *(uint8_t*)0x20001269 = 0;
  *(uint8_t*)0x2000126a = 0;
  *(uint8_t*)0x2000126b = 0;
  *(uint8_t*)0x2000126c = 0;
  *(uint8_t*)0x2000126d = 0;
  *(uint8_t*)0x2000126e = 0;
  *(uint8_t*)0x2000126f = 0;
  *(uint8_t*)0x20001270 = 0;
  *(uint8_t*)0x20001271 = 0;
  *(uint8_t*)0x20001272 = 0;
  *(uint8_t*)0x20001273 = 0;
  *(uint8_t*)0x20001274 = 0;
  *(uint8_t*)0x20001275 = 0;
  *(uint8_t*)0x20001276 = 0;
  *(uint8_t*)0x20001277 = 0;
  *(uint8_t*)0x20001278 = 0;
  *(uint8_t*)0x20001279 = 0;
  *(uint8_t*)0x2000127a = 0;
  *(uint8_t*)0x2000127b = 0;
  *(uint8_t*)0x2000127c = 0;
  *(uint8_t*)0x2000127d = 0;
  *(uint8_t*)0x2000127e = 0;
  *(uint8_t*)0x2000127f = 0;
  *(uint8_t*)0x20001280 = 0;
  *(uint8_t*)0x20001281 = 0;
  *(uint8_t*)0x20001282 = 0;
  *(uint8_t*)0x20001283 = 0;
  *(uint8_t*)0x20001284 = 0;
  *(uint8_t*)0x20001285 = 0;
  *(uint8_t*)0x20001286 = 0;
  *(uint8_t*)0x20001287 = 0;
  *(uint32_t*)0x20001288 = 0;
  *(uint16_t*)0x2000128c = 0xa8;
  *(uint16_t*)0x2000128e = 0xd0;
  *(uint32_t*)0x20001290 = 0;
  *(uint64_t*)0x20001294 = 0;
  *(uint64_t*)0x2000129c = 0;
  *(uint16_t*)0x200012a8 = 0x28;
  memcpy((void*)0x200012aa, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x200012c7 = 0;
  *(uint32_t*)0x200012c8 = -1;
  *(uint64_t*)0x200012d0 = htobe64(0);
  *(uint64_t*)0x200012d8 = htobe64(1);
  *(uint8_t*)0x200012e0 = 0xfe;
  *(uint8_t*)0x200012e1 = 0x80;
  *(uint8_t*)0x200012e2 = 0;
  *(uint8_t*)0x200012e3 = 0;
  *(uint8_t*)0x200012e4 = 0;
  *(uint8_t*)0x200012e5 = 0;
  *(uint8_t*)0x200012e6 = 0;
  *(uint8_t*)0x200012e7 = 0;
  *(uint8_t*)0x200012e8 = 0;
  *(uint8_t*)0x200012e9 = 0;
  *(uint8_t*)0x200012ea = 0;
  *(uint8_t*)0x200012eb = 0;
  *(uint8_t*)0x200012ec = 0;
  *(uint8_t*)0x200012ed = 0;
  *(uint8_t*)0x200012ee = 0;
  *(uint8_t*)0x200012ef = 0xbb;
  *(uint32_t*)0x200012f0 = htobe32(0);
  *(uint32_t*)0x200012f4 = htobe32(0);
  *(uint32_t*)0x200012f8 = htobe32(0);
  *(uint32_t*)0x200012fc = htobe32(0);
  *(uint32_t*)0x20001300 = htobe32(0);
  *(uint32_t*)0x20001304 = htobe32(0);
  *(uint32_t*)0x20001308 = htobe32(0);
  *(uint32_t*)0x2000130c = htobe32(0);
  memcpy((void*)0x20001310,
         "\x76\x63\x61\x6e\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  memcpy((void*)0x20001320,
         "\x67\x72\x65\x74\x61\x70\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  *(uint8_t*)0x20001330 = 0;
  *(uint8_t*)0x20001331 = 0;
  *(uint8_t*)0x20001332 = 0;
  *(uint8_t*)0x20001333 = 0;
  *(uint8_t*)0x20001334 = 0;
  *(uint8_t*)0x20001335 = 0;
  *(uint8_t*)0x20001336 = 0;
  *(uint8_t*)0x20001337 = 0;
  *(uint8_t*)0x20001338 = 0;
  *(uint8_t*)0x20001339 = 0;
  *(uint8_t*)0x2000133a = 0;
  *(uint8_t*)0x2000133b = 0;
  *(uint8_t*)0x2000133c = 0;
  *(uint8_t*)0x2000133d = 0;
  *(uint8_t*)0x2000133e = 0;
  *(uint8_t*)0x2000133f = 0;
  *(uint8_t*)0x20001340 = 0;
  *(uint8_t*)0x20001341 = 0;
  *(uint8_t*)0x20001342 = 0;
  *(uint8_t*)0x20001343 = 0;
  *(uint8_t*)0x20001344 = 0;
  *(uint8_t*)0x20001345 = 0;
  *(uint8_t*)0x20001346 = 0;
  *(uint8_t*)0x20001347 = 0;
  *(uint8_t*)0x20001348 = 0;
  *(uint8_t*)0x20001349 = 0;
  *(uint8_t*)0x2000134a = 0;
  *(uint8_t*)0x2000134b = 0;
  *(uint8_t*)0x2000134c = 0;
  *(uint8_t*)0x2000134d = 0;
  *(uint8_t*)0x2000134e = 0;
  *(uint8_t*)0x2000134f = 0;
  *(uint16_t*)0x20001350 = 0;
  *(uint8_t*)0x20001352 = 0;
  *(uint8_t*)0x20001353 = 0;
  *(uint8_t*)0x20001354 = 0;
  *(uint32_t*)0x20001358 = 0;
  *(uint16_t*)0x2000135c = 0xa8;
  *(uint16_t*)0x2000135e = 0x108;
  *(uint32_t*)0x20001360 = 0;
  *(uint64_t*)0x20001364 = 0;
  *(uint64_t*)0x2000136c = 0;
  *(uint16_t*)0x20001378 = 0x60;
  memcpy((void*)0x2000137a, "\x48\x4d\x41\x52\x4b\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20001397 = 0;
  *(uint8_t*)0x20001398 = 0xfe;
  *(uint8_t*)0x20001399 = 0x80;
  *(uint8_t*)0x2000139a = 0;
  *(uint8_t*)0x2000139b = 0;
  *(uint8_t*)0x2000139c = 0;
  *(uint8_t*)0x2000139d = 0;
  *(uint8_t*)0x2000139e = 0;
  *(uint8_t*)0x2000139f = 0;
  *(uint8_t*)0x200013a0 = 0;
  *(uint8_t*)0x200013a1 = 0;
  *(uint8_t*)0x200013a2 = 0;
  *(uint8_t*)0x200013a3 = 0;
  *(uint8_t*)0x200013a4 = 0;
  *(uint8_t*)0x200013a5 = 0;
  *(uint8_t*)0x200013a6 = 0;
  *(uint8_t*)0x200013a7 = 0xaa;
  *(uint32_t*)0x200013a8 = htobe32(0);
  *(uint32_t*)0x200013ac = htobe32(0);
  *(uint32_t*)0x200013b0 = htobe32(0);
  *(uint32_t*)0x200013b4 = htobe32(0);
  *(uint16_t*)0x200013b8 = 0;
  *(uint16_t*)0x200013ba = 0;
  *(uint16_t*)0x200013bc = 0;
  *(uint16_t*)0x200013be = 0;
  *(uint32_t*)0x200013c0 = 5;
  *(uint16_t*)0x200013c4 = 0;
  *(uint32_t*)0x200013c8 = 0;
  *(uint32_t*)0x200013cc = 0x80;
  *(uint32_t*)0x200013d0 = 0;
  *(uint64_t*)0x20000fc0 = 0;
  *(uint64_t*)0x20000fc8 = 0;
  *(uint64_t*)0x20000fd0 = 0;
  *(uint64_t*)0x20000fd8 = 0;
  *(uint64_t*)0x20000fe0 = 0;
  *(uint64_t*)0x20000fe8 = 0;
  *(uint64_t*)0x20000ff0 = 0;
  *(uint64_t*)0x20000ff8 = 0;
  syscall(__NR_setsockopt, r[0], 0x29, 0x40, 0x20001000, 0x3d8);
}

int main()
{
  for (;;) {
    setup_tun(0, true);
    loop();
  }
}
