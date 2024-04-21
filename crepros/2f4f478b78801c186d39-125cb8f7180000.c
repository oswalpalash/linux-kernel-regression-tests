// https://syzkaller.appspot.com/bug?id=c5fe17a9710b37c446e73ecf88adae7ae930105a
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sched.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
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
#include <time.h>
#include <unistd.h>

#include <linux/capability.h>
#include <linux/futex.h>
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

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

static unsigned long long procid;

static __thread int clone_ongoing;
static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* ctx)
{
  if (__atomic_load_n(&clone_ongoing, __ATOMIC_RELAXED) != 0) {
    exit(sig);
  }
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

static void sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
}

static uint64_t current_time_ms(void)
{
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    exit(1);
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void thread_start(void* (*fn)(void*), void* arg)
{
  pthread_t th;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 128 << 10);
  int i = 0;
  for (; i < 100; i++) {
    if (pthread_create(&th, &attr, fn, arg) == 0) {
      pthread_attr_destroy(&attr);
      return;
    }
    if (errno == EAGAIN) {
      usleep(50);
      continue;
    }
    break;
  }
  exit(1);
}

typedef struct {
  int state;
} event_t;

static void event_init(event_t* ev)
{
  ev->state = 0;
}

static void event_reset(event_t* ev)
{
  ev->state = 0;
}

static void event_set(event_t* ev)
{
  if (ev->state)
    exit(1);
  __atomic_store_n(&ev->state, 1, __ATOMIC_RELEASE);
  syscall(SYS_futex, &ev->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1000000);
}

static void event_wait(event_t* ev)
{
  while (!__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, 0);
}

static int event_isset(event_t* ev)
{
  return __atomic_load_n(&ev->state, __ATOMIC_ACQUIRE);
}

static int event_timedwait(event_t* ev, uint64_t timeout)
{
  uint64_t start = current_time_ms();
  uint64_t now = start;
  for (;;) {
    uint64_t remain = timeout - (now - start);
    struct timespec ts;
    ts.tv_sec = remain / 1000;
    ts.tv_nsec = (remain % 1000) * 1000 * 1000;
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, &ts);
    if (__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
      return 1;
    now = current_time_ms();
    if (now - start > timeout)
      return 0;
  }
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
                                 uint32_t iftype, bool dofail)
{
  struct genlmsghdr genlhdr;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = NL80211_CMD_SET_INTERFACE;
  netlink_init(nlmsg, nl80211_family, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(nlmsg, NL80211_ATTR_IFINDEX, &ifindex, sizeof(ifindex));
  netlink_attr(nlmsg, NL80211_ATTR_IFTYPE, &iftype, sizeof(iftype));
  int err = netlink_send_ext(nlmsg, sock, 0, NULL, dofail);
  if (err < 0) {
  }
  return err;
}

static int nl80211_join_ibss(struct nlmsg* nlmsg, int sock, int nl80211_family,
                             uint32_t ifindex, struct join_ibss_props* props,
                             bool dofail)
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
  int err = netlink_send_ext(nlmsg, sock, 0, NULL, dofail);
  if (err < 0) {
  }
  return err;
}

static int get_ifla_operstate(struct nlmsg* nlmsg, int ifindex, bool dofail)
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
  int err = netlink_send_ext(nlmsg, sock, RTM_NEWLINK, &n, dofail);
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
                                int operstate, bool dofail)
{
  int ifindex = if_nametoindex(interface);
  while (true) {
    usleep(1000);
    int ret = get_ifla_operstate(nlmsg, ifindex, dofail);
    if (ret < 0)
      return ret;
    if (ret == operstate)
      return 0;
  }
  return 0;
}

static int nl80211_setup_ibss_interface(struct nlmsg* nlmsg, int sock,
                                        int nl80211_family_id, char* interface,
                                        struct join_ibss_props* ibss_props,
                                        bool dofail)
{
  int ifindex = if_nametoindex(interface);
  if (ifindex == 0) {
    return -1;
  }
  int ret = nl80211_set_interface(nlmsg, sock, nl80211_family_id, ifindex,
                                  NL80211_IFTYPE_ADHOC, dofail);
  if (ret < 0) {
    return -1;
  }
  ret = set_interface_state(interface, 1);
  if (ret < 0) {
    return -1;
  }
  ret = nl80211_join_ibss(nlmsg, sock, nl80211_family_id, ifindex, ibss_props,
                          dofail);
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
                                     &ibss_props, true) < 0)
      exit(1);
  }
  for (int device_id = 0; device_id < WIFI_INITIAL_DEVICE_COUNT; device_id++) {
    char interface[6] = "wlan0";
    interface[4] += device_id;
    int ret = await_ifla_operstate(&nlmsg, interface, IF_OPER_UP, true);
    if (ret < 0)
      exit(1);
  }
  close(sock);
}

#define MAX_FDS 30

#define BTPROTO_HCI 1
#define ACL_LINK 1
#define SCAN_PAGE 2

typedef struct {
  uint8_t b[6];
} __attribute__((packed)) bdaddr_t;

#define HCI_COMMAND_PKT 1
#define HCI_EVENT_PKT 4
#define HCI_VENDOR_PKT 0xff

struct hci_command_hdr {
  uint16_t opcode;
  uint8_t plen;
} __attribute__((packed));

struct hci_event_hdr {
  uint8_t evt;
  uint8_t plen;
} __attribute__((packed));

#define HCI_EV_CONN_COMPLETE 0x03
struct hci_ev_conn_complete {
  uint8_t status;
  uint16_t handle;
  bdaddr_t bdaddr;
  uint8_t link_type;
  uint8_t encr_mode;
} __attribute__((packed));

#define HCI_EV_CONN_REQUEST 0x04
struct hci_ev_conn_request {
  bdaddr_t bdaddr;
  uint8_t dev_class[3];
  uint8_t link_type;
} __attribute__((packed));

#define HCI_EV_REMOTE_FEATURES 0x0b
struct hci_ev_remote_features {
  uint8_t status;
  uint16_t handle;
  uint8_t features[8];
} __attribute__((packed));

#define HCI_EV_CMD_COMPLETE 0x0e
struct hci_ev_cmd_complete {
  uint8_t ncmd;
  uint16_t opcode;
} __attribute__((packed));

#define HCI_OP_WRITE_SCAN_ENABLE 0x0c1a

#define HCI_OP_READ_BUFFER_SIZE 0x1005
struct hci_rp_read_buffer_size {
  uint8_t status;
  uint16_t acl_mtu;
  uint8_t sco_mtu;
  uint16_t acl_max_pkt;
  uint16_t sco_max_pkt;
} __attribute__((packed));

#define HCI_OP_READ_BD_ADDR 0x1009
struct hci_rp_read_bd_addr {
  uint8_t status;
  bdaddr_t bdaddr;
} __attribute__((packed));

#define HCI_EV_LE_META 0x3e
struct hci_ev_le_meta {
  uint8_t subevent;
} __attribute__((packed));

#define HCI_EV_LE_CONN_COMPLETE 0x01
struct hci_ev_le_conn_complete {
  uint8_t status;
  uint16_t handle;
  uint8_t role;
  uint8_t bdaddr_type;
  bdaddr_t bdaddr;
  uint16_t interval;
  uint16_t latency;
  uint16_t supervision_timeout;
  uint8_t clk_accurancy;
} __attribute__((packed));

struct hci_dev_req {
  uint16_t dev_id;
  uint32_t dev_opt;
};

struct vhci_vendor_pkt_request {
  uint8_t type;
  uint8_t opcode;
} __attribute__((packed));

struct vhci_pkt {
  uint8_t type;
  union {
    struct {
      uint8_t opcode;
      uint16_t id;
    } __attribute__((packed)) vendor_pkt;
    struct hci_command_hdr command_hdr;
  };
} __attribute__((packed));

#define HCIDEVUP _IOW('H', 201, int)
#define HCISETSCAN _IOW('H', 221, int)

static int vhci_fd = -1;

static void rfkill_unblock_all()
{
  int fd = open("/dev/rfkill", O_WRONLY);
  if (fd < 0)
    exit(1);
  struct rfkill_event event = {0};
  event.idx = 0;
  event.type = RFKILL_TYPE_ALL;
  event.op = RFKILL_OP_CHANGE_ALL;
  event.soft = 0;
  event.hard = 0;
  if (write(fd, &event, sizeof(event)) < 0)
    exit(1);
  close(fd);
}

static void hci_send_event_packet(int fd, uint8_t evt, void* data,
                                  size_t data_len)
{
  struct iovec iv[3];
  struct hci_event_hdr hdr;
  hdr.evt = evt;
  hdr.plen = data_len;
  uint8_t type = HCI_EVENT_PKT;
  iv[0].iov_base = &type;
  iv[0].iov_len = sizeof(type);
  iv[1].iov_base = &hdr;
  iv[1].iov_len = sizeof(hdr);
  iv[2].iov_base = data;
  iv[2].iov_len = data_len;
  if (writev(fd, iv, sizeof(iv) / sizeof(struct iovec)) < 0)
    exit(1);
}

static void hci_send_event_cmd_complete(int fd, uint16_t opcode, void* data,
                                        size_t data_len)
{
  struct iovec iv[4];
  struct hci_event_hdr hdr;
  hdr.evt = HCI_EV_CMD_COMPLETE;
  hdr.plen = sizeof(struct hci_ev_cmd_complete) + data_len;
  struct hci_ev_cmd_complete evt_hdr;
  evt_hdr.ncmd = 1;
  evt_hdr.opcode = opcode;
  uint8_t type = HCI_EVENT_PKT;
  iv[0].iov_base = &type;
  iv[0].iov_len = sizeof(type);
  iv[1].iov_base = &hdr;
  iv[1].iov_len = sizeof(hdr);
  iv[2].iov_base = &evt_hdr;
  iv[2].iov_len = sizeof(evt_hdr);
  iv[3].iov_base = data;
  iv[3].iov_len = data_len;
  if (writev(fd, iv, sizeof(iv) / sizeof(struct iovec)) < 0)
    exit(1);
}

static bool process_command_pkt(int fd, char* buf, ssize_t buf_size)
{
  struct hci_command_hdr* hdr = (struct hci_command_hdr*)buf;
  if (buf_size < (ssize_t)sizeof(struct hci_command_hdr) ||
      hdr->plen != buf_size - sizeof(struct hci_command_hdr))
    exit(1);
  switch (hdr->opcode) {
  case HCI_OP_WRITE_SCAN_ENABLE: {
    uint8_t status = 0;
    hci_send_event_cmd_complete(fd, hdr->opcode, &status, sizeof(status));
    return true;
  }
  case HCI_OP_READ_BD_ADDR: {
    struct hci_rp_read_bd_addr rp = {0};
    rp.status = 0;
    memset(&rp.bdaddr, 0xaa, 6);
    hci_send_event_cmd_complete(fd, hdr->opcode, &rp, sizeof(rp));
    return false;
  }
  case HCI_OP_READ_BUFFER_SIZE: {
    struct hci_rp_read_buffer_size rp = {0};
    rp.status = 0;
    rp.acl_mtu = 1021;
    rp.sco_mtu = 96;
    rp.acl_max_pkt = 4;
    rp.sco_max_pkt = 6;
    hci_send_event_cmd_complete(fd, hdr->opcode, &rp, sizeof(rp));
    return false;
  }
  }
  char dummy[0xf9] = {0};
  hci_send_event_cmd_complete(fd, hdr->opcode, dummy, sizeof(dummy));
  return false;
}

static void* event_thread(void* arg)
{
  while (1) {
    char buf[1024] = {0};
    ssize_t buf_size = read(vhci_fd, buf, sizeof(buf));
    if (buf_size < 0)
      exit(1);
    if (buf_size > 0 && buf[0] == HCI_COMMAND_PKT) {
      if (process_command_pkt(vhci_fd, buf + 1, buf_size - 1))
        break;
    }
  }
  return NULL;
}
#define HCI_HANDLE_1 200
#define HCI_HANDLE_2 201

#define HCI_PRIMARY 0
#define HCI_OP_RESET 0x0c03

static void initialize_vhci()
{
  int hci_sock = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
  if (hci_sock < 0)
    exit(1);
  vhci_fd = open("/dev/vhci", O_RDWR);
  if (vhci_fd == -1)
    exit(1);
  const int kVhciFd = 202;
  if (dup2(vhci_fd, kVhciFd) < 0)
    exit(1);
  close(vhci_fd);
  vhci_fd = kVhciFd;
  struct vhci_vendor_pkt_request vendor_pkt_req = {HCI_VENDOR_PKT, HCI_PRIMARY};
  if (write(vhci_fd, &vendor_pkt_req, sizeof(vendor_pkt_req)) !=
      sizeof(vendor_pkt_req))
    exit(1);
  struct vhci_pkt vhci_pkt;
  if (read(vhci_fd, &vhci_pkt, sizeof(vhci_pkt)) != sizeof(vhci_pkt))
    exit(1);
  if (vhci_pkt.type == HCI_COMMAND_PKT &&
      vhci_pkt.command_hdr.opcode == HCI_OP_RESET) {
    char response[1] = {0};
    hci_send_event_cmd_complete(vhci_fd, HCI_OP_RESET, response,
                                sizeof(response));
    if (read(vhci_fd, &vhci_pkt, sizeof(vhci_pkt)) != sizeof(vhci_pkt))
      exit(1);
  }
  if (vhci_pkt.type != HCI_VENDOR_PKT)
    exit(1);
  int dev_id = vhci_pkt.vendor_pkt.id;
  pthread_t th;
  if (pthread_create(&th, NULL, event_thread, NULL))
    exit(1);
  int ret = ioctl(hci_sock, HCIDEVUP, dev_id);
  if (ret) {
    if (errno == ERFKILL) {
      rfkill_unblock_all();
      ret = ioctl(hci_sock, HCIDEVUP, dev_id);
    }
    if (ret && errno != EALREADY)
      exit(1);
  }
  struct hci_dev_req dr = {0};
  dr.dev_id = dev_id;
  dr.dev_opt = SCAN_PAGE;
  if (ioctl(hci_sock, HCISETSCAN, &dr))
    exit(1);
  struct hci_ev_conn_request request;
  memset(&request, 0, sizeof(request));
  memset(&request.bdaddr, 0xaa, 6);
  *(uint8_t*)&request.bdaddr.b[5] = 0x10;
  request.link_type = ACL_LINK;
  hci_send_event_packet(vhci_fd, HCI_EV_CONN_REQUEST, &request,
                        sizeof(request));
  struct hci_ev_conn_complete complete;
  memset(&complete, 0, sizeof(complete));
  complete.status = 0;
  complete.handle = HCI_HANDLE_1;
  memset(&complete.bdaddr, 0xaa, 6);
  *(uint8_t*)&complete.bdaddr.b[5] = 0x10;
  complete.link_type = ACL_LINK;
  complete.encr_mode = 0;
  hci_send_event_packet(vhci_fd, HCI_EV_CONN_COMPLETE, &complete,
                        sizeof(complete));
  struct hci_ev_remote_features features;
  memset(&features, 0, sizeof(features));
  features.status = 0;
  features.handle = HCI_HANDLE_1;
  hci_send_event_packet(vhci_fd, HCI_EV_REMOTE_FEATURES, &features,
                        sizeof(features));
  struct {
    struct hci_ev_le_meta le_meta;
    struct hci_ev_le_conn_complete le_conn;
  } le_conn;
  memset(&le_conn, 0, sizeof(le_conn));
  le_conn.le_meta.subevent = HCI_EV_LE_CONN_COMPLETE;
  memset(&le_conn.le_conn.bdaddr, 0xaa, 6);
  *(uint8_t*)&le_conn.le_conn.bdaddr.b[5] = 0x11;
  le_conn.le_conn.role = 1;
  le_conn.le_conn.handle = HCI_HANDLE_2;
  hci_send_event_packet(vhci_fd, HCI_EV_LE_META, &le_conn, sizeof(le_conn));
  pthread_join(th, NULL);
  close(hci_sock);
}

static void setup_common()
{
  if (mount(0, "/sys/fs/fuse/connections", "fusectl", 0, 0)) {
  }
}

static void setup_binderfs()
{
  if (mkdir("/dev/binderfs", 0777)) {
  }
  if (mount("binder", "/dev/binderfs", "binder", 0, NULL)) {
  }
  if (symlink("/dev/binderfs", "./binderfs")) {
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
  rlim.rlim_cur = rlim.rlim_max = 128 << 20;
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
  initialize_vhci();
  sandbox_common();
  drop_caps();
  if (unshare(CLONE_NEWNET)) {
  }
  write_file("/proc/sys/net/ipv4/ping_group_range", "0 65535");
  initialize_wifi_devices();
  setup_binderfs();
  loop();
  exit(1);
}

static void kill_and_wait(int pid, int* status)
{
  kill(-pid, SIGKILL);
  kill(pid, SIGKILL);
  for (int i = 0; i < 100; i++) {
    if (waitpid(-1, status, WNOHANG | __WALL) == pid)
      return;
    usleep(1000);
  }
  DIR* dir = opendir("/sys/fs/fuse/connections");
  if (dir) {
    for (;;) {
      struct dirent* ent = readdir(dir);
      if (!ent)
        break;
      if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        continue;
      char abort[300];
      snprintf(abort, sizeof(abort), "/sys/fs/fuse/connections/%s/abort",
               ent->d_name);
      int fd = open(abort, O_WRONLY);
      if (fd == -1) {
        continue;
      }
      if (write(fd, abort, 1) < 0) {
      }
      close(fd);
    }
    closedir(dir);
  } else {
  }
  while (waitpid(-1, status, __WALL) != pid) {
  }
}

static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  write_file("/proc/self/oom_score_adj", "1000");
}

static void close_fds()
{
  for (int fd = 3; fd < MAX_FDS; fd++)
    close(fd);
}

static void setup_usb()
{
  if (chmod("/dev/raw-gadget", 0666))
    exit(1);
}

struct thread_t {
  int created, call;
  event_t ready, done;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    event_wait(&th->ready);
    event_reset(&th->ready);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    event_set(&th->done);
  }
  return 0;
}

static void execute_one(void)
{
  int i, call, thread;
  for (call = 0; call < 4; call++) {
    for (thread = 0; thread < (int)(sizeof(threads) / sizeof(threads[0]));
         thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        event_init(&th->ready);
        event_init(&th->done);
        event_set(&th->done);
        thread_start(thr, th);
      }
      if (!event_isset(&th->done))
        continue;
      event_reset(&th->done);
      th->call = call;
      __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
      event_set(&th->ready);
      event_timedwait(&th->done, 50 + (call == 3 ? 500 : 0));
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
  close_fds();
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter = 0;
  for (;; iter++) {
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      setup_test();
      execute_one();
      exit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      if (waitpid(-1, &status, WNOHANG | WAIT_FLAGS) == pid)
        break;
      sleep_ms(1);
      if (current_time_ms() - start < 5000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    NONFAILING(*(uint32_t*)0x200009c0 = 0xf);
    NONFAILING(*(uint32_t*)0x200009c4 = 4);
    NONFAILING(*(uint32_t*)0x200009c8 = 8);
    NONFAILING(*(uint32_t*)0x200009cc = 0xc);
    NONFAILING(*(uint32_t*)0x200009d0 = 0);
    NONFAILING(*(uint32_t*)0x200009d4 = -1);
    NONFAILING(*(uint32_t*)0x200009d8 = 0);
    NONFAILING(memset((void*)0x200009dc, 0, 16));
    NONFAILING(*(uint32_t*)0x200009ec = 0);
    NONFAILING(*(uint32_t*)0x200009f0 = -1);
    NONFAILING(*(uint32_t*)0x200009f4 = 0);
    NONFAILING(*(uint32_t*)0x200009f8 = 0);
    NONFAILING(*(uint32_t*)0x200009fc = 0);
    NONFAILING(*(uint64_t*)0x20000a00 = 0);
    res = syscall(__NR_bpf, /*cmd=*/0ul, /*arg=*/0x200009c0ul, /*size=*/0x48ul);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    NONFAILING(*(uint32_t*)0x200000c0 = 0x1b);
    NONFAILING(*(uint32_t*)0x200000c4 = 0);
    NONFAILING(*(uint32_t*)0x200000c8 = 0);
    NONFAILING(*(uint32_t*)0x200000cc = 0x8000);
    NONFAILING(*(uint32_t*)0x200000d0 = 0);
    NONFAILING(*(uint32_t*)0x200000d4 = -1);
    NONFAILING(*(uint32_t*)0x200000d8 = 0);
    NONFAILING(memset((void*)0x200000dc, 0, 16));
    NONFAILING(*(uint32_t*)0x200000ec = 0);
    NONFAILING(*(uint32_t*)0x200000f0 = -1);
    NONFAILING(*(uint32_t*)0x200000f4 = 0);
    NONFAILING(*(uint32_t*)0x200000f8 = 0);
    NONFAILING(*(uint32_t*)0x200000fc = 0);
    NONFAILING(*(uint64_t*)0x20000100 = 0);
    res = syscall(__NR_bpf, /*cmd=*/0ul, /*arg=*/0x200000c0ul, /*size=*/0x48ul);
    if (res != -1)
      r[1] = res;
    break;
  case 2:
    NONFAILING(*(uint32_t*)0x200000c0 = 0x11);
    NONFAILING(*(uint32_t*)0x200000c4 = 0x10);
    NONFAILING(*(uint64_t*)0x200000c8 = 0x20000280);
    NONFAILING(memcpy((void*)0x20000280,
                      "\x18\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                      "\x00\x00\x18\x11\x00\x00",
                      20));
    NONFAILING(*(uint32_t*)0x20000294 = r[1]);
    NONFAILING(
        memcpy((void*)0x20000298,
               "\x00\x00\x00\x00\x00\x00\x00\x00\xb7\x02\x00\x00\x02\x00\x00"
               "\x00\x85\x00\x00\x00\x86\x00\x00\x00\x18\x11\x00\x00",
               28));
    NONFAILING(*(uint32_t*)0x200002b4 = r[0]);
    NONFAILING(memcpy(
        (void*)0x200002b8,
        "\x00\x00\x00\x00\x00\x00\x00\x00\xb7\x08\x00\x00\x00\x00\x00\x00\x7b"
        "\x8a\xf8\xff\x00\x00\x00\x00\xbf\xa2\x00\x00\x00\x00\x00\x00\x07\x02"
        "\x00\x00\xf8\xff\xff\xff\xb7\x03\x00\x00\x08\x00\x00\x00\xb7\x04\x00"
        "\x00\x00\x00\x00\x00\x85\x00\x00\x00\x03\x00\x00\x00\x95",
        65));
    NONFAILING(*(uint64_t*)0x200000d0 = 0x20000040);
    NONFAILING(memcpy((void*)0x20000040, "syzkaller\000", 10));
    NONFAILING(*(uint32_t*)0x200000d8 = 0);
    NONFAILING(*(uint32_t*)0x200000dc = 0);
    NONFAILING(*(uint64_t*)0x200000e0 = 0);
    NONFAILING(*(uint32_t*)0x200000e8 = 0);
    NONFAILING(*(uint32_t*)0x200000ec = 0);
    NONFAILING(memset((void*)0x200000f0, 0, 16));
    NONFAILING(*(uint32_t*)0x20000100 = 0);
    NONFAILING(*(uint32_t*)0x20000104 = 0);
    NONFAILING(*(uint32_t*)0x20000108 = -1);
    NONFAILING(*(uint32_t*)0x2000010c = 0);
    NONFAILING(*(uint64_t*)0x20000110 = 0);
    NONFAILING(*(uint32_t*)0x20000118 = 0);
    NONFAILING(*(uint32_t*)0x2000011c = 0);
    NONFAILING(*(uint64_t*)0x20000120 = 0);
    NONFAILING(*(uint32_t*)0x20000128 = 0);
    NONFAILING(*(uint32_t*)0x2000012c = 0);
    NONFAILING(*(uint32_t*)0x20000130 = 0);
    NONFAILING(*(uint32_t*)0x20000134 = 0);
    NONFAILING(*(uint64_t*)0x20000138 = 0);
    NONFAILING(*(uint64_t*)0x20000140 = 0);
    NONFAILING(*(uint32_t*)0x20000148 = 0);
    NONFAILING(*(uint32_t*)0x2000014c = 0);
    res = syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x200000c0ul, /*size=*/0x90ul);
    if (res != -1)
      r[2] = res;
    break;
  case 3:
    NONFAILING(*(uint64_t*)0x200001c0 = 0x20000180);
    NONFAILING(memcpy((void*)0x20000180, "kfree\000", 6));
    NONFAILING(*(uint32_t*)0x200001c8 = r[2]);
    syscall(__NR_bpf, /*cmd=*/0x11ul, /*arg=*/0x200001c0ul, /*size=*/0x10ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  setup_usb();
  install_segv_handler();
  for (procid = 0; procid < 5; procid++) {
    if (fork() == 0) {
      do_sandbox_none();
    }
  }
  sleep(1000000);
  return 0;
}
