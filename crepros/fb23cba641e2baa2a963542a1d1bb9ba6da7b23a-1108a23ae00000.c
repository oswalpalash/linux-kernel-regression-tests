// https://syzkaller.appspot.com/bug?id=fb23cba641e2baa2a963542a1d1bb9ba6da7b23a
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdarg.h>
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

static int netlink_send_ext(int sock, uint16_t reply_type, int* reply_len)
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

static int netlink_send(int sock)
{
  return netlink_send_ext(sock, 0, NULL);
}

const int kInitNetNsFd = 239;

#define DEVLINK_FAMILY_NAME "devlink"

#define DEVLINK_CMD_RELOAD 37
#define DEVLINK_ATTR_BUS_NAME 1
#define DEVLINK_ATTR_DEV_NAME 2
#define DEVLINK_ATTR_NETNS_FD 137

static void netlink_devlink_netns_move(const char* bus_name,
                                       const char* dev_name, int netns_fd)
{
  struct genlmsghdr genlhdr;
  struct nlattr* attr;
  int sock, err, n;
  uint16_t id = 0;
  sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (sock == -1)
    exit(1);
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = CTRL_CMD_GETFAMILY;
  netlink_init(GENL_ID_CTRL, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(CTRL_ATTR_FAMILY_NAME, DEVLINK_FAMILY_NAME,
               strlen(DEVLINK_FAMILY_NAME) + 1);
  err = netlink_send_ext(sock, GENL_ID_CTRL, &n);
  if (err) {
    goto error;
  }
  attr =
      (struct nlattr*)(nlmsg.buf + NLMSG_HDRLEN + NLMSG_ALIGN(sizeof(genlhdr)));
  for (; (char*)attr < nlmsg.buf + n;
       attr = (struct nlattr*)((char*)attr + NLMSG_ALIGN(attr->nla_len))) {
    if (attr->nla_type == CTRL_ATTR_FAMILY_ID) {
      id = *(uint16_t*)(attr + 1);
      break;
    }
  }
  if (!id) {
    goto error;
  }
  recv(sock, nlmsg.buf, sizeof(nlmsg.buf), 0); /* recv ack */
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = DEVLINK_CMD_RELOAD;
  netlink_init(id, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(DEVLINK_ATTR_BUS_NAME, bus_name, strlen(bus_name) + 1);
  netlink_attr(DEVLINK_ATTR_DEV_NAME, dev_name, strlen(dev_name) + 1);
  netlink_attr(DEVLINK_ATTR_NETNS_FD, &netns_fd, sizeof(netns_fd));
  netlink_send(sock);
error:
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
}

static int inject_fault(int nth)
{
  int fd;
  fd = open("/proc/thread-self/fail-nth", O_RDWR);
  if (fd == -1)
    exit(1);
  char buf[16];
  sprintf(buf, "%d", nth + 1);
  if (write(fd, buf, strlen(buf)) != (ssize_t)strlen(buf))
    exit(1);
  return fd;
}

static void setup_fault()
{
  static struct {
    const char* file;
    const char* val;
    bool fatal;
  } files[] = {
      {"/sys/kernel/debug/failslab/ignore-gfp-wait", "N", true},
      {"/sys/kernel/debug/fail_futex/ignore-private", "N", false},
      {"/sys/kernel/debug/fail_page_alloc/ignore-gfp-highmem", "N", false},
      {"/sys/kernel/debug/fail_page_alloc/ignore-gfp-wait", "N", false},
      {"/sys/kernel/debug/fail_page_alloc/min-order", "0", false},
  };
  unsigned i;
  for (i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
    if (!write_file(files[i].file, files[i].val)) {
      if (files[i].fatal)
        exit(1);
    }
  }
}

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  setup_fault();
  intptr_t res = 0;
  memcpy((void*)0x20000080, "./bus\000", 6);
  res = syscall(__NR_creat, 0x20000080, 0);
  if (res != -1)
    r[0] = res;
  syscall(__NR_lseek, r[0], 0x7ffffc, 0);
  res = syscall(__NR_creat, 0, 0);
  if (res != -1)
    r[1] = res;
  syscall(__NR_ioctl, r[1], 0x40086602, 0);
  res = syscall(__NR_creat, 0, 0);
  if (res != -1)
    r[2] = res;
  syscall(__NR_lseek, r[2], 0x7ffffe, 0);
  syscall(__NR_write, r[2], 0, 0);
  syscall(__NR_fallocate, r[2], 0x100000003, 0, 0x28120001);
  *(uint32_t*)0x20000380 = 0;
  *(uint16_t*)0x20000384 = 0x18;
  *(uint16_t*)0x20000386 = 0xfa00;
  *(uint64_t*)0x20000388 = 0;
  *(uint64_t*)0x20000390 = 0;
  *(uint16_t*)0x20000398 = 0;
  *(uint8_t*)0x2000039a = 0;
  *(uint8_t*)0x2000039b = 0;
  *(uint8_t*)0x2000039c = 0;
  *(uint8_t*)0x2000039d = 0;
  *(uint8_t*)0x2000039e = 0;
  *(uint8_t*)0x2000039f = 0;
  syscall(__NR_write, r[0], 0x20000380, 0x20);
  inject_fault(2);
  syscall(__NR_fallocate, r[0], 0x100000003, 0, 0x28120001);
  return 0;
}
