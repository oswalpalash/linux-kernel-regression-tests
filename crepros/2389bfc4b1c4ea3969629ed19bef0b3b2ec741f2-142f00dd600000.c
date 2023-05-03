// https://syzkaller.appspot.com/bug?id=2389bfc4b1c4ea3969629ed19bef0b3b2ec741f2
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
}

static void close_fds()
{
  int fd;
  for (fd = 3; fd < 30; fd++)
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

#ifndef __NR_bind
#define __NR_bind 361
#endif
#ifndef __NR_getsockname
#define __NR_getsockname 367
#endif
#ifndef __NR_ioctl
#define __NR_ioctl 54
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_sendmmsg
#define __NR_sendmmsg 345
#endif
#ifndef __NR_sendmsg
#define __NR_sendmsg 370
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[10] = {0xffffffffffffffff,
                  0xffffffffffffffff,
                  0xffffffffffffffff,
                  0xffffffffffffffff,
                  0xffffffffffffffff,
                  0x0,
                  0x0,
                  0xffffffffffffffff,
                  0xffffffffffffffff,
                  0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x11, 3, 0x300);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000000, "batadv0\000\f\000\000\000\000\000\000\000", 16);
  *(uint32_t*)0x20000010 = 0;
  res = syscall(__NR_ioctl, (intptr_t)r[0], 0x8933, 0x20000000);
  if (res != -1)
    r[1] = *(uint32_t*)0x20000010;
  *(uint32_t*)0x20000140 = 0;
  *(uint32_t*)0x20000144 = 0;
  *(uint32_t*)0x20000148 = 0x200000c0;
  *(uint32_t*)0x200000c0 = 0x20000040;
  memcpy((void*)0x20000040, "\x44\x00\x00\x00\x2c\x00\x01\x07\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         20);
  *(uint32_t*)0x20000054 = r[1];
  memcpy((void*)0x20000058, "\x00\x00\x00\x00\x00\x00\x00\x00\xf1\xff\x00\x20"
                            "\x0c\x00\x01\x00\x74\x63\x69\x6e\x64\x65\x78\x00"
                            "\x14\x00\x02\x00\x08\x00\x01\x00\x00\x00\x00\x00"
                            "\x08\x00\x02\x00\x00\x00\x00\x00",
         44);
  *(uint32_t*)0x200000c4 = 0x44;
  *(uint32_t*)0x2000014c = 1;
  *(uint32_t*)0x20000150 = 0;
  *(uint32_t*)0x20000154 = 0;
  *(uint32_t*)0x20000158 = 0;
  syscall(__NR_sendmsg, -1, 0x20000140, 0);
  res = syscall(__NR_socket, 0x10, 0x80002, 0);
  if (res != -1)
    r[2] = res;
  syscall(__NR_sendmmsg, (intptr_t)r[2], 0x20000140, 0x332, 0);
  res = syscall(__NR_socket, 0x10, 3, 0);
  if (res != -1)
    r[3] = res;
  res = syscall(__NR_socket, 0x11, 3, 0x300);
  if (res != -1)
    r[4] = res;
  memcpy((void*)0x20000000, "batadv0\000\000\000\000\000\000\000\000\000", 16);
  *(uint32_t*)0x20000010 = 0;
  res = syscall(__NR_ioctl, (intptr_t)r[4], 0x8933, 0x20000000);
  if (res != -1)
    r[5] = *(uint32_t*)0x20000010;
  *(uint16_t*)0x20000640 = 0x11;
  *(uint16_t*)0x20000642 = htobe16(0);
  *(uint32_t*)0x20000644 = r[5];
  *(uint16_t*)0x20000648 = 1;
  *(uint8_t*)0x2000064a = 0;
  *(uint8_t*)0x2000064b = 6;
  *(uint8_t*)0x2000064c = 1;
  *(uint8_t*)0x2000064d = 0x80;
  *(uint8_t*)0x2000064e = 0xc2;
  *(uint8_t*)0x2000064f = 0;
  *(uint8_t*)0x20000650 = 0;
  *(uint8_t*)0x20000651 = 0;
  *(uint8_t*)0x20000652 = 0;
  *(uint8_t*)0x20000653 = 0;
  syscall(__NR_bind, (intptr_t)r[4], 0x20000640, 0x14);
  *(uint32_t*)0x20000140 = 0x14;
  res = syscall(__NR_getsockname, (intptr_t)r[4], 0x20000100, 0x20000140);
  if (res != -1)
    r[6] = *(uint32_t*)0x20000104;
  *(uint32_t*)0x20000240 = 0;
  *(uint32_t*)0x20000244 = 0;
  *(uint32_t*)0x20000248 = 0x20000080;
  *(uint32_t*)0x20000080 = 0x200006c0;
  *(uint32_t*)0x200006c0 = 0x444;
  *(uint16_t*)0x200006c4 = 0x24;
  *(uint16_t*)0x200006c6 = 0x507;
  *(uint32_t*)0x200006c8 = 0;
  *(uint32_t*)0x200006cc = 0;
  *(uint8_t*)0x200006d0 = 0;
  *(uint32_t*)0x200006d4 = r[6];
  *(uint16_t*)0x200006d8 = 0;
  *(uint16_t*)0x200006da = 0;
  *(uint16_t*)0x200006dc = -1;
  *(uint16_t*)0x200006de = -1;
  *(uint16_t*)0x200006e0 = 0;
  *(uint16_t*)0x200006e2 = 0;
  *(uint16_t*)0x200006e4 = 8;
  *(uint16_t*)0x200006e6 = 1;
  memcpy((void*)0x200006e8, "cbq\000", 4);
  *(uint16_t*)0x200006ec = 0x418;
  *(uint16_t*)0x200006ee = 2;
  *(uint16_t*)0x200006f0 = 0x404;
  *(uint16_t*)0x200006f2 = 6;
  *(uint32_t*)0x200006f4 = 0;
  *(uint32_t*)0x200006f8 = 0;
  *(uint32_t*)0x200006fc = 0;
  *(uint32_t*)0x20000700 = 0;
  *(uint32_t*)0x20000704 = 0;
  *(uint32_t*)0x20000708 = 0;
  *(uint32_t*)0x2000070c = 0;
  *(uint32_t*)0x20000710 = 0;
  *(uint32_t*)0x20000714 = 0;
  *(uint32_t*)0x20000718 = 0x8000000;
  *(uint32_t*)0x2000071c = 0;
  *(uint32_t*)0x20000720 = 0;
  *(uint32_t*)0x20000724 = 0;
  *(uint32_t*)0x20000728 = 0;
  *(uint32_t*)0x2000072c = 0;
  *(uint32_t*)0x20000730 = 0;
  *(uint32_t*)0x20000734 = 0;
  *(uint32_t*)0x20000738 = 0;
  *(uint32_t*)0x2000073c = 0;
  *(uint32_t*)0x20000740 = 0;
  *(uint32_t*)0x20000744 = 0;
  *(uint32_t*)0x20000748 = 0;
  *(uint32_t*)0x2000074c = 0;
  *(uint32_t*)0x20000750 = 0;
  *(uint32_t*)0x20000754 = 0;
  *(uint32_t*)0x20000758 = 0;
  *(uint32_t*)0x2000075c = 0;
  *(uint32_t*)0x20000760 = 0;
  *(uint32_t*)0x20000764 = 0;
  *(uint32_t*)0x20000768 = 0;
  *(uint32_t*)0x2000076c = 0;
  *(uint32_t*)0x20000770 = 0;
  *(uint32_t*)0x20000774 = 0;
  *(uint32_t*)0x20000778 = 0;
  *(uint32_t*)0x2000077c = 0;
  *(uint32_t*)0x20000780 = 0;
  *(uint32_t*)0x20000784 = 0;
  *(uint32_t*)0x20000788 = 0;
  *(uint32_t*)0x2000078c = 0;
  *(uint32_t*)0x20000790 = 0;
  *(uint32_t*)0x20000794 = 0;
  *(uint32_t*)0x20000798 = 0;
  *(uint32_t*)0x2000079c = 0;
  *(uint32_t*)0x200007a0 = 0;
  *(uint32_t*)0x200007a4 = 0;
  *(uint32_t*)0x200007a8 = 0;
  *(uint32_t*)0x200007ac = 0;
  *(uint32_t*)0x200007b0 = 0;
  *(uint32_t*)0x200007b4 = 0;
  *(uint32_t*)0x200007b8 = 0;
  *(uint32_t*)0x200007bc = 0;
  *(uint32_t*)0x200007c0 = 0;
  *(uint32_t*)0x200007c4 = 0;
  *(uint32_t*)0x200007c8 = 0;
  *(uint32_t*)0x200007cc = 0;
  *(uint32_t*)0x200007d0 = 0;
  *(uint32_t*)0x200007d4 = 0;
  *(uint32_t*)0x200007d8 = 0;
  *(uint32_t*)0x200007dc = 0;
  *(uint32_t*)0x200007e0 = 0;
  *(uint32_t*)0x200007e4 = 0;
  *(uint32_t*)0x200007e8 = 0;
  *(uint32_t*)0x200007ec = 0;
  *(uint32_t*)0x200007f0 = 0;
  *(uint32_t*)0x200007f4 = 0;
  *(uint32_t*)0x200007f8 = 0;
  *(uint32_t*)0x200007fc = 0;
  *(uint32_t*)0x20000800 = 0;
  *(uint32_t*)0x20000804 = 0;
  *(uint32_t*)0x20000808 = 0;
  *(uint32_t*)0x2000080c = 0;
  *(uint32_t*)0x20000810 = 0;
  *(uint32_t*)0x20000814 = 0;
  *(uint32_t*)0x20000818 = 0;
  *(uint32_t*)0x2000081c = 0;
  *(uint32_t*)0x20000820 = 0;
  *(uint32_t*)0x20000824 = 0;
  *(uint32_t*)0x20000828 = 0;
  *(uint32_t*)0x2000082c = 0;
  *(uint32_t*)0x20000830 = 0;
  *(uint32_t*)0x20000834 = 0;
  *(uint32_t*)0x20000838 = 0;
  *(uint32_t*)0x2000083c = 0;
  *(uint32_t*)0x20000840 = 0;
  *(uint32_t*)0x20000844 = 0;
  *(uint32_t*)0x20000848 = 0;
  *(uint32_t*)0x2000084c = 0;
  *(uint32_t*)0x20000850 = 0;
  *(uint32_t*)0x20000854 = 0;
  *(uint32_t*)0x20000858 = 0;
  *(uint32_t*)0x2000085c = 0;
  *(uint32_t*)0x20000860 = 0;
  *(uint32_t*)0x20000864 = 0;
  *(uint32_t*)0x20000868 = 0;
  *(uint32_t*)0x2000086c = 0;
  *(uint32_t*)0x20000870 = 0;
  *(uint32_t*)0x20000874 = 0;
  *(uint32_t*)0x20000878 = 0;
  *(uint32_t*)0x2000087c = 0;
  *(uint32_t*)0x20000880 = 0;
  *(uint32_t*)0x20000884 = 0;
  *(uint32_t*)0x20000888 = 0;
  *(uint32_t*)0x2000088c = 0;
  *(uint32_t*)0x20000890 = 0;
  *(uint32_t*)0x20000894 = 0;
  *(uint32_t*)0x20000898 = 0;
  *(uint32_t*)0x2000089c = 0;
  *(uint32_t*)0x200008a0 = 0;
  *(uint32_t*)0x200008a4 = 0;
  *(uint32_t*)0x200008a8 = 0;
  *(uint32_t*)0x200008ac = 0;
  *(uint32_t*)0x200008b0 = 0;
  *(uint32_t*)0x200008b4 = 0;
  *(uint32_t*)0x200008b8 = 0;
  *(uint32_t*)0x200008bc = 0;
  *(uint32_t*)0x200008c0 = 0;
  *(uint32_t*)0x200008c4 = 0;
  *(uint32_t*)0x200008c8 = 0;
  *(uint32_t*)0x200008cc = 0;
  *(uint32_t*)0x200008d0 = 0;
  *(uint32_t*)0x200008d4 = 0;
  *(uint32_t*)0x200008d8 = 0;
  *(uint32_t*)0x200008dc = 0;
  *(uint32_t*)0x200008e0 = 0;
  *(uint32_t*)0x200008e4 = 0;
  *(uint32_t*)0x200008e8 = 0;
  *(uint32_t*)0x200008ec = 0;
  *(uint32_t*)0x200008f0 = 0;
  *(uint32_t*)0x200008f4 = 0;
  *(uint32_t*)0x200008f8 = 0;
  *(uint32_t*)0x200008fc = 0;
  *(uint32_t*)0x20000900 = 0;
  *(uint32_t*)0x20000904 = 0;
  *(uint32_t*)0x20000908 = 0;
  *(uint32_t*)0x2000090c = 0;
  *(uint32_t*)0x20000910 = 0;
  *(uint32_t*)0x20000914 = 0;
  *(uint32_t*)0x20000918 = 0;
  *(uint32_t*)0x2000091c = 0;
  *(uint32_t*)0x20000920 = 0;
  *(uint32_t*)0x20000924 = 0;
  *(uint32_t*)0x20000928 = 0;
  *(uint32_t*)0x2000092c = 0;
  *(uint32_t*)0x20000930 = 0;
  *(uint32_t*)0x20000934 = 0;
  *(uint32_t*)0x20000938 = 0;
  *(uint32_t*)0x2000093c = 0;
  *(uint32_t*)0x20000940 = 0;
  *(uint32_t*)0x20000944 = 0;
  *(uint32_t*)0x20000948 = 0;
  *(uint32_t*)0x2000094c = 0;
  *(uint32_t*)0x20000950 = 0;
  *(uint32_t*)0x20000954 = 0;
  *(uint32_t*)0x20000958 = 0;
  *(uint32_t*)0x2000095c = 0;
  *(uint32_t*)0x20000960 = 0;
  *(uint32_t*)0x20000964 = 0;
  *(uint32_t*)0x20000968 = 0;
  *(uint32_t*)0x2000096c = 0;
  *(uint32_t*)0x20000970 = 0;
  *(uint32_t*)0x20000974 = 0;
  *(uint32_t*)0x20000978 = 0;
  *(uint32_t*)0x2000097c = 0;
  *(uint32_t*)0x20000980 = 0;
  *(uint32_t*)0x20000984 = 0;
  *(uint32_t*)0x20000988 = 0;
  *(uint32_t*)0x2000098c = 0;
  *(uint32_t*)0x20000990 = 0;
  *(uint32_t*)0x20000994 = 0;
  *(uint32_t*)0x20000998 = 0;
  *(uint32_t*)0x2000099c = 0;
  *(uint32_t*)0x200009a0 = 0;
  *(uint32_t*)0x200009a4 = 0;
  *(uint32_t*)0x200009a8 = 0;
  *(uint32_t*)0x200009ac = 0;
  *(uint32_t*)0x200009b0 = 0;
  *(uint32_t*)0x200009b4 = 0;
  *(uint32_t*)0x200009b8 = 0;
  *(uint32_t*)0x200009bc = 0;
  *(uint32_t*)0x200009c0 = 0;
  *(uint32_t*)0x200009c4 = 0;
  *(uint32_t*)0x200009c8 = 0;
  *(uint32_t*)0x200009cc = 0;
  *(uint32_t*)0x200009d0 = 0;
  *(uint32_t*)0x200009d4 = 0;
  *(uint32_t*)0x200009d8 = 0;
  *(uint32_t*)0x200009dc = 0;
  *(uint32_t*)0x200009e0 = 0;
  *(uint32_t*)0x200009e4 = 0;
  *(uint32_t*)0x200009e8 = 0;
  *(uint32_t*)0x200009ec = 0;
  *(uint32_t*)0x200009f0 = 0;
  *(uint32_t*)0x200009f4 = 0;
  *(uint32_t*)0x200009f8 = 0;
  *(uint32_t*)0x200009fc = 0;
  *(uint32_t*)0x20000a00 = 0;
  *(uint32_t*)0x20000a04 = 0;
  *(uint32_t*)0x20000a08 = 0;
  *(uint32_t*)0x20000a0c = 0;
  *(uint32_t*)0x20000a10 = 0;
  *(uint32_t*)0x20000a14 = 0;
  *(uint32_t*)0x20000a18 = 0;
  *(uint32_t*)0x20000a1c = 0;
  *(uint32_t*)0x20000a20 = 0;
  *(uint32_t*)0x20000a24 = 0;
  *(uint32_t*)0x20000a28 = 0;
  *(uint32_t*)0x20000a2c = 0;
  *(uint32_t*)0x20000a30 = 0;
  *(uint32_t*)0x20000a34 = 0;
  *(uint32_t*)0x20000a38 = 0;
  *(uint32_t*)0x20000a3c = 0;
  *(uint32_t*)0x20000a40 = 0;
  *(uint32_t*)0x20000a44 = 0;
  *(uint32_t*)0x20000a48 = 0;
  *(uint32_t*)0x20000a4c = 0;
  *(uint32_t*)0x20000a50 = 0;
  *(uint32_t*)0x20000a54 = 0;
  *(uint32_t*)0x20000a58 = 0;
  *(uint32_t*)0x20000a5c = 0;
  *(uint32_t*)0x20000a60 = 0;
  *(uint32_t*)0x20000a64 = 0;
  *(uint32_t*)0x20000a68 = 0;
  *(uint32_t*)0x20000a6c = 0;
  *(uint32_t*)0x20000a70 = 0;
  *(uint32_t*)0x20000a74 = 0;
  *(uint32_t*)0x20000a78 = 0;
  *(uint32_t*)0x20000a7c = 0;
  *(uint32_t*)0x20000a80 = 0;
  *(uint32_t*)0x20000a84 = 0;
  *(uint32_t*)0x20000a88 = 0;
  *(uint32_t*)0x20000a8c = 0;
  *(uint32_t*)0x20000a90 = 0;
  *(uint32_t*)0x20000a94 = 0;
  *(uint32_t*)0x20000a98 = 0;
  *(uint32_t*)0x20000a9c = 0;
  *(uint32_t*)0x20000aa0 = 0;
  *(uint32_t*)0x20000aa4 = 0;
  *(uint32_t*)0x20000aa8 = 0;
  *(uint32_t*)0x20000aac = 0;
  *(uint32_t*)0x20000ab0 = 0;
  *(uint32_t*)0x20000ab4 = 0;
  *(uint32_t*)0x20000ab8 = 0;
  *(uint32_t*)0x20000abc = 0;
  *(uint32_t*)0x20000ac0 = 0;
  *(uint32_t*)0x20000ac4 = 0;
  *(uint32_t*)0x20000ac8 = 0;
  *(uint32_t*)0x20000acc = 0;
  *(uint32_t*)0x20000ad0 = 0;
  *(uint32_t*)0x20000ad4 = 0;
  *(uint32_t*)0x20000ad8 = 0;
  *(uint32_t*)0x20000adc = 0;
  *(uint32_t*)0x20000ae0 = 0;
  *(uint32_t*)0x20000ae4 = 0;
  *(uint32_t*)0x20000ae8 = 0;
  *(uint32_t*)0x20000aec = 0;
  *(uint32_t*)0x20000af0 = 0;
  *(uint16_t*)0x20000af4 = 0x10;
  *(uint16_t*)0x20000af6 = 5;
  *(uint8_t*)0x20000af8 = 5;
  *(uint8_t*)0x20000af9 = 0;
  *(uint16_t*)0x20000afa = 0;
  *(uint16_t*)0x20000afc = 0;
  *(uint16_t*)0x20000afe = 0;
  *(uint32_t*)0x20000b00 = 0x7f;
  *(uint32_t*)0x20000084 = 0x444;
  *(uint32_t*)0x2000024c = 1;
  *(uint32_t*)0x20000250 = 0;
  *(uint32_t*)0x20000254 = 0;
  *(uint32_t*)0x20000258 = 0;
  syscall(__NR_sendmsg, (intptr_t)r[3], 0x20000240, 0);
  res = syscall(__NR_socket, 0x11, 3, 0x300);
  if (res != -1)
    r[7] = res;
  memcpy((void*)0x20000000, "batadv0\000\f\000\000\000\000\000\000\000", 16);
  *(uint32_t*)0x20000010 = 0;
  res = syscall(__NR_ioctl, (intptr_t)r[7], 0x8933, 0x20000000);
  if (res != -1)
    r[8] = *(uint32_t*)0x20000010;
  *(uint32_t*)0x20000140 = 0;
  *(uint32_t*)0x20000144 = 0;
  *(uint32_t*)0x20000148 = 0x200000c0;
  *(uint32_t*)0x200000c0 = 0x20000040;
  memcpy((void*)0x20000040, "\x44\x00\x00\x00\x2c\x00\x01\x07\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         20);
  *(uint32_t*)0x20000054 = r[8];
  memcpy((void*)0x20000058, "\x00\x00\x00\x00\x00\x00\x00\x00\xf1\xff\x00\x20"
                            "\x0c\x00\x01\x00\x74\x63\x69\x6e\x64\x65\x78\x00"
                            "\x14\x00\x02\x00\x08\x00\x01\x00\x00\xa2\xb4\x00"
                            "\x08\x00\x02\x00\x00\x00\x00\x00",
         44);
  *(uint32_t*)0x200000c4 = 0x44;
  *(uint32_t*)0x2000014c = 1;
  *(uint32_t*)0x20000150 = 0;
  *(uint32_t*)0x20000154 = 0;
  *(uint32_t*)0x20000158 = 0;
  syscall(__NR_sendmsg, -1, 0x20000140, 0);
  res = syscall(__NR_socket, 0x10, 0x80002, 0);
  if (res != -1)
    r[9] = res;
  syscall(__NR_sendmmsg, (intptr_t)r[9], 0x20000140, 0x332, 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  do_sandbox_none();
  return 0;
}
