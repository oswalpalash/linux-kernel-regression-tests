// https://syzkaller.appspot.com/bug?id=dacc7707e9dc2ff70c53cb2ad92b4c675818ac68
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
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

#ifndef __NR_mmap
#define __NR_mmap 222
#endif
#ifndef __NR_sendmsg
#define __NR_sendmsg 211
#endif
#ifndef __NR_socket
#define __NR_socket 198
#endif

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

static long syz_genetlink_get_family_id(volatile long name,
                                        volatile long sock_arg)
{
  int fd = sock_arg;
  if (fd < 0) {
    fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (fd == -1) {
      return -1;
    }
  }
  struct nlmsg nlmsg_tmp;
  int ret = netlink_query_family_id(&nlmsg_tmp, fd, (char*)name, false);
  if ((int)sock_arg < 0)
    close(fd);
  if (ret < 0) {
    return -1;
  }
  return ret;
}

uint64_t r[3] = {0xffffffffffffffff, 0x0, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0x10);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000140, "l2tp\000", 5);
  res = -1;
  res = syz_genetlink_get_family_id(0x20000140, -1);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_socket, 0xaul, 2ul, 0x73);
  if (res != -1)
    r[2] = res;
  *(uint64_t*)0x20000240 = 0;
  *(uint32_t*)0x20000248 = 0;
  *(uint64_t*)0x20000250 = 0x20000200;
  *(uint64_t*)0x20000200 = 0x20000040;
  *(uint32_t*)0x20000040 = 0x3c;
  *(uint16_t*)0x20000044 = r[1];
  *(uint16_t*)0x20000046 = 0x923;
  *(uint32_t*)0x20000048 = 0;
  *(uint32_t*)0x2000004c = 0;
  *(uint8_t*)0x20000050 = 1;
  *(uint8_t*)0x20000051 = 0;
  *(uint16_t*)0x20000052 = 0;
  *(uint16_t*)0x20000054 = 5;
  *(uint16_t*)0x20000056 = 7;
  *(uint8_t*)0x20000058 = 0;
  *(uint16_t*)0x2000005c = 8;
  *(uint16_t*)0x2000005e = 9;
  *(uint32_t*)0x20000060 = 0;
  *(uint16_t*)0x20000064 = 6;
  *(uint16_t*)0x20000066 = 2;
  *(uint16_t*)0x20000068 = 1;
  *(uint16_t*)0x2000006c = 8;
  *(uint16_t*)0x2000006e = 0xa;
  *(uint32_t*)0x20000070 = 0;
  *(uint16_t*)0x20000074 = 8;
  *(uint16_t*)0x20000076 = 0x17;
  *(uint32_t*)0x20000078 = r[2];
  *(uint64_t*)0x20000208 = 0x3c;
  *(uint64_t*)0x20000258 = 1;
  *(uint64_t*)0x20000260 = 0;
  *(uint64_t*)0x20000268 = 0;
  *(uint32_t*)0x20000270 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000240ul, 0ul);
  return 0;
}
