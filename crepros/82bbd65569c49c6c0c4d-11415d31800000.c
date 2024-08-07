// https://syzkaller.appspot.com/bug?id=983a5263ce36ddfe94710b5a86ada468177e635a
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

#define BITMASK_LEN_OFF(type, bf_off, bf_len)                          \
  (type)(BITMASK_LEN(type, (bf_len)) << (bf_off))

#define STORE_BY_BITMASK(type, addr, val, bf_off, bf_len)              \
  if ((bf_off) == 0 && (bf_len) == 0) {                                \
    *(type*)(addr) = (type)(val);                                      \
  } else {                                                             \
    type new_val = *(type*)(addr);                                     \
    new_val &= ~BITMASK_LEN_OFF(type, (bf_off), (bf_len));             \
    new_val |= ((type)(val)&BITMASK_LEN(type, (bf_len))) << (bf_off);  \
    *(type*)(addr) = new_val;                                          \
  }

struct csum_inet {
  uint32_t acc;
};

static void csum_inet_init(struct csum_inet* csum)
{
  csum->acc = 0;
}

static void csum_inet_update(struct csum_inet* csum,
                             const uint8_t* data, size_t length)
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

static void snprintf_check(char* str, size_t size, const char* format,
                           ...)
{
  va_list args;

  va_start(args, format);
  vsnprintf_check(str, size, format, args);
  va_end(args);
}

#define COMMAND_MAX_LEN 128
#define PATH_PREFIX                                                    \
  "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin "
#define PATH_PREFIX_LEN (sizeof(PATH_PREFIX) - 1)

static void execute_command(const char* format, ...)
{
  va_list args;
  char command[PATH_PREFIX_LEN + COMMAND_MAX_LEN];
  int rv;

  va_start(args, format);
  memcpy(command, PATH_PREFIX, PATH_PREFIX_LEN);
  vsnprintf_check(command + PATH_PREFIX_LEN, COMMAND_MAX_LEN, format,
                  args);
  rv = system(command);
  if (rv != 0)
    fail("tun: command \"%s\" failed with code %d", &command[0], rv);

  va_end(args);
}

static int tunfd = -1;
static int tun_frags_enabled;

#define SYZ_TUN_MAX_PACKET_SIZE 1000

#define MAX_PIDS 32
#define ADDR_MAX_LEN 32

#define LOCAL_MAC "aa:aa:aa:aa:aa:%02hx"
#define REMOTE_MAC "bb:bb:bb:bb:bb:%02hx"

#define LOCAL_IPV4 "172.20.%d.170"
#define REMOTE_IPV4 "172.20.%d.187"

#define LOCAL_IPV6 "fe80::%02hxaa"
#define REMOTE_IPV6 "fe80::%02hxbb"

#define IFF_NAPI 0x0010
#define IFF_NAPI_FRAGS 0x0020

static void initialize_tun(uint64_t pid)
{
  if (pid >= MAX_PIDS)
    fail("tun: no more than %d executors", MAX_PIDS);
  int id = pid;

  tunfd = open("/dev/net/tun", O_RDWR | O_NONBLOCK);
  if (tunfd == -1) {
    printf(
        "tun: can't open /dev/net/tun: please enable CONFIG_TUN=y\n");
    printf("otherwise fuzzing or reproducing might not work as "
           "intended\n");
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

  execute_command("sysctl -w net.ipv6.conf.%s.accept_dad=0", iface);

  execute_command("sysctl -w net.ipv6.conf.%s.router_solicitations=0",
                  iface);

  execute_command("ip link set dev %s address %s", iface, local_mac);
  execute_command("ip addr add %s/24 dev %s", local_ipv4, iface);
  execute_command("ip -6 addr add %s/120 dev %s", local_ipv6, iface);
  execute_command("ip neigh add %s lladdr %s dev %s nud permanent",
                  remote_ipv4, remote_mac, iface);
  execute_command("ip -6 neigh add %s lladdr %s dev %s nud permanent",
                  remote_ipv6, remote_mac, iface);
  execute_command("ip link set dev %s up", iface);
}

static void setup_tun(uint64_t pid, bool enable_tun)
{
  if (enable_tun)
    initialize_tun(pid);
}

#define MAX_FRAGS 4
struct vnet_fragmentation {
  uint32_t full;
  uint32_t count;
  uint32_t frags[MAX_FRAGS];
};

static uintptr_t syz_emit_ethernet(uintptr_t a0, uintptr_t a1,
                                   uintptr_t a2)
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

#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

void loop()
{
  syscall(__NR_mmap, 0x20000000, 0xe79000, 0x3, 0x32, 0xffffffff, 0x0);
  *(uint8_t*)0x20000000 = 0xaa;
  *(uint8_t*)0x20000001 = 0xaa;
  *(uint8_t*)0x20000002 = 0xaa;
  *(uint8_t*)0x20000003 = 0xaa;
  *(uint8_t*)0x20000004 = 0xaa;
  *(uint8_t*)0x20000005 = 0x0;
  *(uint8_t*)0x20000006 = 0x0;
  *(uint8_t*)0x20000007 = 0x0;
  *(uint8_t*)0x20000008 = 0x0;
  *(uint8_t*)0x20000009 = 0x0;
  *(uint8_t*)0x2000000a = 0x0;
  *(uint8_t*)0x2000000b = 0x0;
  *(uint16_t*)0x2000000c = htobe16(0x86dd);
  *(uint8_t*)0x2000000e = 0x1;
  *(uint8_t*)0x2000000f = 0x0;
  *(uint8_t*)0x20000010 = 0x0;
  syz_emit_ethernet(0x11, 0x20000000, 0x20000000);
  *(uint8_t*)0x2084c000 = 0xaa;
  *(uint8_t*)0x2084c001 = 0xaa;
  *(uint8_t*)0x2084c002 = 0xaa;
  *(uint8_t*)0x2084c003 = 0xaa;
  *(uint8_t*)0x2084c004 = 0xaa;
  *(uint8_t*)0x2084c005 = 0x0;
  *(uint8_t*)0x2084c006 = 0x0;
  *(uint8_t*)0x2084c007 = 0x0;
  *(uint8_t*)0x2084c008 = 0x0;
  *(uint8_t*)0x2084c009 = 0x0;
  *(uint8_t*)0x2084c00a = 0x0;
  *(uint8_t*)0x2084c00b = 0x0;
  *(uint16_t*)0x2084c00c = htobe16(0x86dd);
  STORE_BY_BITMASK(uint8_t, 0x2084c00e, 0x0, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x2084c00e, 0x6, 4, 4);
  memcpy((void*)0x2084c00f, "\x01\x23\x16", 3);
  *(uint16_t*)0x2084c012 = htobe16(0x10);
  *(uint8_t*)0x2084c014 = 0x32;
  *(uint8_t*)0x2084c015 = 0x0;
  *(uint8_t*)0x2084c016 = 0xfe;
  *(uint8_t*)0x2084c017 = 0x80;
  *(uint8_t*)0x2084c018 = 0x0;
  *(uint8_t*)0x2084c019 = 0x0;
  *(uint8_t*)0x2084c01a = 0x0;
  *(uint8_t*)0x2084c01b = 0x0;
  *(uint8_t*)0x2084c01c = 0x0;
  *(uint8_t*)0x2084c01d = 0x0;
  *(uint8_t*)0x2084c01e = 0x0;
  *(uint8_t*)0x2084c01f = 0x0;
  *(uint8_t*)0x2084c020 = 0x0;
  *(uint8_t*)0x2084c021 = 0x0;
  *(uint8_t*)0x2084c022 = 0x0;
  *(uint8_t*)0x2084c023 = 0x0;
  *(uint8_t*)0x2084c024 = 0x0;
  *(uint8_t*)0x2084c025 = 0xbb;
  *(uint8_t*)0x2084c026 = 0xfe;
  *(uint8_t*)0x2084c027 = 0x80;
  *(uint8_t*)0x2084c028 = 0x0;
  *(uint8_t*)0x2084c029 = 0x0;
  *(uint8_t*)0x2084c02a = 0x0;
  *(uint8_t*)0x2084c02b = 0x0;
  *(uint8_t*)0x2084c02c = 0x0;
  *(uint8_t*)0x2084c02d = 0x0;
  *(uint8_t*)0x2084c02e = 0x0;
  *(uint8_t*)0x2084c02f = 0x0;
  *(uint8_t*)0x2084c030 = 0x0;
  *(uint8_t*)0x2084c031 = 0x0;
  *(uint8_t*)0x2084c032 = 0x0;
  *(uint8_t*)0x2084c033 = 0x0;
  *(uint8_t*)0x2084c034 = 0x0;
  *(uint8_t*)0x2084c035 = 0xaa;
  *(uint16_t*)0x2084c036 = htobe16(0x4e23);
  *(uint16_t*)0x2084c038 = htobe16(0x4e23);
  *(uint8_t*)0x2084c03a = 0x4;
  STORE_BY_BITMASK(uint8_t, 0x2084c03b, 0x1, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x2084c03b, 0x80, 4, 4);
  *(uint16_t*)0x2084c03c = 0x0;
  STORE_BY_BITMASK(uint8_t, 0x2084c03e, 0x0, 0, 1);
  STORE_BY_BITMASK(uint8_t, 0x2084c03e, 0x7, 1, 4);
  STORE_BY_BITMASK(uint8_t, 0x2084c03e, 0xff, 5, 3);
  memcpy((void*)0x2084c03f, "sa&", 3);
  *(uint8_t*)0x2084c042 = 0x2;
  memcpy((void*)0x2084c043, "\xb7\xf2\x01", 3);
  struct csum_inet csum_1;
  csum_inet_init(&csum_1);
  csum_inet_update(&csum_1, (const uint8_t*)0x2084c016, 16);
  csum_inet_update(&csum_1, (const uint8_t*)0x2084c026, 16);
  uint32_t csum_1_chunk_2 = 0x10000000;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_2, 4);
  uint32_t csum_1_chunk_3 = 0x21000000;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_3, 4);
  csum_inet_update(&csum_1, (const uint8_t*)0x2084c036, 16);
  *(uint16_t*)0x2084c03c = csum_inet_digest(&csum_1);
  syz_emit_ethernet(0x46, 0x2084c000, 0x20000000);
}

int main()
{
  setup_tun(0, true);
  loop();
  return 0;
}
