// https://syzkaller.appspot.com/bug?id=6c9d311496cefc7798dec6719880613a8188880d
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

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

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

static struct nlmsg nlmsg;

static long syz_genetlink_get_family_id(volatile long name,
                                        volatile long sock_arg)
{
  bool dofail = false;
  int fd = sock_arg;
  if (fd < 0) {
    dofail = true;
    fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (fd == -1) {
      return -1;
    }
  }
  struct nlmsg nlmsg_tmp;
  int ret = netlink_query_family_id(&nlmsg_tmp, fd, (char*)name, dofail);
  if ((int)sock_arg < 0)
    close(fd);
  if (ret < 0) {
    return -1;
  }
  return ret;
}

static int inject_fault(int nth)
{
  int fd;
  fd = open("/proc/thread-self/fail-nth", O_RDWR);
  if (fd == -1)
    exit(1);
  char buf[16];
  sprintf(buf, "%d", nth);
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

static void setup_sysctl()
{
  char mypid[32];
  snprintf(mypid, sizeof(mypid), "%d", getpid());
  struct {
    const char* name;
    const char* data;
  } files[] = {
      {"/sys/kernel/debug/x86/nmi_longest_ns", "10000000000"},
      {"/proc/sys/kernel/hung_task_check_interval_secs", "20"},
      {"/proc/sys/net/core/bpf_jit_kallsyms", "1"},
      {"/proc/sys/net/core/bpf_jit_harden", "0"},
      {"/proc/sys/kernel/kptr_restrict", "0"},
      {"/proc/sys/kernel/softlockup_all_cpu_backtrace", "1"},
      {"/proc/sys/fs/mount-max", "100"},
      {"/proc/sys/vm/oom_dump_tasks", "0"},
      {"/proc/sys/debug/exception-trace", "0"},
      {"/proc/sys/kernel/printk", "7 4 1 3"},
      {"/proc/sys/net/ipv4/ping_group_range", "0 65535"},
      {"/proc/sys/kernel/keys/gc_delay", "1"},
      {"/proc/sys/vm/oom_kill_allocating_task", "1"},
      {"/proc/sys/kernel/ctrl-alt-del", "0"},
      {"/proc/sys/kernel/cad_pid", mypid},
  };
  for (size_t i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
    if (!write_file(files[i].name, files[i].data))
      printf("write to %s failed: %s\n", files[i].name, strerror(errno));
  }
}

#define NL802154_CMD_SET_SHORT_ADDR 11
#define NL802154_ATTR_IFINDEX 3
#define NL802154_ATTR_SHORT_ADDR 10

static void setup_802154()
{
  int sock_route = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sock_route == -1)
    exit(1);
  int sock_generic = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (sock_generic < 0)
    exit(1);
  int nl802154_family_id =
      netlink_query_family_id(&nlmsg, sock_generic, "nl802154", true);
  for (int i = 0; i < 2; i++) {
    char devname[] = "wpan0";
    devname[strlen(devname) - 1] += i;
    uint64_t hwaddr = 0xaaaaaaaaaaaa0002 + (i << 8);
    uint16_t shortaddr = 0xaaa0 + i;
    int ifindex = if_nametoindex(devname);
    struct genlmsghdr genlhdr;
    memset(&genlhdr, 0, sizeof(genlhdr));
    genlhdr.cmd = NL802154_CMD_SET_SHORT_ADDR;
    netlink_init(&nlmsg, nl802154_family_id, 0, &genlhdr, sizeof(genlhdr));
    netlink_attr(&nlmsg, NL802154_ATTR_IFINDEX, &ifindex, sizeof(ifindex));
    netlink_attr(&nlmsg, NL802154_ATTR_SHORT_ADDR, &shortaddr,
                 sizeof(shortaddr));
    int err = netlink_send(&nlmsg, sock_generic);
    if (err < 0) {
    }
    netlink_device_change(&nlmsg, sock_route, devname, true, 0, &hwaddr,
                          sizeof(hwaddr), 0);
    if (i == 0) {
      netlink_add_device_impl(&nlmsg, "lowpan", "lowpan0");
      netlink_done(&nlmsg);
      netlink_attr(&nlmsg, IFLA_LINK, &ifindex, sizeof(ifindex));
      int err = netlink_send(&nlmsg, sock_route);
      if (err < 0) {
      }
    }
  }
  close(sock_route);
  close(sock_generic);
}

uint64_t r[3] = {0xffffffffffffffff, 0x0, 0x0};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  setup_sysctl();
  setup_fault();
  setup_802154();
  use_temporary_dir();
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0x10);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000080, "nl80211\000", 8);
  res = -1;
  res = syz_genetlink_get_family_id(0x20000080, -1);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x200000c0, "wlan0\000\000\000\000\000\000\000\000\000\000\000",
         16);
  res = syscall(__NR_ioctl, r[0], 0x8933, 0x200000c0ul);
  if (res != -1)
    r[2] = *(uint32_t*)0x200000d0;
  *(uint64_t*)0x20000100 = 0;
  *(uint32_t*)0x20000108 = 0;
  *(uint64_t*)0x20000110 = 0x20000140;
  *(uint64_t*)0x20000140 = 0x20000180;
  *(uint32_t*)0x20000180 = 0x24;
  *(uint16_t*)0x20000184 = r[1];
  *(uint16_t*)0x20000186 = 5;
  *(uint32_t*)0x20000188 = 0;
  *(uint32_t*)0x2000018c = 0;
  *(uint8_t*)0x20000190 = 6;
  *(uint8_t*)0x20000191 = 0;
  *(uint16_t*)0x20000192 = 0;
  *(uint16_t*)0x20000194 = 8;
  *(uint16_t*)0x20000196 = 3;
  *(uint32_t*)0x20000198 = r[2];
  *(uint16_t*)0x2000019c = 8;
  *(uint16_t*)0x2000019e = 5;
  *(uint32_t*)0x200001a0 = 3;
  *(uint64_t*)0x20000148 = 0x24;
  *(uint64_t*)0x20000118 = 1;
  *(uint64_t*)0x20000120 = 0;
  *(uint64_t*)0x20000128 = 0;
  *(uint32_t*)0x20000130 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000100ul, 0ul);
  *(uint64_t*)0x200001c0 = 0;
  *(uint32_t*)0x200001c8 = 0;
  *(uint64_t*)0x200001d0 = 0x20000200;
  *(uint64_t*)0x20000200 = 0x20000240;
  *(uint32_t*)0x20000240 = 0x74;
  *(uint16_t*)0x20000244 = r[1];
  *(uint16_t*)0x20000246 = 5;
  *(uint32_t*)0x20000248 = 0;
  *(uint32_t*)0x2000024c = 0;
  *(uint8_t*)0x20000250 = 0xf;
  *(uint8_t*)0x20000251 = 0;
  *(uint16_t*)0x20000252 = 0;
  *(uint16_t*)0x20000254 = 8;
  *(uint16_t*)0x20000256 = 3;
  *(uint32_t*)0x20000258 = r[2];
  *(uint16_t*)0x2000025c = 0x28;
  *(uint16_t*)0x2000025e = 0xe;
  STORE_BY_BITMASK(uint8_t, , 0x20000260, 0, 0, 2);
  STORE_BY_BITMASK(uint8_t, , 0x20000260, 0, 2, 2);
  STORE_BY_BITMASK(uint8_t, , 0x20000260, 8, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000261, 0, 0, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000261, 0, 1, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000261, 0, 2, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000261, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000261, 0, 4, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000261, 0, 5, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000261, 0, 6, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000261, 0, 7, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000262, 0, 0, 15);
  STORE_BY_BITMASK(uint16_t, , 0x20000263, 0, 7, 1);
  memset((void*)0x20000264, 255, 6);
  *(uint8_t*)0x2000026a = 8;
  *(uint8_t*)0x2000026b = 2;
  *(uint8_t*)0x2000026c = 0x11;
  *(uint8_t*)0x2000026d = 0;
  *(uint8_t*)0x2000026e = 0;
  *(uint8_t*)0x2000026f = 0;
  *(uint8_t*)0x20000270 = 8;
  *(uint8_t*)0x20000271 = 2;
  *(uint8_t*)0x20000272 = 0x11;
  *(uint8_t*)0x20000273 = 0;
  *(uint8_t*)0x20000274 = 0;
  *(uint8_t*)0x20000275 = 0;
  STORE_BY_BITMASK(uint16_t, , 0x20000276, 0, 0, 4);
  STORE_BY_BITMASK(uint16_t, , 0x20000276, 0, 4, 12);
  *(uint64_t*)0x20000278 = 0;
  *(uint16_t*)0x20000280 = 0x64;
  *(uint16_t*)0x20000282 = 1;
  *(uint16_t*)0x20000284 = 8;
  *(uint16_t*)0x20000286 = 0x26;
  *(uint32_t*)0x20000288 = 0x96c;
  *(uint16_t*)0x2000028c = 8;
  *(uint16_t*)0x2000028e = 0xc;
  *(uint32_t*)0x20000290 = 0x64;
  *(uint16_t*)0x20000294 = 8;
  *(uint16_t*)0x20000296 = 0xd;
  *(uint32_t*)0x20000298 = 0;
  *(uint16_t*)0x2000029c = 0xa;
  *(uint16_t*)0x2000029e = 0x34;
  memset((void*)0x200002a0, 2, 6);
  *(uint16_t*)0x200002a8 = 8;
  *(uint16_t*)0x200002aa = 0x35;
  *(uint32_t*)0x200002ac = 0;
  *(uint16_t*)0x200002b0 = 4;
  *(uint16_t*)0x200002b2 = 0x105;
  *(uint64_t*)0x20000208 = 0x74;
  *(uint64_t*)0x200001d8 = 1;
  *(uint64_t*)0x200001e0 = 0;
  *(uint64_t*)0x200001e8 = 0;
  *(uint32_t*)0x200001f0 = 0;
  inject_fault(10);
  syscall(__NR_sendmsg, r[0], 0x200001c0ul, 0ul);
  return 0;
}
