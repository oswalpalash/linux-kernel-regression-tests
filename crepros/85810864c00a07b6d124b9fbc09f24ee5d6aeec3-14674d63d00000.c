// https://syzkaller.appspot.com/bug?id=85810864c00a07b6d124b9fbc09f24ee5d6aeec3
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
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
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <linux/capability.h>
#include <linux/genetlink.h>
#include <linux/if_addr.h>
#include <linux/if_ether.h>
#include <linux/if_link.h>
#include <linux/in6.h>
#include <linux/neighbour.h>
#include <linux/net.h>
#include <linux/netlink.h>
#include <linux/nl80211.h>
#include <linux/rfkill.h>
#include <linux/rtnetlink.h>
#include <linux/veth.h>

static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* ctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  int skip = __atomic_load_n(&skip_segv, __ATOMIC_RELAXED) != 0;
  int valid = addr < prog_start || addr > prog_end;
  if (skip && valid) {
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
  ({                                                                           \
    int ok = 1;                                                                \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    if (_setjmp(segv_env) == 0) {                                              \
      __VA_ARGS__;                                                             \
    } else                                                                     \
      ok = 0;                                                                  \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    ok;                                                                        \
  })

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

#define WIFI_INITIAL_DEVICE_COUNT 2
#define WIFI_MAC_BASE                                                          \
  {                                                                            \
    0x08, 0x02, 0x11, 0x00, 0x00, 0x00                                         \
  }
#define WIFI_IBSS_BSSID                                                        \
  {                                                                            \
    0x50, 0x50, 0x50, 0x50, 0x50, 0x50                                         \
  }
#define WIFI_IBSS_SSID                                                         \
  {                                                                            \
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10                                         \
  }
#define WIFI_DEFAULT_FREQUENCY 2412
#define WIFI_DEFAULT_SIGNAL 0
#define WIFI_DEFAULT_RX_RATE 1
#define HWSIM_CMD_REGISTER 1
#define HWSIM_CMD_FRAME 2
#define HWSIM_CMD_NEW_RADIO 4
#define HWSIM_ATTR_SUPPORT_P2P_DEVICE 14
#define HWSIM_ATTR_PERM_ADDR 22

#define IF_OPER_UP 6
struct join_ibss_props {
  int wiphy_freq;
  bool wiphy_freq_fixed;
  uint8_t* mac;
  uint8_t* ssid;
  int ssid_len;
};

static int set_interface_state(const char* interface_name, int on)
{
  struct ifreq ifr;
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    return -1;
  }
  memset(&ifr, 0, sizeof(ifr));
  strcpy(ifr.ifr_name, interface_name);
  int ret = ioctl(sock, SIOCGIFFLAGS, &ifr);
  if (ret < 0) {
    close(sock);
    return -1;
  }
  if (on)
    ifr.ifr_flags |= IFF_UP;
  else
    ifr.ifr_flags &= ~IFF_UP;
  ret = ioctl(sock, SIOCSIFFLAGS, &ifr);
  close(sock);
  if (ret < 0) {
    return -1;
  }
  return 0;
}

static int nl80211_set_interface(struct nlmsg* nlmsg, int sock,
                                 int nl80211_family, uint32_t ifindex,
                                 uint32_t iftype)
{
  struct genlmsghdr genlhdr;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = NL80211_CMD_SET_INTERFACE;
  netlink_init(nlmsg, nl80211_family, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(nlmsg, NL80211_ATTR_IFINDEX, &ifindex, sizeof(ifindex));
  netlink_attr(nlmsg, NL80211_ATTR_IFTYPE, &iftype, sizeof(iftype));
  int err = netlink_send(nlmsg, sock);
  if (err < 0) {
  }
  return err;
}

static int nl80211_join_ibss(struct nlmsg* nlmsg, int sock, int nl80211_family,
                             uint32_t ifindex, struct join_ibss_props* props)
{
  struct genlmsghdr genlhdr;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = NL80211_CMD_JOIN_IBSS;
  netlink_init(nlmsg, nl80211_family, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(nlmsg, NL80211_ATTR_IFINDEX, &ifindex, sizeof(ifindex));
  netlink_attr(nlmsg, NL80211_ATTR_SSID, props->ssid, props->ssid_len);
  netlink_attr(nlmsg, NL80211_ATTR_WIPHY_FREQ, &(props->wiphy_freq),
               sizeof(props->wiphy_freq));
  if (props->mac)
    netlink_attr(nlmsg, NL80211_ATTR_MAC, props->mac, ETH_ALEN);
  if (props->wiphy_freq_fixed)
    netlink_attr(nlmsg, NL80211_ATTR_FREQ_FIXED, NULL, 0);
  int err = netlink_send(nlmsg, sock);
  if (err < 0) {
  }
  return err;
}

static int get_ifla_operstate(struct nlmsg* nlmsg, int ifindex)
{
  struct ifinfomsg info;
  memset(&info, 0, sizeof(info));
  info.ifi_family = AF_UNSPEC;
  info.ifi_index = ifindex;
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sock == -1) {
    return -1;
  }
  netlink_init(nlmsg, RTM_GETLINK, 0, &info, sizeof(info));
  int n;
  int err = netlink_send_ext(nlmsg, sock, RTM_NEWLINK, &n, true);
  close(sock);
  if (err) {
    return -1;
  }
  struct rtattr* attr = IFLA_RTA(NLMSG_DATA(nlmsg->buf));
  for (; RTA_OK(attr, n); attr = RTA_NEXT(attr, n)) {
    if (attr->rta_type == IFLA_OPERSTATE)
      return *((int32_t*)RTA_DATA(attr));
  }
  return -1;
}

static int await_ifla_operstate(struct nlmsg* nlmsg, char* interface,
                                int operstate)
{
  int ifindex = if_nametoindex(interface);
  while (true) {
    usleep(1000);
    int ret = get_ifla_operstate(nlmsg, ifindex);
    if (ret < 0)
      return ret;
    if (ret == operstate)
      return 0;
  }
  return 0;
}

static int nl80211_setup_ibss_interface(struct nlmsg* nlmsg, int sock,
                                        int nl80211_family_id, char* interface,
                                        struct join_ibss_props* ibss_props)
{
  int ifindex = if_nametoindex(interface);
  if (ifindex == 0) {
    return -1;
  }
  int ret = nl80211_set_interface(nlmsg, sock, nl80211_family_id, ifindex,
                                  NL80211_IFTYPE_ADHOC);
  if (ret < 0) {
    return -1;
  }
  ret = set_interface_state(interface, 1);
  if (ret < 0) {
    return -1;
  }
  ret = nl80211_join_ibss(nlmsg, sock, nl80211_family_id, ifindex, ibss_props);
  if (ret < 0) {
    return -1;
  }
  return 0;
}

static int hwsim80211_create_device(struct nlmsg* nlmsg, int sock,
                                    int hwsim_family,
                                    uint8_t mac_addr[ETH_ALEN])
{
  struct genlmsghdr genlhdr;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = HWSIM_CMD_NEW_RADIO;
  netlink_init(nlmsg, hwsim_family, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(nlmsg, HWSIM_ATTR_SUPPORT_P2P_DEVICE, NULL, 0);
  netlink_attr(nlmsg, HWSIM_ATTR_PERM_ADDR, mac_addr, ETH_ALEN);
  int err = netlink_send(nlmsg, sock);
  if (err < 0) {
  }
  return err;
}

static void initialize_wifi_devices(void)
{
  int rfkill = open("/dev/rfkill", O_RDWR);
  if (rfkill == -1) {
    if (errno != ENOENT && errno != EACCES)
      exit(1);
  } else {
    struct rfkill_event event = {0};
    event.type = RFKILL_TYPE_ALL;
    event.op = RFKILL_OP_CHANGE_ALL;
    if (write(rfkill, &event, sizeof(event)) != (ssize_t)(sizeof(event)))
      exit(1);
    close(rfkill);
  }
  uint8_t mac_addr[6] = WIFI_MAC_BASE;
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (sock < 0) {
    return;
  }
  int hwsim_family_id =
      netlink_query_family_id(&nlmsg, sock, "MAC80211_HWSIM", true);
  int nl80211_family_id =
      netlink_query_family_id(&nlmsg, sock, "nl80211", true);
  uint8_t ssid[] = WIFI_IBSS_SSID;
  uint8_t bssid[] = WIFI_IBSS_BSSID;
  struct join_ibss_props ibss_props = {.wiphy_freq = WIFI_DEFAULT_FREQUENCY,
                                       .wiphy_freq_fixed = true,
                                       .mac = bssid,
                                       .ssid = ssid,
                                       .ssid_len = sizeof(ssid)};
  for (int device_id = 0; device_id < WIFI_INITIAL_DEVICE_COUNT; device_id++) {
    mac_addr[5] = device_id;
    int ret = hwsim80211_create_device(&nlmsg, sock, hwsim_family_id, mac_addr);
    if (ret < 0)
      exit(1);
    char interface[6] = "wlan0";
    interface[4] += device_id;
    if (nl80211_setup_ibss_interface(&nlmsg, sock, nl80211_family_id, interface,
                                     &ibss_props) < 0)
      exit(1);
  }
  for (int device_id = 0; device_id < WIFI_INITIAL_DEVICE_COUNT; device_id++) {
    char interface[6] = "wlan0";
    interface[4] += device_id;
    int ret = await_ifla_operstate(&nlmsg, interface, IF_OPER_UP);
    if (ret < 0)
      exit(1);
  }
  close(sock);
}

#define SIZEOF_IO_URING_SQE 64
#define SIZEOF_IO_URING_CQE 16
#define SQ_HEAD_OFFSET 0
#define SQ_TAIL_OFFSET 64
#define SQ_RING_MASK_OFFSET 256
#define SQ_RING_ENTRIES_OFFSET 264
#define SQ_FLAGS_OFFSET 276
#define SQ_DROPPED_OFFSET 272
#define CQ_HEAD_OFFSET 128
#define CQ_TAIL_OFFSET 192
#define CQ_RING_MASK_OFFSET 260
#define CQ_RING_ENTRIES_OFFSET 268
#define CQ_RING_OVERFLOW_OFFSET 284
#define CQ_FLAGS_OFFSET 280
#define CQ_CQES_OFFSET 320

struct io_sqring_offsets {
  uint32_t head;
  uint32_t tail;
  uint32_t ring_mask;
  uint32_t ring_entries;
  uint32_t flags;
  uint32_t dropped;
  uint32_t array;
  uint32_t resv1;
  uint64_t resv2;
};

struct io_cqring_offsets {
  uint32_t head;
  uint32_t tail;
  uint32_t ring_mask;
  uint32_t ring_entries;
  uint32_t overflow;
  uint32_t cqes;
  uint64_t resv[2];
};

struct io_uring_params {
  uint32_t sq_entries;
  uint32_t cq_entries;
  uint32_t flags;
  uint32_t sq_thread_cpu;
  uint32_t sq_thread_idle;
  uint32_t features;
  uint32_t resv[4];
  struct io_sqring_offsets sq_off;
  struct io_cqring_offsets cq_off;
};

#define IORING_OFF_SQ_RING 0
#define IORING_OFF_SQES 0x10000000ULL

#define sys_io_uring_setup 425
static long syz_io_uring_setup(volatile long a0, volatile long a1,
                               volatile long a2, volatile long a3,
                               volatile long a4, volatile long a5)
{
  uint32_t entries = (uint32_t)a0;
  struct io_uring_params* setup_params = (struct io_uring_params*)a1;
  void* vma1 = (void*)a2;
  void* vma2 = (void*)a3;
  void** ring_ptr_out = (void**)a4;
  void** sqes_ptr_out = (void**)a5;
  uint32_t fd_io_uring = syscall(sys_io_uring_setup, entries, setup_params);
  uint32_t sq_ring_sz =
      setup_params->sq_off.array + setup_params->sq_entries * sizeof(uint32_t);
  uint32_t cq_ring_sz = setup_params->cq_off.cqes +
                        setup_params->cq_entries * SIZEOF_IO_URING_CQE;
  uint32_t ring_sz = sq_ring_sz > cq_ring_sz ? sq_ring_sz : cq_ring_sz;
  *ring_ptr_out = mmap(vma1, ring_sz, PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_POPULATE | MAP_FIXED, fd_io_uring,
                       IORING_OFF_SQ_RING);
  uint32_t sqes_sz = setup_params->sq_entries * SIZEOF_IO_URING_SQE;
  *sqes_ptr_out =
      mmap(vma2, sqes_sz, PROT_READ | PROT_WRITE,
           MAP_SHARED | MAP_POPULATE | MAP_FIXED, fd_io_uring, IORING_OFF_SQES);
  return fd_io_uring;
}

static long syz_io_uring_submit(volatile long a0, volatile long a1,
                                volatile long a2, volatile long a3)
{
  char* ring_ptr = (char*)a0;
  char* sqes_ptr = (char*)a1;
  char* sqe = (char*)a2;
  uint32_t sqes_index = (uint32_t)a3;
  uint32_t sq_ring_entries = *(uint32_t*)(ring_ptr + SQ_RING_ENTRIES_OFFSET);
  uint32_t cq_ring_entries = *(uint32_t*)(ring_ptr + CQ_RING_ENTRIES_OFFSET);
  uint32_t sq_array_off =
      (CQ_CQES_OFFSET + cq_ring_entries * SIZEOF_IO_URING_CQE + 63) & ~63;
  if (sq_ring_entries)
    sqes_index %= sq_ring_entries;
  char* sqe_dest = sqes_ptr + sqes_index * SIZEOF_IO_URING_SQE;
  memcpy(sqe_dest, sqe, SIZEOF_IO_URING_SQE);
  uint32_t sq_ring_mask = *(uint32_t*)(ring_ptr + SQ_RING_MASK_OFFSET);
  uint32_t* sq_tail_ptr = (uint32_t*)(ring_ptr + SQ_TAIL_OFFSET);
  uint32_t sq_tail = *sq_tail_ptr & sq_ring_mask;
  uint32_t sq_tail_next = *sq_tail_ptr + 1;
  uint32_t* sq_array = (uint32_t*)(ring_ptr + sq_array_off);
  *(sq_array + sq_tail) = sqes_index;
  __atomic_store_n(sq_tail_ptr, sq_tail_next, __ATOMIC_RELEASE);
  return 0;
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
  if (unshare(CLONE_NEWNET)) {
  }
  initialize_wifi_devices();
  loop();
  exit(1);
}

static void setup_usb()
{
  if (chmod("/dev/raw-gadget", 0666))
    exit(1);
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
      {"/proc/sys/vm/nr_overcommit_hugepages", "4"},
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

#ifndef __NR_io_uring_enter
#define __NR_io_uring_enter 426
#endif

uint64_t r[4] = {0xffffffffffffffff, 0x0, 0x0, 0xffffffffffffffff};

void loop(void)
{
  intptr_t res = 0;
  NONFAILING(*(uint32_t*)0x20000184 = 0);
  NONFAILING(*(uint32_t*)0x20000188 = 0);
  NONFAILING(*(uint32_t*)0x2000018c = 3);
  NONFAILING(*(uint32_t*)0x20000190 = 0);
  NONFAILING(*(uint32_t*)0x20000198 = -1);
  NONFAILING(memset((void*)0x2000019c, 0, 12));
  res = -1;
  NONFAILING(res = syz_io_uring_setup(0x86, 0x20000180, 0x20ffc000, 0x206d4000,
                                      0x20000040, 0x20000080));
  if (res != -1) {
    r[0] = res;
    NONFAILING(r[1] = *(uint64_t*)0x20000040);
    NONFAILING(r[2] = *(uint64_t*)0x20000080);
  }
  NONFAILING(memcpy((void*)0x20000280, "/dev/dsp\000", 9));
  res =
      syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000280ul, 0x2a402ul, 0ul);
  if (res != -1)
    r[3] = res;
  NONFAILING(*(uint8_t*)0x20000200 = 0x16);
  NONFAILING(*(uint8_t*)0x20000201 = 0);
  NONFAILING(*(uint16_t*)0x20000202 = 0);
  NONFAILING(*(uint32_t*)0x20000204 = r[3]);
  NONFAILING(*(uint64_t*)0x20000208 = 0);
  NONFAILING(*(uint64_t*)0x20000210 = 0);
  NONFAILING(*(uint32_t*)0x20000218 = 0);
  NONFAILING(*(uint32_t*)0x2000021c = 0);
  NONFAILING(*(uint64_t*)0x20000220 = 0);
  NONFAILING(*(uint16_t*)0x20000228 = 0);
  NONFAILING(*(uint16_t*)0x2000022a = 0);
  NONFAILING(memset((void*)0x2000022c, 0, 20));
  NONFAILING(syz_io_uring_submit(r[1], r[2], 0x20000200, 0));
  syscall(__NR_io_uring_enter, r[0], 0x2039, 0, 0ul, 0ul, 0ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  setup_sysctl();
  setup_usb();
  setup_802154();
  install_segv_handler();
  use_temporary_dir();
  do_sandbox_none();
  return 0;
}
