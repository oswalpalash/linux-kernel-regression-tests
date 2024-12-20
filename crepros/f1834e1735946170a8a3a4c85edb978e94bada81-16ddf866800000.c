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

#include <setjmp.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>
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
  for (;;) {
  }
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

#define NONFAILING(...)                                                \
  {                                                                    \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);               \
    if (_setjmp(segv_env) == 0) {                                      \
      __VA_ARGS__;                                                     \
    }                                                                  \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);               \
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

long r[84];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    r[1] = syscall(__NR_socket, 0x26ul, 0x5ul, 0x0ul);
    break;
  case 2:
    NONFAILING(*(uint16_t*)0x20e1e000 = (uint16_t)0x26);
    NONFAILING(memcpy(
        (void*)0x20e1e002,
        "\x68\x61\x73\x68\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
        14));
    NONFAILING(*(uint32_t*)0x20e1e010 = (uint32_t)0x0);
    NONFAILING(*(uint32_t*)0x20e1e014 = (uint32_t)0x0);
    NONFAILING(memcpy((void*)0x20e1e018,
                      "\x63\x72\x63\x33\x32\x63\x00\x00\x00\x00\x00\x00"
                      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                      "\x00\x00\x00\x00",
                      64));
    r[7] = syscall(__NR_bind, r[1], 0x20e1e000ul, 0x58ul);
    break;
  case 3:
    NONFAILING(*(uint16_t*)0x20f3b000 = (uint16_t)0x26);
    NONFAILING(memcpy(
        (void*)0x20f3b002,
        "\x61\x65\x61\x64\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
        14));
    NONFAILING(*(uint32_t*)0x20f3b010 = (uint32_t)0x0);
    NONFAILING(*(uint32_t*)0x20f3b014 = (uint32_t)0x0);
    NONFAILING(memcpy((void*)0x20f3b018,
                      "\x72\x66\x63\x34\x31\x30\x36\x28\x67\x63\x6d\x28"
                      "\x61\x65\x73\x29\x29\x00\x00\x00\x00\x00\x00\x00"
                      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                      "\x00\x00\x00\x00",
                      64));
    r[13] = syscall(__NR_bind, r[1], 0x20f3b000ul, 0x58ul);
    break;
  case 4:
    r[14] =
        syscall(__NR_setsockopt, r[1], 0x117ul, 0x5ul, 0x0ul, 0x789ul);
    break;
  case 5:
    NONFAILING(*(uint16_t*)0x20333fa8 = (uint16_t)0x26);
    NONFAILING(memcpy(
        (void*)0x20333faa,
        "\x72\x6e\x67\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
        14));
    NONFAILING(*(uint32_t*)0x20333fb8 = (uint32_t)0x0);
    NONFAILING(*(uint32_t*)0x20333fbc = (uint32_t)0xfffffffffffffffc);
    NONFAILING(memcpy((void*)0x20333fc0,
                      "\x64\x72\x62\x67\x5f\x6e\x6f\x70\x72\x5f\x63\x74"
                      "\x72\x5f\x61\x65\x73\x31\x32\x38\x00\x00\x00\x00"
                      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                      "\x00\x00\x00\x00",
                      64));
    r[20] = syscall(__NR_bind, r[1], 0x20333fa8ul, 0x10000004eul);
    break;
  case 6:
    NONFAILING(memcpy((void*)0x20890fec, "\x79\x73\x39\x76\xd8\xc3\x0a"
                                         "\x4a\xea\xd9\x60\xf2\xbd\x08"
                                         "\xcf\xe6\x27\x85\xce\x10",
                      20));
    r[22] = syscall(__NR_setsockopt, r[1], 0x117ul, 0x1ul, 0x20890fecul,
                    0x14ul);
    break;
  case 7:
    NONFAILING(*(uint16_t*)0x20a65000 = (uint16_t)0x26);
    NONFAILING(memcpy(
        (void*)0x20a65002,
        "\x61\x65\x61\x64\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
        14));
    NONFAILING(*(uint32_t*)0x20a65010 = (uint32_t)0x0);
    NONFAILING(*(uint32_t*)0x20a65014 = (uint32_t)0x0);
    NONFAILING(memcpy((void*)0x20a65018,
                      "\x67\x63\x6d\x28\x73\x65\x72\x70\x65\x6e\x74\x29"
                      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                      "\x00\x00\x00\x00",
                      64));
    r[28] =
        syscall(__NR_bind, 0xfffffffffffffffful, 0x20a65000ul, 0x58ul);
    break;
  case 8:
    r[29] = syscall(__NR_accept, r[1], 0x0ul, 0x0ul);
    break;
  case 9:
    NONFAILING(*(uint64_t*)0x202fefc8 = (uint64_t)0x0);
    NONFAILING(*(uint32_t*)0x202fefd0 = (uint32_t)0x0);
    NONFAILING(*(uint64_t*)0x202fefd8 = (uint64_t)0x20985000);
    NONFAILING(*(uint64_t*)0x202fefe0 = (uint64_t)0x1);
    NONFAILING(*(uint64_t*)0x202fefe8 = (uint64_t)0x20632f70);
    NONFAILING(*(uint64_t*)0x202feff0 = (uint64_t)0x78);
    NONFAILING(*(uint32_t*)0x202feff8 = (uint32_t)0x1);
    NONFAILING(*(uint64_t*)0x20985000 = (uint64_t)0x206fdf9d);
    NONFAILING(*(uint64_t*)0x20985008 = (uint64_t)0xe);
    NONFAILING(memcpy(
        (void*)0x206fdf9d,
        "\x95\x76\xfb\x56\x78\xf4\xd8\xbb\x1b\x8c\x63\xa2\x29\xd9",
        14));
    NONFAILING(*(uint64_t*)0x20632f70 = (uint64_t)0x18);
    NONFAILING(*(uint32_t*)0x20632f78 = (uint32_t)0x117);
    NONFAILING(*(uint32_t*)0x20632f7c = (uint32_t)0x3);
    NONFAILING(*(uint32_t*)0x20632f80 = (uint32_t)0x0);
    NONFAILING(*(uint64_t*)0x20632f88 = (uint64_t)0x18);
    NONFAILING(*(uint32_t*)0x20632f90 = (uint32_t)0x117);
    NONFAILING(*(uint32_t*)0x20632f94 = (uint32_t)0x3);
    NONFAILING(*(uint32_t*)0x20632f98 = (uint32_t)0x0);
    NONFAILING(*(uint64_t*)0x20632fa0 = (uint64_t)0x18);
    NONFAILING(*(uint32_t*)0x20632fa8 = (uint32_t)0x117);
    NONFAILING(*(uint32_t*)0x20632fac = (uint32_t)0x3);
    NONFAILING(*(uint32_t*)0x20632fb0 = (uint32_t)0x1);
    NONFAILING(*(uint64_t*)0x20632fb8 = (uint64_t)0x18);
    NONFAILING(*(uint32_t*)0x20632fc0 = (uint32_t)0x117);
    NONFAILING(*(uint32_t*)0x20632fc4 = (uint32_t)0x4);
    NONFAILING(*(uint32_t*)0x20632fc8 = (uint32_t)0x100000000);
    NONFAILING(*(uint64_t*)0x20632fd0 = (uint64_t)0x18);
    NONFAILING(*(uint32_t*)0x20632fd8 = (uint32_t)0x117);
    NONFAILING(*(uint32_t*)0x20632fdc = (uint32_t)0x4);
    NONFAILING(*(uint32_t*)0x20632fe0 = (uint32_t)0x3);
    r[60] = syscall(__NR_sendmsg, r[29], 0x202fefc8ul, 0x1ul);
    break;
  case 10:
    NONFAILING(*(uint64_t*)0x2094ffc8 = (uint64_t)0x20967000);
    NONFAILING(*(uint32_t*)0x2094ffd0 = (uint32_t)0x27);
    NONFAILING(*(uint64_t*)0x2094ffd8 = (uint64_t)0x20203000);
    NONFAILING(*(uint64_t*)0x2094ffe0 = (uint64_t)0x7);
    NONFAILING(*(uint64_t*)0x2094ffe8 = (uint64_t)0x207dafc6);
    NONFAILING(*(uint64_t*)0x2094fff0 = (uint64_t)0x0);
    NONFAILING(*(uint32_t*)0x2094fff8 = (uint32_t)0x8);
    NONFAILING(*(uint64_t*)0x20203000 = (uint64_t)0x20aa0f41);
    NONFAILING(*(uint64_t*)0x20203008 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20203010 = (uint64_t)0x20732000);
    NONFAILING(*(uint64_t*)0x20203018 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20203020 = (uint64_t)0x204c2f17);
    NONFAILING(*(uint64_t*)0x20203028 = (uint64_t)0xe9);
    NONFAILING(*(uint64_t*)0x20203030 = (uint64_t)0x205af000);
    NONFAILING(*(uint64_t*)0x20203038 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20203040 = (uint64_t)0x203d8000);
    NONFAILING(*(uint64_t*)0x20203048 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20203050 = (uint64_t)0x20823f4a);
    NONFAILING(*(uint64_t*)0x20203058 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20203060 = (uint64_t)0x20fa1000);
    NONFAILING(*(uint64_t*)0x20203068 = (uint64_t)0x0);
    r[82] = syscall(__NR_recvmsg, r[29], 0x2094ffc8ul, 0x3ul);
    break;
  case 11:
    r[83] = syscall(__NR_accept, r[1], 0x0ul, 0x0ul);
    break;
  }
  return 0;
}

void test()
{
  long i;
  pthread_t th[24];

  memset(r, -1, sizeof(r));
  for (i = 0; i < 12; i++) {
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
      install_segv_handler();
      setup_tun(i, true);
      loop();
      return 0;
    }
  }
  sleep(1000000);
  return 0;
}
