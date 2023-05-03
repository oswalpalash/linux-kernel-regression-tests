// https://syzkaller.appspot.com/bug?id=a152f63add75a07ddbd234667e25286d8da14a34
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
#include <sys/stat.h>
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

static long syz_open_dev(volatile long a0, volatile long a1, volatile long a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  intptr_t res = 0;
  memcpy((void*)0x20000440, "/dev/vcsu#\000", 11);
  res = syz_open_dev(0x20000440, 6, 2);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20006480 = htobe32(0x67446698);
  *(uint32_t*)0x20006484 = 1;
  *(uint16_t*)0x20006488 = 2;
  *(uint16_t*)0x2000648a = 0;
  *(uint32_t*)0x2000648c = 3;
  memcpy(
      (void*)0x20006490,
      "\xef\x88\x27\x6c\xc3\x25\xd8\x1c\xa1\x90\x05\x68\x8b\x62\x90\x15\x70\xc2"
      "\x9f\x8f\x7f\xe6\x3a\x43\x47\x69\xee\x26\xe7\x9f\x37\xae\xdb\x65\xe4\x76"
      "\x79\xd7\x9c\xc9\x32\x99\x75\x5d\xc5\xb8\x0c\x2d\xcc\xa1\x7f\xcf\x18\x7b"
      "\x6c\x23\xdd\xcf\xc9\x48\xff\x1e\xe9\xf4\x31\x53\x0a\x2e\x19\x28\x53\xe3"
      "\xc7\x3d\xf5\x75\x91\x1f\x16\x1a\x1a\xce\x49\x9e\xca\xa8\x4e\xa5\x6f\x1e"
      "\x23\x15\x2a\xc0\xc9\x32\xf2\x80\xdd\x97\x65\x05\x59\xb0\x35\xd6\x34\xff"
      "\xcf\x15\xec\x7f\xa9\x25\xde\x0c\xad\x2c\xd0\x2f\x72\xcb\x40\xe1\x1f\xc0"
      "\xcf\x2f\x2a\xc6\xb9\x92\xf1\xea\x51\xae\x0d\xf0\x85\x63\x39\x58\x7b\xc2"
      "\x35\x54\xbf\xa7\x58\x83\x28\xff\xce\x81\x7c\x83\x3c\x68\x61\x0d\x7b\x78"
      "\xeb\xab\x09\x5e\x3e\xaf\x9d\xe9\x93\xc8\x75\x55\xb6\x8f\x8b\xb4\xb3\xf0"
      "\x50\x8b\xee\x41\x10\x9f\x53\xc9\x81\xc8\xef\x4b\x0d\x74\x3c\xa2\xae\x78"
      "\x78\x43\x6b\x0e\x1c\xec\x7d\x11\x91\x30\xc7\x14\x1f\x69\x4e\x9e\xde\x71"
      "\xc8\x90\xe4\x18\xc7\xd6\x0b\xa9\xa2\x1f\x9b\xca\xbb\xa5\xd7\xf0\x0f\xf1"
      "\x3d\xee\x9d\xee\x33\x2f\x55\x61\xb6\xde\xe8\x1b\x0f\x39\xf0\xb5\xea\xa7"
      "\xaa\xcb\xdc\x63\xfb\xa3\x23\x7d\xf9\x15\x37\x31\x53\x60\x49\xb3\xee\xe1"
      "\x9a\x57\xb2\x2f\xcb\x21\x4b\x87\x5d\x21\xba\xcf\x85\x06\xed\xa5\xad\x17"
      "\x2f\x31\xec\x6b\xa7\xe7\xcb\x01\xeb\x55\xa2\x6e\x2b\x91\x42\x65\x79\xb0"
      "\x8b\x23\x56\xa8\x3c\x6d\xd2\x9d\x7d\x32\x34\x85\xb5\x1d\x69\xaf\x52\x3d"
      "\x11\x59\x20\x7e\x17\x7e\xd0\x07\x20\xe7\x1d\x42\xf4\x51\xdf\xbf\x19\xc1"
      "\x91\x69\x28\xcd\x41\x66\x03\xc3\x11\xbc\x3c\xe3\x05\x85\x47\xf2\x7f\x1e"
      "\x98\x39\x7a\x2b\x29\x96\xca\xd5\x7b\xa7\x5d\x78\x17\x73\xb6\xc2\xb7\x64"
      "\xe1\xb6\xbe\x0d\xd8\x74\x84\xd1\xb6\x60\x66\x26\x8d\x06\x3f\x85\x47\xfe"
      "\x45\xf2\xfe\x2f\xb3\xef\xea\x07\xe7\xf1\xf8\xce\x8c\x1c\x63\xb9\xdc\x36"
      "\x56\xd3\x0a\x24\xb4\xa0\x53\xee\xa5\x69\x9b\x0e\x85\x14\xc7\x45\x91\xdd"
      "\xaa\xdb\x58\x9b\xb2\x55\x7b\x6c\x21\xdc\x96\x52\x1f\x6b\x22\x7d\x68\xe1"
      "\x16\x4b\x12\x27\x04\xcb\x22\x2b\x6c\x9f\x87\x30\xdc\xaa\x9b\xeb\x83\xf2"
      "\x79\xb8\x6f\xa9\xcf\xd2\xf5\x97\x69\x8e\x38\x4d\xd8\xd3\x4a\x76\x47\xfd"
      "\xdd\xd8\x07\x29\x17\x77\x66\xac\x5c\x27\x08\x05\x31\xfa\xb8\x79\xc3\x04"
      "\xbd\xc3\x55\xb8\xea\xe5\xef\x03\x1a\x02\xb2\x82\x34\xa8\x32\x7d\x78\xe8"
      "\x1c\xc8\x22\xd1\x27\x95\x4e\xbd\x49\x94\x79\xf0\xff\x3d\xab\xfd\x30\x14"
      "\xba\xe0\xef\xdb\x22\x5a\xfc\xbd\xf9\x41\x08\xf2\x70\xf6\x0e\x4f\xab\x72"
      "\xe6\xc4\xf2\xd7\x2e\xf4\x4e\x48\x38\x0e\xda\xbf\x91\x3a\xd1\x9c\x64\x12"
      "\x72\x23\x12\x49\x14\x82\x09\x88\x9c\x18\x91\x75\x40\xcb\x55\x71\xda\x3c"
      "\x38\x00\xd3\x3d\x4d\x97\x49\x30\x16\x36\xe8\x06\xbf\x0d\xcb\x1b\xbe\xa2"
      "\xe4\xf1\xdd\x32\xfa\x2d\x35\xcc\xc8\x94\xd9\x82\x7e\xdb\x73\x84\x6c\x82"
      "\x36\xc7\x87\x87\x5f\x62\xcb\x60\x45\xa6\x3b\x60\x1d\xd7\x50\xb4\x3d\xe8"
      "\x5b\x2d\xc1\xa7\x6e\xca\x3a\xa5\x15\x04\xa0\xcf\xdc\xd3\x4a\x6a\xd5\x08"
      "\xce\x46\x1c\x2e\xe6\x30\x83\xbd\x1d\x03\x30\x3a\xd5\xc0\x0b\x82\x92\xf4"
      "\xd8\xf3\x50\x70\xa2\x54\xc2\x7e\x49\xa1\x93\x15\x87\xbe\x24\xb0\xd4\x90"
      "\x77\x83\x5d\x88\x73\xbd\xa0\x0b\x06\xb0\x43\x56\xdb\x65\x68\x0c\xeb\xcc"
      "\xa3\x47\xee\xc4\x78\xe3\x57\x77\x96\x63\x0c\x8b\x5f\x57\x5f\x07\x70\x54"
      "\x7e\x74\xdd\xcb\x67\x7c\x99\x55\x35\x2f\xef\xe5\x51\x6e\xa1\x98\x7a\xa2"
      "\xf9\x70\xfd\xac\x5a\x83\xad\x54\xdf\xc8\xba\xd8\xa0\xf3\xb0\x3a\x58\xf7"
      "\x29\x61\xed\x51\x8b\xcb\x0e\xa1\x75\x7a\xb7\xce\xf2\x03\x0d\x23\xea\x42"
      "\x1d\xcc\xb2\x9e\x90\x11\x1d\x35\x09\x8a\x5f\xf8\x0c\xf1\x33\x6b\x4b\xc7"
      "\xe9\xae\x25\x4c\x10\xb6\xef\x41\xc3\x4a\x5c\xd8\xb2\xd1\x83\x54\x0e\xd6"
      "\x26\xb6\x1b\x88\xe3\x8d\xa1\xe9\x99\x25\x39\x47\xff\x17\xae\x2e\x28\x05"
      "\xcc\xb9\x73\x29\x01\xfa\x65\x24\xbc\xf9\x3d\x87\xc0\x3c\x40\x1c\x2f\x51"
      "\x15\xfd\xb9\x32\x16\x0a\x6e\xa1\xf8\x93\xe3\x04\x18\x82\x5d\x71\x3c\x96"
      "\x3d\x09\x29\x42\x12\x97\x01\x80\x34\x23\x72\x76\xab\x08\xb3\x6b\x47\x1a"
      "\x6b\x37\x83\x21\x1c\x29\xf5\x8b\xf4\x0d\xc6\xbe\x56\xc6\x84\x74\x80\x8d"
      "\x22\x0f\x85\xd9\x60\xf6\x2d\x23\xe8\xa4\x55\x68\x41\xb2\x29\xfd\x52\x61"
      "\xe8\xa5\x10\xee\x2e\x3a\x70\xda\x81\x62\x7d\x37\xc4\xc1\xa4\xfd\x8e\x4d"
      "\xe8\x96\xb8\xea\xaf\x8d\x54\xa7\xda\x09\x70\xc9\xbf\x2c\x4f\x02\xea\x4d"
      "\x46\x9f\x66\xf3\x2b\xbb\x01\x8b\x31\x57\xcd\xbe\x14\x24\x0c\x9a\x72\x41"
      "\x80\x70\x9e\x5a\xa8\x54\xea\x05\xea\x96\xfd\xdd\x61\x51\x7e\x5d\x86\x12"
      "\xf6\x5b\x5c\x65\x66\xd5\x99\xf4\x99\xfd\x0e\x12\x65\x45\xb8\x5b\x92\x55"
      "\x39\xce\x90\x9f\xd9\x98\xbb\xe0\x98\x74\x99\x71\xa5\x64\xb4\xf8\x8f\x62"
      "\x90\xd3\x63\x4b\x3d\x83\xa3\xab\x37\x30\x1e\xdd\x5c\xce\x1a\x05\x2b\x12"
      "\xee\x0d\x6e\x89\x6c\x3a\xce\x6d\xbc\xb8\x20\x8d\x7c\xd9\xb0\x7c\x9b\x03"
      "\x3d\x41\x5e\x52\x3c\x5e\x9a\xe8\x8a\x3a\xb7\x0b\x48\x3a\xb0\xba\xfb\x9b"
      "\xcc\x75\x83\x82\xde\x23\x8e\x14\x87\x5a\x63\x5d\x29\xf7\x0b\x54\xd0\x98"
      "\x14\x18\x08\xe7\xbb\x26\xab\x36\xa3\x97\xa2\x35\xe2\x5a\x4f\xe2\x0d\x67"
      "\xb1\x7d\x0e\x5e\x92\x33\x22\x16\x47\x19\x9e\x29\xa7\x9e\xe4\x07\x5e\x80"
      "\xcc\x38\x54\x36\x4b\x32\x35\x82\x8d\x10\xd0\xe0\x96\x58\x3b\x9d\xdf\xe1"
      "\x5f\x95\x63\xa4\x45\x5d\xe1\xae\x64\xc7\x28\x58\x1f\x39\xe1\x68\x10\x99"
      "\xf6\x89\x15\xbf\x28\x17\x4e\xa0\xc2\x82\x95\xc3\xe0\x8e\x69\x14\xef\x77"
      "\x95\x9c\x91\xfe\xdc\xf8\xf4\xed\x47\x3b\xf6\x40\x05\x61\x29\xa1\xf4\x9e"
      "\x2b\xc3\x1a\xb1\x2a\x91\x6c\x2e\x69\xc7\x2b\x46\x8b\xa7\x90\x46\xb3\xaf"
      "\x65\xb5\xae\x8b\x45\xa1\x53\x49\x92\x51\x40\x6f\x3f\x29\xeb\xd6\xcc\x7a"
      "\x65\xad\xe7\x96\x74\x3e\x29\x84\x28\x34\x6f\xbf\x9f\xcb\x24\x7e\x80\x64"
      "\x8e\x0a\xd0\x13\x5e\x8c\x9f\x3c\x96\xa5\x76\x2b\xf1\xe7\x06\xb5\xb6\x5a"
      "\xb0\x54\xd4\x82\x6f\x24\xb4\x61\x57\xdf\x90\x6c\x05\x10\xf5\x02\xed\x27"
      "\xb6\x30\x6e\x96\x84\xa6\xa1\x19\xe8\x0b\xb7\x22\xfd\xc1\x25\x4a\x92\x92"
      "\xe9\x82\x89\x84\x70\x7a\x15\xcb\x91\x2d\x74\x2d\x0c\x9e\x02\x59\xa4\x18"
      "\xf8\x34\x58\x73\x70\x4d\x38\x8d\xcc\x3b\x81\x05\xdc\xb1\x7b\x45\xe6\x0b"
      "\x3e\xa8\xef\xab\x3d\x3e\x5b\x7e\x17\x3e\x4f\xdf\x51\x8d\xbe\x17\x48\x96"
      "\x20\xdf\x39\x89\x93\xc0\xb5\xae\x2a\xb9\x59\x8e\xda\xbf\x02\x94\xf9\xe0"
      "\xf4\x2a\x9c\xbb\x08\x6d\x57\x9f\x24\x56\x81\x93\xe0\x87\xd7\xcd\xea\xf4"
      "\xbe\x4e\xc1\x8f\x82\xb1\x0e\xfd\x03\xb8\xd7\x9c\xe7\xf6\x96\xb1\xe1\x1f"
      "\x2b\x81\xcb\xf9\xba\x9c\xec\x45\x75\x8f\xdd\x8c\xcc\xf4\x12\x93\x62\xe3"
      "\x63\x75\xbd\xb3\x83\x53\x46\xe8\x53\x8e\xbd\xf1\xd4\xaa\xc0\x9a\xc0\x80"
      "\x1e\x5d\x19\x8d\x2f\x75\x03\xf4\x7a\x52\x5f\x83\xd2\x33\x49\x2f\xa4\x3b"
      "\x39\xf6\x1f\x60\xe7\x8c\x27\xd6\x9a\x20\x99\xb0\xe2\x97\x7d\x0e\xc8\xad"
      "\x70\xa8\x3a\x59\xc3\x74\x49\x9c\x67\xd8\xbd\x12\xde\x62\x7e\xc5\x02\x4c"
      "\xcd\xf9\x9e\x4d\x51\x3c\xff\xb5\xf1\x53\x81\x7a\x96\x58\x65\xaf\xb9\x52"
      "\xb8\xf0\x3f\x0f\x6c\x4d\x4b\x22\x4d\x43\x3a\x24\xf3\x5e\xc2\xaa\xd1\xb5"
      "\x42\x0a\x02\x8a\x5e\x45\x4b\x2e\x51\x03\x63\x41\x2d\xd7\x0c\x6b\x04\x20"
      "\xbd\xee\xb3\xe8\x87\x3c\x54\x99\xcb\xae\xb4\x5b\x4d\x38\x92\x9b\x1b\xcd"
      "\x9f\x7d\x98\xd3\x19\x86\x61\xd2\xcb\x93\x29\x18\x40\x6a\x88\x68\x03\x68"
      "\x2f\x3e\x0c\x91\x31\x66\x86\xd5\x93\xf0\xe9\x5e\x91\xba\xd3\x6d\x50\x55"
      "\xdd\xa1\xec\xd6\x1d\x89\x1d\xe2\xa2\x0f\x77\xa7\xa1\xdf\xb3\xb4\xd8\xfc"
      "\x9b\xeb\x41\x57\x10\x58\x67\xef\x8f\xcf\xb4\x55\xcb\x47\x9b\x48\x49\x6e"
      "\xcf\xc7\x4d\xb1\xe2\xbd\x48\x85\x47\x07\x1b\xbf\x59\xbe\xd4\x3f\xda\x13"
      "\x2c\x84\xdd\x48\x88\x79\x9b\xcb\xeb\xbf\xb7\xff\x53\xa3\x12\x6a\x79\x74"
      "\xd5\x6f\x33\xb2\xdb\x2e\x56\xe4\x93\xf7\x9a\x46\x28\xf2\xa5\xd5\xdb\xcd"
      "\xba\x0d\x19\x07\x0c\x60\xe8\x65\xae\x95\xca\x61\xf6\x6f\x6e\xe2\x09\x53"
      "\xcd\xe2\x96\x66\x77\xb7\xb7\xd7\x13\xe7\xfc\xd8\xd2\x2b\xbb\x27\x6a\x94"
      "\x61\xd9\xbe\xc7\x06\xef\x40\x7e\xe4\x62\x74\x7a\x02\xf8\x45\x45\x3b\xc3"
      "\x6c\xaa\x5b\xd4\x6f\x63\xb2\xe3\xad\x2b\x05\x77\x71\xb4\x9c\xf4\x21\xa6"
      "\xdc\x51\xec\x8f\x7c\x33\x10\x32\xca\x69\xca\xe1\xcb\x16\x16\x27\x56\x41"
      "\xa7\xae\x40\x00\x2c\x36\x50\x72\xaf\x74\xc4\x41\xe2\x65\xe8\x0d\x10\xed"
      "\x3b\x3e\xe3\xac\xd2\x2e\xd0\xea\xec\xdb\x75\x04\xb3\xc8\xf7\xdb\x62\x51"
      "\x0d\x57\x6a\x50\x3f\xf3\x57\xc6\x65\x40\x6f\x97\x28\x19\x08\x34\xcf\x06"
      "\xda\x45\x51\xc7\xb1\x08\x93\xb8\x52\x86\x50\x4c\xf8\xb7\x22\x10\x7e\xe6"
      "\xf4\xa3\xa5\xc6\xe4\x82\xbd\x23\x4e\x8c\x8b\x49\x4b\x99\xb3\x65\x3c\x42"
      "\x92\x05\xb7\x35\x81\x7d\x42\x22\x9a\x13\xe5\x90\x37\x2e\xd7\x10\x75\x38"
      "\x5b\x7a\x15\x84\xf7\xf9\x79\x0d\x19\x8c\x3a\x22\x62\x9e\xd8\x55\xcc\x0b"
      "\xc2\x5e\x3a\x60\xf5\xc2\xde\xef\x5c\x4a\xdf\xec\x20\x14\xf4\xb2\x32\x44"
      "\x8c\x12\xa0\xf8\xc0\x5e\x4d\x65\x2a\x16\x10\xd2\xcb\xf5\xa7\x43\x4f\x50"
      "\xa8\xd4\xf3\xed\xae\x83\x92\x2c\xbb\x14\xfc\xdc\x8d\xba\x25\x62\x7c\x38"
      "\x96\x91\x7c\xe4\xc7\x73\x95\xe6\x1f\xeb\xfa\xa7\x75\xbf\x8b\x93\x2a\x77"
      "\x50\x60\xf2\xf2\x39\x18\xf9\x57\x3f\x8e\xa5\xfe\xdc\xba\x27\x94\xbb\x3e"
      "\xd8\x79\x92\x65\x91\x96\x64\xee\xe5\xd7\x2f\xc6\x81\x9f\xfb\xcd\xf1\x00"
      "\xe1\x28\x08\xe1\x3a\x39\x61\x3c\x57\xc4\x77\x10\x0e\x69\xed\x2f\x67\x1f"
      "\x48\x67\x8e\x2f\xe0\xaa\x80\x50\xdd\xd1\xcc\x8a\xea\xe8\x3f\xe4\xa4\x46"
      "\x9c\x5a\x36\x21\x01\x9f\x83\xff\x6a\x99\xa1\x53\xb0\xfd\x37\x01\xc6\xd0"
      "\x99\x37\xc3\xd6\x5e\x7c\x5c\xb4\xc5\x4d\xf9\xb4\x88\xe6\xad\xed\x22\xba"
      "\x4f\xac\xee\x74\x30\x27\x63\x1e\x55\xd6\x8b\x68\x8c\x3d\x91\xc7\x66\x5a"
      "\x70\xce\xbc\xcc\x98\xd3\x41\x47\x60\xf8\x67\x0b\xd8\x09\x3d\x38\xc7\x8f"
      "\x5d\xc9\x09\x42\xe7\xdf\xea\xd7\xe1\x80\xcd\x1f\x41\xce\x54\x4d\x23\x12"
      "\xab\x88\x71\xe3\x09\x1a\x76\xae\x54\x4e\x8e\x9e\x33\xa3\x7e\xea\x16\x5e"
      "\x4c\xca\xd1\x26\x28\xc9\xdf\x45\x31\x0b\xa9\x35\x5a\xd9\x9f\xfb\x71\xf2"
      "\xdc\x2d\xdd\x8e\xf1\x64\x78\x35\x2c\x67\x57\x60\xb3\x0e\x2a\x2c\xa5\x26"
      "\x19\xb8\x1a\x20\x03\x89\x57\x52\x19\x20\xd2\x7b\x6c\xee\x73\xb8\xcb\xc4"
      "\xcc\x6d\x33\x4b\xe3\x6a\x94\x50\x62\x46\xe1\x94\x10\x97\xc8\xcf\x11\x35"
      "\xba\xe4\x28\x25\x61\x31\x3a\x93\xba\x30\x2e\x68\x59\xc9\x6f\xc3\x70\x6e"
      "\x51\x73\xfc\x97\x68\x0d\xb2\x93\xd6\xcd\x96\x1a\xa1\xd1\x83\xb9\x02\xaf"
      "\x0d\x56\x09\xc4\x39\x4a\x64\xe5\x3a\x76\x59\x5e\x4b\x05\x36\xb5\x52\xd6"
      "\xfc\x47\xe5\x75\x1f\xa2\x27\x62\x2b\x06\x11\x33\x92\x31\x1d\x28\x71\xf9"
      "\x8f\x50\x85\xf7\xfc\x59\x90\xd6\x19\xef\x32\xe7\x24\x9a\x6d\x80\x4f\xb6"
      "\x9c\x5c\x1d\xb0\x41\x32\xfa\xd3\xcb\x3f\x26\xd4\x2e\x02\x2c\x6f\xc5\x3e"
      "\xf1\x06\x4e\x7e\x9a\x37\x50\x61\x75\xa1\xfa\x01\x10\x65\x5f\xe4\xba\x1d"
      "\x50\xd4\x9e\xfd\xd7\xf4\xec\xa1\xdb\x00\x13\x77\x7c\x68\xba\x79\xcc\x0d"
      "\xfa\x77\x8d\x3b\x38\x86\xad\x64\xea\x58\x6f\xb7\x3a\x0e\xb9\x18\x09\x69"
      "\x6e\x18\x01\x0c\x3e\xcd\x83\xc9\x44\xd3\xe5\xf1\x6d\x09\x5e\x25\x7a\x27"
      "\x7c\xf3\xfc\x97\x63\xe6\xe8\xbc\xe3\x6e\x89\xb6\xa0\x9b\xf6\x9e\xf4\xbe"
      "\xdd\x6e\x25\xb2\x0a\x41\xbe\x51\xe3\xe8\x79\x00\x11\x83\x7a\xab\xa1\x51"
      "\x04\x05\xaa\x2e\xb3\x2d\xda\xe9\x1c\x9a\x45\xaa\x4e\x4c\x03\x58\xd5\x0f"
      "\xaf\x63\x4a\x67\x44\xa6\x6a\x4f\x22\x0b\xb8\x55\x63\x65\x81\x23\x23\xf5"
      "\x35\x0e\xe7\x66\x80\x18\xdc\x1a\x98\x6b\x60\xd7\x7e\x3c\x0e\x26\x1c\xbf"
      "\xc8\x8a\x43\xd4\x71\x62\x47\xc3\xcf\x13\xe8\x86\x3c\xf1\xc7\x94\x2a\x64"
      "\x85\x4f\x5e\xad\xbd\xdb\x83\x88\x21\xa6\x45\x09\x12\x23\x2c\x42\xca\x43"
      "\x18\x50\x14\x38\x6d\x19\x96\x04\x3f\x0b\x29\x8a\xc7\x02\x32\x3f\xf2\xe4"
      "\x39\xe4\x03\xb8\x5c\xe1\x51\xad\x5d\x4d\xc5\xa8\x3f\xa7\xc4\x0f\x16\xe5"
      "\x9c\x06\xe0\xaf\x3b\x80\x9f\x72\x19\x1c\x6a\x44\x66\x82\x48\x16\x8c\xa7"
      "\x98\x48\x0e\xdb\xea\xc2\xae\x1e\xec\x22\x68\x86\x7d\xff\x4e\xc1\x67\xbe"
      "\x06\x29\x2c\xd6\xdd\x7e\x44\x1c\xd7\x4b\x2e\x67\xa1\xdb\x29\x19\x47\x99"
      "\x1d\x1c\x59\xa8\x99\x15\x0d\xe6\x5d\xda\xd0\x43\x1f\x5c\xde\x0b\x97\x9a"
      "\x23\x4d\x5d\xd7\x49\x5a\xc6\x22\x6f\xed\x48\x8f\x9e\x0c\x11\x70\x30\xba"
      "\xb8\x8d\x69\xd9\xb2\xa3\x60\xee\xcb\xbb\xcb\x0b\xa3\x7e\x29\x30\xdf\xbd"
      "\xa2\xd6\xdc\x41\x41\xf5\x4c\xec\xcc\x95\xaa\x06\x29\xd9\x62\x8f\x10\x26"
      "\xc5\x58\xdd\x33\x44\xb1\x1b\x97\x7e\x4f\x18\x4a\x42\xea\xcf\xcd\x50\x13"
      "\x7e\xe2\x99\x64\xfd\x11\x3a\x8c\x37\x17\x22\xe4\x29\xa3\x74\x57\x8e\xea"
      "\x37\xc0\xe6\x91\xe4\x17\x87\x86\x75\xbf\x9d\x46\x1d\x63\xdd\xd4\x48\x8b"
      "\x3e\x9a\xa3\x89\x5d\x75\x05\xb4\xbc\x5a\x04\xaf\xca\xad\xa2\x50\x63\x37"
      "\xf0\xfb\x05\xfa\x95\xec\x0a\xcb\x3c\xe6\xbf\x41\x05\xc5\xb4\x19\x80\xb8"
      "\xf8\xa2\x9a\xb1\xb7\xb6\x0e\x71\xea\xae\xb5\xc7\xcc\x5e\xc1\x2d\x44\x30"
      "\xcc\x21\x95\x31\x71\xe7\xb4\xda\x08\x0a\xbc\x0b\xd0\xf6\xf7\xfa\x4d\x16"
      "\xa9\x86\x35\x9b\x2b\xe4\x4c\x6c\x79\x19\x7c\x6e\x82\x80\x8c\x67\xcc\x26"
      "\x40\x50\x78\x56\x37\x0f\x18\x95\x49\x0f\x36\x89\x8b\xbb\xb7\x62\x1a\xb0"
      "\x24\x3e\x3a\x6f\x15\xa5\xf6\x8c\x94\x31\xd8\xef\xd1\x75\xdd\x87\xc4\x87"
      "\x0b\xcc\xa3\x40\xa0\x85\x35\x29\x84\x7a\xd2\x29\x22\x2b\xaf\x87\x51\x49"
      "\xbc\x6e\xfc\x49\x22\x6b\xdc\x2b\x47\x2b\x0f\xce\xd3\x60\x93\xa5\x54\xdb"
      "\x65\x28\x71\x3c\x3c\x90\xbf\x6d\xf4\x33\xa3\xbe\xa4\xe5\xeb\xbb\xbc\x53"
      "\xa2\x2b\x8e\x83\xcc\x8f\xae\xe0\xa5\x8c\x5e\x90\xf9\xbc\x11\xf2\xfd\xc4"
      "\x68\x2f\x1e\x18\x78\xd6\x43\x52\x5b\xab\xd0\x87\xb3\xfb\xda\xc7\xd7\x3a"
      "\x83\xfd\xc7\xa1\x58\xf0\xab\xc1\x19\xd5\x10\x4b\x5d\x12\xc1\xd0\x25\x08"
      "\x54\xfe\x77\x69\x3a\x08\x1c\x37\x20\xf4\xa5\x41\x81\x91\xfc\x2b\xb0\xb2"
      "\x25\x12\xb4\x7c\x93\xcb\x83\x51\x21\xe5\xac\xf0\xe6\x19\x3a\x19\xfa\x41"
      "\xbb\x31\x48\x22\x08\x2c\x40\xfe\x10\xfb\x52\xb1\x18\xe2\x1d\xef\x60\xac"
      "\x92\x22\x89\xd3\xe3\x7e\xfb\xa7\x70\x84\x76\xdd\x37\xc3\x46\xaf\xf4\x68"
      "\x47\x5a\xa2\x14\x3d\xe0\x5c\x05\x02\x11\xc9\xbf\xbd\x65\x2b\x17\xc5\x05"
      "\xd3\xae\x7f\xef\x2c\x4d\x87\xd4\x4c\x35\xfd\x2b\xff\xf1\x1a\xb7\x5d\x78"
      "\x13\x29\x30\x42\xf2\x96\x73\xa9\x20\x5b\x92\x95\xd9\x44\xf8\xc5\x54\xfd"
      "\xc9\x14\xe9\x0f\x63\x61\xdd\x73\x00\xdf\x29\x73\x74\xbb\xd9\xe9\x69\x75"
      "\x3d\x9d\x39\x46\xe5\x7f\x78\x28\xc9\x22\xcf\x5e\x75\xf0\x40\xe6\xad\x4a"
      "\xa1\x5a\x8c\x4b\xd8\xcd\xcb\x67\x87\x68\x69\x76\x5f\x0b\x0e\x26\xe3\x0b"
      "\x68\xa2\xf6\x19\xbb\xc1\xf1\x61\x12\xa1\x3a\x5d\x11\xab\xd5\xb3\x0f\xcf"
      "\x25\xf3\x4e\x7e\x45\xdb\x81\xa3\x13\x07\x84\x80\xdb\x3c\x06\xb2\xc1\xd5"
      "\x7e\x6b\x71\x8f\x4d\xf6\x0a\x4a\x8f\xfc\xa2\x0d\x4a\x64\xb9\x26\x93\xef"
      "\x71\x41\xe1\x3f\x7b\x4d\x7f\x1a\xb2\x6d\x6b\xa2\x44\xee\xa3\x0c\xa4\x73"
      "\xdd\xe4\x20\xe0\xd7\x98\xe5\x78\x4d\x8a\xcc\xb1\x6f\x34\x98\xba\xea\x0b"
      "\x2a\xeb\x2a\xc3\xeb\xc0\xf8\xd9\x15\x2e\xa8\x51\xaf\x5d\xce\xeb\x6a\xae"
      "\xce\x10\xa1\xf0\xcd\x80\x87\x37\x4a\xad\x82\x01\xa2\x4e\xc7\xc3\x5a\x7b"
      "\x0f\x46\x2d\x88\xc6\xf2\x28\x31\x06\xa2\x8d\x20\x21\x18\x09\xf5\x16\x61"
      "\xa1\x7b\x43\x10\x72\xa1\x1d\x26\x68\xca\x75\xc0\x98\xf5\xc0\xd0\x28\x44"
      "\xc1\x2e\xf5\x7e\x72\xc4\xf9\x67\x84\xd1\x5d\x91\xfc\x01\x7f\xa4\xf7\x0f"
      "\x2e\x4d\x8b\x49\x6d\x1c\x35\x4a\xb0\x65\xa8\x95\xc1\x3c\xc5\xa2\x7b\x78"
      "\x2c\x62\x2c\xcf\x84\x35\xf1\xbc\x44\x15\xfa\x84\x92\x82\x91\xde\x3f\x8e"
      "\xb2\x5f\xc8\x06\xfb\xa9\x51\x69\x26\x92\x8d\x80\x98\x22\x16\x0e\x6d\x90"
      "\x9f\x77\xe6\x27\x4c\xef\x0d\xb2\xde\x39\x55\x0d\x53\x9b\x80\x0a\x22\xb1"
      "\xcd\x61\x24\xa9\x65\x48\x57\xa8\xc3\xc3\x62\xd5\x7f\x12\xfd\x94\xa6\x39"
      "\x0a\x2f\x78\xe2\xbc\x72\x87\x3e\x41\x7e\xc9\xdf\xc7\x7a\xad\x89\x02\x0c"
      "\xc3\x28\x64\x6f\x02\x84\x08\xd0\x97\x38\x05\x2d\xb8\xa9\xbe\x15\xab\x1c"
      "\x57\xb7\x62\x13\x9c\x4a\x7f\x1f\x6c\xe7\xd2\x53\x84\xad\xd9\xee\xa5\x5a"
      "\xab\xc9\x19\xf5\xbc\xb3\x69\x9e\x78\x2f\xa1\x03\xb6\x27\x11\x40\x38\xc6"
      "\x77\x80\x9f\x24\x10\x2f\xaf\xaa\xf9\x78\x1d\xa1\x66\x31\xd9\xe0\x71\xd5"
      "\x4a\xaa\x79\x1e\x1f\xce\x81\x8f\xa5\xe3\x94\xd2\x81\x4a\x7d\x3d\xe1\x9b"
      "\x7b\x18\x8f\x74\x9d\xff\x80\xc7\x3d\x99\x2a\x47\x72\xeb\x31\xc0\x67\x3f"
      "\x99\x9e\xce\x6a\xf1\xfe\x0f\x79\xee\x03\xf9\xdd\xdc\x11\x1d\xe9\xb9\x61"
      "\xe6\xc2\x22\xba\x51\x54\x5c\xcf\xb4\xdf\xf6\x8e\xd3\xca\xbe\x10\x5e\xfb"
      "\xd9\x44\x7c\xe2\x22\x9a\xc0\xf4\x55\xf5\x92\x34\x84\x82\xa3\xb1\x7e\x13"
      "\x70\x7b\x61\x50\x32\x38\x39\x54\x35\x6f\x63\xab\xa6\x63\xc6\x09\x5e\xce"
      "\xcf\xbb\x76\x3c\x5d\x7f\x80\x9c\x18\x0e\xac\xa4\xc1\x8c\xa6\x1b\x56\x75"
      "\x5d\x3a\x36\x22\xe3\x3f\xe3\x33\x6c\x98\x18\x1b\xfb\xd4\xdb\x3a\x37\x53"
      "\xef\xca\x96\xa2\x89\xb0\xe7\x09\x55\x39\xc9\xa4\x3f\x29\xbc\xd2\x8a\xad"
      "\x58\xaa\x85\x1a\x97\x22\xd5\x64\x3a\x71\x60\x20\x3b\x75\xff\xaf\x54\xf0"
      "\x2b\x1a\xe3\x2b\x8b\x0b\x77\x95\xb7\x80\x04\x96\x7b\x93\xf4\xb1\x8a\x11"
      "\xeb\xe0\x80\xc3\x07\x8e\xe2\x76\x40\xe2\xba\x00\xad\x43\xcb\xf1\xef\xa3"
      "\x95\x24\x44\x21\xda\x31\x71\x12\x3b\xbf\x05\x1a\x32\x74\xae\x10\xf8\xb1"
      "\xbb\x70\xc2\x33\x32\x82\x6c\x1b\xbf\x77\xc4\x5c\x3b\x0a\xa5\xac\x41\xcd"
      "\x01\x2a\x28\x8a\x22\xd9\x81\x21\xdb\x84\x5e\x09\x48\x42\x95\xb4\xb0\x1f"
      "\xbc\xf2\x78\x5d\xb1\xfd\x9e\xfa\x0a\x88\x41\x66\xb9\x13\x3d\xf3\xda\xca"
      "\x34\xeb\xe6\x0a\x15\x31\x82\xe8\x64\x3a\xed\xd1\x89\x9b\x89\x53\x2b\xac"
      "\x0d\xe7\x5a\x4b\x0b\x3a\x0d\x35\xed\xb3\x86\x92\x77\xd7\xc2\xfd\xf1\xf4"
      "\x81\x9e\xd9\x33\x4c\xef\x6b\xc0\x88\xf4\x89\x9f\xea\x20\x79\xfd\x8d\xa9"
      "\xd1\x57\xd4\xee\x72\x24\x33\xd2\xf8\x8e\xdf\xc0\xa4\x61\x45\x25\x72\x71"
      "\x31\x06\x4a\xbb\x28\xe4\x51\xd4\x86\x00\x17\x90\xeb\x73\xf1\xd4\xeb\x8d"
      "\xfc\x86\x00\xd5\x5e\x04\x5d\x7f\x97\x8a\x92\x2f\x70\x4d\x1f\x05\x9d\x18"
      "\xbd\x73\x9b\xc6\x04\xac\xe2\x7c\x35\xa4\x03\x94\xdc\xda\x49\xf0\x53\x4e"
      "\xff\x30\xbb\x73\x9b\xf3\x34\x85\xb2\x66\xae\x4c\x48\xf7\xf1\xcb\x72\x2a"
      "\xee\xc0\xf7\x09\x0f\x2d\xb0\x58\x5f\x9a\x07\xb5\xe9\x8c\x82\x68\x45\x83"
      "\xe5\x45\xad\x6f\x2f\xdd\xaf\x1d\x8e\x09\x74\xf6\x0f\xb6\xf5\x44\x99\xef"
      "\x5a\xdb\x7a\x12\x72\x0e\x44\x9f\xb0\xf9\x85\xad\x72\x5a\x78\x93\x6b\x17"
      "\xe1\xfd\x55\x74\x5e\x99\x2e\x2d\xcc\xf2\x1c\x5d\x70\xd2\x7c\x57\x1d\x13"
      "\x58\x4f\x5a\x7f\x33\x24\x08\xb1\x95\x6a\xb0\x65\xb1\xcf\x8c\xb3\xec\x62"
      "\x1f\xc8\x0b\x43\xa8\xf1\x6a\xe9\x54\x2f",
      4096);
  syscall(__NR_write, r[0], 0x20006480ul, 0x1010ul);
  return 0;
}
