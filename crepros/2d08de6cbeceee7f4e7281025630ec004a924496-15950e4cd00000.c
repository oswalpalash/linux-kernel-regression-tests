// https://syzkaller.appspot.com/bug?id=2d08de6cbeceee7f4e7281025630ec004a924496
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/genetlink.h>
#include <linux/if_addr.h>
#include <linux/if_ether.h>
#include <linux/if_link.h>
#include <linux/in6.h>
#include <linux/neighbour.h>
#include <linux/net.h>
#include <linux/netlink.h>
#include <linux/nl80211.h>
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
  if (reply_len)
    *reply_len = 0;
  if (hdr->nlmsg_type == NLMSG_DONE)
    return 0;
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
  return ((struct nlmsgerr*)(hdr + 1))->error;
}

static int netlink_send(struct nlmsg* nlmsg, int sock)
{
  return netlink_send_ext(nlmsg, sock, 0, NULL);
}

static int netlink_query_family_id(struct nlmsg* nlmsg, int sock,
                                   const char* family_name)
{
  struct genlmsghdr genlhdr;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = CTRL_CMD_GETFAMILY;
  netlink_init(nlmsg, GENL_ID_CTRL, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(nlmsg, CTRL_ATTR_FAMILY_NAME, family_name,
               strnlen(family_name, GENL_NAMSIZ - 1) + 1);
  int n = 0;
  int err = netlink_send_ext(nlmsg, sock, GENL_ID_CTRL, &n);
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
    return -1;
  }
  recv(sock, nlmsg->buf, sizeof(nlmsg->buf), 0);
  return id;
}

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
    return -1;
  }
  return 0;
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
    return -1;
  }
  return 0;
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
  int err = netlink_send_ext(nlmsg, sock, RTM_NEWLINK, &n);
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
  int ret = netlink_query_family_id(&nlmsg_tmp, fd, (char*)name);
  if ((int)sock_arg >= 0)
    close(fd);
  if (ret < 0) {
    return -1;
  }
  return ret;
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

#define WIFI_MAX_SSID_LEN 32

#define WIFI_JOIN_IBSS_NO_SCAN 0
#define WIFI_JOIN_IBSS_BG_SCAN 1
#define WIFI_JOIN_IBSS_BG_NO_SCAN 2

static long syz_80211_join_ibss(volatile long a0, volatile long a1,
                                volatile long a2, volatile long a3)
{
  char* interface = (char*)a0;
  uint8_t* ssid = (uint8_t*)a1;
  int ssid_len = (int)a2;
  int mode = (int)a3;
  struct nlmsg tmp_msg;
  uint8_t bssid[ETH_ALEN] = WIFI_IBSS_BSSID;
  if (ssid_len < 0 || ssid_len > WIFI_MAX_SSID_LEN) {
    return -1;
  }
  if (mode < 0 || mode > WIFI_JOIN_IBSS_BG_NO_SCAN) {
    return -1;
  }
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (sock < 0) {
    return -1;
  }
  int nl80211_family_id = netlink_query_family_id(&tmp_msg, sock, "nl80211");
  struct join_ibss_props ibss_props = {
      .wiphy_freq = WIFI_DEFAULT_FREQUENCY,
      .wiphy_freq_fixed =
          (mode == WIFI_JOIN_IBSS_NO_SCAN || mode == WIFI_JOIN_IBSS_BG_NO_SCAN),
      .mac = bssid,
      .ssid = ssid,
      .ssid_len = ssid_len};
  int ret = nl80211_setup_ibss_interface(&tmp_msg, sock, nl80211_family_id,
                                         interface, &ibss_props);
  close(sock);
  if (ret < 0) {
    return -1;
  }
  if (mode == WIFI_JOIN_IBSS_NO_SCAN) {
    ret = await_ifla_operstate(&tmp_msg, interface, IF_OPER_UP);
    if (ret < 0) {
      return -1;
    }
  }
  return 0;
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
      if (current_time_ms() - start < 5000) {
        continue;
      }
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  NONFAILING(memcpy((void*)0x20000000, "wlan0\000", 6));
  NONFAILING(syz_80211_join_ibss(0x20000000, 0x9999999999999999, 9, 2));
  res = syscall(__NR_socket, 0x10ul, 3ul, 0x10);
  if (res != -1)
    r[0] = res;
  NONFAILING(memcpy((void*)0x20000200, "nl80211\000", 8));
  res = -1;
  NONFAILING(res = syz_genetlink_get_family_id(0x20000200, -1));
  if (res != -1)
    r[1] = res;
  NONFAILING(memcpy((void*)0x20000700,
                    "wlan0\000\000\000\000\000\000\000\000\000\000\000", 16));
  res = syscall(__NR_ioctl, r[0], 0x8933, 0x20000700ul);
  if (res != -1)
    NONFAILING(r[2] = *(uint32_t*)0x20000710);
  NONFAILING(*(uint64_t*)0x20000340 = 0);
  NONFAILING(*(uint32_t*)0x20000348 = 0);
  NONFAILING(*(uint64_t*)0x20000350 = 0x20000300);
  NONFAILING(*(uint64_t*)0x20000300 = 0x20000240);
  NONFAILING(*(uint32_t*)0x20000240 = 0x24);
  NONFAILING(*(uint16_t*)0x20000244 = r[1]);
  NONFAILING(*(uint16_t*)0x20000246 = 5);
  NONFAILING(*(uint32_t*)0x20000248 = 0);
  NONFAILING(*(uint32_t*)0x2000024c = 0);
  NONFAILING(*(uint8_t*)0x20000250 = 6);
  NONFAILING(*(uint8_t*)0x20000251 = 0);
  NONFAILING(*(uint16_t*)0x20000252 = 0);
  NONFAILING(*(uint16_t*)0x20000254 = 8);
  NONFAILING(*(uint16_t*)0x20000256 = 3);
  NONFAILING(*(uint32_t*)0x20000258 = r[2]);
  NONFAILING(*(uint16_t*)0x2000025c = 8);
  NONFAILING(*(uint16_t*)0x2000025e = 5);
  NONFAILING(*(uint32_t*)0x20000260 = 9);
  NONFAILING(*(uint64_t*)0x20000308 = 0x24);
  NONFAILING(*(uint64_t*)0x20000358 = 1);
  NONFAILING(*(uint64_t*)0x20000360 = 0);
  NONFAILING(*(uint64_t*)0x20000368 = 0);
  NONFAILING(*(uint32_t*)0x20000370 = 0x20040890);
  syscall(__NR_sendmsg, r[0], 0x20000340ul, 0ul);
  NONFAILING(*(uint64_t*)0x20000340 = 0);
  NONFAILING(*(uint32_t*)0x20000348 = 0);
  NONFAILING(*(uint64_t*)0x20000350 = 0x20000300);
  NONFAILING(*(uint64_t*)0x20000300 = 0x20000380);
  NONFAILING(memcpy((void*)0x20000380, "\x00\x03\x00\x00", 4));
  NONFAILING(*(uint16_t*)0x20000384 = r[1]);
  NONFAILING(memcpy((void*)0x20000386,
                    "\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0f\x00\x00\x00"
                    "\x08\x00\x03\x00",
                    18));
  NONFAILING(*(uint32_t*)0x20000398 = r[2]);
  NONFAILING(memcpy(
      (void*)0x2000039c,
      "\x28\x00\x0e\x00\x80\x00\x00\x00\xff\xff\xff\xff\xff\xff\x08\x02\x11\x00"
      "\x00\x01\x4c\x88\xe8\xeb\x7e\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x64\x00\xff\xff\x08\x00\x26\x00\x6c\x09\x00\x00\x08\x00\x0c\x00\x64\x00"
      "\x00\x00\x08\x00\x0d\x00\x00\x00\x00\x00\x0a\x00\x34\x00\x02\x02\x02\x02"
      "\x02\x02\x00\x00\x08\x00\x35\x00\x00\x00\x00\x00\x90\x02\x0f\x00\x04\x4e"
      "\x79\xc1\x99\xe8\x3b\xed\x43\x6d\x9b\x18\xd6\x35\xd7\xd6\xe8\x00\x08\x00"
      "\x00\x88\x54\x41\x3f\x64\xd1\x7b\x8d\x7a\x90\x41\xea\x1a\xa6\xcb\x32\x8e"
      "\x4e\xae\x74\xed\xc1\xe0\xad\x5b\x56\x00\x31\x9a\x0c\xf9\x6d\x64\xa3\x28"
      "\xee\xca\xfa\xc1\x12\xed\x3e\xc8\x84\xa1\x53\x1d\xfd\xf1\x7a\x1c\xbe\x07"
      "\xe1\x3a\xd5\x79\x11\xd1\x2d\x26\x9b\x02\x2c\xcf\x8f\xf9\xaf\x66\xb4\x1d"
      "\x84\xa1\xf4\x7c\xd1\x6e\xce\xac\xcf\x5e\xf9\x15\x9b\x14\xc6\xd0\x25\xae"
      "\xa1\x2d\xba\xe5\x03\xb9\x48\x22\xfd\x6a\x1c\x7b\xbb\x4c\x9d\x1e\xff\xa4"
      "\x6a\x99\xc6\x02\x5a\xba\x12\x6a\xb4\xd7\x43\x3b\x72\x2b\x8c\xf4\x0f\xaa"
      "\xb9\x1f\x78\x6c\x32\x81\x3e\xc3\x1c\xbb\xec\xee\xeb\xe4\x4f\xb3\x52\x7d"
      "\x98\x35\x6b\x7c\xef\x74\x53\xa6\xd0\xbc\x7c\x65\x73\xe1\xe9\x31\x55\x7e"
      "\x85\x94\x57\x2f\xe5\x20\xd0\xb3\x90\x3b\xd7\xe8\x5e\xc4\xab\xc6\x4e\x96"
      "\x4b\x56\x55\xb2\x40\xa4\xe6\x02\x0c\x7a\x9f\x72\xb1\x6a\xf0\xfb\xcd\xcb"
      "\x35\x4e\x91\xc6\x9c\x1a\x6b\xa7\x57\x64\x92\x63\xf8\xb0\xbc\x97\xce\x3f"
      "\xfc\x1f\xff\xa6\x04\x7b\xb4\xa1\x8a\xad\x45\xa0\xfa\x6f\x1c\xf4\x40\x8c"
      "\x8d\xf7\x9b\xce\xdd\xae\xff\xb0\x50\x1f\x61\x5f\xeb\x70\x80\x1a\xac\x79"
      "\xa1\xb3\xb7\x40\x01\x06\x6b\xfe\x7b\x65\x2c\x5b\xa9\xe6\xdf\x56\x06\x10"
      "\xaf\xa7\xb2\xb0\xae\x89\xd8\x16\x7e\xf9\x5c\xab\x5f\x20\x73\x20\x67\x98"
      "\x08\x9f\x72\x5a\x5d\x9f\x20\x7a\x14\x09\x90\x61\x12\xda\x7e\x52\xb9\x0d"
      "\x5e\xa3\x45\x37\xd6\xd6\xce\x24\xa2\xdb\x63\x14\x41\xbe\x1e\x6d\x63\x23"
      "\xd6\x16\x55\xd4\x15\x6b\x50\x6f\xdb\xe3\xf6\xc9\x81\x35\x28\xa9\x50\xdf"
      "\x8a\x4c\x2a\x6a\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
      467));
  NONFAILING(*(uint64_t*)0x20000308 = 0x300);
  NONFAILING(*(uint64_t*)0x20000358 = 1);
  NONFAILING(*(uint64_t*)0x20000360 = 0);
  NONFAILING(*(uint64_t*)0x20000368 = 0);
  NONFAILING(*(uint32_t*)0x20000370 = 0);
  syscall(__NR_sendmsg, r[0], 0x20000340ul, 0ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  install_segv_handler();
  loop();
  return 0;
}
