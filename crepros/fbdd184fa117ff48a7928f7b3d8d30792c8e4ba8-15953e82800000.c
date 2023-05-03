// https://syzkaller.appspot.com/bug?id=fbdd184fa117ff48a7928f7b3d8d30792c8e4ba8
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
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
  if (tunfd == -1)
    fail("tun: can't open /dev/net/tun");

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

long r[79];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xb3f000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  memcpy((void*)0x20000000, "\xef\xad\x07\x00\x00\xa7", 6);
  *(uint8_t*)0x20000006 = (uint8_t)0x0;
  *(uint8_t*)0x20000007 = (uint8_t)0x0;
  *(uint8_t*)0x20000008 = (uint8_t)0x0;
  *(uint8_t*)0x20000009 = (uint8_t)0x0;
  *(uint8_t*)0x2000000a = (uint8_t)0x0;
  *(uint8_t*)0x2000000b = (uint8_t)0x0;
  *(uint16_t*)0x2000000c = (uint16_t)0x8;
  STORE_BY_BITMASK(uint8_t, 0x2000000e, 0x5, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x2000000e, 0x4, 4, 4);
  STORE_BY_BITMASK(uint8_t, 0x2000000f, 0x0, 0, 2);
  STORE_BY_BITMASK(uint8_t, 0x2000000f, 0x40, 2, 6);
  *(uint16_t*)0x20000010 = (uint16_t)0x6c00;
  *(uint16_t*)0x20000012 = (uint16_t)0x6400;
  *(uint16_t*)0x20000014 = (uint16_t)0x0;
  *(uint8_t*)0x20000016 = (uint8_t)0x0;
  *(uint8_t*)0x20000017 = (uint8_t)0x1;
  *(uint16_t*)0x20000018 = (uint16_t)0x0;
  *(uint8_t*)0x2000001a = (uint8_t)0xac;
  *(uint8_t*)0x2000001b = (uint8_t)0x14;
  *(uint8_t*)0x2000001c = (uint8_t)0x0;
  *(uint8_t*)0x2000001d = (uint8_t)0xbb;
  *(uint8_t*)0x2000001e = (uint8_t)0xac;
  *(uint8_t*)0x2000001f = (uint8_t)0x14;
  *(uint8_t*)0x20000020 = (uint8_t)0x0;
  *(uint8_t*)0x20000021 = (uint8_t)0xaa;
  *(uint8_t*)0x20000022 = (uint8_t)0x3;
  *(uint8_t*)0x20000023 = (uint8_t)0x4;
  *(uint16_t*)0x20000024 = (uint16_t)0x0;
  *(uint8_t*)0x20000026 = (uint8_t)0x0;
  *(uint8_t*)0x20000027 = (uint8_t)0x0;
  *(uint16_t*)0x20000028 = (uint16_t)0x0;
  STORE_BY_BITMASK(uint8_t, 0x2000002a, 0x14, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x2000002a, 0x4, 4, 4);
  STORE_BY_BITMASK(uint8_t, 0x2000002b, 0x0, 0, 2);
  STORE_BY_BITMASK(uint8_t, 0x2000002b, 0x0, 2, 6);
  *(uint16_t*)0x2000002c = (uint16_t)0x0;
  *(uint16_t*)0x2000002e = (uint16_t)0x6400;
  *(uint16_t*)0x20000030 = (uint16_t)0x0;
  *(uint8_t*)0x20000032 = (uint8_t)0x0;
  *(uint8_t*)0x20000033 = (uint8_t)0x200000001;
  *(uint16_t*)0x20000034 = (uint16_t)0x0;
  *(uint32_t*)0x20000036 = (uint32_t)0x100007f;
  *(uint32_t*)0x2000003a = (uint32_t)0x0;
  *(uint8_t*)0x2000003e = (uint8_t)0x83;
  *(uint8_t*)0x2000003f = (uint8_t)0x23;
  *(uint8_t*)0x20000040 = (uint8_t)0x0;
  *(uint32_t*)0x20000041 = (uint32_t)0x100007f;
  *(uint32_t*)0x20000045 = (uint32_t)0x20000e0;
  *(uint8_t*)0x20000049 = (uint8_t)0xac;
  *(uint8_t*)0x2000004a = (uint8_t)0x14;
  *(uint8_t*)0x2000004b = (uint8_t)0x0;
  *(uint8_t*)0x2000004c = (uint8_t)0xaa;
  *(uint32_t*)0x2000004d = (uint32_t)0x20000e0;
  *(uint32_t*)0x20000051 = (uint32_t)0x100007f;
  *(uint32_t*)0x20000055 = (uint32_t)0x10000e0;
  *(uint32_t*)0x20000059 = (uint32_t)0x0;
  *(uint8_t*)0x2000005d = (uint8_t)0xac;
  *(uint8_t*)0x2000005e = (uint8_t)0x14;
  *(uint8_t*)0x2000005f = (uint8_t)0x0;
  *(uint8_t*)0x20000060 = (uint8_t)0xaa;
  *(uint8_t*)0x20000061 = (uint8_t)0x44;
  *(uint8_t*)0x20000062 = (uint8_t)0x4;
  *(uint8_t*)0x20000063 = (uint8_t)0x0;
  STORE_BY_BITMASK(uint8_t, 0x20000064, 0x3, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x20000064, 0x0, 4, 4);
  *(uint8_t*)0x20000065 = (uint8_t)0x89;
  *(uint8_t*)0x20000066 = (uint8_t)0xf;
  *(uint8_t*)0x20000067 = (uint8_t)0x0;
  *(uint32_t*)0x20000068 = (uint32_t)0x10000;
  *(uint32_t*)0x2000006c = (uint32_t)0x10000e0;
  *(uint32_t*)0x20000070 = (uint32_t)0x10000e0;
  *(uint8_t*)0x20000074 = (uint8_t)0x1;
  *(uint8_t*)0x20000075 = (uint8_t)0x0;
  *(uint8_t*)0x20000076 = (uint8_t)0x2;
  struct csum_inet csum_76;
  csum_inet_init(&csum_76);
  csum_inet_update(&csum_76, (const uint8_t*)0x20000022, 88);
  *(uint16_t*)0x20000024 = csum_inet_digest(&csum_76);
  struct csum_inet csum_77;
  csum_inet_init(&csum_77);
  csum_inet_update(&csum_77, (const uint8_t*)0x2000000e, 20);
  *(uint16_t*)0x20000018 = csum_inet_digest(&csum_77);
  r[78] = syz_emit_ethernet(0x7aul, 0x20000000ul, 0x20000000ul);
}

int main()
{
  setup_tun(0, true);
  loop();
  return 0;
}
