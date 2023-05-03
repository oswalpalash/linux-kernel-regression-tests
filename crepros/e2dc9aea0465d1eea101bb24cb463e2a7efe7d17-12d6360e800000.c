// https://syzkaller.appspot.com/bug?id=e2dc9aea0465d1eea101bb24cb463e2a7efe7d17
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/if_arp.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
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

static void test();

void loop()
{
  while (1) {
    test();
  }
}

long r[62];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0xaf4000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    r[1] = syscall(__NR_socket, 0x26ul, 0x5ul, 0x0ul);
    break;
  case 2:
    *(uint16_t*)0x20239000 = (uint16_t)0x26;
    memcpy((void*)0x20239002,
           "\x73\x6b\x63\x69\x70\x68\x65\x72\x00\x00\x00\x00\x00\x00",
           14);
    *(uint32_t*)0x20239010 = (uint32_t)0x0;
    *(uint32_t*)0x20239014 = (uint32_t)0x0;
    memcpy((void*)0x20239018,
           "\x63\x74\x72\x28\x74\x77\x6f\x66\x69\x73\x68\x29\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00",
           64);
    r[7] = syscall(__NR_bind, r[1], 0x20239000ul, 0x58ul);
    break;
  case 3:
    r[8] = syscall(__NR_accept, r[1], 0x0ul, 0x0ul);
    break;
  case 4:
    memcpy((void*)0x2028b000, "\xa2\x33\x64\xfd\x5e\x58\x0e\xf2\x4d\x71"
                              "\xa1\x9d\xd9\x3f\xc7\x27",
           16);
    r[10] = syscall(__NR_setsockopt, r[1], 0x117ul, 0x1ul, 0x2028b000ul,
                    0x10ul);
    break;
  case 5:
    *(uint64_t*)0x20054fc8 = (uint64_t)0x0;
    *(uint32_t*)0x20054fd0 = (uint32_t)0x0;
    *(uint64_t*)0x20054fd8 = (uint64_t)0x2028bff0;
    *(uint64_t*)0x20054fe0 = (uint64_t)0x1;
    *(uint64_t*)0x20054fe8 = (uint64_t)0x2028b000;
    *(uint64_t*)0x20054ff0 = (uint64_t)0xa8;
    *(uint32_t*)0x20054ff8 = (uint32_t)0x40800;
    *(uint64_t*)0x2028bff0 = (uint64_t)0x2005a000;
    *(uint64_t*)0x2028bff8 = (uint64_t)0x1000;
    memcpy(
        (void*)0x2005a000,
        "\x36\xc1\xda\x03\x4f\xd5\x96\x0c\x30\x22\x51\x65\xf2\x0f\xf9"
        "\xc3\x84\xf1\x94\xa9\xa1\x54\x7b\x83\xa7\x7e\xda\xcc\x80\x79"
        "\x5c\xc6\xaa\xe1\x6e\xee\x79\x29\xe4\xe0\xcc\x9e\x6f\x21\xa5"
        "\xdf\x28\xe6\x7e\x11\x5d\xc0\xb1\xf0\xe4\xee\xe0\x9f\x75\x53"
        "\xdf\xe9\x8d\xcd\x3b\xb5\xb2\x4f\xfc\xc5\x69\xf7\xbb\x66\x4f"
        "\xdb\xe3\x79\x0b\x3a\xdc\x0d\xba\xce\x22\xcb\xe1\x06\x6d\x61"
        "\x59\xfe\x62\x98\x6e\x89\xbf\x4f\x7b\x3b\x25\xbe\x4e\x5b\xe2"
        "\x22\x0d\x6a\xd5\x00\xe7\x0c\x4f\xe6\x9b\xf1\xca\x01\x7d\xa0"
        "\x78\x72\x74\x0d\x65\x2d\x9e\x23\x33\x83\x97\x15\xc1\xa4\xf8"
        "\x02\x48\x06\x26\x33\x77\xee\x63\xe9\xfd\x80\x2a\xfb\x42\x57"
        "\xac\x9a\xae\xc8\x66\xcb\x59\x52\x9b\xf4\x8a\xa8\xbf\x96\x98"
        "\x97\xae\x35\x2b\x30\xd9\xc1\x75\x65\xfa\x3b\x0a\x86\xbf\xed"
        "\x5a\x82\x3f\x8b\xd5\x20\x81\x72\x20\x34\x1b\x2a\x2c\xc4\x79"
        "\x37\xc9\xad\x0a\xe6\xd3\x34\xc7\x3a\xe8\xfa\x6d\xb3\xf2\xca"
        "\xd2\x6b\x59\x8f\x4c\xc7\x2c\xff\x93\xae\xa8\x64\x54\xba\xea"
        "\x4c\x7d\x61\xd0\xe8\x66\x5d\x08\xe5\x05\xb5\xab\x54\x03\x3f"
        "\x8c\x7e\x4b\xb4\xe5\x61\xba\x2c\xd0\x51\xa6\xe9\x3c\xb7\xa1"
        "\x4b\x4f\x6a\x5f\x23\x91\x80\x47\x94\xd0\x28\x61\xe7\x64\xd3"
        "\x98\x1f\x37\x40\x0d\x5a\x3a\xd8\xd4\xa1\x91\x29\x9a\xc1\xd2"
        "\x5a\x79\x3e\xab\xb7\xb8\x36\xe2\xc8\x44\x08\x21\x36\x10\xfb"
        "\xe0\x88\x84\xec\x04\xa3\x8b\xaa\x2c\x31\xa8\x2e\x04\xec\x2f"
        "\x13\xdb\x59\x57\x5b\x76\xb7\xd7\xb3\x13\x31\x6e\xd9\x51\xbe"
        "\xb9\x64\xed\x8e\x6d\x32\x3d\x90\xdd\x98\x80\x61\xb1\xa6\xb1"
        "\x50\x2a\x8a\x51\x50\x4f\x67\x68\x02\x90\xfd\xeb\x4f\xdb\xad"
        "\xab\xf4\x4b\xfb\x99\x1b\x75\xd1\xbc\xf6\x9b\x69\x93\x49\xe5"
        "\x1a\xb5\xc9\xa7\x08\x0f\x2c\x26\xfd\xdc\xb3\xfe\x8a\x9d\x29"
        "\x01\xe5\x25\x25\xb9\x31\x26\x0f\xa2\x5a\x83\x59\x91\xea\xdc"
        "\xdb\xbd\x82\xf9\x43\x52\x5f\x1a\x71\x44\x24\xee\x36\x3c\x26"
        "\x1a\x29\xd5\x0d\x8d\x90\xa7\xae\xf4\xf0\x7c\x80\xee\x6e\x74"
        "\xda\xe3\x94\xd5\x66\x9a\xc3\x96\x9c\xb4\x3a\xb4\x89\x37\x97"
        "\x49\x75\x08\x45\xcb\x4c\x97\x42\xc3\xae\x7c\x1d\x8a\xf0\xff"
        "\x0f\xd7\x20\x5b\xbe\x45\x05\x9f\x23\x7b\x5a\xa4\x6f\x91\x8a"
        "\x89\xde\x45\x01\x18\x02\xdb\x84\x1a\x03\x6d\xde\x05\x49\x5f"
        "\xba\xc0\x69\x63\x16\x64\x81\xf4\xc3\xae\x01\xa4\xac\xbf\x6f"
        "\x84\xd9\x4a\xc0\x37\x26\x62\xd7\x62\x97\x24\x7a\x90\xdb\xdd"
        "\x95\xea\x28\x46\xaa\x0a\x66\x11\xd4\x06\x6f\xe0\x12\xf9\x02"
        "\xb2\x1e\x55\xd5\x3d\xac\x37\x71\x4d\x3e\x21\x06\x87\x89\x8b"
        "\x43\x88\x2e\xca\x1a\x91\xbd\x1c\xbf\xb8\x4f\x8c\x29\x2b\x5f"
        "\xc7\xff\x1c\x38\x0d\xb1\x3b\x78\xc4\x7c\x99\x8b\xf9\x26\xd3"
        "\x28\x00\xf6\xf3\xfd\x5c\xcd\x4f\xaa\xf4\xf0\x40\xfc\x95\x3c"
        "\x0a\x8f\xd5\xf6\x74\x35\x5a\xa4\xf3\x18\x94\x2d\x6e\x16\x6d"
        "\xe1\xc8\x03\xb5\x1d\x99\x4b\x13\xd4\x58\xfc\xab\x31\x52\x67"
        "\xd2\x83\x2b\x85\xc1\xcc\x75\xf5\xe1\x20\xb2\x2a\xe5\x15\x93"
        "\x78\x2a\xb2\xb9\x25\xea\x4d\xe7\x38\x12\xa4\x0e\x3a\x74\xb0"
        "\x7a\x81\x18\xe2\x85\x9a\x78\x98\x1b\xb3\xdc\xde\x0f\x35\xa9"
        "\x8e\xb0\x46\xd3\x79\xe9\x48\x1f\x35\xd8\x25\xac\x21\x32\xa8"
        "\x05\x5e\xf2\xfc\x2f\x42\x6f\xe7\x8e\x0a\x47\x63\x7a\x3b\xcf"
        "\x6a\xa9\x99\x98\x31\xef\xb7\x89\xa8\x2e\x6b\x9d\x9c\x47\x04"
        "\x70\xd3\x89\x13\x77\x86\xe6\x44\xbe\x0b\x50\x28\xdc\x1a\x41"
        "\xbd\x74\x96\x24\x99\xc5\x53\xc1\x8e\x0c\xc5\x57\xf2\x48\x08"
        "\x58\x8f\x0a\xb3\xd9\x87\x59\xc4\x9c\x89\x4c\x81\xce\xc6\x31"
        "\xa0\x3e\xe9\x63\x80\xff\xfb\xad\x13\xb7\x1f\xf3\x87\xc3\x1a"
        "\x12\xba\xaa\x8e\x60\x0a\xad\x91\xa9\xa1\x62\x37\xb7\xa5\xec"
        "\xb0\x0f\xf9\xc0\x17\xed\xc0\xe1\xd4\x14\x1c\xbb\xda\xa3\xa8"
        "\x4c\xe1\xfd\x20\xd2\x47\x9f\x0f\x17\x90\x00\x99\x05\x4d\x06"
        "\x1a\x37\x34\xa7\xdd\xe8\xb4\x67\x0f\x1e\xec\xa7\x1f\xac\x95"
        "\x0f\x31\x14\x73\x54\x1f\x78\x9a\x30\x87\x2d\xa9\xde\x53\xb8"
        "\xca\x78\x79\x11\x3d\xae\x94\x49\xab\x38\xaf\xcd\x85\x70\x87"
        "\x90\x66\x65\x6b\x76\x39\xc9\x0d\x54\x92\xe4\x01\x72\xce\x06"
        "\x43\xb8\xff\x0b\x63\xc4\x88\x77\x13\x8a\xbf\x8b\x11\x32\x8d"
        "\xf9\x96\x6f\xf3\xba\x3c\x81\xa1\x55\xfa\xe2\xe6\xc4\x3d\xd4"
        "\x11\x47\x3b\xa6\x2e\x9e\xe8\x38\xc6\xa6\x3f\x94\x89\x5b\xe6"
        "\xee\x87\xe1\xe9\x5a\x69\xa1\x3e\xff\x66\x27\x89\xe5\x19\x6f"
        "\xed\xce\x29\xef\x61\xed\x85\xc9\x14\xd2\x71\x86\x28\x8c\x15"
        "\x23\xc2\xa6\xf0\xad\xce\xa9\xdb\x37\xbb\xe3\x27\x25\xbd\x35"
        "\x36\x32\x6b\xd4\x3d\xe3\x7e\x4c\xa4\x81\x30\xb0\x3e\x98\xc4"
        "\x99\x23\xb7\x6e\x5d\x26\x3d\x79\x6b\x0b\x69\x78\x3d\x76\x38"
        "\x7f\x75\xc4\x17\x06\x81\x01\x74\x82\xd6\xba\xd1\x5e\x24\x71"
        "\x64\x8c\x2c\x47\xc6\x46\xda\xf3\xb1\x2f\xd1\x2a\x19\xf4\x2f"
        "\xc3\xb7\xad\x21\x2d\x58\xd7\x5a\xe2\x8f\x82\x7a\x51\xe3\x8a"
        "\x24\x77\x55\xc7\x44\xe2\x26\xa9\x7f\xb6\xd7\x75\xee\xf9\x79"
        "\x7a\x54\x69\xcd\x9a\xea\xbf\x27\xc5\xe0\x3c\x93\x59\x21\x5b"
        "\xfd\x1f\x02\xf1\x50\x8f\xaa\x5b\x25\xe5\x11\x2f\x92\xdb\x1d"
        "\x20\x50\x74\xf9\x3d\x7e\x07\x98\xa9\x1e\xca\x25\x83\x6e\x9b"
        "\xe2\x5a\x96\xb4\xdd\xd3\xe2\xd8\xf9\xc3\x5a\x67\xae\x8f\x81"
        "\x40\xeb\xdc\x44\x08\x54\xef\xd2\x98\xbd\x90\x55\x42\x15\xdf"
        "\x5a\x77\xf7\x71\x79\x5e\x6e\x29\x58\x1e\xe3\x92\x3e\x1b\x72"
        "\xbf\x0c\x27\x4a\x3f\x89\x96\xa7\x4d\xb6\x41\x2d\x1c\x7d\xa8"
        "\xd0\xeb\xdd\x85\xc0\x9b\x52\x81\x3c\x9d\xda\x62\xab\x1e\x5f"
        "\x88\xba\x6d\x9d\x41\x62\x79\x56\x9f\xdf\x2b\xc3\x02\x0d\xe4"
        "\x33\x50\xd6\x2e\x83\x77\x26\x65\xf9\x76\x34\x74\x9b\x81\x2d"
        "\xd9\x31\x03\x1a\x3d\x40\x5d\x43\xb6\x88\x7f\x31\x6c\xbc\x9a"
        "\xfc\xe5\x11\xcf\xf3\xf8\x57\x80\x51\xd5\x61\xf9\x87\x59\x1d"
        "\x07\xdd\xe8\x61\x17\x81\x67\xb5\x0d\x30\x06\xc9\x6b\x7e\x98"
        "\x76\xab\x68\xa7\x5e\x2b\x1e\xd8\xa8\x56\x5b\xf1\xa6\xe2\x11"
        "\x58\x29\x14\x5d\x85\xf9\xc1\x94\x31\x48\x48\xfb\xc0\x03\xf1"
        "\x94\xfd\x96\x11\xae\x5f\xf0\xcb\xa5\xcc\xef\xe0\x72\x41\x11"
        "\xa8\xff\x76\x27\x31\xe9\xf1\x97\x01\x70\xf2\x14\x7f\x3a\x03"
        "\xf9\x11\xf5\xc1\x13\x9c\xcb\xb5\xb6\xaf\xbd\xf6\x3d\xbe\xb9"
        "\xad\x05\xc9\xa3\x30\x84\x0e\x58\x5f\x56\x4c\x5e\xa6\x90\x54"
        "\xe7\x2d\x21\x54\x11\x4b\xf9\xa9\x1e\xb5\x0f\xff\x6c\x6b\xf4"
        "\x2e\x65\x6a\xa1\x1f\x3a\x2b\xdd\xbd\xf1\xfb\x29\x1d\xc5\xd3"
        "\x97\x25\xbb\xd6\xe2\x59\xcf\xf0\x8e\x8d\x66\x59\xcd\x08\xf9"
        "\x58\x31\xe3\x78\xa3\x81\x31\xb8\x64\xe9\x92\xb0\xab\x50\x37"
        "\x17\x29\xfc\x16\x51\x9b\x19\xc5\x40\x6b\x49\x2a\x66\x83\xa2"
        "\x55\xab\x72\x59\x12\x75\x51\x5a\xc0\xcb\x4f\x2b\x9f\xf6\x95"
        "\x2b\x9c\x95\x78\x0a\x89\x1a\xcf\xcc\x67\xf5\x90\xbd\x53\xfc"
        "\xdb\xa5\x8c\x32\x99\xc1\x55\x0b\xda\xf3\x67\xdc\x8d\xc0\x0c"
        "\xae\x45\x93\x01\xd5\x46\xb6\x13\xde\x8b\xd1\x70\xb0\xfe\x82"
        "\xf0\x13\x12\xda\xa4\xd1\x20\x81\xf8\x5c\x4f\x19\x4d\x07\xa9"
        "\x7f\x47\x5a\x88\xf6\x44\x5d\xed\x8e\xc7\x73\x0b\x11\xe2\x9a"
        "\xdc\x53\x00\xed\xa1\x26\x36\x29\xc8\xa3\x5c\x3b\xcf\xef\x02"
        "\x31\x24\xd7\x0b\x34\xc4\x8e\x4c\xb3\x00\xc1\x03\xc0\xfd\x30"
        "\xa6\x48\x38\xe2\x0e\x2e\xcc\x72\xc8\xeb\x84\x93\x35\xcd\xc5"
        "\xd4\xd6\x62\x1a\x26\x23\x26\xd1\x21\x96\xbc\x53\x9a\x99\xbc"
        "\xaa\xfc\xf5\xcf\x25\x33\x5c\x68\x1b\x5c\x6b\x3f\x53\x71\x61"
        "\x24\xbb\x33\xa9\x50\x8e\x85\xac\xc0\x82\x23\x13\xd5\xfa\x6f"
        "\xa9\xc9\x23\x26\x43\x40\xdb\xaf\xf9\xd2\x70\x65\xd7\x4e\xd3"
        "\x28\x66\x0f\xb9\x8c\x46\xc6\x76\xd8\x26\xf0\xe5\xfc\xe9\xf2"
        "\x1e\x9a\xc5\x59\x7f\x5f\xd1\xe2\xf1\xd9\xff\xea\x49\xd3\x83"
        "\xff\x55\x56\x11\x76\x9e\x60\x07\xd1\x24\xb7\xd2\x6f\x8c\x02"
        "\x4c\xbf\xd1\xcd\x20\xdd\xf2\x1e\xd5\x9a\x85\xff\xd3\x52\xd8"
        "\x70\x31\x74\xaa\x83\x1d\x71\x4f\xba\x1c\xb6\x91\xfb\xf4\x86"
        "\x8c\x68\xb4\x85\x3e\x4e\x76\x32\xc6\xc5\x54\xfe\x4a\x56\xc3"
        "\x48\x14\x60\x51\x20\x03\x13\xed\xe4\xb1\x84\xaa\x3b\xd7\x68"
        "\xb7\x16\x57\x43\x54\x1c\xb3\xd9\xac\x31\x4f\xe2\x48\x57\xaf"
        "\x26\x7e\x9c\x04\x88\xa9\xe1\x30\xbe\xad\x85\x57\xe9\x3f\x99"
        "\x09\x1c\x65\x83\x52\x2e\x4c\x64\xde\x48\xcb\x38\xe5\xe5\xc0"
        "\x9b\xbe\xeb\xad\x63\xd7\x30\xc7\x30\x3a\x73\x4f\x62\xc6\x8d"
        "\x4d\x80\x75\x3f\xfe\x1e\x4c\x29\xe0\xa4\xc6\x0a\x05\xd5\x91"
        "\x6e\xcf\x2d\xf5\x27\xe5\xef\x68\x14\x64\x7b\x36\x78\xe7\x68"
        "\xb9\x74\x65\x1a\xe4\xba\x0f\xe8\x60\xcf\x8c\x05\x2b\xe1\xd7"
        "\xeb\x56\x44\x06\x42\x1f\xb5\x22\x09\x38\xa1\x15\x8e\x14\xd3"
        "\xec\x6e\x1d\xa3\x83\x4e\xed\x2c\x11\x44\xca\x33\xde\xd3\xf5"
        "\x60\xa2\x67\xed\x27\xb2\x41\xc7\x39\xee\x54\x1e\xae\x42\xeb"
        "\x7e\x50\xcb\x9a\x70\x06\x38\x9c\x69\xf0\x0c\x7e\xcc\x31\xf9"
        "\x08\x96\x1b\x24\x26\x12\xab\x21\x43\x5d\x87\x86\x85\xe0\xb0"
        "\xa2\xb6\xe2\xfe\x0d\x07\x6a\x27\x5e\x47\x0a\x23\xc2\xf0\x82"
        "\x4b\x44\x76\x14\xd4\x91\x14\x70\xb0\x56\xc9\xe4\xc0\x82\x87"
        "\x25\x3c\xc0\x43\xfc\x8f\x86\xce\xe3\xbb\xc2\x06\x1e\x94\x58"
        "\x32\xa8\x24\xce\xd7\x04\xfe\x6b\x4b\x92\xa3\xcb\x68\xaf\x3a"
        "\x19\xa9\x95\x8b\xd5\x1f\x79\x6d\xb4\xe1\xd4\xe6\x00\x7f\xdb"
        "\x07\x47\xbf\x72\x31\x25\x25\xb7\x28\xcf\xc4\xc7\x0b\x53\x9a"
        "\xc9\x65\xce\x10\xe3\x5f\x3c\xac\x4d\x97\x52\x50\xb3\x81\x9a"
        "\xac\x40\x42\xd7\x29\xec\x3b\x9f\xd3\xdd\x46\x0f\x48\x9f\x1d"
        "\x29\xab\xe2\x34\x83\xe8\x25\xa7\xda\xbf\xbd\xf0\xbd\x1d\x93"
        "\xa6\xb1\x0f\xc6\x5d\xe1\x43\x8f\x74\xc6\x3f\x3b\xc4\x8e\x1e"
        "\x2e\x2d\x82\xdd\x3e\xc8\x3c\xda\xc4\xd9\x5c\x79\x66\x1c\x15"
        "\xdf\xc9\xf9\x4b\x5c\xee\x02\xd6\x2a\x83\x6a\xea\xab\x31\x4a"
        "\x56\x72\x39\x29\x24\x63\x9c\xd4\xce\x21\xc2\x05\xa0\xad\x86"
        "\x24\x3e\x26\x05\xa3\x4c\xb0\xc2\x67\x0f\xab\xff\x7c\xff\x02"
        "\x1d\xdb\x47\xf8\x7f\x00\x3e\x60\xa1\xed\x19\xb2\x6a\x01\x5c"
        "\xa7\x77\x51\x9c\x71\x57\xf0\x59\xd7\x19\xd2\x6e\xb3\xe2\x6a"
        "\x62\x43\xad\x81\x10\xc8\xca\x16\x4e\x71\x7e\x78\x1f\xd9\xf0"
        "\x95\x1c\x7d\x9c\xcc\xe8\xe9\xfb\xe8\x0d\x3c\x69\x80\xad\x58"
        "\xa7\x77\x6d\xf5\x51\x7c\xbb\xb7\x03\x60\xf7\x83\x7b\x66\x8c"
        "\xde\x4d\xed\x8a\x98\xe8\x75\xf8\x4d\x48\x19\x00\xe5\x4f\x91"
        "\x80\x05\xab\x39\x11\xbb\xb3\xa5\x58\xaa\xf5\x26\xfc\x9e\xb5"
        "\x17\x67\xc9\x98\x7f\x72\xa2\x00\x6b\x0e\x25\xd8\xd8\x6a\x41"
        "\x40\xd6\x2b\xff\xa5\x7f\x7e\xf3\xba\x3f\xc6\x33\x40\xa5\xc8"
        "\xf2\x58\xdb\xf1\x91\x92\xf5\x31\x7b\x7c\xc9\x9f\x6d\x25\xce"
        "\xdc\x77\x51\x9c\xd4\x5b\x13\x30\x1f\x0d\x79\x6e\x75\x44\x6e"
        "\x89\x1b\x20\xac\x14\x94\xaf\x9a\x1e\xb0\x49\x56\xfb\xb8\x5e"
        "\x02\xbc\xd3\xba\xa6\xd6\xba\x0f\x51\xd9\x2d\x18\x0e\x34\x22"
        "\x84\xe2\xe7\xa5\xfb\x66\x8a\xa6\xf6\xa3\xe1\x94\xca\x53\x28"
        "\xa8\x3f\xa3\xd8\x68\x5a\x45\xf5\xde\x93\x1d\x18\xf5\x0f\x53"
        "\x13\x4d\x65\xc2\x11\x50\xfa\x83\x6c\x9e\xda\xb2\x2c\x4c\x1a"
        "\x69\xdf\xcf\xae\x5b\x79\xc2\x2d\x7f\x02\x57\x20\x50\x0f\x69"
        "\x7f\xf5\x52\x5c\x24\x0c\xc6\x3f\x0a\x9b\x70\xcb\xe7\xdc\xc7"
        "\xac\x3a\x76\x6c\x3d\x0e\x86\xcd\x80\xb3\xa0\xa0\x0b\x66\xdc"
        "\x4b\xa5\x23\xb1\x7c\x27\x2d\xe0\x26\x1d\x4e\xfe\xfe\xc2\xd7"
        "\xc8\x46\xe6\x98\xbb\x94\xea\xca\x3d\x2f\x31\x33\x2b\x8d\x60"
        "\x7f\xa8\x7d\x9a\x48\x5f\x73\xb8\x3e\x2e\x02\x2b\x2d\xf1\x8c"
        "\x89\xe5\xfa\xbc\x05\x81\xc9\x3e\xea\xd8\x5a\x87\xaf\xe4\xaf"
        "\x8c\xab\x57\x12\x2b\xc1\x80\xf9\x8a\xdd\xe6\x74\xfa\x5b\xc8"
        "\x1e\x3d\xe6\x9f\xb4\xee\x25\x90\x0c\x7f\x40\xc8\xb0\x2d\xc3"
        "\x65\x8e\xc2\x95\x34\xfb\x92\x88\x09\xbd\x24\x92\x20\xd0\xf9"
        "\x1c\xc7\xc9\xc7\x5b\x0c\xb0\x47\x35\x61\xf2\xc7\x46\x31\x5a"
        "\x9e\xec\xd2\x15\x1b\xcb\x73\xdd\xd6\x6c\x46\x4f\x0d\xde\x0f"
        "\x1e\x7c\x97\xc1\x38\xd3\x24\xfa\x97\xd3\x73\x1b\x9d\x00\x3d"
        "\x58\x00\x30\x8f\x39\xb5\x05\x47\x3c\xd4\x68\xf5\xb4\x48\xc2"
        "\x47\x4b\x02\xce\x24\x83\xf4\xc0\x80\x50\x9a\x26\xf9\xa6\x85"
        "\xc3\x15\xf6\x6a\xe3\x66\x81\x02\xae\x4a\x18\x27\xc9\x07\x4c"
        "\xf3\x22\xed\x6f\x4b\xf6\x73\x32\xbf\x14\x05\xc1\xf3\x5a\xc5"
        "\x43\xa7\xa5\x63\x01\xe6\x5c\xbe\x91\x3c\x5a\x96\x8b\x15\x52"
        "\xff\x29\x61\x78\x28\x6f\xcb\x84\x24\xa5\x61\x03\xeb\x7d\xc8"
        "\x2a\xcf\x27\x6f\xb3\x6f\xf0\xcb\xbd\xd5\x05\x67\x7d\xb4\x5b"
        "\x5b\xa2\x06\xe7\xfe\x04\x8b\x6c\x3c\x09\xaa\xbf\x7b\xdc\x7a"
        "\xd0\x3f\x38\xc5\x0c\x3f\xc7\x5c\xf0\x26\x2b\x83\x0c\x51\xd4"
        "\x29\x46\x41\xb2\xf2\x17\xe3\x12\x07\x24\xf3\x61\x92\xcd\x08"
        "\xf9\xaf\x23\x84\x49\x2a\x21\x6c\x9d\x9a\xbd\xe2\x7a\xb3\xed"
        "\x94\x01\x37\x89\x62\xf9\xbf\x22\x5c\x11\x36\x6a\xd8\x99\xe7"
        "\x50\xd2\xc9\x3f\x16\x58\x6e\xd2\x38\xd1\x95\xfc\x34\xd6\xce"
        "\xd1\x74\x9b\x8d\xf1\xc8\x0a\x16\x32\xd3\x12\x1d\x48\x0a\xdf"
        "\xff\x9e\x41\xdd\xad\x9c\x86\xc9\x18\x0f\x25\xd6\xa6\x0e\x8e"
        "\x08\xde\x36\x86\x47\x76\x1f\x02\x83\x1d\x3d\x86\xdd\xad\x7f"
        "\x49\x91\x51\x22\x68\x39\xa2\xf9\x18\x61\x67\x6e\x55\xbb\x48"
        "\x5a\x27\x29\x1f\x61\xe6\x53\xb9\xd8\xcf\x34\xa9\x8e\x27\xc1"
        "\xf5\xab\x32\x7f\xfc\xce\x45\x51\x1c\xcf\xdf\xbc\x47\x0a\xf5"
        "\x20\x56\xe7\x1f\x24\x6c\x10\x64\x59\x20\xbf\x26\xcf\x18\x19"
        "\xb4\xe6\x4b\xf6\x56\x9c\x33\xd5\xbf\x5c\xf4\x3f\xdc\x0e\x59"
        "\x44\x83\x16\x92\x90\x12\x3e\x27\x6e\xc4\xce\x94\x70\x18\xf1"
        "\x12\x68\x3e\xb1\x1c\xe9\xd1\x8a\x02\x7d\x6a\xa5\xd1\x0a\x30"
        "\xa9\xc5\x9c\x56\x25\x1a\xd7\x2a\x2b\x9f\x3f\xcb\x7c\x1c\x12"
        "\x3c\x57\xf5\x42\xde\x5c\x92\x37\x52\xd7\x70\xb9\x3c\x90\x0e"
        "\x73\x57\x3c\xf0\x6c\x59\x10\xa7\x43\x2e\x45\xa4\x63\xf0\xb4"
        "\x5a\x70\x98\xe8\x34\x48\x18\x1f\x8d\x2c\x1e\x01\x77\x25\x07"
        "\x79\xf0\xaf\xdc\x72\x8d\xcd\x5b\xf7\xb3\xc1\x62\x9d\xb6\x8f"
        "\x12\xd0\x60\x19\xff\x33\xee\xc8\x1f\x73\xc9\xc1\xa6\x83\x21"
        "\x85\x4e\x5e\x60\xe8\x13\x50\x0e\x33\x35\x8f\x9a\x1f\xbd\x38"
        "\x59\x4c\x88\xf1\xde\x30\x1b\x61\xfe\x91\x83\xb5\xf9\xc2\xdc"
        "\x5b\xf4\x1d\xd0\x73\x5d\x64\xb9\x0f\xf0\xaf\xee\x67\xfc\xe7"
        "\x85\x3a\x10\x32\xca\xcf\x3e\x2d\x44\x04\x64\x60\x6f\x98\x8e"
        "\xe4\x3c\x91\x34\x0b\x27\x12\xd1\x5c\x19\xbf\x89\x9e\x07\xc5"
        "\xc8\x85\xda\xd0\x40\x36\xd1\xf4\xe4\xf9\x0b\x43\x22\xe8\x67"
        "\xdf\x98\x2f\xcf\x21\x84\xe7\x80\x24\x39\x58\x8c\xbe\x5a\x39"
        "\x9a\xa6\x02\xbe\x5b\x60\x43\x61\x5d\x07\xe7\x50\xca\x71\x45"
        "\x50\xc8\x1a\xb3\x76\xb7\x19\x40\x33\xd9\x2b\xb5\x2c\x70\x6e"
        "\x05\x7a\x0b\x7c\x2c\x7c\x51\xf4\x88\x06\xeb\xda\x27\x67\x1c"
        "\xb3\x7b\x9f\x94\x90\xf4\xa2\xae\x8f\xd6\xe9\xa6\x97\xf2\x8f"
        "\x07\xec\x65\x2e\x05\x3f\xee\x75\xe9\x01\x19\x3c\x32\x70\xec"
        "\x0d\xde\x99\x9f\x7a\xe4\x18\x6c\x65\xda\xf9\x4f\x22\x34\x8c"
        "\xfd\x6c\xad\x62\x50\x9c\xc8\xa1\x71\x89\xa2\x35\x3e\xf1\xbc"
        "\xea\xda\x26\x9b\x21\x43\x3d\x5f\x7b\xfd\x17\x8e\x24\xb8\xd7"
        "\x23\x05\xba\x1b\x63\xf3\xae\x77\xf0\xbe\x82\x8e\x13\x7b\xbf"
        "\x08\x62\x62\x0f\x7b\xc4\x2e\x72\xb7\x2f\xd4\x1f\x22\x8c\xe7"
        "\xc2\x5c\xf1\x15\xbb\xdd\xe1\x29\x02\x16\x38\xd3\xe3\xb9\x8d"
        "\x64\x59\x27\xed\xcc\x12\xbb\x44\xb6\x56\xf6\x31\x39\x1a\x07"
        "\x5b\xc6\xe3\x18\xc8\x64\x6c\xae\x6f\x54\x26\x0a\xd1\x4a\x11"
        "\xea\xe9\x07\xdf\x01\x6e\xa5\x9c\xa4\xa6\x6c\x01\x08\x92\x00"
        "\x6e\x20\xdb\xe7\xcd\xde\x3a\xc3\x7e\x61\xb2\x8e\x2e\x85\xcd"
        "\x0c\x33\x4b\xc7\xb7\x44\x21\xa0\x28\x4a\x5a\xbe\x12\x7e\xac"
        "\xfb\x19\xfb\x32\x5e\x73\xb8\x91\x08\x9b\x34\xfa\x52\x7a\x16"
        "\x5b\xb9\xfc\xdc\xcd\x06\x9d\xf3\xf9\x82\x0d\x16\x02\x62\xa3"
        "\xf0\x2d\x4b\xb8\xef\x0c\x9e\x5a\xff\x84\x3f\x0a\xa3\x6e\x1a"
        "\xe2\x81\x6b\x27\x6e\x28\x41\x7f\x96\x4f\xc7\x7b\x50\x4b\xb8"
        "\xb0\x8d\xac\x8c\x57\xb8\xfb\x76\x0e\xa3\x16\xf1\x3f\x8d\xd4"
        "\x3f\xc9\x84\x22\xd0\x5b\xdd\xeb\x1b\xcb\xce\xf0\xbd\x3c\x5f"
        "\x70\xc2\xc3\xb1\x7b\x8c\xf0\xfb\xea\x21\xf5\xe5\xd0\x5f\x8d"
        "\x4e\x3f\xcb\xca\x25\xa4\x95\x9b\x3a\x00\x8c\x89\x46\x0a\xb3"
        "\x7c\x54\x4c\x23\x3c\xc6\x7c\x05\x82\x13\x98\xc8\x25\xd5\x0a"
        "\xfe\x4d\xb0\x2a\xb7\x35\xf5\x66\xc6\x53\xf2\x0b\x10\x55\xb6"
        "\x93\x7c\xa6\xb7\x75\xd2\x0d\x76\x5c\xec\x29\x0d\xa7\x54\x18"
        "\x16\xf9\x2a\xae\x33\x6c\x40\x25\x8a\xca\xc9\xdc\x49\xac\x4a"
        "\x4d\x2c\x26\x8d\xd8\x0d\x28\xf7\x34\xd7\x6c\xe0\x72\xca\x0a"
        "\x35\x91\xfd\x1d\x95\xb7\xfb\x3f\x6a\xf7\x34\x22\xb0\x06\xd8"
        "\x7d\x49\x4c\x00\x2a\xd3\xcd\x22\xd5\x05\x97\xa4\x9d\x3f\x29"
        "\x5e\x77\x30\xec\x60\x2d\xe7\x7a\x5e\x54\x78\x16\x34\x3a\x5d"
        "\xff\x45\xc2\xd1\xbd\x91\x76\xe0\xae\x9b\xdd\x7e\x94\xa0\x57"
        "\x4e\xf9\x96\xce\xd9\xcf\xe8\x3c\x92\x80\x00\x11\x5e\xab\xcd"
        "\x45\xb3\x3b\x96\x38\xb9\x6a\xb7\xbf\x40\x83\xe6\xcb\x50\x5c"
        "\x50\x0c\x9a\x10\xb3\xbf\xb8\x29\xfa\xba\xc4\x6c\xe0\x72\xcf"
        "\x11\x53\x52\xde\x95\x48\x9f\x74\x47\xae\x8d\xc9\x53\x7d\x38"
        "\xe8\x72\x7e\x42\xf0\x04\xb5\x53\x65\xd2\xb3\xa2\x77\xda\x4d"
        "\xac\x64\x77\x39\x2c\xb3\x51\x1f\x3e\xd5\x46\x4f\xd9\xe8\xe5"
        "\x24\x87\xe0\x83\xeb\x6d\x8a\xaf\xf4\x0d\xba\x3d\x4f\xb5\x71"
        "\xe2\x76\xb7\xfd\x80\x1e\xeb\x15\x33\xf2\xba\x46\x81\x38\x47"
        "\x2d\x67\x75\x9f\x09\x3d\x10\x76\x6c\xaf\x4e\xb1\xb8\xa6\xca"
        "\x7a\xe3\xdf\x32\x81\xbc\x91\x42\x40\x87\x81\xae\x90\xe9\xbc"
        "\xae\xdc\xf5\xd8\x74\x17\x47\x6b\xb0\x95\xfb\x57\x48\x94\xba"
        "\xa0\x2c\xa0\x39\x14\x98\x47\x7c\xbf\x72\xa4\x62\x2c\x18\x82"
        "\x91\x56\x32\x52\x2b\x9b\x9a\x41\x4a\xda\xc5\x05\xae\xe1\x49"
        "\xae\x86\x43\x9c\x4f\x5b\x1a\xd5\x7a\xd5\x5a\x85\x73\x34\x82"
        "\xe4\x53\xad\x6b\xcf\xee\xd9\x80\x92\x21\xdb\x8c\x3b\x72\x82"
        "\x2c\x04\xef\x0b\x6e\x04\xd8\x4d\xd8\x87\x3e\x38\xf8\x45\x1c"
        "\x8d\xfd\xac\xec\x8b\xac\xac\x69\x8f\x49\x8a\xda\x19\xa7\x89"
        "\xeb\x69\x31\x1e\x67\x4f\xb9\x7f\xed\x67\xf6\x6a\x78\xbf\xa8"
        "\x97\xb6\x8f\x21\xdc\x50\x5b\x3d\x28\x10\x29\x5f\x93\xbc\x55"
        "\x14\x02\xc3\x77\xed\x56\x9d\x94\x7f\x4d\xd0\x88\xa5\xef\x94"
        "\x3b\xa4\x83\x5a\xdd\xe5\x22\x9d\xa8\xc0\x09\xfc\x29\x6a\x09"
        "\x84\x18\x3c\x13\x5b\xe4\xdf\x03\x68\x7a\x5b\x14\x8c\xc8\x8c"
        "\xd2\xdf\xea\x76\xa1\x04\x0f\xfd\x37\x01\x1a\x1c\x16\xd5\x43"
        "\xa0\x24\xa7\x6a\x77\xf5\xe5\x27\xcd\x4d\x52\x9c\x48\xff\xe0"
        "\xe2\x84\x1a\x1f\xd9\x5b\x9a\x47\x27\x87\x8c\xc2\x13\x58\x31"
        "\x48\x38\x7f\xf8\x78\x1a\xa0\x70\x4b\x36\xce\x23\xff\x58\xa1"
        "\x80\xbd\x86\x38\x0a\xe3\xba\xec\xbf\x7b\x9d\xdb\xd8\x8f\x31"
        "\x4e\xf6\x92\xf6\x34\x2a\xea\x32\x49\x3d\xce\x97\x40\x7c\x24"
        "\x9e\x8d\xd3\x9f\x1b\x83\xad\xb4\xe1\xcc\x8e\x80\xf0\xd8\x42"
        "\x64\x5f\x88\x91\xe9\xdd\x98\x85\x5b\x0a\x2e\x3a\xff\x94\x5d"
        "\x17\x39\x78\x76\x26\x91\xe0\x7a\xdb\x66\x0c\x70\x7e\x5a\xa0"
        "\xe9\xe1\xd6\x38\xcc\x4d\x50\xcc\xaf\x66\xf9\xb6\x9b\xd7\x8a"
        "\xf5\x48\x1a\xac\x79\x82\x75\xcd\x3c\x89\xaa\x49\x56\xd0\xf2"
        "\x01\xc6\x74\x37\x15\x45\xc2\x01\x81\x7e\x05\x4d\x5a\x4b\xaf"
        "\x54",
        4096);
    *(uint64_t*)0x2028b000 = (uint64_t)0x18;
    *(uint32_t*)0x2028b008 = (uint32_t)0x117;
    *(uint32_t*)0x2028b00c = (uint32_t)0x3;
    *(uint32_t*)0x2028b010 = (uint32_t)0x1;
    *(uint64_t*)0x2028b018 = (uint64_t)0x18;
    *(uint32_t*)0x2028b020 = (uint32_t)0x117;
    *(uint32_t*)0x2028b024 = (uint32_t)0x4;
    *(uint32_t*)0x2028b028 = (uint32_t)0x10000;
    *(uint64_t*)0x2028b030 = (uint64_t)0x18;
    *(uint32_t*)0x2028b038 = (uint32_t)0x117;
    *(uint32_t*)0x2028b03c = (uint32_t)0x3;
    *(uint32_t*)0x2028b040 = (uint32_t)0x1;
    *(uint64_t*)0x2028b048 = (uint64_t)0x18;
    *(uint32_t*)0x2028b050 = (uint32_t)0x117;
    *(uint32_t*)0x2028b054 = (uint32_t)0x4;
    *(uint32_t*)0x2028b058 = (uint32_t)0x72ff;
    *(uint64_t*)0x2028b060 = (uint64_t)0x18;
    *(uint32_t*)0x2028b068 = (uint32_t)0x117;
    *(uint32_t*)0x2028b06c = (uint32_t)0x4;
    *(uint32_t*)0x2028b070 = (uint32_t)0x1;
    *(uint64_t*)0x2028b078 = (uint64_t)0x18;
    *(uint32_t*)0x2028b080 = (uint32_t)0x117;
    *(uint32_t*)0x2028b084 = (uint32_t)0x4;
    *(uint32_t*)0x2028b088 = (uint32_t)0xfa4;
    *(uint64_t*)0x2028b090 = (uint64_t)0x18;
    *(uint32_t*)0x2028b098 = (uint32_t)0x117;
    *(uint32_t*)0x2028b09c = (uint32_t)0x3;
    *(uint32_t*)0x2028b0a0 = (uint32_t)0x1;
    r[49] = syscall(__NR_sendmsg, r[8], 0x20054fc8ul, 0x0ul);
    break;
  case 6:
    *(uint64_t*)0x2027e000 = (uint64_t)0x20284000;
    *(uint32_t*)0x2027e008 = (uint32_t)0x10;
    *(uint64_t*)0x2027e010 = (uint64_t)0x2008bf80;
    *(uint64_t*)0x2027e018 = (uint64_t)0x2;
    *(uint64_t*)0x2027e020 = (uint64_t)0x20284000;
    *(uint64_t*)0x2027e028 = (uint64_t)0x0;
    *(uint32_t*)0x2027e030 = (uint32_t)0x0;
    *(uint64_t*)0x2008bf80 = (uint64_t)0x20043f18;
    *(uint64_t*)0x2008bf88 = (uint64_t)0x0;
    *(uint64_t*)0x2008bf90 = (uint64_t)0x20284f0c;
    *(uint64_t*)0x2008bf98 = (uint64_t)0x4f;
    r[61] = syscall(__NR_recvmsg, r[8], 0x2027e000ul, 0x0ul);
    break;
  }
  return 0;
}

void test()
{
  long i;
  pthread_t th[14];

  memset(r, -1, sizeof(r));
  srand(getpid());
  for (i = 0; i < 7; i++) {
    pthread_create(&th[i], 0, thr, (void*)i);
    usleep(rand() % 10000);
  }
  for (i = 0; i < 7; i++) {
    pthread_create(&th[7 + i], 0, thr, (void*)i);
    if (rand() % 2)
      usleep(rand() % 10000);
  }
  usleep(rand() % 100000);
}

int main()
{
  int i;
  for (i = 0; i < 8; i++) {
    if (fork() == 0) {
      setup_tun(i, true);
      loop();
      return 0;
    }
  }
  sleep(1000000);
  return 0;
}
