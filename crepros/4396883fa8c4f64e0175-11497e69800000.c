// https://syzkaller.appspot.com/bug?id=be2e39518462291fd049f665261632ded34b79fd
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/time.h>
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

static void remove_dir(const char* dir)
{
  DIR* dp;
  struct dirent* ep;
  int iter = 0;
retry:
  dp = opendir(dir);
  if (dp == NULL) {
    if (errno == EMFILE) {
      exitf("opendir(%s) failed due to NOFILE, exiting", dir);
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

long r[1];
void test()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  memcpy((void*)0x2053bff8, "./file0", 8);
  syscall(__NR_mkdir, 0x2053bff8, 0);
  memcpy((void*)0x2000a000, "./file0", 8);
  memcpy((void*)0x20db0ff8, "./file0", 8);
  memcpy((void*)0x20f9cffb, "proc", 5);
  syscall(__NR_mount, 0x2000a000, 0x20db0ff8, 0x20f9cffb, 0, 0x20b71000);
  r[0] = syscall(__NR_socket, 0xf, 3, 2);
  *(uint64_t*)0x208befc8 = 0;
  *(uint32_t*)0x208befd0 = 0;
  *(uint64_t*)0x208befd8 = 0x208feff0;
  *(uint64_t*)0x208befe0 = 1;
  *(uint64_t*)0x208befe8 = 0;
  *(uint64_t*)0x208beff0 = 0;
  *(uint32_t*)0x208beff8 = 0;
  *(uint64_t*)0x208feff0 = 0x2022a000;
  *(uint64_t*)0x208feff8 = 0x80;
  *(uint8_t*)0x2022a000 = 2;
  *(uint8_t*)0x2022a001 = 3;
  *(uint8_t*)0x2022a002 = 0;
  *(uint8_t*)0x2022a003 = 9;
  *(uint16_t*)0x2022a004 = 0x10;
  *(uint16_t*)0x2022a006 = 0;
  *(uint32_t*)0x2022a008 = 0;
  *(uint32_t*)0x2022a00c = 0;
  *(uint16_t*)0x2022a010 = 2;
  *(uint16_t*)0x2022a012 = 0x13;
  *(uint8_t*)0x2022a014 = 2;
  *(uint8_t*)0x2022a015 = 0;
  *(uint16_t*)0x2022a016 = 0;
  *(uint32_t*)0x2022a018 = 0;
  *(uint32_t*)0x2022a01c = 0;
  *(uint16_t*)0x2022a020 = 5;
  *(uint16_t*)0x2022a022 = 6;
  *(uint8_t*)0x2022a024 = 0;
  *(uint8_t*)0x2022a025 = 0;
  *(uint16_t*)0x2022a026 = 0;
  *(uint16_t*)0x2022a028 = 0xa;
  *(uint16_t*)0x2022a02a = htobe16(0x4e20);
  *(uint32_t*)0x2022a02c = 0;
  *(uint8_t*)0x2022a030 = -1;
  *(uint8_t*)0x2022a031 = 2;
  *(uint8_t*)0x2022a032 = 0;
  *(uint8_t*)0x2022a033 = 0;
  *(uint8_t*)0x2022a034 = 0;
  *(uint8_t*)0x2022a035 = 0;
  *(uint8_t*)0x2022a036 = 0;
  *(uint8_t*)0x2022a037 = 0;
  *(uint8_t*)0x2022a038 = 0;
  *(uint8_t*)0x2022a039 = 0;
  *(uint8_t*)0x2022a03a = 0;
  *(uint8_t*)0x2022a03b = 0;
  *(uint8_t*)0x2022a03c = 0;
  *(uint8_t*)0x2022a03d = 0;
  *(uint8_t*)0x2022a03e = 0;
  *(uint8_t*)0x2022a03f = 1;
  *(uint32_t*)0x2022a040 = 0;
  *(uint16_t*)0x2022a048 = 2;
  *(uint16_t*)0x2022a04a = 1;
  *(uint32_t*)0x2022a04c = htobe32(0);
  *(uint8_t*)0x2022a050 = 0;
  *(uint8_t*)0x2022a051 = 0;
  *(uint8_t*)0x2022a052 = 0;
  *(uint8_t*)0x2022a053 = 2;
  *(uint32_t*)0x2022a054 = 0;
  *(uint16_t*)0x2022a058 = 5;
  *(uint16_t*)0x2022a05a = 5;
  *(uint8_t*)0x2022a05c = 0;
  *(uint8_t*)0x2022a05d = 0;
  *(uint16_t*)0x2022a05e = 0;
  *(uint16_t*)0x2022a060 = 0xa;
  *(uint16_t*)0x2022a062 = htobe16(0x4e20);
  *(uint32_t*)0x2022a064 = 0;
  *(uint8_t*)0x2022a068 = 0xfe;
  *(uint8_t*)0x2022a069 = 0x80;
  *(uint8_t*)0x2022a06a = 0;
  *(uint8_t*)0x2022a06b = 0;
  *(uint8_t*)0x2022a06c = 0;
  *(uint8_t*)0x2022a06d = 0;
  *(uint8_t*)0x2022a06e = 0;
  *(uint8_t*)0x2022a06f = 0;
  *(uint8_t*)0x2022a070 = 0;
  *(uint8_t*)0x2022a071 = 0;
  *(uint8_t*)0x2022a072 = 0;
  *(uint8_t*)0x2022a073 = 0;
  *(uint8_t*)0x2022a074 = 0;
  *(uint8_t*)0x2022a075 = 0;
  *(uint8_t*)0x2022a076 = 0;
  *(uint8_t*)0x2022a077 = 0xaa;
  *(uint32_t*)0x2022a078 = 0;
  syscall(__NR_sendmsg, r[0], 0x208befc8, 0);
}

int main()
{
  char* cwd = get_current_dir_name();
  for (;;) {
    if (chdir(cwd))
      fail("failed to chdir");
    use_temporary_dir();
    loop();
  }
}
