// https://syzkaller.appspot.com/bug?id=092600a456f1778bf4d02821dc04b49d032cc447
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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

const int kFailStatus = 67;
const int kRetryStatus = 69;

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}

__attribute__((noreturn)) static void fail(const char* msg, ...)
{
  int e = errno;
  fflush(stdout);
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

static void execute_command(const char* format, ...)
{
  va_list args;
  char command[COMMAND_MAX_LEN];
  int rv;

  va_start(args, format);

  vsnprintf_check(command, sizeof(command), format, args);
  rv = system(command);
  if (rv != 0)
    fail("tun: command \"%s\" failed with code %d", &command[0], rv);

  va_end(args);
}

static int tunfd = -1;

#define SYZ_TUN_MAX_PACKET_SIZE 1000

#define MAX_PIDS 32
#define ADDR_MAX_LEN 32

#define LOCAL_MAC "aa:aa:aa:aa:aa:%02hx"
#define REMOTE_MAC "bb:bb:bb:bb:bb:%02hx"

#define LOCAL_IPV4 "172.20.%d.170"
#define REMOTE_IPV4 "172.20.%d.187"

#define LOCAL_IPV6 "fe80::%02hxaa"
#define REMOTE_IPV6 "fe80::%02hxbb"

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
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
  if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0)
    fail("tun: ioctl(TUNSETIFF) failed");

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

static int read_tun(char* data, int size)
{
  int rv = read(tunfd, data, size);
  if (rv < 0) {
    if (errno == EAGAIN)
      return -1;
    fail("tun: read failed with %d, errno: %d", rv, errno);
  }
  return rv;
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

static uintptr_t syz_emit_ethernet(uintptr_t a0, uintptr_t a1)
{

  if (tunfd < 0)
    return (uintptr_t)-1;

  int64_t length = a0;
  char* data = (char*)a1;
  return write(tunfd, data, length);
}

struct ipv6hdr {
  __u8 priority : 4, version : 4;
  __u8 flow_lbl[3];

  __be16 payload_len;
  __u8 nexthdr;
  __u8 hop_limit;

  struct in6_addr saddr;
  struct in6_addr daddr;
};

struct tcp_resources {
  int32_t seq;
  int32_t ack;
};

static uintptr_t syz_extract_tcp_res(uintptr_t a0, uintptr_t a1,
                                     uintptr_t a2)
{

  if (tunfd < 0)
    return (uintptr_t)-1;

  char data[SYZ_TUN_MAX_PACKET_SIZE];
  int rv = read_tun(&data[0], sizeof(data));
  if (rv == -1)
    return (uintptr_t)-1;
  size_t length = rv;

  struct tcphdr* tcphdr;

  if (length < sizeof(struct ethhdr))
    return (uintptr_t)-1;
  struct ethhdr* ethhdr = (struct ethhdr*)&data[0];

  if (ethhdr->h_proto == htons(ETH_P_IP)) {
    if (length < sizeof(struct ethhdr) + sizeof(struct iphdr))
      return (uintptr_t)-1;
    struct iphdr* iphdr = (struct iphdr*)&data[sizeof(struct ethhdr)];
    if (iphdr->protocol != IPPROTO_TCP)
      return (uintptr_t)-1;
    if (length <
        sizeof(struct ethhdr) + iphdr->ihl * 4 + sizeof(struct tcphdr))
      return (uintptr_t)-1;
    tcphdr =
        (struct tcphdr*)&data[sizeof(struct ethhdr) + iphdr->ihl * 4];
  } else {
    if (length < sizeof(struct ethhdr) + sizeof(struct ipv6hdr))
      return (uintptr_t)-1;
    struct ipv6hdr* ipv6hdr =
        (struct ipv6hdr*)&data[sizeof(struct ethhdr)];
    if (ipv6hdr->nexthdr != IPPROTO_TCP)
      return (uintptr_t)-1;
    if (length < sizeof(struct ethhdr) + sizeof(struct ipv6hdr) +
                     sizeof(struct tcphdr))
      return (uintptr_t)-1;
    tcphdr = (struct tcphdr*)&data[sizeof(struct ethhdr) +
                                   sizeof(struct ipv6hdr)];
  }

  struct tcp_resources* res = (struct tcp_resources*)a0;
  res->seq = htonl((ntohl(tcphdr->seq) + (uint32_t)a1));
  res->ack = htonl((ntohl(tcphdr->ack_seq) + (uint32_t)a2));

  return 0;
}

static void test();

void loop()
{
  while (1) {
    test();
  }
}

long r[116];
void test()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  r[1] = syscall(__NR_socket, 0x2ul, 0x1ul, 0x0ul);
  *(uint16_t*)0x20001000 = (uint16_t)0x2;
  *(uint16_t*)0x20001002 = (uint16_t)0x204e;
  *(uint32_t*)0x20001004 = (uint32_t)0x0;
  *(uint8_t*)0x20001008 = (uint8_t)0x0;
  *(uint8_t*)0x20001009 = (uint8_t)0x0;
  *(uint8_t*)0x2000100a = (uint8_t)0x0;
  *(uint8_t*)0x2000100b = (uint8_t)0x0;
  *(uint8_t*)0x2000100c = (uint8_t)0x0;
  *(uint8_t*)0x2000100d = (uint8_t)0x0;
  *(uint8_t*)0x2000100e = (uint8_t)0x0;
  *(uint8_t*)0x2000100f = (uint8_t)0x0;
  r[13] = syscall(__NR_bind, r[1], 0x20001000ul, 0x10ul);
  r[14] = syscall(__NR_listen, r[1], 0x8ul);
  *(uint8_t*)0x20002000 = (uint8_t)0xaa;
  *(uint8_t*)0x20002001 = (uint8_t)0xaa;
  *(uint8_t*)0x20002002 = (uint8_t)0xaa;
  *(uint8_t*)0x20002003 = (uint8_t)0xaa;
  *(uint8_t*)0x20002004 = (uint8_t)0xaa;
  *(uint8_t*)0x20002005 = (uint8_t)0x0;
  memcpy((void*)0x20002006, "\x4c\x61\x12\xcc\x15\xd8", 6);
  *(uint16_t*)0x2000200c = (uint16_t)0x8;
  STORE_BY_BITMASK(uint8_t, 0x2000200e, 0x5, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x2000200e, 0x4, 4, 4);
  STORE_BY_BITMASK(uint8_t, 0x2000200f, 0x0, 0, 2);
  STORE_BY_BITMASK(uint8_t, 0x2000200f, 0x0, 2, 6);
  *(uint16_t*)0x20002010 = (uint16_t)0x2c00;
  *(uint16_t*)0x20002012 = (uint16_t)0x6400;
  *(uint16_t*)0x20002014 = (uint16_t)0x0;
  *(uint8_t*)0x20002016 = (uint8_t)0x0;
  *(uint8_t*)0x20002017 = (uint8_t)0x6;
  *(uint16_t*)0x20002018 = (uint16_t)0x0;
  *(uint8_t*)0x2000201a = (uint8_t)0xac;
  *(uint8_t*)0x2000201b = (uint8_t)0x14;
  *(uint8_t*)0x2000201c = (uint8_t)0x0;
  *(uint8_t*)0x2000201d = (uint8_t)0xbb;
  *(uint8_t*)0x2000201e = (uint8_t)0xac;
  *(uint8_t*)0x2000201f = (uint8_t)0x14;
  *(uint8_t*)0x20002020 = (uint8_t)0x0;
  *(uint8_t*)0x20002021 = (uint8_t)0xaa;
  *(uint16_t*)0x20002022 = (uint16_t)0x214e;
  *(uint16_t*)0x20002024 = (uint16_t)0x204e;
  *(uint32_t*)0x20002026 = (uint32_t)0x42424242;
  *(uint32_t*)0x2000202a = (uint32_t)0x42424242;
  STORE_BY_BITMASK(uint8_t, 0x2000202e, 0x0, 0, 1);
  STORE_BY_BITMASK(uint8_t, 0x2000202e, 0x0, 1, 3);
  STORE_BY_BITMASK(uint8_t, 0x2000202e, 0x6, 4, 4);
  *(uint8_t*)0x2000202f = (uint8_t)0x2;
  *(uint16_t*)0x20002030 = (uint16_t)0x0;
  *(uint16_t*)0x20002032 = (uint16_t)0x0;
  *(uint16_t*)0x20002034 = (uint16_t)0x0;
  *(uint8_t*)0x20002036 = (uint8_t)0x22;
  *(uint8_t*)0x20002037 = (uint8_t)0x2;
  struct csum_inet csum_54;
  csum_inet_init(&csum_54);
  csum_inet_update(&csum_54, (const uint8_t*)0x2000201a, 4);
  csum_inet_update(&csum_54, (const uint8_t*)0x2000201e, 4);
  uint16_t csum_54_chunk_2 = 0x600;
  csum_inet_update(&csum_54, (const uint8_t*)&csum_54_chunk_2, 2);
  uint16_t csum_54_chunk_3 = 0x1800;
  csum_inet_update(&csum_54, (const uint8_t*)&csum_54_chunk_3, 2);
  csum_inet_update(&csum_54, (const uint8_t*)0x20002022, 24);
  *(uint16_t*)0x20002032 = csum_inet_digest(&csum_54);
  struct csum_inet csum_55;
  csum_inet_init(&csum_55);
  csum_inet_update(&csum_55, (const uint8_t*)0x2000200e, 20);
  *(uint16_t*)0x20002018 = csum_inet_digest(&csum_55);
  r[56] = syz_emit_ethernet(0x3aul, 0x20002000ul);
  r[57] = syz_extract_tcp_res(0x20017000ul, 0x1ul, 0x8ul);
  if (r[57] != -1)
    r[58] = *(uint32_t*)0x20017000;
  if (r[57] != -1)
    r[59] = *(uint32_t*)0x20017004;
  *(uint16_t*)0x20016ff0 = (uint16_t)0x2;
  *(uint64_t*)0x20016ff8 = (uint64_t)0x20016000;
  *(uint16_t*)0x20016000 = (uint16_t)0x10000;
  *(uint8_t*)0x20016002 = (uint8_t)0x6;
  *(uint8_t*)0x20016003 = (uint8_t)0x9958;
  *(uint32_t*)0x20016004 = (uint32_t)0x1;
  *(uint16_t*)0x20016008 = (uint16_t)0x6;
  *(uint8_t*)0x2001600a = (uint8_t)0xffffffffffffffff;
  *(uint8_t*)0x2001600b = (uint8_t)0x6;
  *(uint32_t*)0x2001600c = (uint32_t)0x0;
  r[70] = syscall(__NR_setsockopt, r[1], 0x1ul, 0x1aul, 0x20016ff0ul,
                  0x10ul);
  *(uint8_t*)0x20004000 = (uint8_t)0xaa;
  *(uint8_t*)0x20004001 = (uint8_t)0xaa;
  *(uint8_t*)0x20004002 = (uint8_t)0xaa;
  *(uint8_t*)0x20004003 = (uint8_t)0xaa;
  *(uint8_t*)0x20004004 = (uint8_t)0xaa;
  *(uint8_t*)0x20004005 = (uint8_t)0x0;
  *(uint8_t*)0x20004006 = (uint8_t)0xbb;
  *(uint8_t*)0x20004007 = (uint8_t)0xbb;
  *(uint8_t*)0x20004008 = (uint8_t)0xbb;
  *(uint8_t*)0x20004009 = (uint8_t)0xbb;
  *(uint8_t*)0x2000400a = (uint8_t)0xbb;
  *(uint8_t*)0x2000400b = (uint8_t)0x0;
  *(uint16_t*)0x2000400c = (uint16_t)0x8;
  STORE_BY_BITMASK(uint8_t, 0x2000400e, 0x5, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x2000400e, 0x4, 4, 4);
  STORE_BY_BITMASK(uint8_t, 0x2000400f, 0x0, 0, 2);
  STORE_BY_BITMASK(uint8_t, 0x2000400f, 0x0, 2, 6);
  *(uint16_t*)0x20004010 = (uint16_t)0x2800;
  *(uint16_t*)0x20004012 = (uint16_t)0x6400;
  *(uint16_t*)0x20004014 = (uint16_t)0x0;
  *(uint8_t*)0x20004016 = (uint8_t)0x0;
  *(uint8_t*)0x20004017 = (uint8_t)0x6;
  *(uint16_t*)0x20004018 = (uint16_t)0x0;
  *(uint8_t*)0x2000401a = (uint8_t)0xac;
  *(uint8_t*)0x2000401b = (uint8_t)0x14;
  *(uint8_t*)0x2000401c = (uint8_t)0x0;
  *(uint8_t*)0x2000401d = (uint8_t)0xbb;
  *(uint8_t*)0x2000401e = (uint8_t)0xac;
  *(uint8_t*)0x2000401f = (uint8_t)0x14;
  *(uint8_t*)0x20004020 = (uint8_t)0x0;
  *(uint8_t*)0x20004021 = (uint8_t)0xaa;
  *(uint16_t*)0x20004022 = (uint16_t)0x214e;
  *(uint16_t*)0x20004024 = (uint16_t)0x204e;
  *(uint32_t*)0x20004026 = r[59];
  *(uint32_t*)0x2000402a = r[58];
  STORE_BY_BITMASK(uint8_t, 0x2000402e, 0x0, 0, 1);
  STORE_BY_BITMASK(uint8_t, 0x2000402e, 0x0, 1, 3);
  STORE_BY_BITMASK(uint8_t, 0x2000402e, 0x5, 4, 4);
  *(uint8_t*)0x2000402f = (uint8_t)0x10;
  *(uint16_t*)0x20004030 = (uint16_t)0x0;
  *(uint16_t*)0x20004032 = (uint16_t)0x0;
  *(uint16_t*)0x20004034 = (uint16_t)0x0;
  struct csum_inet csum_113;
  csum_inet_init(&csum_113);
  csum_inet_update(&csum_113, (const uint8_t*)0x2000401a, 4);
  csum_inet_update(&csum_113, (const uint8_t*)0x2000401e, 4);
  uint16_t csum_113_chunk_2 = 0x600;
  csum_inet_update(&csum_113, (const uint8_t*)&csum_113_chunk_2, 2);
  uint16_t csum_113_chunk_3 = 0x1400;
  csum_inet_update(&csum_113, (const uint8_t*)&csum_113_chunk_3, 2);
  csum_inet_update(&csum_113, (const uint8_t*)0x20004022, 20);
  *(uint16_t*)0x20004032 = csum_inet_digest(&csum_113);
  struct csum_inet csum_114;
  csum_inet_init(&csum_114);
  csum_inet_update(&csum_114, (const uint8_t*)0x2000400e, 20);
  *(uint16_t*)0x20004018 = csum_inet_digest(&csum_114);
  r[115] = syz_emit_ethernet(0x36ul, 0x20004000ul);
}

int main()
{
  setup_tun(0, true);
  loop();
  return 0;
}
