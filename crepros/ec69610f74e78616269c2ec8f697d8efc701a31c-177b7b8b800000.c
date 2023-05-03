// https://syzkaller.appspot.com/bug?id=ec69610f74e78616269c2ec8f697d8efc701a31c
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
void loop()
{
  long res;
  res = syscall(__NR_socket, 0xa, 2, 0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_socket, 0x18, 1, 1);
  if (res != -1)
    r[1] = res;
  *(uint16_t*)0x205fafd2 = 0x18;
  *(uint32_t*)0x205fafd4 = 1;
  *(uint32_t*)0x205fafd8 = 0;
  *(uint32_t*)0x205fafdc = r[0];
  *(uint16_t*)0x205fafe0 = 2;
  *(uint16_t*)0x205fafe2 = htobe16(0x4e21);
  *(uint32_t*)0x205fafe4 = htobe32(0xe0000002);
  *(uint8_t*)0x205fafe8 = 0;
  *(uint8_t*)0x205fafe9 = 0;
  *(uint8_t*)0x205fafea = 0;
  *(uint8_t*)0x205fafeb = 0;
  *(uint8_t*)0x205fafec = 0;
  *(uint8_t*)0x205fafed = 0;
  *(uint8_t*)0x205fafee = 0;
  *(uint8_t*)0x205fafef = 0;
  *(uint32_t*)0x205faff0 = 4;
  *(uint32_t*)0x205faff4 = 0;
  *(uint32_t*)0x205faff8 = 2;
  *(uint32_t*)0x205faffc = 0;
  syscall(__NR_connect, r[1], 0x205fafd2, 0x2e);
  *(uint16_t*)0x20000280 = 0xa;
  *(uint16_t*)0x20000282 = htobe16(0x4e24);
  *(uint32_t*)0x20000284 = 4;
  *(uint8_t*)0x20000288 = 0;
  *(uint8_t*)0x20000289 = 0;
  *(uint8_t*)0x2000028a = 0;
  *(uint8_t*)0x2000028b = 0;
  *(uint8_t*)0x2000028c = 0;
  *(uint8_t*)0x2000028d = 0;
  *(uint8_t*)0x2000028e = 0;
  *(uint8_t*)0x2000028f = 0;
  *(uint8_t*)0x20000290 = 0;
  *(uint8_t*)0x20000291 = 0;
  *(uint8_t*)0x20000292 = -1;
  *(uint8_t*)0x20000293 = -1;
  *(uint32_t*)0x20000294 = htobe32(0x7f000001);
  *(uint32_t*)0x20000298 = 4;
  syscall(__NR_connect, r[0], 0x20000280, 0x1c);
  *(uint64_t*)0x2037ffc8 = 0x209dd000;
  *(uint16_t*)0x209dd000 = 0x10;
  *(uint16_t*)0x209dd002 = 0;
  *(uint32_t*)0x209dd004 = 0;
  *(uint32_t*)0x209dd008 = 2;
  *(uint32_t*)0x2037ffd0 = 0xc;
  *(uint64_t*)0x2037ffd8 = 0x202ceff0;
  *(uint64_t*)0x202ceff0 = 0x20000040;
  *(uint32_t*)0x20000040 = 0x130;
  *(uint16_t*)0x20000044 = 0x13;
  *(uint16_t*)0x20000046 = 0x100;
  *(uint32_t*)0x20000048 = 0x70bd2a;
  *(uint32_t*)0x2000004c = 0x25dfdbfc;
  memcpy((void*)0x20000050,
         "\x74\x67\x72\x31\x32\x38\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         64);
  *(uint8_t*)0x20000090 = 0;
  *(uint8_t*)0x20000091 = 0;
  *(uint8_t*)0x20000092 = 0;
  *(uint8_t*)0x20000093 = 0;
  *(uint8_t*)0x20000094 = 0;
  *(uint8_t*)0x20000095 = 0;
  *(uint8_t*)0x20000096 = 0;
  *(uint8_t*)0x20000097 = 0;
  *(uint8_t*)0x20000098 = 0;
  *(uint8_t*)0x20000099 = 0;
  *(uint8_t*)0x2000009a = 0;
  *(uint8_t*)0x2000009b = 0;
  *(uint8_t*)0x2000009c = 0;
  *(uint8_t*)0x2000009d = 0;
  *(uint8_t*)0x2000009e = 0;
  *(uint8_t*)0x2000009f = 0;
  *(uint8_t*)0x200000a0 = 0;
  *(uint8_t*)0x200000a1 = 0;
  *(uint8_t*)0x200000a2 = 0;
  *(uint8_t*)0x200000a3 = 0;
  *(uint8_t*)0x200000a4 = 0;
  *(uint8_t*)0x200000a5 = 0;
  *(uint8_t*)0x200000a6 = 0;
  *(uint8_t*)0x200000a7 = 0;
  *(uint8_t*)0x200000a8 = 0;
  *(uint8_t*)0x200000a9 = 0;
  *(uint8_t*)0x200000aa = 0;
  *(uint8_t*)0x200000ab = 0;
  *(uint8_t*)0x200000ac = 0;
  *(uint8_t*)0x200000ad = 0;
  *(uint8_t*)0x200000ae = 0;
  *(uint8_t*)0x200000af = 0;
  *(uint8_t*)0x200000b0 = 0;
  *(uint8_t*)0x200000b1 = 0;
  *(uint8_t*)0x200000b2 = 0;
  *(uint8_t*)0x200000b3 = 0;
  *(uint8_t*)0x200000b4 = 0;
  *(uint8_t*)0x200000b5 = 0;
  *(uint8_t*)0x200000b6 = 0;
  *(uint8_t*)0x200000b7 = 0;
  *(uint8_t*)0x200000b8 = 0;
  *(uint8_t*)0x200000b9 = 0;
  *(uint8_t*)0x200000ba = 0;
  *(uint8_t*)0x200000bb = 0;
  *(uint8_t*)0x200000bc = 0;
  *(uint8_t*)0x200000bd = 0;
  *(uint8_t*)0x200000be = 0;
  *(uint8_t*)0x200000bf = 0;
  *(uint8_t*)0x200000c0 = 0;
  *(uint8_t*)0x200000c1 = 0;
  *(uint8_t*)0x200000c2 = 0;
  *(uint8_t*)0x200000c3 = 0;
  *(uint8_t*)0x200000c4 = 0;
  *(uint8_t*)0x200000c5 = 0;
  *(uint8_t*)0x200000c6 = 0;
  *(uint8_t*)0x200000c7 = 0;
  *(uint8_t*)0x200000c8 = 0;
  *(uint8_t*)0x200000c9 = 0;
  *(uint8_t*)0x200000ca = 0;
  *(uint8_t*)0x200000cb = 0;
  *(uint8_t*)0x200000cc = 0;
  *(uint8_t*)0x200000cd = 0;
  *(uint8_t*)0x200000ce = 0;
  *(uint8_t*)0x200000cf = 0;
  *(uint8_t*)0x200000d0 = 0;
  *(uint8_t*)0x200000d1 = 0;
  *(uint8_t*)0x200000d2 = 0;
  *(uint8_t*)0x200000d3 = 0;
  *(uint8_t*)0x200000d4 = 0;
  *(uint8_t*)0x200000d5 = 0;
  *(uint8_t*)0x200000d6 = 0;
  *(uint8_t*)0x200000d7 = 0;
  *(uint8_t*)0x200000d8 = 0;
  *(uint8_t*)0x200000d9 = 0;
  *(uint8_t*)0x200000da = 0;
  *(uint8_t*)0x200000db = 0;
  *(uint8_t*)0x200000dc = 0;
  *(uint8_t*)0x200000dd = 0;
  *(uint8_t*)0x200000de = 0;
  *(uint8_t*)0x200000df = 0;
  *(uint8_t*)0x200000e0 = 0;
  *(uint8_t*)0x200000e1 = 0;
  *(uint8_t*)0x200000e2 = 0;
  *(uint8_t*)0x200000e3 = 0;
  *(uint8_t*)0x200000e4 = 0;
  *(uint8_t*)0x200000e5 = 0;
  *(uint8_t*)0x200000e6 = 0;
  *(uint8_t*)0x200000e7 = 0;
  *(uint8_t*)0x200000e8 = 0;
  *(uint8_t*)0x200000e9 = 0;
  *(uint8_t*)0x200000ea = 0;
  *(uint8_t*)0x200000eb = 0;
  *(uint8_t*)0x200000ec = 0;
  *(uint8_t*)0x200000ed = 0;
  *(uint8_t*)0x200000ee = 0;
  *(uint8_t*)0x200000ef = 0;
  *(uint8_t*)0x200000f0 = 0;
  *(uint8_t*)0x200000f1 = 0;
  *(uint8_t*)0x200000f2 = 0;
  *(uint8_t*)0x200000f3 = 0;
  *(uint8_t*)0x200000f4 = 0;
  *(uint8_t*)0x200000f5 = 0;
  *(uint8_t*)0x200000f6 = 0;
  *(uint8_t*)0x200000f7 = 0;
  *(uint8_t*)0x200000f8 = 0;
  *(uint8_t*)0x200000f9 = 0;
  *(uint8_t*)0x200000fa = 0;
  *(uint8_t*)0x200000fb = 0;
  *(uint8_t*)0x200000fc = 0;
  *(uint8_t*)0x200000fd = 0;
  *(uint8_t*)0x200000fe = 0;
  *(uint8_t*)0x200000ff = 0;
  *(uint8_t*)0x20000100 = 0;
  *(uint8_t*)0x20000101 = 0;
  *(uint8_t*)0x20000102 = 0;
  *(uint8_t*)0x20000103 = 0;
  *(uint8_t*)0x20000104 = 0;
  *(uint8_t*)0x20000105 = 0;
  *(uint8_t*)0x20000106 = 0;
  *(uint8_t*)0x20000107 = 0;
  *(uint8_t*)0x20000108 = 0;
  *(uint8_t*)0x20000109 = 0;
  *(uint8_t*)0x2000010a = 0;
  *(uint8_t*)0x2000010b = 0;
  *(uint8_t*)0x2000010c = 0;
  *(uint8_t*)0x2000010d = 0;
  *(uint8_t*)0x2000010e = 0;
  *(uint8_t*)0x2000010f = 0;
  *(uint32_t*)0x20000110 = 0x400;
  *(uint32_t*)0x20000114 = 0x2400;
  *(uint32_t*)0x20000118 = 0;
  *(uint32_t*)0x2000011c = 0;
  *(uint16_t*)0x20000120 = 8;
  *(uint16_t*)0x20000122 = 1;
  *(uint32_t*)0x20000124 = 2;
  *(uint16_t*)0x20000128 = 8;
  *(uint16_t*)0x2000012a = 1;
  *(uint32_t*)0x2000012c = 0x80;
  *(uint16_t*)0x20000130 = 8;
  *(uint16_t*)0x20000132 = 1;
  *(uint32_t*)0x20000134 = 0xd0406000;
  *(uint16_t*)0x20000138 = 8;
  *(uint16_t*)0x2000013a = 1;
  *(uint32_t*)0x2000013c = 1;
  *(uint16_t*)0x20000140 = 8;
  *(uint16_t*)0x20000142 = 1;
  *(uint32_t*)0x20000144 = 0x3ead9cbf;
  *(uint16_t*)0x20000148 = 8;
  *(uint16_t*)0x2000014a = 1;
  *(uint32_t*)0x2000014c = 3;
  *(uint16_t*)0x20000150 = 8;
  *(uint16_t*)0x20000152 = 1;
  *(uint32_t*)0x20000154 = 0;
  *(uint16_t*)0x20000158 = 8;
  *(uint16_t*)0x2000015a = 1;
  *(uint32_t*)0x2000015c = 0x800;
  *(uint16_t*)0x20000160 = 8;
  *(uint16_t*)0x20000162 = 1;
  *(uint32_t*)0x20000164 = 0;
  *(uint16_t*)0x20000168 = 8;
  *(uint16_t*)0x2000016a = 1;
  *(uint32_t*)0x2000016c = 1;
  *(uint64_t*)0x202ceff8 = 0x130;
  *(uint64_t*)0x2037ffe0 = 1;
  *(uint64_t*)0x2037ffe8 = 0;
  *(uint64_t*)0x2037fff0 = 0;
  *(uint32_t*)0x2037fff8 = 0x8820;
  syscall(__NR_sendmsg, r[1], 0x2037ffc8, 0x81);
  *(uint32_t*)0x2004d000 = 0;
  *(uint32_t*)0x2004d004 = 0;
  *(uint32_t*)0x2004d008 = 0;
  *(uint32_t*)0x2004d00c = 0;
  *(uint32_t*)0x2004d010 = 0;
  *(uint32_t*)0x2004d014 = 0;
  syz_emit_ethernet(1, 0x20000000, 0x2004d000);
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
