// https://syzkaller.appspot.com/bug?id=0d259373da8be7356652213543e1efc254a5abf0
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <errno.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/capability.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <signal.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/mount.h>
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

static void exitf(const char* msg, ...)
{
  int e = errno;
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit(kRetryStatus);
}

static uint64_t current_time_ms()
{
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    fail("clock_gettime failed");
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
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
  pid = clone(namespace_sandbox_proc,
              &sandbox_stack[sizeof(sandbox_stack) - 64],
              CLONE_NEWUSER | CLONE_NEWPID, NULL);
  if (pid < 0)
    fail("sandbox clone failed");
  return pid;
}

static void remove_dir(const char* dir)
{
  DIR* dp;
  struct dirent* ep;
  int iter = 0;
retry:
  dp = opendir(dir);
  if (dp == NULL) {
    if (errno == EMFILE) {
      exitf("opendir(%s) failed due to NOFILE, exiting");
    }
    exitf("opendir(%s) failed", dir);
  }
  while ((ep = readdir(dp))) {
    if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
      continue;
    char filename[FILENAME_MAX];
    snprintf(filename, sizeof(filename), "%s/%s", dir, ep->d_name);
    struct stat st;
    if (lstat(filename, &st))
      exitf("lstat(%s) failed", filename);
    if (S_ISDIR(st.st_mode)) {
      remove_dir(filename);
      continue;
    }
    int i;
    for (i = 0;; i++) {
      if (unlink(filename) == 0)
        break;
      if (errno == EROFS) {
        break;
      }
      if (errno != EBUSY || i > 100)
        exitf("unlink(%s) failed", filename);
      if (umount2(filename, MNT_DETACH))
        exitf("umount(%s) failed", filename);
    }
  }
  closedir(dp);
  int i;
  for (i = 0;; i++) {
    if (rmdir(dir) == 0)
      break;
    if (i < 100) {
      if (errno == EROFS) {
        break;
      }
      if (errno == EBUSY) {
        if (umount2(dir, MNT_DETACH))
          exitf("umount(%s) failed", dir);
        continue;
      }
      if (errno == ENOTEMPTY) {
        if (iter < 100) {
          iter++;
          goto retry;
        }
      }
    }
    exitf("rmdir(%s) failed", dir);
  }
}

static void test();

void loop()
{
  int iter;
  for (iter = 0;; iter++) {
    char cwdbuf[256];
    sprintf(cwdbuf, "./%d", iter);
    if (mkdir(cwdbuf, 0777))
      fail("failed to mkdir");
    int pid = fork();
    if (pid < 0)
      fail("loop fork failed");
    if (pid == 0) {
      prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
      setpgrp();
      if (chdir(cwdbuf))
        fail("failed to chdir");
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
    remove_dir(cwdbuf);
  }
}

long r[3];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
            0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    memcpy((void*)0x20e37000, "/dev/kvm", 9);
    r[0] = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20e37000ul,
                   0x0ul, 0x0ul);
    break;
  case 2:
    r[1] = syscall(__NR_ioctl, r[0], 0xae01ul, 0x0ul);
    break;
  case 3:
    r[2] = syscall(__NR_ioctl, r[1], 0xae41ul, 0x0ul);
    break;
  case 4:
    *(uint64_t*)0x2099afd0 = (uint64_t)0x2000;
    *(uint32_t*)0x2099afd8 = (uint32_t)0x6000;
    *(uint16_t*)0x2099afdc = (uint16_t)0xa;
    *(uint8_t*)0x2099afde = (uint8_t)0x11;
    *(uint8_t*)0x2099afdf = (uint8_t)0xff;
    *(uint8_t*)0x2099afe0 = (uint8_t)0x5e3fc9f4;
    *(uint8_t*)0x2099afe1 = (uint8_t)0x1;
    *(uint8_t*)0x2099afe2 = (uint8_t)0x81;
    *(uint8_t*)0x2099afe3 = (uint8_t)0x0;
    *(uint8_t*)0x2099afe4 = (uint8_t)0x6;
    *(uint8_t*)0x2099afe5 = (uint8_t)0x4;
    *(uint8_t*)0x2099afe6 = (uint8_t)0x101;
    *(uint8_t*)0x2099afe7 = (uint8_t)0x0;
    *(uint64_t*)0x2099afe8 = (uint64_t)0xf000;
    *(uint32_t*)0x2099aff0 = (uint32_t)0xd000;
    *(uint16_t*)0x2099aff4 = (uint16_t)0x0;
    *(uint8_t*)0x2099aff6 = (uint8_t)0x7;
    *(uint8_t*)0x2099aff7 = (uint8_t)0x1000;
    *(uint8_t*)0x2099aff8 = (uint8_t)0x1205;
    *(uint8_t*)0x2099aff9 = (uint8_t)0x3;
    *(uint8_t*)0x2099affa = (uint8_t)0x80000000;
    *(uint8_t*)0x2099affb = (uint8_t)0xb27;
    *(uint8_t*)0x2099affc = (uint8_t)0x5;
    *(uint8_t*)0x2099affd = (uint8_t)0x9;
    *(uint8_t*)0x2099affe = (uint8_t)0x0;
    *(uint8_t*)0x2099afff = (uint8_t)0x0;
    *(uint64_t*)0x2099b000 = (uint64_t)0x3000;
    *(uint32_t*)0x2099b008 = (uint32_t)0x0;
    *(uint16_t*)0x2099b00c = (uint16_t)0xf;
    *(uint8_t*)0x2099b00e = (uint8_t)0x80000000;
    *(uint8_t*)0x2099b00f = (uint8_t)0x1ff;
    *(uint8_t*)0x2099b010 = (uint8_t)0x7ff;
    *(uint8_t*)0x2099b011 = (uint8_t)0x1;
    *(uint8_t*)0x2099b012 = (uint8_t)0x7;
    *(uint8_t*)0x2099b013 = (uint8_t)0xf84;
    *(uint8_t*)0x2099b014 = (uint8_t)0x1ff;
    *(uint8_t*)0x2099b015 = (uint8_t)0x2;
    *(uint8_t*)0x2099b016 = (uint8_t)0x1000;
    *(uint8_t*)0x2099b017 = (uint8_t)0x0;
    *(uint64_t*)0x2099b018 = (uint64_t)0x3000;
    *(uint32_t*)0x2099b020 = (uint32_t)0x10000;
    *(uint16_t*)0x2099b024 = (uint16_t)0x8;
    *(uint8_t*)0x2099b026 = (uint8_t)0x1;
    *(uint8_t*)0x2099b027 = (uint8_t)0x7;
    *(uint8_t*)0x2099b028 = (uint8_t)0x9de1;
    *(uint8_t*)0x2099b029 = (uint8_t)0xffffffffffffffc1;
    *(uint8_t*)0x2099b02a = (uint8_t)0x7;
    *(uint8_t*)0x2099b02b = (uint8_t)0x7fff;
    *(uint8_t*)0x2099b02c = (uint8_t)0x9;
    *(uint8_t*)0x2099b02d = (uint8_t)0x1;
    *(uint8_t*)0x2099b02e = (uint8_t)0x89fd;
    *(uint8_t*)0x2099b02f = (uint8_t)0x0;
    *(uint64_t*)0x2099b030 = (uint64_t)0x2;
    *(uint32_t*)0x2099b038 = (uint32_t)0x1000;
    *(uint16_t*)0x2099b03c = (uint16_t)0x86ef34c8e8fc3ca8;
    *(uint8_t*)0x2099b03e = (uint8_t)0xdc33;
    *(uint8_t*)0x2099b03f = (uint8_t)0xffffffff;
    *(uint8_t*)0x2099b040 = (uint8_t)0xae13;
    *(uint8_t*)0x2099b041 = (uint8_t)0x100000000;
    *(uint8_t*)0x2099b042 = (uint8_t)0x10000;
    *(uint8_t*)0x2099b043 = (uint8_t)0x5;
    *(uint8_t*)0x2099b044 = (uint8_t)0xfff;
    *(uint8_t*)0x2099b045 = (uint8_t)0x4;
    *(uint8_t*)0x2099b046 = (uint8_t)0x7;
    *(uint8_t*)0x2099b047 = (uint8_t)0x0;
    *(uint64_t*)0x2099b048 = (uint64_t)0x100000;
    *(uint32_t*)0x2099b050 = (uint32_t)0xd004;
    *(uint16_t*)0x2099b054 = (uint16_t)0x4;
    *(uint8_t*)0x2099b056 = (uint8_t)0x3;
    *(uint8_t*)0x2099b057 = (uint8_t)0x4;
    *(uint8_t*)0x2099b058 = (uint8_t)0x1;
    *(uint8_t*)0x2099b059 = (uint8_t)0xfffffffffffffb53;
    *(uint8_t*)0x2099b05a = (uint8_t)0x9b;
    *(uint8_t*)0x2099b05b = (uint8_t)0x1;
    *(uint8_t*)0x2099b05c = (uint8_t)0x2;
    *(uint8_t*)0x2099b05d = (uint8_t)0x3;
    *(uint8_t*)0x2099b05e = (uint8_t)0x2;
    *(uint8_t*)0x2099b05f = (uint8_t)0x0;
    *(uint64_t*)0x2099b060 = (uint64_t)0x6001;
    *(uint32_t*)0x2099b068 = (uint32_t)0xd000;
    *(uint16_t*)0x2099b06c = (uint16_t)0x0;
    *(uint8_t*)0x2099b06e = (uint8_t)0x64d;
    *(uint8_t*)0x2099b06f = (uint8_t)0x9;
    *(uint8_t*)0x2099b070 = (uint8_t)0xba70;
    *(uint8_t*)0x2099b071 = (uint8_t)0xfff;
    *(uint8_t*)0x2099b072 = (uint8_t)0x1000;
    *(uint8_t*)0x2099b073 = (uint8_t)0x22;
    *(uint8_t*)0x2099b074 = (uint8_t)0x88e;
    *(uint8_t*)0x2099b075 = (uint8_t)0x91;
    *(uint8_t*)0x2099b076 = (uint8_t)0x8;
    *(uint8_t*)0x2099b077 = (uint8_t)0x0;
    *(uint64_t*)0x2099b078 = (uint64_t)0x0;
    *(uint32_t*)0x2099b080 = (uint32_t)0xf002;
    *(uint16_t*)0x2099b084 = (uint16_t)0x19;
    *(uint8_t*)0x2099b086 = (uint8_t)0x4;
    *(uint8_t*)0x2099b087 = (uint8_t)0x5;
    *(uint8_t*)0x2099b088 = (uint8_t)0x3;
    *(uint8_t*)0x2099b089 = (uint8_t)0x0;
    *(uint8_t*)0x2099b08a = (uint8_t)0x7;
    *(uint8_t*)0x2099b08b = (uint8_t)0x8;
    *(uint8_t*)0x2099b08c = (uint8_t)0x8;
    *(uint8_t*)0x2099b08d = (uint8_t)0x9;
    *(uint8_t*)0x2099b08e = (uint8_t)0x100;
    *(uint8_t*)0x2099b08f = (uint8_t)0x0;
    *(uint64_t*)0x2099b090 = (uint64_t)0x3000;
    *(uint16_t*)0x2099b098 = (uint16_t)0xf000;
    *(uint16_t*)0x2099b09a = (uint16_t)0x0;
    *(uint16_t*)0x2099b09c = (uint16_t)0x0;
    *(uint16_t*)0x2099b09e = (uint16_t)0x0;
    *(uint64_t*)0x2099b0a0 = (uint64_t)0x2000;
    *(uint16_t*)0x2099b0a8 = (uint16_t)0x3000;
    *(uint16_t*)0x2099b0aa = (uint16_t)0x0;
    *(uint16_t*)0x2099b0ac = (uint16_t)0x0;
    *(uint16_t*)0x2099b0ae = (uint16_t)0x0;
    *(uint64_t*)0x2099b0b0 = (uint64_t)0x1;
    *(uint64_t*)0x2099b0b8 = (uint64_t)0x0;
    *(uint64_t*)0x2099b0c0 = (uint64_t)0x10d000;
    *(uint64_t*)0x2099b0c8 = (uint64_t)0x100000;
    *(uint64_t*)0x2099b0d0 = (uint64_t)0x2;
    *(uint64_t*)0x2099b0d8 = (uint64_t)0x100;
    *(uint64_t*)0x2099b0e0 = (uint64_t)0x7002;
    *(uint64_t*)0x2099b0e8 = (uint64_t)0x2;
    *(uint64_t*)0x2099b0f0 = (uint64_t)0x8;
    *(uint64_t*)0x2099b0f8 = (uint64_t)0x2;
    *(uint64_t*)0x2099b100 = (uint64_t)0x80000001;
    syscall(__NR_ioctl, r[2], 0x4138ae84ul, 0x2099afd0ul);
    break;
  }
  return 0;
}

void test()
{
  long i;
  pthread_t th[10];

  memset(r, -1, sizeof(r));
  srand(getpid());
  for (i = 0; i < 5; i++) {
    pthread_create(&th[i], 0, thr, (void*)i);
    usleep(rand() % 10000);
  }
  for (i = 0; i < 5; i++) {
    pthread_create(&th[5 + i], 0, thr, (void*)i);
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
      use_temporary_dir();
      int pid = do_sandbox_namespace(i, false);
      int status = 0;
      while (waitpid(pid, &status, __WALL) != pid) {
      }
      return 0;
    }
  }
  sleep(1000000);
  return 0;
}
