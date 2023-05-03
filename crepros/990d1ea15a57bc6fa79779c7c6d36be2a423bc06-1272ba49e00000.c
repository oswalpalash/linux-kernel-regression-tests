// https://syzkaller.appspot.com/bug?id=990d1ea15a57bc6fa79779c7c6d36be2a423bc06
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <endian.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/genetlink.h>
#include <linux/if_addr.h>
#include <linux/if_link.h>
#include <linux/in6.h>
#include <linux/neighbour.h>
#include <linux/net.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/veth.h>

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

static int netlink_next_msg(struct nlmsg* nlmsg, unsigned int offset,
                            unsigned int total_len)
{
  struct nlmsghdr* hdr = (struct nlmsghdr*)(nlmsg->buf + offset);
  if (offset == total_len || offset + hdr->nlmsg_len > total_len)
    return -1;
  return hdr->nlmsg_len;
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

const int kInitNetNsFd = 239;

#define DEVLINK_FAMILY_NAME "devlink"

#define DEVLINK_CMD_PORT_GET 5
#define DEVLINK_CMD_RELOAD 37
#define DEVLINK_ATTR_BUS_NAME 1
#define DEVLINK_ATTR_DEV_NAME 2
#define DEVLINK_ATTR_NETDEV_NAME 7
#define DEVLINK_ATTR_NETNS_FD 138

static int netlink_devlink_id_get(struct nlmsg* nlmsg, int sock)
{
  struct genlmsghdr genlhdr;
  struct nlattr* attr;
  int err, n;
  uint16_t id = 0;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = CTRL_CMD_GETFAMILY;
  netlink_init(nlmsg, GENL_ID_CTRL, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(nlmsg, CTRL_ATTR_FAMILY_NAME, DEVLINK_FAMILY_NAME,
               strlen(DEVLINK_FAMILY_NAME) + 1);
  err = netlink_send_ext(nlmsg, sock, GENL_ID_CTRL, &n);
  if (err) {
    return -1;
  }
  attr = (struct nlattr*)(nlmsg->buf + NLMSG_HDRLEN +
                          NLMSG_ALIGN(sizeof(genlhdr)));
  for (; (char*)attr < nlmsg->buf + n;
       attr = (struct nlattr*)((char*)attr + NLMSG_ALIGN(attr->nla_len))) {
    if (attr->nla_type == CTRL_ATTR_FAMILY_ID) {
      id = *(uint16_t*)(attr + 1);
      break;
    }
  }
  if (!id) {
    return -1;
  }
  recv(sock, nlmsg->buf, sizeof(nlmsg->buf), 0); /* recv ack */
  return id;
}

static void netlink_devlink_netns_move(const char* bus_name,
                                       const char* dev_name, int netns_fd)
{
  struct genlmsghdr genlhdr;
  int sock;
  int id, err;
  sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (sock == -1)
    exit(1);
  id = netlink_devlink_id_get(&nlmsg, sock);
  if (id == -1)
    goto error;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = DEVLINK_CMD_RELOAD;
  netlink_init(&nlmsg, id, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(&nlmsg, DEVLINK_ATTR_BUS_NAME, bus_name, strlen(bus_name) + 1);
  netlink_attr(&nlmsg, DEVLINK_ATTR_DEV_NAME, dev_name, strlen(dev_name) + 1);
  netlink_attr(&nlmsg, DEVLINK_ATTR_NETNS_FD, &netns_fd, sizeof(netns_fd));
  err = netlink_send(&nlmsg, sock);
  if (err) {
  }
error:
  close(sock);
}

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
  id = netlink_devlink_id_get(&nlmsg, sock);
  if (id == -1)
    goto error;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = DEVLINK_CMD_PORT_GET;
  netlink_init(&nlmsg, id, NLM_F_DUMP, &genlhdr, sizeof(genlhdr));
  netlink_attr(&nlmsg, DEVLINK_ATTR_BUS_NAME, bus_name, strlen(bus_name) + 1);
  netlink_attr(&nlmsg, DEVLINK_ATTR_DEV_NAME, dev_name, strlen(dev_name) + 1);
  err = netlink_send_ext(&nlmsg, sock, id, &total_len);
  if (err) {
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

static void initialize_devlink_pci(void)
{
  int netns = open("/proc/self/ns/net", O_RDONLY);
  if (netns == -1)
    exit(1);
  int ret = setns(kInitNetNsFd, 0);
  if (ret == -1)
    exit(1);
  netlink_devlink_netns_move("pci", "0000:00:10.0", netns);
  ret = setns(netns, 0);
  if (ret == -1)
    exit(1);
  close(netns);
  initialize_devlink_ports("pci", "0000:00:10.0", "netpci");
}

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  intptr_t res = 0;
  res = syscall(__NR_socket, 2ul, 2ul, 0ul);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000800, "filter\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000",
         32);
  *(uint32_t*)0x20000820 = 4;
  *(uint32_t*)0x20000824 = 4;
  *(uint32_t*)0x20000828 = 0x4e0;
  *(uint32_t*)0x2000082c = 0x2e8;
  *(uint32_t*)0x20000830 = 0x2e8;
  *(uint32_t*)0x20000834 = 0x2e8;
  *(uint32_t*)0x20000838 = 0x3f8;
  *(uint32_t*)0x2000083c = 0x3f8;
  *(uint32_t*)0x20000840 = 0x3f8;
  *(uint32_t*)0x20000844 = 4;
  *(uint64_t*)0x20000848 = 0;
  *(uint8_t*)0x20000850 = 0;
  *(uint8_t*)0x20000851 = 0;
  *(uint8_t*)0x20000852 = 0;
  *(uint8_t*)0x20000853 = 0;
  *(uint8_t*)0x20000854 = 0;
  *(uint8_t*)0x20000855 = 0;
  *(uint8_t*)0x20000856 = 0;
  *(uint8_t*)0x20000857 = 0;
  *(uint8_t*)0x20000858 = 0;
  *(uint8_t*)0x20000859 = 0;
  *(uint8_t*)0x2000085a = 0;
  *(uint8_t*)0x2000085b = 0;
  *(uint8_t*)0x2000085c = 0;
  *(uint8_t*)0x2000085d = 0;
  *(uint8_t*)0x2000085e = 0;
  *(uint8_t*)0x2000085f = 0;
  *(uint8_t*)0x20000860 = 0;
  *(uint8_t*)0x20000861 = 0;
  *(uint8_t*)0x20000862 = 0;
  *(uint8_t*)0x20000863 = 0;
  *(uint8_t*)0x20000864 = 0;
  *(uint8_t*)0x20000865 = 0;
  *(uint8_t*)0x20000866 = 0;
  *(uint8_t*)0x20000867 = 0;
  *(uint8_t*)0x20000868 = 0;
  *(uint8_t*)0x20000869 = 0;
  *(uint8_t*)0x2000086a = 0;
  *(uint8_t*)0x2000086b = 0;
  *(uint8_t*)0x2000086c = 0;
  *(uint8_t*)0x2000086d = 0;
  *(uint8_t*)0x2000086e = 0;
  *(uint8_t*)0x2000086f = 0;
  *(uint8_t*)0x20000870 = 0;
  *(uint8_t*)0x20000871 = 0;
  *(uint8_t*)0x20000872 = 0;
  *(uint8_t*)0x20000873 = 0;
  *(uint8_t*)0x20000874 = 0;
  *(uint8_t*)0x20000875 = 0;
  *(uint8_t*)0x20000876 = 0;
  *(uint8_t*)0x20000877 = 0;
  *(uint8_t*)0x20000878 = 0;
  *(uint8_t*)0x20000879 = 0;
  *(uint8_t*)0x2000087a = 0;
  *(uint8_t*)0x2000087b = 0;
  *(uint8_t*)0x2000087c = 0;
  *(uint8_t*)0x2000087d = 0;
  *(uint8_t*)0x2000087e = 0;
  *(uint8_t*)0x2000087f = 0;
  *(uint8_t*)0x20000880 = 0;
  *(uint8_t*)0x20000881 = 0;
  *(uint8_t*)0x20000882 = 0;
  *(uint8_t*)0x20000883 = 0;
  *(uint8_t*)0x20000884 = 0;
  *(uint8_t*)0x20000885 = 0;
  *(uint8_t*)0x20000886 = 0;
  *(uint8_t*)0x20000887 = 0;
  *(uint8_t*)0x20000888 = 0;
  *(uint8_t*)0x20000889 = 0;
  *(uint8_t*)0x2000088a = 0;
  *(uint8_t*)0x2000088b = 0;
  *(uint8_t*)0x2000088c = 0;
  *(uint8_t*)0x2000088d = 0;
  *(uint8_t*)0x2000088e = 0;
  *(uint8_t*)0x2000088f = 0;
  *(uint8_t*)0x20000890 = 0;
  *(uint8_t*)0x20000891 = 0;
  *(uint8_t*)0x20000892 = 0;
  *(uint8_t*)0x20000893 = 0;
  *(uint8_t*)0x20000894 = 0;
  *(uint8_t*)0x20000895 = 0;
  *(uint8_t*)0x20000896 = 0;
  *(uint8_t*)0x20000897 = 0;
  *(uint8_t*)0x20000898 = 0;
  *(uint8_t*)0x20000899 = 0;
  *(uint8_t*)0x2000089a = 0;
  *(uint8_t*)0x2000089b = 0;
  *(uint8_t*)0x2000089c = 0;
  *(uint8_t*)0x2000089d = 0;
  *(uint8_t*)0x2000089e = 0;
  *(uint8_t*)0x2000089f = 0;
  *(uint8_t*)0x200008a0 = 0;
  *(uint8_t*)0x200008a1 = 0;
  *(uint8_t*)0x200008a2 = 0;
  *(uint8_t*)0x200008a3 = 0;
  *(uint8_t*)0x200008a4 = 0;
  *(uint8_t*)0x200008a5 = 0;
  *(uint8_t*)0x200008a6 = 0;
  *(uint8_t*)0x200008a7 = 0;
  *(uint8_t*)0x200008a8 = 0;
  *(uint8_t*)0x200008a9 = 0;
  *(uint8_t*)0x200008aa = 0;
  *(uint8_t*)0x200008ab = 0;
  *(uint8_t*)0x200008ac = 0;
  *(uint8_t*)0x200008ad = 0;
  *(uint8_t*)0x200008ae = 0;
  *(uint8_t*)0x200008af = 0;
  *(uint8_t*)0x200008b0 = 0;
  *(uint8_t*)0x200008b1 = 0;
  *(uint8_t*)0x200008b2 = 0;
  *(uint8_t*)0x200008b3 = 0;
  *(uint8_t*)0x200008b4 = 0;
  *(uint8_t*)0x200008b5 = 0;
  *(uint8_t*)0x200008b6 = 0;
  *(uint8_t*)0x200008b7 = 0;
  *(uint8_t*)0x200008b8 = 0;
  *(uint8_t*)0x200008b9 = 0;
  *(uint8_t*)0x200008ba = 0;
  *(uint8_t*)0x200008bb = 0;
  *(uint8_t*)0x200008bc = 0;
  *(uint8_t*)0x200008bd = 0;
  *(uint8_t*)0x200008be = 0;
  *(uint8_t*)0x200008bf = 0;
  *(uint8_t*)0x200008c0 = 0;
  *(uint8_t*)0x200008c1 = 0;
  *(uint8_t*)0x200008c2 = 0;
  *(uint8_t*)0x200008c3 = 0;
  *(uint8_t*)0x200008c4 = 0;
  *(uint8_t*)0x200008c5 = 0;
  *(uint8_t*)0x200008c6 = 0;
  *(uint8_t*)0x200008c7 = 0;
  *(uint8_t*)0x200008c8 = 0;
  *(uint8_t*)0x200008c9 = 0;
  *(uint8_t*)0x200008ca = 0;
  *(uint8_t*)0x200008cb = 0;
  *(uint8_t*)0x200008cc = 0;
  *(uint8_t*)0x200008cd = 0;
  *(uint8_t*)0x200008ce = 0;
  *(uint8_t*)0x200008cf = 0;
  *(uint8_t*)0x200008d0 = 0;
  *(uint8_t*)0x200008d1 = 0;
  *(uint8_t*)0x200008d2 = 0;
  *(uint8_t*)0x200008d3 = 0;
  *(uint8_t*)0x200008d4 = 0;
  *(uint8_t*)0x200008d5 = 0;
  *(uint8_t*)0x200008d6 = 0;
  *(uint8_t*)0x200008d7 = 0;
  *(uint8_t*)0x200008d8 = 0;
  *(uint8_t*)0x200008d9 = 0;
  *(uint8_t*)0x200008da = 0;
  *(uint8_t*)0x200008db = 0;
  *(uint8_t*)0x200008dc = 0;
  *(uint8_t*)0x200008dd = 0;
  *(uint8_t*)0x200008de = 0;
  *(uint8_t*)0x200008df = 0;
  *(uint8_t*)0x200008e0 = 0;
  *(uint8_t*)0x200008e1 = 0;
  *(uint8_t*)0x200008e2 = 0;
  *(uint8_t*)0x200008e3 = 0;
  *(uint8_t*)0x200008e4 = 0;
  *(uint8_t*)0x200008e5 = 0;
  *(uint8_t*)0x200008e6 = 0;
  *(uint8_t*)0x200008e7 = 0;
  *(uint8_t*)0x200008e8 = 0;
  *(uint8_t*)0x200008e9 = 0;
  *(uint8_t*)0x200008ea = 0;
  *(uint8_t*)0x200008eb = 0;
  *(uint8_t*)0x200008ec = 0;
  *(uint8_t*)0x200008ed = 0;
  *(uint8_t*)0x200008ee = 0;
  *(uint8_t*)0x200008ef = 0;
  *(uint8_t*)0x200008f0 = 0;
  *(uint8_t*)0x200008f1 = 0;
  *(uint8_t*)0x200008f2 = 0;
  *(uint8_t*)0x200008f3 = 0;
  *(uint16_t*)0x200008f4 = 0xc0;
  *(uint16_t*)0x200008f6 = 0x100;
  *(uint32_t*)0x200008f8 = 0;
  *(uint64_t*)0x20000900 = 0;
  *(uint64_t*)0x20000908 = 0;
  *(uint16_t*)0x20000910 = 0x40;
  memcpy((void*)0x20000912, "RATEEST\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000",
         29);
  *(uint8_t*)0x2000092f = 0;
  memcpy((void*)0x20000930,
         "syz0\000\000\000\000\000\000\000\000\000\000\000\000", 16);
  *(uint8_t*)0x20000940 = 0xc7;
  *(uint8_t*)0x20000941 = 0x80;
  *(uint64_t*)0x20000948 = 8;
  *(uint8_t*)0x20000950 = 0xac;
  *(uint8_t*)0x20000951 = 0x14;
  *(uint8_t*)0x20000952 = 0x14;
  *(uint8_t*)0x20000953 = 0xbb;
  *(uint32_t*)0x20000954 = htobe32(0);
  *(uint32_t*)0x20000958 = htobe32(0);
  *(uint32_t*)0x2000095c = htobe32(0);
  *(uint8_t*)0x20000960 = 0;
  *(uint8_t*)0x20000961 = 0;
  *(uint8_t*)0x20000962 = 0;
  *(uint8_t*)0x20000963 = 0;
  *(uint8_t*)0x20000964 = 0;
  *(uint8_t*)0x20000965 = 0;
  *(uint8_t*)0x20000966 = 0;
  *(uint8_t*)0x20000967 = 0;
  *(uint8_t*)0x20000968 = 0;
  *(uint8_t*)0x20000969 = 0;
  *(uint8_t*)0x2000096a = 0;
  *(uint8_t*)0x2000096b = 0;
  *(uint8_t*)0x2000096c = 0;
  *(uint8_t*)0x2000096d = 0;
  *(uint8_t*)0x2000096e = 0;
  *(uint8_t*)0x2000096f = 0;
  *(uint8_t*)0x20000970 = 0;
  *(uint8_t*)0x20000971 = 0;
  *(uint8_t*)0x20000972 = 0;
  *(uint8_t*)0x20000973 = 0;
  *(uint8_t*)0x20000974 = 0;
  *(uint8_t*)0x20000975 = 0;
  *(uint8_t*)0x20000976 = 0;
  *(uint8_t*)0x20000977 = 0;
  *(uint8_t*)0x20000982 = 0;
  *(uint8_t*)0x20000983 = 0;
  *(uint8_t*)0x20000984 = 0;
  *(uint8_t*)0x20000985 = 0;
  *(uint8_t*)0x20000986 = 0;
  *(uint8_t*)0x20000987 = 0;
  *(uint8_t*)0x20000988 = 0;
  *(uint8_t*)0x20000989 = 0;
  *(uint8_t*)0x2000098a = 0;
  *(uint8_t*)0x2000098b = 0;
  *(uint8_t*)0x2000098c = 0;
  *(uint8_t*)0x2000098d = 0;
  *(uint8_t*)0x2000098e = 0;
  *(uint8_t*)0x2000098f = 0;
  *(uint8_t*)0x20000990 = 0;
  *(uint8_t*)0x20000991 = 0;
  *(uint8_t*)0x20000992 = 0x80;
  *(uint8_t*)0x20000993 = 0;
  *(uint8_t*)0x20000994 = 0;
  *(uint8_t*)0x20000995 = 0;
  *(uint8_t*)0x20000996 = 0;
  *(uint8_t*)0x20000997 = 0;
  *(uint16_t*)0x200009a2 = htobe16(0);
  *(uint16_t*)0x200009a4 = htobe16(0);
  *(uint16_t*)0x200009a6 = htobe16(0);
  *(uint16_t*)0x200009a8 = htobe16(0);
  *(uint16_t*)0x200009aa = htobe16(0);
  *(uint16_t*)0x200009ac = htobe16(0);
  memcpy((void*)0x200009ae,
         "yam0\000\000\000\000\000\000\000\000\000\000\000\000", 16);
  memcpy((void*)0x200009be, "ip6tnl0\000\000\000\000\000\000\000\000\000", 16);
  *(uint8_t*)0x200009ce = 0;
  *(uint8_t*)0x200009de = 0;
  *(uint8_t*)0x200009ee = 0;
  *(uint16_t*)0x200009f0 = 0;
  *(uint16_t*)0x200009f4 = 0xc0;
  *(uint16_t*)0x200009f6 = 0x1e8;
  *(uint32_t*)0x200009f8 = 0;
  *(uint64_t*)0x20000a00 = 0;
  *(uint64_t*)0x20000a08 = 0;
  *(uint16_t*)0x20000a10 = 0x128;
  memcpy((void*)0x20000a12, "SECMARK\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000",
         29);
  *(uint8_t*)0x20000a2f = 0;
  *(uint8_t*)0x20000a30 = 0;
  *(uint32_t*)0x20000a34 = 0;
  memcpy((void*)0x20000a38,
         "system_u:object_r:ptmx_t:"
         "s0\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000",
         256);
  *(uint32_t*)0x20000b38 = htobe32(0xe0000001);
  *(uint32_t*)0x20000b3c = htobe32(0x7f000001);
  *(uint32_t*)0x20000b40 = htobe32(0);
  *(uint32_t*)0x20000b44 = htobe32(0);
  *(uint8_t*)0x20000b48 = 0;
  *(uint8_t*)0x20000b49 = 0;
  *(uint8_t*)0x20000b4a = 0;
  *(uint8_t*)0x20000b4b = 0;
  *(uint8_t*)0x20000b4c = 0;
  *(uint8_t*)0x20000b4d = 0;
  *(uint8_t*)0x20000b4e = 0;
  *(uint8_t*)0x20000b4f = 0;
  *(uint8_t*)0x20000b50 = 0;
  *(uint8_t*)0x20000b51 = 0;
  *(uint8_t*)0x20000b52 = 0;
  *(uint8_t*)0x20000b53 = 0;
  *(uint8_t*)0x20000b54 = 0;
  *(uint8_t*)0x20000b55 = 0;
  *(uint8_t*)0x20000b56 = 0;
  *(uint8_t*)0x20000b57 = 0;
  *(uint8_t*)0x20000b58 = 0;
  *(uint8_t*)0x20000b59 = 0;
  *(uint8_t*)0x20000b5a = 0;
  *(uint8_t*)0x20000b5b = 0;
  *(uint8_t*)0x20000b5c = 0;
  *(uint8_t*)0x20000b5d = 0;
  *(uint8_t*)0x20000b5e = 0;
  *(uint8_t*)0x20000b5f = 0;
  *(uint8_t*)0x20000b6a = 0;
  *(uint8_t*)0x20000b6b = 0;
  *(uint8_t*)0x20000b6c = 0;
  *(uint8_t*)0x20000b6d = 0;
  *(uint8_t*)0x20000b6e = 0;
  *(uint8_t*)0x20000b6f = 0;
  *(uint8_t*)0x20000b70 = 0;
  *(uint8_t*)0x20000b71 = 0;
  *(uint8_t*)0x20000b72 = 0;
  *(uint8_t*)0x20000b73 = 0;
  *(uint8_t*)0x20000b74 = 0;
  *(uint8_t*)0x20000b75 = 0;
  *(uint8_t*)0x20000b76 = 0;
  *(uint8_t*)0x20000b77 = 0;
  *(uint8_t*)0x20000b78 = 0;
  *(uint8_t*)0x20000b79 = 0;
  *(uint8_t*)0x20000b7a = 0;
  *(uint8_t*)0x20000b7b = 0;
  *(uint8_t*)0x20000b7c = 0;
  *(uint8_t*)0x20000b7d = 0;
  *(uint8_t*)0x20000b7e = 0;
  *(uint8_t*)0x20000b7f = 0;
  *(uint16_t*)0x20000b8a = htobe16(0);
  *(uint16_t*)0x20000b8c = htobe16(0);
  *(uint16_t*)0x20000b8e = htobe16(0);
  *(uint16_t*)0x20000b90 = htobe16(0);
  *(uint16_t*)0x20000b92 = htobe16(0);
  *(uint16_t*)0x20000b94 = htobe16(0);
  memcpy((void*)0x20000b96, "bond_slave_0\000\000\000\000", 16);
  memcpy((void*)0x20000ba6,
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000",
         16);
  *(uint8_t*)0x20000bb6 = 0;
  *(uint8_t*)0x20000bc6 = 0;
  *(uint8_t*)0x20000bd6 = 0;
  *(uint16_t*)0x20000bd8 = 0;
  *(uint16_t*)0x20000bdc = 0xc0;
  *(uint16_t*)0x20000bde = 0x110;
  *(uint32_t*)0x20000be0 = 0;
  *(uint64_t*)0x20000be8 = 0;
  *(uint64_t*)0x20000bf0 = 0;
  *(uint16_t*)0x20000bf8 = 0x50;
  memcpy((void*)0x20000bfa, "mangle\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000",
         29);
  *(uint8_t*)0x20000c17 = 0;
  *(uint8_t*)0x20000c18 = 0;
  *(uint8_t*)0x20000c19 = 0;
  *(uint8_t*)0x20000c1a = 0;
  *(uint8_t*)0x20000c1b = 0;
  *(uint8_t*)0x20000c1c = 0;
  *(uint8_t*)0x20000c1d = 0;
  *(uint8_t*)0x20000c1e = 0;
  *(uint8_t*)0x20000c1f = 0;
  *(uint8_t*)0x20000c20 = 0;
  *(uint8_t*)0x20000c21 = 0;
  *(uint8_t*)0x20000c22 = 0;
  *(uint8_t*)0x20000c23 = 0;
  *(uint8_t*)0x20000c24 = 0;
  *(uint8_t*)0x20000c25 = 0;
  *(uint8_t*)0x20000c26 = 0;
  *(uint8_t*)0x20000c27 = 0;
  *(uint8_t*)0x20000c28 = 0;
  *(uint8_t*)0x20000c29 = 0;
  *(uint8_t*)0x20000c2a = 0;
  *(uint8_t*)0x20000c2b = 0;
  *(uint8_t*)0x20000c2c = 0;
  *(uint8_t*)0x20000c2d = 0;
  *(uint8_t*)0x20000c2e = 0;
  *(uint8_t*)0x20000c2f = 0;
  *(uint8_t*)0x20000c30 = 0;
  *(uint8_t*)0x20000c31 = 0;
  *(uint8_t*)0x20000c32 = 0;
  *(uint8_t*)0x20000c33 = 0;
  *(uint8_t*)0x20000c34 = 0;
  *(uint8_t*)0x20000c35 = 0;
  *(uint8_t*)0x20000c36 = 0;
  *(uint8_t*)0x20000c37 = 0;
  *(uint32_t*)0x20000c38 = htobe32(-1);
  *(uint8_t*)0x20000c3c = 0xac;
  *(uint8_t*)0x20000c3d = 0x1e;
  *(uint8_t*)0x20000c3e = 0;
  *(uint8_t*)0x20000c3f = 1;
  *(uint8_t*)0x20000c40 = 0;
  *(uint32_t*)0x20000c44 = 0;
  *(uint8_t*)0x20000c48 = 0;
  *(uint8_t*)0x20000c49 = 0;
  *(uint8_t*)0x20000c4a = 0;
  *(uint8_t*)0x20000c4b = 0;
  *(uint8_t*)0x20000c4c = 0;
  *(uint8_t*)0x20000c4d = 0;
  *(uint8_t*)0x20000c4e = 0;
  *(uint8_t*)0x20000c4f = 0;
  *(uint8_t*)0x20000c50 = 0;
  *(uint8_t*)0x20000c51 = 0;
  *(uint8_t*)0x20000c52 = 0;
  *(uint8_t*)0x20000c53 = 0;
  *(uint8_t*)0x20000c54 = 0;
  *(uint8_t*)0x20000c55 = 0;
  *(uint8_t*)0x20000c56 = 0;
  *(uint8_t*)0x20000c57 = 0;
  *(uint8_t*)0x20000c58 = 0;
  *(uint8_t*)0x20000c59 = 0;
  *(uint8_t*)0x20000c5a = 0;
  *(uint8_t*)0x20000c5b = 0;
  *(uint8_t*)0x20000c5c = 0;
  *(uint8_t*)0x20000c5d = 0;
  *(uint8_t*)0x20000c5e = 0;
  *(uint8_t*)0x20000c5f = 0;
  *(uint8_t*)0x20000c60 = 0;
  *(uint8_t*)0x20000c61 = 0;
  *(uint8_t*)0x20000c62 = 0;
  *(uint8_t*)0x20000c63 = 0;
  *(uint8_t*)0x20000c64 = 0;
  *(uint8_t*)0x20000c65 = 0;
  *(uint8_t*)0x20000c66 = 0;
  *(uint8_t*)0x20000c67 = 0;
  *(uint8_t*)0x20000c68 = 0;
  *(uint8_t*)0x20000c69 = 0;
  *(uint8_t*)0x20000c6a = 0;
  *(uint8_t*)0x20000c6b = 0;
  *(uint8_t*)0x20000c6c = 0;
  *(uint8_t*)0x20000c6d = 0;
  *(uint8_t*)0x20000c6e = 0;
  *(uint8_t*)0x20000c6f = 0;
  *(uint8_t*)0x20000c70 = 0;
  *(uint8_t*)0x20000c71 = 0;
  *(uint8_t*)0x20000c72 = 0;
  *(uint8_t*)0x20000c73 = 0;
  *(uint8_t*)0x20000c74 = 0;
  *(uint8_t*)0x20000c75 = 0;
  *(uint8_t*)0x20000c76 = 0;
  *(uint8_t*)0x20000c77 = 0;
  *(uint8_t*)0x20000c78 = 0;
  *(uint8_t*)0x20000c79 = 0;
  *(uint8_t*)0x20000c7a = 0;
  *(uint8_t*)0x20000c7b = 0;
  *(uint8_t*)0x20000c7c = 0;
  *(uint8_t*)0x20000c7d = 0;
  *(uint8_t*)0x20000c7e = 0;
  *(uint8_t*)0x20000c7f = 0;
  *(uint8_t*)0x20000c80 = 0;
  *(uint8_t*)0x20000c81 = 0;
  *(uint8_t*)0x20000c82 = 0;
  *(uint8_t*)0x20000c83 = 0;
  *(uint8_t*)0x20000c84 = 0;
  *(uint8_t*)0x20000c85 = 0;
  *(uint8_t*)0x20000c86 = 0;
  *(uint8_t*)0x20000c87 = 0;
  *(uint8_t*)0x20000c88 = 0;
  *(uint8_t*)0x20000c89 = 0;
  *(uint8_t*)0x20000c8a = 0;
  *(uint8_t*)0x20000c8b = 0;
  *(uint8_t*)0x20000c8c = 0;
  *(uint8_t*)0x20000c8d = 0;
  *(uint8_t*)0x20000c8e = 0;
  *(uint8_t*)0x20000c8f = 0;
  *(uint8_t*)0x20000c90 = 0;
  *(uint8_t*)0x20000c91 = 0;
  *(uint8_t*)0x20000c92 = 0;
  *(uint8_t*)0x20000c93 = 0;
  *(uint8_t*)0x20000c94 = 0;
  *(uint8_t*)0x20000c95 = 0;
  *(uint8_t*)0x20000c96 = 0;
  *(uint8_t*)0x20000c97 = 0;
  *(uint8_t*)0x20000c98 = 0;
  *(uint8_t*)0x20000c99 = 0;
  *(uint8_t*)0x20000c9a = 0;
  *(uint8_t*)0x20000c9b = 0;
  *(uint8_t*)0x20000c9c = 0;
  *(uint8_t*)0x20000c9d = 0;
  *(uint8_t*)0x20000c9e = 0;
  *(uint8_t*)0x20000c9f = 0;
  *(uint8_t*)0x20000ca0 = 0;
  *(uint8_t*)0x20000ca1 = 0;
  *(uint8_t*)0x20000ca2 = 0;
  *(uint8_t*)0x20000ca3 = 0;
  *(uint8_t*)0x20000ca4 = 0;
  *(uint8_t*)0x20000ca5 = 0;
  *(uint8_t*)0x20000ca6 = 0;
  *(uint8_t*)0x20000ca7 = 0;
  *(uint8_t*)0x20000ca8 = 0;
  *(uint8_t*)0x20000ca9 = 0;
  *(uint8_t*)0x20000caa = 0;
  *(uint8_t*)0x20000cab = 0;
  *(uint8_t*)0x20000cac = 0;
  *(uint8_t*)0x20000cad = 0;
  *(uint8_t*)0x20000cae = 0;
  *(uint8_t*)0x20000caf = 0;
  *(uint8_t*)0x20000cb0 = 0;
  *(uint8_t*)0x20000cb1 = 0;
  *(uint8_t*)0x20000cb2 = 0;
  *(uint8_t*)0x20000cb3 = 0;
  *(uint8_t*)0x20000cb4 = 0;
  *(uint8_t*)0x20000cb5 = 0;
  *(uint8_t*)0x20000cb6 = 0;
  *(uint8_t*)0x20000cb7 = 0;
  *(uint8_t*)0x20000cb8 = 0;
  *(uint8_t*)0x20000cb9 = 0;
  *(uint8_t*)0x20000cba = 0;
  *(uint8_t*)0x20000cbb = 0;
  *(uint8_t*)0x20000cbc = 0;
  *(uint8_t*)0x20000cbd = 0;
  *(uint8_t*)0x20000cbe = 0;
  *(uint8_t*)0x20000cbf = 0;
  *(uint8_t*)0x20000cc0 = 0;
  *(uint8_t*)0x20000cc1 = 0;
  *(uint8_t*)0x20000cc2 = 0;
  *(uint8_t*)0x20000cc3 = 0;
  *(uint8_t*)0x20000cc4 = 0;
  *(uint8_t*)0x20000cc5 = 0;
  *(uint8_t*)0x20000cc6 = 0;
  *(uint8_t*)0x20000cc7 = 0;
  *(uint8_t*)0x20000cc8 = 0;
  *(uint8_t*)0x20000cc9 = 0;
  *(uint8_t*)0x20000cca = 0;
  *(uint8_t*)0x20000ccb = 0;
  *(uint8_t*)0x20000ccc = 0;
  *(uint8_t*)0x20000ccd = 0;
  *(uint8_t*)0x20000cce = 0;
  *(uint8_t*)0x20000ccf = 0;
  *(uint8_t*)0x20000cd0 = 0;
  *(uint8_t*)0x20000cd1 = 0;
  *(uint8_t*)0x20000cd2 = 0;
  *(uint8_t*)0x20000cd3 = 0;
  *(uint8_t*)0x20000cd4 = 0;
  *(uint8_t*)0x20000cd5 = 0;
  *(uint8_t*)0x20000cd6 = 0;
  *(uint8_t*)0x20000cd7 = 0;
  *(uint8_t*)0x20000cd8 = 0;
  *(uint8_t*)0x20000cd9 = 0;
  *(uint8_t*)0x20000cda = 0;
  *(uint8_t*)0x20000cdb = 0;
  *(uint8_t*)0x20000cdc = 0;
  *(uint8_t*)0x20000cdd = 0;
  *(uint8_t*)0x20000cde = 0;
  *(uint8_t*)0x20000cdf = 0;
  *(uint8_t*)0x20000ce0 = 0;
  *(uint8_t*)0x20000ce1 = 0;
  *(uint8_t*)0x20000ce2 = 0;
  *(uint8_t*)0x20000ce3 = 0;
  *(uint8_t*)0x20000ce4 = 0;
  *(uint8_t*)0x20000ce5 = 0;
  *(uint8_t*)0x20000ce6 = 0;
  *(uint8_t*)0x20000ce7 = 0;
  *(uint8_t*)0x20000ce8 = 0;
  *(uint8_t*)0x20000ce9 = 0;
  *(uint8_t*)0x20000cea = 0;
  *(uint8_t*)0x20000ceb = 0;
  *(uint16_t*)0x20000cec = 0xc0;
  *(uint16_t*)0x20000cee = 0xe8;
  *(uint32_t*)0x20000cf0 = 0;
  *(uint64_t*)0x20000cf8 = 0;
  *(uint64_t*)0x20000d00 = 0;
  *(uint16_t*)0x20000d08 = 0x28;
  memcpy((void*)0x20000d0a, "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000",
         29);
  *(uint8_t*)0x20000d27 = 0;
  *(uint32_t*)0x20000d28 = 0xfffffffe;
  syscall(__NR_setsockopt, r[0], 0xa02000000000000ul, 0x60ul, 0x20000800ul,
          0x530ul);
  return 0;
}
