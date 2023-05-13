// https://syzkaller.appspot.com/bug?id=3783bbc6fd59013c1436e386ccd48bdf5260e1fb
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
  r[0] = syscall(__NR_socket, 0x10, 2, 0);
  *(uint64_t*)0x20504000 = 0x20dfeff4;
  *(uint32_t*)0x20504008 = 0xc;
  *(uint64_t*)0x20504010 = 0x20986000;
  *(uint64_t*)0x20504018 = 1;
  *(uint64_t*)0x20504020 = 0;
  *(uint64_t*)0x20504028 = 0;
  *(uint32_t*)0x20504030 = 0;
  *(uint16_t*)0x20dfeff4 = 0x10;
  *(uint16_t*)0x20dfeff6 = 0;
  *(uint32_t*)0x20dfeff8 = 0;
  *(uint32_t*)0x20dfeffc = 0;
  *(uint64_t*)0x20986000 = 0x200c4f74;
  *(uint64_t*)0x20986008 = 0x3c;
  *(uint32_t*)0x200c4f74 = 0x3c;
  *(uint16_t*)0x200c4f78 = 0x10;
  *(uint16_t*)0x200c4f7a = 9;
  *(uint32_t*)0x200c4f7c = 0;
  *(uint32_t*)0x200c4f80 = 0;
  *(uint8_t*)0x200c4f84 = 0;
  *(uint8_t*)0x200c4f85 = 0;
  *(uint16_t*)0x200c4f86 = 0;
  *(uint32_t*)0x200c4f88 = 0;
  *(uint32_t*)0x200c4f8c = 0;
  *(uint32_t*)0x200c4f90 = 0;
  *(uint16_t*)0x200c4f94 = 0x14;
  *(uint16_t*)0x200c4f96 = 3;
  *(uint8_t*)0x200c4f98 = 0x73;
  *(uint8_t*)0x200c4f99 = 0x79;
  *(uint8_t*)0x200c4f9a = 0x7a;
  *(uint8_t*)0x200c4f9b = 0x30;
  *(uint8_t*)0x200c4f9c = 0;
  *(uint16_t*)0x200c4fa8 = 8;
  *(uint16_t*)0x200c4faa = 0xd;
  *(uint32_t*)0x200c4fac = 0x80000001;
  syscall(__NR_sendmsg, r[0], 0x20504000, 0);
  *(uint8_t*)0x20003f3d = 0;
  *(uint8_t*)0x20003f3e = 0;
  *(uint8_t*)0x20003f3f = 0;
  *(uint8_t*)0x20003f40 = 0;
  *(uint8_t*)0x20003f41 = 0;
  *(uint8_t*)0x20003f42 = 0;
  *(uint8_t*)0x20003f43 = 1;
  *(uint8_t*)0x20003f44 = 0x80;
  *(uint8_t*)0x20003f45 = 0xc2;
  *(uint8_t*)0x20003f46 = 0;
  *(uint8_t*)0x20003f47 = 0;
  *(uint8_t*)0x20003f48 = 3;
  *(uint16_t*)0x20003f49 = htobe16(0x9100);
  STORE_BY_BITMASK(uint16_t, 0x20003f4b, 5, 0, 3);
  STORE_BY_BITMASK(uint16_t, 0x20003f4b, 8, 3, 1);
  STORE_BY_BITMASK(uint16_t, 0x20003f4b, 0x3ff, 4, 12);
  *(uint16_t*)0x20003f4d = htobe16(0x8100);
  STORE_BY_BITMASK(uint16_t, 0x20003f4f, 9, 0, 3);
  STORE_BY_BITMASK(uint16_t, 0x20003f4f, 0, 3, 1);
  STORE_BY_BITMASK(uint16_t, 0x20003f4f, 0xfffb, 4, 12);
  *(uint16_t*)0x20003f51 = htobe16(0x8848);
  STORE_BY_BITMASK(uint32_t, 0x20003f53, htobe32(9), 0, 8);
  STORE_BY_BITMASK(uint32_t, 0x20003f53, htobe32(0), 8, 1);
  STORE_BY_BITMASK(uint32_t, 0x20003f53, htobe32(2), 9, 3);
  STORE_BY_BITMASK(uint32_t, 0x20003f53, htobe32(1), 12, 20);
  STORE_BY_BITMASK(uint32_t, 0x20003f57, htobe32(0x7fffffff), 0, 8);
  STORE_BY_BITMASK(uint32_t, 0x20003f57, htobe32(4), 8, 1);
  STORE_BY_BITMASK(uint32_t, 0x20003f57, htobe32(0), 9, 3);
  STORE_BY_BITMASK(uint32_t, 0x20003f57, htobe32(0), 12, 20);
  STORE_BY_BITMASK(uint32_t, 0x20003f5b, htobe32(0x3f), 0, 8);
  STORE_BY_BITMASK(uint32_t, 0x20003f5b, htobe32(0xd2), 8, 1);
  STORE_BY_BITMASK(uint32_t, 0x20003f5b, htobe32(9), 9, 3);
  STORE_BY_BITMASK(uint32_t, 0x20003f5b, htobe32(0), 12, 20);
  *(uint8_t*)0x20003f5f = 1;
  *(uint8_t*)0x20003f60 = 0xaa;
  memcpy((void*)0x20003f61, "*", 1);
  memcpy((void*)0x20003f62, "\xed\x8b\x9c", 3);
  *(uint16_t*)0x20003f65 = htobe16(0);
  *(uint32_t*)0x20001fec = 0;
  *(uint32_t*)0x20001ff0 = 3;
  *(uint32_t*)0x20001ff4 = 0x77b;
  *(uint32_t*)0x20001ff8 = 0x8cc;
  *(uint32_t*)0x20001ffc = 0x591;
  syz_emit_ethernet(0x2a, 0x20003f3d, 0x20001fec);
}

int main()
{
  setup_tun(0, true);
  loop();
  return 0;
}
