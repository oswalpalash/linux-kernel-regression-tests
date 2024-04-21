// https://syzkaller.appspot.com/bug?id=371873989564922b25b65a0c8bebb3e702ebe24a
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <dirent.h>
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
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/swap.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/capability.h>
#include <linux/falloc.h>
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
                                    const char* name, bool up)
{
  struct ifinfomsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  if (up)
    hdr.ifi_flags = hdr.ifi_change = IFF_UP;
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

static int runcmdline(char* cmdline)
{
  int ret = system(cmdline);
  if (ret) {
  }
  return ret;
}

#define MAX_FDS 30

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

#define FS_IOC_SETFLAGS _IOW('f', 2, long)
static void remove_dir(const char* dir)
{
  int iter = 0;
  DIR* dp = 0;
retry:
  const int umount_flags = MNT_FORCE | UMOUNT_NOFOLLOW;
  while (umount2(dir, umount_flags) == 0) {
  }
  dp = opendir(dir);
  if (dp == NULL) {
    if (errno == EMFILE) {
      exit(1);
    }
    exit(1);
  }
  struct dirent* ep = 0;
  while ((ep = readdir(dp))) {
    if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
      continue;
    char filename[FILENAME_MAX];
    snprintf(filename, sizeof(filename), "%s/%s", dir, ep->d_name);
    while (umount2(filename, umount_flags) == 0) {
    }
    struct stat st;
    if (lstat(filename, &st))
      exit(1);
    if (S_ISDIR(st.st_mode)) {
      remove_dir(filename);
      continue;
    }
    int i;
    for (i = 0;; i++) {
      if (unlink(filename) == 0)
        break;
      if (errno == EPERM) {
        int fd = open(filename, O_RDONLY);
        if (fd != -1) {
          long flags = 0;
          if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == 0) {
          }
          close(fd);
          continue;
        }
      }
      if (errno == EROFS) {
        break;
      }
      if (errno != EBUSY || i > 100)
        exit(1);
      if (umount2(filename, umount_flags))
        exit(1);
    }
  }
  closedir(dp);
  for (int i = 0;; i++) {
    if (rmdir(dir) == 0)
      break;
    if (i < 100) {
      if (errno == EPERM) {
        int fd = open(dir, O_RDONLY);
        if (fd != -1) {
          long flags = 0;
          if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == 0) {
          }
          close(fd);
          continue;
        }
      }
      if (errno == EROFS) {
        break;
      }
      if (errno == EBUSY) {
        if (umount2(dir, umount_flags))
          exit(1);
        continue;
      }
      if (errno == ENOTEMPTY) {
        if (iter < 100) {
          iter++;
          goto retry;
        }
      }
    }
    exit(1);
  }
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
  if (symlink("/dev/binderfs", "./binderfs")) {
  }
}

static void close_fds()
{
  for (int fd = 3; fd < MAX_FDS; fd++)
    close(fd);
}

static void setup_binfmt_misc()
{
  if (mount(0, "/proc/sys/fs/binfmt_misc", "binfmt_misc", 0, 0)) {
  }
  write_file("/proc/sys/fs/binfmt_misc/register", ":syz0:M:0:\x01::./file0:");
  write_file("/proc/sys/fs/binfmt_misc/register",
             ":syz1:M:1:\x02::./file0:POC");
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
      netlink_add_device_impl(&nlmsg, "lowpan", "lowpan0", false);
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

#define SWAP_FILE "./swap-file"
#define SWAP_FILE_SIZE (128 * 1000 * 1000)

static void setup_swap()
{
  swapoff(SWAP_FILE);
  unlink(SWAP_FILE);
  int fd = open(SWAP_FILE, O_CREAT | O_WRONLY | O_CLOEXEC, 0600);
  if (fd == -1) {
    exit(1);
    return;
  }
  fallocate(fd, FALLOC_FL_ZERO_RANGE, 0, SWAP_FILE_SIZE);
  close(fd);
  char cmdline[64];
  sprintf(cmdline, "mkswap %s", SWAP_FILE);
  if (runcmdline(cmdline)) {
    exit(1);
    return;
  }
  if (swapon(SWAP_FILE, SWAP_FLAG_PREFER) == 1) {
    exit(1);
    return;
  }
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter = 0;
  for (;; iter++) {
    char cwdbuf[32];
    sprintf(cwdbuf, "./%d", iter);
    if (mkdir(cwdbuf, 0777))
      exit(1);
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      if (chdir(cwdbuf))
        exit(1);
      setup_test();
      execute_one();
      close_fds();
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
    remove_dir(cwdbuf);
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  syscall(__NR_socketpair, /*domain=AF_UNIX*/ 1ul, /*type=SOCK_STREAM*/ 1ul,
          /*proto=*/0, /*fds=*/0x20000000ul);
  NONFAILING(*(uint32_t*)0x200000c0 = 0x12);
  NONFAILING(*(uint32_t*)0x200000c4 = 2);
  NONFAILING(*(uint32_t*)0x200000c8 = 4);
  NONFAILING(*(uint32_t*)0x200000cc = 2);
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
    r[0] = res;
  NONFAILING(*(uint32_t*)0x200000c0 = 0);
  NONFAILING(*(uint32_t*)0x200000c4 = 0xc);
  NONFAILING(*(uint64_t*)0x200000c8 = 0x20000440);
  NONFAILING(*(uint8_t*)0x20000440 = 0x18);
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000441, 0, 0, 4));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000441, 0, 4, 4));
  NONFAILING(*(uint16_t*)0x20000442 = 0);
  NONFAILING(*(uint32_t*)0x20000444 = 0);
  NONFAILING(*(uint8_t*)0x20000448 = 0);
  NONFAILING(*(uint8_t*)0x20000449 = 0);
  NONFAILING(*(uint16_t*)0x2000044a = 0);
  NONFAILING(*(uint32_t*)0x2000044c = 0);
  NONFAILING(*(uint8_t*)0x20000450 = 0x18);
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000451, 1, 0, 4));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000451, 1, 4, 4));
  NONFAILING(*(uint16_t*)0x20000452 = 0);
  NONFAILING(*(uint32_t*)0x20000454 = r[0]);
  NONFAILING(*(uint8_t*)0x20000458 = 0);
  NONFAILING(*(uint8_t*)0x20000459 = 0);
  NONFAILING(*(uint16_t*)0x2000045a = 0);
  NONFAILING(*(uint32_t*)0x2000045c = 0);
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000460, 7, 0, 3));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000460, 0, 3, 1));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000460, 0xb, 4, 4));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000461, 8, 0, 4));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000461, 0, 4, 4));
  NONFAILING(*(uint16_t*)0x20000462 = 0);
  NONFAILING(*(uint32_t*)0x20000464 = 0);
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000468, 3, 0, 3));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000468, 3, 3, 2));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000468, 3, 5, 3));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000469, 0xa, 0, 4));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000469, 8, 4, 4));
  NONFAILING(*(uint16_t*)0x2000046a = 0xfff8);
  NONFAILING(*(uint32_t*)0x2000046c = 0);
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000470, 7, 0, 3));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000470, 1, 3, 1));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000470, 0xb, 4, 4));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000471, 2, 0, 4));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000471, 0xa, 4, 4));
  NONFAILING(*(uint16_t*)0x20000472 = 0);
  NONFAILING(*(uint32_t*)0x20000474 = 0);
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000478, 7, 0, 3));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000478, 0, 3, 1));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000478, 0, 4, 4));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000479, 2, 0, 4));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000479, 0, 4, 4));
  NONFAILING(*(uint16_t*)0x2000047a = 0);
  NONFAILING(*(uint32_t*)0x2000047c = 0xfffffff8);
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000480, 7, 0, 3));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000480, 0, 3, 1));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000480, 0xb, 4, 4));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000481, 3, 0, 4));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000481, 0, 4, 4));
  NONFAILING(*(uint16_t*)0x20000482 = 0);
  NONFAILING(*(uint32_t*)0x20000484 = 8);
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000488, 7, 0, 3));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000488, 0, 3, 1));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000488, 0xb, 4, 4));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000489, 4, 0, 4));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x20000489, 0, 4, 4));
  NONFAILING(*(uint16_t*)0x2000048a = 0);
  NONFAILING(*(uint32_t*)0x2000048c = 0);
  NONFAILING(*(uint8_t*)0x20000490 = 0x85);
  NONFAILING(*(uint8_t*)0x20000491 = 0);
  NONFAILING(*(uint16_t*)0x20000492 = 0);
  NONFAILING(*(uint32_t*)0x20000494 = 3);
  NONFAILING(*(uint8_t*)0x20000498 = 0x95);
  NONFAILING(*(uint8_t*)0x20000499 = 0);
  NONFAILING(*(uint16_t*)0x2000049a = 0);
  NONFAILING(*(uint32_t*)0x2000049c = 0);
  NONFAILING(*(uint64_t*)0x200000d0 = 0);
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
  syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x200000c0ul, /*size=*/0x90ul);
  NONFAILING(*(uint32_t*)0x20000ac0 = 0x11);
  NONFAILING(*(uint32_t*)0x20000ac4 = 0xc);
  NONFAILING(*(uint64_t*)0x20000ac8 = 0x20000440);
  NONFAILING(*(uint64_t*)0x20000ad0 = 0x20000240);
  NONFAILING(memcpy((void*)0x20000240, "GPL\000", 4));
  NONFAILING(*(uint32_t*)0x20000ad8 = 0);
  NONFAILING(*(uint32_t*)0x20000adc = 0);
  NONFAILING(*(uint64_t*)0x20000ae0 = 0);
  NONFAILING(*(uint32_t*)0x20000ae8 = 0);
  NONFAILING(*(uint32_t*)0x20000aec = 0);
  NONFAILING(memset((void*)0x20000af0, 0, 16));
  NONFAILING(*(uint32_t*)0x20000b00 = 0);
  NONFAILING(*(uint32_t*)0x20000b04 = 0);
  NONFAILING(*(uint32_t*)0x20000b08 = -1);
  NONFAILING(*(uint32_t*)0x20000b0c = 0);
  NONFAILING(*(uint64_t*)0x20000b10 = 0);
  NONFAILING(*(uint32_t*)0x20000b18 = 0);
  NONFAILING(*(uint32_t*)0x20000b1c = 0);
  NONFAILING(*(uint64_t*)0x20000b20 = 0);
  NONFAILING(*(uint32_t*)0x20000b28 = 0);
  NONFAILING(*(uint32_t*)0x20000b2c = 0);
  NONFAILING(*(uint32_t*)0x20000b30 = 0);
  NONFAILING(*(uint32_t*)0x20000b34 = 0);
  NONFAILING(*(uint64_t*)0x20000b38 = 0);
  NONFAILING(*(uint64_t*)0x20000b40 = 0);
  NONFAILING(*(uint32_t*)0x20000b48 = 0);
  NONFAILING(*(uint32_t*)0x20000b4c = 0);
  res = syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x20000ac0ul, /*size=*/0x90ul);
  if (res != -1)
    r[1] = res;
  NONFAILING(*(uint64_t*)0x200001c0 = 0x20000080);
  NONFAILING(memcpy((void*)0x20000080, "kfree\000", 6));
  NONFAILING(*(uint32_t*)0x200001c8 = r[1]);
  syscall(__NR_bpf, /*cmd=*/0x11ul, /*arg=*/0x200001c0ul, /*size=*/0x10ul);
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
  setup_binfmt_misc();
  setup_802154();
  setup_swap();
  install_segv_handler();
  for (procid = 0; procid < 5; procid++) {
    if (fork() == 0) {
      use_temporary_dir();
      do_sandbox_none();
    }
  }
  sleep(1000000);
  return 0;
}
