// https://syzkaller.appspot.com/bug?id=f1834e1735946170a8a3a4c85edb978e94bada81
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <errno.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/if_arp.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <time.h>
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

static uint64_t current_time_ms()
{
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    fail("clock_gettime failed");
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
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
    fail("tun: read failed with %d", rv);
  }
  return rv;
}

static void flush_tun()
{
  char data[SYZ_TUN_MAX_PACKET_SIZE];
  while (read_tun(&data[0], sizeof(data)) != -1)
    ;
}

static void test();

void loop()
{
  int iter;
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      fail("loop fork failed");
    if (pid == 0) {
      prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
      setpgrp();
      flush_tun();
      test();
      doexit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      int res = waitpid(-1, &status, __WALL | WNOHANG);
      if (res == pid)
        break;
      usleep(1000);
      if (current_time_ms() - start > 5 * 1000) {
        kill(-pid, SIGKILL);
        kill(pid, SIGKILL);
        while (waitpid(-1, &status, __WALL) != pid) {
        }
        break;
      }
    }
  }
}

long r[27];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0xf42000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    r[1] = syscall(__NR_socket, 0x26ul, 0x5ul, 0x0ul);
    break;
  case 2:
    *(uint16_t*)0x20f37000 = (uint16_t)0x26;
    memcpy((void*)0x20f37002,
           "\x61\x65\x61\x64\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
           14);
    *(uint32_t*)0x20f37010 = (uint32_t)0x0;
    *(uint32_t*)0x20f37014 = (uint32_t)0x0;
    memcpy((void*)0x20f37018,
           "\x67\x63\x6d\x28\x61\x65\x73\x29\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00",
           64);
    r[7] = syscall(__NR_bind, r[1], 0x20f37000ul, 0x58ul);
    break;
  case 3:
    memcpy((void*)0x204f7000, "\x64\x9c\x47\xad\x46\x39\x0d\xc8\x6d\xae"
                              "\x79\xfa\x40\x9d\x4d\x54",
           16);
    r[9] = syscall(__NR_setsockopt, r[1], 0x117ul, 0x1ul, 0x204f7000ul,
                   0x10ul);
    break;
  case 4:
    r[10] = syscall(__NR_accept, r[1], 0x0ul, 0x0ul);
    break;
  case 5:
    r[11] = syscall(__NR_accept, r[1], 0x0ul, 0x0ul);
    break;
  case 6:
    memcpy((void*)0x20519f73,
           "\x35\x95\xc9\x17\xd6\x71\xb6\xb5\x38\x5f\x00\x42\xbd\x83"
           "\xa6\xd2\x6b\x71\xaa\x9a\x6d\xea\xf4\xce\xde\xed\x63\xf6"
           "\xad\x0a\x14\xcf\xab\x6e\x65\xf4\xbc\xd7\x40\x82\x9a\x3a"
           "\x6e\xfa\xc8\xec\x4c\x77\xb3\x74\x9b\x24\x4a\xbc\xf0\xb5"
           "\x99\x8c\x60\xf6\xd0\x43\x0a\xce\x54\xba\xa4\x15\x6e\xbb"
           "\xfa\xa2\xff\xdf\xf4\xbb\xa9\x89\xf4\xae\xb3\xcd\x1f\x87"
           "\x87\x87\xd9\xfc\x26\xac\x60\x20\x14\x40\x00\x00\x00\x00"
           "\x00\x59\xfc\x8d\xd6\x23\xe5\xcb\xbe\xc8\x5d\x30\xdb\x1a"
           "\xb1\x64\x17\x4f\xe4\x0b\x99\x98\x89\xf0\x29\x88\x36\x9b"
           "\xe7\x57\x01\x71\x78\xac\x1e\x74\x7f\x49\x2f\xc2\x65\x65"
           "\x4d",
           141);
    r[13] = syscall(__NR_write, r[10], 0x20519f73ul, 0x8dul);
    break;
  case 7:
    *(uint64_t*)0x206b0fc8 = (uint64_t)0x20f38ff7;
    *(uint32_t*)0x206b0fd0 = (uint32_t)0x9;
    *(uint64_t*)0x206b0fd8 = (uint64_t)0x20f38fe0;
    *(uint64_t*)0x206b0fe0 = (uint64_t)0x2;
    *(uint64_t*)0x206b0fe8 = (uint64_t)0x204d3f31;
    *(uint64_t*)0x206b0ff0 = (uint64_t)0x0;
    *(uint32_t*)0x206b0ff8 = (uint32_t)0x3;
    *(uint64_t*)0x20f38fe0 = (uint64_t)0x20f41000;
    *(uint64_t*)0x20f38fe8 = (uint64_t)0x0;
    *(uint64_t*)0x20f38ff0 = (uint64_t)0x207c0000;
    *(uint64_t*)0x20f38ff8 = (uint64_t)0x26;
    r[25] = syscall(__NR_recvmsg, r[10], 0x206b0fc8ul, 0x40ul);
    break;
  case 8:
    r[26] = syscall(__NR_accept, r[1], 0x0ul, 0x0ul);
    break;
  }
  return 0;
}

void test()
{
  long i;
  pthread_t th[18];

  memset(r, -1, sizeof(r));
  for (i = 0; i < 9; i++) {
    pthread_create(&th[i], 0, thr, (void*)i);
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
