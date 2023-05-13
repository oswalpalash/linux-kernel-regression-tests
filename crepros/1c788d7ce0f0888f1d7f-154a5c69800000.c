// https://syzkaller.appspot.com/bug?id=4f32ed6daf43670024f14827ebfd333cff8334da
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

long r[1];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 0x10, 3, 0);
  *(uint64_t*)0x2001bfc8 = 0x20016000;
  *(uint32_t*)0x2001bfd0 = 0xc;
  *(uint64_t*)0x2001bfd8 = 0x2000b000;
  *(uint64_t*)0x2001bfe0 = 1;
  *(uint64_t*)0x2001bfe8 = 0;
  *(uint64_t*)0x2001bff0 = 0;
  *(uint32_t*)0x2001bff8 = 0;
  *(uint16_t*)0x20016000 = 0x10;
  *(uint16_t*)0x20016002 = 0;
  *(uint32_t*)0x20016004 = 0;
  *(uint32_t*)0x20016008 = 0;
  *(uint64_t*)0x2000b000 = 0x2000d000;
  *(uint64_t*)0x2000b008 = 0x34;
  *(uint32_t*)0x2000d000 = 0x34;
  *(uint16_t*)0x2000d004 = 0x11;
  *(uint16_t*)0x2000d006 = 0x209;
  *(uint32_t*)0x2000d008 = 0;
  *(uint32_t*)0x2000d00c = 0;
  *(uint8_t*)0x2000d010 = 0;
  *(uint8_t*)0x2000d011 = 0;
  *(uint16_t*)0x2000d012 = 0;
  *(uint32_t*)0x2000d014 = 0;
  *(uint32_t*)0x2000d018 = 0;
  *(uint32_t*)0x2000d01c = 0xfffffffe;
  *(uint16_t*)0x2000d020 = 0x14;
  *(uint16_t*)0x2000d022 = 3;
  *(uint8_t*)0x2000d024 = 0x73;
  *(uint8_t*)0x2000d025 = 0x79;
  *(uint8_t*)0x2000d026 = 0x7a;
  *(uint8_t*)0x2000d027 = 0x30;
  *(uint8_t*)0x2000d028 = 0;
  syscall(__NR_sendmsg, r[0], 0x2001bfc8, 0x20000040);
  *(uint8_t*)0x20ecbf66 = 0xaa;
  *(uint8_t*)0x20ecbf67 = 0xaa;
  *(uint8_t*)0x20ecbf68 = 0xaa;
  *(uint8_t*)0x20ecbf69 = 0xaa;
  *(uint8_t*)0x20ecbf6a = 0;
  *(uint8_t*)0x20ecbf6b = 0xaa;
  *(uint8_t*)0x20ecbf6c = 0xaa;
  *(uint8_t*)0x20ecbf6d = 0xaa;
  *(uint8_t*)0x20ecbf6e = 0xaa;
  *(uint8_t*)0x20ecbf6f = 0xaa;
  *(uint8_t*)0x20ecbf70 = 0;
  *(uint8_t*)0x20ecbf71 = 0;
  *(uint16_t*)0x20ecbf72 = htobe16(0x800);
  STORE_BY_BITMASK(uint8_t, 0x20ecbf74, 0x93, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x20ecbf74, 4, 4, 4);
  STORE_BY_BITMASK(uint8_t, 0x20ecbf75, 0, 0, 2);
  STORE_BY_BITMASK(uint8_t, 0x20ecbf75, 0, 2, 6);
  *(uint16_t*)0x20ecbf76 = htobe16(0x30);
  *(uint16_t*)0x20ecbf78 = htobe16(0x67);
  *(uint16_t*)0x20ecbf7a = htobe16(0);
  *(uint8_t*)0x20ecbf7c = 0;
  *(uint8_t*)0x20ecbf7d = 0;
  *(uint16_t*)0x20ecbf7e = 0;
  *(uint32_t*)0x20ecbf80 = htobe32(0);
  *(uint8_t*)0x20ecbf84 = 0xac;
  *(uint8_t*)0x20ecbf85 = 0x14;
  *(uint8_t*)0x20ecbf86 = 0;
  *(uint8_t*)0x20ecbf87 = 0xaa;
  *(uint8_t*)0x20ecbf88 = 3;
  *(uint8_t*)0x20ecbf89 = 0;
  *(uint16_t*)0x20ecbf8a = 0;
  *(uint8_t*)0x20ecbf8c = 0;
  *(uint8_t*)0x20ecbf8d = 0;
  *(uint16_t*)0x20ecbf8e = htobe16(0);
  STORE_BY_BITMASK(uint8_t, 0x20ecbf90, 3, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x20ecbf90, 4, 4, 4);
  STORE_BY_BITMASK(uint8_t, 0x20ecbf91, 0, 0, 2);
  STORE_BY_BITMASK(uint8_t, 0x20ecbf91, 0, 2, 6);
  *(uint16_t*)0x20ecbf92 = htobe16(3);
  *(uint16_t*)0x20ecbf94 = 0;
  *(uint16_t*)0x20ecbf96 = htobe16(0);
  *(uint8_t*)0x20ecbf98 = 0;
  *(uint8_t*)0x20ecbf99 = 0;
  *(uint16_t*)0x20ecbf9a = htobe16(0);
  *(uint8_t*)0x20ecbf9c = 0xac;
  *(uint8_t*)0x20ecbf9d = 0x14;
  *(uint8_t*)0x20ecbf9e = 0;
  *(uint8_t*)0x20ecbf9f = 0xbb;
  *(uint8_t*)0x20ecbfa0 = 0xac;
  *(uint8_t*)0x20ecbfa1 = 0x14;
  *(uint8_t*)0x20ecbfa2 = 0;
  *(uint8_t*)0x20ecbfa3 = 0;
  *(uint32_t*)0x205ecff8 = 0;
  *(uint32_t*)0x205ecffc = 0;
  struct csum_inet csum_1;
  csum_inet_init(&csum_1);
  csum_inet_update(&csum_1, (const uint8_t*)0x20ecbf88, 28);
  *(uint16_t*)0x20ecbf8a = csum_inet_digest(&csum_1);
  struct csum_inet csum_2;
  csum_inet_init(&csum_2);
  csum_inet_update(&csum_2, (const uint8_t*)0x20ecbf74, 20);
  *(uint16_t*)0x20ecbf7e = csum_inet_digest(&csum_2);
  syz_emit_ethernet(0xfed7, 0x20ecbf66, 0x205ecff8);
}

int main()
{
  setup_tun(0, true);
  loop();
  return 0;
}
