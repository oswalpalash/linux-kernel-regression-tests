// https://syzkaller.appspot.com/bug?id=4b1e9cf490ffaaa8d9b87c2db04490923603aedd
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
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

static unsigned long long procid;

static void sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
}

static uint64_t current_time_ms(void)
{
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    exit(1);
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void use_temporary_dir(void)
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

#define MAX_FDS 30

static void mount_cgroups(const char* dir, const char** controllers, int count)
{
  if (mkdir(dir, 0777)) {
  }
  char enabled[128] = {0};
  int i = 0;
  for (; i < count; i++) {
    if (mount("none", dir, "cgroup", 0, controllers[i])) {
      continue;
    }
    umount(dir);
    strcat(enabled, ",");
    strcat(enabled, controllers[i]);
  }
  if (enabled[0] == 0)
    return;
  if (mount("none", dir, "cgroup", 0, enabled + 1)) {
  }
  if (chmod(dir, 0777)) {
  }
}

static void setup_cgroups()
{
  const char* unified_controllers[] = {"+cpu", "+memory", "+io", "+pids"};
  const char* net_controllers[] = {"net", "net_prio", "devices", "blkio",
                                   "freezer"};
  const char* cpu_controllers[] = {"cpuset", "cpuacct", "hugetlb", "rlimit"};
  if (mkdir("/syzcgroup", 0777)) {
  }
  if (mkdir("/syzcgroup/unified", 0777)) {
  }
  if (mount("none", "/syzcgroup/unified", "cgroup2", 0, NULL)) {
  }
  if (chmod("/syzcgroup/unified", 0777)) {
  }
  int unified_control =
      open("/syzcgroup/unified/cgroup.subtree_control", O_WRONLY);
  if (unified_control != -1) {
    unsigned i;
    for (i = 0;
         i < sizeof(unified_controllers) / sizeof(unified_controllers[0]); i++)
      if (write(unified_control, unified_controllers[i],
                strlen(unified_controllers[i])) < 0) {
      }
    close(unified_control);
  }
  mount_cgroups("/syzcgroup/net", net_controllers,
                sizeof(net_controllers) / sizeof(net_controllers[0]));
  mount_cgroups("/syzcgroup/cpu", cpu_controllers,
                sizeof(cpu_controllers) / sizeof(cpu_controllers[0]));
  write_file("/syzcgroup/cpu/cgroup.clone_children", "1");
  write_file("/syzcgroup/cpu/cpuset.memory_pressure_enabled", "1");
}

static void setup_cgroups_loop()
{
  int pid = getpid();
  char file[128];
  char cgroupdir[64];
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/unified/syz%llu", procid);
  if (mkdir(cgroupdir, 0777)) {
  }
  snprintf(file, sizeof(file), "%s/pids.max", cgroupdir);
  write_file(file, "32");
  snprintf(file, sizeof(file), "%s/memory.low", cgroupdir);
  write_file(file, "%d", 298 << 20);
  snprintf(file, sizeof(file), "%s/memory.high", cgroupdir);
  write_file(file, "%d", 299 << 20);
  snprintf(file, sizeof(file), "%s/memory.max", cgroupdir);
  write_file(file, "%d", 300 << 20);
  snprintf(file, sizeof(file), "%s/cgroup.procs", cgroupdir);
  write_file(file, "%d", pid);
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/cpu/syz%llu", procid);
  if (mkdir(cgroupdir, 0777)) {
  }
  snprintf(file, sizeof(file), "%s/cgroup.procs", cgroupdir);
  write_file(file, "%d", pid);
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/net/syz%llu", procid);
  if (mkdir(cgroupdir, 0777)) {
  }
  snprintf(file, sizeof(file), "%s/cgroup.procs", cgroupdir);
  write_file(file, "%d", pid);
}

static void setup_cgroups_test()
{
  char cgroupdir[64];
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/unified/syz%llu", procid);
  if (symlink(cgroupdir, "./cgroup")) {
  }
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/cpu/syz%llu", procid);
  if (symlink(cgroupdir, "./cgroup.cpu")) {
  }
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/net/syz%llu", procid);
  if (symlink(cgroupdir, "./cgroup.net")) {
  }
}

static void setup_common()
{
  if (mount(0, "/sys/fs/fuse/connections", "fusectl", 0, 0)) {
  }
}

static void setup_binderfs()
{
  if (mkdir("/dev/binderfs", 0777)) {
  }
  if (mount("binder", "/dev/binderfs", "binder", 0, NULL)) {
  }
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setsid();
  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = (200 << 20);
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 32 << 20;
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
  if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL)) {
  }
  if (unshare(CLONE_NEWIPC)) {
  }
  if (unshare(0x02000000)) {
  }
  if (unshare(CLONE_NEWUTS)) {
  }
  if (unshare(CLONE_SYSVSEM)) {
  }
  typedef struct {
    const char* name;
    const char* value;
  } sysctl_t;
  static const sysctl_t sysctls[] = {
      {"/proc/sys/kernel/shmmax", "16777216"},
      {"/proc/sys/kernel/shmall", "536870912"},
      {"/proc/sys/kernel/shmmni", "1024"},
      {"/proc/sys/kernel/msgmax", "8192"},
      {"/proc/sys/kernel/msgmni", "1024"},
      {"/proc/sys/kernel/msgmnb", "1024"},
      {"/proc/sys/kernel/sem", "1024 1048576 500 1024"},
  };
  unsigned i;
  for (i = 0; i < sizeof(sysctls) / sizeof(sysctls[0]); i++)
    write_file(sysctls[i].name, sysctls[i].value);
}

static int wait_for_loop(int pid)
{
  if (pid < 0)
    exit(1);
  int status = 0;
  while (waitpid(-1, &status, __WALL) != pid) {
  }
  return WEXITSTATUS(status);
}

static void drop_caps(void)
{
  struct __user_cap_header_struct cap_hdr = {};
  struct __user_cap_data_struct cap_data[2] = {};
  cap_hdr.version = _LINUX_CAPABILITY_VERSION_3;
  cap_hdr.pid = getpid();
  if (syscall(SYS_capget, &cap_hdr, &cap_data))
    exit(1);
  const int drop = (1 << CAP_SYS_PTRACE) | (1 << CAP_SYS_NICE);
  cap_data[0].effective &= ~drop;
  cap_data[0].permitted &= ~drop;
  cap_data[0].inheritable &= ~drop;
  if (syscall(SYS_capset, &cap_hdr, &cap_data))
    exit(1);
}

static int do_sandbox_none(void)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid != 0)
    return wait_for_loop(pid);
  setup_common();
  sandbox_common();
  drop_caps();
  if (unshare(CLONE_NEWNET)) {
  }
  setup_binderfs();
  loop();
  exit(1);
}

#define FS_IOC_SETFLAGS _IOW('f', 2, long)
static void remove_dir(const char* dir)
{
  int iter = 0;
  DIR* dp = 0;
retry:
  while (umount2(dir, MNT_DETACH | UMOUNT_NOFOLLOW) == 0) {
  }
  dp = opendir(dir);
  if (dp == NULL) {
    if (errno == EMFILE) {
      exit(1);
    }
    exit(1);
  }
  struct dirent* ep = 0;
  while ((ep = readdir(dp))) {
    if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
      continue;
    char filename[FILENAME_MAX];
    snprintf(filename, sizeof(filename), "%s/%s", dir, ep->d_name);
    while (umount2(filename, MNT_DETACH | UMOUNT_NOFOLLOW) == 0) {
    }
    struct stat st;
    if (lstat(filename, &st))
      exit(1);
    if (S_ISDIR(st.st_mode)) {
      remove_dir(filename);
      continue;
    }
    int i;
    for (i = 0;; i++) {
      if (unlink(filename) == 0)
        break;
      if (errno == EPERM) {
        int fd = open(filename, O_RDONLY);
        if (fd != -1) {
          long flags = 0;
          if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == 0) {
          }
          close(fd);
          continue;
        }
      }
      if (errno == EROFS) {
        break;
      }
      if (errno != EBUSY || i > 100)
        exit(1);
      if (umount2(filename, MNT_DETACH | UMOUNT_NOFOLLOW))
        exit(1);
    }
  }
  closedir(dp);
  for (int i = 0;; i++) {
    if (rmdir(dir) == 0)
      break;
    if (i < 100) {
      if (errno == EPERM) {
        int fd = open(dir, O_RDONLY);
        if (fd != -1) {
          long flags = 0;
          if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == 0) {
          }
          close(fd);
          continue;
        }
      }
      if (errno == EROFS) {
        break;
      }
      if (errno == EBUSY) {
        if (umount2(dir, MNT_DETACH | UMOUNT_NOFOLLOW))
          exit(1);
        continue;
      }
      if (errno == ENOTEMPTY) {
        if (iter < 100) {
          iter++;
          goto retry;
        }
      }
    }
    exit(1);
  }
}

static void kill_and_wait(int pid, int* status)
{
  kill(-pid, SIGKILL);
  kill(pid, SIGKILL);
  for (int i = 0; i < 100; i++) {
    if (waitpid(-1, status, WNOHANG | __WALL) == pid)
      return;
    usleep(1000);
  }
  DIR* dir = opendir("/sys/fs/fuse/connections");
  if (dir) {
    for (;;) {
      struct dirent* ent = readdir(dir);
      if (!ent)
        break;
      if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        continue;
      char abort[300];
      snprintf(abort, sizeof(abort), "/sys/fs/fuse/connections/%s/abort",
               ent->d_name);
      int fd = open(abort, O_WRONLY);
      if (fd == -1) {
        continue;
      }
      if (write(fd, abort, 1) < 0) {
      }
      close(fd);
    }
    closedir(dir);
  } else {
  }
  while (waitpid(-1, status, __WALL) != pid) {
  }
}

static void setup_loop()
{
  setup_cgroups_loop();
}

static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setup_cgroups_test();
  write_file("/proc/self/oom_score_adj", "1000");
  if (symlink("/dev/binderfs", "./binderfs")) {
  }
}

static void close_fds()
{
  for (int fd = 3; fd < MAX_FDS; fd++)
    close(fd);
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  setup_loop();
  int iter = 0;
  for (;; iter++) {
    char cwdbuf[32];
    sprintf(cwdbuf, "./%d", iter);
    if (mkdir(cwdbuf, 0777))
      exit(1);
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      if (chdir(cwdbuf))
        exit(1);
      setup_test();
      execute_one();
      close_fds();
      exit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      if (waitpid(-1, &status, WNOHANG | WAIT_FLAGS) == pid)
        break;
      sleep_ms(1);
      if (current_time_ms() - start < 5000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
    remove_dir(cwdbuf);
  }
}

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  memcpy((void*)0x20000000, "./cgroup/syz0\000", 14);
  syscall(__NR_mkdirat, 0xffffffffffffff9cul, 0x20000000ul, 0x1fful);
  res =
      syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000000ul, 0x200002ul, 0ul);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000080, "cpu.pressure\000", 13);
  res = syscall(__NR_openat, r[0], 0x20000080ul, 2ul, 0ul);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x20001340, "some", 4);
  *(uint8_t*)0x20001344 = 0x20;
  sprintf((char*)0x20001345, "%020llu", (long long)2);
  *(uint8_t*)0x20001359 = 0x20;
  sprintf((char*)0x2000135a, "%020llu", (long long)0x100000);
  *(uint8_t*)0x2000136e = 0;
  syscall(__NR_write, r[1], 0x20001340ul, 0x2ful);
  res = syscall(__NR_epoll_create, 0x10001);
  if (res != -1)
    r[2] = res;
  *(uint32_t*)0x20000040 = 0;
  *(uint64_t*)0x20000044 = 0;
  syscall(__NR_epoll_ctl, r[2], 1ul, r[1], 0x20000040ul);
  memcpy((void*)0x20001340, "some", 4);
  *(uint8_t*)0x20001344 = 0x20;
  sprintf((char*)0x20001345, "%020llu", (long long)2);
  *(uint8_t*)0x20001359 = 0x20;
  sprintf((char*)0x2000135a, "%020llu", (long long)0x100000);
  *(uint8_t*)0x2000136e = 0;
  syscall(__NR_write, r[1], 0x20001340ul, 0x2ful);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  setup_cgroups();
  use_temporary_dir();
  do_sandbox_none();
  return 0;
}
