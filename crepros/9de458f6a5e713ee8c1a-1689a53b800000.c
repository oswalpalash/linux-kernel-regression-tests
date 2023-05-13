// https://syzkaller.appspot.com/bug?id=d204aaca3ac260c553e053c566b529f350ea6454
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/capability.h>
#include <linux/futex.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
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
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <sys/stat.h>

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

#define BITMASK_LEN(type, bf_len) (type)((1ull << (bf_len)) - 1)

#define BITMASK_LEN_OFF(type, bf_off, bf_len)                                  \
  (type)(BITMASK_LEN(type, (bf_len)) << (bf_off))

#define STORE_BY_BITMASK(type, addr, val, bf_off, bf_len)                      \
  if ((bf_off) == 0 && (bf_len) == 0) {                                        \
    *(type*)(addr) = (type)(val);                                              \
  } else {                                                                     \
    type new_val = *(type*)(addr);                                             \
    new_val &= ~BITMASK_LEN_OFF(type, (bf_off), (bf_len));                     \
    new_val |= ((type)(val)&BITMASK_LEN(type, (bf_len))) << (bf_off);          \
    *(type*)(addr) = new_val;                                                  \
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

static void use_temporary_dir()
{
  char tmpdir_template[] = "./syzkaller.XXXXXX";
  char* tmpdir = mkdtemp(tmpdir_template);
  if (!tmpdir)
    fail("failed to mkdtemp");
  if (chmod(tmpdir, 0777))
    fail("failed to chmod");
  if (chdir(tmpdir))
    fail("failed to chdir");
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

static void setup_cgroups()
{
  if (mkdir("/syzcgroup", 0777)) {
  }
  if (mkdir("/syzcgroup/unified", 0777)) {
  }
  if (mount("none", "/syzcgroup/unified", "cgroup2", 0, NULL)) {
  }
  if (chmod("/syzcgroup/unified", 0777)) {
  }
  if (!write_file("/syzcgroup/unified/cgroup.subtree_control",
                  "+cpu +memory +io +pids +rdma")) {
  }
  if (mkdir("/syzcgroup/cpu", 0777)) {
  }
  if (mount("none", "/syzcgroup/cpu", "cgroup", 0,
            "cpuset,cpuacct,perf_event,hugetlb")) {
  }
  if (!write_file("/syzcgroup/cpu/cgroup.clone_children", "1")) {
  }
  if (chmod("/syzcgroup/cpu", 0777)) {
  }
  if (mkdir("/syzcgroup/net", 0777)) {
  }
  if (mount("none", "/syzcgroup/net", "cgroup", 0,
            "net_cls,net_prio,devices,freezer")) {
  }
  if (chmod("/syzcgroup/net", 0777)) {
  }
}

static void setup_binfmt_misc()
{
  if (!write_file("/proc/sys/fs/binfmt_misc/register",
                  ":syz0:M:0:syz0::./file0:")) {
  }
  if (!write_file("/proc/sys/fs/binfmt_misc/register",
                  ":syz1:M:1:yz1::./file0:POC")) {
  }
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

static int real_uid;
static int real_gid;
__attribute__((aligned(64 << 10))) static char sandbox_stack[1 << 20];

static int namespace_sandbox_proc(void* arg)
{
  sandbox_common();

  write_file("/proc/self/setgroups", "deny");
  if (!write_file("/proc/self/uid_map", "0 %d 1\n", real_uid))
    fail("write of /proc/self/uid_map failed");
  if (!write_file("/proc/self/gid_map", "0 %d 1\n", real_gid))
    fail("write of /proc/self/gid_map failed");

  if (unshare(CLONE_NEWNET))
    fail("unshare(CLONE_NEWNET)");

  if (mkdir("./syz-tmp", 0777))
    fail("mkdir(syz-tmp) failed");
  if (mount("", "./syz-tmp", "tmpfs", 0, NULL))
    fail("mount(tmpfs) failed");
  if (mkdir("./syz-tmp/newroot", 0777))
    fail("mkdir failed");
  if (mkdir("./syz-tmp/newroot/dev", 0700))
    fail("mkdir failed");
  unsigned mount_flags = MS_BIND | MS_REC | MS_PRIVATE;
  if (mount("/dev", "./syz-tmp/newroot/dev", NULL, mount_flags, NULL))
    fail("mount(dev) failed");
  if (mkdir("./syz-tmp/newroot/proc", 0700))
    fail("mkdir failed");
  if (mount(NULL, "./syz-tmp/newroot/proc", "proc", 0, NULL))
    fail("mount(proc) failed");
  if (mkdir("./syz-tmp/newroot/selinux", 0700))
    fail("mkdir failed");
  const char* selinux_path = "./syz-tmp/newroot/selinux";
  if (mount("/selinux", selinux_path, NULL, mount_flags, NULL)) {
    if (errno != ENOENT)
      fail("mount(/selinux) failed");
    if (mount("/sys/fs/selinux", selinux_path, NULL, mount_flags, NULL) &&
        errno != ENOENT)
      fail("mount(/sys/fs/selinux) failed");
  }
  if (mkdir("./syz-tmp/newroot/sys", 0700))
    fail("mkdir failed");
  if (mount(NULL, "./syz-tmp/newroot/sys", "sysfs", 0, NULL))
    fail("mount(sysfs) failed");
  if (mkdir("./syz-tmp/newroot/syzcgroup", 0700))
    fail("mkdir failed");
  if (mkdir("./syz-tmp/newroot/syzcgroup/unified", 0700))
    fail("mkdir failed");
  if (mkdir("./syz-tmp/newroot/syzcgroup/cpu", 0700))
    fail("mkdir failed");
  if (mkdir("./syz-tmp/newroot/syzcgroup/net", 0700))
    fail("mkdir failed");
  if (mount("/syzcgroup/unified", "./syz-tmp/newroot/syzcgroup/unified", NULL,
            mount_flags, NULL)) {
  }
  if (mount("/syzcgroup/cpu", "./syz-tmp/newroot/syzcgroup/cpu", NULL,
            mount_flags, NULL)) {
  }
  if (mount("/syzcgroup/net", "./syz-tmp/newroot/syzcgroup/net", NULL,
            mount_flags, NULL)) {
  }
  if (mkdir("./syz-tmp/pivot", 0777))
    fail("mkdir failed");
  if (syscall(SYS_pivot_root, "./syz-tmp", "./syz-tmp/pivot")) {
    if (chdir("./syz-tmp"))
      fail("chdir failed");
  } else {
    if (chdir("/"))
      fail("chdir failed");
    if (umount2("./pivot", MNT_DETACH))
      fail("umount failed");
  }
  if (chroot("./newroot"))
    fail("chroot failed");
  if (chdir("/"))
    fail("chdir failed");

  struct __user_cap_header_struct cap_hdr = {};
  struct __user_cap_data_struct cap_data[2] = {};
  cap_hdr.version = _LINUX_CAPABILITY_VERSION_3;
  cap_hdr.pid = getpid();
  if (syscall(SYS_capget, &cap_hdr, &cap_data))
    fail("capget failed");
  cap_data[0].effective &= ~(1 << CAP_SYS_PTRACE);
  cap_data[0].permitted &= ~(1 << CAP_SYS_PTRACE);
  cap_data[0].inheritable &= ~(1 << CAP_SYS_PTRACE);
  if (syscall(SYS_capset, &cap_hdr, &cap_data))
    fail("capset failed");

  loop();
  doexit(1);
}

static int do_sandbox_namespace(void)
{
  int pid;

  setup_cgroups();
  setup_binfmt_misc();
  real_uid = getuid();
  real_gid = getgid();
  mprotect(sandbox_stack, 4096, PROT_NONE);
  pid =
      clone(namespace_sandbox_proc, &sandbox_stack[sizeof(sandbox_stack) - 64],
            CLONE_NEWUSER | CLONE_NEWPID, 0);
  if (pid < 0)
    fail("sandbox clone failed");
  return pid;
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

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};
unsigned long long procid;
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    NONFAILING(*(uint64_t*)0x20000000 = 0x20000040);
    NONFAILING(*(uint64_t*)0x20000008 = 0);
    syscall(__NR_writev, -1, 0x20000000, 1);
    break;
  case 1:
    res = syscall(__NR_socket, 0xa, 2, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 2:
    NONFAILING(memcpy((void*)0x20000800, "./cgroup/syz0", 14));
    syscall(__NR_mkdirat, 0xffffffffffffff9c, 0x20000800, 0x1ff);
    break;
  case 3:
    NONFAILING(*(uint32_t*)0x20940000 = 2);
    NONFAILING(*(uint32_t*)0x20940004 = 0x78);
    NONFAILING(*(uint8_t*)0x20940008 = 0xfd);
    NONFAILING(*(uint8_t*)0x20940009 = 0);
    NONFAILING(*(uint8_t*)0x2094000a = 0);
    NONFAILING(*(uint8_t*)0x2094000b = 0);
    NONFAILING(*(uint32_t*)0x2094000c = 0);
    NONFAILING(*(uint64_t*)0x20940010 = 0);
    NONFAILING(*(uint64_t*)0x20940018 = 0);
    NONFAILING(*(uint64_t*)0x20940020 = 0);
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20940028, 0, 0, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20940028, 0, 1, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20940028, 0, 2, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20940028, 0, 3, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20940028, 0, 4, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20940028, 3, 5, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20940028, 0, 6, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20940028, 0, 7, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20940028, 0, 8, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20940028, 0, 9, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20940028, 0, 10, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20940028, 0, 11, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20940028, 0, 12, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20940028, 0, 13, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20940028, 0, 14, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20940028, 0, 15, 2));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20940028, 0, 17, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20940028, 0, 18, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20940028, 0, 19, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20940028, 0, 20, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20940028, 0, 21, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20940028, 0, 22, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20940028, 0, 23, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20940028, 0, 24, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20940028, 0, 25, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20940028, 0, 26, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20940028, 0, 27, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20940028, 0, 28, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20940028, 0, 29, 35));
    NONFAILING(*(uint32_t*)0x20940030 = 0);
    NONFAILING(*(uint32_t*)0x20940034 = 0);
    NONFAILING(*(uint64_t*)0x20940038 = 0x20000000);
    NONFAILING(*(uint64_t*)0x20940040 = 0);
    NONFAILING(*(uint64_t*)0x20940048 = 0);
    NONFAILING(*(uint64_t*)0x20940050 = 0);
    NONFAILING(*(uint32_t*)0x20940058 = 0);
    NONFAILING(*(uint32_t*)0x2094005c = 0);
    NONFAILING(*(uint64_t*)0x20940060 = 0);
    NONFAILING(*(uint32_t*)0x20940068 = 0);
    NONFAILING(*(uint16_t*)0x2094006c = 0);
    NONFAILING(*(uint16_t*)0x2094006e = 0);
    res = syscall(__NR_perf_event_open, 0x20940000, 0, 0, -1, 0);
    if (res != -1)
      r[1] = res;
    break;
  case 4:
    res = syscall(__NR_fcntl, r[0], 0, r[1]);
    if (res != -1)
      r[2] = res;
    break;
  case 5:
    NONFAILING(*(uint32_t*)0x20000100 = 2);
    NONFAILING(memcpy(
        (void*)0x20000104,
        "\x6c\x6f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
        16));
    NONFAILING(*(uint32_t*)0x20000114 = 0);
    syscall(__NR_setsockopt, r[2], 0, 0x48b, 0x20000100, 0x18);
    break;
  case 6:
    NONFAILING(*(uint32_t*)0x20000500 = 2);
    NONFAILING(memcpy(
        (void*)0x20000504,
        "\x69\x70\x64\x64\x70\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
        16));
    NONFAILING(*(uint32_t*)0x20000514 = 0);
    syscall(__NR_setsockopt, r[2], 0, 0x48c, 0x20000500, 0x18);
    break;
  case 7:
    NONFAILING(*(uint64_t*)0x20000740 = 0x20000180);
    NONFAILING(*(uint64_t*)0x20000748 = 0);
    NONFAILING(*(uint64_t*)0x20000750 = 0x20000240);
    NONFAILING(*(uint64_t*)0x20000758 = 0);
    NONFAILING(*(uint64_t*)0x20000760 = 0x200002c0);
    NONFAILING(*(uint64_t*)0x20000768 = 0);
    NONFAILING(*(uint64_t*)0x20000770 = 0x20000380);
    NONFAILING(*(uint64_t*)0x20000778 = 0);
    NONFAILING(*(uint64_t*)0x20000780 = 0x20000440);
    NONFAILING(*(uint64_t*)0x20000788 = 0);
    NONFAILING(*(uint64_t*)0x20000790 = 0x20000540);
    NONFAILING(*(uint64_t*)0x20000798 = 0);
    NONFAILING(*(uint64_t*)0x200007a0 = 0x20000640);
    NONFAILING(*(uint64_t*)0x200007a8 = 0);
    syscall(__NR_pwritev, r[1], 0x20000740, 7, 0);
    break;
  case 8:
    NONFAILING(*(uint32_t*)0x20000040 = 0);
    NONFAILING(*(uint32_t*)0x20000044 = 0);
    NONFAILING(*(uint32_t*)0x20000080 = 8);
    syscall(__NR_getsockopt, -1, 0x84, 0x13, 0x20000040, 0x20000080);
    break;
  }
}

void execute_one()
{
  execute(9);
  collide = 1;
  execute(9);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  char* cwd = get_current_dir_name();
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      install_segv_handler();
      for (;;) {
        if (chdir(cwd))
          fail("failed to chdir");
        use_temporary_dir();
        int pid = do_sandbox_namespace();
        int status = 0;
        while (waitpid(pid, &status, __WALL) != pid) {
        }
      }
    }
  }
  sleep(1000000);
  return 0;
}
