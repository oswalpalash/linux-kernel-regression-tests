// https://syzkaller.appspot.com/bug?id=03cac969e671adbca500f9737d1a8656c37c24a4
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <errno.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/futex.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/net.h>
#include <linux/tcp.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <signal.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/wait.h>
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
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
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

#define TUN_IFACE "syz_tun"

#define LOCAL_MAC "aa:aa:aa:aa:aa:aa"
#define REMOTE_MAC "aa:aa:aa:aa:aa:bb"

#define LOCAL_IPV4 "172.20.20.170"
#define REMOTE_IPV4 "172.20.20.187"

#define LOCAL_IPV6 "fe80::aa"
#define REMOTE_IPV6 "fe80::bb"

#define IFF_NAPI 0x0010
#define IFF_NAPI_FRAGS 0x0020

static void initialize_tun(void)
{
  tunfd = open("/dev/net/tun", O_RDWR | O_NONBLOCK);
  if (tunfd == -1) {
    printf("tun: can't open /dev/net/tun: please enable CONFIG_TUN=y\n");
    printf("otherwise fuzzing or reproducing might not work as intended\n");
    return;
  }
  const int kTunFd = 252;
  if (dup2(tunfd, kTunFd) < 0)
    fail("dup2(tunfd, kTunFd) failed");
  close(tunfd);
  tunfd = kTunFd;

  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, TUN_IFACE, IFNAMSIZ);
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI | IFF_NAPI | IFF_NAPI_FRAGS;
  if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0) {
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0)
      fail("tun: ioctl(TUNSETIFF) failed");
  }
  if (ioctl(tunfd, TUNGETIFF, (void*)&ifr) < 0)
    fail("tun: ioctl(TUNGETIFF) failed");
  tun_frags_enabled = (ifr.ifr_flags & IFF_NAPI_FRAGS) != 0;

  execute_command(1, "sysctl -w net.ipv6.conf.%s.accept_dad=0", TUN_IFACE);

  execute_command(1, "sysctl -w net.ipv6.conf.%s.router_solicitations=0",
                  TUN_IFACE);

  execute_command(1, "ip link set dev %s address %s", TUN_IFACE, LOCAL_MAC);
  execute_command(1, "ip addr add %s/24 dev %s", LOCAL_IPV4, TUN_IFACE);
  execute_command(1, "ip -6 addr add %s/120 dev %s", LOCAL_IPV6, TUN_IFACE);
  execute_command(1, "ip neigh add %s lladdr %s dev %s nud permanent",
                  REMOTE_IPV4, REMOTE_MAC, TUN_IFACE);
  execute_command(1, "ip -6 neigh add %s lladdr %s dev %s nud permanent",
                  REMOTE_IPV6, REMOTE_MAC, TUN_IFACE);
  execute_command(1, "ip link set dev %s up", TUN_IFACE);
}

#define DEV_IPV4 "172.20.20.%d"
#define DEV_IPV6 "fe80::%02hx"
#define DEV_MAC "aa:aa:aa:aa:aa:%02hx"

static void initialize_netdevices(void)
{
  unsigned i;
  const char* devtypes[] = {"ip6gretap", "bridge", "vcan", "bond", "veth"};
  const char* devnames[] = {"lo",       "sit0",    "bridge0", "vcan0",
                            "tunl0",    "gre0",    "gretap0", "ip_vti0",
                            "ip6_vti0", "ip6tnl0", "ip6gre0", "ip6gretap0",
                            "erspan0",  "bond0",   "veth0",   "veth1"};

  for (i = 0; i < sizeof(devtypes) / (sizeof(devtypes[0])); i++)
    execute_command(0, "ip link add dev %s0 type %s", devtypes[i], devtypes[i]);
  execute_command(0, "ip link add dev veth1 type veth");
  for (i = 0; i < sizeof(devnames) / (sizeof(devnames[0])); i++) {
    char addr[32];
    snprintf_check(addr, sizeof(addr), DEV_IPV4, i + 10);
    execute_command(0, "ip -4 addr add %s/24 dev %s", addr, devnames[i]);
    snprintf_check(addr, sizeof(addr), DEV_IPV6, i + 10);
    execute_command(0, "ip -6 addr add %s/120 dev %s", addr, devnames[i]);
    snprintf_check(addr, sizeof(addr), DEV_MAC, i + 10);
    execute_command(0, "ip link set dev %s address %s", devnames[i], addr);
    execute_command(0, "ip link set dev %s up", devnames[i]);
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

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();

  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = 128 << 20;
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 8 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);

#define CLONE_NEWCGROUP 0x02000000

  if (unshare(CLONE_NEWNS)) {
  }
  if (unshare(CLONE_NEWIPC)) {
  }
  if (unshare(CLONE_NEWCGROUP)) {
  }
  if (unshare(CLONE_NEWUTS)) {
  }
  if (unshare(CLONE_SYSVSEM)) {
  }
}

static int do_sandbox_none(void)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid < 0)
    fail("sandbox fork failed");
  if (pid)
    return pid;

  sandbox_common();
  if (unshare(CLONE_NEWNET)) {
  }
  initialize_tun();
  initialize_netdevices();

  loop();
  doexit(1);
}

#define XT_TABLE_SIZE 1536
#define XT_MAX_ENTRIES 10

struct xt_counters {
  uint64_t pcnt, bcnt;
};

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
  void* entrytable[XT_TABLE_SIZE / sizeof(void*)];
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
  char entrytable[XT_TABLE_SIZE];
};

struct ipt_table_desc {
  const char* name;
  struct ipt_getinfo info;
  struct ipt_replace replace;
};

static struct ipt_table_desc ipv4_tables[] = {
    {.name = "filter"}, {.name = "nat"},      {.name = "mangle"},
    {.name = "raw"},    {.name = "security"},
};

static struct ipt_table_desc ipv6_tables[] = {
    {.name = "filter"}, {.name = "nat"},      {.name = "mangle"},
    {.name = "raw"},    {.name = "security"},
};

#define IPT_BASE_CTL 64
#define IPT_SO_SET_REPLACE (IPT_BASE_CTL)
#define IPT_SO_GET_INFO (IPT_BASE_CTL)
#define IPT_SO_GET_ENTRIES (IPT_BASE_CTL + 1)

struct arpt_getinfo {
  char name[32];
  unsigned int valid_hooks;
  unsigned int hook_entry[3];
  unsigned int underflow[3];
  unsigned int num_entries;
  unsigned int size;
};

struct arpt_get_entries {
  char name[32];
  unsigned int size;
  void* entrytable[XT_TABLE_SIZE / sizeof(void*)];
};

struct arpt_replace {
  char name[32];
  unsigned int valid_hooks;
  unsigned int num_entries;
  unsigned int size;
  unsigned int hook_entry[3];
  unsigned int underflow[3];
  unsigned int num_counters;
  struct xt_counters* counters;
  char entrytable[XT_TABLE_SIZE];
};

struct arpt_table_desc {
  const char* name;
  struct arpt_getinfo info;
  struct arpt_replace replace;
};

static struct arpt_table_desc arpt_tables[] = {
    {.name = "filter"},
};

#define ARPT_BASE_CTL 96
#define ARPT_SO_SET_REPLACE (ARPT_BASE_CTL)
#define ARPT_SO_GET_INFO (ARPT_BASE_CTL)
#define ARPT_SO_GET_ENTRIES (ARPT_BASE_CTL + 1)

static void checkpoint_iptables(struct ipt_table_desc* tables, int num_tables,
                                int family, int level)
{
  struct ipt_get_entries entries;
  socklen_t optlen;
  int fd, i;

  fd = socket(family, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1)
    fail("socket(%d, SOCK_STREAM, IPPROTO_TCP)", family);
  for (i = 0; i < num_tables; i++) {
    struct ipt_table_desc* table = &tables[i];
    strcpy(table->info.name, table->name);
    strcpy(table->replace.name, table->name);
    optlen = sizeof(table->info);
    if (getsockopt(fd, level, IPT_SO_GET_INFO, &table->info, &optlen)) {
      switch (errno) {
      case EPERM:
      case ENOENT:
      case ENOPROTOOPT:
        continue;
      }
      fail("getsockopt(IPT_SO_GET_INFO)");
    }
    if (table->info.size > sizeof(table->replace.entrytable))
      fail("table size is too large: %u", table->info.size);
    if (table->info.num_entries > XT_MAX_ENTRIES)
      fail("too many counters: %u", table->info.num_entries);
    memset(&entries, 0, sizeof(entries));
    strcpy(entries.name, table->name);
    entries.size = table->info.size;
    optlen = sizeof(entries) - sizeof(entries.entrytable) + table->info.size;
    if (getsockopt(fd, level, IPT_SO_GET_ENTRIES, &entries, &optlen))
      fail("getsockopt(IPT_SO_GET_ENTRIES)");
    table->replace.valid_hooks = table->info.valid_hooks;
    table->replace.num_entries = table->info.num_entries;
    table->replace.size = table->info.size;
    memcpy(table->replace.hook_entry, table->info.hook_entry,
           sizeof(table->replace.hook_entry));
    memcpy(table->replace.underflow, table->info.underflow,
           sizeof(table->replace.underflow));
    memcpy(table->replace.entrytable, entries.entrytable, table->info.size);
  }
  close(fd);
}

static void reset_iptables(struct ipt_table_desc* tables, int num_tables,
                           int family, int level)
{
  struct xt_counters counters[XT_MAX_ENTRIES];
  struct ipt_get_entries entries;
  struct ipt_getinfo info;
  socklen_t optlen;
  int fd, i;

  fd = socket(family, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1)
    fail("socket(%d, SOCK_STREAM, IPPROTO_TCP)", family);
  for (i = 0; i < num_tables; i++) {
    struct ipt_table_desc* table = &tables[i];
    if (table->info.valid_hooks == 0)
      continue;
    memset(&info, 0, sizeof(info));
    strcpy(info.name, table->name);
    optlen = sizeof(info);
    if (getsockopt(fd, level, IPT_SO_GET_INFO, &info, &optlen))
      fail("getsockopt(IPT_SO_GET_INFO)");
    if (memcmp(&table->info, &info, sizeof(table->info)) == 0) {
      memset(&entries, 0, sizeof(entries));
      strcpy(entries.name, table->name);
      entries.size = table->info.size;
      optlen = sizeof(entries) - sizeof(entries.entrytable) + entries.size;
      if (getsockopt(fd, level, IPT_SO_GET_ENTRIES, &entries, &optlen))
        fail("getsockopt(IPT_SO_GET_ENTRIES)");
      if (memcmp(table->replace.entrytable, entries.entrytable,
                 table->info.size) == 0)
        continue;
    }
    table->replace.num_counters = info.num_entries;
    table->replace.counters = counters;
    optlen = sizeof(table->replace) - sizeof(table->replace.entrytable) +
             table->replace.size;
    if (setsockopt(fd, level, IPT_SO_SET_REPLACE, &table->replace, optlen))
      fail("setsockopt(IPT_SO_SET_REPLACE)");
  }
  close(fd);
}

static void checkpoint_arptables(void)
{
  struct arpt_get_entries entries;
  socklen_t optlen;
  unsigned i;
  int fd;

  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1)
    fail("socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)");
  for (i = 0; i < sizeof(arpt_tables) / sizeof(arpt_tables[0]); i++) {
    struct arpt_table_desc* table = &arpt_tables[i];
    strcpy(table->info.name, table->name);
    strcpy(table->replace.name, table->name);
    optlen = sizeof(table->info);
    if (getsockopt(fd, SOL_IP, ARPT_SO_GET_INFO, &table->info, &optlen)) {
      switch (errno) {
      case EPERM:
      case ENOENT:
      case ENOPROTOOPT:
        continue;
      }
      fail("getsockopt(ARPT_SO_GET_INFO)");
    }
    if (table->info.size > sizeof(table->replace.entrytable))
      fail("table size is too large: %u", table->info.size);
    if (table->info.num_entries > XT_MAX_ENTRIES)
      fail("too many counters: %u", table->info.num_entries);
    memset(&entries, 0, sizeof(entries));
    strcpy(entries.name, table->name);
    entries.size = table->info.size;
    optlen = sizeof(entries) - sizeof(entries.entrytable) + table->info.size;
    if (getsockopt(fd, SOL_IP, ARPT_SO_GET_ENTRIES, &entries, &optlen))
      fail("getsockopt(ARPT_SO_GET_ENTRIES)");
    table->replace.valid_hooks = table->info.valid_hooks;
    table->replace.num_entries = table->info.num_entries;
    table->replace.size = table->info.size;
    memcpy(table->replace.hook_entry, table->info.hook_entry,
           sizeof(table->replace.hook_entry));
    memcpy(table->replace.underflow, table->info.underflow,
           sizeof(table->replace.underflow));
    memcpy(table->replace.entrytable, entries.entrytable, table->info.size);
  }
  close(fd);
}

static void reset_arptables()
{
  struct xt_counters counters[XT_MAX_ENTRIES];
  struct arpt_get_entries entries;
  struct arpt_getinfo info;
  socklen_t optlen;
  unsigned i;
  int fd;

  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1)
    fail("socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)");
  for (i = 0; i < sizeof(arpt_tables) / sizeof(arpt_tables[0]); i++) {
    struct arpt_table_desc* table = &arpt_tables[i];
    if (table->info.valid_hooks == 0)
      continue;
    memset(&info, 0, sizeof(info));
    strcpy(info.name, table->name);
    optlen = sizeof(info);
    if (getsockopt(fd, SOL_IP, ARPT_SO_GET_INFO, &info, &optlen))
      fail("getsockopt(ARPT_SO_GET_INFO)");
    if (memcmp(&table->info, &info, sizeof(table->info)) == 0) {
      memset(&entries, 0, sizeof(entries));
      strcpy(entries.name, table->name);
      entries.size = table->info.size;
      optlen = sizeof(entries) - sizeof(entries.entrytable) + entries.size;
      if (getsockopt(fd, SOL_IP, ARPT_SO_GET_ENTRIES, &entries, &optlen))
        fail("getsockopt(ARPT_SO_GET_ENTRIES)");
      if (memcmp(table->replace.entrytable, entries.entrytable,
                 table->info.size) == 0)
        continue;
    }
    table->replace.num_counters = info.num_entries;
    table->replace.counters = counters;
    optlen = sizeof(table->replace) - sizeof(table->replace.entrytable) +
             table->replace.size;
    if (setsockopt(fd, SOL_IP, ARPT_SO_SET_REPLACE, &table->replace, optlen))
      fail("setsockopt(ARPT_SO_SET_REPLACE)");
  }
  close(fd);
}
#include <linux/if.h>
#include <linux/netfilter_bridge/ebtables.h>

struct ebt_table_desc {
  const char* name;
  struct ebt_replace replace;
  char entrytable[XT_TABLE_SIZE];
};

static struct ebt_table_desc ebt_tables[] = {
    {.name = "filter"}, {.name = "nat"}, {.name = "broute"},
};

static void checkpoint_ebtables(void)
{
  socklen_t optlen;
  unsigned i;
  int fd;

  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1)
    fail("socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)");
  for (i = 0; i < sizeof(ebt_tables) / sizeof(ebt_tables[0]); i++) {
    struct ebt_table_desc* table = &ebt_tables[i];
    strcpy(table->replace.name, table->name);
    optlen = sizeof(table->replace);
    if (getsockopt(fd, SOL_IP, EBT_SO_GET_INIT_INFO, &table->replace,
                   &optlen)) {
      switch (errno) {
      case EPERM:
      case ENOENT:
      case ENOPROTOOPT:
        continue;
      }
      fail("getsockopt(EBT_SO_GET_INIT_INFO)");
    }
    if (table->replace.entries_size > sizeof(table->entrytable))
      fail("table size is too large: %u", table->replace.entries_size);
    table->replace.num_counters = 0;
    table->replace.entries = table->entrytable;
    optlen = sizeof(table->replace) + table->replace.entries_size;
    if (getsockopt(fd, SOL_IP, EBT_SO_GET_INIT_ENTRIES, &table->replace,
                   &optlen))
      fail("getsockopt(EBT_SO_GET_INIT_ENTRIES)");
  }
  close(fd);
}

static void reset_ebtables()
{
  struct ebt_replace replace;
  char entrytable[XT_TABLE_SIZE];
  socklen_t optlen;
  unsigned i, j, h;
  int fd;

  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1)
    fail("socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)");
  for (i = 0; i < sizeof(ebt_tables) / sizeof(ebt_tables[0]); i++) {
    struct ebt_table_desc* table = &ebt_tables[i];
    if (table->replace.valid_hooks == 0)
      continue;
    memset(&replace, 0, sizeof(replace));
    strcpy(replace.name, table->name);
    optlen = sizeof(replace);
    if (getsockopt(fd, SOL_IP, EBT_SO_GET_INFO, &replace, &optlen))
      fail("getsockopt(EBT_SO_GET_INFO)");
    replace.num_counters = 0;
    table->replace.entries = 0;
    for (h = 0; h < NF_BR_NUMHOOKS; h++)
      table->replace.hook_entry[h] = 0;
    if (memcmp(&table->replace, &replace, sizeof(table->replace)) == 0) {
      memset(&entrytable, 0, sizeof(entrytable));
      replace.entries = entrytable;
      optlen = sizeof(replace) + replace.entries_size;
      if (getsockopt(fd, SOL_IP, EBT_SO_GET_ENTRIES, &replace, &optlen))
        fail("getsockopt(EBT_SO_GET_ENTRIES)");
      if (memcmp(table->entrytable, entrytable, replace.entries_size) == 0)
        continue;
    }
    for (j = 0, h = 0; h < NF_BR_NUMHOOKS; h++) {
      if (table->replace.valid_hooks & (1 << h)) {
        table->replace.hook_entry[h] =
            (struct ebt_entries*)table->entrytable + j;
        j++;
      }
    }
    table->replace.entries = table->entrytable;
    optlen = sizeof(table->replace) + table->replace.entries_size;
    if (setsockopt(fd, SOL_IP, EBT_SO_SET_ENTRIES, &table->replace, optlen))
      fail("setsockopt(EBT_SO_SET_ENTRIES)");
  }
  close(fd);
}

static void checkpoint_net_namespace(void)
{
  checkpoint_ebtables();
  checkpoint_arptables();
  checkpoint_iptables(ipv4_tables, sizeof(ipv4_tables) / sizeof(ipv4_tables[0]),
                      AF_INET, SOL_IP);
  checkpoint_iptables(ipv6_tables, sizeof(ipv6_tables) / sizeof(ipv6_tables[0]),
                      AF_INET6, SOL_IPV6);
}

static void reset_net_namespace(void)
{
  reset_ebtables();
  reset_arptables();
  reset_iptables(ipv4_tables, sizeof(ipv4_tables) / sizeof(ipv4_tables[0]),
                 AF_INET, SOL_IP);
  reset_iptables(ipv6_tables, sizeof(ipv6_tables) / sizeof(ipv6_tables[0]),
                 AF_INET6, SOL_IPV6);
}

static void execute_one();
extern unsigned long long procid;

static void loop()
{
  checkpoint_net_namespace();
  int iter;
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      fail("clone failed");
    if (pid == 0) {
      prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
      setpgrp();
      flush_tun();
      execute_one();
      doexit(0);
    }

    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      int res = waitpid(-1, &status, __WALL | WNOHANG);
      if (res == pid) {
        break;
      }
      usleep(1000);
      if (current_time_ms() - start < 3 * 1000)
        continue;
      kill(-pid, SIGKILL);
      kill(pid, SIGKILL);
      while (waitpid(-1, &status, __WALL) != pid) {
      }
      break;
    }
    reset_net_namespace();
  }
}

struct thread_t {
  int created, running, call;
  pthread_t th;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    while (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE))
      syscall(SYS_futex, &th->running, FUTEX_WAIT, 0, 0);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    __atomic_store_n(&th->running, 0, __ATOMIC_RELEASE);
    syscall(SYS_futex, &th->running, FUTEX_WAKE);
  }
  return 0;
}

static void execute(int num_calls)
{
  int call, thread;
  running = 0;
  for (call = 0; call < num_calls; call++) {
    for (thread = 0; thread < sizeof(threads) / sizeof(threads[0]); thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 128 << 10);
        pthread_create(&th->th, &attr, thr, th);
      }
      if (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE)) {
        th->call = call;
        __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
        __atomic_store_n(&th->running, 1, __ATOMIC_RELEASE);
        syscall(SYS_futex, &th->running, FUTEX_WAKE);
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 20 * 1000 * 1000;
        syscall(SYS_futex, &th->running, FUTEX_WAIT, 1, &ts);
        if (running)
          usleep((call == num_calls - 1) ? 10000 : 1000);
        break;
      }
    }
  }
}

uint64_t r[1] = {0xffffffffffffffff};
unsigned long long procid;
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    *(uint32_t*)0x20000480 = 0;
    *(uint16_t*)0x20000488 = 2;
    *(uint16_t*)0x2000048a = htobe16(0x4e22);
    *(uint8_t*)0x2000048c = 0xac;
    *(uint8_t*)0x2000048d = 0x14;
    *(uint8_t*)0x2000048e = 0x14;
    *(uint8_t*)0x2000048f = 0x10;
    *(uint8_t*)0x20000490 = 0;
    *(uint8_t*)0x20000491 = 0;
    *(uint8_t*)0x20000492 = 0;
    *(uint8_t*)0x20000493 = 0;
    *(uint8_t*)0x20000494 = 0;
    *(uint8_t*)0x20000495 = 0;
    *(uint8_t*)0x20000496 = 0;
    *(uint8_t*)0x20000497 = 0;
    *(uint16_t*)0x20000508 = 1;
    *(uint16_t*)0x2000050a = 0x7ff;
    syscall(__NR_setsockopt, -1, 0x84, 0x1f, 0x20000480, 0x90);
    break;
  case 1:
    *(uint8_t*)0x20000080 = 0xaa;
    *(uint8_t*)0x20000081 = 0xaa;
    *(uint8_t*)0x20000082 = 0xaa;
    *(uint8_t*)0x20000083 = 0xaa;
    *(uint8_t*)0x20000084 = 0xaa;
    *(uint8_t*)0x20000085 = 0xaa;
    *(uint8_t*)0x20000086 = 0;
    *(uint8_t*)0x20000087 = 0;
    *(uint8_t*)0x20000088 = 0;
    *(uint8_t*)0x20000089 = 0;
    *(uint8_t*)0x2000008a = 0;
    *(uint8_t*)0x2000008b = 0;
    *(uint16_t*)0x2000008c = htobe16(0x800);
    STORE_BY_BITMASK(uint8_t, 0x2000008e, 5, 0, 4);
    STORE_BY_BITMASK(uint8_t, 0x2000008e, 4, 4, 4);
    STORE_BY_BITMASK(uint8_t, 0x2000008f, 0, 0, 2);
    STORE_BY_BITMASK(uint8_t, 0x2000008f, 0, 2, 6);
    *(uint16_t*)0x20000090 = htobe16(0x1c);
    *(uint16_t*)0x20000092 = htobe16(0);
    *(uint16_t*)0x20000094 = htobe16(0);
    *(uint8_t*)0x20000096 = 0;
    *(uint8_t*)0x20000097 = 0x11;
    *(uint16_t*)0x20000098 = 0;
    *(uint8_t*)0x2000009a = 0xac;
    *(uint8_t*)0x2000009b = 0x23;
    *(uint8_t*)0x2000009c = 0x14;
    *(uint8_t*)0x2000009d = 0xaa;
    *(uint8_t*)0x2000009e = 0xac;
    *(uint8_t*)0x2000009f = 0x14;
    *(uint8_t*)0x200000a0 = 0x14;
    *(uint8_t*)0x200000a1 = 0x2b;
    *(uint16_t*)0x200000a2 = htobe16(0x4e20);
    *(uint16_t*)0x200000a4 = htobe16(0x4e20);
    *(uint16_t*)0x200000a6 = htobe16(8);
    *(uint16_t*)0x200000a8 = 0;
    *(uint32_t*)0x200000c0 = 0;
    *(uint32_t*)0x200000c4 = 0;
    *(uint32_t*)0x200000c8 = 0;
    *(uint32_t*)0x200000cc = 0;
    *(uint32_t*)0x200000d0 = 0;
    *(uint32_t*)0x200000d4 = 0;
    struct csum_inet csum_1;
    csum_inet_init(&csum_1);
    csum_inet_update(&csum_1, (const uint8_t*)0x2000009a, 4);
    csum_inet_update(&csum_1, (const uint8_t*)0x2000009e, 4);
    uint16_t csum_1_chunk_2 = 0x1100;
    csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_2, 2);
    uint16_t csum_1_chunk_3 = 0x800;
    csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_3, 2);
    csum_inet_update(&csum_1, (const uint8_t*)0x200000a2, 8);
    *(uint16_t*)0x200000a8 = csum_inet_digest(&csum_1);
    struct csum_inet csum_2;
    csum_inet_init(&csum_2);
    csum_inet_update(&csum_2, (const uint8_t*)0x2000008e, 20);
    *(uint16_t*)0x20000098 = csum_inet_digest(&csum_2);
    syz_emit_ethernet(0x2a, 0x20000080, 0x200000c0);
    break;
  case 2:
    *(uint16_t*)0x20b63fe4 = 0xa;
    *(uint16_t*)0x20b63fe6 = htobe16(0x4e22);
    *(uint32_t*)0x20b63fe8 = 0;
    *(uint8_t*)0x20b63fec = 0;
    *(uint8_t*)0x20b63fed = 0;
    *(uint8_t*)0x20b63fee = 0;
    *(uint8_t*)0x20b63fef = 0;
    *(uint8_t*)0x20b63ff0 = 0;
    *(uint8_t*)0x20b63ff1 = 0;
    *(uint8_t*)0x20b63ff2 = 0;
    *(uint8_t*)0x20b63ff3 = 0;
    *(uint8_t*)0x20b63ff4 = 0;
    *(uint8_t*)0x20b63ff5 = 0;
    *(uint8_t*)0x20b63ff6 = 0;
    *(uint8_t*)0x20b63ff7 = 0;
    *(uint8_t*)0x20b63ff8 = 0;
    *(uint8_t*)0x20b63ff9 = 0;
    *(uint8_t*)0x20b63ffa = 0;
    *(uint8_t*)0x20b63ffb = 0;
    *(uint32_t*)0x20b63ffc = 0;
    syscall(__NR_sendto, -1, 0x20f6f000, 0, 0x20000004, 0x20b63fe4, 0x1c);
    break;
  case 3:
    *(uint64_t*)0x20000480 = 0x20000300;
    *(uint16_t*)0x20000300 = 0x10;
    *(uint16_t*)0x20000302 = 0;
    *(uint32_t*)0x20000304 = 0;
    *(uint32_t*)0x20000308 = 0x20000010;
    *(uint32_t*)0x20000488 = 0xc;
    *(uint64_t*)0x20000490 = 0x20000440;
    *(uint64_t*)0x20000440 = 0x20000340;
    *(uint32_t*)0x20000340 = 0xc8;
    *(uint16_t*)0x20000344 = 0;
    *(uint16_t*)0x20000346 = 0x10;
    *(uint32_t*)0x20000348 = 0x70bd2d;
    *(uint32_t*)0x2000034c = 0x25dfdbfb;
    *(uint8_t*)0x20000350 = 0xf;
    *(uint8_t*)0x20000351 = 0;
    *(uint16_t*)0x20000352 = 0;
    *(uint16_t*)0x20000354 = 0xc;
    *(uint16_t*)0x20000356 = 2;
    *(uint16_t*)0x20000358 = 8;
    *(uint16_t*)0x2000035a = 6;
    *(uint32_t*)0x2000035c = 0x200;
    *(uint16_t*)0x20000360 = 8;
    *(uint16_t*)0x20000362 = 6;
    *(uint32_t*)0x20000364 = 1;
    *(uint16_t*)0x20000368 = 0xc;
    *(uint16_t*)0x2000036a = 3;
    *(uint16_t*)0x2000036c = 8;
    *(uint16_t*)0x2000036e = 4;
    *(uint16_t*)0x20000370 = 4;
    *(uint16_t*)0x20000374 = 0x50;
    *(uint16_t*)0x20000376 = 3;
    *(uint16_t*)0x20000378 = 8;
    *(uint16_t*)0x2000037a = 3;
    *(uint32_t*)0x2000037c = 1;
    *(uint16_t*)0x20000380 = 8;
    *(uint16_t*)0x20000382 = 1;
    *(uint32_t*)0x20000384 = 1;
    *(uint16_t*)0x20000388 = 8;
    *(uint16_t*)0x2000038a = 1;
    *(uint32_t*)0x2000038c = 1;
    *(uint16_t*)0x20000390 = 8;
    *(uint16_t*)0x20000392 = 5;
    *(uint8_t*)0x20000394 = 0xac;
    *(uint8_t*)0x20000395 = 0x14;
    *(uint8_t*)0x20000396 = 0x14;
    *(uint8_t*)0x20000397 = 0xbb;
    *(uint16_t*)0x20000398 = 0x14;
    *(uint16_t*)0x2000039a = 6;
    *(uint8_t*)0x2000039c = -1;
    *(uint8_t*)0x2000039d = 1;
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
    *(uint8_t*)0x200003ab = 1;
    *(uint16_t*)0x200003ac = 8;
    *(uint16_t*)0x200003ae = 4;
    *(uint16_t*)0x200003b0 = 0;
    *(uint16_t*)0x200003b4 = 8;
    *(uint16_t*)0x200003b6 = 7;
    *(uint16_t*)0x200003b8 = htobe16(0x4e24);
    *(uint16_t*)0x200003bc = 8;
    *(uint16_t*)0x200003be = 1;
    *(uint32_t*)0x200003c0 = 0;
    *(uint16_t*)0x200003c4 = 8;
    *(uint16_t*)0x200003c6 = 5;
    *(uint32_t*)0x200003c8 = 1;
    *(uint16_t*)0x200003cc = 0x3c;
    *(uint16_t*)0x200003ce = 3;
    *(uint16_t*)0x200003d0 = 8;
    *(uint16_t*)0x200003d2 = 7;
    *(uint16_t*)0x200003d4 = htobe16(0x4e20);
    *(uint16_t*)0x200003d8 = 8;
    *(uint16_t*)0x200003da = 1;
    *(uint32_t*)0x200003dc = 1;
    *(uint16_t*)0x200003e0 = 8;
    *(uint16_t*)0x200003e2 = 3;
    *(uint32_t*)0x200003e4 = 1;
    *(uint16_t*)0x200003e8 = 8;
    *(uint16_t*)0x200003ea = 8;
    *(uint8_t*)0x200003ec = 9;
    *(uint16_t*)0x200003f0 = 8;
    *(uint16_t*)0x200003f2 = 4;
    *(uint16_t*)0x200003f4 = 8;
    *(uint16_t*)0x200003f8 = 8;
    *(uint16_t*)0x200003fa = 4;
    *(uint16_t*)0x200003fc = 4;
    *(uint16_t*)0x20000400 = 8;
    *(uint16_t*)0x20000402 = 8;
    *(uint8_t*)0x20000404 = 0xdd;
    *(uint64_t*)0x20000448 = 0xc8;
    *(uint64_t*)0x20000498 = 1;
    *(uint64_t*)0x200004a0 = 0;
    *(uint64_t*)0x200004a8 = 0;
    *(uint32_t*)0x200004b0 = 0x80;
    syscall(__NR_sendmsg, -1, 0x20000480, 0);
    break;
  case 4:
    *(uint64_t*)0x20014000 = 0;
    *(uint32_t*)0x20014008 = 0;
    *(uint64_t*)0x20014010 = 0x20000000;
    *(uint64_t*)0x20000000 = 0x20008000;
    memcpy((void*)0x20008000,
           "\x4c\x00\x00\x00\x12\x00\xff\x09\xff\xfe\xfd\x95\x6f\xa2\x83\x00"
           "\x07\xa6\x00\x80\x00\x00\x00\x00\x00\x00\x00\x68\x35\x40\x15\x00"
           "\x24\x00\x1d\x00\x04\xc4\x11\x80\xb5\x98\xbc\x59\x3a\xb6\x82\x11"
           "\x48\xa7\x30\xde\x33\xa4\x98\x68\xc6\x2b\x2c\xa6\x54\xa6\x61\x3b"
           "\x6a\xab\xf3\x5d\x4c\x1c\xbc\x88\x2b\x07\x98\x81",
           76);
    *(uint64_t*)0x20000008 = 0x4c;
    *(uint64_t*)0x20014018 = 1;
    *(uint64_t*)0x20014020 = 0;
    *(uint64_t*)0x20014028 = 0;
    *(uint32_t*)0x20014030 = 0;
    syscall(__NR_sendmsg, -1, 0x20014000, 0);
    break;
  case 5:
    res = syscall(__NR_socket, 2, 3, 2);
    if (res != -1)
      r[0] = res;
    break;
  case 6:
    *(uint32_t*)0x20000100 = 2;
    memcpy((void*)0x20000104,
           "\x76\x63\x61\x6e\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
           16);
    *(uint32_t*)0x20000114 = 3;
    syscall(__NR_setsockopt, r[0], 0, 0x48b, 0x20000100, 0x18);
    break;
  case 7:
    *(uint32_t*)0x20000240 = 2;
    memcpy((void*)0x20000244,
           "\x69\x66\x62\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
           16);
    *(uint32_t*)0x20000254 = 0;
    syscall(__NR_setsockopt, r[0], 0, 0x48c, 0x20000240, 0x18);
    break;
  }
}

void execute_one()
{
  execute(8);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      for (;;) {
        int pid = do_sandbox_none();
        int status = 0;
        while (waitpid(pid, &status, __WALL) != pid) {
        }
      }
    }
  }
  sleep(1000000);
  return 0;
}
