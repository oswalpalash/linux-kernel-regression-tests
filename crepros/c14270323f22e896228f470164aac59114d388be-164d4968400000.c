// https://syzkaller.appspot.com/bug?id=c14270323f22e896228f470164aac59114d388be
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
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
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

  execute_command(0, "sysctl -w net.ipv6.conf.%s.accept_dad=0", TUN_IFACE);

  execute_command(0, "sysctl -w net.ipv6.conf.%s.router_solicitations=0",
                  TUN_IFACE);

  execute_command(1, "ip link set dev %s address %s", TUN_IFACE, LOCAL_MAC);
  execute_command(1, "ip addr add %s/24 dev %s", LOCAL_IPV4, TUN_IFACE);
  execute_command(1, "ip neigh add %s lladdr %s dev %s nud permanent",
                  REMOTE_IPV4, REMOTE_MAC, TUN_IFACE);
  execute_command(0, "ip -6 addr add %s/120 dev %s", LOCAL_IPV6, TUN_IFACE);
  execute_command(0, "ip -6 neigh add %s lladdr %s dev %s nud permanent",
                  REMOTE_IPV6, REMOTE_MAC, TUN_IFACE);
  execute_command(1, "ip link set dev %s up", TUN_IFACE);
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();

  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = 160 << 20;
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 8 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 136 << 20;
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

int wait_for_loop(int pid)
{
  if (pid < 0)
    fail("sandbox fork failed");
  int status = 0;
  while (waitpid(-1, &status, __WALL) != pid) {
  }
  return WEXITSTATUS(status);
}

static int do_sandbox_none(void)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid != 0)
    return wait_for_loop(pid);

  sandbox_common();
  if (unshare(CLONE_NEWNET)) {
  }
  initialize_tun();
  loop();
  doexit(1);
}

static void execute_one();
extern unsigned long long procid;

void loop()
{
  while (1) {
    execute_one();
  }
}

struct thread_t {
  int created, running, call;
  pthread_t th;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

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
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 20 * 1000 * 1000;
        syscall(SYS_futex, &th->running, FUTEX_WAIT, 1, &ts);
        if (__atomic_load_n(&running, __ATOMIC_RELAXED))
          usleep((call == num_calls - 1) ? 10000 : 1000);
        break;
      }
    }
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    res = syscall(__NR_socket, 2, 6, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    *(uint16_t*)0x20000100 = 2;
    *(uint16_t*)0x20000102 = htobe16(0);
    *(uint8_t*)0x20000104 = 0xac;
    *(uint8_t*)0x20000105 = 0x14;
    *(uint8_t*)0x20000106 = 0x14;
    *(uint8_t*)0x20000107 = 0x1b;
    *(uint8_t*)0x20000108 = 0;
    *(uint8_t*)0x20000109 = 0;
    *(uint8_t*)0x2000010a = 0;
    *(uint8_t*)0x2000010b = 0;
    *(uint8_t*)0x2000010c = 0;
    *(uint8_t*)0x2000010d = 0;
    *(uint8_t*)0x2000010e = 0;
    *(uint8_t*)0x2000010f = 0;
    syscall(__NR_bind, r[0], 0x20000100, 0x10);
    break;
  case 2:
    *(uint16_t*)0x20000140 = 2;
    *(uint16_t*)0x20000142 = htobe16(0);
    *(uint8_t*)0x20000144 = 0xac;
    *(uint8_t*)0x20000145 = 0x14;
    *(uint8_t*)0x20000146 = 0x14;
    *(uint8_t*)0x20000147 = 0xaa;
    *(uint8_t*)0x20000148 = 0;
    *(uint8_t*)0x20000149 = 0;
    *(uint8_t*)0x2000014a = 0;
    *(uint8_t*)0x2000014b = 0;
    *(uint8_t*)0x2000014c = 0;
    *(uint8_t*)0x2000014d = 0;
    *(uint8_t*)0x2000014e = 0;
    *(uint8_t*)0x2000014f = 0;
    syscall(__NR_connect, r[0], 0x20000140, 0x10);
    break;
  case 3:
    memcpy((void*)0x20000f80, "./file0", 8);
    res = syscall(__NR_open, 0x20000f80, 0x40, 0);
    if (res != -1)
      r[1] = res;
    break;
  case 4:
    memcpy((void*)0x20000f00, "./file0", 8);
    memcpy((void*)0x20000f40, "9p", 3);
    memcpy((void*)0x20000fc0, "trans=fd,", 9);
    memcpy((void*)0x20000fc9, "rfdno", 5);
    *(uint8_t*)0x20000fce = 0x3d;
    sprintf((char*)0x20000fcf, "0x%016llx", (long long)r[1]);
    *(uint8_t*)0x20000fe1 = 0x2c;
    memcpy((void*)0x20000fe2, "wfdno", 5);
    *(uint8_t*)0x20000fe7 = 0x3d;
    sprintf((char*)0x20000fe8, "0x%016llx", (long long)r[0]);
    *(uint8_t*)0x20000ffa = 0x2c;
    *(uint8_t*)0x20000ffb = 0;
    syscall(__NR_mount, 0, 0x20000f00, 0x20000f40, 0, 0x20000fc0);
    break;
  case 5:
    *(uint16_t*)0x20000080 = 0;
    memcpy((void*)0x20000082,
           "\x2e\x2f\x66\x69\x6c\x65\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
           108);
    syscall(__NR_connect, r[0], 0x20000080, 0x6e);
    break;
  }
}

void execute_one()
{
  execute(6);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    do_sandbox_none();
  }
}
