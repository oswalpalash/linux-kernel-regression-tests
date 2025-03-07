// https://syzkaller.appspot.com/bug?id=319d600149b51a6469bb9813513414e4a983e63e
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/if_arp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/uio.h>
#include <unistd.h>

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <sys/stat.h>

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

static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* uctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  if (__atomic_load_n(&skip_segv, __ATOMIC_RELAXED) &&
      (addr < prog_start || addr > prog_end)) {
    _longjmp(segv_env, 1);
  }
  doexit(sig);
}

static void install_segv_handler()
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

static void use_temporary_dir()
{
  char tmpdir_template[] = "./syzkaller.XXXXXX";
  char* tmpdir = mkdtemp(tmpdir_template);
  if (!tmpdir)
    fail("failed to mkdtemp");
  if (chmod(tmpdir, 0777))
    fail("failed to chmod");
  if (chdir(tmpdir))
    fail("failed to chdir");
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

#define MAX_PIDS 32
#define ADDR_MAX_LEN 32

#define LOCAL_MAC "aa:aa:aa:aa:%02hx:aa"
#define REMOTE_MAC "aa:aa:aa:aa:%02hx:bb"

#define LOCAL_IPV4 "172.20.%d.170"
#define REMOTE_IPV4 "172.20.%d.187"

#define LOCAL_IPV6 "fe80::%02hx:aa"
#define REMOTE_IPV6 "fe80::%02hx:bb"

#define IFF_NAPI 0x0010
#define IFF_NAPI_FRAGS 0x0020

static void initialize_tun(int id)
{
  if (id >= MAX_PIDS)
    fail("tun: no more than %d executors", MAX_PIDS);

  tunfd = open("/dev/net/tun", O_RDWR | O_NONBLOCK);
  if (tunfd == -1) {
    printf("tun: can't open /dev/net/tun: please enable CONFIG_TUN=y\n");
    printf("otherwise fuzzing or reproducing might not work as intended\n");
    return;
  }

  char iface[IFNAMSIZ];
  snprintf_check(iface, sizeof(iface), "syz%d", id);

  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, iface, IFNAMSIZ);
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI | IFF_NAPI | IFF_NAPI_FRAGS;
  if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0) {
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0)
      fail("tun: ioctl(TUNSETIFF) failed");
  }
  if (ioctl(tunfd, TUNGETIFF, (void*)&ifr) < 0)
    fail("tun: ioctl(TUNGETIFF) failed");
  tun_frags_enabled = (ifr.ifr_flags & IFF_NAPI_FRAGS) != 0;

  char local_mac[ADDR_MAX_LEN];
  snprintf_check(local_mac, sizeof(local_mac), LOCAL_MAC, id);
  char remote_mac[ADDR_MAX_LEN];
  snprintf_check(remote_mac, sizeof(remote_mac), REMOTE_MAC, id);

  char local_ipv4[ADDR_MAX_LEN];
  snprintf_check(local_ipv4, sizeof(local_ipv4), LOCAL_IPV4, id);
  char remote_ipv4[ADDR_MAX_LEN];
  snprintf_check(remote_ipv4, sizeof(remote_ipv4), REMOTE_IPV4, id);

  char local_ipv6[ADDR_MAX_LEN];
  snprintf_check(local_ipv6, sizeof(local_ipv6), LOCAL_IPV6, id);
  char remote_ipv6[ADDR_MAX_LEN];
  snprintf_check(remote_ipv6, sizeof(remote_ipv6), REMOTE_IPV6, id);

  execute_command(1, "sysctl -w net.ipv6.conf.%s.accept_dad=0", iface);

  execute_command(1, "sysctl -w net.ipv6.conf.%s.router_solicitations=0",
                  iface);

  execute_command(1, "ip link set dev %s address %s", iface, local_mac);
  execute_command(1, "ip addr add %s/24 dev %s", local_ipv4, iface);
  execute_command(1, "ip -6 addr add %s/120 dev %s", local_ipv6, iface);
  execute_command(1, "ip neigh add %s lladdr %s dev %s nud permanent",
                  remote_ipv4, remote_mac, iface);
  execute_command(1, "ip -6 neigh add %s lladdr %s dev %s nud permanent",
                  remote_ipv6, remote_mac, iface);
  execute_command(1, "ip link set dev %s up", iface);
}

#define DEV_IPV4 "172.20.%d.%d"
#define DEV_IPV6 "fe80::%02hx:%02hx"
#define DEV_MAC "aa:aa:aa:aa:%02hx:%02hx"

static void initialize_netdevices(int id)
{
  unsigned i;
  const char* devtypes[] = {"ip6gretap", "bridge", "vcan"};
  const char* devnames[] = {"lo",       "sit0",    "bridge0", "vcan0",
                            "tunl0",    "gre0",    "gretap0", "ip_vti0",
                            "ip6_vti0", "ip6tnl0", "ip6gre0", "ip6gretap0",
                            "erspan0"};

  for (i = 0; i < sizeof(devtypes) / (sizeof(devtypes[0])); i++)
    execute_command(0, "ip link add dev %s0 type %s", devtypes[i], devtypes[i]);
  for (i = 0; i < sizeof(devnames) / (sizeof(devnames[0])); i++) {
    char addr[ADDR_MAX_LEN];
    snprintf_check(addr, sizeof(addr), DEV_IPV4, id, id + 10);
    execute_command(0, "ip -4 addr add %s/24 dev %s", addr, devnames[i]);
    snprintf_check(addr, sizeof(addr), DEV_IPV6, id, id + 10);
    execute_command(0, "ip -6 addr add %s/120 dev %s", addr, devnames[i]);
    snprintf_check(addr, sizeof(addr), DEV_MAC, id, id + 10);
    execute_command(0, "ip link set dev %s address %s", devnames[i], addr);
    execute_command(0, "ip link set dev %s up", devnames[i]);
  }
}

static void setup_tun(uint64_t pid, bool enable_tun)
{
  if (enable_tun) {
    initialize_tun(pid);
    initialize_netdevices(pid);
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
    NONFAILING(full = frags->full);
    NONFAILING(count = frags->count);
    if (count > MAX_FRAGS)
      count = MAX_FRAGS;
    for (i = 0; i < count && length != 0; i++) {
      uint32_t size = 0;
      NONFAILING(size = frags->frags[i]);
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

long r[1];
void loop()
{
  memset(r, -1, sizeof(r));
  NONFAILING(*(uint8_t*)0x20694ffe = -1);
  NONFAILING(*(uint8_t*)0x20694fff = -1);
  NONFAILING(*(uint8_t*)0x20695000 = -1);
  NONFAILING(*(uint8_t*)0x20695001 = -1);
  NONFAILING(*(uint8_t*)0x20695002 = -1);
  NONFAILING(*(uint8_t*)0x20695003 = -1);
  NONFAILING(*(uint8_t*)0x20695004 = 1);
  NONFAILING(*(uint8_t*)0x20695005 = 0x80);
  NONFAILING(*(uint8_t*)0x20695006 = 0xc2);
  NONFAILING(*(uint8_t*)0x20695007 = 0);
  NONFAILING(*(uint8_t*)0x20695008 = 0);
  NONFAILING(*(uint8_t*)0x20695009 = 0);
  NONFAILING(*(uint16_t*)0x2069500a = htobe16(0x86dd));
  NONFAILING(STORE_BY_BITMASK(uint8_t, 0x2069500c, 0, 0, 4));
  NONFAILING(STORE_BY_BITMASK(uint8_t, 0x2069500c, 6, 4, 4));
  NONFAILING(memcpy((void*)0x2069500d, "\x06\xf5\x26", 3));
  NONFAILING(*(uint16_t*)0x20695010 = htobe16(8));
  NONFAILING(*(uint8_t*)0x20695012 = 0x11);
  NONFAILING(*(uint8_t*)0x20695013 = 0);
  NONFAILING(*(uint8_t*)0x20695014 = 0xfe);
  NONFAILING(*(uint8_t*)0x20695015 = 0x80);
  NONFAILING(*(uint8_t*)0x20695016 = 0);
  NONFAILING(*(uint8_t*)0x20695017 = 0);
  NONFAILING(*(uint8_t*)0x20695018 = 0);
  NONFAILING(*(uint8_t*)0x20695019 = 0);
  NONFAILING(*(uint8_t*)0x2069501a = 0);
  NONFAILING(*(uint8_t*)0x2069501b = 0);
  NONFAILING(*(uint8_t*)0x2069501c = 0);
  NONFAILING(*(uint8_t*)0x2069501d = 0);
  NONFAILING(*(uint8_t*)0x2069501e = 0);
  NONFAILING(*(uint8_t*)0x2069501f = 0);
  NONFAILING(*(uint8_t*)0x20695020 = 0);
  NONFAILING(*(uint8_t*)0x20695021 = 0);
  NONFAILING(*(uint8_t*)0x20695022 = 0);
  NONFAILING(*(uint8_t*)0x20695023 = 0xbb);
  NONFAILING(*(uint8_t*)0x20695024 = -1);
  NONFAILING(*(uint8_t*)0x20695025 = 2);
  NONFAILING(*(uint8_t*)0x20695026 = 0);
  NONFAILING(*(uint8_t*)0x20695027 = 0);
  NONFAILING(*(uint8_t*)0x20695028 = 0);
  NONFAILING(*(uint8_t*)0x20695029 = 0);
  NONFAILING(*(uint8_t*)0x2069502a = 0);
  NONFAILING(*(uint8_t*)0x2069502b = 0);
  NONFAILING(*(uint8_t*)0x2069502c = 0);
  NONFAILING(*(uint8_t*)0x2069502d = 0);
  NONFAILING(*(uint8_t*)0x2069502e = 0);
  NONFAILING(*(uint8_t*)0x2069502f = 0);
  NONFAILING(*(uint8_t*)0x20695030 = 0);
  NONFAILING(*(uint8_t*)0x20695031 = 0);
  NONFAILING(*(uint8_t*)0x20695032 = 0);
  NONFAILING(*(uint8_t*)0x20695033 = 1);
  NONFAILING(*(uint16_t*)0x20695034 = 0);
  NONFAILING(*(uint16_t*)0x20695036 = htobe16(0x4e22));
  NONFAILING(*(uint16_t*)0x20695038 = htobe16(8));
  NONFAILING(*(uint16_t*)0x2069503a = 0);
  NONFAILING(*(uint32_t*)0x20775000 = 0);
  NONFAILING(*(uint32_t*)0x20775004 = 1);
  NONFAILING(*(uint32_t*)0x20775008 = 0);
  struct csum_inet csum_1;
  csum_inet_init(&csum_1);
  NONFAILING(csum_inet_update(&csum_1, (const uint8_t*)0x20695014, 16));
  NONFAILING(csum_inet_update(&csum_1, (const uint8_t*)0x20695024, 16));
  uint32_t csum_1_chunk_2 = 0x8000000;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_2, 4);
  uint32_t csum_1_chunk_3 = 0x11000000;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_3, 4);
  NONFAILING(csum_inet_update(&csum_1, (const uint8_t*)0x20695034, 8));
  NONFAILING(*(uint16_t*)0x2069503a = csum_inet_digest(&csum_1));
  syz_emit_ethernet(0x3e, 0x20694ffe, 0x20775000);
  r[0] = syscall(__NR_socket, 0xa, 3, 0x3c);
  NONFAILING(memcpy((void*)0x20038b58, "\x72\x61\x77\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00",
                    32));
  NONFAILING(*(uint32_t*)0x20038b78 = 9);
  NONFAILING(*(uint32_t*)0x20038b7c = 3);
  NONFAILING(*(uint32_t*)0x20038b80 = 0x448);
  NONFAILING(*(uint32_t*)0x20038b84 = 0);
  NONFAILING(*(uint32_t*)0x20038b88 = -1);
  NONFAILING(*(uint32_t*)0x20038b8c = -1);
  NONFAILING(*(uint32_t*)0x20038b90 = 0);
  NONFAILING(*(uint32_t*)0x20038b94 = -1);
  NONFAILING(*(uint32_t*)0x20038b98 = 0x378);
  NONFAILING(*(uint32_t*)0x20038b9c = -1);
  NONFAILING(*(uint32_t*)0x20038ba0 = -1);
  NONFAILING(*(uint32_t*)0x20038ba4 = 0x378);
  NONFAILING(*(uint32_t*)0x20038ba8 = -1);
  NONFAILING(*(uint32_t*)0x20038bac = 3);
  NONFAILING(*(uint64_t*)0x20038bb0 = 0x20849fd0);
  NONFAILING(*(uint8_t*)0x20038bb8 = 0);
  NONFAILING(*(uint8_t*)0x20038bb9 = 0);
  NONFAILING(*(uint8_t*)0x20038bba = 0);
  NONFAILING(*(uint8_t*)0x20038bbb = 0);
  NONFAILING(*(uint8_t*)0x20038bbc = 0);
  NONFAILING(*(uint8_t*)0x20038bbd = 0);
  NONFAILING(*(uint8_t*)0x20038bbe = 0);
  NONFAILING(*(uint8_t*)0x20038bbf = 0);
  NONFAILING(*(uint8_t*)0x20038bc0 = 0);
  NONFAILING(*(uint8_t*)0x20038bc1 = 0);
  NONFAILING(*(uint8_t*)0x20038bc2 = 0);
  NONFAILING(*(uint8_t*)0x20038bc3 = 0);
  NONFAILING(*(uint8_t*)0x20038bc4 = 0);
  NONFAILING(*(uint8_t*)0x20038bc5 = 0);
  NONFAILING(*(uint8_t*)0x20038bc6 = 0);
  NONFAILING(*(uint8_t*)0x20038bc7 = 0);
  NONFAILING(*(uint8_t*)0x20038bc8 = 0);
  NONFAILING(*(uint8_t*)0x20038bc9 = 0);
  NONFAILING(*(uint8_t*)0x20038bca = 0);
  NONFAILING(*(uint8_t*)0x20038bcb = 0);
  NONFAILING(*(uint8_t*)0x20038bcc = 0);
  NONFAILING(*(uint8_t*)0x20038bcd = 0);
  NONFAILING(*(uint8_t*)0x20038bce = 0);
  NONFAILING(*(uint8_t*)0x20038bcf = 0);
  NONFAILING(*(uint8_t*)0x20038bd0 = 0);
  NONFAILING(*(uint8_t*)0x20038bd1 = 0);
  NONFAILING(*(uint8_t*)0x20038bd2 = 0);
  NONFAILING(*(uint8_t*)0x20038bd3 = 0);
  NONFAILING(*(uint8_t*)0x20038bd4 = 0);
  NONFAILING(*(uint8_t*)0x20038bd5 = 0);
  NONFAILING(*(uint8_t*)0x20038bd6 = 0);
  NONFAILING(*(uint8_t*)0x20038bd7 = 0);
  NONFAILING(*(uint8_t*)0x20038bd8 = 0);
  NONFAILING(*(uint8_t*)0x20038bd9 = 0);
  NONFAILING(*(uint8_t*)0x20038bda = 0);
  NONFAILING(*(uint8_t*)0x20038bdb = 0);
  NONFAILING(*(uint8_t*)0x20038bdc = 0);
  NONFAILING(*(uint8_t*)0x20038bdd = 0);
  NONFAILING(*(uint8_t*)0x20038bde = 0);
  NONFAILING(*(uint8_t*)0x20038bdf = 0);
  NONFAILING(*(uint8_t*)0x20038be0 = 0);
  NONFAILING(*(uint8_t*)0x20038be1 = 0);
  NONFAILING(*(uint8_t*)0x20038be2 = 0);
  NONFAILING(*(uint8_t*)0x20038be3 = 0);
  NONFAILING(*(uint8_t*)0x20038be4 = 0);
  NONFAILING(*(uint8_t*)0x20038be5 = 0);
  NONFAILING(*(uint8_t*)0x20038be6 = 0);
  NONFAILING(*(uint8_t*)0x20038be7 = 0);
  NONFAILING(*(uint8_t*)0x20038be8 = 0);
  NONFAILING(*(uint8_t*)0x20038be9 = 0);
  NONFAILING(*(uint8_t*)0x20038bea = 0);
  NONFAILING(*(uint8_t*)0x20038beb = 0);
  NONFAILING(*(uint8_t*)0x20038bec = 0);
  NONFAILING(*(uint8_t*)0x20038bed = 0);
  NONFAILING(*(uint8_t*)0x20038bee = 0);
  NONFAILING(*(uint8_t*)0x20038bef = 0);
  NONFAILING(*(uint8_t*)0x20038bf0 = 0);
  NONFAILING(*(uint8_t*)0x20038bf1 = 0);
  NONFAILING(*(uint8_t*)0x20038bf2 = 0);
  NONFAILING(*(uint8_t*)0x20038bf3 = 0);
  NONFAILING(*(uint8_t*)0x20038bf4 = 0);
  NONFAILING(*(uint8_t*)0x20038bf5 = 0);
  NONFAILING(*(uint8_t*)0x20038bf6 = 0);
  NONFAILING(*(uint8_t*)0x20038bf7 = 0);
  NONFAILING(*(uint8_t*)0x20038bf8 = 0);
  NONFAILING(*(uint8_t*)0x20038bf9 = 0);
  NONFAILING(*(uint8_t*)0x20038bfa = 0);
  NONFAILING(*(uint8_t*)0x20038bfb = 0);
  NONFAILING(*(uint8_t*)0x20038bfc = 0);
  NONFAILING(*(uint8_t*)0x20038bfd = 0);
  NONFAILING(*(uint8_t*)0x20038bfe = 0);
  NONFAILING(*(uint8_t*)0x20038bff = 0);
  NONFAILING(*(uint8_t*)0x20038c00 = 0);
  NONFAILING(*(uint8_t*)0x20038c01 = 0);
  NONFAILING(*(uint8_t*)0x20038c02 = 0);
  NONFAILING(*(uint8_t*)0x20038c03 = 0);
  NONFAILING(*(uint8_t*)0x20038c04 = 0);
  NONFAILING(*(uint8_t*)0x20038c05 = 0);
  NONFAILING(*(uint8_t*)0x20038c06 = 0);
  NONFAILING(*(uint8_t*)0x20038c07 = 0);
  NONFAILING(*(uint8_t*)0x20038c08 = 0);
  NONFAILING(*(uint8_t*)0x20038c09 = 0);
  NONFAILING(*(uint8_t*)0x20038c0a = 0);
  NONFAILING(*(uint8_t*)0x20038c0b = 0);
  NONFAILING(*(uint8_t*)0x20038c0c = 0);
  NONFAILING(*(uint8_t*)0x20038c0d = 0);
  NONFAILING(*(uint8_t*)0x20038c0e = 0);
  NONFAILING(*(uint8_t*)0x20038c0f = 0);
  NONFAILING(*(uint8_t*)0x20038c10 = 0);
  NONFAILING(*(uint8_t*)0x20038c11 = 0);
  NONFAILING(*(uint8_t*)0x20038c12 = 0);
  NONFAILING(*(uint8_t*)0x20038c13 = 0);
  NONFAILING(*(uint8_t*)0x20038c14 = 0);
  NONFAILING(*(uint8_t*)0x20038c15 = 0);
  NONFAILING(*(uint8_t*)0x20038c16 = 0);
  NONFAILING(*(uint8_t*)0x20038c17 = 0);
  NONFAILING(*(uint8_t*)0x20038c18 = 0);
  NONFAILING(*(uint8_t*)0x20038c19 = 0);
  NONFAILING(*(uint8_t*)0x20038c1a = 0);
  NONFAILING(*(uint8_t*)0x20038c1b = 0);
  NONFAILING(*(uint8_t*)0x20038c1c = 0);
  NONFAILING(*(uint8_t*)0x20038c1d = 0);
  NONFAILING(*(uint8_t*)0x20038c1e = 0);
  NONFAILING(*(uint8_t*)0x20038c1f = 0);
  NONFAILING(*(uint8_t*)0x20038c20 = 0);
  NONFAILING(*(uint8_t*)0x20038c21 = 0);
  NONFAILING(*(uint8_t*)0x20038c22 = 0);
  NONFAILING(*(uint8_t*)0x20038c23 = 0);
  NONFAILING(*(uint8_t*)0x20038c24 = 0);
  NONFAILING(*(uint8_t*)0x20038c25 = 0);
  NONFAILING(*(uint8_t*)0x20038c26 = 0);
  NONFAILING(*(uint8_t*)0x20038c27 = 0);
  NONFAILING(*(uint8_t*)0x20038c28 = 0);
  NONFAILING(*(uint8_t*)0x20038c29 = 0);
  NONFAILING(*(uint8_t*)0x20038c2a = 0);
  NONFAILING(*(uint8_t*)0x20038c2b = 0);
  NONFAILING(*(uint8_t*)0x20038c2c = 0);
  NONFAILING(*(uint8_t*)0x20038c2d = 0);
  NONFAILING(*(uint8_t*)0x20038c2e = 0);
  NONFAILING(*(uint8_t*)0x20038c2f = 0);
  NONFAILING(*(uint8_t*)0x20038c30 = 0);
  NONFAILING(*(uint8_t*)0x20038c31 = 0);
  NONFAILING(*(uint8_t*)0x20038c32 = 0);
  NONFAILING(*(uint8_t*)0x20038c33 = 0);
  NONFAILING(*(uint8_t*)0x20038c34 = 0);
  NONFAILING(*(uint8_t*)0x20038c35 = 0);
  NONFAILING(*(uint8_t*)0x20038c36 = 0);
  NONFAILING(*(uint8_t*)0x20038c37 = 0);
  NONFAILING(*(uint8_t*)0x20038c38 = 0);
  NONFAILING(*(uint8_t*)0x20038c39 = 0);
  NONFAILING(*(uint8_t*)0x20038c3a = 0);
  NONFAILING(*(uint8_t*)0x20038c3b = 0);
  NONFAILING(*(uint8_t*)0x20038c3c = 0);
  NONFAILING(*(uint8_t*)0x20038c3d = 0);
  NONFAILING(*(uint8_t*)0x20038c3e = 0);
  NONFAILING(*(uint8_t*)0x20038c3f = 0);
  NONFAILING(*(uint32_t*)0x20038c40 = 0);
  NONFAILING(*(uint16_t*)0x20038c44 = 0x200);
  NONFAILING(*(uint16_t*)0x20038c46 = 0x268);
  NONFAILING(*(uint32_t*)0x20038c48 = 0);
  NONFAILING(*(uint64_t*)0x20038c50 = 0);
  NONFAILING(*(uint64_t*)0x20038c58 = 0);
  NONFAILING(*(uint16_t*)0x20038c60 = 0x158);
  NONFAILING(memcpy((void*)0x20038c62, "\x68\x61\x73\x68\x6c\x69\x6d\x69\x74"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00",
                    29));
  NONFAILING(*(uint8_t*)0x20038c7f = 3);
  NONFAILING(memcpy(
      (void*)0x20038c80,
      "\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x31\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00",
      255));
  NONFAILING(*(uint64_t*)0x20038d80 = 0x40);
  NONFAILING(*(uint64_t*)0x20038d88 = 0);
  NONFAILING(*(uint32_t*)0x20038d90 = 0x42);
  NONFAILING(*(uint32_t*)0x20038d94 = 0);
  NONFAILING(*(uint32_t*)0x20038d98 = 0);
  NONFAILING(*(uint32_t*)0x20038d9c = 4);
  NONFAILING(*(uint32_t*)0x20038da0 = 0x80000001);
  NONFAILING(*(uint32_t*)0x20038da4 = 8);
  NONFAILING(*(uint8_t*)0x20038da8 = 0);
  NONFAILING(*(uint8_t*)0x20038da9 = 0);
  NONFAILING(*(uint64_t*)0x20038db0 = 0);
  NONFAILING(*(uint16_t*)0x20038db8 = 0x68);
  NONFAILING(memcpy((void*)0x20038dba, "\x43\x54\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00",
                    29));
  NONFAILING(*(uint8_t*)0x20038dd7 = 2);
  NONFAILING(*(uint16_t*)0x20038dd8 = 0xf);
  NONFAILING(*(uint16_t*)0x20038dda = 0);
  NONFAILING(*(uint32_t*)0x20038ddc = 0);
  NONFAILING(*(uint32_t*)0x20038de0 = 0);
  NONFAILING(memcpy(
      (void*)0x20038de4,
      "\x70\x70\x74\x70\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16));
  NONFAILING(memcpy((void*)0x20038df4, "\x73\x79\x7a\x31\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00",
                    32));
  NONFAILING(*(uint64_t*)0x20038e18 = 0);
  NONFAILING(*(uint8_t*)0x20038e20 = 0);
  NONFAILING(*(uint8_t*)0x20038e21 = 0);
  NONFAILING(*(uint8_t*)0x20038e22 = 0);
  NONFAILING(*(uint8_t*)0x20038e23 = 0);
  NONFAILING(*(uint8_t*)0x20038e24 = 0);
  NONFAILING(*(uint8_t*)0x20038e25 = 0);
  NONFAILING(*(uint8_t*)0x20038e26 = 0);
  NONFAILING(*(uint8_t*)0x20038e27 = 0);
  NONFAILING(*(uint8_t*)0x20038e28 = 0);
  NONFAILING(*(uint8_t*)0x20038e29 = 0);
  NONFAILING(*(uint8_t*)0x20038e2a = 0);
  NONFAILING(*(uint8_t*)0x20038e2b = 0);
  NONFAILING(*(uint8_t*)0x20038e2c = 0);
  NONFAILING(*(uint8_t*)0x20038e2d = 0);
  NONFAILING(*(uint8_t*)0x20038e2e = 0);
  NONFAILING(*(uint8_t*)0x20038e2f = 0);
  NONFAILING(*(uint8_t*)0x20038e30 = 0);
  NONFAILING(*(uint8_t*)0x20038e31 = 0);
  NONFAILING(*(uint8_t*)0x20038e32 = 0);
  NONFAILING(*(uint8_t*)0x20038e33 = 0);
  NONFAILING(*(uint8_t*)0x20038e34 = 0);
  NONFAILING(*(uint8_t*)0x20038e35 = 0);
  NONFAILING(*(uint8_t*)0x20038e36 = 0);
  NONFAILING(*(uint8_t*)0x20038e37 = 0);
  NONFAILING(*(uint8_t*)0x20038e38 = 0);
  NONFAILING(*(uint8_t*)0x20038e39 = 0);
  NONFAILING(*(uint8_t*)0x20038e3a = 0);
  NONFAILING(*(uint8_t*)0x20038e3b = 0);
  NONFAILING(*(uint8_t*)0x20038e3c = 0);
  NONFAILING(*(uint8_t*)0x20038e3d = 0);
  NONFAILING(*(uint8_t*)0x20038e3e = 0);
  NONFAILING(*(uint8_t*)0x20038e3f = 0);
  NONFAILING(*(uint8_t*)0x20038e40 = 0);
  NONFAILING(*(uint8_t*)0x20038e41 = 0);
  NONFAILING(*(uint8_t*)0x20038e42 = 0);
  NONFAILING(*(uint8_t*)0x20038e43 = 0);
  NONFAILING(*(uint8_t*)0x20038e44 = 0);
  NONFAILING(*(uint8_t*)0x20038e45 = 0);
  NONFAILING(*(uint8_t*)0x20038e46 = 0);
  NONFAILING(*(uint8_t*)0x20038e47 = 0);
  NONFAILING(*(uint8_t*)0x20038e48 = 0);
  NONFAILING(*(uint8_t*)0x20038e49 = 0);
  NONFAILING(*(uint8_t*)0x20038e4a = 0);
  NONFAILING(*(uint8_t*)0x20038e4b = 0);
  NONFAILING(*(uint8_t*)0x20038e4c = 0);
  NONFAILING(*(uint8_t*)0x20038e4d = 0);
  NONFAILING(*(uint8_t*)0x20038e4e = 0);
  NONFAILING(*(uint8_t*)0x20038e4f = 0);
  NONFAILING(*(uint8_t*)0x20038e50 = 0);
  NONFAILING(*(uint8_t*)0x20038e51 = 0);
  NONFAILING(*(uint8_t*)0x20038e52 = 0);
  NONFAILING(*(uint8_t*)0x20038e53 = 0);
  NONFAILING(*(uint8_t*)0x20038e54 = 0);
  NONFAILING(*(uint8_t*)0x20038e55 = 0);
  NONFAILING(*(uint8_t*)0x20038e56 = 0);
  NONFAILING(*(uint8_t*)0x20038e57 = 0);
  NONFAILING(*(uint8_t*)0x20038e58 = 0);
  NONFAILING(*(uint8_t*)0x20038e59 = 0);
  NONFAILING(*(uint8_t*)0x20038e5a = 0);
  NONFAILING(*(uint8_t*)0x20038e5b = 0);
  NONFAILING(*(uint8_t*)0x20038e5c = 0);
  NONFAILING(*(uint8_t*)0x20038e5d = 0);
  NONFAILING(*(uint8_t*)0x20038e5e = 0);
  NONFAILING(*(uint8_t*)0x20038e5f = 0);
  NONFAILING(*(uint8_t*)0x20038e60 = 0);
  NONFAILING(*(uint8_t*)0x20038e61 = 0);
  NONFAILING(*(uint8_t*)0x20038e62 = 0);
  NONFAILING(*(uint8_t*)0x20038e63 = 0);
  NONFAILING(*(uint8_t*)0x20038e64 = 0);
  NONFAILING(*(uint8_t*)0x20038e65 = 0);
  NONFAILING(*(uint8_t*)0x20038e66 = 0);
  NONFAILING(*(uint8_t*)0x20038e67 = 0);
  NONFAILING(*(uint8_t*)0x20038e68 = 0);
  NONFAILING(*(uint8_t*)0x20038e69 = 0);
  NONFAILING(*(uint8_t*)0x20038e6a = 0);
  NONFAILING(*(uint8_t*)0x20038e6b = 0);
  NONFAILING(*(uint8_t*)0x20038e6c = 0);
  NONFAILING(*(uint8_t*)0x20038e6d = 0);
  NONFAILING(*(uint8_t*)0x20038e6e = 0);
  NONFAILING(*(uint8_t*)0x20038e6f = 0);
  NONFAILING(*(uint8_t*)0x20038e70 = 0);
  NONFAILING(*(uint8_t*)0x20038e71 = 0);
  NONFAILING(*(uint8_t*)0x20038e72 = 0);
  NONFAILING(*(uint8_t*)0x20038e73 = 0);
  NONFAILING(*(uint8_t*)0x20038e74 = 0);
  NONFAILING(*(uint8_t*)0x20038e75 = 0);
  NONFAILING(*(uint8_t*)0x20038e76 = 0);
  NONFAILING(*(uint8_t*)0x20038e77 = 0);
  NONFAILING(*(uint8_t*)0x20038e78 = 0);
  NONFAILING(*(uint8_t*)0x20038e79 = 0);
  NONFAILING(*(uint8_t*)0x20038e7a = 0);
  NONFAILING(*(uint8_t*)0x20038e7b = 0);
  NONFAILING(*(uint8_t*)0x20038e7c = 0);
  NONFAILING(*(uint8_t*)0x20038e7d = 0);
  NONFAILING(*(uint8_t*)0x20038e7e = 0);
  NONFAILING(*(uint8_t*)0x20038e7f = 0);
  NONFAILING(*(uint8_t*)0x20038e80 = 0);
  NONFAILING(*(uint8_t*)0x20038e81 = 0);
  NONFAILING(*(uint8_t*)0x20038e82 = 0);
  NONFAILING(*(uint8_t*)0x20038e83 = 0);
  NONFAILING(*(uint8_t*)0x20038e84 = 0);
  NONFAILING(*(uint8_t*)0x20038e85 = 0);
  NONFAILING(*(uint8_t*)0x20038e86 = 0);
  NONFAILING(*(uint8_t*)0x20038e87 = 0);
  NONFAILING(*(uint8_t*)0x20038e88 = 0);
  NONFAILING(*(uint8_t*)0x20038e89 = 0);
  NONFAILING(*(uint8_t*)0x20038e8a = 0);
  NONFAILING(*(uint8_t*)0x20038e8b = 0);
  NONFAILING(*(uint8_t*)0x20038e8c = 0);
  NONFAILING(*(uint8_t*)0x20038e8d = 0);
  NONFAILING(*(uint8_t*)0x20038e8e = 0);
  NONFAILING(*(uint8_t*)0x20038e8f = 0);
  NONFAILING(*(uint8_t*)0x20038e90 = 0);
  NONFAILING(*(uint8_t*)0x20038e91 = 0);
  NONFAILING(*(uint8_t*)0x20038e92 = 0);
  NONFAILING(*(uint8_t*)0x20038e93 = 0);
  NONFAILING(*(uint8_t*)0x20038e94 = 0);
  NONFAILING(*(uint8_t*)0x20038e95 = 0);
  NONFAILING(*(uint8_t*)0x20038e96 = 0);
  NONFAILING(*(uint8_t*)0x20038e97 = 0);
  NONFAILING(*(uint8_t*)0x20038e98 = 0);
  NONFAILING(*(uint8_t*)0x20038e99 = 0);
  NONFAILING(*(uint8_t*)0x20038e9a = 0);
  NONFAILING(*(uint8_t*)0x20038e9b = 0);
  NONFAILING(*(uint8_t*)0x20038e9c = 0);
  NONFAILING(*(uint8_t*)0x20038e9d = 0);
  NONFAILING(*(uint8_t*)0x20038e9e = 0);
  NONFAILING(*(uint8_t*)0x20038e9f = 0);
  NONFAILING(*(uint8_t*)0x20038ea0 = 0);
  NONFAILING(*(uint8_t*)0x20038ea1 = 0);
  NONFAILING(*(uint8_t*)0x20038ea2 = 0);
  NONFAILING(*(uint8_t*)0x20038ea3 = 0);
  NONFAILING(*(uint8_t*)0x20038ea4 = 0);
  NONFAILING(*(uint8_t*)0x20038ea5 = 0);
  NONFAILING(*(uint8_t*)0x20038ea6 = 0);
  NONFAILING(*(uint8_t*)0x20038ea7 = 0);
  NONFAILING(*(uint32_t*)0x20038ea8 = 0);
  NONFAILING(*(uint16_t*)0x20038eac = 0xa8);
  NONFAILING(*(uint16_t*)0x20038eae = 0x110);
  NONFAILING(*(uint32_t*)0x20038eb0 = 0);
  NONFAILING(*(uint64_t*)0x20038eb8 = 0);
  NONFAILING(*(uint64_t*)0x20038ec0 = 0);
  NONFAILING(*(uint16_t*)0x20038ec8 = 0x68);
  NONFAILING(memcpy((void*)0x20038eca, "\x43\x54\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00",
                    29));
  NONFAILING(*(uint8_t*)0x20038ee7 = 2);
  NONFAILING(*(uint16_t*)0x20038ee8 = 0x11);
  NONFAILING(*(uint16_t*)0x20038eea = 0);
  NONFAILING(*(uint32_t*)0x20038eec = 0);
  NONFAILING(*(uint32_t*)0x20038ef0 = 0);
  NONFAILING(memcpy(
      (void*)0x20038ef4,
      "\x6e\x65\x74\x62\x69\x6f\x73\x2d\x6e\x73\x00\x00\x00\x00\x00\x00", 16));
  NONFAILING(memcpy((void*)0x20038f04, "\x73\x79\x7a\x31\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00",
                    32));
  NONFAILING(*(uint64_t*)0x20038f28 = 0);
  NONFAILING(*(uint8_t*)0x20038f30 = 0);
  NONFAILING(*(uint8_t*)0x20038f31 = 0);
  NONFAILING(*(uint8_t*)0x20038f32 = 0);
  NONFAILING(*(uint8_t*)0x20038f33 = 0);
  NONFAILING(*(uint8_t*)0x20038f34 = 0);
  NONFAILING(*(uint8_t*)0x20038f35 = 0);
  NONFAILING(*(uint8_t*)0x20038f36 = 0);
  NONFAILING(*(uint8_t*)0x20038f37 = 0);
  NONFAILING(*(uint8_t*)0x20038f38 = 0);
  NONFAILING(*(uint8_t*)0x20038f39 = 0);
  NONFAILING(*(uint8_t*)0x20038f3a = 0);
  NONFAILING(*(uint8_t*)0x20038f3b = 0);
  NONFAILING(*(uint8_t*)0x20038f3c = 0);
  NONFAILING(*(uint8_t*)0x20038f3d = 0);
  NONFAILING(*(uint8_t*)0x20038f3e = 0);
  NONFAILING(*(uint8_t*)0x20038f3f = 0);
  NONFAILING(*(uint8_t*)0x20038f40 = 0);
  NONFAILING(*(uint8_t*)0x20038f41 = 0);
  NONFAILING(*(uint8_t*)0x20038f42 = 0);
  NONFAILING(*(uint8_t*)0x20038f43 = 0);
  NONFAILING(*(uint8_t*)0x20038f44 = 0);
  NONFAILING(*(uint8_t*)0x20038f45 = 0);
  NONFAILING(*(uint8_t*)0x20038f46 = 0);
  NONFAILING(*(uint8_t*)0x20038f47 = 0);
  NONFAILING(*(uint8_t*)0x20038f48 = 0);
  NONFAILING(*(uint8_t*)0x20038f49 = 0);
  NONFAILING(*(uint8_t*)0x20038f4a = 0);
  NONFAILING(*(uint8_t*)0x20038f4b = 0);
  NONFAILING(*(uint8_t*)0x20038f4c = 0);
  NONFAILING(*(uint8_t*)0x20038f4d = 0);
  NONFAILING(*(uint8_t*)0x20038f4e = 0);
  NONFAILING(*(uint8_t*)0x20038f4f = 0);
  NONFAILING(*(uint8_t*)0x20038f50 = 0);
  NONFAILING(*(uint8_t*)0x20038f51 = 0);
  NONFAILING(*(uint8_t*)0x20038f52 = 0);
  NONFAILING(*(uint8_t*)0x20038f53 = 0);
  NONFAILING(*(uint8_t*)0x20038f54 = 0);
  NONFAILING(*(uint8_t*)0x20038f55 = 0);
  NONFAILING(*(uint8_t*)0x20038f56 = 0);
  NONFAILING(*(uint8_t*)0x20038f57 = 0);
  NONFAILING(*(uint8_t*)0x20038f58 = 0);
  NONFAILING(*(uint8_t*)0x20038f59 = 0);
  NONFAILING(*(uint8_t*)0x20038f5a = 0);
  NONFAILING(*(uint8_t*)0x20038f5b = 0);
  NONFAILING(*(uint8_t*)0x20038f5c = 0);
  NONFAILING(*(uint8_t*)0x20038f5d = 0);
  NONFAILING(*(uint8_t*)0x20038f5e = 0);
  NONFAILING(*(uint8_t*)0x20038f5f = 0);
  NONFAILING(*(uint8_t*)0x20038f60 = 0);
  NONFAILING(*(uint8_t*)0x20038f61 = 0);
  NONFAILING(*(uint8_t*)0x20038f62 = 0);
  NONFAILING(*(uint8_t*)0x20038f63 = 0);
  NONFAILING(*(uint8_t*)0x20038f64 = 0);
  NONFAILING(*(uint8_t*)0x20038f65 = 0);
  NONFAILING(*(uint8_t*)0x20038f66 = 0);
  NONFAILING(*(uint8_t*)0x20038f67 = 0);
  NONFAILING(*(uint8_t*)0x20038f68 = 0);
  NONFAILING(*(uint8_t*)0x20038f69 = 0);
  NONFAILING(*(uint8_t*)0x20038f6a = 0);
  NONFAILING(*(uint8_t*)0x20038f6b = 0);
  NONFAILING(*(uint8_t*)0x20038f6c = 0);
  NONFAILING(*(uint8_t*)0x20038f6d = 0);
  NONFAILING(*(uint8_t*)0x20038f6e = 0);
  NONFAILING(*(uint8_t*)0x20038f6f = 0);
  NONFAILING(*(uint8_t*)0x20038f70 = 0);
  NONFAILING(*(uint8_t*)0x20038f71 = 0);
  NONFAILING(*(uint8_t*)0x20038f72 = 0);
  NONFAILING(*(uint8_t*)0x20038f73 = 0);
  NONFAILING(*(uint8_t*)0x20038f74 = 0);
  NONFAILING(*(uint8_t*)0x20038f75 = 0);
  NONFAILING(*(uint8_t*)0x20038f76 = 0);
  NONFAILING(*(uint8_t*)0x20038f77 = 0);
  NONFAILING(*(uint8_t*)0x20038f78 = 0);
  NONFAILING(*(uint8_t*)0x20038f79 = 0);
  NONFAILING(*(uint8_t*)0x20038f7a = 0);
  NONFAILING(*(uint8_t*)0x20038f7b = 0);
  NONFAILING(*(uint8_t*)0x20038f7c = 0);
  NONFAILING(*(uint8_t*)0x20038f7d = 0);
  NONFAILING(*(uint8_t*)0x20038f7e = 0);
  NONFAILING(*(uint8_t*)0x20038f7f = 0);
  NONFAILING(*(uint8_t*)0x20038f80 = 0);
  NONFAILING(*(uint8_t*)0x20038f81 = 0);
  NONFAILING(*(uint8_t*)0x20038f82 = 0);
  NONFAILING(*(uint8_t*)0x20038f83 = 0);
  NONFAILING(*(uint8_t*)0x20038f84 = 0);
  NONFAILING(*(uint8_t*)0x20038f85 = 0);
  NONFAILING(*(uint8_t*)0x20038f86 = 0);
  NONFAILING(*(uint8_t*)0x20038f87 = 0);
  NONFAILING(*(uint8_t*)0x20038f88 = 0);
  NONFAILING(*(uint8_t*)0x20038f89 = 0);
  NONFAILING(*(uint8_t*)0x20038f8a = 0);
  NONFAILING(*(uint8_t*)0x20038f8b = 0);
  NONFAILING(*(uint8_t*)0x20038f8c = 0);
  NONFAILING(*(uint8_t*)0x20038f8d = 0);
  NONFAILING(*(uint8_t*)0x20038f8e = 0);
  NONFAILING(*(uint8_t*)0x20038f8f = 0);
  NONFAILING(*(uint8_t*)0x20038f90 = 0);
  NONFAILING(*(uint8_t*)0x20038f91 = 0);
  NONFAILING(*(uint8_t*)0x20038f92 = 0);
  NONFAILING(*(uint8_t*)0x20038f93 = 0);
  NONFAILING(*(uint8_t*)0x20038f94 = 0);
  NONFAILING(*(uint8_t*)0x20038f95 = 0);
  NONFAILING(*(uint8_t*)0x20038f96 = 0);
  NONFAILING(*(uint8_t*)0x20038f97 = 0);
  NONFAILING(*(uint8_t*)0x20038f98 = 0);
  NONFAILING(*(uint8_t*)0x20038f99 = 0);
  NONFAILING(*(uint8_t*)0x20038f9a = 0);
  NONFAILING(*(uint8_t*)0x20038f9b = 0);
  NONFAILING(*(uint8_t*)0x20038f9c = 0);
  NONFAILING(*(uint8_t*)0x20038f9d = 0);
  NONFAILING(*(uint8_t*)0x20038f9e = 0);
  NONFAILING(*(uint8_t*)0x20038f9f = 0);
  NONFAILING(*(uint8_t*)0x20038fa0 = 0);
  NONFAILING(*(uint8_t*)0x20038fa1 = 0);
  NONFAILING(*(uint8_t*)0x20038fa2 = 0);
  NONFAILING(*(uint8_t*)0x20038fa3 = 0);
  NONFAILING(*(uint8_t*)0x20038fa4 = 0);
  NONFAILING(*(uint8_t*)0x20038fa5 = 0);
  NONFAILING(*(uint8_t*)0x20038fa6 = 0);
  NONFAILING(*(uint8_t*)0x20038fa7 = 0);
  NONFAILING(*(uint8_t*)0x20038fa8 = 0);
  NONFAILING(*(uint8_t*)0x20038fa9 = 0);
  NONFAILING(*(uint8_t*)0x20038faa = 0);
  NONFAILING(*(uint8_t*)0x20038fab = 0);
  NONFAILING(*(uint8_t*)0x20038fac = 0);
  NONFAILING(*(uint8_t*)0x20038fad = 0);
  NONFAILING(*(uint8_t*)0x20038fae = 0);
  NONFAILING(*(uint8_t*)0x20038faf = 0);
  NONFAILING(*(uint8_t*)0x20038fb0 = 0);
  NONFAILING(*(uint8_t*)0x20038fb1 = 0);
  NONFAILING(*(uint8_t*)0x20038fb2 = 0);
  NONFAILING(*(uint8_t*)0x20038fb3 = 0);
  NONFAILING(*(uint8_t*)0x20038fb4 = 0);
  NONFAILING(*(uint8_t*)0x20038fb5 = 0);
  NONFAILING(*(uint8_t*)0x20038fb6 = 0);
  NONFAILING(*(uint8_t*)0x20038fb7 = 0);
  NONFAILING(*(uint32_t*)0x20038fb8 = 0);
  NONFAILING(*(uint16_t*)0x20038fbc = 0xa8);
  NONFAILING(*(uint16_t*)0x20038fbe = 0xd0);
  NONFAILING(*(uint32_t*)0x20038fc0 = 0);
  NONFAILING(*(uint64_t*)0x20038fc8 = 0);
  NONFAILING(*(uint64_t*)0x20038fd0 = 0);
  NONFAILING(*(uint16_t*)0x20038fd8 = 0x28);
  NONFAILING(memcpy((void*)0x20038fda, "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00",
                    29));
  NONFAILING(*(uint8_t*)0x20038ff7 = 0);
  NONFAILING(*(uint32_t*)0x20038ff8 = 0xfffffffe);
  syscall(__NR_setsockopt, r[0], 0x29, 0x40, 0x20038b58, 0x4a8);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  install_segv_handler();
  use_temporary_dir();
  setup_tun(0, true);
  loop();
  return 0;
}
