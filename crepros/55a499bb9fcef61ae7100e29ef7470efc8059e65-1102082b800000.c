// https://syzkaller.appspot.com/bug?id=55a499bb9fcef61ae7100e29ef7470efc8059e65
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/if_arp.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <unistd.h>

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

const int kFailStatus = 67;
const int kRetryStatus = 69;

static void fail(const char* msg, ...)
{
  int e = errno;
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit((e == ENOMEM || e == EAGAIN) ? kRetryStatus : kFailStatus);
}

#define BITMASK_LEN(type, bf_len) (type)((1ull << (bf_len)) - 1)

#define BITMASK_LEN_OFF(type, bf_off, bf_len)                                  \
  (type)(BITMASK_LEN(type, (bf_len)) << (bf_off))

#define STORE_BY_BITMASK(type, addr, val, bf_off, bf_len)                      \
  if ((bf_off) == 0 && (bf_len) == 0) {                                        \
    *(type*)(addr) = (type)(val);                                              \
  } else {                                                                     \
    type new_val = *(type*)(addr);                                             \
    new_val &= ~BITMASK_LEN_OFF(type, (bf_off), (bf_len));                     \
    new_val |= ((type)(val)&BITMASK_LEN(type, (bf_len))) << (bf_off);          \
    *(type*)(addr) = new_val;                                                  \
  }

struct csum_inet {
  uint32_t acc;
};

static void csum_inet_init(struct csum_inet* csum)
{
  csum->acc = 0;
}

static void csum_inet_update(struct csum_inet* csum, const uint8_t* data,
                             size_t length)
{
  if (length == 0)
    return;

  size_t i;
  for (i = 0; i < length - 1; i += 2)
    csum->acc += *(uint16_t*)&data[i];

  if (length & 1)
    csum->acc += (uint16_t)data[length - 1];

  while (csum->acc > 0xffff)
    csum->acc = (csum->acc & 0xffff) + (csum->acc >> 16);
}

static uint16_t csum_inet_digest(struct csum_inet* csum)
{
  return ~csum->acc;
}

static void vsnprintf_check(char* str, size_t size, const char* format,
                            va_list args)
{
  int rv;

  rv = vsnprintf(str, size, format, args);
  if (rv < 0)
    fail("tun: snprintf failed");
  if ((size_t)rv >= size)
    fail("tun: string '%s...' doesn't fit into buffer", str);
}

static void snprintf_check(char* str, size_t size, const char* format, ...)
{
  va_list args;

  va_start(args, format);
  vsnprintf_check(str, size, format, args);
  va_end(args);
}

#define COMMAND_MAX_LEN 128
#define PATH_PREFIX                                                            \
  "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin "
#define PATH_PREFIX_LEN (sizeof(PATH_PREFIX) - 1)

static void execute_command(bool panic, const char* format, ...)
{
  va_list args;
  char command[PATH_PREFIX_LEN + COMMAND_MAX_LEN];
  int rv;

  va_start(args, format);
  memcpy(command, PATH_PREFIX, PATH_PREFIX_LEN);
  vsnprintf_check(command + PATH_PREFIX_LEN, COMMAND_MAX_LEN, format, args);
  rv = system(command);
  if (panic && rv != 0)
    fail("tun: command \"%s\" failed with code %d", &command[0], rv);

  va_end(args);
}

static int tunfd = -1;
static int tun_frags_enabled;

#define SYZ_TUN_MAX_PACKET_SIZE 1000

#define TUN_IFACE "syz_tun"

#define LOCAL_MAC "aa:aa:aa:aa:aa:aa"
#define REMOTE_MAC "aa:aa:aa:aa:aa:bb"

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
  const int kTunFd = 252;
  if (dup2(tunfd, kTunFd) < 0)
    fail("dup2(tunfd, kTunFd) failed");
  close(tunfd);
  tunfd = kTunFd;

  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, TUN_IFACE, IFNAMSIZ);
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI | IFF_NAPI | IFF_NAPI_FRAGS;
  if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0) {
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0)
      fail("tun: ioctl(TUNSETIFF) failed");
  }
  if (ioctl(tunfd, TUNGETIFF, (void*)&ifr) < 0)
    fail("tun: ioctl(TUNGETIFF) failed");
  tun_frags_enabled = (ifr.ifr_flags & IFF_NAPI_FRAGS) != 0;

  execute_command(1, "sysctl -w net.ipv6.conf.%s.accept_dad=0", TUN_IFACE);

  execute_command(1, "sysctl -w net.ipv6.conf.%s.router_solicitations=0",
                  TUN_IFACE);

  execute_command(1, "ip link set dev %s address %s", TUN_IFACE, LOCAL_MAC);
  execute_command(1, "ip addr add %s/24 dev %s", LOCAL_IPV4, TUN_IFACE);
  execute_command(1, "ip -6 addr add %s/120 dev %s", LOCAL_IPV6, TUN_IFACE);
  execute_command(1, "ip neigh add %s lladdr %s dev %s nud permanent",
                  REMOTE_IPV4, REMOTE_MAC, TUN_IFACE);
  execute_command(1, "ip -6 neigh add %s lladdr %s dev %s nud permanent",
                  REMOTE_IPV6, REMOTE_MAC, TUN_IFACE);
  execute_command(1, "ip link set dev %s up", TUN_IFACE);
}

#define DEV_IPV4 "172.20.20.%d"
#define DEV_IPV6 "fe80::%02hx"
#define DEV_MAC "aa:aa:aa:aa:aa:%02hx"

static void initialize_netdevices(void)
{
  unsigned i;
  const char* devtypes[] = {"ip6gretap", "bridge", "vcan", "bond", "veth"};
  const char* devnames[] = {"lo",       "sit0",    "bridge0", "vcan0",
                            "tunl0",    "gre0",    "gretap0", "ip_vti0",
                            "ip6_vti0", "ip6tnl0", "ip6gre0", "ip6gretap0",
                            "erspan0",  "bond0",   "veth0",   "veth1"};

  for (i = 0; i < sizeof(devtypes) / (sizeof(devtypes[0])); i++)
    execute_command(0, "ip link add dev %s0 type %s", devtypes[i], devtypes[i]);
  execute_command(0, "ip link add dev veth1 type veth");
  for (i = 0; i < sizeof(devnames) / (sizeof(devnames[0])); i++) {
    char addr[32];
    snprintf_check(addr, sizeof(addr), DEV_IPV4, i + 10);
    execute_command(0, "ip -4 addr add %s/24 dev %s", addr, devnames[i]);
    snprintf_check(addr, sizeof(addr), DEV_IPV6, i + 10);
    execute_command(0, "ip -6 addr add %s/120 dev %s", addr, devnames[i]);
    snprintf_check(addr, sizeof(addr), DEV_MAC, i + 10);
    execute_command(0, "ip link set dev %s address %s", devnames[i], addr);
    execute_command(0, "ip link set dev %s up", devnames[i]);
  }
}

#define MAX_FRAGS 4
struct vnet_fragmentation {
  uint32_t full;
  uint32_t count;
  uint32_t frags[MAX_FRAGS];
};

static uintptr_t syz_emit_ethernet(uintptr_t a0, uintptr_t a1, uintptr_t a2)
{
  if (tunfd < 0)
    return (uintptr_t)-1;

  uint32_t length = a0;
  char* data = (char*)a1;

  struct vnet_fragmentation* frags = (struct vnet_fragmentation*)a2;
  struct iovec vecs[MAX_FRAGS + 1];
  uint32_t nfrags = 0;
  if (!tun_frags_enabled || frags == NULL) {
    vecs[nfrags].iov_base = data;
    vecs[nfrags].iov_len = length;
    nfrags++;
  } else {
    bool full = true;
    uint32_t i, count = 0;
    full = frags->full;
    count = frags->count;
    if (count > MAX_FRAGS)
      count = MAX_FRAGS;
    for (i = 0; i < count && length != 0; i++) {
      uint32_t size = 0;
      size = frags->frags[i];
      if (size > length)
        size = length;
      vecs[nfrags].iov_base = data;
      vecs[nfrags].iov_len = size;
      nfrags++;
      data += size;
      length -= size;
    }
    if (length != 0 && (full || nfrags == 0)) {
      vecs[nfrags].iov_base = data;
      vecs[nfrags].iov_len = length;
      nfrags++;
    }
  }
  return writev(tunfd, vecs, nfrags);
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();

  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = 128 << 20;
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 8 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);

#define CLONE_NEWCGROUP 0x02000000

  if (unshare(CLONE_NEWNS)) {
  }
  if (unshare(CLONE_NEWIPC)) {
  }
  if (unshare(CLONE_NEWCGROUP)) {
  }
  if (unshare(CLONE_NEWUTS)) {
  }
  if (unshare(CLONE_SYSVSEM)) {
  }
}

static int do_sandbox_none(void)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid < 0)
    fail("sandbox fork failed");
  if (pid)
    return pid;

  sandbox_common();
  if (unshare(CLONE_NEWNET)) {
  }
  initialize_tun();
  initialize_netdevices();

  loop();
  doexit(1);
}

uint64_t r[1] = {0xffffffffffffffff};
void loop()
{
  long res;
  res = syscall(__NR_socket, 0x11, 3, 0x300);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000080 = 0x110;
  syscall(__NR_setsockopt, r[0], 1, 0x25, 0x20000080, 4);
  *(uint16_t*)0x20005fe4 = 0xa;
  *(uint16_t*)0x20005fe6 = htobe16(0);
  *(uint32_t*)0x20005fe8 = 9;
  *(uint64_t*)0x20005fec = htobe64(0);
  *(uint64_t*)0x20005ff4 = htobe64(1);
  *(uint32_t*)0x20005ffc = 0;
  syscall(__NR_sendto, r[0], 0x20000000, 0, 0, 0x20005fe4, 0x1c);
  *(uint64_t*)0x20000940 = 0;
  *(uint32_t*)0x20000948 = 0;
  *(uint64_t*)0x20000950 = 0x20000340;
  *(uint64_t*)0x20000958 = 0;
  *(uint64_t*)0x20000960 = 0;
  *(uint64_t*)0x20000968 = 0;
  *(uint32_t*)0x20000970 = 0;
  *(uint32_t*)0x20000978 = 0xfffffffd;
  *(uint64_t*)0x20000980 = 0x20000380;
  *(uint32_t*)0x20000988 = 0x80;
  *(uint64_t*)0x20000990 = 0x20000000;
  *(uint64_t*)0x20000998 = 0;
  *(uint64_t*)0x200009a0 = 0x20000840;
  *(uint64_t*)0x200009a8 = 0xd9;
  *(uint32_t*)0x200009b0 = 0;
  *(uint32_t*)0x200009b8 = 0;
  *(uint64_t*)0x200009c0 = 0x77359400;
  *(uint64_t*)0x200009c8 = 0;
  syscall(__NR_recvmmsg, r[0], 0x20000940, 2, 0x12000, 0x200009c0);
  memcpy((void*)0x20005faa, "\xff\x39\x97\xff\xcd\x00", 6);
  *(uint8_t*)0x20005fb0 = 0;
  *(uint8_t*)0x20005fb1 = 0;
  *(uint8_t*)0x20005fb2 = 0;
  *(uint8_t*)0x20005fb3 = 0;
  *(uint8_t*)0x20005fb4 = 0;
  *(uint8_t*)0x20005fb5 = 0;
  *(uint16_t*)0x20005fb6 = htobe16(0x86dd);
  STORE_BY_BITMASK(uint8_t, 0x20005fb8, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x20005fb8, 6, 4, 4);
  memcpy((void*)0x20005fb9, "\xc8\xfb\xa2", 3);
  *(uint16_t*)0x20005fbc = htobe16(0x18);
  *(uint8_t*)0x20005fbe = 0x3a;
  *(uint8_t*)0x20005fbf = 0;
  *(uint8_t*)0x20005fc0 = 0;
  *(uint8_t*)0x20005fc1 = 0;
  *(uint8_t*)0x20005fc2 = 0;
  *(uint8_t*)0x20005fc3 = 0;
  *(uint8_t*)0x20005fc4 = 0;
  *(uint8_t*)0x20005fc5 = 0;
  *(uint8_t*)0x20005fc6 = 0;
  *(uint8_t*)0x20005fc7 = 0;
  *(uint8_t*)0x20005fc8 = 0;
  *(uint8_t*)0x20005fc9 = 0;
  *(uint8_t*)0x20005fca = 0;
  *(uint8_t*)0x20005fcb = 0;
  *(uint8_t*)0x20005fcc = 0;
  *(uint8_t*)0x20005fcd = 0;
  *(uint8_t*)0x20005fce = 0;
  *(uint8_t*)0x20005fcf = 0;
  *(uint8_t*)0x20005fd0 = -1;
  *(uint8_t*)0x20005fd1 = 2;
  *(uint8_t*)0x20005fd2 = 0;
  *(uint8_t*)0x20005fd3 = 0;
  *(uint8_t*)0x20005fd4 = 0;
  *(uint8_t*)0x20005fd5 = 0;
  *(uint8_t*)0x20005fd6 = 0;
  *(uint8_t*)0x20005fd7 = 0;
  *(uint8_t*)0x20005fd8 = 0;
  *(uint8_t*)0x20005fd9 = 0;
  *(uint8_t*)0x20005fda = 0;
  *(uint8_t*)0x20005fdb = 0;
  *(uint8_t*)0x20005fdc = 0;
  *(uint8_t*)0x20005fdd = 0;
  *(uint8_t*)0x20005fde = 0;
  *(uint8_t*)0x20005fdf = 1;
  *(uint8_t*)0x20005fe0 = 0x8c;
  *(uint8_t*)0x20005fe1 = 0xd;
  *(uint16_t*)0x20005fe2 = 0;
  *(uint16_t*)0x20005fe4 = htobe16(0);
  *(uint16_t*)0x20005fe6 = 0;
  *(uint8_t*)0x20005fe8 = -1;
  *(uint8_t*)0x20005fe9 = 2;
  *(uint8_t*)0x20005fea = 0;
  *(uint8_t*)0x20005feb = 0;
  *(uint8_t*)0x20005fec = 0;
  *(uint8_t*)0x20005fed = 0;
  *(uint8_t*)0x20005fee = 0;
  *(uint8_t*)0x20005fef = 0;
  *(uint8_t*)0x20005ff0 = 0;
  *(uint8_t*)0x20005ff1 = 0;
  *(uint8_t*)0x20005ff2 = 0;
  *(uint8_t*)0x20005ff3 = 0;
  *(uint8_t*)0x20005ff4 = 0;
  *(uint8_t*)0x20005ff5 = 0;
  *(uint8_t*)0x20005ff6 = 0;
  *(uint8_t*)0x20005ff7 = 1;
  struct csum_inet csum_1;
  csum_inet_init(&csum_1);
  csum_inet_update(&csum_1, (const uint8_t*)0x20005fc0, 16);
  csum_inet_update(&csum_1, (const uint8_t*)0x20005fd0, 16);
  uint32_t csum_1_chunk_2 = 0x18000000;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_2, 4);
  uint32_t csum_1_chunk_3 = 0x3a000000;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_3, 4);
  csum_inet_update(&csum_1, (const uint8_t*)0x20005fe0, 24);
  *(uint16_t*)0x20005fe2 = csum_inet_digest(&csum_1);
  syz_emit_ethernet(0x4e, 0x20005faa, 0);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  int pid = do_sandbox_none();
  int status = 0;
  while (waitpid(pid, &status, __WALL) != pid) {
  }
  return 0;
}
