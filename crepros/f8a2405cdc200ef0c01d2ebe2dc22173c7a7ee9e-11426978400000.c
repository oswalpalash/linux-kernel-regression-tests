// https://syzkaller.appspot.com/bug?id=f8a2405cdc200ef0c01d2ebe2dc22173c7a7ee9e
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if_arp.h>
#include <pthread.h>
#include <sched.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/capability.h>
#include <linux/futex.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/tcp.h>

static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* ctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  if (__atomic_load_n(&skip_segv, __ATOMIC_RELAXED) &&
      (addr < prog_start || addr > prog_end)) {
    _longjmp(segv_env, 1);
  }
  exit(sig);
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

#define NONFAILING(...)                                                        \
  {                                                                            \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    if (_setjmp(segv_env) == 0) {                                              \
      __VA_ARGS__;                                                             \
    }                                                                          \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
  }

static void sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
}

uint64_t current_time_ms()
{
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    exit(1);
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void use_temporary_dir()
{
  char tmpdir_template[] = "./syzkaller.XXXXXX";
  char* tmpdir = mkdtemp(tmpdir_template);
  if (!tmpdir)
    exit(1);
  if (chmod(tmpdir, 0777))
    exit(1);
  if (chdir(tmpdir))
    exit(1);
}

static void thread_start(void* (*fn)(void*), void* arg)
{
  pthread_t th;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 128 << 10);
  if (pthread_create(&th, &attr, fn, arg))
    exit(1);
  pthread_attr_destroy(&attr);
}

typedef struct {
  int state;
} event_t;

static void event_init(event_t* ev)
{
  ev->state = 0;
}

static void event_reset(event_t* ev)
{
  ev->state = 0;
}

static void event_set(event_t* ev)
{
  if (ev->state)
    exit(1);
  __atomic_store_n(&ev->state, 1, __ATOMIC_RELEASE);
  syscall(SYS_futex, &ev->state, FUTEX_WAKE);
}

static void event_wait(event_t* ev)
{
  while (!__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
    syscall(SYS_futex, &ev->state, FUTEX_WAIT, 0, 0);
}

static int event_isset(event_t* ev)
{
  return __atomic_load_n(&ev->state, __ATOMIC_ACQUIRE);
}

static int event_timedwait(event_t* ev, uint64_t timeout)
{
  uint64_t start = current_time_ms();
  uint64_t now = start;
  for (;;) {
    uint64_t remain = timeout - (now - start);
    struct timespec ts;
    ts.tv_sec = remain / 1000;
    ts.tv_nsec = (remain % 1000) * 1000 * 1000;
    syscall(SYS_futex, &ev->state, FUTEX_WAIT, 0, &ts);
    if (__atomic_load_n(&ev->state, __ATOMIC_RELAXED))
      return 1;
    now = current_time_ms();
    if (now - start > timeout)
      return 0;
  }
}

static void vsnprintf_check(char* str, size_t size, const char* format,
                            va_list args)
{
  int rv;
  rv = vsnprintf(str, size, format, args);
  if (rv < 0)
    exit(1);
  if ((size_t)rv >= size)
    exit(1);
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
      exit(1);
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
  const int kTunFd = 240;
  if (dup2(tunfd, kTunFd) < 0)
    exit(1);
  close(tunfd);
  tunfd = kTunFd;
  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, TUN_IFACE, IFNAMSIZ);
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI | IFF_NAPI | IFF_NAPI_FRAGS;
  if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0) {
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0)
      exit(1);
  }
  if (ioctl(tunfd, TUNGETIFF, (void*)&ifr) < 0)
    exit(1);
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
    int err = errno;
    close(fd);
    errno = err;
    return false;
  }
  close(fd);
  return true;
}

static void setup_common()
{
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
  rlim.rlim_cur = rlim.rlim_max = 256;
  setrlimit(RLIMIT_NOFILE, &rlim);
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
    exit(1);
  int status = 0;
  while (waitpid(-1, &status, __WALL) != pid) {
  }
  return WEXITSTATUS(status);
}

static int real_uid;
static int real_gid;
__attribute__((aligned(64 << 10))) static char sandbox_stack[1 << 20];

static int namespace_sandbox_proc(void* arg)
{
  sandbox_common();
  write_file("/proc/self/setgroups", "deny");
  if (!write_file("/proc/self/uid_map", "0 %d 1\n", real_uid))
    exit(1);
  if (!write_file("/proc/self/gid_map", "0 %d 1\n", real_gid))
    exit(1);
  if (unshare(CLONE_NEWNET))
    exit(1);
  initialize_tun();
  if (mkdir("./syz-tmp", 0777))
    exit(1);
  if (mount("", "./syz-tmp", "tmpfs", 0, NULL))
    exit(1);
  if (mkdir("./syz-tmp/newroot", 0777))
    exit(1);
  if (mkdir("./syz-tmp/newroot/dev", 0700))
    exit(1);
  unsigned bind_mount_flags = MS_BIND | MS_REC | MS_PRIVATE;
  if (mount("/dev", "./syz-tmp/newroot/dev", NULL, bind_mount_flags, NULL))
    exit(1);
  if (mkdir("./syz-tmp/newroot/proc", 0700))
    exit(1);
  if (mount(NULL, "./syz-tmp/newroot/proc", "proc", 0, NULL))
    exit(1);
  if (mkdir("./syz-tmp/newroot/selinux", 0700))
    exit(1);
  const char* selinux_path = "./syz-tmp/newroot/selinux";
  if (mount("/selinux", selinux_path, NULL, bind_mount_flags, NULL)) {
    if (errno != ENOENT)
      exit(1);
    if (mount("/sys/fs/selinux", selinux_path, NULL, bind_mount_flags, NULL) &&
        errno != ENOENT)
      exit(1);
  }
  if (mkdir("./syz-tmp/newroot/sys", 0700))
    exit(1);
  if (mount("/sys", "./syz-tmp/newroot/sys", 0, bind_mount_flags, NULL))
    exit(1);
  if (mkdir("./syz-tmp/pivot", 0777))
    exit(1);
  if (syscall(SYS_pivot_root, "./syz-tmp", "./syz-tmp/pivot")) {
    if (chdir("./syz-tmp"))
      exit(1);
  } else {
    if (chdir("/"))
      exit(1);
    if (umount2("./pivot", MNT_DETACH))
      exit(1);
  }
  if (chroot("./newroot"))
    exit(1);
  if (chdir("/"))
    exit(1);
  struct __user_cap_header_struct cap_hdr = {};
  struct __user_cap_data_struct cap_data[2] = {};
  cap_hdr.version = _LINUX_CAPABILITY_VERSION_3;
  cap_hdr.pid = getpid();
  if (syscall(SYS_capget, &cap_hdr, &cap_data))
    exit(1);
  cap_data[0].effective &= ~(1 << CAP_SYS_PTRACE);
  cap_data[0].permitted &= ~(1 << CAP_SYS_PTRACE);
  cap_data[0].inheritable &= ~(1 << CAP_SYS_PTRACE);
  if (syscall(SYS_capset, &cap_hdr, &cap_data))
    exit(1);
  loop();
  exit(1);
}

static int do_sandbox_namespace(void)
{
  int pid;
  setup_common();
  real_uid = getuid();
  real_gid = getgid();
  mprotect(sandbox_stack, 4096, PROT_NONE);
  pid =
      clone(namespace_sandbox_proc, &sandbox_stack[sizeof(sandbox_stack) - 64],
            CLONE_NEWUSER | CLONE_NEWPID, 0);
  return wait_for_loop(pid);
}

struct thread_t {
  int created, call;
  event_t ready, done;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    event_wait(&th->ready);
    event_reset(&th->ready);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    event_set(&th->done);
  }
  return 0;
}

static void loop()
{
  int call, thread;
  for (call = 0; call < 6; call++) {
    for (thread = 0; thread < sizeof(threads) / sizeof(threads[0]); thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        event_init(&th->ready);
        event_init(&th->done);
        event_set(&th->done);
        thread_start(thr, th);
      }
      if (!event_isset(&th->done))
        continue;
      event_reset(&th->done);
      th->call = call;
      __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
      event_set(&th->ready);
      event_timedwait(&th->done, 25);
      if (__atomic_load_n(&running, __ATOMIC_RELAXED))
        sleep_ms((call == 6 - 1) ? 10 : 2);
      break;
    }
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    res = syscall(__NR_socket, 0xa, 2, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    NONFAILING(*(uint16_t*)0x20000080 = 0xa);
    NONFAILING(*(uint16_t*)0x20000082 = htobe16(0));
    NONFAILING(*(uint32_t*)0x20000084 = 0);
    NONFAILING(*(uint8_t*)0x20000088 = 0xfe);
    NONFAILING(*(uint8_t*)0x20000089 = 0x80);
    NONFAILING(*(uint8_t*)0x2000008a = 0);
    NONFAILING(*(uint8_t*)0x2000008b = 0);
    NONFAILING(*(uint8_t*)0x2000008c = 0);
    NONFAILING(*(uint8_t*)0x2000008d = 0);
    NONFAILING(*(uint8_t*)0x2000008e = 0);
    NONFAILING(*(uint8_t*)0x2000008f = 0);
    NONFAILING(*(uint8_t*)0x20000090 = 0);
    NONFAILING(*(uint8_t*)0x20000091 = 0);
    NONFAILING(*(uint8_t*)0x20000092 = 0);
    NONFAILING(*(uint8_t*)0x20000093 = 0);
    NONFAILING(*(uint8_t*)0x20000094 = 0);
    NONFAILING(*(uint8_t*)0x20000095 = 0);
    NONFAILING(*(uint8_t*)0x20000096 = 0);
    NONFAILING(*(uint8_t*)0x20000097 = 0xbb);
    NONFAILING(*(uint32_t*)0x20000098 = 0xb);
    syscall(__NR_connect, r[0], 0x20000080, 0x1c);
    break;
  case 2:
    res = syscall(__NR_socket, 0x18, 1, 1);
    if (res != -1)
      r[1] = res;
    break;
  case 3:
    NONFAILING(*(uint16_t*)0x205fafd2 = 0x18);
    NONFAILING(*(uint32_t*)0x205fafd4 = 1);
    NONFAILING(*(uint32_t*)0x205fafd8 = 0);
    NONFAILING(*(uint32_t*)0x205fafdc = r[0]);
    NONFAILING(*(uint16_t*)0x205fafe0 = 2);
    NONFAILING(*(uint16_t*)0x205fafe2 = htobe16(0));
    NONFAILING(*(uint32_t*)0x205fafe4 = htobe32(0xe0000002));
    NONFAILING(*(uint8_t*)0x205fafe8 = 0);
    NONFAILING(*(uint8_t*)0x205fafe9 = 0);
    NONFAILING(*(uint8_t*)0x205fafea = 0);
    NONFAILING(*(uint8_t*)0x205fafeb = 0);
    NONFAILING(*(uint8_t*)0x205fafec = 0);
    NONFAILING(*(uint8_t*)0x205fafed = 0);
    NONFAILING(*(uint8_t*)0x205fafee = 0);
    NONFAILING(*(uint8_t*)0x205fafef = 0);
    NONFAILING(*(uint32_t*)0x205faff0 = 4);
    NONFAILING(*(uint32_t*)0x205faff4 = 0);
    NONFAILING(*(uint32_t*)0x205faff8 = 0);
    NONFAILING(*(uint32_t*)0x205faffc = 0);
    syscall(__NR_connect, r[1], 0x205fafd2, 0x2e);
    break;
  case 4:
    NONFAILING(*(uint64_t*)0x20005fc0 = 0x20005680);
    NONFAILING(*(uint16_t*)0x20005680 = 0x1f);
    NONFAILING(*(uint8_t*)0x20005682 = 0);
    NONFAILING(*(uint8_t*)0x20005683 = 0);
    NONFAILING(*(uint8_t*)0x20005684 = 0);
    NONFAILING(*(uint8_t*)0x20005685 = 0);
    NONFAILING(*(uint8_t*)0x20005686 = 0);
    NONFAILING(*(uint8_t*)0x20005687 = 0);
    NONFAILING(*(uint32_t*)0x20005fc8 = 0x80);
    NONFAILING(*(uint64_t*)0x20005fd0 = 0x20005b00);
    NONFAILING(*(uint64_t*)0x20005fd8 = 0);
    NONFAILING(*(uint64_t*)0x20005fe0 = 0);
    NONFAILING(*(uint64_t*)0x20005fe8 = 0);
    NONFAILING(*(uint32_t*)0x20005ff0 = 0);
    NONFAILING(*(uint32_t*)0x20005ff8 = 0);
    NONFAILING(*(uint64_t*)0x20006000 = 0x20005b80);
    NONFAILING(*(uint16_t*)0x20005b80 = 0x1f);
    NONFAILING(*(uint16_t*)0x20005b82 = 0);
    NONFAILING(*(uint8_t*)0x20005b84 = 0);
    NONFAILING(*(uint8_t*)0x20005b85 = 0);
    NONFAILING(*(uint8_t*)0x20005b86 = 0);
    NONFAILING(*(uint8_t*)0x20005b87 = 0);
    NONFAILING(*(uint8_t*)0x20005b88 = 0);
    NONFAILING(*(uint8_t*)0x20005b89 = 0);
    NONFAILING(*(uint16_t*)0x20005b8a = 0);
    NONFAILING(*(uint8_t*)0x20005b8c = 0);
    NONFAILING(*(uint32_t*)0x20006008 = 0x80);
    NONFAILING(*(uint64_t*)0x20006010 = 0x20005c40);
    NONFAILING(*(uint64_t*)0x20006018 = 0x1f4);
    NONFAILING(*(uint64_t*)0x20006020 = 0x20005c80);
    NONFAILING(*(uint64_t*)0x20006028 = 0x3a00);
    NONFAILING(*(uint32_t*)0x20006030 = 0);
    NONFAILING(*(uint32_t*)0x20006038 = 0);
    syscall(__NR_sendmmsg, r[1], 0x20005fc0, 0x3e8, 0);
    break;
  case 5:
    syscall(__NR_sendmmsg, r[0], 0x20001b00, 0xb8, 0);
    break;
  }
}
int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  install_segv_handler();
  use_temporary_dir();
  do_sandbox_namespace();
  return 0;
}
