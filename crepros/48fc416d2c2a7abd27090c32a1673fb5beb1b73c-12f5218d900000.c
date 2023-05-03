// https://syzkaller.appspot.com/bug?id=48fc416d2c2a7abd27090c32a1673fb5beb1b73c
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
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
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
#include <linux/rtnetlink.h>
#include <linux/veth.h>

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

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

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
    return -1;
  }
  return 0;
}

static void initialize_wifi_devices(void)
{
  uint8_t mac_addr[6] = WIFI_MAC_BASE;
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (sock < 0) {
    return;
  }
  int hwsim_family_id = netlink_query_family_id(&nlmsg, sock, "MAC80211_HWSIM");
  int nl80211_family_id = netlink_query_family_id(&nlmsg, sock, "nl80211");
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

#define MAX_FDS 30

static void setup_common()
{
  if (mount(0, "/sys/fs/fuse/connections", "fusectl", 0, 0)) {
  }
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
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
  int collide = 0;
again:
  for (call = 0; call < 17; call++) {
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
      if (collide && (call % 2) == 0)
        break;
      event_timedwait(&th->done, 45);
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
  close_fds();
  if (!collide) {
    collide = 1;
    goto again;
  }
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
      if (current_time_ms() - start < 5 * 1000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    res = syscall(__NR_openat, 0xffffff9c, 0ul, 0x26e1ul, 0ul);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    sprintf((char*)0x20000200, "0x%016llx", (long long)0);
    syscall(__NR_write, r[0], 0x20000200ul, 0x12ul);
    break;
  case 2:
    syscall(__NR_ioctl, -1, 0x40042408, -1);
    break;
  case 3:
    syscall(__NR_bpf, 0xful, 0ul, 0ul);
    break;
  case 4:
    res = syscall(__NR_socket, 0x29ul, 2ul, 0);
    if (res != -1)
      r[1] = res;
    break;
  case 5:
    *(uint64_t*)0x20000000 = 0;
    syscall(__NR_ioctl, -1, 0x40305839, 0x20000000ul);
    break;
  case 6:
    memcpy((void*)0x200001c0, "memory.current\000", 15);
    res = syscall(__NR_openat, 0xffffff9c, 0x200001c0ul, 0x100002ul, 0ul);
    if (res != -1)
      r[2] = res;
    break;
  case 7:
    syscall(__NR_ioctl, r[2], 0xc028660f, 0ul);
    break;
  case 8:
    syscall(__NR_openat, r[2], 0ul, 2ul, 0ul);
    break;
  case 9:
    *(uint32_t*)0x20000100 = 0x1a;
    *(uint32_t*)0x20000104 = 6;
    *(uint32_t*)0x20000108 = 0x800;
    *(uint32_t*)0x2000010c = 6;
    *(uint32_t*)0x20000110 = 0x120;
    *(uint32_t*)0x20000114 = -1;
    *(uint32_t*)0x20000118 = 0xfffffff9;
    *(uint8_t*)0x2000011c = 0;
    *(uint8_t*)0x2000011d = 0;
    *(uint8_t*)0x2000011e = 0;
    *(uint8_t*)0x2000011f = 0;
    *(uint8_t*)0x20000120 = 0;
    *(uint8_t*)0x20000121 = 0;
    *(uint8_t*)0x20000122 = 0;
    *(uint8_t*)0x20000123 = 0;
    *(uint8_t*)0x20000124 = 0;
    *(uint8_t*)0x20000125 = 0;
    *(uint8_t*)0x20000126 = 0;
    *(uint8_t*)0x20000127 = 0;
    *(uint8_t*)0x20000128 = 0;
    *(uint8_t*)0x20000129 = 0;
    *(uint8_t*)0x2000012a = 0;
    *(uint8_t*)0x2000012b = 0;
    *(uint32_t*)0x2000012c = 0;
    *(uint32_t*)0x20000130 = -1;
    *(uint32_t*)0x20000134 = 5;
    *(uint32_t*)0x20000138 = 3;
    *(uint32_t*)0x2000013c = 0;
    syscall(__NR_bpf, 0ul, 0x20000100ul, 0x40ul);
    break;
  case 10:
    memcpy((void*)0x20000000,
           "wlan1\000\033\032\354\265\022\003F\331\000\000\037\000\000\000\000"
           "\000\377\020\000,C\375j\343\215\343\326\340|6l\351\331;"
           "\023\337\367\276r\'\212\325\325\341\365\\\233\262\b\336\273g\3018"
           "\204,:f\313\350oOArYZ\341\037\353p\365\373\252d\032\240\261\234,"
           "\350\377^9P\356\212G\335\000\016\330\a\312\310~"
           "\202\366\252j\345\367\031\353#;|\253A1\252\2747Tf\361Y\034\355~"
           "\351\351\371\317W\334\317\351\215\350\317\2362I["
           "\317\367\260K\034\034\006h\214d\f6\376\267\277\256\352\231\352\263G"
           "\212\343\234\226\365\370\265\031\twp\372\251\330\277Sa\271\v\263"
           "\261\005\2405\334\022\206\271\3713\254\024\037Xf\375\3360\203\177"
           "\200\335L[t% "
           "/H\355\352\200*NA\272X\r\352\v\203<"
           "\241d\000\000\000\000\000\000\020\000\000\257\366\2752",
           238);
    syscall(__NR_ioctl, r[1], 0x8b28, 0x20000000ul);
    break;
  case 11:
    syscall(__NR_socket, 0x10ul, 2ul, 0);
    break;
  case 12:
    syscall(__NR_perf_event_open, 0ul, 0, -1ul, -1, 0ul);
    break;
  case 13:
    *(uint32_t*)0x20000300 = 1;
    *(uint32_t*)0x20000304 = 0x70;
    *(uint8_t*)0x20000308 = 0;
    *(uint8_t*)0x20000309 = 0;
    *(uint8_t*)0x2000030a = 0;
    *(uint8_t*)0x2000030b = 0;
    *(uint32_t*)0x2000030c = 0;
    *(uint64_t*)0x20000310 = 0x203;
    *(uint64_t*)0x20000318 = 0;
    *(uint64_t*)0x20000320 = 4;
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 0, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 1, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 2, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 3, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 4, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 5, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 6, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 7, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 8, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 9, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 10, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 11, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 12, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 13, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 14, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 15, 2);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 17, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 18, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 19, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 20, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 21, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 22, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 23, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 24, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 25, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 26, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 27, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 28, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 29, 35);
    *(uint32_t*)0x20000330 = 0;
    *(uint32_t*)0x20000334 = 0;
    *(uint64_t*)0x20000338 = 0;
    *(uint64_t*)0x20000340 = 0xfffffffffffffffc;
    *(uint64_t*)0x20000348 = 0x800;
    *(uint64_t*)0x20000350 = 0;
    *(uint32_t*)0x20000358 = 0x804;
    *(uint32_t*)0x2000035c = 0;
    *(uint64_t*)0x20000360 = 0;
    *(uint32_t*)0x20000368 = 0x1001;
    *(uint16_t*)0x2000036c = 0xfffd;
    *(uint16_t*)0x2000036e = 0;
    syscall(__NR_perf_event_open, 0x20000300ul, 0, -1ul, -1, 0ul);
    break;
  case 14:
    syscall(__NR_perf_event_open, 0ul, 0, 0xdul, -1, 0ul);
    break;
  case 15:
    *(uint32_t*)0x20001f80 = 2;
    *(uint32_t*)0x20001f84 = 0x70;
    *(uint8_t*)0x20001f88 = 0x40;
    *(uint8_t*)0x20001f89 = 2;
    *(uint8_t*)0x20001f8a = 7;
    *(uint8_t*)0x20001f8b = 4;
    *(uint32_t*)0x20001f8c = 0;
    *(uint64_t*)0x20001f90 = 4;
    *(uint64_t*)0x20001f98 = 0x614030bbc056506a;
    *(uint64_t*)0x20001fa0 = 0xcdd3f34d7821d21c;
    STORE_BY_BITMASK(uint64_t, , 0x20001fa8, 0, 0, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20001fa8, 0, 1, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20001fa8, 0, 2, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20001fa8, 0, 3, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20001fa8, 0, 4, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20001fa8, 0, 5, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20001fa8, 0, 6, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20001fa8, 0, 7, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20001fa8, 0, 8, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20001fa8, 0, 9, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20001fa8, 1, 10, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20001fa8, 0, 11, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20001fa8, 0, 12, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20001fa8, 0, 13, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20001fa8, 0, 14, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20001fa8, 0, 15, 2);
    STORE_BY_BITMASK(uint64_t, , 0x20001fa8, 0, 17, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20001fa8, 1, 18, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20001fa8, 0, 19, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20001fa8, 1, 20, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20001fa8, 1, 21, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20001fa8, 1, 22, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20001fa8, 1, 23, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20001fa8, 1, 24, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20001fa8, 1, 25, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20001fa8, 0, 26, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20001fa8, 0, 27, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20001fa8, 0, 28, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20001fa8, 0, 29, 35);
    *(uint32_t*)0x20001fb0 = 2;
    *(uint32_t*)0x20001fb4 = 2;
    *(uint64_t*)0x20001fb8 = 0x80000000;
    *(uint64_t*)0x20001fc0 = 8;
    *(uint64_t*)0x20001fc8 = 0x41040;
    *(uint64_t*)0x20001fd0 = 5;
    *(uint32_t*)0x20001fd8 = 0xffff;
    *(uint32_t*)0x20001fdc = 5;
    *(uint64_t*)0x20001fe0 = 7;
    *(uint32_t*)0x20001fe8 = 9;
    *(uint16_t*)0x20001fec = 0;
    *(uint16_t*)0x20001fee = 0;
    syscall(__NR_perf_event_open, 0x20001f80ul, -1, -1ul, -1, 0x1aul);
    break;
  case 16:
    syscall(__NR_sendmsg, -1, 0ul, 0x20000802ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  do_sandbox_none();
  return 0;
}
