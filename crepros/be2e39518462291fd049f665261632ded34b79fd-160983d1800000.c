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

#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_mkdir
#define __NR_mkdir 39
#endif
#ifndef __NR_mount
#define __NR_mount 21
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#ifndef __NR_sendmsg
#define __NR_sendmsg 370
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[1];
void test()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  memcpy((void*)0x20885ff8, "./file0", 8);
  syscall(__NR_mkdir, 0x20885ff8, 0);
  memcpy((void*)0x20527ff8, "./file0", 8);
  memcpy((void*)0x20a17964, "./file0", 8);
  memcpy((void*)0x20014000, "proc", 5);
  syscall(__NR_mount, 0x20527ff8, 0x20a17964, 0x20014000, 0, 0x20fc9000);
  r[0] = syscall(__NR_socket, 0xf, 3, 2);
  *(uint32_t*)0x208befc8 = 0;
  *(uint32_t*)0x208befcc = 0;
  *(uint32_t*)0x208befd0 = 0x208feff0;
  *(uint32_t*)0x208befd4 = 1;
  *(uint32_t*)0x208befd8 = 0;
  *(uint32_t*)0x208befdc = 0;
  *(uint32_t*)0x208befe0 = 0;
  *(uint32_t*)0x208feff0 = 0x20f54f80;
  *(uint32_t*)0x208feff4 = 0x80;
  *(uint8_t*)0x20f54f80 = 2;
  *(uint8_t*)0x20f54f81 = 3;
  *(uint8_t*)0x20f54f82 = 0;
  *(uint8_t*)0x20f54f83 = 9;
  *(uint16_t*)0x20f54f84 = 0x10;
  *(uint16_t*)0x20f54f86 = 0;
  *(uint32_t*)0x20f54f88 = 0;
  *(uint32_t*)0x20f54f8c = 0;
  *(uint16_t*)0x20f54f90 = 2;
  *(uint16_t*)0x20f54f92 = 0x13;
  *(uint8_t*)0x20f54f94 = 2;
  *(uint8_t*)0x20f54f95 = 0;
  *(uint16_t*)0x20f54f96 = 0;
  *(uint32_t*)0x20f54f98 = 0;
  *(uint32_t*)0x20f54f9c = 0;
  *(uint16_t*)0x20f54fa0 = 5;
  *(uint16_t*)0x20f54fa2 = 6;
  *(uint8_t*)0x20f54fa4 = 0;
  *(uint8_t*)0x20f54fa5 = 0;
  *(uint16_t*)0x20f54fa6 = 0;
  *(uint16_t*)0x20f54fa8 = 0xa;
  *(uint16_t*)0x20f54faa = htobe16(0x4e20);
  *(uint32_t*)0x20f54fac = 0;
  *(uint64_t*)0x20f54fb0 = htobe64(0);
  *(uint64_t*)0x20f54fb8 = htobe64(1);
  *(uint32_t*)0x20f54fc0 = 0;
  *(uint16_t*)0x20f54fc8 = 2;
  *(uint16_t*)0x20f54fca = 1;
  *(uint32_t*)0x20f54fcc = htobe32(0);
  *(uint8_t*)0x20f54fd0 = 0;
  *(uint8_t*)0x20f54fd1 = 0;
  *(uint8_t*)0x20f54fd2 = 0;
  *(uint8_t*)0x20f54fd3 = 2;
  *(uint32_t*)0x20f54fd4 = 0;
  *(uint16_t*)0x20f54fd8 = 5;
  *(uint16_t*)0x20f54fda = 5;
  *(uint8_t*)0x20f54fdc = 0;
  *(uint8_t*)0x20f54fdd = 0;
  *(uint16_t*)0x20f54fde = 0;
  *(uint16_t*)0x20f54fe0 = 0xa;
  *(uint16_t*)0x20f54fe2 = htobe16(0x4e20);
  *(uint32_t*)0x20f54fe4 = 0;
  *(uint8_t*)0x20f54fe8 = 0xfe;
  *(uint8_t*)0x20f54fe9 = 0x80;
  *(uint8_t*)0x20f54fea = 0;
  *(uint8_t*)0x20f54feb = 0;
  *(uint8_t*)0x20f54fec = 0;
  *(uint8_t*)0x20f54fed = 0;
  *(uint8_t*)0x20f54fee = 0;
  *(uint8_t*)0x20f54fef = 0;
  *(uint8_t*)0x20f54ff0 = 0;
  *(uint8_t*)0x20f54ff1 = 0;
  *(uint8_t*)0x20f54ff2 = 0;
  *(uint8_t*)0x20f54ff3 = 0;
  *(uint8_t*)0x20f54ff4 = 0;
  *(uint8_t*)0x20f54ff5 = 0;
  *(uint8_t*)0x20f54ff6 = 0;
  *(uint8_t*)0x20f54ff7 = 0xbb;
  *(uint32_t*)0x20f54ff8 = 0;
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
