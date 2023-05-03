// https://syzkaller.appspot.com/bug?id=09636f2b72962748fa4f9e969e071fc2a2053045
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
#include <linux/genetlink.h>
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

static unsigned long long procid;

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
  char buf[4096];
};

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
  if (size > 0)
    memcpy(attr + 1, data, size);
  nlmsg->pos += NLMSG_ALIGN(attr->nla_len);
}

static void netlink_nest(struct nlmsg* nlmsg, int typ)
{
  struct nlattr* attr = (struct nlattr*)nlmsg->pos;
  attr->nla_type = typ;
  nlmsg->pos += sizeof(*attr);
  nlmsg->nested[nlmsg->nesting++] = attr;
}

static void netlink_done(struct nlmsg* nlmsg)
{
  struct nlattr* attr = nlmsg->nested[--nlmsg->nesting];
  attr->nla_len = nlmsg->pos - (char*)attr;
}

static int netlink_send_ext(struct nlmsg* nlmsg, int sock, uint16_t reply_type,
                            int* reply_len, bool dofail)
{
  if (nlmsg->pos > nlmsg->buf + sizeof(nlmsg->buf) || nlmsg->nesting)
    exit(1);
  struct nlmsghdr* hdr = (struct nlmsghdr*)nlmsg->buf;
  hdr->nlmsg_len = nlmsg->pos - nlmsg->buf;
  struct sockaddr_nl addr;
  memset(&addr, 0, sizeof(addr));
  addr.nl_family = AF_NETLINK;
  ssize_t n = sendto(sock, nlmsg->buf, hdr->nlmsg_len, 0,
                     (struct sockaddr*)&addr, sizeof(addr));
  if (n != (ssize_t)hdr->nlmsg_len) {
    if (dofail)
      exit(1);
    return -1;
  }
  n = recv(sock, nlmsg->buf, sizeof(nlmsg->buf), 0);
  if (reply_len)
    *reply_len = 0;
  if (n < 0) {
    if (dofail)
      exit(1);
    return -1;
  }
  if (n < (ssize_t)sizeof(struct nlmsghdr)) {
    errno = EINVAL;
    if (dofail)
      exit(1);
    return -1;
  }
  if (hdr->nlmsg_type == NLMSG_DONE)
    return 0;
  if (reply_len && hdr->nlmsg_type == reply_type) {
    *reply_len = n;
    return 0;
  }
  if (n < (ssize_t)(sizeof(struct nlmsghdr) + sizeof(struct nlmsgerr))) {
    errno = EINVAL;
    if (dofail)
      exit(1);
    return -1;
  }
  if (hdr->nlmsg_type != NLMSG_ERROR) {
    errno = EINVAL;
    if (dofail)
      exit(1);
    return -1;
  }
  errno = -((struct nlmsgerr*)(hdr + 1))->error;
  return -errno;
}

static int netlink_send(struct nlmsg* nlmsg, int sock)
{
  return netlink_send_ext(nlmsg, sock, 0, NULL, true);
}

static int netlink_query_family_id(struct nlmsg* nlmsg, int sock,
                                   const char* family_name, bool dofail)
{
  struct genlmsghdr genlhdr;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = CTRL_CMD_GETFAMILY;
  netlink_init(nlmsg, GENL_ID_CTRL, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(nlmsg, CTRL_ATTR_FAMILY_NAME, family_name,
               strnlen(family_name, GENL_NAMSIZ - 1) + 1);
  int n = 0;
  int err = netlink_send_ext(nlmsg, sock, GENL_ID_CTRL, &n, dofail);
  if (err < 0) {
    return -1;
  }
  uint16_t id = 0;
  struct nlattr* attr = (struct nlattr*)(nlmsg->buf + NLMSG_HDRLEN +
                                         NLMSG_ALIGN(sizeof(genlhdr)));
  for (; (char*)attr < nlmsg->buf + n;
       attr = (struct nlattr*)((char*)attr + NLMSG_ALIGN(attr->nla_len))) {
    if (attr->nla_type == CTRL_ATTR_FAMILY_ID) {
      id = *(uint16_t*)(attr + 1);
      break;
    }
  }
  if (!id) {
    errno = EINVAL;
    return -1;
  }
  recv(sock, nlmsg->buf, sizeof(nlmsg->buf), 0);
  return id;
}

static int netlink_next_msg(struct nlmsg* nlmsg, unsigned int offset,
                            unsigned int total_len)
{
  struct nlmsghdr* hdr = (struct nlmsghdr*)(nlmsg->buf + offset);
  if (offset == total_len || offset + hdr->nlmsg_len > total_len)
    return -1;
  return hdr->nlmsg_len;
}

static void netlink_add_device_impl(struct nlmsg* nlmsg, const char* type,
                                    const char* name)
{
  struct ifinfomsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  netlink_init(nlmsg, RTM_NEWLINK, NLM_F_EXCL | NLM_F_CREATE, &hdr,
               sizeof(hdr));
  if (name)
    netlink_attr(nlmsg, IFLA_IFNAME, name, strlen(name));
  netlink_nest(nlmsg, IFLA_LINKINFO);
  netlink_attr(nlmsg, IFLA_INFO_KIND, type, strlen(type));
}

static void netlink_add_device(struct nlmsg* nlmsg, int sock, const char* type,
                               const char* name)
{
  netlink_add_device_impl(nlmsg, type, name);
  netlink_done(nlmsg);
  int err = netlink_send(nlmsg, sock);
  if (err < 0) {
  }
}

static void netlink_add_veth(struct nlmsg* nlmsg, int sock, const char* name,
                             const char* peer)
{
  netlink_add_device_impl(nlmsg, "veth", name);
  netlink_nest(nlmsg, IFLA_INFO_DATA);
  netlink_nest(nlmsg, VETH_INFO_PEER);
  nlmsg->pos += sizeof(struct ifinfomsg);
  netlink_attr(nlmsg, IFLA_IFNAME, peer, strlen(peer));
  netlink_done(nlmsg);
  netlink_done(nlmsg);
  netlink_done(nlmsg);
  int err = netlink_send(nlmsg, sock);
  if (err < 0) {
  }
}

static void netlink_add_hsr(struct nlmsg* nlmsg, int sock, const char* name,
                            const char* slave1, const char* slave2)
{
  netlink_add_device_impl(nlmsg, "hsr", name);
  netlink_nest(nlmsg, IFLA_INFO_DATA);
  int ifindex1 = if_nametoindex(slave1);
  netlink_attr(nlmsg, IFLA_HSR_SLAVE1, &ifindex1, sizeof(ifindex1));
  int ifindex2 = if_nametoindex(slave2);
  netlink_attr(nlmsg, IFLA_HSR_SLAVE2, &ifindex2, sizeof(ifindex2));
  netlink_done(nlmsg);
  netlink_done(nlmsg);
  int err = netlink_send(nlmsg, sock);
  if (err < 0) {
  }
}

static void netlink_add_linked(struct nlmsg* nlmsg, int sock, const char* type,
                               const char* name, const char* link)
{
  netlink_add_device_impl(nlmsg, type, name);
  netlink_done(nlmsg);
  int ifindex = if_nametoindex(link);
  netlink_attr(nlmsg, IFLA_LINK, &ifindex, sizeof(ifindex));
  int err = netlink_send(nlmsg, sock);
  if (err < 0) {
  }
}

static void netlink_add_vlan(struct nlmsg* nlmsg, int sock, const char* name,
                             const char* link, uint16_t id, uint16_t proto)
{
  netlink_add_device_impl(nlmsg, "vlan", name);
  netlink_nest(nlmsg, IFLA_INFO_DATA);
  netlink_attr(nlmsg, IFLA_VLAN_ID, &id, sizeof(id));
  netlink_attr(nlmsg, IFLA_VLAN_PROTOCOL, &proto, sizeof(proto));
  netlink_done(nlmsg);
  netlink_done(nlmsg);
  int ifindex = if_nametoindex(link);
  netlink_attr(nlmsg, IFLA_LINK, &ifindex, sizeof(ifindex));
  int err = netlink_send(nlmsg, sock);
  if (err < 0) {
  }
}

static void netlink_add_macvlan(struct nlmsg* nlmsg, int sock, const char* name,
                                const char* link)
{
  netlink_add_device_impl(nlmsg, "macvlan", name);
  netlink_nest(nlmsg, IFLA_INFO_DATA);
  uint32_t mode = MACVLAN_MODE_BRIDGE;
  netlink_attr(nlmsg, IFLA_MACVLAN_MODE, &mode, sizeof(mode));
  netlink_done(nlmsg);
  netlink_done(nlmsg);
  int ifindex = if_nametoindex(link);
  netlink_attr(nlmsg, IFLA_LINK, &ifindex, sizeof(ifindex));
  int err = netlink_send(nlmsg, sock);
  if (err < 0) {
  }
}

static void netlink_add_geneve(struct nlmsg* nlmsg, int sock, const char* name,
                               uint32_t vni, struct in_addr* addr4,
                               struct in6_addr* addr6)
{
  netlink_add_device_impl(nlmsg, "geneve", name);
  netlink_nest(nlmsg, IFLA_INFO_DATA);
  netlink_attr(nlmsg, IFLA_GENEVE_ID, &vni, sizeof(vni));
  if (addr4)
    netlink_attr(nlmsg, IFLA_GENEVE_REMOTE, addr4, sizeof(*addr4));
  if (addr6)
    netlink_attr(nlmsg, IFLA_GENEVE_REMOTE6, addr6, sizeof(*addr6));
  netlink_done(nlmsg);
  netlink_done(nlmsg);
  int err = netlink_send(nlmsg, sock);
  if (err < 0) {
  }
}

#define IFLA_IPVLAN_FLAGS 2
#define IPVLAN_MODE_L3S 2
#undef IPVLAN_F_VEPA
#define IPVLAN_F_VEPA 2

static void netlink_add_ipvlan(struct nlmsg* nlmsg, int sock, const char* name,
                               const char* link, uint16_t mode, uint16_t flags)
{
  netlink_add_device_impl(nlmsg, "ipvlan", name);
  netlink_nest(nlmsg, IFLA_INFO_DATA);
  netlink_attr(nlmsg, IFLA_IPVLAN_MODE, &mode, sizeof(mode));
  netlink_attr(nlmsg, IFLA_IPVLAN_FLAGS, &flags, sizeof(flags));
  netlink_done(nlmsg);
  netlink_done(nlmsg);
  int ifindex = if_nametoindex(link);
  netlink_attr(nlmsg, IFLA_LINK, &ifindex, sizeof(ifindex));
  int err = netlink_send(nlmsg, sock);
  if (err < 0) {
  }
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
  if (err < 0) {
  }
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
  if (err < 0) {
  }
}

static void netlink_add_addr6(struct nlmsg* nlmsg, int sock, const char* dev,
                              const char* addr)
{
  struct in6_addr in6_addr;
  inet_pton(AF_INET6, addr, &in6_addr);
  int err = netlink_add_addr(nlmsg, sock, dev, &in6_addr, sizeof(in6_addr));
  if (err < 0) {
  }
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
  if (err < 0) {
  }
}

static struct nlmsg nlmsg;

static int tunfd = -1;

#define TUN_IFACE "syz_tun"
#define LOCAL_MAC 0xaaaaaaaaaaaa
#define REMOTE_MAC 0xaaaaaaaaaabb
#define LOCAL_IPV4 "172.20.20.170"
#define REMOTE_IPV4 "172.20.20.187"
#define LOCAL_IPV6 "fe80::aa"
#define REMOTE_IPV6 "fe80::bb"

#define IFF_NAPI 0x0010

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
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
  if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0) {
    exit(1);
  }
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

#define DEVLINK_FAMILY_NAME "devlink"

#define DEVLINK_CMD_PORT_GET 5
#define DEVLINK_ATTR_BUS_NAME 1
#define DEVLINK_ATTR_DEV_NAME 2
#define DEVLINK_ATTR_NETDEV_NAME 7

static struct nlmsg nlmsg2;

static void initialize_devlink_ports(const char* bus_name, const char* dev_name,
                                     const char* netdev_prefix)
{
  struct genlmsghdr genlhdr;
  int len, total_len, id, err, offset;
  uint16_t netdev_index;
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (sock == -1)
    exit(1);
  int rtsock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (rtsock == -1)
    exit(1);
  id = netlink_query_family_id(&nlmsg, sock, DEVLINK_FAMILY_NAME, true);
  if (id == -1)
    goto error;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = DEVLINK_CMD_PORT_GET;
  netlink_init(&nlmsg, id, NLM_F_DUMP, &genlhdr, sizeof(genlhdr));
  netlink_attr(&nlmsg, DEVLINK_ATTR_BUS_NAME, bus_name, strlen(bus_name) + 1);
  netlink_attr(&nlmsg, DEVLINK_ATTR_DEV_NAME, dev_name, strlen(dev_name) + 1);
  err = netlink_send_ext(&nlmsg, sock, id, &total_len, true);
  if (err < 0) {
    goto error;
  }
  offset = 0;
  netdev_index = 0;
  while ((len = netlink_next_msg(&nlmsg, offset, total_len)) != -1) {
    struct nlattr* attr = (struct nlattr*)(nlmsg.buf + offset + NLMSG_HDRLEN +
                                           NLMSG_ALIGN(sizeof(genlhdr)));
    for (; (char*)attr < nlmsg.buf + offset + len;
         attr = (struct nlattr*)((char*)attr + NLMSG_ALIGN(attr->nla_len))) {
      if (attr->nla_type == DEVLINK_ATTR_NETDEV_NAME) {
        char* port_name;
        char netdev_name[IFNAMSIZ];
        port_name = (char*)(attr + 1);
        snprintf(netdev_name, sizeof(netdev_name), "%s%d", netdev_prefix,
                 netdev_index);
        netlink_device_change(&nlmsg2, rtsock, port_name, true, 0, 0, 0,
                              netdev_name);
        break;
      }
    }
    offset += len;
    netdev_index++;
  }
error:
  close(rtsock);
  close(sock);
}

#define DEV_IPV4 "172.20.20.%d"
#define DEV_IPV6 "fe80::%02x"
#define DEV_MAC 0x00aaaaaaaaaa

static void netdevsim_add(unsigned int addr, unsigned int port_count)
{
  char buf[16];
  sprintf(buf, "%u %u", addr, port_count);
  if (write_file("/sys/bus/netdevsim/new_device", buf)) {
    snprintf(buf, sizeof(buf), "netdevsim%d", addr);
    initialize_devlink_ports("netdevsim", buf, "netdevsim");
  }
}

#define WG_GENL_NAME "wireguard"
enum wg_cmd {
  WG_CMD_GET_DEVICE,
  WG_CMD_SET_DEVICE,
};
enum wgdevice_attribute {
  WGDEVICE_A_UNSPEC,
  WGDEVICE_A_IFINDEX,
  WGDEVICE_A_IFNAME,
  WGDEVICE_A_PRIVATE_KEY,
  WGDEVICE_A_PUBLIC_KEY,
  WGDEVICE_A_FLAGS,
  WGDEVICE_A_LISTEN_PORT,
  WGDEVICE_A_FWMARK,
  WGDEVICE_A_PEERS,
};
enum wgpeer_attribute {
  WGPEER_A_UNSPEC,
  WGPEER_A_PUBLIC_KEY,
  WGPEER_A_PRESHARED_KEY,
  WGPEER_A_FLAGS,
  WGPEER_A_ENDPOINT,
  WGPEER_A_PERSISTENT_KEEPALIVE_INTERVAL,
  WGPEER_A_LAST_HANDSHAKE_TIME,
  WGPEER_A_RX_BYTES,
  WGPEER_A_TX_BYTES,
  WGPEER_A_ALLOWEDIPS,
  WGPEER_A_PROTOCOL_VERSION,
};
enum wgallowedip_attribute {
  WGALLOWEDIP_A_UNSPEC,
  WGALLOWEDIP_A_FAMILY,
  WGALLOWEDIP_A_IPADDR,
  WGALLOWEDIP_A_CIDR_MASK,
};

static void netlink_wireguard_setup(void)
{
  const char ifname_a[] = "wg0";
  const char ifname_b[] = "wg1";
  const char ifname_c[] = "wg2";
  const char private_a[] =
      "\xa0\x5c\xa8\x4f\x6c\x9c\x8e\x38\x53\xe2\xfd\x7a\x70\xae\x0f\xb2\x0f\xa1"
      "\x52\x60\x0c\xb0\x08\x45\x17\x4f\x08\x07\x6f\x8d\x78\x43";
  const char private_b[] =
      "\xb0\x80\x73\xe8\xd4\x4e\x91\xe3\xda\x92\x2c\x22\x43\x82\x44\xbb\x88\x5c"
      "\x69\xe2\x69\xc8\xe9\xd8\x35\xb1\x14\x29\x3a\x4d\xdc\x6e";
  const char private_c[] =
      "\xa0\xcb\x87\x9a\x47\xf5\xbc\x64\x4c\x0e\x69\x3f\xa6\xd0\x31\xc7\x4a\x15"
      "\x53\xb6\xe9\x01\xb9\xff\x2f\x51\x8c\x78\x04\x2f\xb5\x42";
  const char public_a[] =
      "\x97\x5c\x9d\x81\xc9\x83\xc8\x20\x9e\xe7\x81\x25\x4b\x89\x9f\x8e\xd9\x25"
      "\xae\x9f\x09\x23\xc2\x3c\x62\xf5\x3c\x57\xcd\xbf\x69\x1c";
  const char public_b[] =
      "\xd1\x73\x28\x99\xf6\x11\xcd\x89\x94\x03\x4d\x7f\x41\x3d\xc9\x57\x63\x0e"
      "\x54\x93\xc2\x85\xac\xa4\x00\x65\xcb\x63\x11\xbe\x69\x6b";
  const char public_c[] =
      "\xf4\x4d\xa3\x67\xa8\x8e\xe6\x56\x4f\x02\x02\x11\x45\x67\x27\x08\x2f\x5c"
      "\xeb\xee\x8b\x1b\xf5\xeb\x73\x37\x34\x1b\x45\x9b\x39\x22";
  const uint16_t listen_a = 20001;
  const uint16_t listen_b = 20002;
  const uint16_t listen_c = 20003;
  const uint16_t af_inet = AF_INET;
  const uint16_t af_inet6 = AF_INET6;
  const struct sockaddr_in endpoint_b_v4 = {
      .sin_family = AF_INET,
      .sin_port = htons(listen_b),
      .sin_addr = {htonl(INADDR_LOOPBACK)}};
  const struct sockaddr_in endpoint_c_v4 = {
      .sin_family = AF_INET,
      .sin_port = htons(listen_c),
      .sin_addr = {htonl(INADDR_LOOPBACK)}};
  struct sockaddr_in6 endpoint_a_v6 = {.sin6_family = AF_INET6,
                                       .sin6_port = htons(listen_a)};
  endpoint_a_v6.sin6_addr = in6addr_loopback;
  struct sockaddr_in6 endpoint_c_v6 = {.sin6_family = AF_INET6,
                                       .sin6_port = htons(listen_c)};
  endpoint_c_v6.sin6_addr = in6addr_loopback;
  const struct in_addr first_half_v4 = {0};
  const struct in_addr second_half_v4 = {(uint32_t)htonl(128 << 24)};
  const struct in6_addr first_half_v6 = {{{0}}};
  const struct in6_addr second_half_v6 = {{{0x80}}};
  const uint8_t half_cidr = 1;
  const uint16_t persistent_keepalives[] = {1, 3, 7, 9, 14, 19};
  struct genlmsghdr genlhdr = {.cmd = WG_CMD_SET_DEVICE, .version = 1};
  int sock;
  int id, err;
  sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (sock == -1) {
    return;
  }
  id = netlink_query_family_id(&nlmsg, sock, WG_GENL_NAME, true);
  if (id == -1)
    goto error;
  netlink_init(&nlmsg, id, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(&nlmsg, WGDEVICE_A_IFNAME, ifname_a, strlen(ifname_a) + 1);
  netlink_attr(&nlmsg, WGDEVICE_A_PRIVATE_KEY, private_a, 32);
  netlink_attr(&nlmsg, WGDEVICE_A_LISTEN_PORT, &listen_a, 2);
  netlink_nest(&nlmsg, NLA_F_NESTED | WGDEVICE_A_PEERS);
  netlink_nest(&nlmsg, NLA_F_NESTED | 0);
  netlink_attr(&nlmsg, WGPEER_A_PUBLIC_KEY, public_b, 32);
  netlink_attr(&nlmsg, WGPEER_A_ENDPOINT, &endpoint_b_v4,
               sizeof(endpoint_b_v4));
  netlink_attr(&nlmsg, WGPEER_A_PERSISTENT_KEEPALIVE_INTERVAL,
               &persistent_keepalives[0], 2);
  netlink_nest(&nlmsg, NLA_F_NESTED | WGPEER_A_ALLOWEDIPS);
  netlink_nest(&nlmsg, NLA_F_NESTED | 0);
  netlink_attr(&nlmsg, WGALLOWEDIP_A_FAMILY, &af_inet, 2);
  netlink_attr(&nlmsg, WGALLOWEDIP_A_IPADDR, &first_half_v4,
               sizeof(first_half_v4));
  netlink_attr(&nlmsg, WGALLOWEDIP_A_CIDR_MASK, &half_cidr, 1);
  netlink_done(&nlmsg);
  netlink_nest(&nlmsg, NLA_F_NESTED | 0);
  netlink_attr(&nlmsg, WGALLOWEDIP_A_FAMILY, &af_inet6, 2);
  netlink_attr(&nlmsg, WGALLOWEDIP_A_IPADDR, &first_half_v6,
               sizeof(first_half_v6));
  netlink_attr(&nlmsg, WGALLOWEDIP_A_CIDR_MASK, &half_cidr, 1);
  netlink_done(&nlmsg);
  netlink_done(&nlmsg);
  netlink_done(&nlmsg);
  netlink_nest(&nlmsg, NLA_F_NESTED | 0);
  netlink_attr(&nlmsg, WGPEER_A_PUBLIC_KEY, public_c, 32);
  netlink_attr(&nlmsg, WGPEER_A_ENDPOINT, &endpoint_c_v6,
               sizeof(endpoint_c_v6));
  netlink_attr(&nlmsg, WGPEER_A_PERSISTENT_KEEPALIVE_INTERVAL,
               &persistent_keepalives[1], 2);
  netlink_nest(&nlmsg, NLA_F_NESTED | WGPEER_A_ALLOWEDIPS);
  netlink_nest(&nlmsg, NLA_F_NESTED | 0);
  netlink_attr(&nlmsg, WGALLOWEDIP_A_FAMILY, &af_inet, 2);
  netlink_attr(&nlmsg, WGALLOWEDIP_A_IPADDR, &second_half_v4,
               sizeof(second_half_v4));
  netlink_attr(&nlmsg, WGALLOWEDIP_A_CIDR_MASK, &half_cidr, 1);
  netlink_done(&nlmsg);
  netlink_nest(&nlmsg, NLA_F_NESTED | 0);
  netlink_attr(&nlmsg, WGALLOWEDIP_A_FAMILY, &af_inet6, 2);
  netlink_attr(&nlmsg, WGALLOWEDIP_A_IPADDR, &second_half_v6,
               sizeof(second_half_v6));
  netlink_attr(&nlmsg, WGALLOWEDIP_A_CIDR_MASK, &half_cidr, 1);
  netlink_done(&nlmsg);
  netlink_done(&nlmsg);
  netlink_done(&nlmsg);
  netlink_done(&nlmsg);
  err = netlink_send(&nlmsg, sock);
  if (err < 0) {
  }
  netlink_init(&nlmsg, id, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(&nlmsg, WGDEVICE_A_IFNAME, ifname_b, strlen(ifname_b) + 1);
  netlink_attr(&nlmsg, WGDEVICE_A_PRIVATE_KEY, private_b, 32);
  netlink_attr(&nlmsg, WGDEVICE_A_LISTEN_PORT, &listen_b, 2);
  netlink_nest(&nlmsg, NLA_F_NESTED | WGDEVICE_A_PEERS);
  netlink_nest(&nlmsg, NLA_F_NESTED | 0);
  netlink_attr(&nlmsg, WGPEER_A_PUBLIC_KEY, public_a, 32);
  netlink_attr(&nlmsg, WGPEER_A_ENDPOINT, &endpoint_a_v6,
               sizeof(endpoint_a_v6));
  netlink_attr(&nlmsg, WGPEER_A_PERSISTENT_KEEPALIVE_INTERVAL,
               &persistent_keepalives[2], 2);
  netlink_nest(&nlmsg, NLA_F_NESTED | WGPEER_A_ALLOWEDIPS);
  netlink_nest(&nlmsg, NLA_F_NESTED | 0);
  netlink_attr(&nlmsg, WGALLOWEDIP_A_FAMILY, &af_inet, 2);
  netlink_attr(&nlmsg, WGALLOWEDIP_A_IPADDR, &first_half_v4,
               sizeof(first_half_v4));
  netlink_attr(&nlmsg, WGALLOWEDIP_A_CIDR_MASK, &half_cidr, 1);
  netlink_done(&nlmsg);
  netlink_nest(&nlmsg, NLA_F_NESTED | 0);
  netlink_attr(&nlmsg, WGALLOWEDIP_A_FAMILY, &af_inet6, 2);
  netlink_attr(&nlmsg, WGALLOWEDIP_A_IPADDR, &first_half_v6,
               sizeof(first_half_v6));
  netlink_attr(&nlmsg, WGALLOWEDIP_A_CIDR_MASK, &half_cidr, 1);
  netlink_done(&nlmsg);
  netlink_done(&nlmsg);
  netlink_done(&nlmsg);
  netlink_nest(&nlmsg, NLA_F_NESTED | 0);
  netlink_attr(&nlmsg, WGPEER_A_PUBLIC_KEY, public_c, 32);
  netlink_attr(&nlmsg, WGPEER_A_ENDPOINT, &endpoint_c_v4,
               sizeof(endpoint_c_v4));
  netlink_attr(&nlmsg, WGPEER_A_PERSISTENT_KEEPALIVE_INTERVAL,
               &persistent_keepalives[3], 2);
  netlink_nest(&nlmsg, NLA_F_NESTED | WGPEER_A_ALLOWEDIPS);
  netlink_nest(&nlmsg, NLA_F_NESTED | 0);
  netlink_attr(&nlmsg, WGALLOWEDIP_A_FAMILY, &af_inet, 2);
  netlink_attr(&nlmsg, WGALLOWEDIP_A_IPADDR, &second_half_v4,
               sizeof(second_half_v4));
  netlink_attr(&nlmsg, WGALLOWEDIP_A_CIDR_MASK, &half_cidr, 1);
  netlink_done(&nlmsg);
  netlink_nest(&nlmsg, NLA_F_NESTED | 0);
  netlink_attr(&nlmsg, WGALLOWEDIP_A_FAMILY, &af_inet6, 2);
  netlink_attr(&nlmsg, WGALLOWEDIP_A_IPADDR, &second_half_v6,
               sizeof(second_half_v6));
  netlink_attr(&nlmsg, WGALLOWEDIP_A_CIDR_MASK, &half_cidr, 1);
  netlink_done(&nlmsg);
  netlink_done(&nlmsg);
  netlink_done(&nlmsg);
  netlink_done(&nlmsg);
  err = netlink_send(&nlmsg, sock);
  if (err < 0) {
  }
  netlink_init(&nlmsg, id, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(&nlmsg, WGDEVICE_A_IFNAME, ifname_c, strlen(ifname_c) + 1);
  netlink_attr(&nlmsg, WGDEVICE_A_PRIVATE_KEY, private_c, 32);
  netlink_attr(&nlmsg, WGDEVICE_A_LISTEN_PORT, &listen_c, 2);
  netlink_nest(&nlmsg, NLA_F_NESTED | WGDEVICE_A_PEERS);
  netlink_nest(&nlmsg, NLA_F_NESTED | 0);
  netlink_attr(&nlmsg, WGPEER_A_PUBLIC_KEY, public_a, 32);
  netlink_attr(&nlmsg, WGPEER_A_ENDPOINT, &endpoint_a_v6,
               sizeof(endpoint_a_v6));
  netlink_attr(&nlmsg, WGPEER_A_PERSISTENT_KEEPALIVE_INTERVAL,
               &persistent_keepalives[4], 2);
  netlink_nest(&nlmsg, NLA_F_NESTED | WGPEER_A_ALLOWEDIPS);
  netlink_nest(&nlmsg, NLA_F_NESTED | 0);
  netlink_attr(&nlmsg, WGALLOWEDIP_A_FAMILY, &af_inet, 2);
  netlink_attr(&nlmsg, WGALLOWEDIP_A_IPADDR, &first_half_v4,
               sizeof(first_half_v4));
  netlink_attr(&nlmsg, WGALLOWEDIP_A_CIDR_MASK, &half_cidr, 1);
  netlink_done(&nlmsg);
  netlink_nest(&nlmsg, NLA_F_NESTED | 0);
  netlink_attr(&nlmsg, WGALLOWEDIP_A_FAMILY, &af_inet6, 2);
  netlink_attr(&nlmsg, WGALLOWEDIP_A_IPADDR, &first_half_v6,
               sizeof(first_half_v6));
  netlink_attr(&nlmsg, WGALLOWEDIP_A_CIDR_MASK, &half_cidr, 1);
  netlink_done(&nlmsg);
  netlink_done(&nlmsg);
  netlink_done(&nlmsg);
  netlink_nest(&nlmsg, NLA_F_NESTED | 0);
  netlink_attr(&nlmsg, WGPEER_A_PUBLIC_KEY, public_b, 32);
  netlink_attr(&nlmsg, WGPEER_A_ENDPOINT, &endpoint_b_v4,
               sizeof(endpoint_b_v4));
  netlink_attr(&nlmsg, WGPEER_A_PERSISTENT_KEEPALIVE_INTERVAL,
               &persistent_keepalives[5], 2);
  netlink_nest(&nlmsg, NLA_F_NESTED | WGPEER_A_ALLOWEDIPS);
  netlink_nest(&nlmsg, NLA_F_NESTED | 0);
  netlink_attr(&nlmsg, WGALLOWEDIP_A_FAMILY, &af_inet, 2);
  netlink_attr(&nlmsg, WGALLOWEDIP_A_IPADDR, &second_half_v4,
               sizeof(second_half_v4));
  netlink_attr(&nlmsg, WGALLOWEDIP_A_CIDR_MASK, &half_cidr, 1);
  netlink_done(&nlmsg);
  netlink_nest(&nlmsg, NLA_F_NESTED | 0);
  netlink_attr(&nlmsg, WGALLOWEDIP_A_FAMILY, &af_inet6, 2);
  netlink_attr(&nlmsg, WGALLOWEDIP_A_IPADDR, &second_half_v6,
               sizeof(second_half_v6));
  netlink_attr(&nlmsg, WGALLOWEDIP_A_CIDR_MASK, &half_cidr, 1);
  netlink_done(&nlmsg);
  netlink_done(&nlmsg);
  netlink_done(&nlmsg);
  netlink_done(&nlmsg);
  err = netlink_send(&nlmsg, sock);
  if (err < 0) {
  }

error:
  close(sock);
}
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
      {"xfrm", "xfrm0"},           {"wireguard", "wg0"},
      {"wireguard", "wg1"},        {"wireguard", "wg2"},
  };
  const char* devmasters[] = {"bridge", "bond", "team", "batadv"};
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
      {"vxcan0", 0, true},
      {"vxcan1", 0, true},
      {"caif0", ETH_ALEN},
      {"batadv0", ETH_ALEN},
      {netdevsim, ETH_ALEN},
      {"xfrm0", ETH_ALEN},
      {"veth0_virt_wifi", ETH_ALEN},
      {"veth1_virt_wifi", ETH_ALEN},
      {"virt_wifi0", ETH_ALEN},
      {"veth0_vlan", ETH_ALEN},
      {"veth1_vlan", ETH_ALEN},
      {"vlan0", ETH_ALEN},
      {"vlan1", ETH_ALEN},
      {"macvlan0", ETH_ALEN},
      {"macvlan1", ETH_ALEN},
      {"ipvlan0", ETH_ALEN},
      {"ipvlan1", ETH_ALEN},
      {"veth0_macvtap", ETH_ALEN},
      {"veth1_macvtap", ETH_ALEN},
      {"macvtap0", ETH_ALEN},
      {"macsec0", ETH_ALEN},
      {"veth0_to_batadv", ETH_ALEN},
      {"veth1_to_batadv", ETH_ALEN},
      {"batadv_slave_0", ETH_ALEN},
      {"batadv_slave_1", ETH_ALEN},
      {"geneve0", ETH_ALEN},
      {"geneve1", ETH_ALEN},
      {"wg0", 0},
      {"wg1", 0},
      {"wg2", 0},
  };
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sock == -1)
    exit(1);
  unsigned i;
  for (i = 0; i < sizeof(devtypes) / sizeof(devtypes[0]); i++)
    netlink_add_device(&nlmsg, sock, devtypes[i].type, devtypes[i].dev);
  for (i = 0; i < sizeof(devmasters) / (sizeof(devmasters[0])); i++) {
    char master[32], slave0[32], veth0[32], slave1[32], veth1[32];
    sprintf(slave0, "%s_slave_0", devmasters[i]);
    sprintf(veth0, "veth0_to_%s", devmasters[i]);
    netlink_add_veth(&nlmsg, sock, slave0, veth0);
    sprintf(slave1, "%s_slave_1", devmasters[i]);
    sprintf(veth1, "veth1_to_%s", devmasters[i]);
    netlink_add_veth(&nlmsg, sock, slave1, veth1);
    sprintf(master, "%s0", devmasters[i]);
    netlink_device_change(&nlmsg, sock, slave0, false, master, 0, 0, NULL);
    netlink_device_change(&nlmsg, sock, slave1, false, master, 0, 0, NULL);
  }
  netlink_device_change(&nlmsg, sock, "bridge_slave_0", true, 0, 0, 0, NULL);
  netlink_device_change(&nlmsg, sock, "bridge_slave_1", true, 0, 0, 0, NULL);
  netlink_add_veth(&nlmsg, sock, "hsr_slave_0", "veth0_to_hsr");
  netlink_add_veth(&nlmsg, sock, "hsr_slave_1", "veth1_to_hsr");
  netlink_add_hsr(&nlmsg, sock, "hsr0", "hsr_slave_0", "hsr_slave_1");
  netlink_device_change(&nlmsg, sock, "hsr_slave_0", true, 0, 0, 0, NULL);
  netlink_device_change(&nlmsg, sock, "hsr_slave_1", true, 0, 0, 0, NULL);
  netlink_add_veth(&nlmsg, sock, "veth0_virt_wifi", "veth1_virt_wifi");
  netlink_add_linked(&nlmsg, sock, "virt_wifi", "virt_wifi0",
                     "veth1_virt_wifi");
  netlink_add_veth(&nlmsg, sock, "veth0_vlan", "veth1_vlan");
  netlink_add_vlan(&nlmsg, sock, "vlan0", "veth0_vlan", 0, htons(ETH_P_8021Q));
  netlink_add_vlan(&nlmsg, sock, "vlan1", "veth0_vlan", 1, htons(ETH_P_8021AD));
  netlink_add_macvlan(&nlmsg, sock, "macvlan0", "veth1_vlan");
  netlink_add_macvlan(&nlmsg, sock, "macvlan1", "veth1_vlan");
  netlink_add_ipvlan(&nlmsg, sock, "ipvlan0", "veth0_vlan", IPVLAN_MODE_L2, 0);
  netlink_add_ipvlan(&nlmsg, sock, "ipvlan1", "veth0_vlan", IPVLAN_MODE_L3S,
                     IPVLAN_F_VEPA);
  netlink_add_veth(&nlmsg, sock, "veth0_macvtap", "veth1_macvtap");
  netlink_add_linked(&nlmsg, sock, "macvtap", "macvtap0", "veth0_macvtap");
  netlink_add_linked(&nlmsg, sock, "macsec", "macsec0", "veth1_macvtap");
  char addr[32];
  sprintf(addr, DEV_IPV4, 14 + 10);
  struct in_addr geneve_addr4;
  if (inet_pton(AF_INET, addr, &geneve_addr4) <= 0)
    exit(1);
  struct in6_addr geneve_addr6;
  if (inet_pton(AF_INET6, "fc00::01", &geneve_addr6) <= 0)
    exit(1);
  netlink_add_geneve(&nlmsg, sock, "geneve0", 0, &geneve_addr4, 0);
  netlink_add_geneve(&nlmsg, sock, "geneve1", 1, 0, &geneve_addr6);
  netdevsim_add((int)procid, 4);
  netlink_wireguard_setup();
  for (i = 0; i < sizeof(devices) / (sizeof(devices[0])); i++) {
    char addr[32];
    sprintf(addr, DEV_IPV4, i + 10);
    netlink_add_addr4(&nlmsg, sock, devices[i].name, addr);
    if (!devices[i].noipv6) {
      sprintf(addr, DEV_IPV6, i + 10);
      netlink_add_addr6(&nlmsg, sock, devices[i].name, addr);
    }
    uint64_t macaddr = DEV_MAC + ((i + 10ull) << 40);
    netlink_device_change(&nlmsg, sock, devices[i].name, true, 0, &macaddr,
                          devices[i].macsize, NULL);
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
      {"nr", 7, true},
      {"rose", 5, true, true},
  };
  unsigned i;
  for (i = 0; i < sizeof(devtypes) / sizeof(devtypes[0]); i++) {
    char dev[32], addr[32];
    sprintf(dev, "%s%d", devtypes[i].type, (int)procid);
    sprintf(addr, "172.30.%d.%d", i, (int)procid + 1);
    netlink_add_addr4(&nlmsg, sock, dev, addr);
    if (!devtypes[i].noipv6) {
      sprintf(addr, "fe88::%02x:%02x", i, (int)procid + 1);
      netlink_add_addr6(&nlmsg, sock, dev, addr);
    }
    int macsize = devtypes[i].macsize;
    uint64_t macaddr = 0xbbbbbb +
                       ((unsigned long long)i << (8 * (macsize - 2))) +
                       (procid << (8 * (macsize - 1)));
    netlink_device_change(&nlmsg, sock, dev, !devtypes[i].noup, 0, &macaddr,
                          macsize, NULL);
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
  if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL)) {
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

static int wait_for_loop(int pid)
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

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0x0};

void loop(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_socket, 0x10ul, 0x803ul, 0);
  if (res != -1)
    r[1] = res;
  *(uint64_t*)0x20000440 = 0;
  *(uint32_t*)0x20000448 = 0;
  *(uint64_t*)0x20000450 = 0x20000180;
  *(uint64_t*)0x20000180 = 0;
  *(uint64_t*)0x20000188 = 0;
  *(uint64_t*)0x20000458 = 1;
  *(uint64_t*)0x20000460 = 0;
  *(uint64_t*)0x20000468 = 0;
  *(uint32_t*)0x20000470 = 0;
  syscall(__NR_sendmsg, r[1], 0x20000440ul, 0ul);
  *(uint32_t*)0x20000200 = 0x14;
  res = syscall(__NR_getsockname, r[1], 0x20000380ul, 0x20000200ul);
  if (res != -1)
    r[2] = *(uint32_t*)0x20000384;
  *(uint64_t*)0x20000040 = 0;
  *(uint32_t*)0x20000048 = 0;
  *(uint64_t*)0x20000050 = 0x20000100;
  *(uint64_t*)0x20000100 = 0x20000080;
  memcpy((void*)0x20000080,
         "\x2c\x00\x00\x00\x24\x00\x1d\x0f\x00\x00\x00\xec\xff\xbf\xff\xff\xff"
         "\xff\xff\x00",
         20);
  *(uint32_t*)0x20000094 = r[2];
  memcpy((void*)0x20000098,
         "\x00\x00\x00\x00\xf1\xff\xff\xff\x00\x00\x00\x00\x08\x00\x01\x00\x64"
         "\x72\x72",
         19);
  *(uint64_t*)0x20000108 = 0x2c;
  *(uint64_t*)0x20000058 = 1;
  *(uint64_t*)0x20000060 = 0;
  *(uint64_t*)0x20000068 = 0;
  *(uint32_t*)0x20000070 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000040ul, 0ul);
  *(uint64_t*)0x20000080 = 0;
  syscall(__NR_getsockopt, -1, 0x112, 7, 0ul, 0x20000080ul);
  *(uint64_t*)0x20000280 = 0;
  *(uint32_t*)0x20000288 = 0;
  *(uint64_t*)0x20000290 = 0x200000c0;
  *(uint64_t*)0x200000c0 = 0x20000340;
  *(uint32_t*)0x20000340 = 0x38;
  *(uint16_t*)0x20000344 = 0x28;
  *(uint16_t*)0x20000346 = 0xf31;
  *(uint32_t*)0x20000348 = 0;
  *(uint32_t*)0x2000034c = 0;
  *(uint8_t*)0x20000350 = 0;
  *(uint8_t*)0x20000351 = 0;
  *(uint16_t*)0x20000352 = 0;
  *(uint32_t*)0x20000354 = r[2];
  *(uint16_t*)0x20000358 = 0;
  *(uint16_t*)0x2000035a = 0;
  *(uint16_t*)0x2000035c = 0;
  *(uint16_t*)0x2000035e = -1;
  *(uint16_t*)0x20000360 = 0;
  *(uint16_t*)0x20000362 = 0;
  *(uint16_t*)0x20000364 = 8;
  *(uint16_t*)0x20000366 = 1;
  memcpy((void*)0x20000368, "u32\000", 4);
  *(uint16_t*)0x2000036c = 4;
  *(uint16_t*)0x2000036e = 2;
  *(uint16_t*)0x20000370 = 6;
  *(uint16_t*)0x20000372 = 5;
  *(uint8_t*)0x20000374 = 0;
  *(uint8_t*)0x20000375 = 0x10;
  *(uint64_t*)0x200000c8 = 0x38;
  *(uint64_t*)0x20000298 = 0xf;
  *(uint64_t*)0x200002a0 = 0;
  *(uint64_t*)0x200002a8 = 0;
  *(uint32_t*)0x200002b0 = 0;
  syscall(__NR_sendmsg, r[1], 0x20000280ul, 0ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  do_sandbox_none();
  return 0;
}
