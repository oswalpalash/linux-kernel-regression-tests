// https://syzkaller.appspot.com/bug?id=c922592229951800c197ce48a5eaab8877c33723
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <errno.h>
#include <errno.h>
#include <sched.h>
#include <signal.h>
#include <signal.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/prctl.h>
#include <sys/resource.h>
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

static int do_sandbox_none(int executor_pid, bool enable_tun)
{
  unshare(CLONE_NEWPID);
  int pid = fork();
  if (pid < 0)
    fail("sandbox fork failed");
  if (pid)
    return pid;

  sandbox_common();

  loop();
  doexit(1);
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

long r[2];
void test()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
          0xfffffffffffffffful, 0x0ul);
  r[0] = syscall(__NR_socket, 0x10ul, 0x3ul, 0x6ul);
  *(uint16_t*)0x204ff000 = (uint16_t)0x10;
  *(uint16_t*)0x204ff002 = (uint16_t)0x0;
  *(uint32_t*)0x204ff004 = (uint32_t)0x0;
  *(uint32_t*)0x204ff008 = (uint32_t)0x5;
  syscall(__NR_bind, r[0], 0x204ff000ul, 0xcul);
  r[1] = syscall(__NR_socket, 0x2ul, 0x2ul, 0x0ul);
  *(uint64_t*)0x20000000 = (uint64_t)0x0;
  *(uint64_t*)0x20000008 = (uint64_t)0x100000000000000;
  *(uint8_t*)0x20000010 = (uint8_t)0xac;
  *(uint8_t*)0x20000011 = (uint8_t)0x14;
  *(uint8_t*)0x20000012 = (uint8_t)0x0;
  *(uint8_t*)0x20000013 = (uint8_t)0xbb;
  *(uint16_t*)0x20000020 = (uint16_t)0x204e;
  *(uint16_t*)0x20000022 = (uint16_t)0x0;
  *(uint16_t*)0x20000024 = (uint16_t)0x204e;
  *(uint16_t*)0x20000026 = (uint16_t)0x0;
  *(uint16_t*)0x20000028 = (uint16_t)0x2;
  *(uint8_t*)0x2000002a = (uint8_t)0x0;
  *(uint8_t*)0x2000002b = (uint8_t)0x0;
  *(uint8_t*)0x2000002c = (uint8_t)0x0;
  *(uint32_t*)0x20000030 = (uint32_t)0x0;
  *(uint32_t*)0x20000034 = (uint32_t)0x0;
  *(uint64_t*)0x20000038 = (uint64_t)0x0;
  *(uint64_t*)0x20000040 = (uint64_t)0x0;
  *(uint64_t*)0x20000048 = (uint64_t)0x0;
  *(uint64_t*)0x20000050 = (uint64_t)0x0;
  *(uint64_t*)0x20000058 = (uint64_t)0x0;
  *(uint64_t*)0x20000060 = (uint64_t)0x0;
  *(uint64_t*)0x20000068 = (uint64_t)0x0;
  *(uint64_t*)0x20000070 = (uint64_t)0x0;
  *(uint64_t*)0x20000078 = (uint64_t)0x0;
  *(uint64_t*)0x20000080 = (uint64_t)0x0;
  *(uint64_t*)0x20000088 = (uint64_t)0x0;
  *(uint64_t*)0x20000090 = (uint64_t)0x0;
  *(uint32_t*)0x20000098 = (uint32_t)0x0;
  *(uint32_t*)0x2000009c = (uint32_t)0x0;
  *(uint8_t*)0x200000a0 = (uint8_t)0x1;
  *(uint8_t*)0x200000a1 = (uint8_t)0x0;
  *(uint8_t*)0x200000a2 = (uint8_t)0x0;
  *(uint8_t*)0x200000a3 = (uint8_t)0x0;
  *(uint64_t*)0x200000a8 = (uint64_t)0x0;
  *(uint64_t*)0x200000b0 = (uint64_t)0x100000000000000;
  *(uint32_t*)0x200000b8 = (uint32_t)0x0;
  *(uint8_t*)0x200000bc = (uint8_t)0x0;
  *(uint16_t*)0x200000c0 = (uint16_t)0x0;
  *(uint8_t*)0x200000c4 = (uint8_t)0xac;
  *(uint8_t*)0x200000c5 = (uint8_t)0x14;
  *(uint8_t*)0x200000c6 = (uint8_t)0x0;
  *(uint8_t*)0x200000c7 = (uint8_t)0xaa;
  *(uint32_t*)0x200000d4 = (uint32_t)0x0;
  *(uint8_t*)0x200000d8 = (uint8_t)0x0;
  *(uint8_t*)0x200000d9 = (uint8_t)0x0;
  *(uint8_t*)0x200000da = (uint8_t)0xfffffffffffffffd;
  *(uint32_t*)0x200000dc = (uint32_t)0x0;
  *(uint32_t*)0x200000e0 = (uint32_t)0x0;
  *(uint32_t*)0x200000e4 = (uint32_t)0x0;
  syscall(__NR_setsockopt, r[1], 0x0ul, 0x11ul, 0x20000000ul, 0xe8ul);
  *(uint16_t*)0x20c24000 = (uint16_t)0x2;
  *(uint16_t*)0x20c24002 = (uint16_t)0x204e;
  *(uint32_t*)0x20c24004 = (uint32_t)0x0;
  *(uint8_t*)0x20c24008 = (uint8_t)0x0;
  *(uint8_t*)0x20c24009 = (uint8_t)0x0;
  *(uint8_t*)0x20c2400a = (uint8_t)0x0;
  *(uint8_t*)0x20c2400b = (uint8_t)0x0;
  *(uint8_t*)0x20c2400c = (uint8_t)0x0;
  *(uint8_t*)0x20c2400d = (uint8_t)0x0;
  *(uint8_t*)0x20c2400e = (uint8_t)0x0;
  *(uint8_t*)0x20c2400f = (uint8_t)0x0;
  syscall(__NR_sendto, r[1], 0x20098000ul, 0x0ul, 0x0ul, 0x20c24000ul,
          0x10ul);
  memcpy((void*)0x20632000, "./file0", 8);
  syscall(__NR_mkdir, 0x20632000ul, 0x16ul);
  memcpy((void*)0x2000a000, "./file0", 8);
  memcpy((void*)0x20014ff8, "./file0", 8);
  memcpy((void*)0x20014000, "proc", 5);
  syscall(__NR_mount, 0x2000a000ul, 0x20014ff8ul, 0x20014000ul, 0x0ul,
          0x20fc9000ul);
}

int main()
{
  int i;
  for (i = 0; i < 8; i++) {
    if (fork() == 0) {
      use_temporary_dir();
      int pid = do_sandbox_none(i, false);
      int status = 0;
      while (waitpid(pid, &status, __WALL) != pid) {
      }
      return 0;
    }
  }
  sleep(1000000);
  return 0;
}
