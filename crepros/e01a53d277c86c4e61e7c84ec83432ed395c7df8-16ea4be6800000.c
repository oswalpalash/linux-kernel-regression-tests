// https://syzkaller.appspot.com/bug?id=e2dc9aea0465d1eea101bb24cb463e2a7efe7d17
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
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
      fail("clone failed");
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

long r[55];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    r[1] = syscall(__NR_socket, 0x26ul, 0x5ul, 0x0ul);
    break;
  case 2:
    *(uint16_t*)0x20bcf000 = (uint16_t)0x26;
    memcpy((void*)0x20bcf002,
           "\x73\x6b\x63\x69\x70\x68\x65\x72\x00\x00\x00\x00\x00\x00",
           14);
    *(uint32_t*)0x20bcf010 = (uint32_t)0x0;
    *(uint32_t*)0x20bcf014 = (uint32_t)0x0;
    memcpy((void*)0x20bcf018,
           "\x65\x63\x62\x28\x74\x77\x6f\x66\x69\x73\x68\x29\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00",
           64);
    r[7] = syscall(__NR_bind, r[1], 0x20bcf000ul, 0x58ul);
    break;
  case 3:
    memcpy((void*)0x204be000, "\xe5\xfe\xb5\xbf\x91\xaa\x78\x55\xfb\xcd"
                              "\xfe\xf1\xdd\xb4\x21\x13",
           16);
    r[9] = syscall(__NR_setsockopt, r[1], 0x117ul, 0x1ul, 0x204be000ul,
                   0x10ul);
    break;
  case 4:
    r[10] = syscall(__NR_accept, r[1], 0x0ul, 0x0ul);
    break;
  case 5:
    *(uint64_t*)0x20a90fc8 = (uint64_t)0x20eee000;
    *(uint32_t*)0x20a90fd0 = (uint32_t)0x58;
    *(uint64_t*)0x20a90fd8 = (uint64_t)0x204e0fe0;
    *(uint64_t*)0x20a90fe0 = (uint64_t)0x2;
    *(uint64_t*)0x20a90fe8 = (uint64_t)0x20475000;
    *(uint64_t*)0x20a90ff0 = (uint64_t)0x0;
    *(uint32_t*)0x20a90ff8 = (uint32_t)0x10000;
    *(uint64_t*)0x204e0fe0 = (uint64_t)0x20748fc9;
    *(uint64_t*)0x204e0fe8 = (uint64_t)0x0;
    *(uint64_t*)0x204e0ff0 = (uint64_t)0x2006e000;
    *(uint64_t*)0x204e0ff8 = (uint64_t)0xda;
    r[22] = syscall(__NR_recvmsg, r[10], 0x20a90fc8ul, 0x40ul);
    break;
  case 6:
    *(uint64_t*)0x20889fc8 = (uint64_t)0x0;
    *(uint32_t*)0x20889fd0 = (uint32_t)0x0;
    *(uint64_t*)0x20889fd8 = (uint64_t)0x203f6fd0;
    *(uint64_t*)0x20889fe0 = (uint64_t)0x3;
    *(uint64_t*)0x20889fe8 = (uint64_t)0x204ecfa0;
    *(uint64_t*)0x20889ff0 = (uint64_t)0x60;
    *(uint32_t*)0x20889ff8 = (uint32_t)0x810;
    *(uint64_t*)0x203f6fd0 = (uint64_t)0x20801000;
    *(uint64_t*)0x203f6fd8 = (uint64_t)0x0;
    *(uint64_t*)0x203f6fe0 = (uint64_t)0x209a7000;
    *(uint64_t*)0x203f6fe8 = (uint64_t)0x16;
    *(uint64_t*)0x203f6ff0 = (uint64_t)0x20b9a000;
    *(uint64_t*)0x203f6ff8 = (uint64_t)0x2b;
    memcpy((void*)0x209a7000, "\x53\x95\x0d\x2e\x35\xc9\xb7\x65\x31\xb6"
                              "\x1a\x6b\x2a\x2a\x37\x77\xa6\xfd\xb3\x57"
                              "\x81\x6d",
           22);
    memcpy((void*)0x20b9a000, "\x0b\xd8\xc7\xda\x7e\xb8\x78\x8d\xa1\x5b"
                              "\xa2\x8c\x1f\x9f\xbb\x5b\xa7\xa0\xa9\x09"
                              "\xbe\x07\xaf\xe6\xf7\x51\xf5\x7c\x9b\xcf"
                              "\xe8\x53\xb4\x21\x33\xc9\xc1\x21\x54\x95"
                              "\x1e\xc6\x6c",
           43);
    *(uint64_t*)0x204ecfa0 = (uint64_t)0x18;
    *(uint32_t*)0x204ecfa8 = (uint32_t)0x117;
    *(uint32_t*)0x204ecfac = (uint32_t)0x4;
    *(uint32_t*)0x204ecfb0 = (uint32_t)0x407;
    *(uint64_t*)0x204ecfb8 = (uint64_t)0x18;
    *(uint32_t*)0x204ecfc0 = (uint32_t)0x117;
    *(uint32_t*)0x204ecfc4 = (uint32_t)0x4;
    *(uint32_t*)0x204ecfc8 = (uint32_t)0x9;
    *(uint64_t*)0x204ecfd0 = (uint64_t)0x18;
    *(uint32_t*)0x204ecfd8 = (uint32_t)0x117;
    *(uint32_t*)0x204ecfdc = (uint32_t)0x3;
    *(uint32_t*)0x204ecfe0 = (uint32_t)0x1;
    *(uint64_t*)0x204ecfe8 = (uint64_t)0x18;
    *(uint32_t*)0x204ecff0 = (uint32_t)0x117;
    *(uint32_t*)0x204ecff4 = (uint32_t)0x4;
    *(uint32_t*)0x204ecff8 = (uint32_t)0x6;
    r[54] = syscall(__NR_sendmmsg, r[10], 0x20889fc8ul, 0x1ul,
                    0x20000001ul);
    break;
  }
  return 0;
}

void test()
{
  long i;
  pthread_t th[14];

  memset(r, -1, sizeof(r));
  srand(getpid());
  for (i = 0; i < 7; i++) {
    pthread_create(&th[i], 0, thr, (void*)i);
    usleep(rand() % 10000);
  }
  for (i = 0; i < 7; i++) {
    pthread_create(&th[7 + i], 0, thr, (void*)i);
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
      loop();
      return 0;
    }
  }
  sleep(1000000);
  return 0;
}
