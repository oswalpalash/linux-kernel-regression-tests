// https://syzkaller.appspot.com/bug?id=bbfa653205516be2a33b51c381ef827c534ba596
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
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

static void install_segv_handler(void)
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

static uint64_t current_time_ms(void)
{
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    exit(1);
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void use_temporary_dir(void)
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
  initialize_netdevices();
  loop();
  exit(1);
}

static void close_fds()
{
  int fd;
  for (fd = 3; fd < 30; fd++)
    close(fd);
}

#define KMEMLEAK_FILE "/sys/kernel/debug/kmemleak"

static void setup_leak()
{
  if (!write_file(KMEMLEAK_FILE, "scan"))
    exit(1);
  sleep(5);
  if (!write_file(KMEMLEAK_FILE, "scan"))
    exit(1);
  if (!write_file(KMEMLEAK_FILE, "clear"))
    exit(1);
}

static void check_leaks(void)
{
  int fd = open(KMEMLEAK_FILE, O_RDWR);
  if (fd == -1)
    exit(1);
  uint64_t start = current_time_ms();
  if (write(fd, "scan", 4) != 4)
    exit(1);
  sleep(1);
  while (current_time_ms() - start < 4 * 1000)
    sleep(1);
  if (write(fd, "scan", 4) != 4)
    exit(1);
  static char buf[128 << 10];
  ssize_t n = read(fd, buf, sizeof(buf) - 1);
  if (n < 0)
    exit(1);
  int nleaks = 0;
  if (n != 0) {
    sleep(1);
    if (write(fd, "scan", 4) != 4)
      exit(1);
    if (lseek(fd, 0, SEEK_SET) < 0)
      exit(1);
    n = read(fd, buf, sizeof(buf) - 1);
    if (n < 0)
      exit(1);
    buf[n] = 0;
    char* pos = buf;
    char* end = buf + n;
    while (pos < end) {
      char* next = strstr(pos + 1, "unreferenced object");
      if (!next)
        next = end;
      char prev = *next;
      *next = 0;
      fprintf(stderr, "BUG: memory leak\n%s\n", pos);
      *next = prev;
      pos = next;
      nleaks++;
    }
  }
  if (write(fd, "clear", 5) != 5)
    exit(1);
  close(fd);
  if (nleaks)
    exit(1);
}

static void setup_binfmt_misc()
{
  if (mount(0, "/proc/sys/fs/binfmt_misc", "binfmt_misc", 0, 0)) {
  }
  write_file("/proc/sys/fs/binfmt_misc/register", ":syz0:M:0:\x01::./file0:");
  write_file("/proc/sys/fs/binfmt_misc/register",
             ":syz1:M:1:\x02::./file0:POC");
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void loop(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 2, 2, 0);
  if (res != -1)
    r[0] = res;
  syscall(__NR_close, r[0]);
  res = syscall(__NR_socket, 0xa, 1, 0x84);
  if (res != -1)
    r[1] = res;
  NONFAILING(*(uint16_t*)0x20000100 = 0xa);
  NONFAILING(*(uint16_t*)0x20000102 = htobe16(0x4e21));
  NONFAILING(*(uint32_t*)0x20000104 = htobe32(0));
  NONFAILING(*(uint8_t*)0x20000108 = 0);
  NONFAILING(*(uint8_t*)0x20000109 = 0);
  NONFAILING(*(uint8_t*)0x2000010a = 0);
  NONFAILING(*(uint8_t*)0x2000010b = 0);
  NONFAILING(*(uint8_t*)0x2000010c = 0);
  NONFAILING(*(uint8_t*)0x2000010d = 0);
  NONFAILING(*(uint8_t*)0x2000010e = 0);
  NONFAILING(*(uint8_t*)0x2000010f = 0);
  NONFAILING(*(uint8_t*)0x20000110 = 0);
  NONFAILING(*(uint8_t*)0x20000111 = 0);
  NONFAILING(*(uint8_t*)0x20000112 = 0);
  NONFAILING(*(uint8_t*)0x20000113 = 0);
  NONFAILING(*(uint8_t*)0x20000114 = 0);
  NONFAILING(*(uint8_t*)0x20000115 = 0);
  NONFAILING(*(uint8_t*)0x20000116 = 0);
  NONFAILING(*(uint8_t*)0x20000117 = 0);
  NONFAILING(*(uint32_t*)0x20000118 = 0);
  syscall(__NR_bind, r[1], 0x20000100, 0x1c);
  NONFAILING(*(uint16_t*)0x20000240 = 0xa);
  NONFAILING(*(uint16_t*)0x20000242 = htobe16(0x4e21));
  NONFAILING(*(uint32_t*)0x20000244 = htobe32(0));
  NONFAILING(*(uint64_t*)0x20000248 = htobe64(0));
  NONFAILING(*(uint64_t*)0x20000250 = htobe64(1));
  NONFAILING(*(uint32_t*)0x20000258 = 0);
  syscall(__NR_connect, r[1], 0x20000240, 0x1c);
  NONFAILING(*(uint16_t*)0x200000c0 = 1);
  NONFAILING(*(uint64_t*)0x200000c8 = 0x20000000);
  NONFAILING(*(uint16_t*)0x20000000 = 6);
  NONFAILING(*(uint8_t*)0x20000002 = 0);
  NONFAILING(*(uint8_t*)0x20000003 = 0);
  NONFAILING(*(uint32_t*)0x20000004 = 0);
  syscall(__NR_setsockopt, r[0], 1, 0x1a, 0x200000c0, 0x10);
  NONFAILING(*(uint8_t*)0x200009c0 = 0x7f);
  NONFAILING(*(uint8_t*)0x200009c1 = 0x45);
  NONFAILING(*(uint8_t*)0x200009c2 = 0x4c);
  NONFAILING(*(uint8_t*)0x200009c3 = 0x46);
  NONFAILING(*(uint8_t*)0x200009c4 = 8);
  NONFAILING(*(uint8_t*)0x200009c5 = 9);
  NONFAILING(*(uint8_t*)0x200009c6 = 5);
  NONFAILING(*(uint8_t*)0x200009c7 = 0x1c);
  NONFAILING(*(uint64_t*)0x200009c8 = 0xfffffffffffffffe);
  NONFAILING(*(uint16_t*)0x200009d0 = 2);
  NONFAILING(*(uint16_t*)0x200009d2 = 0);
  NONFAILING(*(uint32_t*)0x200009d4 = 1);
  NONFAILING(*(uint64_t*)0x200009d8 = 0x121);
  NONFAILING(*(uint64_t*)0x200009e0 = 0x40);
  NONFAILING(*(uint64_t*)0x200009e8 = 0);
  NONFAILING(*(uint32_t*)0x200009f0 = 0);
  NONFAILING(*(uint16_t*)0x200009f4 = 0);
  NONFAILING(*(uint16_t*)0x200009f6 = 0x38);
  NONFAILING(*(uint16_t*)0x200009f8 = 2);
  NONFAILING(*(uint16_t*)0x200009fa = 2);
  NONFAILING(*(uint16_t*)0x200009fc = 0x100);
  NONFAILING(*(uint16_t*)0x200009fe = 0x8000);
  NONFAILING(*(uint32_t*)0x20000a00 = 3);
  NONFAILING(*(uint32_t*)0x20000a04 = 2);
  NONFAILING(*(uint64_t*)0x20000a08 = 1);
  NONFAILING(*(uint64_t*)0x20000a10 = 3);
  NONFAILING(*(uint64_t*)0x20000a18 = 9);
  NONFAILING(*(uint64_t*)0x20000a20 = 0x3ff);
  NONFAILING(*(uint64_t*)0x20000a28 = 0xfcc);
  NONFAILING(*(uint64_t*)0x20000a30 = 0xfffffffffffffffd);
  NONFAILING(*(uint64_t*)0x20000a38 = 0);
  NONFAILING(*(uint64_t*)0x20000a40 = 0);
  NONFAILING(*(uint64_t*)0x20000a48 = 0);
  NONFAILING(*(uint64_t*)0x20000a50 = 0);
  NONFAILING(*(uint64_t*)0x20000a58 = 0);
  NONFAILING(*(uint64_t*)0x20000a60 = 0);
  NONFAILING(*(uint64_t*)0x20000a68 = 0);
  NONFAILING(*(uint64_t*)0x20000a70 = 0);
  NONFAILING(*(uint64_t*)0x20000a78 = 0);
  NONFAILING(*(uint64_t*)0x20000a80 = 0);
  NONFAILING(*(uint64_t*)0x20000a88 = 0);
  NONFAILING(*(uint64_t*)0x20000a90 = 0);
  NONFAILING(*(uint64_t*)0x20000a98 = 0);
  NONFAILING(*(uint64_t*)0x20000aa0 = 0);
  NONFAILING(*(uint64_t*)0x20000aa8 = 0);
  NONFAILING(*(uint64_t*)0x20000ab0 = 0);
  NONFAILING(*(uint64_t*)0x20000ab8 = 0);
  NONFAILING(*(uint64_t*)0x20000ac0 = 0);
  NONFAILING(*(uint64_t*)0x20000ac8 = 0);
  NONFAILING(*(uint64_t*)0x20000ad0 = 0);
  NONFAILING(*(uint64_t*)0x20000ad8 = 0);
  NONFAILING(*(uint64_t*)0x20000ae0 = 0);
  NONFAILING(*(uint64_t*)0x20000ae8 = 0);
  NONFAILING(*(uint64_t*)0x20000af0 = 0);
  NONFAILING(*(uint64_t*)0x20000af8 = 0);
  NONFAILING(*(uint64_t*)0x20000b00 = 0);
  NONFAILING(*(uint64_t*)0x20000b08 = 0);
  NONFAILING(*(uint64_t*)0x20000b10 = 0);
  NONFAILING(*(uint64_t*)0x20000b18 = 0);
  NONFAILING(*(uint64_t*)0x20000b20 = 0);
  NONFAILING(*(uint64_t*)0x20000b28 = 0);
  NONFAILING(*(uint64_t*)0x20000b30 = 0);
  NONFAILING(*(uint64_t*)0x20000b38 = 0);
  NONFAILING(*(uint64_t*)0x20000b40 = 0);
  NONFAILING(*(uint64_t*)0x20000b48 = 0);
  NONFAILING(*(uint64_t*)0x20000b50 = 0);
  NONFAILING(*(uint64_t*)0x20000b58 = 0);
  NONFAILING(*(uint64_t*)0x20000b60 = 0);
  NONFAILING(*(uint64_t*)0x20000b68 = 0);
  NONFAILING(*(uint64_t*)0x20000b70 = 0);
  NONFAILING(*(uint64_t*)0x20000b78 = 0);
  NONFAILING(*(uint64_t*)0x20000b80 = 0);
  NONFAILING(*(uint64_t*)0x20000b88 = 0);
  NONFAILING(*(uint64_t*)0x20000b90 = 0);
  NONFAILING(*(uint64_t*)0x20000b98 = 0);
  NONFAILING(*(uint64_t*)0x20000ba0 = 0);
  NONFAILING(*(uint64_t*)0x20000ba8 = 0);
  NONFAILING(*(uint64_t*)0x20000bb0 = 0);
  NONFAILING(*(uint64_t*)0x20000bb8 = 0);
  NONFAILING(*(uint64_t*)0x20000bc0 = 0);
  NONFAILING(*(uint64_t*)0x20000bc8 = 0);
  NONFAILING(*(uint64_t*)0x20000bd0 = 0);
  NONFAILING(*(uint64_t*)0x20000bd8 = 0);
  NONFAILING(*(uint64_t*)0x20000be0 = 0);
  NONFAILING(*(uint64_t*)0x20000be8 = 0);
  NONFAILING(*(uint64_t*)0x20000bf0 = 0);
  NONFAILING(*(uint64_t*)0x20000bf8 = 0);
  NONFAILING(*(uint64_t*)0x20000c00 = 0);
  NONFAILING(*(uint64_t*)0x20000c08 = 0);
  NONFAILING(*(uint64_t*)0x20000c10 = 0);
  NONFAILING(*(uint64_t*)0x20000c18 = 0);
  NONFAILING(*(uint64_t*)0x20000c20 = 0);
  NONFAILING(*(uint64_t*)0x20000c28 = 0);
  NONFAILING(*(uint64_t*)0x20000c30 = 0);
  NONFAILING(*(uint64_t*)0x20000c38 = 0);
  NONFAILING(*(uint64_t*)0x20000c40 = 0);
  NONFAILING(*(uint64_t*)0x20000c48 = 0);
  NONFAILING(*(uint64_t*)0x20000c50 = 0);
  NONFAILING(*(uint64_t*)0x20000c58 = 0);
  NONFAILING(*(uint64_t*)0x20000c60 = 0);
  NONFAILING(*(uint64_t*)0x20000c68 = 0);
  NONFAILING(*(uint64_t*)0x20000c70 = 0);
  NONFAILING(*(uint64_t*)0x20000c78 = 0);
  NONFAILING(*(uint64_t*)0x20000c80 = 0);
  NONFAILING(*(uint64_t*)0x20000c88 = 0);
  NONFAILING(*(uint64_t*)0x20000c90 = 0);
  NONFAILING(*(uint64_t*)0x20000c98 = 0);
  NONFAILING(*(uint64_t*)0x20000ca0 = 0);
  NONFAILING(*(uint64_t*)0x20000ca8 = 0);
  NONFAILING(*(uint64_t*)0x20000cb0 = 0);
  NONFAILING(*(uint64_t*)0x20000cb8 = 0);
  NONFAILING(*(uint64_t*)0x20000cc0 = 0);
  NONFAILING(*(uint64_t*)0x20000cc8 = 0);
  NONFAILING(*(uint64_t*)0x20000cd0 = 0);
  NONFAILING(*(uint64_t*)0x20000cd8 = 0);
  NONFAILING(*(uint64_t*)0x20000ce0 = 0);
  NONFAILING(*(uint64_t*)0x20000ce8 = 0);
  NONFAILING(*(uint64_t*)0x20000cf0 = 0);
  NONFAILING(*(uint64_t*)0x20000cf8 = 0);
  NONFAILING(*(uint64_t*)0x20000d00 = 0);
  NONFAILING(*(uint64_t*)0x20000d08 = 0);
  NONFAILING(*(uint64_t*)0x20000d10 = 0);
  NONFAILING(*(uint64_t*)0x20000d18 = 0);
  NONFAILING(*(uint64_t*)0x20000d20 = 0);
  NONFAILING(*(uint64_t*)0x20000d28 = 0);
  NONFAILING(*(uint64_t*)0x20000d30 = 0);
  NONFAILING(*(uint64_t*)0x20000d38 = 0);
  NONFAILING(*(uint64_t*)0x20000d40 = 0);
  NONFAILING(*(uint64_t*)0x20000d48 = 0);
  NONFAILING(*(uint64_t*)0x20000d50 = 0);
  NONFAILING(*(uint64_t*)0x20000d58 = 0);
  NONFAILING(*(uint64_t*)0x20000d60 = 0);
  NONFAILING(*(uint64_t*)0x20000d68 = 0);
  NONFAILING(*(uint64_t*)0x20000d70 = 0);
  NONFAILING(*(uint64_t*)0x20000d78 = 0);
  NONFAILING(*(uint64_t*)0x20000d80 = 0);
  NONFAILING(*(uint64_t*)0x20000d88 = 0);
  NONFAILING(*(uint64_t*)0x20000d90 = 0);
  NONFAILING(*(uint64_t*)0x20000d98 = 0);
  NONFAILING(*(uint64_t*)0x20000da0 = 0);
  NONFAILING(*(uint64_t*)0x20000da8 = 0);
  NONFAILING(*(uint64_t*)0x20000db0 = 0);
  NONFAILING(*(uint64_t*)0x20000db8 = 0);
  NONFAILING(*(uint64_t*)0x20000dc0 = 0);
  NONFAILING(*(uint64_t*)0x20000dc8 = 0);
  NONFAILING(*(uint64_t*)0x20000dd0 = 0);
  NONFAILING(*(uint64_t*)0x20000dd8 = 0);
  NONFAILING(*(uint64_t*)0x20000de0 = 0);
  NONFAILING(*(uint64_t*)0x20000de8 = 0);
  NONFAILING(*(uint64_t*)0x20000df0 = 0);
  NONFAILING(*(uint64_t*)0x20000df8 = 0);
  NONFAILING(*(uint64_t*)0x20000e00 = 0);
  NONFAILING(*(uint64_t*)0x20000e08 = 0);
  NONFAILING(*(uint64_t*)0x20000e10 = 0);
  NONFAILING(*(uint64_t*)0x20000e18 = 0);
  NONFAILING(*(uint64_t*)0x20000e20 = 0);
  NONFAILING(*(uint64_t*)0x20000e28 = 0);
  NONFAILING(*(uint64_t*)0x20000e30 = 0);
  NONFAILING(*(uint64_t*)0x20000e38 = 0);
  NONFAILING(*(uint64_t*)0x20000e40 = 0);
  NONFAILING(*(uint64_t*)0x20000e48 = 0);
  NONFAILING(*(uint64_t*)0x20000e50 = 0);
  NONFAILING(*(uint64_t*)0x20000e58 = 0);
  NONFAILING(*(uint64_t*)0x20000e60 = 0);
  NONFAILING(*(uint64_t*)0x20000e68 = 0);
  NONFAILING(*(uint64_t*)0x20000e70 = 0);
  NONFAILING(*(uint64_t*)0x20000e78 = 0);
  NONFAILING(*(uint64_t*)0x20000e80 = 0);
  NONFAILING(*(uint64_t*)0x20000e88 = 0);
  NONFAILING(*(uint64_t*)0x20000e90 = 0);
  NONFAILING(*(uint64_t*)0x20000e98 = 0);
  NONFAILING(*(uint64_t*)0x20000ea0 = 0);
  NONFAILING(*(uint64_t*)0x20000ea8 = 0);
  NONFAILING(*(uint64_t*)0x20000eb0 = 0);
  NONFAILING(*(uint64_t*)0x20000eb8 = 0);
  NONFAILING(*(uint64_t*)0x20000ec0 = 0);
  NONFAILING(*(uint64_t*)0x20000ec8 = 0);
  NONFAILING(*(uint64_t*)0x20000ed0 = 0);
  NONFAILING(*(uint64_t*)0x20000ed8 = 0);
  NONFAILING(*(uint64_t*)0x20000ee0 = 0);
  NONFAILING(*(uint64_t*)0x20000ee8 = 0);
  NONFAILING(*(uint64_t*)0x20000ef0 = 0);
  NONFAILING(*(uint64_t*)0x20000ef8 = 0);
  NONFAILING(*(uint64_t*)0x20000f00 = 0);
  NONFAILING(*(uint64_t*)0x20000f08 = 0);
  NONFAILING(*(uint64_t*)0x20000f10 = 0);
  NONFAILING(*(uint64_t*)0x20000f18 = 0);
  NONFAILING(*(uint64_t*)0x20000f20 = 0);
  NONFAILING(*(uint64_t*)0x20000f28 = 0);
  NONFAILING(*(uint64_t*)0x20000f30 = 0);
  NONFAILING(*(uint64_t*)0x20000f38 = 0);
  NONFAILING(*(uint64_t*)0x20000f40 = 0);
  NONFAILING(*(uint64_t*)0x20000f48 = 0);
  NONFAILING(*(uint64_t*)0x20000f50 = 0);
  NONFAILING(*(uint64_t*)0x20000f58 = 0);
  NONFAILING(*(uint64_t*)0x20000f60 = 0);
  NONFAILING(*(uint64_t*)0x20000f68 = 0);
  NONFAILING(*(uint64_t*)0x20000f70 = 0);
  NONFAILING(*(uint64_t*)0x20000f78 = 0);
  NONFAILING(*(uint64_t*)0x20000f80 = 0);
  NONFAILING(*(uint64_t*)0x20000f88 = 0);
  NONFAILING(*(uint64_t*)0x20000f90 = 0);
  NONFAILING(*(uint64_t*)0x20000f98 = 0);
  NONFAILING(*(uint64_t*)0x20000fa0 = 0);
  NONFAILING(*(uint64_t*)0x20000fa8 = 0);
  NONFAILING(*(uint64_t*)0x20000fb0 = 0);
  NONFAILING(*(uint64_t*)0x20000fb8 = 0);
  NONFAILING(*(uint64_t*)0x20000fc0 = 0);
  NONFAILING(*(uint64_t*)0x20000fc8 = 0);
  NONFAILING(*(uint64_t*)0x20000fd0 = 0);
  NONFAILING(*(uint64_t*)0x20000fd8 = 0);
  NONFAILING(*(uint64_t*)0x20000fe0 = 0);
  NONFAILING(*(uint64_t*)0x20000fe8 = 0);
  NONFAILING(*(uint64_t*)0x20000ff0 = 0);
  NONFAILING(*(uint64_t*)0x20000ff8 = 0);
  NONFAILING(*(uint64_t*)0x20001000 = 0);
  NONFAILING(*(uint64_t*)0x20001008 = 0);
  NONFAILING(*(uint64_t*)0x20001010 = 0);
  NONFAILING(*(uint64_t*)0x20001018 = 0);
  NONFAILING(*(uint64_t*)0x20001020 = 0);
  NONFAILING(*(uint64_t*)0x20001028 = 0);
  NONFAILING(*(uint64_t*)0x20001030 = 0);
  NONFAILING(*(uint64_t*)0x20001038 = 0);
  NONFAILING(*(uint64_t*)0x20001040 = 0);
  NONFAILING(*(uint64_t*)0x20001048 = 0);
  NONFAILING(*(uint64_t*)0x20001050 = 0);
  NONFAILING(*(uint64_t*)0x20001058 = 0);
  NONFAILING(*(uint64_t*)0x20001060 = 0);
  NONFAILING(*(uint64_t*)0x20001068 = 0);
  NONFAILING(*(uint64_t*)0x20001070 = 0);
  NONFAILING(*(uint64_t*)0x20001078 = 0);
  NONFAILING(*(uint64_t*)0x20001080 = 0);
  NONFAILING(*(uint64_t*)0x20001088 = 0);
  NONFAILING(*(uint64_t*)0x20001090 = 0);
  NONFAILING(*(uint64_t*)0x20001098 = 0);
  NONFAILING(*(uint64_t*)0x200010a0 = 0);
  NONFAILING(*(uint64_t*)0x200010a8 = 0);
  NONFAILING(*(uint64_t*)0x200010b0 = 0);
  NONFAILING(*(uint64_t*)0x200010b8 = 0);
  NONFAILING(*(uint64_t*)0x200010c0 = 0);
  NONFAILING(*(uint64_t*)0x200010c8 = 0);
  NONFAILING(*(uint64_t*)0x200010d0 = 0);
  NONFAILING(*(uint64_t*)0x200010d8 = 0);
  NONFAILING(*(uint64_t*)0x200010e0 = 0);
  NONFAILING(*(uint64_t*)0x200010e8 = 0);
  NONFAILING(*(uint64_t*)0x200010f0 = 0);
  NONFAILING(*(uint64_t*)0x200010f8 = 0);
  NONFAILING(*(uint64_t*)0x20001100 = 0);
  NONFAILING(*(uint64_t*)0x20001108 = 0);
  NONFAILING(*(uint64_t*)0x20001110 = 0);
  NONFAILING(*(uint64_t*)0x20001118 = 0);
  NONFAILING(*(uint64_t*)0x20001120 = 0);
  NONFAILING(*(uint64_t*)0x20001128 = 0);
  NONFAILING(*(uint64_t*)0x20001130 = 0);
  NONFAILING(*(uint64_t*)0x20001138 = 0);
  NONFAILING(*(uint64_t*)0x20001140 = 0);
  NONFAILING(*(uint64_t*)0x20001148 = 0);
  NONFAILING(*(uint64_t*)0x20001150 = 0);
  NONFAILING(*(uint64_t*)0x20001158 = 0);
  NONFAILING(*(uint64_t*)0x20001160 = 0);
  NONFAILING(*(uint64_t*)0x20001168 = 0);
  NONFAILING(*(uint64_t*)0x20001170 = 0);
  NONFAILING(*(uint64_t*)0x20001178 = 0);
  NONFAILING(*(uint64_t*)0x20001180 = 0);
  NONFAILING(*(uint64_t*)0x20001188 = 0);
  NONFAILING(*(uint64_t*)0x20001190 = 0);
  NONFAILING(*(uint64_t*)0x20001198 = 0);
  NONFAILING(*(uint64_t*)0x200011a0 = 0);
  NONFAILING(*(uint64_t*)0x200011a8 = 0);
  NONFAILING(*(uint64_t*)0x200011b0 = 0);
  NONFAILING(*(uint64_t*)0x200011b8 = 0);
  NONFAILING(*(uint64_t*)0x200011c0 = 0);
  NONFAILING(*(uint64_t*)0x200011c8 = 0);
  NONFAILING(*(uint64_t*)0x200011d0 = 0);
  NONFAILING(*(uint64_t*)0x200011d8 = 0);
  NONFAILING(*(uint64_t*)0x200011e0 = 0);
  NONFAILING(*(uint64_t*)0x200011e8 = 0);
  NONFAILING(*(uint64_t*)0x200011f0 = 0);
  NONFAILING(*(uint64_t*)0x200011f8 = 0);
  NONFAILING(*(uint64_t*)0x20001200 = 0);
  NONFAILING(*(uint64_t*)0x20001208 = 0);
  NONFAILING(*(uint64_t*)0x20001210 = 0);
  NONFAILING(*(uint64_t*)0x20001218 = 0);
  NONFAILING(*(uint64_t*)0x20001220 = 0);
  NONFAILING(*(uint64_t*)0x20001228 = 0);
  NONFAILING(*(uint64_t*)0x20001230 = 0);
  NONFAILING(*(uint64_t*)0x20001238 = 0);
  NONFAILING(*(uint64_t*)0x20001240 = 0);
  NONFAILING(*(uint64_t*)0x20001248 = 0);
  NONFAILING(*(uint64_t*)0x20001250 = 0);
  NONFAILING(*(uint64_t*)0x20001258 = 0);
  NONFAILING(*(uint64_t*)0x20001260 = 0);
  NONFAILING(*(uint64_t*)0x20001268 = 0);
  NONFAILING(*(uint64_t*)0x20001270 = 0);
  NONFAILING(*(uint64_t*)0x20001278 = 0);
  NONFAILING(*(uint64_t*)0x20001280 = 0);
  NONFAILING(*(uint64_t*)0x20001288 = 0);
  NONFAILING(*(uint64_t*)0x20001290 = 0);
  NONFAILING(*(uint64_t*)0x20001298 = 0);
  NONFAILING(*(uint64_t*)0x200012a0 = 0);
  NONFAILING(*(uint64_t*)0x200012a8 = 0);
  NONFAILING(*(uint64_t*)0x200012b0 = 0);
  NONFAILING(*(uint64_t*)0x200012b8 = 0);
  NONFAILING(*(uint64_t*)0x200012c0 = 0);
  NONFAILING(*(uint64_t*)0x200012c8 = 0);
  NONFAILING(*(uint64_t*)0x200012d0 = 0);
  NONFAILING(*(uint64_t*)0x200012d8 = 0);
  NONFAILING(*(uint64_t*)0x200012e0 = 0);
  NONFAILING(*(uint64_t*)0x200012e8 = 0);
  NONFAILING(*(uint64_t*)0x200012f0 = 0);
  NONFAILING(*(uint64_t*)0x200012f8 = 0);
  NONFAILING(*(uint64_t*)0x20001300 = 0);
  NONFAILING(*(uint64_t*)0x20001308 = 0);
  NONFAILING(*(uint64_t*)0x20001310 = 0);
  NONFAILING(*(uint64_t*)0x20001318 = 0);
  NONFAILING(*(uint64_t*)0x20001320 = 0);
  NONFAILING(*(uint64_t*)0x20001328 = 0);
  NONFAILING(*(uint64_t*)0x20001330 = 0);
  syscall(__NR_write, r[0], 0x200009c0, 0x978);
  close_fds();
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  setup_binfmt_misc();
  setup_leak();
  install_segv_handler();
  use_temporary_dir();
  do_sandbox_none();
  check_leaks();
  return 0;
}
