// https://syzkaller.appspot.com/bug?id=1a97a5bd119fd97995f752819fd87840ab9479a9
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sched.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/futex.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/net.h>
#include <linux/netfilter_bridge/ebtables.h>
#include <linux/tcp.h>

unsigned long long procid;

static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* ctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  if (__atomic_load_n(&skip_segv, __ATOMIC_RELAXED) &&
      (addr < prog_start || addr > prog_end)) {
    _longjmp(segv_env, 1);
  }
  exit(sig);
}

static void install_segv_handler()
{
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_IGN;
  syscall(SYS_rt_sigaction, 0x20, &sa, NULL, 8);
  syscall(SYS_rt_sigaction, 0x21, &sa, NULL, 8);
  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = segv_handler;
  sa.sa_flags = SA_NODEFER | SA_SIGINFO;
  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGBUS, &sa, NULL);
}

#define NONFAILING(...)                                                        \
  {                                                                            \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    if (_setjmp(segv_env) == 0) {                                              \
      __VA_ARGS__;                                                             \
    }                                                                          \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
  }

static void sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
}

uint64_t current_time_ms()
{
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    exit(1);
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void use_temporary_dir()
{
  char tmpdir_template[] = "./syzkaller.XXXXXX";
  char* tmpdir = mkdtemp(tmpdir_template);
  if (!tmpdir)
    exit(1);
  if (chmod(tmpdir, 0777))
    exit(1);
  if (chdir(tmpdir))
    exit(1);
}

static void thread_start(void* (*fn)(void*), void* arg)
{
  pthread_t th;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 128 << 10);
  if (pthread_create(&th, &attr, fn, arg))
    exit(1);
  pthread_attr_destroy(&attr);
}

typedef struct {
  int state;
} event_t;

static void event_init(event_t* ev)
{
  ev->state = 0;
}

static void event_reset(event_t* ev)
{
  ev->state = 0;
}

static void event_set(event_t* ev)
{
  if (ev->state)
    exit(1);
  __atomic_store_n(&ev->state, 1, __ATOMIC_RELEASE);
  syscall(SYS_futex, &ev->state, FUTEX_WAKE);
}

static void event_wait(event_t* ev)
{
  while (!__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
    syscall(SYS_futex, &ev->state, FUTEX_WAIT, 0, 0);
}

static int event_isset(event_t* ev)
{
  return __atomic_load_n(&ev->state, __ATOMIC_ACQUIRE);
}

static int event_timedwait(event_t* ev, uint64_t timeout)
{
  uint64_t start = current_time_ms();
  uint64_t now = start;
  for (;;) {
    uint64_t remain = timeout - (now - start);
    struct timespec ts;
    ts.tv_sec = remain / 1000;
    ts.tv_nsec = (remain % 1000) * 1000 * 1000;
    syscall(SYS_futex, &ev->state, FUTEX_WAIT, 0, &ts);
    if (__atomic_load_n(&ev->state, __ATOMIC_RELAXED))
      return 1;
    now = current_time_ms();
    if (now - start > timeout)
      return 0;
  }
}

static void vsnprintf_check(char* str, size_t size, const char* format,
                            va_list args)
{
  int rv;
  rv = vsnprintf(str, size, format, args);
  if (rv < 0)
    exit(1);
  if ((size_t)rv >= size)
    exit(1);
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
  va_end(args);
  rv = system(command);
  if (rv) {
    if (panic)
      exit(1);
  }
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
  const int kTunFd = 240;
  if (dup2(tunfd, kTunFd) < 0)
    exit(1);
  close(tunfd);
  tunfd = kTunFd;
  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, TUN_IFACE, IFNAMSIZ);
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI | IFF_NAPI | IFF_NAPI_FRAGS;
  if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0) {
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0)
      exit(1);
  }
  if (ioctl(tunfd, TUNGETIFF, (void*)&ifr) < 0)
    exit(1);
  tun_frags_enabled = (ifr.ifr_flags & IFF_NAPI_FRAGS) != 0;
  execute_command(0, "sysctl -w net.ipv6.conf.%s.accept_dad=0", TUN_IFACE);
  execute_command(0, "sysctl -w net.ipv6.conf.%s.router_solicitations=0",
                  TUN_IFACE);
  execute_command(1, "ip link set dev %s address %s", TUN_IFACE, LOCAL_MAC);
  execute_command(1, "ip addr add %s/24 dev %s", LOCAL_IPV4, TUN_IFACE);
  execute_command(1, "ip neigh add %s lladdr %s dev %s nud permanent",
                  REMOTE_IPV4, REMOTE_MAC, TUN_IFACE);
  execute_command(0, "ip -6 addr add %s/120 dev %s", LOCAL_IPV6, TUN_IFACE);
  execute_command(0, "ip -6 neigh add %s lladdr %s dev %s nud permanent",
                  REMOTE_IPV6, REMOTE_MAC, TUN_IFACE);
  execute_command(1, "ip link set dev %s up", TUN_IFACE);
}

#define DEV_IPV4 "172.20.20.%d"
#define DEV_IPV6 "fe80::%02hx"
#define DEV_MAC "aa:aa:aa:aa:aa:%02hx"

static void snprintf_check(char* str, size_t size, const char* format, ...)
{
  va_list args;
  va_start(args, format);
  vsnprintf_check(str, size, format, args);
  va_end(args);
}
static void initialize_netdevices(void)
{
  unsigned i;
  const char* devtypes[] = {"ip6gretap", "bridge", "vcan", "bond", "team"};
  const char* devnames[] = {"lo",
                            "sit0",
                            "bridge0",
                            "vcan0",
                            "tunl0",
                            "gre0",
                            "gretap0",
                            "ip_vti0",
                            "ip6_vti0",
                            "ip6tnl0",
                            "ip6gre0",
                            "ip6gretap0",
                            "erspan0",
                            "bond0",
                            "veth0",
                            "veth1",
                            "team0",
                            "veth0_to_bridge",
                            "veth1_to_bridge",
                            "veth0_to_bond",
                            "veth1_to_bond",
                            "veth0_to_team",
                            "veth1_to_team"};
  const char* devmasters[] = {"bridge", "bond", "team"};
  for (i = 0; i < sizeof(devtypes) / (sizeof(devtypes[0])); i++)
    execute_command(0, "ip link add dev %s0 type %s", devtypes[i], devtypes[i]);
  execute_command(0, "ip link add type veth");
  for (i = 0; i < sizeof(devmasters) / (sizeof(devmasters[0])); i++) {
    execute_command(
        0, "ip link add name %s_slave_0 type veth peer name veth0_to_%s",
        devmasters[i], devmasters[i]);
    execute_command(
        0, "ip link add name %s_slave_1 type veth peer name veth1_to_%s",
        devmasters[i], devmasters[i]);
    execute_command(0, "ip link set %s_slave_0 master %s0", devmasters[i],
                    devmasters[i]);
    execute_command(0, "ip link set %s_slave_1 master %s0", devmasters[i],
                    devmasters[i]);
    execute_command(0, "ip link set veth0_to_%s up", devmasters[i]);
    execute_command(0, "ip link set veth1_to_%s up", devmasters[i]);
  }
  execute_command(0, "ip link set bridge_slave_0 up");
  execute_command(0, "ip link set bridge_slave_1 up");
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
    exit(1);
  }
  return rv;
}

static void flush_tun()
{
  char data[SYZ_TUN_MAX_PACKET_SIZE];
  while (read_tun(&data[0], sizeof(data)) != -1) {
  }
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
  if (fd == -1) {
    switch (errno) {
    case EAFNOSUPPORT:
    case ENOPROTOOPT:
      return;
    }
    exit(1);
  }
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
      exit(1);
    }
    if (table->info.size > sizeof(table->replace.entrytable))
      exit(1);
    if (table->info.num_entries > XT_MAX_ENTRIES)
      exit(1);
    memset(&entries, 0, sizeof(entries));
    strcpy(entries.name, table->name);
    entries.size = table->info.size;
    optlen = sizeof(entries) - sizeof(entries.entrytable) + table->info.size;
    if (getsockopt(fd, level, IPT_SO_GET_ENTRIES, &entries, &optlen))
      exit(1);
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
  if (fd == -1) {
    switch (errno) {
    case EAFNOSUPPORT:
    case ENOPROTOOPT:
      return;
    }
    exit(1);
  }
  for (i = 0; i < num_tables; i++) {
    struct ipt_table_desc* table = &tables[i];
    if (table->info.valid_hooks == 0)
      continue;
    memset(&info, 0, sizeof(info));
    strcpy(info.name, table->name);
    optlen = sizeof(info);
    if (getsockopt(fd, level, IPT_SO_GET_INFO, &info, &optlen))
      exit(1);
    if (memcmp(&table->info, &info, sizeof(table->info)) == 0) {
      memset(&entries, 0, sizeof(entries));
      strcpy(entries.name, table->name);
      entries.size = table->info.size;
      optlen = sizeof(entries) - sizeof(entries.entrytable) + entries.size;
      if (getsockopt(fd, level, IPT_SO_GET_ENTRIES, &entries, &optlen))
        exit(1);
      if (memcmp(table->replace.entrytable, entries.entrytable,
                 table->info.size) == 0)
        continue;
    }
    table->replace.num_counters = info.num_entries;
    table->replace.counters = counters;
    optlen = sizeof(table->replace) - sizeof(table->replace.entrytable) +
             table->replace.size;
    if (setsockopt(fd, level, IPT_SO_SET_REPLACE, &table->replace, optlen))
      exit(1);
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
  if (fd == -1) {
    switch (errno) {
    case EAFNOSUPPORT:
    case ENOPROTOOPT:
      return;
    }
    exit(1);
  }
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
      exit(1);
    }
    if (table->info.size > sizeof(table->replace.entrytable))
      exit(1);
    if (table->info.num_entries > XT_MAX_ENTRIES)
      exit(1);
    memset(&entries, 0, sizeof(entries));
    strcpy(entries.name, table->name);
    entries.size = table->info.size;
    optlen = sizeof(entries) - sizeof(entries.entrytable) + table->info.size;
    if (getsockopt(fd, SOL_IP, ARPT_SO_GET_ENTRIES, &entries, &optlen))
      exit(1);
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
  if (fd == -1) {
    switch (errno) {
    case EAFNOSUPPORT:
    case ENOPROTOOPT:
      return;
    }
    exit(1);
  }
  for (i = 0; i < sizeof(arpt_tables) / sizeof(arpt_tables[0]); i++) {
    struct arpt_table_desc* table = &arpt_tables[i];
    if (table->info.valid_hooks == 0)
      continue;
    memset(&info, 0, sizeof(info));
    strcpy(info.name, table->name);
    optlen = sizeof(info);
    if (getsockopt(fd, SOL_IP, ARPT_SO_GET_INFO, &info, &optlen))
      exit(1);
    if (memcmp(&table->info, &info, sizeof(table->info)) == 0) {
      memset(&entries, 0, sizeof(entries));
      strcpy(entries.name, table->name);
      entries.size = table->info.size;
      optlen = sizeof(entries) - sizeof(entries.entrytable) + entries.size;
      if (getsockopt(fd, SOL_IP, ARPT_SO_GET_ENTRIES, &entries, &optlen))
        exit(1);
      if (memcmp(table->replace.entrytable, entries.entrytable,
                 table->info.size) == 0)
        continue;
    }
    table->replace.num_counters = info.num_entries;
    table->replace.counters = counters;
    optlen = sizeof(table->replace) - sizeof(table->replace.entrytable) +
             table->replace.size;
    if (setsockopt(fd, SOL_IP, ARPT_SO_SET_REPLACE, &table->replace, optlen))
      exit(1);
  }
  close(fd);
}

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
  if (fd == -1) {
    switch (errno) {
    case EAFNOSUPPORT:
    case ENOPROTOOPT:
      return;
    }
    exit(1);
  }
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
      exit(1);
    }
    if (table->replace.entries_size > sizeof(table->entrytable))
      exit(1);
    table->replace.num_counters = 0;
    table->replace.entries = table->entrytable;
    optlen = sizeof(table->replace) + table->replace.entries_size;
    if (getsockopt(fd, SOL_IP, EBT_SO_GET_INIT_ENTRIES, &table->replace,
                   &optlen))
      exit(1);
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
  if (fd == -1) {
    switch (errno) {
    case EAFNOSUPPORT:
    case ENOPROTOOPT:
      return;
    }
    exit(1);
  }
  for (i = 0; i < sizeof(ebt_tables) / sizeof(ebt_tables[0]); i++) {
    struct ebt_table_desc* table = &ebt_tables[i];
    if (table->replace.valid_hooks == 0)
      continue;
    memset(&replace, 0, sizeof(replace));
    strcpy(replace.name, table->name);
    optlen = sizeof(replace);
    if (getsockopt(fd, SOL_IP, EBT_SO_GET_INFO, &replace, &optlen))
      exit(1);
    replace.num_counters = 0;
    table->replace.entries = 0;
    for (h = 0; h < NF_BR_NUMHOOKS; h++)
      table->replace.hook_entry[h] = 0;
    if (memcmp(&table->replace, &replace, sizeof(table->replace)) == 0) {
      memset(&entrytable, 0, sizeof(entrytable));
      replace.entries = entrytable;
      optlen = sizeof(replace) + replace.entries_size;
      if (getsockopt(fd, SOL_IP, EBT_SO_GET_ENTRIES, &replace, &optlen))
        exit(1);
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
      exit(1);
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

static void setup_common()
{
  checkpoint_net_namespace();
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();
  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = 160 << 20;
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 8 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 136 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 256;
  setrlimit(RLIMIT_NOFILE, &rlim);
  if (unshare(CLONE_NEWNS)) {
  }
  if (unshare(CLONE_NEWIPC)) {
  }
  if (unshare(0x02000000)) {
  }
  if (unshare(CLONE_NEWUTS)) {
  }
  if (unshare(CLONE_SYSVSEM)) {
  }
}

int wait_for_loop(int pid)
{
  if (pid < 0)
    exit(1);
  int status = 0;
  while (waitpid(-1, &status, __WALL) != pid) {
  }
  return WEXITSTATUS(status);
}

static int do_sandbox_none(void)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid != 0)
    return wait_for_loop(pid);
  setup_common();
  sandbox_common();
  if (unshare(CLONE_NEWNET)) {
  }
  initialize_tun();
  initialize_netdevices();
  loop();
  exit(1);
}

static void remove_dir(const char* dir)
{
  DIR* dp;
  struct dirent* ep;
  int iter = 0;
retry:
  while (umount2(dir, MNT_DETACH) == 0) {
  }
  dp = opendir(dir);
  if (dp == NULL) {
    if (errno == EMFILE) {
      exit(1);
    }
    exit(1);
  }
  while ((ep = readdir(dp))) {
    if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
      continue;
    char filename[FILENAME_MAX];
    snprintf(filename, sizeof(filename), "%s/%s", dir, ep->d_name);
    while (umount2(filename, MNT_DETACH) == 0) {
    }
    struct stat st;
    if (lstat(filename, &st))
      exit(1);
    if (S_ISDIR(st.st_mode)) {
      remove_dir(filename);
      continue;
    }
    int i;
    for (i = 0;; i++) {
      if (unlink(filename) == 0)
        break;
      if (errno == EROFS) {
        break;
      }
      if (errno != EBUSY || i > 100)
        exit(1);
      if (umount2(filename, MNT_DETACH))
        exit(1);
    }
  }
  closedir(dp);
  int i;
  for (i = 0;; i++) {
    if (rmdir(dir) == 0)
      break;
    if (i < 100) {
      if (errno == EROFS) {
        break;
      }
      if (errno == EBUSY) {
        if (umount2(dir, MNT_DETACH))
          exit(1);
        continue;
      }
      if (errno == ENOTEMPTY) {
        if (iter < 100) {
          iter++;
          goto retry;
        }
      }
    }
    exit(1);
  }
}

#define SYZ_HAVE_RESET_LOOP 1
static void reset_loop()
{
  reset_net_namespace();
}

#define SYZ_HAVE_SETUP_TEST 1
static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  flush_tun();
}

#define SYZ_HAVE_RESET_TEST 1
static void reset_test()
{
  int fd;
  for (fd = 3; fd < 30; fd++)
    close(fd);
}

struct thread_t {
  int created, call;
  event_t ready, done;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    event_wait(&th->ready);
    event_reset(&th->ready);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    event_set(&th->done);
  }
  return 0;
}

static void execute_one()
{
  int call, thread;
  int collide = 0;
again:
  for (call = 0; call < 7; call++) {
    for (thread = 0; thread < sizeof(threads) / sizeof(threads[0]); thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        event_init(&th->ready);
        event_init(&th->done);
        event_set(&th->done);
        thread_start(thr, th);
      }
      if (!event_isset(&th->done))
        continue;
      event_reset(&th->done);
      th->call = call;
      __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
      event_set(&th->ready);
      if (collide && (call % 2) == 0)
        break;
      event_timedwait(&th->done, 25);
      if (__atomic_load_n(&running, __ATOMIC_RELAXED))
        sleep_ms((call == 7 - 1) ? 10 : 2);
      break;
    }
  }
  if (!collide) {
    collide = 1;
    goto again;
  }
}

static void execute_one();

#define WAIT_FLAGS __WALL

static void loop()
{
  int iter;
  for (iter = 0;; iter++) {
    char cwdbuf[32];
    sprintf(cwdbuf, "./%d", iter);
    if (mkdir(cwdbuf, 0777))
      exit(1);
    reset_loop();
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      setup_test();
      if (chdir(cwdbuf))
        exit(1);
      execute_one();
      reset_test();
      exit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      if (waitpid(-1, &status, WNOHANG | WAIT_FLAGS) == pid)
        break;
      sleep_ms(1);
      if (current_time_ms() - start < 5 * 1000)
        continue;
      kill(-pid, SIGKILL);
      kill(pid, SIGKILL);
      while (waitpid(-1, &status, WAIT_FLAGS) != pid) {
      }
      break;
    }
    remove_dir(cwdbuf);
  }
}

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0x0};

void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    res = syscall(__NR_socket, 2, 2, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    NONFAILING(*(uint32_t*)0x20000000 = 1);
    NONFAILING(memcpy(
        (void*)0x20000004,
        "\x73\x79\x7a\x5f\x74\x75\x6e\x00\x00\x00\x00\x00\x00\x00\x00\x00",
        16));
    NONFAILING(*(uint32_t*)0x20000014 = 0);
    syscall(__NR_setsockopt, r[0], 0, 0x48b, 0x20000000, 0x18);
    break;
  case 2:
    NONFAILING(*(uint32_t*)0x20000080 = 1);
    NONFAILING(memcpy(
        (void*)0x20000084,
        "\x69\x72\x6c\x61\x6e\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
        16));
    NONFAILING(*(uint32_t*)0x20000094 = 0);
    syscall(__NR_setsockopt, r[0], 0, 0x48c, 0x20000080, 0x18);
    break;
  case 3:
    res = syscall(__NR_inotify_init1, 0x80800);
    if (res != -1)
      r[1] = res;
    break;
  case 4:
    syscall(__NR_fcntl, r[1], 8, -1);
    break;
  case 5:
    res = syscall(__NR_fcntl, r[1], 0x10, 0x20000300);
    if (res != -1)
      NONFAILING(r[2] = *(uint32_t*)0x20000304);
    break;
  case 6:
    NONFAILING(*(uint32_t*)0x20000040 = 0);
    NONFAILING(*(uint32_t*)0x20000044 = 0);
    NONFAILING(*(uint32_t*)0x20000048 = -1);
    NONFAILING(*(uint32_t*)0x2000004c = 0);
    syscall(__NR_rt_sigqueueinfo, r[2], 0xb, 0x20000040);
    break;
  }
}
int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  install_segv_handler();
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      use_temporary_dir();
      do_sandbox_none();
    }
  }
  sleep(1000000);
  return 0;
}
