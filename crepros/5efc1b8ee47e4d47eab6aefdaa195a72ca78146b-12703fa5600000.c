// https://syzkaller.appspot.com/bug?id=5efc1b8ee47e4d47eab6aefdaa195a72ca78146b
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <net/if_arp.h>
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
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/capability.h>
#include <linux/if_addr.h>
#include <linux/if_ether.h>
#include <linux/if_link.h>
#include <linux/if_tun.h>
#include <linux/in6.h>
#include <linux/ip.h>
#include <linux/neighbour.h>
#include <linux/net.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/tcp.h>
#include <linux/veth.h>

unsigned long long procid;

static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* ctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  if (__atomic_load_n(&skip_segv, __ATOMIC_RELAXED) &&
      (addr < prog_start || addr > prog_end)) {
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
  {                                                                            \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    if (_setjmp(segv_env) == 0) {                                              \
      __VA_ARGS__;                                                             \
    }                                                                          \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
  }

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

static int netlink_send(int sock)
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
  if (n < sizeof(struct nlmsghdr) + sizeof(struct nlmsgerr))
    exit(1);
  if (hdr->nlmsg_type != NLMSG_ERROR)
    exit(1);
  return -((struct nlmsgerr*)(hdr + 1))->error;
}

static void netlink_device_change(int sock, const char* name, bool up,
                                  const char* master, const void* mac,
                                  int macsize)
{
  struct ifinfomsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  if (up)
    hdr.ifi_flags = hdr.ifi_change = IFF_UP;
  netlink_init(RTM_NEWLINK, 0, &hdr, sizeof(hdr));
  netlink_attr(IFLA_IFNAME, name, strlen(name));
  if (master) {
    int ifindex = if_nametoindex(master);
    netlink_attr(IFLA_MASTER, &ifindex, sizeof(ifindex));
  }
  if (macsize)
    netlink_attr(IFLA_ADDRESS, mac, macsize);
  int err = netlink_send(sock);
  (void)err;
}

static int netlink_add_addr(int sock, const char* dev, const void* addr,
                            int addrsize)
{
  struct ifaddrmsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  hdr.ifa_family = addrsize == 4 ? AF_INET : AF_INET6;
  hdr.ifa_prefixlen = addrsize == 4 ? 24 : 120;
  hdr.ifa_scope = RT_SCOPE_UNIVERSE;
  hdr.ifa_index = if_nametoindex(dev);
  netlink_init(RTM_NEWADDR, NLM_F_CREATE | NLM_F_REPLACE, &hdr, sizeof(hdr));
  netlink_attr(IFA_LOCAL, addr, addrsize);
  netlink_attr(IFA_ADDRESS, addr, addrsize);
  return netlink_send(sock);
}

static void netlink_add_addr4(int sock, const char* dev, const char* addr)
{
  struct in_addr in_addr;
  inet_pton(AF_INET, addr, &in_addr);
  int err = netlink_add_addr(sock, dev, &in_addr, sizeof(in_addr));
  (void)err;
}

static void netlink_add_addr6(int sock, const char* dev, const char* addr)
{
  struct in6_addr in6_addr;
  inet_pton(AF_INET6, addr, &in6_addr);
  int err = netlink_add_addr(sock, dev, &in6_addr, sizeof(in6_addr));
  (void)err;
}

static void netlink_add_neigh(int sock, const char* name, const void* addr,
                              int addrsize, const void* mac, int macsize)
{
  struct ndmsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  hdr.ndm_family = addrsize == 4 ? AF_INET : AF_INET6;
  hdr.ndm_ifindex = if_nametoindex(name);
  hdr.ndm_state = NUD_PERMANENT;
  netlink_init(RTM_NEWNEIGH, NLM_F_EXCL | NLM_F_CREATE, &hdr, sizeof(hdr));
  netlink_attr(NDA_DST, addr, addrsize);
  netlink_attr(NDA_LLADDR, mac, macsize);
  int err = netlink_send(sock);
  (void)err;
}

static int tunfd = -1;
static int tun_frags_enabled;
#define SYZ_TUN_MAX_PACKET_SIZE 1000

#define TUN_IFACE "syz_tun"

#define LOCAL_MAC 0xaaaaaaaaaaaa
#define REMOTE_MAC 0xaaaaaaaaaabb

#define LOCAL_IPV4 "172.20.20.170"
#define REMOTE_IPV4 "172.20.20.187"

#define LOCAL_IPV6 "fe80::aa"
#define REMOTE_IPV6 "fe80::bb"

#define IFF_NAPI 0x0010
#define IFF_NAPI_FRAGS 0x0020

static void initialize_tun(void)
{
  tunfd = open("/dev/net/tun", O_RDWR | O_NONBLOCK);
  if (tunfd == -1) {
    printf("tun: can't open /dev/net/tun: please enable CONFIG_TUN=y\n");
    printf("otherwise fuzzing or reproducing might not work as intended\n");
    return;
  }
  const int kTunFd = 240;
  if (dup2(tunfd, kTunFd) < 0)
    exit(1);
  close(tunfd);
  tunfd = kTunFd;
  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, TUN_IFACE, IFNAMSIZ);
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI | IFF_NAPI | IFF_NAPI_FRAGS;
  if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0) {
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0)
      exit(1);
  }
  if (ioctl(tunfd, TUNGETIFF, (void*)&ifr) < 0)
    exit(1);
  tun_frags_enabled = (ifr.ifr_flags & IFF_NAPI_FRAGS) != 0;
  char sysctl[64];
  sprintf(sysctl, "/proc/sys/net/ipv6/conf/%s/accept_dad", TUN_IFACE);
  write_file(sysctl, "0");
  sprintf(sysctl, "/proc/sys/net/ipv6/conf/%s/router_solicitations", TUN_IFACE);
  write_file(sysctl, "0");
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sock == -1)
    exit(1);
  netlink_add_addr4(sock, TUN_IFACE, LOCAL_IPV4);
  netlink_add_addr6(sock, TUN_IFACE, LOCAL_IPV6);
  uint64_t macaddr = REMOTE_MAC;
  struct in_addr in_addr;
  inet_pton(AF_INET, REMOTE_IPV4, &in_addr);
  netlink_add_neigh(sock, TUN_IFACE, &in_addr, sizeof(in_addr), &macaddr,
                    ETH_ALEN);
  struct in6_addr in6_addr;
  inet_pton(AF_INET6, REMOTE_IPV6, &in6_addr);
  netlink_add_neigh(sock, TUN_IFACE, &in6_addr, sizeof(in6_addr), &macaddr,
                    ETH_ALEN);
  macaddr = LOCAL_MAC;
  netlink_device_change(sock, TUN_IFACE, true, 0, &macaddr, ETH_ALEN);
  close(sock);
}

static int read_tun(char* data, int size)
{
  if (tunfd < 0)
    return -1;
  int rv = read(tunfd, data, size);
  if (rv < 0) {
    if (errno == EAGAIN)
      return -1;
    if (errno == EBADFD)
      return -1;
    exit(1);
  }
  return rv;
}

static void flush_tun()
{
  char data[SYZ_TUN_MAX_PACKET_SIZE];
  while (read_tun(&data[0], sizeof(data)) != -1) {
  }
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
    NONFAILING(strncpy(buf, (char*)a0, sizeof(buf) - 1));
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

static void setup_cgroups()
{
  if (mkdir("/syzcgroup", 0777)) {
  }
  if (mkdir("/syzcgroup/unified", 0777)) {
  }
  if (mount("none", "/syzcgroup/unified", "cgroup2", 0, NULL)) {
  }
  if (chmod("/syzcgroup/unified", 0777)) {
  }
  write_file("/syzcgroup/unified/cgroup.subtree_control",
             "+cpu +memory +io +pids +rdma");
  if (mkdir("/syzcgroup/cpu", 0777)) {
  }
  if (mount("none", "/syzcgroup/cpu", "cgroup", 0,
            "cpuset,cpuacct,perf_event,hugetlb")) {
  }
  write_file("/syzcgroup/cpu/cgroup.clone_children", "1");
  if (chmod("/syzcgroup/cpu", 0777)) {
  }
  if (mkdir("/syzcgroup/net", 0777)) {
  }
  if (mount("none", "/syzcgroup/net", "cgroup", 0,
            "net_cls,net_prio,devices,freezer")) {
  }
  if (chmod("/syzcgroup/net", 0777)) {
  }
}

static void setup_cgroups_loop()
{
  int pid = getpid();
  char file[128];
  char cgroupdir[64];
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/unified/syz%llu", procid);
  if (mkdir(cgroupdir, 0777)) {
  }
  snprintf(file, sizeof(file), "%s/pids.max", cgroupdir);
  write_file(file, "32");
  snprintf(file, sizeof(file), "%s/memory.low", cgroupdir);
  write_file(file, "%d", 298 << 20);
  snprintf(file, sizeof(file), "%s/memory.high", cgroupdir);
  write_file(file, "%d", 299 << 20);
  snprintf(file, sizeof(file), "%s/memory.max", cgroupdir);
  write_file(file, "%d", 300 << 20);
  snprintf(file, sizeof(file), "%s/cgroup.procs", cgroupdir);
  write_file(file, "%d", pid);
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/cpu/syz%llu", procid);
  if (mkdir(cgroupdir, 0777)) {
  }
  snprintf(file, sizeof(file), "%s/cgroup.procs", cgroupdir);
  write_file(file, "%d", pid);
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/net/syz%llu", procid);
  if (mkdir(cgroupdir, 0777)) {
  }
  snprintf(file, sizeof(file), "%s/cgroup.procs", cgroupdir);
  write_file(file, "%d", pid);
}

static void setup_cgroups_test()
{
  char cgroupdir[64];
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/unified/syz%llu", procid);
  if (symlink(cgroupdir, "./cgroup")) {
  }
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/cpu/syz%llu", procid);
  if (symlink(cgroupdir, "./cgroup.cpu")) {
  }
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/net/syz%llu", procid);
  if (symlink(cgroupdir, "./cgroup.net")) {
  }
}

static void setup_common()
{
  if (mount(0, "/sys/fs/fuse/connections", "fusectl", 0, 0)) {
  }
  setup_cgroups();
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

int wait_for_loop(int pid)
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
  initialize_tun();
  loop();
  exit(1);
}

#define FS_IOC_SETFLAGS _IOW('f', 2, long)
static void remove_dir(const char* dir)
{
  DIR* dp;
  struct dirent* ep;
  int iter = 0;
retry:
  while (umount2(dir, MNT_DETACH) == 0) {
  }
  dp = opendir(dir);
  if (dp == NULL) {
    if (errno == EMFILE) {
      exit(1);
    }
    exit(1);
  }
  while ((ep = readdir(dp))) {
    if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
      continue;
    char filename[FILENAME_MAX];
    snprintf(filename, sizeof(filename), "%s/%s", dir, ep->d_name);
    while (umount2(filename, MNT_DETACH) == 0) {
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
          if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == 0)
            close(fd);
          continue;
        }
      }
      if (errno == EROFS) {
        break;
      }
      if (errno != EBUSY || i > 100)
        exit(1);
      if (umount2(filename, MNT_DETACH))
        exit(1);
    }
  }
  closedir(dp);
  int i;
  for (i = 0;; i++) {
    if (rmdir(dir) == 0)
      break;
    if (i < 100) {
      if (errno == EPERM) {
        int fd = open(dir, O_RDONLY);
        if (fd != -1) {
          long flags = 0;
          if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == 0)
            close(fd);
          continue;
        }
      }
      if (errno == EROFS) {
        break;
      }
      if (errno == EBUSY) {
        if (umount2(dir, MNT_DETACH))
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
  int i;
  for (i = 0; i < 100; i++) {
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

static void setup_loop()
{
  setup_cgroups_loop();
}

static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setup_cgroups_test();
  write_file("/proc/self/oom_score_adj", "1000");
  flush_tun();
}

static void close_fds()
{
  int fd;
  for (fd = 3; fd < 30; fd++)
    close(fd);
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  setup_loop();
  int iter;
  for (iter = 0;; iter++) {
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
      if (current_time_ms() - start < 5 * 1000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
    remove_dir(cwdbuf);
  }
}

uint64_t r[1] = {0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  NONFAILING(memcpy((void*)0x20000040, "/dev/uinput\000", 12));
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000040, 2, 0);
  if (res != -1)
    r[0] = res;
  NONFAILING(memcpy((void*)0x20000880,
                    "syz1\000\000\000\000\000\000\000\000\000\000\000\000\000"
                    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
                    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
                    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
                    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
                    "\000\000\000\000\000\000\000",
                    80));
  NONFAILING(*(uint16_t*)0x200008d0 = 0);
  NONFAILING(*(uint16_t*)0x200008d2 = 0);
  NONFAILING(*(uint16_t*)0x200008d4 = 0);
  NONFAILING(*(uint16_t*)0x200008d6 = 0);
  NONFAILING(*(uint32_t*)0x200008d8 = 0);
  NONFAILING(*(uint32_t*)0x200008dc = 0);
  NONFAILING(*(uint32_t*)0x200008e0 = 0);
  NONFAILING(*(uint32_t*)0x200008e4 = 0);
  NONFAILING(*(uint32_t*)0x200008e8 = 0);
  NONFAILING(*(uint32_t*)0x200008ec = 0);
  NONFAILING(*(uint32_t*)0x200008f0 = 0);
  NONFAILING(*(uint32_t*)0x200008f4 = 0);
  NONFAILING(*(uint32_t*)0x200008f8 = -1);
  NONFAILING(*(uint32_t*)0x200008fc = 0);
  NONFAILING(*(uint32_t*)0x20000900 = 0);
  NONFAILING(*(uint32_t*)0x20000904 = 0);
  NONFAILING(*(uint32_t*)0x20000908 = 0);
  NONFAILING(*(uint32_t*)0x2000090c = 0);
  NONFAILING(*(uint32_t*)0x20000910 = 0);
  NONFAILING(*(uint32_t*)0x20000914 = 0);
  NONFAILING(*(uint32_t*)0x20000918 = 0);
  NONFAILING(*(uint32_t*)0x2000091c = 0);
  NONFAILING(*(uint32_t*)0x20000920 = 0);
  NONFAILING(*(uint32_t*)0x20000924 = 0);
  NONFAILING(*(uint32_t*)0x20000928 = 0);
  NONFAILING(*(uint32_t*)0x2000092c = 0);
  NONFAILING(*(uint32_t*)0x20000930 = 0);
  NONFAILING(*(uint32_t*)0x20000934 = 0);
  NONFAILING(*(uint32_t*)0x20000938 = 5);
  NONFAILING(*(uint32_t*)0x2000093c = 0);
  NONFAILING(*(uint32_t*)0x20000940 = 0);
  NONFAILING(*(uint32_t*)0x20000944 = 0);
  NONFAILING(*(uint32_t*)0x20000948 = 0);
  NONFAILING(*(uint32_t*)0x2000094c = 0);
  NONFAILING(*(uint32_t*)0x20000950 = 0);
  NONFAILING(*(uint32_t*)0x20000954 = 0);
  NONFAILING(*(uint32_t*)0x20000958 = 0);
  NONFAILING(*(uint32_t*)0x2000095c = 0);
  NONFAILING(*(uint32_t*)0x20000960 = 0);
  NONFAILING(*(uint32_t*)0x20000964 = 0);
  NONFAILING(*(uint32_t*)0x20000968 = 0);
  NONFAILING(*(uint32_t*)0x2000096c = 0);
  NONFAILING(*(uint32_t*)0x20000970 = 0);
  NONFAILING(*(uint32_t*)0x20000974 = 0);
  NONFAILING(*(uint32_t*)0x20000978 = 0);
  NONFAILING(*(uint32_t*)0x2000097c = 0);
  NONFAILING(*(uint32_t*)0x20000980 = 0);
  NONFAILING(*(uint32_t*)0x20000984 = 0);
  NONFAILING(*(uint32_t*)0x20000988 = 0);
  NONFAILING(*(uint32_t*)0x2000098c = 0);
  NONFAILING(*(uint32_t*)0x20000990 = 0);
  NONFAILING(*(uint32_t*)0x20000994 = 0);
  NONFAILING(*(uint32_t*)0x20000998 = 0);
  NONFAILING(*(uint32_t*)0x2000099c = 0);
  NONFAILING(*(uint32_t*)0x200009a0 = 0);
  NONFAILING(*(uint32_t*)0x200009a4 = 0);
  NONFAILING(*(uint32_t*)0x200009a8 = 0);
  NONFAILING(*(uint32_t*)0x200009ac = 0);
  NONFAILING(*(uint32_t*)0x200009b0 = 0);
  NONFAILING(*(uint32_t*)0x200009b4 = 0);
  NONFAILING(*(uint32_t*)0x200009b8 = 0);
  NONFAILING(*(uint32_t*)0x200009bc = 0);
  NONFAILING(*(uint32_t*)0x200009c0 = 0);
  NONFAILING(*(uint32_t*)0x200009c4 = 0);
  NONFAILING(*(uint32_t*)0x200009c8 = 0);
  NONFAILING(*(uint32_t*)0x200009cc = 0);
  NONFAILING(*(uint32_t*)0x200009d0 = 0);
  NONFAILING(*(uint32_t*)0x200009d4 = 0);
  NONFAILING(*(uint32_t*)0x200009d8 = 0);
  NONFAILING(*(uint32_t*)0x200009dc = 0);
  NONFAILING(*(uint32_t*)0x200009e0 = 0);
  NONFAILING(*(uint32_t*)0x200009e4 = 0);
  NONFAILING(*(uint32_t*)0x200009e8 = 0);
  NONFAILING(*(uint32_t*)0x200009ec = 0);
  NONFAILING(*(uint32_t*)0x200009f0 = 0);
  NONFAILING(*(uint32_t*)0x200009f4 = 0);
  NONFAILING(*(uint32_t*)0x200009f8 = 0);
  NONFAILING(*(uint32_t*)0x200009fc = 4);
  NONFAILING(*(uint32_t*)0x20000a00 = 0);
  NONFAILING(*(uint32_t*)0x20000a04 = 0);
  NONFAILING(*(uint32_t*)0x20000a08 = 0);
  NONFAILING(*(uint32_t*)0x20000a0c = 0);
  NONFAILING(*(uint32_t*)0x20000a10 = 0);
  NONFAILING(*(uint32_t*)0x20000a14 = 0);
  NONFAILING(*(uint32_t*)0x20000a18 = 0);
  NONFAILING(*(uint32_t*)0x20000a1c = 0);
  NONFAILING(*(uint32_t*)0x20000a20 = 0);
  NONFAILING(*(uint32_t*)0x20000a24 = 0);
  NONFAILING(*(uint32_t*)0x20000a28 = 0);
  NONFAILING(*(uint32_t*)0x20000a2c = 0);
  NONFAILING(*(uint32_t*)0x20000a30 = 0);
  NONFAILING(*(uint32_t*)0x20000a34 = 0);
  NONFAILING(*(uint32_t*)0x20000a38 = 0);
  NONFAILING(*(uint32_t*)0x20000a3c = 0);
  NONFAILING(*(uint32_t*)0x20000a40 = 0);
  NONFAILING(*(uint32_t*)0x20000a44 = 0);
  NONFAILING(*(uint32_t*)0x20000a48 = 0);
  NONFAILING(*(uint32_t*)0x20000a4c = 0);
  NONFAILING(*(uint32_t*)0x20000a50 = 0);
  NONFAILING(*(uint32_t*)0x20000a54 = 0);
  NONFAILING(*(uint32_t*)0x20000a58 = 0);
  NONFAILING(*(uint32_t*)0x20000a5c = 0);
  NONFAILING(*(uint32_t*)0x20000a60 = 0);
  NONFAILING(*(uint32_t*)0x20000a64 = 0);
  NONFAILING(*(uint32_t*)0x20000a68 = 0);
  NONFAILING(*(uint32_t*)0x20000a6c = 0);
  NONFAILING(*(uint32_t*)0x20000a70 = 0);
  NONFAILING(*(uint32_t*)0x20000a74 = 0);
  NONFAILING(*(uint32_t*)0x20000a78 = 0);
  NONFAILING(*(uint32_t*)0x20000a7c = 0);
  NONFAILING(*(uint32_t*)0x20000a80 = 0);
  NONFAILING(*(uint32_t*)0x20000a84 = 0);
  NONFAILING(*(uint32_t*)0x20000a88 = 0);
  NONFAILING(*(uint32_t*)0x20000a8c = 0);
  NONFAILING(*(uint32_t*)0x20000a90 = 0);
  NONFAILING(*(uint32_t*)0x20000a94 = 0);
  NONFAILING(*(uint32_t*)0x20000a98 = 0);
  NONFAILING(*(uint32_t*)0x20000a9c = 0);
  NONFAILING(*(uint32_t*)0x20000aa0 = 0);
  NONFAILING(*(uint32_t*)0x20000aa4 = 0);
  NONFAILING(*(uint32_t*)0x20000aa8 = 0);
  NONFAILING(*(uint32_t*)0x20000aac = 0);
  NONFAILING(*(uint32_t*)0x20000ab0 = 0);
  NONFAILING(*(uint32_t*)0x20000ab4 = 0);
  NONFAILING(*(uint32_t*)0x20000ab8 = 0);
  NONFAILING(*(uint32_t*)0x20000abc = 0);
  NONFAILING(*(uint32_t*)0x20000ac0 = 0);
  NONFAILING(*(uint32_t*)0x20000ac4 = 0);
  NONFAILING(*(uint32_t*)0x20000ac8 = 0);
  NONFAILING(*(uint32_t*)0x20000acc = 0);
  NONFAILING(*(uint32_t*)0x20000ad0 = 0);
  NONFAILING(*(uint32_t*)0x20000ad4 = 0);
  NONFAILING(*(uint32_t*)0x20000ad8 = 0);
  NONFAILING(*(uint32_t*)0x20000adc = 0);
  NONFAILING(*(uint32_t*)0x20000ae0 = 2);
  NONFAILING(*(uint32_t*)0x20000ae4 = 0);
  NONFAILING(*(uint32_t*)0x20000ae8 = 0);
  NONFAILING(*(uint32_t*)0x20000aec = 0);
  NONFAILING(*(uint32_t*)0x20000af0 = 0);
  NONFAILING(*(uint32_t*)0x20000af4 = 0);
  NONFAILING(*(uint32_t*)0x20000af8 = 0);
  NONFAILING(*(uint32_t*)0x20000afc = 0);
  NONFAILING(*(uint32_t*)0x20000b00 = 0);
  NONFAILING(*(uint32_t*)0x20000b04 = 0);
  NONFAILING(*(uint32_t*)0x20000b08 = 0);
  NONFAILING(*(uint32_t*)0x20000b0c = 0);
  NONFAILING(*(uint32_t*)0x20000b10 = 0);
  NONFAILING(*(uint32_t*)0x20000b14 = 0);
  NONFAILING(*(uint32_t*)0x20000b18 = 0);
  NONFAILING(*(uint32_t*)0x20000b1c = 0);
  NONFAILING(*(uint32_t*)0x20000b20 = 0);
  NONFAILING(*(uint32_t*)0x20000b24 = 0);
  NONFAILING(*(uint32_t*)0x20000b28 = 0);
  NONFAILING(*(uint32_t*)0x20000b2c = 0);
  NONFAILING(*(uint32_t*)0x20000b30 = 0);
  NONFAILING(*(uint32_t*)0x20000b34 = 0);
  NONFAILING(*(uint32_t*)0x20000b38 = 0);
  NONFAILING(*(uint32_t*)0x20000b3c = 0);
  NONFAILING(*(uint32_t*)0x20000b40 = 0);
  NONFAILING(*(uint32_t*)0x20000b44 = 0);
  NONFAILING(*(uint32_t*)0x20000b48 = 0);
  NONFAILING(*(uint32_t*)0x20000b4c = 0);
  NONFAILING(*(uint32_t*)0x20000b50 = 0);
  NONFAILING(*(uint32_t*)0x20000b54 = 0);
  NONFAILING(*(uint32_t*)0x20000b58 = 0);
  NONFAILING(*(uint32_t*)0x20000b5c = 0);
  NONFAILING(*(uint32_t*)0x20000b60 = 0);
  NONFAILING(*(uint32_t*)0x20000b64 = 0);
  NONFAILING(*(uint32_t*)0x20000b68 = 0);
  NONFAILING(*(uint32_t*)0x20000b6c = 0xfffffffe);
  NONFAILING(*(uint32_t*)0x20000b70 = 0);
  NONFAILING(*(uint32_t*)0x20000b74 = 0);
  NONFAILING(*(uint32_t*)0x20000b78 = 0);
  NONFAILING(*(uint32_t*)0x20000b7c = 0);
  NONFAILING(*(uint32_t*)0x20000b80 = 0);
  NONFAILING(*(uint32_t*)0x20000b84 = 2);
  NONFAILING(*(uint32_t*)0x20000b88 = 0);
  NONFAILING(*(uint32_t*)0x20000b8c = 0);
  NONFAILING(*(uint32_t*)0x20000b90 = 0);
  NONFAILING(*(uint32_t*)0x20000b94 = 0);
  NONFAILING(*(uint32_t*)0x20000b98 = 0);
  NONFAILING(*(uint32_t*)0x20000b9c = 0);
  NONFAILING(*(uint32_t*)0x20000ba0 = 0);
  NONFAILING(*(uint32_t*)0x20000ba4 = 0);
  NONFAILING(*(uint32_t*)0x20000ba8 = 0);
  NONFAILING(*(uint32_t*)0x20000bac = 0);
  NONFAILING(*(uint32_t*)0x20000bb0 = 0);
  NONFAILING(*(uint32_t*)0x20000bb4 = 0);
  NONFAILING(*(uint32_t*)0x20000bb8 = 0);
  NONFAILING(*(uint32_t*)0x20000bbc = 0);
  NONFAILING(*(uint32_t*)0x20000bc0 = 0);
  NONFAILING(*(uint32_t*)0x20000bc4 = 0);
  NONFAILING(*(uint32_t*)0x20000bc8 = 0);
  NONFAILING(*(uint32_t*)0x20000bcc = 0);
  NONFAILING(*(uint32_t*)0x20000bd0 = 0);
  NONFAILING(*(uint32_t*)0x20000bd4 = 0);
  NONFAILING(*(uint32_t*)0x20000bd8 = 0);
  NONFAILING(*(uint32_t*)0x20000bdc = 0);
  NONFAILING(*(uint32_t*)0x20000be0 = 0);
  NONFAILING(*(uint32_t*)0x20000be4 = 0);
  NONFAILING(*(uint32_t*)0x20000be8 = 0);
  NONFAILING(*(uint32_t*)0x20000bec = 0);
  NONFAILING(*(uint32_t*)0x20000bf0 = 0);
  NONFAILING(*(uint32_t*)0x20000bf4 = 0);
  NONFAILING(*(uint32_t*)0x20000bf8 = 0);
  NONFAILING(*(uint32_t*)0x20000bfc = 0);
  NONFAILING(*(uint32_t*)0x20000c00 = 0);
  NONFAILING(*(uint32_t*)0x20000c04 = 0);
  NONFAILING(*(uint32_t*)0x20000c08 = 0);
  NONFAILING(*(uint32_t*)0x20000c0c = 0);
  NONFAILING(*(uint32_t*)0x20000c10 = 0);
  NONFAILING(*(uint32_t*)0x20000c14 = 0);
  NONFAILING(*(uint32_t*)0x20000c18 = 0);
  NONFAILING(*(uint32_t*)0x20000c1c = 0);
  NONFAILING(*(uint32_t*)0x20000c20 = 0);
  NONFAILING(*(uint32_t*)0x20000c24 = 0);
  NONFAILING(*(uint32_t*)0x20000c28 = 0);
  NONFAILING(*(uint32_t*)0x20000c2c = 0);
  NONFAILING(*(uint32_t*)0x20000c30 = 0);
  NONFAILING(*(uint32_t*)0x20000c34 = 0);
  NONFAILING(*(uint32_t*)0x20000c38 = 0);
  NONFAILING(*(uint32_t*)0x20000c3c = 0);
  NONFAILING(*(uint32_t*)0x20000c40 = 0xfffffffc);
  NONFAILING(*(uint32_t*)0x20000c44 = 0);
  NONFAILING(*(uint32_t*)0x20000c48 = 0);
  NONFAILING(*(uint32_t*)0x20000c4c = 0);
  NONFAILING(*(uint32_t*)0x20000c50 = 0);
  NONFAILING(*(uint32_t*)0x20000c54 = 0);
  NONFAILING(*(uint32_t*)0x20000c58 = 0);
  NONFAILING(*(uint32_t*)0x20000c5c = 0);
  NONFAILING(*(uint32_t*)0x20000c60 = 0);
  NONFAILING(*(uint32_t*)0x20000c64 = 0);
  NONFAILING(*(uint32_t*)0x20000c68 = 0);
  NONFAILING(*(uint32_t*)0x20000c6c = 0);
  NONFAILING(*(uint32_t*)0x20000c70 = 0);
  NONFAILING(*(uint32_t*)0x20000c74 = 0);
  NONFAILING(*(uint32_t*)0x20000c78 = 0);
  NONFAILING(*(uint32_t*)0x20000c7c = 0);
  NONFAILING(*(uint32_t*)0x20000c80 = 0);
  NONFAILING(*(uint32_t*)0x20000c84 = 0);
  NONFAILING(*(uint32_t*)0x20000c88 = 0);
  NONFAILING(*(uint32_t*)0x20000c8c = 0);
  NONFAILING(*(uint32_t*)0x20000c90 = 0);
  NONFAILING(*(uint32_t*)0x20000c94 = 0);
  NONFAILING(*(uint32_t*)0x20000c98 = 0);
  NONFAILING(*(uint32_t*)0x20000c9c = 0);
  NONFAILING(*(uint32_t*)0x20000ca0 = 0);
  NONFAILING(*(uint32_t*)0x20000ca4 = 0);
  NONFAILING(*(uint32_t*)0x20000ca8 = 0);
  NONFAILING(*(uint32_t*)0x20000cac = 0);
  NONFAILING(*(uint32_t*)0x20000cb0 = 0);
  NONFAILING(*(uint32_t*)0x20000cb4 = 0);
  NONFAILING(*(uint32_t*)0x20000cb8 = 0);
  NONFAILING(*(uint32_t*)0x20000cbc = 0);
  NONFAILING(*(uint32_t*)0x20000cc0 = 0);
  NONFAILING(*(uint32_t*)0x20000cc4 = 0);
  NONFAILING(*(uint32_t*)0x20000cc8 = 0);
  NONFAILING(*(uint32_t*)0x20000ccc = 0);
  NONFAILING(*(uint32_t*)0x20000cd0 = 0);
  NONFAILING(*(uint32_t*)0x20000cd4 = 0);
  NONFAILING(*(uint32_t*)0x20000cd8 = 0);
  syscall(__NR_write, r[0], 0x20000880, 0x45c);
  syscall(__NR_ioctl, r[0], 0x5501, 0);
  NONFAILING(memcpy((void*)0x20000e00, "/dev/input/event#\000", 18));
  syz_open_dev(0x20000e00, 8, 0x24440);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  install_segv_handler();
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      use_temporary_dir();
      do_sandbox_none();
    }
  }
  sleep(1000000);
  return 0;
}
