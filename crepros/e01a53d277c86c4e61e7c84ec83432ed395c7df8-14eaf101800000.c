// https://syzkaller.appspot.com/bug?id=e2dc9aea0465d1eea101bb24cb463e2a7efe7d17
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/futex.h>
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

struct thread_t {
  int created, running, call;
  pthread_t th;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;
static int collide;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    while (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE))
      syscall(SYS_futex, &th->running, FUTEX_WAIT, 0, 0);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    __atomic_store_n(&th->running, 0, __ATOMIC_RELEASE);
    syscall(SYS_futex, &th->running, FUTEX_WAKE);
  }
  return 0;
}

static void execute(int num_calls)
{
  int call, thread;
  running = 0;
  for (call = 0; call < num_calls; call++) {
    for (thread = 0; thread < sizeof(threads) / sizeof(threads[0]);
         thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 128 << 10);
        pthread_create(&th->th, &attr, thr, th);
      }
      if (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE)) {
        th->call = call;
        __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
        __atomic_store_n(&th->running, 1, __ATOMIC_RELEASE);
        syscall(SYS_futex, &th->running, FUTEX_WAKE);
        if (collide && call % 2)
          break;
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 20 * 1000 * 1000;
        syscall(SYS_futex, &th->running, FUTEX_WAIT, 1, &ts);
        if (running)
          usleep((call == num_calls - 1) ? 10000 : 1000);
        break;
      }
    }
  }
}

long r[2];
uint64_t procid;
void execute_call(int call)
{
  switch (call) {
  case 0:
    syscall(__NR_mmap, 0x20000000, 0xfff000, 0x3, 0x32, 0xffffffff,
            0x0);
    break;
  case 1:
    r[0] = syscall(__NR_socket, 0x26, 0x5, 0x0);
    break;
  case 2:
    *(uint16_t*)0x2038c000 = 0x26;
    memcpy((void*)0x2038c002,
           "\x73\x6b\x63\x69\x70\x68\x65\x72\x00\x00\x00\x00\x00\x00",
           14);
    *(uint32_t*)0x2038c010 = 0x0;
    *(uint32_t*)0x2038c014 = 0x0;
    memcpy((void*)0x2038c018,
           "\x65\x63\x62\x2d\x74\x77\x6f\x66\x69\x73\x68\x2d\x61\x76"
           "\x78\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00",
           64);
    syscall(__NR_bind, r[0], 0x2038c000, 0x58);
    break;
  case 3:
    memcpy((void*)0x201ec000, "\x0a\x07\x75\xb0\xd5\xe3\x83\xe5\xb3\xb6"
                              "\x0c\xed\x5c\x54\xdb\xb7",
           16);
    syscall(__NR_setsockopt, r[0], 0x117, 0x1, 0x201ec000, 0x10);
    break;
  case 4:
    r[1] = syscall(__NR_accept, r[0], 0x0, 0x0);
    break;
  case 5:
    *(uint64_t*)0x20f74fc8 = 0x0;
    *(uint32_t*)0x20f74fd0 = 0x0;
    *(uint64_t*)0x20f74fd8 = 0x20381ff0;
    *(uint64_t*)0x20f74fe0 = 0x1;
    *(uint64_t*)0x20f74fe8 = 0x2090efb8;
    *(uint64_t*)0x20f74ff0 = 0x18;
    *(uint32_t*)0x20f74ff8 = 0x0;
    *(uint64_t*)0x20381ff0 = 0x2091c000;
    *(uint64_t*)0x20381ff8 = 0x280;
    memcpy(
        (void*)0x2091c000,
        "\x5d\xfb\xc3\x3d\xc1\x9c\xb8\x70\x84\x3d\xf3\x02\x73\xb3\x81"
        "\xfa\xa8\xd6\x2a\x74\xea\xc9\x3d\x92\x5f\x73\x14\x76\x83\xc8"
        "\x0e\x60\x33\x71\x91\xa5\x8d\xf0\xc2\xc6\xd5\xb8\x70\xca\x6e"
        "\x04\xca\xf4\x1e\xab\x5e\x60\x6a\x54\x00\xab\x3f\x2b\x56\xcf"
        "\xac\x84\x40\x12\x00\x88\x5e\x8e\xd0\x98\xaa\x66\x15\xba\x7d"
        "\x2e\x3a\x54\x5e\x7c\x9e\xbb\x48\x84\x3f\x0a\x66\x11\x47\x64"
        "\x42\x88\xac\x33\xc0\x34\xe2\x1d\x98\x66\xce\xa3\x16\x44\x1b"
        "\x3f\xca\x92\x94\x25\xac\x31\xac\x96\xfa\xe3\x8c\x27\x9f\xaa"
        "\x18\xcb\xd3\xfc\x29\x2a\xab\x32\xde\x88\x5d\x74\x9a\xf3\xdb"
        "\x38\x9d\x1e\x12\x8e\x78\x08\x0b\x2c\x00\x2a\x5f\xa9\xc8\xab"
        "\x59\xac\x3a\xaa\x03\x44\x84\x9a\x06\x16\xe8\x11\xf0\x92\xef"
        "\x32\xd1\x56\xb4\xdb\x5b\x36\xb9\x68\x94\x37\xda\x01\xe8\xf9"
        "\x6d\x5a\x7b\x94\x33\x60\x2b\xc3\x59\x98\x91\x12\x9d\x5a\xa3"
        "\xb8\x84\xcd\x91\x5f\x01\x2b\xf4\x05\x46\x3e\xef\x85\x8d\x87"
        "\x49\x7d\xf9\x69\x35\xbd\x9f\x53\xc7\x34\xc1\x8b\x29\x8a\xd7"
        "\xa9\xb3\x78\xe8\xc5\x4c\x28\xa8\x9e\xa5\x7a\x59\x45\xaf\xd1"
        "\xb2\x3f\x6a\x45\x99\x37\x26\x9a\x10\x6e\x46\x7a\x29\x76\x6c"
        "\xbe\x0c\xc9\x6f\xd3\x06\xd6\x87\xdb\x2f\x13\x30\x84\x74\x5e"
        "\xa5\x73\xf3\x26\x39\x19\x88\x12\x34\xd9\x3a\xc5\x4d\xe6\x5e"
        "\xc1\x91\x13\x12\xfa\x2b\x2a\xfc\x9b\x16\xc6\x5d\x8a\xb8\x67"
        "\x82\xef\xf4\xfc\xfb\x00\xa1\x23\x63\x86\x28\x3f\x41\xe0\x95"
        "\x47\x40\xab\x75\x14\xc8\x50\x12\x6c\x40\x7f\xe2\x87\xec\x7a"
        "\x91\x76\xf3\xb2\xcc\xdd\x5b\x97\x01\x48\x84\x61\x88\xff\x57"
        "\xce\x85\xc4\x51\x63\xaf\x7d\xab\xcf\x89\x7f\x35\x8a\x20\xe7"
        "\x19\xc6\x9c\xd0\xc8\x93\xc3\x61\xdd\x8f\x5f\x5d\x86\x5a\xa8"
        "\x32\x4b\xe5\x76\xac\xb6\x60\x24\xb4\x60\x75\xaf\x67\x42\xd5"
        "\xf4\x4b\xf3\x4c\x86\x2f\x4f\xee\x23\x2c\x5d\x64\xc5\xbe\x17"
        "\x6d\x4f\x7d\x42\x2e\x69\xb7\xe5\xca\x45\xbe\x4c\xca\x3e\x6a"
        "\xe0\x4d\x90\x2f\x4d\x4f\xbd\x3c\x61\x37\x8d\x03\xd7\x84\x8e"
        "\x1e\xd6\x65\xf7\xe5\xa7\xcf\xe3\x9c\x7b\xb1\xd8\x50\xcf\x18"
        "\x88\x92\x30\x5f\xdc\x51\xf3\x2b\xc7\x22\xd2\x2b\x01\xf9\x77"
        "\x89\xd1\x9f\xcb\x6d\x7b\xf0\x43\xfb\x79\xfe\xde\x42\x7f\x43"
        "\x39\xc5\xe7\x0f\xb5\x3d\xf8\x15\xed\x0e\xd8\x6c\x29\x5b\xa5"
        "\xc8\x12\x7b\xe8\x3c\x02\x5f\xc0\x5e\x5f\x2e\x78\x21\x91\x60"
        "\x6c\x46\x3f\x77\xfc\x5c\xba\xb6\x3d\x19\x78\x71\x85\x58\x23"
        "\xc3\xb7\x92\x45\xa2\x93\x62\x18\x14\xc7\xc1\x13\x53\x51\x95"
        "\x75\xa8\xb9\xb6\x5a\x14\x92\x78\xa1\xa4\xa8\xf6\x99\xee\x0f"
        "\x7b\x4c\x3a\x29\x4b\xa6\x20\x2b\xaf\x49\x4b\x8a\x31\x58\x51"
        "\x3f\xab\x94\xb9\x82\xe2\x2a\x13\x85\x44\xc9\xdb\x8a\xfd\xf9"
        "\xf7\x1a\xb9\x30\x7f\x23\x71\x06\x34\xca\x6c\x7f\x7a\x02\x32"
        "\x76\xeb\x8f\x95\x30\xa8\x63\x29\xd8\x1e\xd4\xc2\xe4\x3d\xd1"
        "\x0d\x3d\x4b\xd0\xf9\x78\xd2\x5d\x58\x54\xd5\x43\x74\x48\x72"
        "\x1d\x7f\x5a\x2e\x3d\x1f\x99\xd7\xf3\xf6",
        640);
    *(uint64_t*)0x2090efb8 = 0x18;
    *(uint32_t*)0x2090efc0 = 0x117;
    *(uint32_t*)0x2090efc4 = 0x3;
    *(uint32_t*)0x2090efc8 = 0x1;
    syscall(__NR_sendmsg, r[1], 0x20f74fc8, 0x0);
    break;
  case 6:
    *(uint64_t*)0x20b2dfc8 = 0x207ebffa;
    *(uint32_t*)0x20b2dfd0 = 0x6;
    *(uint64_t*)0x20b2dfd8 = 0x20f75000;
    *(uint64_t*)0x20b2dfe0 = 0x2;
    *(uint64_t*)0x20b2dfe8 = 0x20ec7000;
    *(uint64_t*)0x20b2dff0 = 0x6b;
    *(uint32_t*)0x20b2dff8 = 0x0;
    *(uint64_t*)0x20f75000 = 0x2074b000;
    *(uint64_t*)0x20f75008 = 0x95;
    *(uint64_t*)0x20f75010 = 0x20f11000;
    *(uint64_t*)0x20f75018 = 0x1000;
    syscall(__NR_recvmsg, r[1], 0x20b2dfc8, 0x0);
    break;
  }
}

void test()
{
  memset(r, -1, sizeof(r));
  execute(7);
  collide = 1;
  execute(7);
}

int main()
{
  int i;
  for (i = 0; i < 8; i++) {
    if (fork() == 0) {
      procid = i;
      setup_tun(i, true);
      loop();
      return 0;
    }
  }
  sleep(1000000);
  return 0;
}
