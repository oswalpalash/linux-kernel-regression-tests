// https://syzkaller.appspot.com/bug?id=0540b60313a5559f649e9940e45c9a320c885806
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
  va_end(args);
  rv = system(command);
  if (rv) {
    if (panic)
      fail("command '%s' failed: %d", &command[0], rv);
  }
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
  const char* devtypes[] = {"ip6gretap", "bridge", "vcan", "bond", "team"};
  const char* devnames[] = {"lo",
                            "sit0",
                            "bridge0",
                            "vcan0",
                            "tunl0",
                            "gre0",
                            "gretap0",
                            "ip_vti0",
                            "ip6_vti0",
                            "ip6tnl0",
                            "ip6gre0",
                            "ip6gretap0",
                            "erspan0",
                            "bond0",
                            "veth0",
                            "veth1",
                            "team0",
                            "veth0_to_bridge",
                            "veth1_to_bridge",
                            "veth0_to_bond",
                            "veth1_to_bond",
                            "veth0_to_team",
                            "veth1_to_team"};
  const char* devmasters[] = {"bridge", "bond", "team"};

  for (i = 0; i < sizeof(devtypes) / (sizeof(devtypes[0])); i++)
    execute_command(0, "ip link add dev %s0 type %s", devtypes[i], devtypes[i]);
  execute_command(0, "ip link add type veth");

  for (i = 0; i < sizeof(devmasters) / (sizeof(devmasters[0])); i++) {
    execute_command(
        0, "ip link add name %s_slave_0 type veth peer name veth0_to_%s",
        devmasters[i], devmasters[i]);
    execute_command(
        0, "ip link add name %s_slave_1 type veth peer name veth1_to_%s",
        devmasters[i], devmasters[i]);
    execute_command(0, "ip link set %s_slave_0 master %s0", devmasters[i],
                    devmasters[i]);
    execute_command(0, "ip link set %s_slave_1 master %s0", devmasters[i],
                    devmasters[i]);
    execute_command(0, "ip link set veth0_to_%s up", devmasters[i]);
    execute_command(0, "ip link set veth1_to_%s up", devmasters[i]);
  }
  execute_command(0, "ip link set bridge_slave_0 up");
  execute_command(0, "ip link set bridge_slave_1 up");

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
  rlim.rlim_cur = rlim.rlim_max = 32 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);

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
  long res = 0;
  res = syscall(__NR_socket, 0x11, 0x40000000000803, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000200 = 0x104;
  syscall(__NR_setsockopt, r[0], 1, 0x25, 0x20000200, 4);
  *(uint64_t*)0x2087dfc8 = 0x2005e000;
  *(uint16_t*)0x2005e000 = 0x27;
  *(uint32_t*)0x2005e004 = 9;
  *(uint32_t*)0x2005e008 = 0;
  *(uint32_t*)0x2005e00c = 0;
  *(uint8_t*)0x2005e010 = 0;
  *(uint8_t*)0x2005e011 = 0;
  memcpy((void*)0x2005e012,
         "\xf0\xae\xec\x0d\x84\x01\x30\xe5\xc5\x48\x5e\x44\xa7\x62\x7f\x9a\x88"
         "\x64\xab\xed\x4d\x44\x60\xdd\x5b\x1a\x01\x82\xc6\xc8\x4a\x14\x3e\x64"
         "\xf3\x1d\x1c\x11\x34\x9c\x94\x90\x54\x86\x0b\x9c\x13\x19\x50\xa6\x10"
         "\xe2\x73\x66\x12\x82\x75\x82\x53\x0c\x3c\x1d\xf9",
         63);
  *(uint64_t*)0x2005e058 = 0;
  *(uint32_t*)0x2087dfd0 = 0x80;
  *(uint64_t*)0x2087dfd8 = 0x20000180;
  *(uint64_t*)0x2087dfe0 = 0;
  *(uint64_t*)0x2087dfe8 = 0;
  *(uint64_t*)0x2087dff0 = 0;
  *(uint32_t*)0x2087dff8 = 0;
  syscall(__NR_sendmsg, r[0], 0x2087dfc8, 0);
  *(uint8_t*)0x20015e15 = -1;
  *(uint8_t*)0x20015e16 = -1;
  *(uint8_t*)0x20015e17 = -1;
  *(uint8_t*)0x20015e18 = -1;
  *(uint8_t*)0x20015e19 = -1;
  *(uint8_t*)0x20015e1a = -1;
  *(uint8_t*)0x20015e1b = 0xaa;
  *(uint8_t*)0x20015e1c = 0xaa;
  *(uint8_t*)0x20015e1d = 0xaa;
  *(uint8_t*)0x20015e1e = 0xaa;
  *(uint8_t*)0x20015e1f = 0xaa;
  *(uint8_t*)0x20015e20 = 0xbb;
  *(uint16_t*)0x20015e21 = htobe16(0x86dd);
  STORE_BY_BITMASK(uint8_t, 0x20015e23, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x20015e23, 6, 4, 4);
  memcpy((void*)0x20015e24, "v`Q", 3);
  *(uint16_t*)0x20015e27 = htobe16(0x30);
  *(uint8_t*)0x20015e29 = 0x3a;
  *(uint8_t*)0x20015e2a = -1;
  *(uint8_t*)0x20015e2b = 0xfe;
  *(uint8_t*)0x20015e2c = 0x80;
  *(uint8_t*)0x20015e2d = 0;
  *(uint8_t*)0x20015e2e = 0;
  *(uint8_t*)0x20015e2f = 0;
  *(uint8_t*)0x20015e30 = 0;
  *(uint8_t*)0x20015e31 = 0;
  *(uint8_t*)0x20015e32 = 0;
  *(uint8_t*)0x20015e33 = 0;
  *(uint8_t*)0x20015e34 = 0;
  *(uint8_t*)0x20015e35 = 0;
  *(uint8_t*)0x20015e36 = 0;
  *(uint8_t*)0x20015e37 = 0;
  *(uint8_t*)0x20015e38 = 0;
  *(uint8_t*)0x20015e39 = 0;
  *(uint8_t*)0x20015e3a = -1;
  *(uint8_t*)0x20015e3b = -1;
  *(uint8_t*)0x20015e3c = 2;
  *(uint8_t*)0x20015e3d = 0;
  *(uint8_t*)0x20015e3e = 0;
  *(uint8_t*)0x20015e3f = 0;
  *(uint8_t*)0x20015e40 = 0;
  *(uint8_t*)0x20015e41 = 0;
  *(uint8_t*)0x20015e42 = 0;
  *(uint8_t*)0x20015e43 = 0;
  *(uint8_t*)0x20015e44 = 0;
  *(uint8_t*)0x20015e45 = 0;
  *(uint8_t*)0x20015e46 = 0;
  *(uint8_t*)0x20015e47 = 0;
  *(uint8_t*)0x20015e48 = 0;
  *(uint8_t*)0x20015e49 = 0;
  *(uint8_t*)0x20015e4a = 1;
  *(uint8_t*)0x20015e4b = 0x80;
  *(uint8_t*)0x20015e4c = 0;
  *(uint16_t*)0x20015e4d = 0;
  *(uint8_t*)0x20015e4f = 0;
  *(uint8_t*)0x20015e50 = 0x14;
  *(uint8_t*)0x20015e51 = 0;
  *(uint8_t*)0x20015e52 = 0;
  STORE_BY_BITMASK(uint8_t, 0x20015e53, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x20015e53, 6, 4, 4);
  memcpy((void*)0x20015e54, "\xc5\x96\x1e", 3);
  *(uint16_t*)0x20015e57 = htobe16(0);
  *(uint8_t*)0x20015e59 = 0;
  *(uint8_t*)0x20015e5a = 0;
  *(uint8_t*)0x20015e5b = -1;
  *(uint8_t*)0x20015e5c = 1;
  *(uint8_t*)0x20015e5d = 0;
  *(uint8_t*)0x20015e5e = 0;
  *(uint8_t*)0x20015e5f = 0;
  *(uint8_t*)0x20015e60 = 0;
  *(uint8_t*)0x20015e61 = 0;
  *(uint8_t*)0x20015e62 = 0;
  *(uint8_t*)0x20015e63 = 5;
  *(uint8_t*)0x20015e64 = 3;
  *(uint8_t*)0x20015e65 = 0;
  *(uint8_t*)0x20015e66 = 0;
  *(uint8_t*)0x20015e67 = 0;
  *(uint8_t*)0x20015e68 = 0;
  *(uint8_t*)0x20015e69 = 0;
  *(uint8_t*)0x20015e6a = 1;
  *(uint8_t*)0x20015e6b = -1;
  *(uint8_t*)0x20015e6c = 2;
  *(uint8_t*)0x20015e6d = 0;
  *(uint8_t*)0x20015e6e = 0;
  *(uint8_t*)0x20015e6f = 0;
  *(uint8_t*)0x20015e70 = 0;
  *(uint8_t*)0x20015e71 = 0;
  *(uint8_t*)0x20015e72 = 0;
  *(uint8_t*)0x20015e73 = 0;
  *(uint8_t*)0x20015e74 = 0;
  *(uint8_t*)0x20015e75 = 0;
  *(uint8_t*)0x20015e76 = 0;
  *(uint8_t*)0x20015e77 = 0;
  *(uint8_t*)0x20015e78 = 0;
  *(uint8_t*)0x20015e79 = 0;
  *(uint8_t*)0x20015e7a = 1;
  struct csum_inet csum_1;
  csum_inet_init(&csum_1);
  csum_inet_update(&csum_1, (const uint8_t*)0x20015e2b, 16);
  csum_inet_update(&csum_1, (const uint8_t*)0x20015e3b, 16);
  uint32_t csum_1_chunk_2 = 0x30000000;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_2, 4);
  uint32_t csum_1_chunk_3 = 0x3a000000;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_3, 4);
  csum_inet_update(&csum_1, (const uint8_t*)0x20015e4b, 48);
  *(uint16_t*)0x20015e4d = csum_inet_digest(&csum_1);
  syz_emit_ethernet(0x66, 0x20015e15, 0);
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
