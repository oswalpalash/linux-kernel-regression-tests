// https://syzkaller.appspot.com/bug?id=c80120fd851dfa2216058149b13138c51afe77e0
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/genetlink.h>
#include <linux/netlink.h>

static long syz_genetlink_get_family_id(long name)
{
  char buf[512] = {0};
  struct nlmsghdr* hdr = (struct nlmsghdr*)buf;
  struct genlmsghdr* genlhdr = (struct genlmsghdr*)NLMSG_DATA(hdr);
  struct nlattr* attr = (struct nlattr*)(genlhdr + 1);
  hdr->nlmsg_len =
      sizeof(*hdr) + sizeof(*genlhdr) + sizeof(*attr) + GENL_NAMSIZ;
  hdr->nlmsg_type = GENL_ID_CTRL;
  hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
  genlhdr->cmd = CTRL_CMD_GETFAMILY;
  attr->nla_type = CTRL_ATTR_FAMILY_NAME;
  attr->nla_len = sizeof(*attr) + GENL_NAMSIZ;
  strncpy((char*)(attr + 1), (char*)name, GENL_NAMSIZ);
  struct iovec iov = {hdr, hdr->nlmsg_len};
  struct sockaddr_nl addr = {0};
  addr.nl_family = AF_NETLINK;
  int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (fd == -1) {
    return -1;
  }
  struct msghdr msg = {&addr, sizeof(addr), &iov, 1, NULL, 0, 0};
  if (sendmsg(fd, &msg, 0) == -1) {
    close(fd);
    return -1;
  }
  ssize_t n = recv(fd, buf, sizeof(buf), 0);
  close(fd);
  if (n <= 0) {
    return -1;
  }
  if (hdr->nlmsg_type != GENL_ID_CTRL) {
    return -1;
  }
  for (; (char*)attr < buf + n;
       attr = (struct nlattr*)((char*)attr + NLMSG_ALIGN(attr->nla_len))) {
    if (attr->nla_type == CTRL_ATTR_FAMILY_ID)
      return *(uint16_t*)(attr + 1);
  }
  return -1;
}

uint64_t r[2] = {0xffffffffffffffff, 0x0};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  res = syscall(__NR_socket, 0x10, 3, 0x10);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000040, "TIPC\000", 5);
  res = syz_genetlink_get_family_id(0x20000040);
  if (res != -1)
    r[1] = res;
  *(uint64_t*)0x200001c0 = 0;
  *(uint32_t*)0x200001c8 = 0;
  *(uint64_t*)0x200001d0 = 0x20000140;
  *(uint64_t*)0x20000140 = 0x200000c0;
  *(uint32_t*)0x200000c0 = 0x24;
  *(uint16_t*)0x200000c4 = r[1];
  *(uint16_t*)0x200000c6 = 0x829;
  *(uint32_t*)0x200000c8 = 0;
  *(uint32_t*)0x200000cc = 0;
  *(uint8_t*)0x200000d0 = 1;
  *(uint8_t*)0x200000d1 = 0;
  *(uint16_t*)0x200000d2 = 0;
  *(uint32_t*)0x200000d4 = 0;
  *(uint16_t*)0x200000d8 = 5;
  *(uint16_t*)0x200000da = 0;
  *(uint16_t*)0x200000dc = htobe16(8);
  *(uint16_t*)0x200000de = htobe16(0x19);
  *(uint32_t*)0x200000e0 = htobe32(0);
  *(uint64_t*)0x20000148 = 0x24;
  *(uint64_t*)0x200001d8 = 1;
  *(uint64_t*)0x200001e0 = 0;
  *(uint64_t*)0x200001e8 = 0;
  *(uint32_t*)0x200001f0 = 0;
  syscall(__NR_sendmsg, r[0], 0x200001c0, 0);
  return 0;
}
