// https://syzkaller.appspot.com/bug?id=2efdf7d22ac297f25a96c18287cf2b310f9c827a
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
#include <sys/swap.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/capability.h>
#include <linux/falloc.h>

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

static int runcmdline(char* cmdline)
{
  int ret = system(cmdline);
  if (ret) {
  }
  return ret;
}

#define MAX_FDS 30

static void mount_cgroups(const char* dir, const char** controllers, int count)
{
  if (mkdir(dir, 0777)) {
    return;
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
  if (enabled[0] == 0) {
    if (rmdir(dir) && errno != EBUSY)
      exit(1);
    return;
  }
  if (mount("none", dir, "cgroup", 0, enabled + 1)) {
    if (rmdir(dir) && errno != EBUSY)
      exit(1);
  }
  if (chmod(dir, 0777)) {
  }
}

static void mount_cgroups2(const char** controllers, int count)
{
  if (mkdir("/syzcgroup/unified", 0777)) {
    return;
  }
  if (mount("none", "/syzcgroup/unified", "cgroup2", 0, NULL)) {
    if (rmdir("/syzcgroup/unified") && errno != EBUSY)
      exit(1);
    return;
  }
  if (chmod("/syzcgroup/unified", 0777)) {
  }
  int control = open("/syzcgroup/unified/cgroup.subtree_control", O_WRONLY);
  if (control == -1)
    return;
  int i;
  for (i = 0; i < count; i++)
    if (write(control, controllers[i], strlen(controllers[i])) < 0) {
    }
  close(control);
}

static void setup_cgroups()
{
  const char* unified_controllers[] = {"+cpu", "+io", "+pids"};
  const char* net_controllers[] = {"net", "net_prio", "devices", "blkio",
                                   "freezer"};
  const char* cpu_controllers[] = {"cpuset", "cpuacct", "hugetlb", "rlimit",
                                   "memory"};
  if (mkdir("/syzcgroup", 0777)) {
    return;
  }
  mount_cgroups2(unified_controllers,
                 sizeof(unified_controllers) / sizeof(unified_controllers[0]));
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
  snprintf(file, sizeof(file), "%s/cgroup.procs", cgroupdir);
  write_file(file, "%d", pid);
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/cpu/syz%llu", procid);
  if (mkdir(cgroupdir, 0777)) {
  }
  snprintf(file, sizeof(file), "%s/cgroup.procs", cgroupdir);
  write_file(file, "%d", pid);
  snprintf(file, sizeof(file), "%s/memory.soft_limit_in_bytes", cgroupdir);
  write_file(file, "%d", 299 << 20);
  snprintf(file, sizeof(file), "%s/memory.limit_in_bytes", cgroupdir);
  write_file(file, "%d", 300 << 20);
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
  rlim.rlim_cur = rlim.rlim_max = 128 << 20;
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
  write_file("/proc/sys/net/ipv4/ping_group_range", "0 65535");
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

#define SWAP_FILE "./swap-file"
#define SWAP_FILE_SIZE (128 * 1000 * 1000)

static void setup_swap()
{
  swapoff(SWAP_FILE);
  unlink(SWAP_FILE);
  int fd = open(SWAP_FILE, O_CREAT | O_WRONLY | O_CLOEXEC, 0600);
  if (fd == -1) {
    exit(1);
    return;
  }
  fallocate(fd, FALLOC_FL_ZERO_RANGE, 0, SWAP_FILE_SIZE);
  close(fd);
  char cmdline[64];
  sprintf(cmdline, "mkswap %s", SWAP_FILE);
  if (runcmdline(cmdline)) {
    exit(1);
    return;
  }
  if (swapon(SWAP_FILE, SWAP_FLAG_PREFER) == 1) {
    exit(1);
    return;
  }
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

uint64_t r[1] = {0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, /*domain=*/2ul, /*type=*/3ul, /*proto=*/2);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000500,
         "raw\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000",
         32);
  *(uint32_t*)0x20000520 = 8;
  *(uint32_t*)0x20000524 = 0x8000000;
  *(uint32_t*)0x20000528 = 0x1fffffbf;
  *(uint32_t*)0x2000052c = 0x318;
  *(uint32_t*)0x20000530 = 0x11;
  *(uint32_t*)0x20000534 = 0x148;
  *(uint32_t*)0x20000538 = 0;
  *(uint32_t*)0x2000053c = 0x10;
  *(uint32_t*)0x20000540 = 0x3e8;
  *(uint32_t*)0x20000544 = 0x2a8;
  *(uint32_t*)0x20000548 = 0x2a8;
  *(uint32_t*)0x2000054c = 0x3e8;
  *(uint32_t*)0x20000550 = 0x2a8;
  *(uint32_t*)0x20000554 = 0xac;
  *(uint64_t*)0x20000558 = 0;
  memset((void*)0x20000560, 0, 84);
  *(uint32_t*)0x200005b4 = 0x10;
  *(uint16_t*)0x200005b8 = 0x70;
  *(uint16_t*)0x200005ba = 0x90;
  *(uint32_t*)0x200005bc = 0x1c;
  *(uint64_t*)0x200005c0 = 0;
  *(uint64_t*)0x200005c8 = 0;
  *(uint16_t*)0x200005d0 = 0x20;
  memcpy((void*)0x200005d2,
         "TRACE\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000",
         29);
  *(uint8_t*)0x200005ef = 0;
  *(uint8_t*)0x200005f0 = 0xac;
  *(uint8_t*)0x200005f1 = 0x14;
  *(uint8_t*)0x200005f2 = 0x14;
  *(uint8_t*)0x200005f3 = 0;
  *(uint32_t*)0x200005f4 = htobe32(0);
  *(uint32_t*)0x200005f8 = htobe32(0);
  *(uint32_t*)0x200005fc = htobe32(0);
  memcpy((void*)0x20000600, "syzkaller0\000\000\000\000\000\000", 16);
  memcpy((void*)0x20000610, "veth1_to_batadv\000", 16);
  *(uint8_t*)0x20000620 = 0;
  *(uint8_t*)0x20000630 = 0;
  *(uint16_t*)0x20000640 = 0;
  *(uint8_t*)0x20000642 = 0;
  *(uint8_t*)0x20000643 = 0;
  *(uint32_t*)0x20000644 = 0;
  *(uint16_t*)0x20000648 = 0x70;
  *(uint16_t*)0x2000064a = 0xd0;
  *(uint32_t*)0x2000064c = 0;
  *(uint64_t*)0x20000650 = 0;
  *(uint64_t*)0x20000658 = 0;
  *(uint16_t*)0x20000660 = 0x60;
  memcpy((void*)0x20000662,
         "SET\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000",
         29);
  *(uint8_t*)0x2000067f = 0;
  *(uint16_t*)0x20000680 = 0;
  *(uint32_t*)0x20000684 = 0;
  *(uint32_t*)0x20000688 = 0;
  *(uint32_t*)0x2000068c = 0;
  *(uint32_t*)0x20000690 = 0;
  *(uint32_t*)0x20000694 = 0;
  *(uint32_t*)0x20000698 = 0;
  *(uint8_t*)0x2000069c = 0;
  *(uint8_t*)0x2000069d = 0;
  *(uint16_t*)0x200006a0 = 0;
  *(uint32_t*)0x200006a4 = 0;
  *(uint32_t*)0x200006a8 = 0;
  *(uint32_t*)0x200006ac = 0;
  *(uint32_t*)0x200006b0 = 0;
  *(uint32_t*)0x200006b4 = 0;
  *(uint32_t*)0x200006b8 = 0;
  *(uint8_t*)0x200006bc = 0;
  *(uint8_t*)0x200006bd = 0;
  memset((void*)0x200006c0, 0, 84);
  *(uint32_t*)0x20000714 = 0xc8;
  *(uint16_t*)0x20000718 = 0x70;
  *(uint16_t*)0x2000071a = 0x98;
  *(uint32_t*)0x2000071c = 0;
  *(uint64_t*)0x20000720 = 0;
  *(uint64_t*)0x20000728 = 0;
  *(uint16_t*)0x20000730 = 0x28;
  memset((void*)0x20000732, 0, 29);
  *(uint8_t*)0x2000074f = 0;
  *(uint32_t*)0x20000750 = 0xfffffffe;
  syscall(__NR_setsockopt, /*fd=*/r[0], /*level=*/0, /*opt=*/0x40,
          /*val=*/0x20000500ul, /*len=*/0x258ul);
}
int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul, /*prot=*/7ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  setup_cgroups();
  setup_swap();
  use_temporary_dir();
  do_sandbox_none();
  return 0;
}
