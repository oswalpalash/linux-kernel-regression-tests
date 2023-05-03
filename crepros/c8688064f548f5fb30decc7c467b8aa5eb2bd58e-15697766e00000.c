// https://syzkaller.appspot.com/bug?id=c8688064f548f5fb30decc7c467b8aa5eb2bd58e
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

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

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

struct nlmsg {
  char* pos;
  int nesting;
  struct nlattr* nested[8];
  char buf[1024];
};

static struct nlmsg nlmsg;

static void netlink_init(struct nlmsg* nlmsg, int typ, int flags,
                         const void* data, int size)
{
  memset(nlmsg, 0, sizeof(*nlmsg));
  struct nlmsghdr* hdr = (struct nlmsghdr*)nlmsg->buf;
  hdr->nlmsg_type = typ;
  hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | flags;
  memcpy(hdr + 1, data, size);
  nlmsg->pos = (char*)(hdr + 1) + NLMSG_ALIGN(size);
}

static void netlink_attr(struct nlmsg* nlmsg, int typ, const void* data,
                         int size)
{
  struct nlattr* attr = (struct nlattr*)nlmsg->pos;
  attr->nla_len = sizeof(*attr) + size;
  attr->nla_type = typ;
  memcpy(attr + 1, data, size);
  nlmsg->pos += NLMSG_ALIGN(attr->nla_len);
}

static int netlink_send_ext(struct nlmsg* nlmsg, int sock, uint16_t reply_type,
                            int* reply_len)
{
  if (nlmsg->pos > nlmsg->buf + sizeof(nlmsg->buf) || nlmsg->nesting)
    exit(1);
  struct nlmsghdr* hdr = (struct nlmsghdr*)nlmsg->buf;
  hdr->nlmsg_len = nlmsg->pos - nlmsg->buf;
  struct sockaddr_nl addr;
  memset(&addr, 0, sizeof(addr));
  addr.nl_family = AF_NETLINK;
  unsigned n = sendto(sock, nlmsg->buf, hdr->nlmsg_len, 0,
                      (struct sockaddr*)&addr, sizeof(addr));
  if (n != hdr->nlmsg_len)
    exit(1);
  n = recv(sock, nlmsg->buf, sizeof(nlmsg->buf), 0);
  if (hdr->nlmsg_type == NLMSG_DONE) {
    *reply_len = 0;
    return 0;
  }
  if (n < sizeof(struct nlmsghdr))
    exit(1);
  if (reply_len && hdr->nlmsg_type == reply_type) {
    *reply_len = n;
    return 0;
  }
  if (n < sizeof(struct nlmsghdr) + sizeof(struct nlmsgerr))
    exit(1);
  if (hdr->nlmsg_type != NLMSG_ERROR)
    exit(1);
  return -((struct nlmsgerr*)(hdr + 1))->error;
}

static int netlink_send(struct nlmsg* nlmsg, int sock)
{
  return netlink_send_ext(nlmsg, sock, 0, NULL);
}

static void netlink_device_change(struct nlmsg* nlmsg, int sock,
                                  const char* name, bool up, const char* master,
                                  const void* mac, int macsize,
                                  const char* new_name)
{
  struct ifinfomsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  if (up)
    hdr.ifi_flags = hdr.ifi_change = IFF_UP;
  hdr.ifi_index = if_nametoindex(name);
  netlink_init(nlmsg, RTM_NEWLINK, 0, &hdr, sizeof(hdr));
  if (new_name)
    netlink_attr(nlmsg, IFLA_IFNAME, new_name, strlen(new_name));
  if (master) {
    int ifindex = if_nametoindex(master);
    netlink_attr(nlmsg, IFLA_MASTER, &ifindex, sizeof(ifindex));
  }
  if (macsize)
    netlink_attr(nlmsg, IFLA_ADDRESS, mac, macsize);
  int err = netlink_send(nlmsg, sock);
  (void)err;
}

static int netlink_add_addr(struct nlmsg* nlmsg, int sock, const char* dev,
                            const void* addr, int addrsize)
{
  struct ifaddrmsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  hdr.ifa_family = addrsize == 4 ? AF_INET : AF_INET6;
  hdr.ifa_prefixlen = addrsize == 4 ? 24 : 120;
  hdr.ifa_scope = RT_SCOPE_UNIVERSE;
  hdr.ifa_index = if_nametoindex(dev);
  netlink_init(nlmsg, RTM_NEWADDR, NLM_F_CREATE | NLM_F_REPLACE, &hdr,
               sizeof(hdr));
  netlink_attr(nlmsg, IFA_LOCAL, addr, addrsize);
  netlink_attr(nlmsg, IFA_ADDRESS, addr, addrsize);
  return netlink_send(nlmsg, sock);
}

static void netlink_add_addr4(struct nlmsg* nlmsg, int sock, const char* dev,
                              const char* addr)
{
  struct in_addr in_addr;
  inet_pton(AF_INET, addr, &in_addr);
  int err = netlink_add_addr(nlmsg, sock, dev, &in_addr, sizeof(in_addr));
  (void)err;
}

static void netlink_add_addr6(struct nlmsg* nlmsg, int sock, const char* dev,
                              const char* addr)
{
  struct in6_addr in6_addr;
  inet_pton(AF_INET6, addr, &in6_addr);
  int err = netlink_add_addr(nlmsg, sock, dev, &in6_addr, sizeof(in6_addr));
  (void)err;
}

static void netlink_add_neigh(struct nlmsg* nlmsg, int sock, const char* name,
                              const void* addr, int addrsize, const void* mac,
                              int macsize)
{
  struct ndmsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  hdr.ndm_family = addrsize == 4 ? AF_INET : AF_INET6;
  hdr.ndm_ifindex = if_nametoindex(name);
  hdr.ndm_state = NUD_PERMANENT;
  netlink_init(nlmsg, RTM_NEWNEIGH, NLM_F_EXCL | NLM_F_CREATE, &hdr,
               sizeof(hdr));
  netlink_attr(nlmsg, NDA_DST, addr, addrsize);
  netlink_attr(nlmsg, NDA_LLADDR, mac, macsize);
  int err = netlink_send(nlmsg, sock);
  (void)err;
}

static int tunfd = -1;
static int tun_frags_enabled;

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
  netlink_add_addr4(&nlmsg, sock, TUN_IFACE, LOCAL_IPV4);
  netlink_add_addr6(&nlmsg, sock, TUN_IFACE, LOCAL_IPV6);
  uint64_t macaddr = REMOTE_MAC;
  struct in_addr in_addr;
  inet_pton(AF_INET, REMOTE_IPV4, &in_addr);
  netlink_add_neigh(&nlmsg, sock, TUN_IFACE, &in_addr, sizeof(in_addr),
                    &macaddr, ETH_ALEN);
  struct in6_addr in6_addr;
  inet_pton(AF_INET6, REMOTE_IPV6, &in6_addr);
  netlink_add_neigh(&nlmsg, sock, TUN_IFACE, &in6_addr, sizeof(in6_addr),
                    &macaddr, ETH_ALEN);
  macaddr = LOCAL_MAC;
  netlink_device_change(&nlmsg, sock, TUN_IFACE, true, 0, &macaddr, ETH_ALEN,
                        NULL);
  close(sock);
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
    NONFAILING(full = frags->full);
    NONFAILING(count = frags->count);
    if (count > MAX_FRAGS)
      count = MAX_FRAGS;
    for (i = 0; i < count && length != 0; i++) {
      uint32_t size = 0;
      NONFAILING(size = frags->frags[i]);
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
  if (unshare(CLONE_NEWNET)) {
  }
  initialize_tun();
  loop();
  exit(1);
}

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void loop(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0ul);
  if (res != -1)
    r[0] = res;
  NONFAILING(*(uint32_t*)0x20000180 = 6);
  NONFAILING(*(uint32_t*)0x20000184 = 4);
  NONFAILING(*(uint64_t*)0x20000188 = 0x20000200);
  NONFAILING(*(uint8_t*)0x20000200 = 0x18);
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000201, 2, 0, 4));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000201, 0, 4, 4));
  NONFAILING(*(uint16_t*)0x20000202 = 0);
  NONFAILING(*(uint32_t*)0x20000204 = 0);
  NONFAILING(*(uint8_t*)0x20000208 = 0);
  NONFAILING(*(uint8_t*)0x20000209 = 0);
  NONFAILING(*(uint16_t*)0x2000020a = 0);
  NONFAILING(*(uint32_t*)0x2000020c = 0);
  NONFAILING(*(uint8_t*)0x20000210 = 0x85);
  NONFAILING(*(uint8_t*)0x20000211 = 0);
  NONFAILING(*(uint16_t*)0x20000212 = 0);
  NONFAILING(*(uint32_t*)0x20000214 = 0x2c);
  NONFAILING(*(uint8_t*)0x20000218 = 0x95);
  NONFAILING(*(uint8_t*)0x20000219 = 0);
  NONFAILING(*(uint16_t*)0x2000021a = 0);
  NONFAILING(*(uint32_t*)0x2000021c = 0);
  NONFAILING(*(uint64_t*)0x20000190 = 0x200000c0);
  NONFAILING(memcpy((void*)0x200000c0, "GPL\000", 4));
  NONFAILING(*(uint32_t*)0x20000198 = 4);
  NONFAILING(*(uint32_t*)0x2000019c = 0x1000);
  NONFAILING(*(uint64_t*)0x200001a0 = 0x2062b000);
  NONFAILING(*(uint32_t*)0x200001a8 = 0);
  NONFAILING(*(uint32_t*)0x200001ac = 0);
  NONFAILING(*(uint8_t*)0x200001b0 = 0);
  NONFAILING(*(uint8_t*)0x200001b1 = 0);
  NONFAILING(*(uint8_t*)0x200001b2 = 0);
  NONFAILING(*(uint8_t*)0x200001b3 = 0);
  NONFAILING(*(uint8_t*)0x200001b4 = 0);
  NONFAILING(*(uint8_t*)0x200001b5 = 0);
  NONFAILING(*(uint8_t*)0x200001b6 = 0);
  NONFAILING(*(uint8_t*)0x200001b7 = 0);
  NONFAILING(*(uint8_t*)0x200001b8 = 0);
  NONFAILING(*(uint8_t*)0x200001b9 = 0);
  NONFAILING(*(uint8_t*)0x200001ba = 0);
  NONFAILING(*(uint8_t*)0x200001bb = 0);
  NONFAILING(*(uint8_t*)0x200001bc = 0);
  NONFAILING(*(uint8_t*)0x200001bd = 0);
  NONFAILING(*(uint8_t*)0x200001be = 0);
  NONFAILING(*(uint8_t*)0x200001bf = 0);
  NONFAILING(*(uint32_t*)0x200001c0 = 0);
  NONFAILING(*(uint32_t*)0x200001c4 = 0);
  NONFAILING(*(uint32_t*)0x200001c8 = -1);
  NONFAILING(*(uint32_t*)0x200001cc = 8);
  NONFAILING(*(uint64_t*)0x200001d0 = 0);
  NONFAILING(*(uint32_t*)0x200001d8 = 0);
  NONFAILING(*(uint32_t*)0x200001dc = 0x10);
  NONFAILING(*(uint64_t*)0x200001e0 = 0);
  NONFAILING(*(uint32_t*)0x200001e8 = 0);
  NONFAILING(*(uint32_t*)0x200001ec = 0);
  NONFAILING(*(uint32_t*)0x200001f0 = -1);
  res = syscall(__NR_bpf, 5ul, 0x20000180ul, 0x70ul);
  if (res != -1)
    r[1] = res;
  NONFAILING(*(uint64_t*)0x20000140 = 0);
  NONFAILING(*(uint32_t*)0x20000148 = 0);
  NONFAILING(*(uint64_t*)0x20000150 = 0x20000080);
  NONFAILING(*(uint64_t*)0x20000080 = 0x20000040);
  NONFAILING(*(uint32_t*)0x20000040 = 0x34);
  NONFAILING(*(uint16_t*)0x20000044 = 0x10);
  NONFAILING(*(uint16_t*)0x20000046 = 0x801);
  NONFAILING(*(uint32_t*)0x20000048 = 0);
  NONFAILING(*(uint32_t*)0x2000004c = 0);
  NONFAILING(*(uint8_t*)0x20000050 = 0);
  NONFAILING(*(uint8_t*)0x20000051 = 0);
  NONFAILING(*(uint16_t*)0x20000052 = 0);
  NONFAILING(*(uint32_t*)0x20000054 = 0);
  NONFAILING(*(uint32_t*)0x20000058 = 0);
  NONFAILING(*(uint32_t*)0x2000005c = 0);
  NONFAILING(*(uint16_t*)0x20000060 = 0xc);
  NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x20000062, 0x2b, 0, 14));
  NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x20000063, 0, 6, 1));
  NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x20000063, 1, 7, 1));
  NONFAILING(*(uint16_t*)0x20000064 = 8);
  NONFAILING(*(uint16_t*)0x20000066 = 1);
  NONFAILING(*(uint32_t*)0x20000068 = r[1]);
  NONFAILING(*(uint16_t*)0x2000006c = 8);
  NONFAILING(*(uint16_t*)0x2000006e = 0x1b);
  NONFAILING(*(uint32_t*)0x20000070 = 0);
  NONFAILING(*(uint64_t*)0x20000088 = 0x34);
  NONFAILING(*(uint64_t*)0x20000158 = 1);
  NONFAILING(*(uint64_t*)0x20000160 = 0x300000000000000);
  NONFAILING(*(uint64_t*)0x20000168 = 0);
  NONFAILING(*(uint32_t*)0x20000170 = 0);
  syscall(__NR_sendmsg, r[0], 0x20000140ul, 0ul);
  NONFAILING(*(uint8_t*)0x20000100 = 0xaa);
  NONFAILING(*(uint8_t*)0x20000101 = 0xaa);
  NONFAILING(*(uint8_t*)0x20000102 = 0xaa);
  NONFAILING(*(uint8_t*)0x20000103 = 0xaa);
  NONFAILING(*(uint8_t*)0x20000104 = 0xaa);
  NONFAILING(*(uint8_t*)0x20000105 = 0xaa);
  NONFAILING(*(uint8_t*)0x20000106 = 0xaa);
  NONFAILING(*(uint8_t*)0x20000107 = 0xaa);
  NONFAILING(*(uint8_t*)0x20000108 = 0xaa);
  NONFAILING(*(uint8_t*)0x20000109 = 0xaa);
  NONFAILING(*(uint8_t*)0x2000010a = 0xaa);
  NONFAILING(*(uint8_t*)0x2000010b = 0);
  NONFAILING(*(uint16_t*)0x2000010c = htobe16(0x86dd));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x2000010e, 0, 0, 4));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x2000010e, 6, 4, 4));
  NONFAILING(memcpy((void*)0x2000010f, "\x20\x92\x00", 3));
  NONFAILING(*(uint16_t*)0x20000112 = htobe16(0x18));
  NONFAILING(*(uint8_t*)0x20000114 = 6);
  NONFAILING(*(uint8_t*)0x20000115 = 0);
  NONFAILING(*(uint8_t*)0x20000116 = 0);
  NONFAILING(*(uint8_t*)0x20000117 = 0);
  NONFAILING(*(uint8_t*)0x20000118 = 0);
  NONFAILING(*(uint8_t*)0x20000119 = 0);
  NONFAILING(*(uint8_t*)0x2000011a = 0);
  NONFAILING(*(uint8_t*)0x2000011b = 0);
  NONFAILING(*(uint8_t*)0x2000011c = 0);
  NONFAILING(*(uint8_t*)0x2000011d = 0);
  NONFAILING(*(uint8_t*)0x2000011e = 0);
  NONFAILING(*(uint8_t*)0x2000011f = 0);
  NONFAILING(*(uint8_t*)0x20000120 = 0);
  NONFAILING(*(uint8_t*)0x20000121 = 0);
  NONFAILING(*(uint8_t*)0x20000122 = 0);
  NONFAILING(*(uint8_t*)0x20000123 = 0);
  NONFAILING(*(uint8_t*)0x20000124 = 0);
  NONFAILING(*(uint8_t*)0x20000125 = 0);
  NONFAILING(*(uint8_t*)0x20000126 = 0xfe);
  NONFAILING(*(uint8_t*)0x20000127 = 0x80);
  NONFAILING(*(uint8_t*)0x20000128 = 0);
  NONFAILING(*(uint8_t*)0x20000129 = 0);
  NONFAILING(*(uint8_t*)0x2000012a = 0);
  NONFAILING(*(uint8_t*)0x2000012b = 0);
  NONFAILING(*(uint8_t*)0x2000012c = 0);
  NONFAILING(*(uint8_t*)0x2000012d = 0);
  NONFAILING(*(uint8_t*)0x2000012e = 0);
  NONFAILING(*(uint8_t*)0x2000012f = 0);
  NONFAILING(*(uint8_t*)0x20000130 = 0);
  NONFAILING(*(uint8_t*)0x20000131 = 0);
  NONFAILING(*(uint8_t*)0x20000132 = 0);
  NONFAILING(*(uint8_t*)0x20000133 = 0);
  NONFAILING(*(uint8_t*)0x20000134 = 0);
  NONFAILING(*(uint8_t*)0x20000135 = 0xaa);
  NONFAILING(*(uint16_t*)0x20000136 = htobe16(0));
  NONFAILING(*(uint16_t*)0x20000138 = htobe16(0x4e22));
  NONFAILING(*(uint32_t*)0x2000013a = 0x41424344);
  NONFAILING(*(uint32_t*)0x2000013e = 0x41424344);
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000142, 0, 0, 1));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000142, 0, 1, 3));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000142, 6, 4, 4));
  NONFAILING(*(uint8_t*)0x20000143 = 0xc2);
  NONFAILING(*(uint16_t*)0x20000144 = htobe16(0));
  NONFAILING(*(uint16_t*)0x20000146 = htobe16(0));
  NONFAILING(*(uint16_t*)0x20000148 = htobe16(0));
  NONFAILING(*(uint8_t*)0x2000014a = 6);
  NONFAILING(*(uint8_t*)0x2000014b = 2);
  struct csum_inet csum_1;
  csum_inet_init(&csum_1);
  NONFAILING(csum_inet_update(&csum_1, (const uint8_t*)0x20000116, 16));
  NONFAILING(csum_inet_update(&csum_1, (const uint8_t*)0x20000126, 16));
  uint32_t csum_1_chunk_2 = 0x18000000;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_2, 4);
  uint32_t csum_1_chunk_3 = 0x6000000;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_3, 4);
  NONFAILING(csum_inet_update(&csum_1, (const uint8_t*)0x20000136, 24));
  NONFAILING(*(uint16_t*)0x20000146 = csum_inet_digest(&csum_1));
  syz_emit_ethernet(0x4e, 0x20000100, 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  install_segv_handler();
  do_sandbox_none();
  return 0;
}
