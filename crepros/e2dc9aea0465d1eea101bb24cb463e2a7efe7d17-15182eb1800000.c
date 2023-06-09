// https://syzkaller.appspot.com/bug?id=e2dc9aea0465d1eea101bb24cb463e2a7efe7d17
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
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
#include <sched.h>
#include <signal.h>
#include <signal.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/wait.h>
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

static void execute_command(const char* format, ...)
{
  va_list args;
  char command[PATH_PREFIX_LEN + COMMAND_MAX_LEN];
  int rv;

  va_start(args, format);
  memcpy(command, PATH_PREFIX, PATH_PREFIX_LEN);
  vsnprintf_check(command + PATH_PREFIX_LEN, COMMAND_MAX_LEN, format, args);
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

  execute_command("sysctl -w net.ipv6.conf.%s.accept_dad=0", iface);

  execute_command("sysctl -w net.ipv6.conf.%s.router_solicitations=0", iface);

  execute_command("ip link set dev %s address %s", iface, local_mac);
  execute_command("ip addr add %s/24 dev %s", local_ipv4, iface);
  execute_command("ip -6 addr add %s/120 dev %s", local_ipv6, iface);
  execute_command("ip neigh add %s lladdr %s dev %s nud permanent", remote_ipv4,
                  remote_mac, iface);
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

  unshare(CLONE_NEWNS);
  unshare(CLONE_NEWIPC);
  unshare(CLONE_NEWCGROUP);
  unshare(CLONE_NEWNET);
  unshare(CLONE_NEWUTS);
  unshare(CLONE_SYSVSEM);
}

static int do_sandbox_none(int executor_pid, bool enable_tun)
{
  unshare(CLONE_NEWPID);
  int pid = fork();
  if (pid < 0)
    fail("sandbox fork failed");
  if (pid)
    return pid;

  sandbox_common();
  setup_tun(executor_pid, enable_tun);

  loop();
  doexit(1);
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
    for (thread = 0; thread < sizeof(threads) / sizeof(threads[0]); thread++) {
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
    syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
    break;
  case 1:
    r[0] = syscall(__NR_socket, 0x26, 5, 0);
    break;
  case 2:
    *(uint16_t*)0x2016b000 = 0x26;
    memcpy((void*)0x2016b002,
           "\x73\x6b\x63\x69\x70\x68\x65\x72\x00\x00\x00\x00\x00\x00", 14);
    *(uint32_t*)0x2016b010 = 0;
    *(uint32_t*)0x2016b014 = 0;
    memcpy((void*)0x2016b018,
           "\x65\x63\x62\x28\x74\x77\x6f\x66\x69\x73\x68\x29\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
           64);
    syscall(__NR_bind, r[0], 0x2016b000, 0x58);
    break;
  case 3:
    memcpy((void*)0x203c1000, "\x0a\x07\x75\xb0\xd5\xe3\x83\xe5\xb3\xb6\x0c\xed"
                              "\x5c\x54\xdb\xaa\x29\x5d\xf0\xdf\x82\x17\xad\x40"
                              "\x00\x00\x00\x00\x00\x00\x00\xe6",
           32);
    syscall(__NR_setsockopt, r[0], 0x117, 1, 0x203c1000, 0x20);
    break;
  case 4:
    r[1] = syscall(__NR_accept, r[0], 0, 0);
    break;
  case 5:
    *(uint64_t*)0x20dde000 = 0x20ea9ff0;
    *(uint32_t*)0x20dde008 = 0x10;
    *(uint64_t*)0x20dde010 = 0x20882fb0;
    *(uint64_t*)0x20dde018 = 2;
    *(uint64_t*)0x20dde020 = 0x20f9c000;
    *(uint64_t*)0x20dde028 = 0x20;
    *(uint32_t*)0x20dde030 = 0x80;
    *(uint16_t*)0x20ea9ff0 = 3;
    memcpy((void*)0x20ea9ff2, "\xb1\x53\xe3\x5b\x12\x6c\xee", 7);
    *(uint32_t*)0x20ea9ffc = 0;
    *(uint64_t*)0x20882fb0 = 0x20fa1000;
    *(uint64_t*)0x20882fb8 = 0;
    *(uint64_t*)0x20882fc0 = 0x20fa3f04;
    *(uint64_t*)0x20882fc8 = 0xfc;
    memcpy((void*)0x20fa3f04,
           "\x7d\x57\x13\x1d\xc6\x2f\xa4\xe5\xec\xea\x21\xeb\xec\x56\x77\x14"
           "\x08\x69\xad\x4f\x0e\x70\x6f\x19\xed\x79\x90\x57\x97\x8b\x0d\x5c"
           "\xf1\x54\x7d\xb7\x9c\x81\x4c\xbd\x67\xca\x13\xce\x0f\x4c\xc5\xdc"
           "\xbf\x99\xaa\xb6\x67\xb1\x47\xab\x97\x5f\xad\x90\xcf\x88\xaa\x7f"
           "\xea\xba\x86\xc5\x68\x19\xae\x4a\x6e\xf7\xaa\x50\xef\x79\x21\xb7"
           "\x57\xb8\xc8\xde\x8c\xc6\x68\xb9\x14\x46\x46\xfb\x2a\x79\x76\x46"
           "\xde\x77\x43\x66\x61\x20\x3a\x6c\x92\x81\xbf\x1a\xc5\x71\x0d\x5a"
           "\x94\x7d\x00\x61\x6a\xb6\xdc\x06\x34\x7c\x0e\xaf\x00\xe5\x66\x89"
           "\xfa\x11\x2e\xae\xc6\x84\xd1\x86\x34\xad\x70\xf2\xb1\xee\x7c\xe6"
           "\xb3\x7f\x1c\x40\x04\x80\x67\x9d\x42\xf5\x44\x9e\x76\xc2\x44\xf3"
           "\xc0\xbf\xb8\xfd\xd5\x31\xca\x1f\x40\xd2\x9d\x38\x5a\x6a\xf4\x56"
           "\x87\xa1\x5f\xd3\xcf\xfc\xa8\xab\x5b\x29\xd5\x5c\xdc\xff\x0a\x4c"
           "\x4d\x9c\x61\x88\xab\xfa\x05\x5b\xd1\xf1\x61\xe1\x8f\x96\xc3\xef"
           "\x00\xc9\x1e\xcd\xa3\x66\xc1\xd3\x46\x74\x9f\xc7\xaa\xd1\xc2\xa9"
           "\x51\x93\xef\x28\x7f\x73\xfa\x32\x31\x74\x77\xdb\x3b\x77\x14\xe5"
           "\x19\xcf\x81\x78\x62\x06\xd2\xf9\x58\x0c\x37\xef",
           252);
    *(uint64_t*)0x20f9c000 = 0x10;
    *(uint32_t*)0x20f9c008 = 0x108;
    *(uint32_t*)0x20f9c00c = 2;
    *(uint64_t*)0x20f9c010 = 0x10;
    *(uint32_t*)0x20f9c018 = 0x10b;
    *(uint32_t*)0x20f9c01c = 0;
    syscall(__NR_sendmsg, r[1], 0x20dde000, 0x20040015);
    break;
  case 6:
    *(uint64_t*)0x201f5000 = 0x20f7ffa8;
    *(uint32_t*)0x201f5008 = 0x58;
    *(uint64_t*)0x201f5010 = 0x20612f80;
    *(uint64_t*)0x201f5018 = 8;
    *(uint64_t*)0x201f5020 = 0x20f7ffca;
    *(uint64_t*)0x201f5028 = 0x57;
    *(uint32_t*)0x201f5030 = 0xfe;
    *(uint64_t*)0x20612f80 = 0x20f7f000;
    *(uint64_t*)0x20612f88 = 0xdf;
    *(uint64_t*)0x20612f90 = 0x20589000;
    *(uint64_t*)0x20612f98 = 0x1000;
    *(uint64_t*)0x20612fa0 = 0x20f82000;
    *(uint64_t*)0x20612fa8 = 0x30;
    *(uint64_t*)0x20612fb0 = 0x205f5f9a;
    *(uint64_t*)0x20612fb8 = 0x66;
    *(uint64_t*)0x20612fc0 = 0x2012ff39;
    *(uint64_t*)0x20612fc8 = 0xc7;
    *(uint64_t*)0x20612fd0 = 0x2075ef54;
    *(uint64_t*)0x20612fd8 = 0xac;
    *(uint64_t*)0x20612fe0 = 0x20f80f9a;
    *(uint64_t*)0x20612fe8 = 0x66;
    *(uint64_t*)0x20612ff0 = 0x20f81000;
    *(uint64_t*)0x20612ff8 = 0x1000;
    syscall(__NR_recvmsg, r[1], 0x201f5000, 0x40);
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
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      for (;;) {
        int pid = do_sandbox_none(procid, true);
        int status = 0;
        while (waitpid(pid, &status, __WALL) != pid) {
        }
      }
    }
  }
  sleep(1000000);
  return 0;
}
