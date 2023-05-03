// https://syzkaller.appspot.com/bug?id=d369eafab7836fe0dd2e027d2689da06cad8e1a3
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <errno.h>
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
#include <sched.h>
#include <setjmp.h>
#include <signal.h>
#include <signal.h>
#include <signal.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

const int kFailStatus = 67;
const int kRetryStatus = 69;

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}

__attribute__((noreturn)) static void fail(const char* msg, ...)
{
  int e = errno;
  fflush(stdout);
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit((e == ENOMEM || e == EAGAIN) ? kRetryStatus : kFailStatus);
}

__attribute__((noreturn)) static void exitf(const char* msg, ...)
{
  int e = errno;
  fflush(stdout);
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit(kRetryStatus);
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

static void execute_command(const char* format, ...)
{
  va_list args;
  char command[COMMAND_MAX_LEN];
  int rv;

  va_start(args, format);

  vsnprintf_check(command, sizeof(command), format, args);
  rv = system(command);
  if (rv != 0)
    fail("tun: command \"%s\" failed with code %d", &command[0], rv);

  va_end(args);
}

static int tunfd = -1;

#define SYZ_TUN_MAX_PACKET_SIZE 1000

#define MAX_PIDS 32
#define ADDR_MAX_LEN 32

#define LOCAL_MAC "aa:aa:aa:aa:aa:%02hx"
#define REMOTE_MAC "bb:bb:bb:bb:bb:%02hx"

#define LOCAL_IPV4 "172.20.%d.170"
#define REMOTE_IPV4 "172.20.%d.187"

#define LOCAL_IPV6 "fe80::%02hxaa"
#define REMOTE_IPV6 "fe80::%02hxbb"

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
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
  if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0)
    fail("tun: ioctl(TUNSETIFF) failed");

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
    fail("tun: read failed with %d, errno: %d", rv, errno);
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

  unshare(CLONE_NEWNS);
  unshare(CLONE_NEWIPC);
  unshare(CLONE_IO);
}

static int do_sandbox_none(int executor_pid, bool enable_tun)
{
  int pid = fork();
  if (pid)
    return pid;

  sandbox_common();
  setup_tun(executor_pid, enable_tun);

  loop();
  doexit(1);
}

static uint64_t current_time_ms()
{
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    fail("clock_gettime failed");
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
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

long r[93];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0xe98000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    NONFAILING(memcpy((void*)0x203ebff7,
                      "\x2f\x64\x65\x76\x2f\x6b\x76\x6d\x00", 9));
    r[2] = syscall(__NR_openat, 0xffffffffffffff9cul, 0x203ebff7ul,
                   0x10000143000ul, 0x0ul);
    break;
  case 2:
    r[3] = syscall(__NR_ioctl, r[2], 0xae01ul, 0x0ul);
    break;
  case 3:
    NONFAILING(*(uint32_t*)0x20e96ff4 = (uint32_t)0x1);
    NONFAILING(*(uint32_t*)0x20e96ff8 = r[3]);
    NONFAILING(*(uint32_t*)0x20e96ffc = (uint32_t)0x1);
    r[7] = syscall(__NR_ioctl, r[3], 0xc00caee0ul, 0x20e96ff4ul);
    if (r[7] != -1)
      NONFAILING(r[8] = *(uint32_t*)0x20e96ff8);
    break;
  case 4:
    NONFAILING(*(uint64_t*)0x20768e60 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768e68 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768e70 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768e78 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768e80 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768e88 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768e90 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768e98 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768ea0 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768ea8 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768eb0 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768eb8 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768ec0 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768ec8 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768ed0 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768ed8 = (uint64_t)0x0);
    NONFAILING(*(uint16_t*)0x20768ee0 = (uint16_t)0x3);
    NONFAILING(*(uint8_t*)0x20768ee2 = (uint8_t)0x7);
    NONFAILING(*(uint8_t*)0x20768ee3 = (uint8_t)0xfff);
    NONFAILING(*(uint8_t*)0x20768ee4 = (uint8_t)0x0);
    NONFAILING(*(uint16_t*)0x20768ee6 = (uint16_t)0x8);
    NONFAILING(*(uint64_t*)0x20768ee8 = (uint64_t)0x3000);
    NONFAILING(*(uint64_t*)0x20768ef0 = (uint64_t)0xd000);
    NONFAILING(*(uint64_t*)0x20768ef8 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768f00 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768f08 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768f10 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768f18 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768f20 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768f28 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768f30 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768f38 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768f40 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768f48 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768f50 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768f58 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768f60 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768f68 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768f70 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768f78 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768f80 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768f88 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768f90 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768f98 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768fa0 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768fa8 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768fb0 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768fb8 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768fc0 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768fc8 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768fd0 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768fd8 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768fe0 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768fe8 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20768ff0 = (uint64_t)0x0);
    NONFAILING(*(uint32_t*)0x20768ff8 = (uint32_t)0x8);
    NONFAILING(*(uint32_t*)0x20768ffc = (uint32_t)0x0);
    r[66] = syscall(__NR_ioctl, r[8], 0x41a0ae8dul, 0x20768e60ul);
    break;
  case 5:
    r[67] = syscall(__NR_eventfd2, 0xedful, 0x800ul);
    break;
  case 6:
    NONFAILING(*(uint64_t*)0x2013b000 = (uint64_t)0xfffffffffffff5e7);
    r[69] = syscall(__NR_write, r[67], 0x2013b000ul, 0x8ul);
    break;
  case 7:
    r[70] = syscall(__NR_eventfd2, 0x0ul, 0x0ul);
    break;
  case 8:
    r[71] = syscall(__NR_eventfd2, 0xffffffffffffff01ul, 0x800000002ul);
    break;
  case 9:
    NONFAILING(*(uint32_t*)0x20027fe0 = (uint32_t)0xffffffffffffffff);
    NONFAILING(*(uint32_t*)0x20027fe4 = (uint32_t)0x7f080000000000);
    NONFAILING(*(uint32_t*)0x20027fe8 = (uint32_t)0x1);
    NONFAILING(*(uint32_t*)0x20027fec = (uint32_t)0xffffffffffffffff);
    NONFAILING(*(uint8_t*)0x20027ff0 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20027ff1 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20027ff2 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20027ff3 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20027ff4 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20027ff5 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20027ff6 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20027ff7 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20027ff8 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20027ff9 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20027ffa = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20027ffb = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20027ffc = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20027ffd = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20027ffe = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20027fff = (uint8_t)0x0);
    r[92] = syscall(__NR_ioctl, 0xfffffffffffffffful, 0x4020ae76ul,
                    0x20027fe0ul);
    break;
  }
  return 0;
}

void test()
{
  long i;
  pthread_t th[20];

  memset(r, -1, sizeof(r));
  srand(getpid());
  for (i = 0; i < 10; i++) {
    pthread_create(&th[i], 0, thr, (void*)i);
    usleep(rand() % 10000);
  }
  for (i = 0; i < 10; i++) {
    pthread_create(&th[10 + i], 0, thr, (void*)i);
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
      install_segv_handler();
      int pid = do_sandbox_none(i, true);
      int status = 0;
      while (waitpid(pid, &status, __WALL) != pid) {
      }
      return 0;
    }
  }
  sleep(1000000);
  return 0;
}
