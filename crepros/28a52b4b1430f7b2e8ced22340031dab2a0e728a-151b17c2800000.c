// https://syzkaller.appspot.com/bug?id=f5e3a56fe960737cfe8f3dda308e65d5b2347d35
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
  if (tunfd == -1)
    fail("tun: can't open /dev/net/tun");

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
      fail("clone failed");
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

long r[63];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    memcpy((void*)0x20000ffa, "\x6c\x6f\x67\x6f\x6e\x00", 6);
    *(uint8_t*)0x20000ffb = (uint8_t)0x73;
    *(uint8_t*)0x20000ffc = (uint8_t)0x79;
    *(uint8_t*)0x20000ffd = (uint8_t)0x7a;
    *(uint8_t*)0x20000ffe = (uint8_t)0x23;
    *(uint8_t*)0x20000fff = (uint8_t)0x0;
    memcpy((void*)0x20000ff7, "\x2f\x64\x65\x76\x2f\x6b\x76\x6d\x00",
           9);
    r[8] = syscall(__NR_request_key, 0x20000ffaul, 0x20000ffbul,
                   0x20000ff7ul, 0xfffffffffffffffful);
    break;
  case 2:
    r[9] = syscall(__NR_ioctl, 0xfffffffffffffffful, 0xae01ul, 0x0ul);
    break;
  case 3:
    memcpy((void*)0x20851ff7, "\x2f\x64\x65\x76\x2f\x6b\x76\x6d\x00",
           9);
    r[11] = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20851ff7ul,
                    0x0ul, 0x0ul);
    break;
  case 4:
    r[12] = syscall(__NR_ioctl, r[11], 0xae01ul, 0x0ul);
    break;
  case 5:
    *(uint32_t*)0x20afa000 = (uint32_t)0x0;
    *(uint32_t*)0x20afa004 = (uint32_t)0x0;
    *(uint32_t*)0x20afa008 = (uint32_t)0x0;
    *(uint32_t*)0x20afa00c = (uint32_t)0x0;
    *(uint32_t*)0x20afa010 = (uint32_t)0x0;
    *(uint32_t*)0x20afa014 = (uint32_t)0x0;
    *(uint32_t*)0x20afa018 = (uint32_t)0x0;
    *(uint32_t*)0x20afa01c = (uint32_t)0x0;
    *(uint32_t*)0x20afa020 = (uint32_t)0x0;
    *(uint32_t*)0x20afa024 = (uint32_t)0x0;
    *(uint32_t*)0x20afa028 = (uint32_t)0x0;
    *(uint32_t*)0x20afa02c = (uint32_t)0x0;
    *(uint32_t*)0x20afa030 = (uint32_t)0x0;
    *(uint32_t*)0x20afa034 = (uint32_t)0x0;
    *(uint32_t*)0x20afa038 = (uint32_t)0x0;
    *(uint32_t*)0x20afa03c = (uint32_t)0x0;
    r[29] = syscall(__NR_ioctl, r[12], 0x4040ae77ul, 0x20afa000ul);
    break;
  case 6:
    *(uint8_t*)0x2068afe0 = (uint8_t)0x2;
    *(uint8_t*)0x2068afe1 = (uint8_t)0x0;
    *(uint8_t*)0x2068afe2 = (uint8_t)0x0;
    *(uint8_t*)0x2068afe3 = (uint8_t)0x0;
    *(uint8_t*)0x2068afe4 = (uint8_t)0x0;
    *(uint8_t*)0x2068afe5 = (uint8_t)0x0;
    *(uint8_t*)0x2068afe6 = (uint8_t)0x0;
    *(uint8_t*)0x2068afe7 = (uint8_t)0x0;
    *(uint8_t*)0x2068afe8 = (uint8_t)0x0;
    *(uint8_t*)0x2068afe9 = (uint8_t)0x0;
    *(uint8_t*)0x2068afea = (uint8_t)0x0;
    *(uint8_t*)0x2068afeb = (uint8_t)0x0;
    *(uint8_t*)0x2068afec = (uint8_t)0x0;
    *(uint8_t*)0x2068afed = (uint8_t)0x0;
    *(uint8_t*)0x2068afee = (uint8_t)0x0;
    *(uint8_t*)0x2068afef = (uint8_t)0x0;
    *(uint8_t*)0x2068aff0 = (uint8_t)0x0;
    *(uint8_t*)0x2068aff1 = (uint8_t)0x0;
    *(uint8_t*)0x2068aff2 = (uint8_t)0x0;
    *(uint8_t*)0x2068aff3 = (uint8_t)0x0;
    *(uint8_t*)0x2068aff4 = (uint8_t)0x0;
    *(uint8_t*)0x2068aff5 = (uint8_t)0x0;
    *(uint8_t*)0x2068aff6 = (uint8_t)0x0;
    *(uint8_t*)0x2068aff7 = (uint8_t)0x0;
    *(uint8_t*)0x2068aff8 = (uint8_t)0x0;
    *(uint8_t*)0x2068aff9 = (uint8_t)0x0;
    *(uint8_t*)0x2068affa = (uint8_t)0x0;
    *(uint8_t*)0x2068affb = (uint8_t)0x0;
    *(uint8_t*)0x2068affc = (uint8_t)0x0;
    *(uint8_t*)0x2068affd = (uint8_t)0x0;
    *(uint8_t*)0x2068affe = (uint8_t)0x0;
    *(uint8_t*)0x2068afff = (uint8_t)0x0;
    r[62] = syscall(__NR_ioctl, r[12], 0xae71ul, 0x2068afe0ul);
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
