// https://syzkaller.appspot.com/bug?id=668579fa698736a44e81d21c348d6df1b436c52c
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/capability.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
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
#include <stdlib.h>
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
    close(fd);
    return false;
  }
  close(fd);
  return true;
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

  if (mkdir("./syz-tmp", 0777))
    fail("mkdir(syz-tmp) failed");
  if (mount("", "./syz-tmp", "tmpfs", 0, NULL))
    fail("mount(tmpfs) failed");
  if (mkdir("./syz-tmp/newroot", 0777))
    fail("mkdir failed");
  if (mkdir("./syz-tmp/newroot/dev", 0700))
    fail("mkdir failed");
  if (mount("/dev", "./syz-tmp/newroot/dev", NULL,
            MS_BIND | MS_REC | MS_PRIVATE, NULL))
    fail("mount(dev) failed");
  if (mkdir("./syz-tmp/newroot/proc", 0700))
    fail("mkdir failed");
  if (mount(NULL, "./syz-tmp/newroot/proc", "proc", 0, NULL))
    fail("mount(proc) failed");
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

static int do_sandbox_namespace(int executor_pid, bool enable_tun)
{
  int pid;

  real_uid = getuid();
  real_gid = getgid();
  mprotect(sandbox_stack, 4096, PROT_NONE);
  pid =
      clone(namespace_sandbox_proc, &sandbox_stack[sizeof(sandbox_stack) - 64],
            CLONE_NEWUSER | CLONE_NEWPID, NULL);
  if (pid < 0)
    fail("sandbox clone failed");
  return pid;
}

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

void loop()
{
  syscall(__NR_mmap, 0x20000000, 0xf01000, 3, 0x32, -1, 0);
  *(uint32_t*)0x204f9fe4 = 3;
  *(uint32_t*)0x204f9fe8 = 4;
  *(uint32_t*)0x204f9fec = 4;
  *(uint32_t*)0x204f9ff0 = 0xfffffffd;
  *(uint32_t*)0x204f9ff4 = 0;
  *(uint32_t*)0x204f9ff8 = 0;
  *(uint32_t*)0x204f9ffc = 0;
  *(uint8_t*)0x204fa000 = 0;
  *(uint8_t*)0x204fa001 = 0;
  *(uint8_t*)0x204fa002 = 0;
  *(uint8_t*)0x204fa003 = 0;
  *(uint8_t*)0x204fa004 = 0;
  *(uint8_t*)0x204fa005 = 0;
  *(uint8_t*)0x204fa006 = 0;
  *(uint8_t*)0x204fa007 = 0;
  *(uint8_t*)0x204fa008 = 0;
  *(uint8_t*)0x204fa009 = 0;
  *(uint8_t*)0x204fa00a = 0;
  *(uint8_t*)0x204fa00b = 0;
  *(uint8_t*)0x204fa00c = 0;
  *(uint8_t*)0x204fa00d = 0;
  *(uint8_t*)0x204fa00e = 0;
  *(uint8_t*)0x204fa00f = 0;
  syscall(__NR_bpf, 0, 0x204f9fe4, 0x2c);
}

int main()
{
  use_temporary_dir();
  int pid = do_sandbox_namespace(0, false);
  int status = 0;
  while (waitpid(pid, &status, __WALL) != pid) {
  }
  return 0;
}
