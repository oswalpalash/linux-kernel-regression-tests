// https://syzkaller.appspot.com/bug?id=912f70e859dfffd2533b7cfcd2c71e0dd6f71e9d
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/net.h>
#include <linux/tcp.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <time.h>
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

static uint64_t current_time_ms()
{
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    fail("clock_gettime failed");
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
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

static int read_tun(char* data, int size)
{
  if (tunfd < 0)
    return -1;

  int rv = read(tunfd, data, size);
  if (rv < 0) {
    if (errno == EAGAIN)
      return -1;
    if (errno == EBADFD)
      return -1;
    fail("tun: read failed with %d", rv);
  }
  return rv;
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

static void flush_tun()
{
  char data[SYZ_TUN_MAX_PACKET_SIZE];
  while (read_tun(&data[0], sizeof(data)) != -1)
    ;
}

struct ipt_getinfo {
  char name[32];
  unsigned int valid_hooks;
  unsigned int hook_entry[5];
  unsigned int underflow[5];
  unsigned int num_entries;
  unsigned int size;
};

struct ipt_get_entries {
  char name[32];
  unsigned int size;
  void* entrytable[1024 / sizeof(void*)];
};

struct xt_counters {
  uint64_t pcnt, bcnt;
};

struct ipt_replace {
  char name[32];
  unsigned int valid_hooks;
  unsigned int num_entries;
  unsigned int size;
  unsigned int hook_entry[5];
  unsigned int underflow[5];
  unsigned int num_counters;
  struct xt_counters* counters;
  char entrytable[1024];
};

struct ipt_table_desc {
  const char* name;
  struct ipt_getinfo info;
  struct ipt_get_entries entries;
  struct ipt_replace replace;
  struct xt_counters counters[10];
};

static struct ipt_table_desc ipv4_tables[] = {
    {.name = "filter"}, {.name = "nat"},      {.name = "mangle"},
    {.name = "raw"},    {.name = "security"},
};

#define IPT_BASE_CTL 64
#define IPT_SO_SET_REPLACE (IPT_BASE_CTL)
#define IPT_SO_GET_INFO (IPT_BASE_CTL)
#define IPT_SO_GET_ENTRIES (IPT_BASE_CTL + 1)

static void checkpoint_net_namespace(void)
{
  socklen_t optlen;
  unsigned i;
  int fd;

  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1)
    fail("socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)");
  for (i = 0; i < sizeof(ipv4_tables) / sizeof(ipv4_tables[0]); i++) {
    struct ipt_table_desc* table = &ipv4_tables[i];
    strcpy(table->info.name, table->name);
    strcpy(table->entries.name, table->name);
    strcpy(table->replace.name, table->name);
    optlen = sizeof(table->info);
    if (getsockopt(fd, SOL_IP, IPT_SO_GET_INFO, &table->info, &optlen)) {
      switch (errno) {
      case EPERM:
      case ENOENT:
      case ENOPROTOOPT:
        continue;
      }
      fail("getsockopt(IPT_SO_GET_INFO)");
    }
    if (table->info.size > sizeof(table->entries.entrytable))
      fail("table size is too large: %u", table->info.size);
    if (table->info.num_entries >
        sizeof(table->counters) / sizeof(table->counters[0]))
      fail("too many counters: %u", table->info.num_entries);
    table->entries.size = table->info.size;
    optlen = sizeof(table->entries) - sizeof(table->entries.entrytable) +
             table->info.size;
    if (getsockopt(fd, SOL_IP, IPT_SO_GET_ENTRIES, &table->entries, &optlen))
      fail("getsockopt(IPT_SO_GET_ENTRIES)");
    table->replace.valid_hooks = table->info.valid_hooks;
    table->replace.num_entries = table->info.num_entries;
    table->replace.counters = table->counters;
    table->replace.size = table->info.size;
    memcpy(table->replace.hook_entry, table->info.hook_entry,
           sizeof(table->replace.hook_entry));
    memcpy(table->replace.underflow, table->info.underflow,
           sizeof(table->replace.underflow));
    memcpy(table->replace.entrytable, table->entries.entrytable,
           table->info.size);
  }
  close(fd);
}

static void reset_net_namespace(void)
{
  struct ipt_get_entries entries;
  struct ipt_getinfo info;
  socklen_t optlen;
  unsigned i;
  int fd;

  memset(&info, 0, sizeof(info));
  memset(&entries, 0, sizeof(entries));
  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1)
    fail("socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)");
  for (i = 0; i < sizeof(ipv4_tables) / sizeof(ipv4_tables[0]); i++) {
    struct ipt_table_desc* table = &ipv4_tables[i];
    if (table->info.valid_hooks == 0)
      continue;
    strcpy(info.name, table->name);
    optlen = sizeof(info);
    if (getsockopt(fd, SOL_IP, IPT_SO_GET_INFO, &info, &optlen))
      fail("getsockopt(IPT_SO_GET_INFO)");
    if (memcmp(&table->info, &info, sizeof(table->info)) == 0) {
      strcpy(entries.name, table->name);
      entries.size = table->info.size;
      optlen = sizeof(entries) - sizeof(entries.entrytable) + entries.size;
      if (getsockopt(fd, SOL_IP, IPT_SO_GET_ENTRIES, &entries, &optlen))
        fail("getsockopt(IPT_SO_GET_ENTRIES)");
      if (memcmp(&table->entries, &entries, optlen) == 0)
        continue;
    }
    table->replace.num_counters = info.num_entries;
    optlen = sizeof(table->replace) - sizeof(table->replace.entrytable) +
             table->replace.size;
    if (setsockopt(fd, SOL_IP, IPT_SO_SET_REPLACE, &table->replace, optlen))
      fail("setsockopt(IPT_SO_SET_REPLACE)");
  }
  close(fd);
}

static void test();

void loop()
{
  int iter;
  checkpoint_net_namespace();
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      fail("loop fork failed");
    if (pid == 0) {
      prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
      setpgrp();
      flush_tun();
      test();
      doexit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      int res = waitpid(-1, &status, __WALL | WNOHANG);
      if (res == pid)
        break;
      usleep(1000);
      if (current_time_ms() - start > 5 * 1000) {
        kill(-pid, SIGKILL);
        kill(pid, SIGKILL);
        while (waitpid(-1, &status, __WALL) != pid) {
        }
        break;
      }
    }
    reset_net_namespace();
  }
}

long r[3];
void test()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 0xa, 1, 0);
  *(uint32_t*)0x20000000 = 0x1e;
  syscall(__NR_getsockopt, r[0], 0x29, 0x45, 0x20ab2fe2, 0x20000000);
  r[1] = syscall(__NR_socket, 2, 3, 0x81);
  memcpy((void*)0x2000b000, "\x66\x69\x6c\x74\x65\x72\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x2000b020 = 0xe;
  *(uint32_t*)0x2000b024 = 4;
  *(uint32_t*)0x2000b028 = 0x298;
  *(uint32_t*)0x2000b02c = -1;
  *(uint32_t*)0x2000b030 = 0;
  *(uint32_t*)0x2000b034 = 0;
  *(uint32_t*)0x2000b038 = 0;
  *(uint32_t*)0x2000b03c = -1;
  *(uint32_t*)0x2000b040 = -1;
  *(uint32_t*)0x2000b044 = 0;
  *(uint32_t*)0x2000b048 = 0;
  *(uint32_t*)0x2000b04c = 0;
  *(uint32_t*)0x2000b050 = -1;
  *(uint32_t*)0x2000b054 = 4;
  *(uint64_t*)0x2000b058 = 0x20012fc0;
  *(uint8_t*)0x2000b060 = 0;
  *(uint8_t*)0x2000b061 = 0;
  *(uint8_t*)0x2000b062 = 0;
  *(uint8_t*)0x2000b063 = 0;
  *(uint8_t*)0x2000b064 = 0;
  *(uint8_t*)0x2000b065 = 0;
  *(uint8_t*)0x2000b066 = 0;
  *(uint8_t*)0x2000b067 = 0;
  *(uint8_t*)0x2000b068 = 0;
  *(uint8_t*)0x2000b069 = 0;
  *(uint8_t*)0x2000b06a = 0;
  *(uint8_t*)0x2000b06b = 0;
  *(uint8_t*)0x2000b06c = 0;
  *(uint8_t*)0x2000b06d = 0;
  *(uint8_t*)0x2000b06e = 0;
  *(uint8_t*)0x2000b06f = 0;
  *(uint8_t*)0x2000b070 = 0;
  *(uint8_t*)0x2000b071 = 0;
  *(uint8_t*)0x2000b072 = 0;
  *(uint8_t*)0x2000b073 = 0;
  *(uint8_t*)0x2000b074 = 0;
  *(uint8_t*)0x2000b075 = 0;
  *(uint8_t*)0x2000b076 = 0;
  *(uint8_t*)0x2000b077 = 0;
  *(uint8_t*)0x2000b078 = 0;
  *(uint8_t*)0x2000b079 = 0;
  *(uint8_t*)0x2000b07a = 0;
  *(uint8_t*)0x2000b07b = 0;
  *(uint8_t*)0x2000b07c = 0;
  *(uint8_t*)0x2000b07d = 0;
  *(uint8_t*)0x2000b07e = 0;
  *(uint8_t*)0x2000b07f = 0;
  *(uint8_t*)0x2000b080 = 0;
  *(uint8_t*)0x2000b081 = 0;
  *(uint8_t*)0x2000b082 = 0;
  *(uint8_t*)0x2000b083 = 0;
  *(uint8_t*)0x2000b084 = 0;
  *(uint8_t*)0x2000b085 = 0;
  *(uint8_t*)0x2000b086 = 0;
  *(uint8_t*)0x2000b087 = 0;
  *(uint8_t*)0x2000b088 = 0;
  *(uint8_t*)0x2000b089 = 0;
  *(uint8_t*)0x2000b08a = 0;
  *(uint8_t*)0x2000b08b = 0;
  *(uint8_t*)0x2000b08c = 0;
  *(uint8_t*)0x2000b08d = 0;
  *(uint8_t*)0x2000b08e = 0;
  *(uint8_t*)0x2000b08f = 0;
  *(uint8_t*)0x2000b090 = 0;
  *(uint8_t*)0x2000b091 = 0;
  *(uint8_t*)0x2000b092 = 0;
  *(uint8_t*)0x2000b093 = 0;
  *(uint8_t*)0x2000b094 = 0;
  *(uint8_t*)0x2000b095 = 0;
  *(uint8_t*)0x2000b096 = 0;
  *(uint8_t*)0x2000b097 = 0;
  *(uint8_t*)0x2000b098 = 0;
  *(uint8_t*)0x2000b099 = 0;
  *(uint8_t*)0x2000b09a = 0;
  *(uint8_t*)0x2000b09b = 0;
  *(uint8_t*)0x2000b09c = 0;
  *(uint8_t*)0x2000b09d = 0;
  *(uint8_t*)0x2000b09e = 0;
  *(uint8_t*)0x2000b09f = 0;
  *(uint8_t*)0x2000b0a0 = 0;
  *(uint8_t*)0x2000b0a1 = 0;
  *(uint8_t*)0x2000b0a2 = 0;
  *(uint8_t*)0x2000b0a3 = 0;
  *(uint8_t*)0x2000b0a4 = 0;
  *(uint8_t*)0x2000b0a5 = 0;
  *(uint8_t*)0x2000b0a6 = 0;
  *(uint8_t*)0x2000b0a7 = 0;
  *(uint8_t*)0x2000b0a8 = 0;
  *(uint8_t*)0x2000b0a9 = 0;
  *(uint8_t*)0x2000b0aa = 0;
  *(uint8_t*)0x2000b0ab = 0;
  *(uint8_t*)0x2000b0ac = 0;
  *(uint8_t*)0x2000b0ad = 0;
  *(uint8_t*)0x2000b0ae = 0;
  *(uint8_t*)0x2000b0af = 0;
  *(uint8_t*)0x2000b0b0 = 0;
  *(uint8_t*)0x2000b0b1 = 0;
  *(uint8_t*)0x2000b0b2 = 0;
  *(uint8_t*)0x2000b0b3 = 0;
  *(uint32_t*)0x2000b0b4 = 0;
  *(uint16_t*)0x2000b0b8 = 0x70;
  *(uint16_t*)0x2000b0ba = 0x98;
  *(uint32_t*)0x2000b0bc = 0;
  *(uint64_t*)0x2000b0c0 = 0;
  *(uint64_t*)0x2000b0c8 = 0;
  *(uint16_t*)0x2000b0d0 = 0x28;
  memcpy((void*)0x2000b0d2, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x2000b0ef = 0;
  *(uint32_t*)0x2000b0f0 = 0xfffffffe;
  *(uint32_t*)0x2000b0f8 = htobe32(0);
  *(uint8_t*)0x2000b0fc = 0xac;
  *(uint8_t*)0x2000b0fd = 0x14;
  *(uint8_t*)0x2000b0fe = 0;
  *(uint8_t*)0x2000b0ff = 0;
  *(uint32_t*)0x2000b100 = htobe32(0);
  *(uint32_t*)0x2000b104 = htobe32(0);
  *(uint8_t*)0x2000b108 = 0x73;
  *(uint8_t*)0x2000b109 = 0x79;
  *(uint8_t*)0x2000b10a = 0x7a;
  *(uint8_t*)0x2000b10b = 0;
  *(uint8_t*)0x2000b10c = 0;
  *(uint8_t*)0x2000b118 = 0x73;
  *(uint8_t*)0x2000b119 = 0x79;
  *(uint8_t*)0x2000b11a = 0x7a;
  *(uint8_t*)0x2000b11b = 0;
  *(uint8_t*)0x2000b11c = 0;
  *(uint8_t*)0x2000b128 = 0;
  *(uint8_t*)0x2000b129 = 0;
  *(uint8_t*)0x2000b12a = 0;
  *(uint8_t*)0x2000b12b = 0;
  *(uint8_t*)0x2000b12c = 0;
  *(uint8_t*)0x2000b12d = 0;
  *(uint8_t*)0x2000b12e = 0;
  *(uint8_t*)0x2000b12f = 0;
  *(uint8_t*)0x2000b130 = 0;
  *(uint8_t*)0x2000b131 = 0;
  *(uint8_t*)0x2000b132 = 0;
  *(uint8_t*)0x2000b133 = 0;
  *(uint8_t*)0x2000b134 = 0;
  *(uint8_t*)0x2000b135 = 0;
  *(uint8_t*)0x2000b136 = 0;
  *(uint8_t*)0x2000b137 = 0;
  *(uint8_t*)0x2000b138 = 0;
  *(uint8_t*)0x2000b139 = 0;
  *(uint8_t*)0x2000b13a = 0;
  *(uint8_t*)0x2000b13b = 0;
  *(uint8_t*)0x2000b13c = 0;
  *(uint8_t*)0x2000b13d = 0;
  *(uint8_t*)0x2000b13e = 0;
  *(uint8_t*)0x2000b13f = 0;
  *(uint8_t*)0x2000b140 = 0;
  *(uint8_t*)0x2000b141 = 0;
  *(uint8_t*)0x2000b142 = 0;
  *(uint8_t*)0x2000b143 = 0;
  *(uint8_t*)0x2000b144 = 0;
  *(uint8_t*)0x2000b145 = 0;
  *(uint8_t*)0x2000b146 = 0;
  *(uint8_t*)0x2000b147 = 0;
  *(uint16_t*)0x2000b148 = 0;
  *(uint8_t*)0x2000b14a = 0;
  *(uint8_t*)0x2000b14b = 0;
  *(uint32_t*)0x2000b14c = 0;
  *(uint16_t*)0x2000b150 = 0x70;
  *(uint16_t*)0x2000b152 = 0x98;
  *(uint32_t*)0x2000b154 = 0;
  *(uint64_t*)0x2000b158 = 0;
  *(uint64_t*)0x2000b160 = 0;
  *(uint16_t*)0x2000b168 = 0x28;
  memcpy((void*)0x2000b16a, "\x43\x4c\x41\x53\x53\x49\x46\x59\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x2000b187 = 0;
  *(uint32_t*)0x2000b188 = 0;
  *(uint32_t*)0x2000b190 = htobe32(0);
  *(uint32_t*)0x2000b194 = htobe32(0xe0000002);
  *(uint32_t*)0x2000b198 = htobe32(0);
  *(uint32_t*)0x2000b19c = htobe32(0);
  *(uint8_t*)0x2000b1a0 = 0x73;
  *(uint8_t*)0x2000b1a1 = 0x79;
  *(uint8_t*)0x2000b1a2 = 0x7a;
  *(uint8_t*)0x2000b1a3 = 0;
  *(uint8_t*)0x2000b1a4 = 0;
  memcpy((void*)0x2000b1b0,
         "\x6c\x6f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  *(uint8_t*)0x2000b1c0 = 0;
  *(uint8_t*)0x2000b1c1 = 0;
  *(uint8_t*)0x2000b1c2 = 0;
  *(uint8_t*)0x2000b1c3 = 0;
  *(uint8_t*)0x2000b1c4 = 0;
  *(uint8_t*)0x2000b1c5 = 0;
  *(uint8_t*)0x2000b1c6 = 0;
  *(uint8_t*)0x2000b1c7 = 0;
  *(uint8_t*)0x2000b1c8 = 0;
  *(uint8_t*)0x2000b1c9 = 0;
  *(uint8_t*)0x2000b1ca = 0;
  *(uint8_t*)0x2000b1cb = 0;
  *(uint8_t*)0x2000b1cc = 0;
  *(uint8_t*)0x2000b1cd = 0;
  *(uint8_t*)0x2000b1ce = 0;
  *(uint8_t*)0x2000b1cf = 0;
  *(uint8_t*)0x2000b1d0 = 0;
  *(uint8_t*)0x2000b1d1 = 0;
  *(uint8_t*)0x2000b1d2 = 0;
  *(uint8_t*)0x2000b1d3 = 0;
  *(uint8_t*)0x2000b1d4 = 0;
  *(uint8_t*)0x2000b1d5 = 0;
  *(uint8_t*)0x2000b1d6 = 0;
  *(uint8_t*)0x2000b1d7 = 0;
  *(uint8_t*)0x2000b1d8 = 0;
  *(uint8_t*)0x2000b1d9 = 0;
  *(uint8_t*)0x2000b1da = 0;
  *(uint8_t*)0x2000b1db = 0;
  *(uint8_t*)0x2000b1dc = 0;
  *(uint8_t*)0x2000b1dd = 0;
  *(uint8_t*)0x2000b1de = 0;
  *(uint8_t*)0x2000b1df = 0;
  *(uint16_t*)0x2000b1e0 = 0;
  *(uint8_t*)0x2000b1e2 = 0;
  *(uint8_t*)0x2000b1e3 = 0;
  *(uint32_t*)0x2000b1e4 = 0;
  *(uint16_t*)0x2000b1e8 = 0x70;
  *(uint16_t*)0x2000b1ea = 0x98;
  *(uint32_t*)0x2000b1ec = 0;
  *(uint64_t*)0x2000b1f0 = 0;
  *(uint64_t*)0x2000b1f8 = 0;
  *(uint16_t*)0x2000b200 = 0x28;
  memcpy((void*)0x2000b202, "\x52\x45\x4a\x45\x43\x54\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x2000b21f = 0;
  *(uint32_t*)0x2000b220 = 0;
  *(uint32_t*)0x2000b228 = htobe32(0);
  *(uint32_t*)0x2000b22c = htobe32(0xe0000002);
  *(uint32_t*)0x2000b230 = htobe32(0);
  *(uint32_t*)0x2000b234 = htobe32(-1);
  *(uint8_t*)0x2000b238 = 0x73;
  *(uint8_t*)0x2000b239 = 0x79;
  *(uint8_t*)0x2000b23a = 0x7a;
  *(uint8_t*)0x2000b23b = 0x30;
  *(uint8_t*)0x2000b23c = 0;
  memcpy((void*)0x2000b248,
         "\x86\xd3\xd6\x53\xc2\x5e\xa8\xd7\x4a\x86\xad\xd0\x35\xad\xfd\xcf",
         16);
  *(uint8_t*)0x2000b258 = 0;
  *(uint8_t*)0x2000b259 = 0;
  *(uint8_t*)0x2000b25a = 0;
  *(uint8_t*)0x2000b25b = 0;
  *(uint8_t*)0x2000b25c = 0;
  *(uint8_t*)0x2000b25d = 0;
  *(uint8_t*)0x2000b25e = 0;
  *(uint8_t*)0x2000b25f = 0;
  *(uint8_t*)0x2000b260 = 0;
  *(uint8_t*)0x2000b261 = 0;
  *(uint8_t*)0x2000b262 = 0;
  *(uint8_t*)0x2000b263 = 0;
  *(uint8_t*)0x2000b264 = 0;
  *(uint8_t*)0x2000b265 = 0;
  *(uint8_t*)0x2000b266 = 0;
  *(uint8_t*)0x2000b267 = 0;
  *(uint8_t*)0x2000b268 = 0;
  *(uint8_t*)0x2000b269 = 0;
  *(uint8_t*)0x2000b26a = 0;
  *(uint8_t*)0x2000b26b = 0;
  *(uint8_t*)0x2000b26c = 0;
  *(uint8_t*)0x2000b26d = 0;
  *(uint8_t*)0x2000b26e = 0;
  *(uint8_t*)0x2000b26f = 0;
  *(uint8_t*)0x2000b270 = 0;
  *(uint8_t*)0x2000b271 = 0;
  *(uint8_t*)0x2000b272 = 0;
  *(uint8_t*)0x2000b273 = 0;
  *(uint8_t*)0x2000b274 = 0;
  *(uint8_t*)0x2000b275 = 0;
  *(uint8_t*)0x2000b276 = 0;
  *(uint8_t*)0x2000b277 = 0;
  *(uint16_t*)0x2000b278 = 0;
  *(uint8_t*)0x2000b27a = 0;
  *(uint8_t*)0x2000b27b = 0;
  *(uint32_t*)0x2000b27c = 0;
  *(uint16_t*)0x2000b280 = 0x70;
  *(uint16_t*)0x2000b282 = 0xd0;
  *(uint32_t*)0x2000b284 = 0;
  *(uint64_t*)0x2000b288 = 0;
  *(uint64_t*)0x2000b290 = 0;
  *(uint16_t*)0x2000b298 = 0x60;
  memcpy((void*)0x2000b29a, "\x43\x4c\x55\x53\x54\x45\x52\x49\x50\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x2000b2b7 = 0;
  *(uint32_t*)0x2000b2b8 = 1;
  *(uint8_t*)0x2000b2bc = 0;
  *(uint8_t*)0x2000b2bd = 0;
  *(uint8_t*)0x2000b2be = 0;
  *(uint8_t*)0x2000b2bf = 0;
  *(uint8_t*)0x2000b2c0 = 0;
  *(uint8_t*)0x2000b2c1 = 0;
  *(uint16_t*)0x2000b2c2 = 0;
  *(uint16_t*)0x2000b2c4 = 0;
  *(uint16_t*)0x2000b2c6 = 0;
  *(uint16_t*)0x2000b2c8 = 0;
  *(uint16_t*)0x2000b2ca = 0;
  *(uint16_t*)0x2000b2cc = 0;
  *(uint16_t*)0x2000b2ce = 0;
  *(uint16_t*)0x2000b2d0 = 0;
  *(uint16_t*)0x2000b2d2 = 0;
  *(uint16_t*)0x2000b2d4 = 0;
  *(uint16_t*)0x2000b2d6 = 0;
  *(uint16_t*)0x2000b2d8 = 0;
  *(uint16_t*)0x2000b2da = 0;
  *(uint16_t*)0x2000b2dc = 0;
  *(uint16_t*)0x2000b2de = 0;
  *(uint16_t*)0x2000b2e0 = 0;
  *(uint16_t*)0x2000b2e2 = 0;
  *(uint16_t*)0x2000b2e4 = 0;
  *(uint32_t*)0x2000b2e8 = 0;
  *(uint32_t*)0x2000b2ec = 0;
  *(uint64_t*)0x2000b2f0 = 0;
  *(uint64_t*)0x20012fc0 = 0;
  *(uint64_t*)0x20012fc8 = 0;
  *(uint64_t*)0x20012fd0 = 0;
  *(uint64_t*)0x20012fd8 = 0;
  *(uint64_t*)0x20012fe0 = 0;
  *(uint64_t*)0x20012fe8 = 0;
  *(uint64_t*)0x20012ff0 = 0;
  *(uint64_t*)0x20012ff8 = 0;
  syscall(__NR_setsockopt, r[1], 0, 0x40, 0x2000b000, 0x2f8);
  r[2] = syscall(__NR_socket, 0xa, 1, 0x84);
  *(uint32_t*)0x20058000 = 1;
  *(uint16_t*)0x20058008 = 0xa;
  *(uint16_t*)0x2005800a = 0;
  *(uint32_t*)0x2005800c = 0;
  *(uint8_t*)0x20058010 = -1;
  *(uint8_t*)0x20058011 = 1;
  *(uint8_t*)0x20058012 = 0;
  *(uint8_t*)0x20058013 = 0;
  *(uint8_t*)0x20058014 = 0;
  *(uint8_t*)0x20058015 = 0;
  *(uint8_t*)0x20058016 = 0;
  *(uint8_t*)0x20058017 = 0;
  *(uint8_t*)0x20058018 = 0;
  *(uint8_t*)0x20058019 = 0;
  *(uint8_t*)0x2005801a = 0;
  *(uint8_t*)0x2005801b = 0;
  *(uint8_t*)0x2005801c = 0;
  *(uint8_t*)0x2005801d = 0;
  *(uint8_t*)0x2005801e = 0;
  *(uint8_t*)0x2005801f = 1;
  *(uint32_t*)0x20058020 = 0;
  *(uint64_t*)0x20058028 = 0;
  *(uint64_t*)0x20058030 = 0;
  *(uint64_t*)0x20058038 = 0;
  *(uint64_t*)0x20058040 = 0;
  *(uint64_t*)0x20058048 = 0;
  *(uint64_t*)0x20058050 = 0;
  *(uint64_t*)0x20058058 = 0;
  *(uint64_t*)0x20058060 = 0;
  *(uint64_t*)0x20058068 = 0;
  *(uint64_t*)0x20058070 = 0;
  *(uint64_t*)0x20058078 = 0;
  *(uint64_t*)0x20058080 = 0;
  syscall(__NR_setsockopt, r[2], 0x29, 0x2a, 0x20058000, 0x88);
  memcpy((void*)0x2000c000, "\x3b\xa7\x55\x91\xf4\xef", 6);
  *(uint8_t*)0x2000c006 = 0xaa;
  *(uint8_t*)0x2000c007 = 0xaa;
  *(uint8_t*)0x2000c008 = 0xaa;
  *(uint8_t*)0x2000c009 = 0xaa;
  *(uint8_t*)0x2000c00a = 0;
  *(uint8_t*)0x2000c00b = 0;
  *(uint16_t*)0x2000c00c = htobe16(0x800);
  STORE_BY_BITMASK(uint8_t, 0x2000c00e, 5, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x2000c00e, 4, 4, 4);
  STORE_BY_BITMASK(uint8_t, 0x2000c00f, 0, 0, 2);
  STORE_BY_BITMASK(uint8_t, 0x2000c00f, 0, 2, 6);
  *(uint16_t*)0x2000c010 = htobe16(0x24);
  *(uint16_t*)0x2000c012 = 0;
  *(uint16_t*)0x2000c014 = htobe16(0);
  *(uint8_t*)0x2000c016 = 0;
  *(uint8_t*)0x2000c017 = 0;
  *(uint16_t*)0x2000c018 = 0;
  *(uint8_t*)0x2000c01a = 0xac;
  *(uint8_t*)0x2000c01b = 0x14;
  *(uint8_t*)0x2000c01c = 0;
  *(uint8_t*)0x2000c01d = 0xaa;
  *(uint32_t*)0x2000c01e = htobe32(0xfffffff8);
  STORE_BY_BITMASK(uint16_t, 0x2000c022, 0, 0, 1);
  STORE_BY_BITMASK(uint16_t, 0x2000c022, 0, 1, 1);
  STORE_BY_BITMASK(uint16_t, 0x2000c022, 1, 2, 1);
  STORE_BY_BITMASK(uint16_t, 0x2000c022, 0, 3, 1);
  STORE_BY_BITMASK(uint16_t, 0x2000c022, 0, 4, 4);
  STORE_BY_BITMASK(uint16_t, 0x2000c022, 0, 8, 1);
  STORE_BY_BITMASK(uint16_t, 0x2000c022, 0, 9, 4);
  STORE_BY_BITMASK(uint16_t, 0x2000c022, 1, 13, 3);
  *(uint16_t*)0x2000c024 = htobe16(0x880b);
  *(uint16_t*)0x2000c026 = htobe16(0);
  *(uint16_t*)0x2000c028 = htobe16(0);
  STORE_BY_BITMASK(uint16_t, 0x2000c02a, 0, 0, 1);
  STORE_BY_BITMASK(uint16_t, 0x2000c02a, 0, 1, 1);
  STORE_BY_BITMASK(uint16_t, 0x2000c02a, 0, 2, 1);
  STORE_BY_BITMASK(uint16_t, 0x2000c02a, 0, 3, 1);
  STORE_BY_BITMASK(uint16_t, 0x2000c02a, 0, 4, 9);
  STORE_BY_BITMASK(uint16_t, 0x2000c02a, 0, 13, 3);
  *(uint16_t*)0x2000c02c = htobe16(0x800);
  STORE_BY_BITMASK(uint16_t, 0x2000c02e, 0, 0, 1);
  STORE_BY_BITMASK(uint16_t, 0x2000c02e, 0, 1, 1);
  STORE_BY_BITMASK(uint16_t, 0x2000c02e, 0, 2, 1);
  STORE_BY_BITMASK(uint16_t, 0x2000c02e, 0, 3, 1);
  STORE_BY_BITMASK(uint16_t, 0x2000c02e, 0, 4, 9);
  STORE_BY_BITMASK(uint16_t, 0x2000c02e, 0, 13, 3);
  *(uint16_t*)0x2000c030 = htobe16(0x86dd);
  struct csum_inet csum_1;
  csum_inet_init(&csum_1);
  csum_inet_update(&csum_1, (const uint8_t*)0x2000c00e, 20);
  *(uint16_t*)0x2000c018 = csum_inet_digest(&csum_1);
  syz_emit_ethernet(0x32, 0x2000c000, 0x20000000);
}

int main()
{
  for (;;) {
    setup_tun(0, true);
    loop();
  }
}
