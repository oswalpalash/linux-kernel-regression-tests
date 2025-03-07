// https://syzkaller.appspot.com/bug?id=b0f0a3d34f0e9d551e1c0ab1fd911aaaa18bdcb9
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <errno.h>
#include <errno.h>
#include <fcntl.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/if_arp.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
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

#include <stdint.h>
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

static void snprintf_check(char* str, size_t size, const char* format,
                           ...)
{
  va_list args;

  va_start(args, format);
  vsnprintf_check(str, size, format, args);
  va_end(args);
}

#define COMMAND_MAX_LEN 128
#define PATH_PREFIX                                                    \
  "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin "
#define PATH_PREFIX_LEN (sizeof(PATH_PREFIX) - 1)

static void execute_command(const char* format, ...)
{
  va_list args;
  char command[PATH_PREFIX_LEN + COMMAND_MAX_LEN];
  int rv;

  va_start(args, format);
  memcpy(command, PATH_PREFIX, PATH_PREFIX_LEN);
  vsnprintf_check(command + PATH_PREFIX_LEN, COMMAND_MAX_LEN, format,
                  args);
  rv = system(command);
  if (rv != 0)
    fail("tun: command \"%s\" failed with code %d", &command[0], rv);

  va_end(args);
}

static int tunfd = -1;
static int tun_frags_enabled;

#define SYZ_TUN_MAX_PACKET_SIZE 1000

#define MAX_PIDS 32
#define ADDR_MAX_LEN 32

#define LOCAL_MAC "aa:aa:aa:aa:aa:%02hx"
#define REMOTE_MAC "bb:bb:bb:bb:bb:%02hx"

#define LOCAL_IPV4 "172.20.%d.170"
#define REMOTE_IPV4 "172.20.%d.187"

#define LOCAL_IPV6 "fe80::%02hxaa"
#define REMOTE_IPV6 "fe80::%02hxbb"

#define IFF_NAPI 0x0010
#define IFF_NAPI_FRAGS 0x0020

static void initialize_tun(uint64_t pid)
{
  if (pid >= MAX_PIDS)
    fail("tun: no more than %d executors", MAX_PIDS);
  int id = pid;

  tunfd = open("/dev/net/tun", O_RDWR | O_NONBLOCK);
  if (tunfd == -1) {
    printf(
        "tun: can't open /dev/net/tun: please enable CONFIG_TUN=y\n");
    printf("otherwise fuzzing or reproducing might not work as "
           "intended\n");
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

  execute_command("sysctl -w net.ipv6.conf.%s.accept_dad=0", iface);

  execute_command("sysctl -w net.ipv6.conf.%s.router_solicitations=0",
                  iface);

  execute_command("ip link set dev %s address %s", iface, local_mac);
  execute_command("ip addr add %s/24 dev %s", local_ipv4, iface);
  execute_command("ip -6 addr add %s/120 dev %s", local_ipv6, iface);
  execute_command("ip neigh add %s lladdr %s dev %s nud permanent",
                  remote_ipv4, remote_mac, iface);
  execute_command("ip -6 neigh add %s lladdr %s dev %s nud permanent",
                  remote_ipv6, remote_mac, iface);
  execute_command("ip link set dev %s up", iface);
}

static void setup_tun(uint64_t pid, bool enable_tun)
{
  if (enable_tun)
    initialize_tun(pid);
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
    close(fd);
    return false;
  }
  close(fd);
  return true;
}

static int inject_fault(int nth)
{
  int fd;
  char buf[128];

  sprintf(buf, "/proc/self/task/%d/fail-nth", (int)syscall(SYS_gettid));
  fd = open(buf, O_RDWR);
  if (fd == -1)
    fail("failed to open /proc/self/task/tid/fail-nth");
  sprintf(buf, "%d", nth + 1);
  if (write(fd, buf, strlen(buf)) != (ssize_t)strlen(buf))
    fail("failed to write /proc/self/task/tid/fail-nth");
  return fd;
}

#ifndef __NR_add_key
#define __NR_add_key 286
#endif
#ifndef __NR_keyctl
#define __NR_keyctl 288
#endif
#ifndef __NR_mmap
#define __NR_mmap 90
#endif
#ifndef __NR_openat
#define __NR_openat 295
#endif
#ifndef __NR_ioctl
#define __NR_ioctl 54
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[41];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  memcpy((void*)0x20fd7ff4,
         "\x2f\x64\x65\x76\x2f\x72\x66\x6b\x69\x6c\x6c\x00", 12);
  r[2] = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20fd7ff4ul, 0x0ul,
                 0x0ul);
  memcpy((void*)0x20000000, "\xcf\x11\x4f\x3e\x17\xab\x82\xb3\x2a\xcf"
                            "\x95\x85\x81\x36\x89\xbc",
         16);
  memcpy((void*)0x20000010, "\x69\x5a\x01\x00\x76\x74\x69\x1b\x00\x00"
                            "\xe2\xff\xff\xff\x00\x00",
         16);
  r[5] = syscall(__NR_ioctl, 0xfffffffffffffffful, 0x400454d9ul,
                 0x20000000ul);
  memcpy((void*)0x205a8ffb, "\x75\x73\x65\x72\x00", 5);
  *(uint8_t*)0x20fccffb = (uint8_t)0x73;
  *(uint8_t*)0x20fccffc = (uint8_t)0x79;
  *(uint8_t*)0x20fccffd = (uint8_t)0x7a;
  *(uint8_t*)0x20fccffe = (uint8_t)0x20;
  *(uint8_t*)0x20fccfff = (uint8_t)0x0;
  memcpy((void*)0x20d46000,
         "\xa9\x75\x3f\x2f\x58\x21\x3e\xd3\xfe\x15\xd3\x68\x31\x28\x30"
         "\x49\xfe\x76\x8d\x0a\xb3\xc5\x5d\x0a\x6b\x20\xe3\x50\xa7\xd0"
         "\x7c\x2b\x79\xef\xab\x14\xa1\x90\x6e\x0b\x03\x4c\xa4\xf5\x82"
         "\x0a\xb4\xc6\xf2\x95\x05\xc0\xcd\x16\x57\xff\x8c\x79\xec\xac"
         "\x99\x21\x2b\xaf\x26\x8a\xb5\x6f\x4a\x55\x7b\x39\x87\x80\xad"
         "\x06\x27\x7b\x2c\x22\x3f\xf5\xb9\xda\x96\x32\xcb\xc7\x8a\x9f"
         "\xaf\x14\x41\x0e\x4e\x94\x52\x73\x21\xb5\x72\x8b\x09\xdc\x73"
         "\x24\x94\xa3\xdf\x57\xb8\x27\xa3\x88\xb2\xcc\xbe\x72\x8b\x89"
         "\xb1\x80\x66\x94\x39\xe5\xac\xb4\x29\x24\xf9\x1b\x37\x8b\x8b"
         "\x70\x76\x23\x1a\x8e\xf5\xd5\xc3\xc8\x98\x67\x37\xce\x07\xea"
         "\x00\x06\x68\x95\xec\xe4\xf7\xe2\x2b\xf3\x79\x55\xdd\x94\xe3"
         "\x32\x41\x98\xba\xae\x9d\x98\xd9\x35\xd5\xcc\x50\x7b\xc4\x35"
         "\x17\x30\x64\xdc\xda\x4d\xf5\x05\x4d\x36\x10\x67",
         192);
  r[13] = syscall(__NR_add_key, 0x205a8ffbul, 0x20fccffbul,
                  0x20d46000ul, 0xc0ul, 0xfffffffffffffffful);
  *(uint16_t*)0x20603000 = (uint16_t)0x0;
  *(uint16_t*)0x20603002 = (uint16_t)0x0;
  *(uint16_t*)0x20603004 = (uint16_t)0x0;
  *(uint16_t*)0x20603006 = (uint16_t)0x0;
  *(uint8_t*)0x20603008 = (uint8_t)0x0;
  *(uint8_t*)0x20603009 = (uint8_t)0x0;
  *(uint8_t*)0x2060300a = (uint8_t)0x0;
  *(uint8_t*)0x2060300b = (uint8_t)0x0;
  *(uint32_t*)0x2060300c = (uint32_t)0x0;
  *(uint8_t*)0x20603010 = (uint8_t)0x0;
  r[24] =
      syscall(__NR_ioctl, 0xfffffffffffffffful, 0x5404ul, 0x20603000ul);
  *(uint32_t*)0x2046cff4 = r[13];
  *(uint32_t*)0x2046cff8 = r[13];
  *(uint32_t*)0x2046cffc = r[13];
  *(uint32_t*)0x20496fc8 = (uint32_t)0x20a46000;
  *(uint32_t*)0x20496fcc = (uint32_t)0x20000000;
  *(uint32_t*)0x20496fd0 = (uint32_t)0x0;
  *(uint32_t*)0x20496fd4 = (uint32_t)0x0;
  *(uint32_t*)0x20496fd8 = (uint32_t)0x0;
  *(uint32_t*)0x20496fdc = (uint32_t)0x0;
  *(uint32_t*)0x20496fe0 = (uint32_t)0x0;
  *(uint32_t*)0x20496fe4 = (uint32_t)0x0;
  *(uint32_t*)0x20496fe8 = (uint32_t)0x0;
  *(uint32_t*)0x20496fec = (uint32_t)0x0;
  *(uint32_t*)0x20496ff0 = (uint32_t)0x0;
  memcpy((void*)0x20a46000, "\x72\x6d\x64\x31\x32\x38\x00", 7);
  write_file("/sys/kernel/debug/failslab/ignore-gfp-wait", "N");
  write_file("/sys/kernel/debug/fail_futex/ignore-private", "N");
  inject_fault(10);
  r[40] = syscall(__NR_keyctl, 0x17ul, 0x2046cff4ul, 0x202e4000ul,
                  0x0ul, 0x20496fc8ul);
}

int main()
{
  setup_tun(0, true);
  loop();
  return 0;
}
