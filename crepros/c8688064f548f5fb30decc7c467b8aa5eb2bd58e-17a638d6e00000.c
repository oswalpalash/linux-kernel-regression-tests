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

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0x0};

void loop(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0ul);
  if (res != -1)
    r[0] = res;
  syscall(__NR_socket, 0x10ul, 0x80002ul, 0);
  syscall(__NR_socket, 0x200000000000011ul, 3ul, 0);
  syscall(__NR_socket, 0x10ul, 3ul, 0x10ul);
  *(uint32_t*)0x200004c0 = 1;
  *(uint32_t*)0x200004c4 = 5;
  *(uint32_t*)0x200004c8 = 0x899;
  *(uint32_t*)0x200004cc = 2;
  *(uint32_t*)0x200004d0 = 0;
  *(uint32_t*)0x200004d4 = -1;
  *(uint32_t*)0x200004d8 = 0;
  *(uint8_t*)0x200004dc = 0;
  *(uint8_t*)0x200004dd = 0;
  *(uint8_t*)0x200004de = 0;
  *(uint8_t*)0x200004df = 0;
  *(uint8_t*)0x200004e0 = 0;
  *(uint8_t*)0x200004e1 = 4;
  *(uint8_t*)0x200004e2 = 0;
  *(uint8_t*)0x200004e3 = 0;
  *(uint8_t*)0x200004e4 = 0;
  *(uint8_t*)0x200004e5 = 0;
  *(uint8_t*)0x200004e6 = 0;
  *(uint8_t*)0x200004e7 = 0;
  *(uint8_t*)0x200004e8 = 0;
  *(uint8_t*)0x200004e9 = 0;
  *(uint8_t*)0x200004ea = 0;
  *(uint8_t*)0x200004eb = 0;
  *(uint32_t*)0x200004ec = 0;
  *(uint32_t*)0x200004f0 = -1;
  *(uint32_t*)0x200004f4 = 0;
  *(uint32_t*)0x200004f8 = 0;
  syscall(__NR_bpf, 0ul, 0x200004c0ul, 0x3cul);
  *(uint32_t*)0x20000180 = 6;
  *(uint32_t*)0x20000184 = 4;
  *(uint64_t*)0x20000188 = 0x20000200;
  *(uint8_t*)0x20000200 = 0x18;
  STORE_BY_BITMASK(uint8_t, , 0x20000201, 2, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000201, 0, 4, 4);
  *(uint16_t*)0x20000202 = 0;
  *(uint32_t*)0x20000204 = 0;
  *(uint8_t*)0x20000208 = 0;
  *(uint8_t*)0x20000209 = 0;
  *(uint16_t*)0x2000020a = 0;
  *(uint32_t*)0x2000020c = 0;
  *(uint8_t*)0x20000210 = 0x85;
  *(uint8_t*)0x20000211 = 0;
  *(uint16_t*)0x20000212 = 0;
  *(uint32_t*)0x20000214 = 0x2c;
  *(uint8_t*)0x20000218 = 0x95;
  *(uint8_t*)0x20000219 = 0;
  *(uint16_t*)0x2000021a = 0;
  *(uint32_t*)0x2000021c = 0;
  *(uint64_t*)0x20000190 = 0x200000c0;
  memcpy((void*)0x200000c0, "GPL\000", 4);
  *(uint32_t*)0x20000198 = 4;
  *(uint32_t*)0x2000019c = 0x1000;
  *(uint64_t*)0x200001a0 = 0x2062b000;
  *(uint32_t*)0x200001a8 = 0;
  *(uint32_t*)0x200001ac = 0;
  *(uint8_t*)0x200001b0 = 0;
  *(uint8_t*)0x200001b1 = 0;
  *(uint8_t*)0x200001b2 = 0;
  *(uint8_t*)0x200001b3 = 0;
  *(uint8_t*)0x200001b4 = 0;
  *(uint8_t*)0x200001b5 = 0;
  *(uint8_t*)0x200001b6 = 0;
  *(uint8_t*)0x200001b7 = 0;
  *(uint8_t*)0x200001b8 = 0;
  *(uint8_t*)0x200001b9 = 0;
  *(uint8_t*)0x200001ba = 0;
  *(uint8_t*)0x200001bb = 0;
  *(uint8_t*)0x200001bc = 0;
  *(uint8_t*)0x200001bd = 0;
  *(uint8_t*)0x200001be = 0;
  *(uint8_t*)0x200001bf = 0;
  *(uint32_t*)0x200001c0 = 0;
  *(uint32_t*)0x200001c4 = 0;
  *(uint32_t*)0x200001c8 = -1;
  *(uint32_t*)0x200001cc = 8;
  *(uint64_t*)0x200001d0 = 0;
  *(uint32_t*)0x200001d8 = 0;
  *(uint32_t*)0x200001dc = 0x10;
  *(uint64_t*)0x200001e0 = 0;
  *(uint32_t*)0x200001e8 = 0;
  *(uint32_t*)0x200001ec = 0;
  *(uint32_t*)0x200001f0 = -1;
  res = syscall(__NR_bpf, 5ul, 0x20000180ul, 0x70ul);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x200004c0 = r[1];
  *(uint32_t*)0x200004c4 = 0xc0;
  *(uint64_t*)0x200004c8 = 0x20000400;
  *(uint64_t*)0x20000470 = 0;
  *(uint64_t*)0x20000488 = 0;
  *(uint64_t*)0x20000498 = 0;
  *(uint64_t*)0x200004a0 = 0;
  *(uint64_t*)0x200004b8 = 0;
  res = syscall(__NR_bpf, 0xful, 0x200004c0ul, 0x10ul);
  if (res != -1)
    r[2] = *(uint32_t*)0x20000404;
  *(uint32_t*)0x20000240 = r[2];
  *(uint32_t*)0x20000244 = 0;
  *(uint32_t*)0x20000248 = 0;
  syscall(__NR_bpf, 0xdul, 0x20000240ul, 0xcul);
  *(uint32_t*)0x20002040 = r[2];
  *(uint32_t*)0x20002044 = 0;
  *(uint32_t*)0x20002048 = 0;
  syscall(__NR_bpf, 0xdul, 0x20002040ul, 0xcul);
  *(uint64_t*)0x20000140 = 0;
  *(uint32_t*)0x20000148 = 0;
  *(uint64_t*)0x20000150 = 0x20000080;
  *(uint64_t*)0x20000080 = 0x20000640;
  *(uint32_t*)0x20000640 = 0xa8;
  *(uint16_t*)0x20000644 = 0x10;
  *(uint16_t*)0x20000646 = 0x801;
  *(uint32_t*)0x20000648 = 0;
  *(uint32_t*)0x2000064c = 0;
  *(uint8_t*)0x20000650 = 0;
  *(uint8_t*)0x20000651 = 0;
  *(uint16_t*)0x20000652 = 0;
  *(uint32_t*)0x20000654 = 0;
  *(uint32_t*)0x20000658 = 0;
  *(uint32_t*)0x2000065c = 0;
  *(uint16_t*)0x20000660 = 0x80;
  *(uint16_t*)0x20000662 = 0x2b;
  *(uint16_t*)0x20000664 = 0x7c;
  *(uint16_t*)0x20000666 = 1;
  *(uint32_t*)0x20000668 = 0xa;
  *(uint32_t*)0x2000066c = 0;
  *(uint64_t*)0x20000670 = 0;
  *(uint64_t*)0x20000678 = 0;
  *(uint32_t*)0x20000680 = 0;
  *(uint32_t*)0x20000684 = 0;
  *(uint64_t*)0x20000688 = 0;
  *(uint32_t*)0x20000690 = 0;
  *(uint32_t*)0x20000694 = 0;
  *(uint8_t*)0x20000698 = 0;
  *(uint8_t*)0x20000699 = 0;
  *(uint8_t*)0x2000069a = 0;
  *(uint8_t*)0x2000069b = 0;
  *(uint8_t*)0x2000069c = 0;
  *(uint8_t*)0x2000069d = 0;
  *(uint8_t*)0x2000069e = 0;
  *(uint8_t*)0x2000069f = 0;
  *(uint8_t*)0x200006a0 = 0;
  *(uint8_t*)0x200006a1 = 0;
  *(uint8_t*)0x200006a2 = 0;
  *(uint8_t*)0x200006a3 = 0;
  *(uint8_t*)0x200006a4 = 0;
  *(uint8_t*)0x200006a5 = 0;
  *(uint8_t*)0x200006a6 = 0;
  *(uint8_t*)0x200006a7 = 0;
  *(uint32_t*)0x200006a8 = 0;
  *(uint32_t*)0x200006ac = 0;
  *(uint32_t*)0x200006b0 = -1;
  *(uint32_t*)0x200006b4 = 8;
  *(uint64_t*)0x200006b8 = 0;
  *(uint32_t*)0x200006c0 = 0;
  *(uint32_t*)0x200006c4 = 0x10;
  *(uint64_t*)0x200006c8 = 0;
  *(uint32_t*)0x200006d0 = 0;
  *(uint32_t*)0x200006d4 = 0;
  *(uint32_t*)0x200006d8 = -1;
  *(uint16_t*)0x200006e0 = 8;
  *(uint16_t*)0x200006e2 = 0x1b;
  *(uint32_t*)0x200006e4 = 0;
  *(uint64_t*)0x20000088 = 0xa8;
  *(uint64_t*)0x20000158 = 1;
  *(uint64_t*)0x20000160 = 0;
  *(uint64_t*)0x20000168 = 0;
  *(uint32_t*)0x20000170 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000140ul, 0ul);
  *(uint8_t*)0x20000100 = -1;
  *(uint8_t*)0x20000101 = -1;
  *(uint8_t*)0x20000102 = -1;
  *(uint8_t*)0x20000103 = -1;
  *(uint8_t*)0x20000104 = -1;
  *(uint8_t*)0x20000105 = -1;
  memcpy((void*)0x20000106, "\x96\xc1\xe9\x57\x4b\x31", 6);
  *(uint16_t*)0x2000010c = htobe16(0x800);
  STORE_BY_BITMASK(uint8_t, , 0x2000010e, 0xa, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x2000010e, 4, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x2000010f, 0, 0, 2);
  STORE_BY_BITMASK(uint8_t, , 0x2000010f, 0x27, 2, 6);
  *(uint16_t*)0x20000110 = htobe16(0x30);
  *(uint16_t*)0x20000112 = htobe16(0x67);
  *(uint16_t*)0x20000114 = htobe16(0);
  *(uint8_t*)0x20000116 = 0xfa;
  *(uint8_t*)0x20000117 = 2;
  *(uint16_t*)0x20000118 = htobe16(0);
  *(uint32_t*)0x2000011a = htobe32(0);
  *(uint32_t*)0x2000011e = htobe32(0);
  *(uint8_t*)0x20000122 = 0x89;
  *(uint8_t*)0x20000123 = 0x13;
  *(uint8_t*)0x20000124 = 0x3d;
  *(uint32_t*)0x20000125 = htobe32(3);
  *(uint8_t*)0x20000129 = 0xac;
  *(uint8_t*)0x2000012a = 0x14;
  *(uint8_t*)0x2000012b = 0x14;
  *(uint8_t*)0x2000012c = 0x2b;
  *(uint8_t*)0x2000012d = 0xac;
  *(uint8_t*)0x2000012e = 0x14;
  *(uint8_t*)0x2000012f = 0x14;
  *(uint8_t*)0x20000130 = 0xbb;
  *(uint32_t*)0x20000131 = htobe32(0xe0000001);
  *(uint8_t*)0x20000136 = 1;
  *(uint8_t*)0x20000137 = 9;
  *(uint16_t*)0x20000138 = htobe16(0);
  *(uint32_t*)0x2000013a = htobe32(0x80);
  struct csum_inet csum_1;
  csum_inet_init(&csum_1);
  csum_inet_update(&csum_1, (const uint8_t*)0x20000136, 8);
  *(uint16_t*)0x20000138 = csum_inet_digest(&csum_1);
  struct csum_inet csum_2;
  csum_inet_init(&csum_2);
  csum_inet_update(&csum_2, (const uint8_t*)0x2000010e, 40);
  *(uint16_t*)0x20000118 = csum_inet_digest(&csum_2);
  syz_emit_ethernet(0x3e, 0x20000100, 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  do_sandbox_none();
  return 0;
}
