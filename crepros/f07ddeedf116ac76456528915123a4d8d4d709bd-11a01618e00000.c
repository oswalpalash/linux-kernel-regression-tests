// https://syzkaller.appspot.com/bug?id=f07ddeedf116ac76456528915123a4d8d4d709bd
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <sched.h>
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

#include <linux/capability.h>
#include <linux/if_addr.h>
#include <linux/if_ether.h>
#include <linux/if_link.h>
#include <linux/if_tun.h>
#include <linux/in6.h>
#include <linux/ip.h>
#include <linux/neighbour.h>
#include <linux/net.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/tcp.h>
#include <linux/veth.h>

unsigned long long procid;

static void sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
}

static uint64_t current_time_ms(void)
{
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    exit(1);
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

static bool write_file(const char* file, const char* what, ...)
{
  char buf[1024];
  va_list args;
  va_start(args, what);
  vsnprintf(buf, sizeof(buf), what, args);
  va_end(args);
  buf[sizeof(buf) - 1] = 0;
  int len = strlen(buf);
  int fd = open(file, O_WRONLY | O_CLOEXEC);
  if (fd == -1)
    return false;
  if (write(fd, buf, len) != len) {
    int err = errno;
    close(fd);
    errno = err;
    return false;
  }
  close(fd);
  return true;
}

static struct {
  char* pos;
  int nesting;
  struct nlattr* nested[8];
  char buf[1024];
} nlmsg;

static void netlink_init(int typ, int flags, const void* data, int size)
{
  memset(&nlmsg, 0, sizeof(nlmsg));
  struct nlmsghdr* hdr = (struct nlmsghdr*)nlmsg.buf;
  hdr->nlmsg_type = typ;
  hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | flags;
  memcpy(hdr + 1, data, size);
  nlmsg.pos = (char*)(hdr + 1) + NLMSG_ALIGN(size);
}

static void netlink_attr(int typ, const void* data, int size)
{
  struct nlattr* attr = (struct nlattr*)nlmsg.pos;
  attr->nla_len = sizeof(*attr) + size;
  attr->nla_type = typ;
  memcpy(attr + 1, data, size);
  nlmsg.pos += NLMSG_ALIGN(attr->nla_len);
}

static void netlink_nest(int typ)
{
  struct nlattr* attr = (struct nlattr*)nlmsg.pos;
  attr->nla_type = typ;
  nlmsg.pos += sizeof(*attr);
  nlmsg.nested[nlmsg.nesting++] = attr;
}

static void netlink_done(void)
{
  struct nlattr* attr = nlmsg.nested[--nlmsg.nesting];
  attr->nla_len = nlmsg.pos - (char*)attr;
}

static int netlink_send(int sock)
{
  if (nlmsg.pos > nlmsg.buf + sizeof(nlmsg.buf) || nlmsg.nesting)
    exit(1);
  struct nlmsghdr* hdr = (struct nlmsghdr*)nlmsg.buf;
  hdr->nlmsg_len = nlmsg.pos - nlmsg.buf;
  struct sockaddr_nl addr;
  memset(&addr, 0, sizeof(addr));
  addr.nl_family = AF_NETLINK;
  unsigned n = sendto(sock, nlmsg.buf, hdr->nlmsg_len, 0,
                      (struct sockaddr*)&addr, sizeof(addr));
  if (n != hdr->nlmsg_len)
    exit(1);
  n = recv(sock, nlmsg.buf, sizeof(nlmsg.buf), 0);
  if (n < sizeof(struct nlmsghdr) + sizeof(struct nlmsgerr))
    exit(1);
  if (hdr->nlmsg_type != NLMSG_ERROR)
    exit(1);
  return -((struct nlmsgerr*)(hdr + 1))->error;
}

static void netlink_add_device_impl(const char* type, const char* name)
{
  struct ifinfomsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  netlink_init(RTM_NEWLINK, NLM_F_EXCL | NLM_F_CREATE, &hdr, sizeof(hdr));
  if (name)
    netlink_attr(IFLA_IFNAME, name, strlen(name));
  netlink_nest(IFLA_LINKINFO);
  netlink_attr(IFLA_INFO_KIND, type, strlen(type));
}

static void netlink_add_device(int sock, const char* type, const char* name)
{
  netlink_add_device_impl(type, name);
  netlink_done();
  int err = netlink_send(sock);
  (void)err;
}

static void netlink_add_veth(int sock, const char* name, const char* peer)
{
  netlink_add_device_impl("veth", name);
  netlink_nest(IFLA_INFO_DATA);
  netlink_nest(VETH_INFO_PEER);
  nlmsg.pos += sizeof(struct ifinfomsg);
  netlink_attr(IFLA_IFNAME, peer, strlen(peer));
  netlink_done();
  netlink_done();
  netlink_done();
  int err = netlink_send(sock);
  (void)err;
}

static void netlink_add_hsr(int sock, const char* name, const char* slave1,
                            const char* slave2)
{
  netlink_add_device_impl("hsr", name);
  netlink_nest(IFLA_INFO_DATA);
  int ifindex1 = if_nametoindex(slave1);
  netlink_attr(IFLA_HSR_SLAVE1, &ifindex1, sizeof(ifindex1));
  int ifindex2 = if_nametoindex(slave2);
  netlink_attr(IFLA_HSR_SLAVE2, &ifindex2, sizeof(ifindex2));
  netlink_done();
  netlink_done();
  int err = netlink_send(sock);
  (void)err;
}

static void netlink_device_change(int sock, const char* name, bool up,
                                  const char* master, const void* mac,
                                  int macsize)
{
  struct ifinfomsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  if (up)
    hdr.ifi_flags = hdr.ifi_change = IFF_UP;
  netlink_init(RTM_NEWLINK, 0, &hdr, sizeof(hdr));
  netlink_attr(IFLA_IFNAME, name, strlen(name));
  if (master) {
    int ifindex = if_nametoindex(master);
    netlink_attr(IFLA_MASTER, &ifindex, sizeof(ifindex));
  }
  if (macsize)
    netlink_attr(IFLA_ADDRESS, mac, macsize);
  int err = netlink_send(sock);
  (void)err;
}

static int netlink_add_addr(int sock, const char* dev, const void* addr,
                            int addrsize)
{
  struct ifaddrmsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  hdr.ifa_family = addrsize == 4 ? AF_INET : AF_INET6;
  hdr.ifa_prefixlen = addrsize == 4 ? 24 : 120;
  hdr.ifa_scope = RT_SCOPE_UNIVERSE;
  hdr.ifa_index = if_nametoindex(dev);
  netlink_init(RTM_NEWADDR, NLM_F_CREATE | NLM_F_REPLACE, &hdr, sizeof(hdr));
  netlink_attr(IFA_LOCAL, addr, addrsize);
  netlink_attr(IFA_ADDRESS, addr, addrsize);
  return netlink_send(sock);
}

static void netlink_add_addr4(int sock, const char* dev, const char* addr)
{
  struct in_addr in_addr;
  inet_pton(AF_INET, addr, &in_addr);
  int err = netlink_add_addr(sock, dev, &in_addr, sizeof(in_addr));
  (void)err;
}

static void netlink_add_addr6(int sock, const char* dev, const char* addr)
{
  struct in6_addr in6_addr;
  inet_pton(AF_INET6, addr, &in6_addr);
  int err = netlink_add_addr(sock, dev, &in6_addr, sizeof(in6_addr));
  (void)err;
}

static void netlink_add_neigh(int sock, const char* name, const void* addr,
                              int addrsize, const void* mac, int macsize)
{
  struct ndmsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  hdr.ndm_family = addrsize == 4 ? AF_INET : AF_INET6;
  hdr.ndm_ifindex = if_nametoindex(name);
  hdr.ndm_state = NUD_PERMANENT;
  netlink_init(RTM_NEWNEIGH, NLM_F_EXCL | NLM_F_CREATE, &hdr, sizeof(hdr));
  netlink_attr(NDA_DST, addr, addrsize);
  netlink_attr(NDA_LLADDR, mac, macsize);
  int err = netlink_send(sock);
  (void)err;
}

static int tunfd = -1;
static int tun_frags_enabled;
#define SYZ_TUN_MAX_PACKET_SIZE 1000

#define TUN_IFACE "syz_tun"

#define LOCAL_MAC 0xaaaaaaaaaaaa
#define REMOTE_MAC 0xaaaaaaaaaabb

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
  char sysctl[64];
  sprintf(sysctl, "/proc/sys/net/ipv6/conf/%s/accept_dad", TUN_IFACE);
  write_file(sysctl, "0");
  sprintf(sysctl, "/proc/sys/net/ipv6/conf/%s/router_solicitations", TUN_IFACE);
  write_file(sysctl, "0");
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sock == -1)
    exit(1);
  netlink_add_addr4(sock, TUN_IFACE, LOCAL_IPV4);
  netlink_add_addr6(sock, TUN_IFACE, LOCAL_IPV6);
  uint64_t macaddr = REMOTE_MAC;
  struct in_addr in_addr;
  inet_pton(AF_INET, REMOTE_IPV4, &in_addr);
  netlink_add_neigh(sock, TUN_IFACE, &in_addr, sizeof(in_addr), &macaddr,
                    ETH_ALEN);
  struct in6_addr in6_addr;
  inet_pton(AF_INET6, REMOTE_IPV6, &in6_addr);
  netlink_add_neigh(sock, TUN_IFACE, &in6_addr, sizeof(in6_addr), &macaddr,
                    ETH_ALEN);
  macaddr = LOCAL_MAC;
  netlink_device_change(sock, TUN_IFACE, true, 0, &macaddr, ETH_ALEN);
  close(sock);
}

#define DEV_IPV4 "172.20.20.%d"
#define DEV_IPV6 "fe80::%02x"
#define DEV_MAC 0x00aaaaaaaaaa
static void initialize_netdevices(void)
{
  char netdevsim[16];
  sprintf(netdevsim, "netdevsim%d", (int)procid);
  struct {
    const char* type;
    const char* dev;
  } devtypes[] = {
      {"ip6gretap", "ip6gretap0"}, {"bridge", "bridge0"},
      {"vcan", "vcan0"},           {"bond", "bond0"},
      {"team", "team0"},           {"dummy", "dummy0"},
      {"nlmon", "nlmon0"},         {"caif", "caif0"},
      {"batadv", "batadv0"},       {"vxcan", "vxcan1"},
      {"netdevsim", netdevsim},    {"veth", 0},
  };
  const char* devmasters[] = {"bridge", "bond", "team"};
  struct {
    const char* name;
    int macsize;
    bool noipv6;
  } devices[] = {
      {"lo", ETH_ALEN},
      {"sit0", 0},
      {"bridge0", ETH_ALEN},
      {"vcan0", 0, true},
      {"tunl0", 0},
      {"gre0", 0},
      {"gretap0", ETH_ALEN},
      {"ip_vti0", 0},
      {"ip6_vti0", 0},
      {"ip6tnl0", 0},
      {"ip6gre0", 0},
      {"ip6gretap0", ETH_ALEN},
      {"erspan0", ETH_ALEN},
      {"bond0", ETH_ALEN},
      {"veth0", ETH_ALEN},
      {"veth1", ETH_ALEN},
      {"team0", ETH_ALEN},
      {"veth0_to_bridge", ETH_ALEN},
      {"veth1_to_bridge", ETH_ALEN},
      {"veth0_to_bond", ETH_ALEN},
      {"veth1_to_bond", ETH_ALEN},
      {"veth0_to_team", ETH_ALEN},
      {"veth1_to_team", ETH_ALEN},
      {"veth0_to_hsr", ETH_ALEN},
      {"veth1_to_hsr", ETH_ALEN},
      {"hsr0", 0},
      {"dummy0", ETH_ALEN},
      {"nlmon0", 0},
      {"vxcan1", 0, true},
      {"caif0", ETH_ALEN},
      {"batadv0", ETH_ALEN},
      {netdevsim, ETH_ALEN},
  };
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sock == -1)
    exit(1);
  unsigned i;
  for (i = 0; i < sizeof(devtypes) / sizeof(devtypes[0]); i++)
    netlink_add_device(sock, devtypes[i].type, devtypes[i].dev);
  for (i = 0; i < sizeof(devmasters) / (sizeof(devmasters[0])); i++) {
    char master[32], slave0[32], veth0[32], slave1[32], veth1[32];
    sprintf(slave0, "%s_slave_0", devmasters[i]);
    sprintf(veth0, "veth0_to_%s", devmasters[i]);
    netlink_add_veth(sock, slave0, veth0);
    sprintf(slave1, "%s_slave_1", devmasters[i]);
    sprintf(veth1, "veth1_to_%s", devmasters[i]);
    netlink_add_veth(sock, slave1, veth1);
    sprintf(master, "%s0", devmasters[i]);
    netlink_device_change(sock, slave0, false, master, 0, 0);
    netlink_device_change(sock, slave1, false, master, 0, 0);
  }
  netlink_device_change(sock, "bridge_slave_0", true, 0, 0, 0);
  netlink_device_change(sock, "bridge_slave_1", true, 0, 0, 0);
  netlink_add_veth(sock, "hsr_slave_0", "veth0_to_hsr");
  netlink_add_veth(sock, "hsr_slave_1", "veth1_to_hsr");
  netlink_add_hsr(sock, "hsr0", "hsr_slave_0", "hsr_slave_1");
  netlink_device_change(sock, "hsr_slave_0", true, 0, 0, 0);
  netlink_device_change(sock, "hsr_slave_1", true, 0, 0, 0);
  for (i = 0; i < sizeof(devices) / (sizeof(devices[0])); i++) {
    char addr[32];
    sprintf(addr, DEV_IPV4, i + 10);
    netlink_add_addr4(sock, devices[i].name, addr);
    if (!devices[i].noipv6) {
      sprintf(addr, DEV_IPV6, i + 10);
      netlink_add_addr6(sock, devices[i].name, addr);
    }
    uint64_t macaddr = DEV_MAC + ((i + 10ull) << 40);
    netlink_device_change(sock, devices[i].name, true, 0, &macaddr,
                          devices[i].macsize);
  }
  close(sock);
}
static void initialize_netdevices_init(void)
{
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sock == -1)
    exit(1);
  struct {
    const char* type;
    int macsize;
    bool noipv6;
    bool noup;
  } devtypes[] = {
      {"nr", 7, true}, {"rose", 5, true, true},
  };
  unsigned i;
  for (i = 0; i < sizeof(devtypes) / sizeof(devtypes[0]); i++) {
    char dev[32], addr[32];
    sprintf(dev, "%s%d", devtypes[i].type, (int)procid);
    sprintf(addr, "172.30.%d.%d", i, (int)procid + 1);
    netlink_add_addr4(sock, dev, addr);
    if (!devtypes[i].noipv6) {
      sprintf(addr, "fe88::%02x:%02x", i, (int)procid + 1);
      netlink_add_addr6(sock, dev, addr);
    }
    int macsize = devtypes[i].macsize;
    uint64_t macaddr = 0xbbbbbb +
                       ((unsigned long long)i << (8 * (macsize - 2))) +
                       (procid << (8 * (macsize - 1)));
    netlink_device_change(sock, dev, !devtypes[i].noup, 0, &macaddr, macsize);
  }
  close(sock);
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

#define MAX_FRAGS 4
struct vnet_fragmentation {
  uint32_t full;
  uint32_t count;
  uint32_t frags[MAX_FRAGS];
};

static long syz_emit_ethernet(volatile long a0, volatile long a1,
                              volatile long a2)
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
  while (read_tun(&data[0], sizeof(data)) != -1) {
  }
}

#define MAX_FDS 30

static void setup_common()
{
  if (mount(0, "/sys/fs/fuse/connections", "fusectl", 0, 0)) {
  }
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();
  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = (200 << 20);
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 32 << 20;
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
  typedef struct {
    const char* name;
    const char* value;
  } sysctl_t;
  static const sysctl_t sysctls[] = {
      {"/proc/sys/kernel/shmmax", "16777216"},
      {"/proc/sys/kernel/shmall", "536870912"},
      {"/proc/sys/kernel/shmmni", "1024"},
      {"/proc/sys/kernel/msgmax", "8192"},
      {"/proc/sys/kernel/msgmni", "1024"},
      {"/proc/sys/kernel/msgmnb", "1024"},
      {"/proc/sys/kernel/sem", "1024 1048576 500 1024"},
  };
  unsigned i;
  for (i = 0; i < sizeof(sysctls) / sizeof(sysctls[0]); i++)
    write_file(sysctls[i].name, sysctls[i].value);
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

static void drop_caps(void)
{
  struct __user_cap_header_struct cap_hdr = {};
  struct __user_cap_data_struct cap_data[2] = {};
  cap_hdr.version = _LINUX_CAPABILITY_VERSION_3;
  cap_hdr.pid = getpid();
  if (syscall(SYS_capget, &cap_hdr, &cap_data))
    exit(1);
  const int drop = (1 << CAP_SYS_PTRACE) | (1 << CAP_SYS_NICE);
  cap_data[0].effective &= ~drop;
  cap_data[0].permitted &= ~drop;
  cap_data[0].inheritable &= ~drop;
  if (syscall(SYS_capset, &cap_hdr, &cap_data))
    exit(1);
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
  drop_caps();
  initialize_netdevices_init();
  if (unshare(CLONE_NEWNET)) {
  }
  initialize_tun();
  initialize_netdevices();
  loop();
  exit(1);
}

static void kill_and_wait(int pid, int* status)
{
  kill(-pid, SIGKILL);
  kill(pid, SIGKILL);
  int i;
  for (i = 0; i < 100; i++) {
    if (waitpid(-1, status, WNOHANG | __WALL) == pid)
      return;
    usleep(1000);
  }
  DIR* dir = opendir("/sys/fs/fuse/connections");
  if (dir) {
    for (;;) {
      struct dirent* ent = readdir(dir);
      if (!ent)
        break;
      if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        continue;
      char abort[300];
      snprintf(abort, sizeof(abort), "/sys/fs/fuse/connections/%s/abort",
               ent->d_name);
      int fd = open(abort, O_WRONLY);
      if (fd == -1) {
        continue;
      }
      if (write(fd, abort, 1) < 0) {
      }
      close(fd);
    }
    closedir(dir);
  } else {
  }
  while (waitpid(-1, status, __WALL) != pid) {
  }
}

static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  write_file("/proc/self/oom_score_adj", "1000");
  flush_tun();
}

static void close_fds()
{
  int fd;
  for (fd = 3; fd < MAX_FDS; fd++)
    close(fd);
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter;
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      setup_test();
      execute_one();
      close_fds();
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
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  memcpy((void*)0x20000b00, "/dev/null\000", 10);
  syscall(__NR_openat, 0xffffffffffffff9c, 0x20000b00, 0x40000, 0);
  syscall(__NR_mkdir, 0, 0);
  *(uint32_t*)0x2001d000 = 1;
  *(uint32_t*)0x2001d004 = 0x70;
  *(uint8_t*)0x2001d008 = 0;
  *(uint8_t*)0x2001d009 = 0;
  *(uint8_t*)0x2001d00a = 0;
  *(uint8_t*)0x2001d00b = 0;
  *(uint32_t*)0x2001d00c = 0;
  *(uint64_t*)0x2001d010 = 0x41c1;
  *(uint64_t*)0x2001d018 = 0;
  *(uint64_t*)0x2001d020 = 0;
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 0, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 1, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 2, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 3, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 4, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 5, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 6, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 7, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 8, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 9, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 10, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 11, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 12, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 13, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 14, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 15, 2);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 17, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 18, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 19, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 20, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 21, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 22, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 23, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 24, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 25, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 26, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 27, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 28, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 29, 35);
  *(uint32_t*)0x2001d030 = 0;
  *(uint32_t*)0x2001d034 = 0;
  *(uint64_t*)0x2001d038 = 0;
  *(uint64_t*)0x2001d040 = 0;
  *(uint64_t*)0x2001d048 = 0;
  *(uint64_t*)0x2001d050 = 0;
  *(uint32_t*)0x2001d058 = 0;
  *(uint32_t*)0x2001d05c = 0;
  *(uint64_t*)0x2001d060 = 0;
  *(uint32_t*)0x2001d068 = 0;
  *(uint16_t*)0x2001d06c = 0;
  *(uint16_t*)0x2001d06e = 0;
  syscall(__NR_perf_event_open, 0x2001d000, 0, -1, -1, 0);
  syscall(__NR_perf_event_open, 0, 0, -1, -1, 0);
  memcpy((void*)0x20000100, "\x8d\x40\x42\x0d\x29\x7d", 6);
  *(uint8_t*)0x20000106 = -1;
  *(uint8_t*)0x20000107 = -1;
  *(uint8_t*)0x20000108 = -1;
  *(uint8_t*)0x20000109 = -1;
  *(uint8_t*)0x2000010a = -1;
  *(uint8_t*)0x2000010b = -1;
  *(uint16_t*)0x2000010c = htobe16(0x86dd);
  STORE_BY_BITMASK(uint8_t, , 0x2000010e, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x2000010e, 6, 4, 4);
  memcpy((void*)0x2000010f, "5&h", 3);
  *(uint16_t*)0x20000112 = htobe16(0x44);
  *(uint8_t*)0x20000114 = 0x2f;
  *(uint8_t*)0x20000115 = 0;
  *(uint8_t*)0x20000116 = 0xfe;
  *(uint8_t*)0x20000117 = 0x80;
  *(uint8_t*)0x20000118 = 0;
  *(uint8_t*)0x20000119 = 0;
  *(uint8_t*)0x2000011a = 0;
  *(uint8_t*)0x2000011b = 0;
  *(uint8_t*)0x2000011c = 0;
  *(uint8_t*)0x2000011d = 0;
  *(uint8_t*)0x2000011e = 0;
  *(uint8_t*)0x2000011f = 0;
  *(uint8_t*)0x20000120 = 0;
  *(uint8_t*)0x20000121 = 0;
  *(uint8_t*)0x20000122 = 0;
  *(uint8_t*)0x20000123 = 0;
  *(uint8_t*)0x20000124 = 0;
  *(uint8_t*)0x20000125 = 0xaa;
  *(uint8_t*)0x20000126 = 0xfe;
  *(uint8_t*)0x20000127 = 0x80;
  *(uint8_t*)0x20000128 = 0;
  *(uint8_t*)0x20000129 = 0;
  *(uint8_t*)0x2000012a = 0;
  *(uint8_t*)0x2000012b = 0;
  *(uint8_t*)0x2000012c = 0;
  *(uint8_t*)0x2000012d = 0;
  *(uint8_t*)0x2000012e = 0;
  *(uint8_t*)0x2000012f = 0;
  *(uint8_t*)0x20000130 = 0;
  *(uint8_t*)0x20000131 = 0;
  *(uint8_t*)0x20000132 = 0;
  *(uint8_t*)0x20000133 = 0;
  *(uint8_t*)0x20000134 = 0;
  *(uint8_t*)0x20000135 = 0xaa;
  STORE_BY_BITMASK(uint16_t, , 0x20000136, 0, 0, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000136, 0, 1, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000136, 1, 2, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000136, 0, 3, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000136, 0, 4, 4);
  STORE_BY_BITMASK(uint16_t, , 0x20000136, 0, 8, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000136, 0, 9, 4);
  STORE_BY_BITMASK(uint16_t, , 0x20000136, 1, 13, 3);
  *(uint16_t*)0x20000138 = htobe16(0x6558);
  *(uint16_t*)0x2000013a = htobe16(0);
  *(uint16_t*)0x2000013c = htobe16(0);
  STORE_BY_BITMASK(uint16_t, , 0x2000013e, 0, 0, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000013e, 0, 1, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000013e, 0, 2, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000013e, 0, 3, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000013e, 0, 4, 9);
  STORE_BY_BITMASK(uint16_t, , 0x2000013e, 0, 13, 3);
  *(uint16_t*)0x20000140 = htobe16(0x800);
  STORE_BY_BITMASK(uint16_t, , 0x20000142, 0, 0, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000142, 0, 1, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000142, 0, 2, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000142, 0, 3, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000142, 0, 4, 9);
  STORE_BY_BITMASK(uint16_t, , 0x20000142, 0, 13, 3);
  *(uint16_t*)0x20000144 = htobe16(0x86dd);
  *(uint16_t*)0x20000146 = 8;
  *(uint16_t*)0x20000148 = htobe16(0x88be);
  *(uint32_t*)0x2000014a = htobe32(0);
  STORE_BY_BITMASK(uint8_t, , 0x2000014e, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x2000014e, 1, 4, 4);
  *(uint8_t*)0x2000014f = 0;
  STORE_BY_BITMASK(uint8_t, , 0x20000150, 0, 0, 2);
  STORE_BY_BITMASK(uint8_t, , 0x20000150, 0, 2, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000150, 0, 3, 2);
  STORE_BY_BITMASK(uint8_t, , 0x20000150, 0, 5, 3);
  *(uint8_t*)0x20000151 = 0;
  *(uint32_t*)0x20000152 = 1;
  *(uint32_t*)0x20000156 = htobe32(0);
  *(uint16_t*)0x2000015a = 8;
  *(uint16_t*)0x2000015c = htobe16(0x22eb);
  *(uint32_t*)0x2000015e = htobe32(0);
  STORE_BY_BITMASK(uint8_t, , 0x20000162, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000162, 2, 4, 4);
  *(uint8_t*)0x20000163 = 0;
  STORE_BY_BITMASK(uint8_t, , 0x20000164, 0, 0, 2);
  STORE_BY_BITMASK(uint8_t, , 0x20000164, 0, 2, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000164, 0, 3, 2);
  STORE_BY_BITMASK(uint8_t, , 0x20000164, 0, 5, 3);
  *(uint8_t*)0x20000165 = 0;
  *(uint32_t*)0x20000166 = 2;
  *(uint32_t*)0x2000016a = htobe32(0);
  *(uint16_t*)0x2000016e = htobe16(1);
  STORE_BY_BITMASK(uint8_t, , 0x20000170, 0, 0, 2);
  STORE_BY_BITMASK(uint8_t, , 0x20000170, 0, 2, 5);
  STORE_BY_BITMASK(uint8_t, , 0x20000170, 0, 7, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000171, 0, 0, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000171, 0, 1, 2);
  STORE_BY_BITMASK(uint8_t, , 0x20000171, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000171, 0, 4, 1);
  *(uint16_t*)0x20000172 = 8;
  *(uint16_t*)0x20000174 = htobe16(0x6558);
  *(uint32_t*)0x20000176 = htobe32(0);
  syz_emit_ethernet(0x7a, 0x20000100, 0);
  syscall(__NR_keyctl, 5, 0xfffffffffffffffe, 0, 0, 0);
  syscall(__NR_add_key, 0, 0, 0, 0, 0xfffffffd);
  syscall(__NR_write, -1, 0, 0);
  *(uint32_t*)0x20000080 = 0x18;
  *(uint32_t*)0x20000084 = 0;
  *(uint64_t*)0x20000088 = 6;
  *(uint64_t*)0x20000090 = 0;
  syscall(__NR_write, -1, 0x20000080, 0x18);
  memcpy((void*)0x20000000, "./file0\000", 8);
  memcpy((void*)0x20000140, "9p\000", 3);
  syscall(__NR_mount, 0, 0x20000000, 0x20000140, 0, 0);
  syscall(__NR_ioctl, -1, 0xc028660f, 0);
  *(uint64_t*)0x20000080 = 0;
  *(uint64_t*)0x20000088 = 2;
  *(uint64_t*)0x20000090 = 0;
  *(uint64_t*)0x20000098 = 0;
  *(uint64_t*)0x200000a0 = 0;
  *(uint64_t*)0x200000a8 = 0;
  *(uint64_t*)0x200000b0 = 0;
  *(uint64_t*)0x200000b8 = 0;
  res = syscall(__NR_mq_open, 0, 0, 0, 0x20000080);
  if (res != -1)
    r[0] = res;
  syscall(__NR_mq_timedsend, r[0], 0, 0, 0, 0);
  syscall(__NR_mq_timedsend, -1, 0, 0, 0, 0);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0, 0, 0);
  if (res != -1)
    r[1] = res;
  syscall(__NR_lseek, r[1], 0, 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  do_sandbox_none();
  return 0;
}
