// https://syzkaller.appspot.com/bug?id=13c7434436faf91b6cf112289188dcde8ff0f172
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <linux/net.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/socket.h>
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

struct ipt_getinfo {
  char name[32];
  unsigned int valid_hooks;
  unsigned int hook_entry[5];
  unsigned int underflow[5];
  unsigned int num_entries;
  unsigned int size;
};

struct ipt_get_entries {
  char name[32];
  unsigned int size;
  void* entrytable[1024 / sizeof(void*)];
};

struct xt_counters {
  uint64_t pcnt, bcnt;
};

struct ipt_replace {
  char name[32];
  unsigned int valid_hooks;
  unsigned int num_entries;
  unsigned int size;
  unsigned int hook_entry[5];
  unsigned int underflow[5];
  unsigned int num_counters;
  struct xt_counters* counters;
  char entrytable[1024];
};

struct ipt_table_desc {
  const char* name;
  struct ipt_getinfo info;
  struct ipt_get_entries entries;
  struct ipt_replace replace;
  struct xt_counters counters[10];
};

static struct ipt_table_desc ipv4_tables[] = {
    {.name = "filter"}, {.name = "nat"},      {.name = "mangle"},
    {.name = "raw"},    {.name = "security"},
};

#define IPT_BASE_CTL 64
#define IPT_SO_SET_REPLACE (IPT_BASE_CTL)
#define IPT_SO_GET_INFO (IPT_BASE_CTL)
#define IPT_SO_GET_ENTRIES (IPT_BASE_CTL + 1)

static void checkpoint_net_namespace(void)
{
  socklen_t optlen;
  unsigned i;
  int fd;

  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1)
    fail("socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)");
  for (i = 0; i < sizeof(ipv4_tables) / sizeof(ipv4_tables[0]); i++) {
    struct ipt_table_desc* table = &ipv4_tables[i];
    strcpy(table->info.name, table->name);
    strcpy(table->entries.name, table->name);
    strcpy(table->replace.name, table->name);
    optlen = sizeof(table->info);
    if (getsockopt(fd, SOL_IP, IPT_SO_GET_INFO, &table->info, &optlen)) {
      switch (errno) {
      case EPERM:
      case ENOENT:
      case ENOPROTOOPT:
        continue;
      }
      fail("getsockopt(IPT_SO_GET_INFO)");
    }
    if (table->info.size > sizeof(table->entries.entrytable))
      fail("table size is too large: %u", table->info.size);
    if (table->info.num_entries >
        sizeof(table->counters) / sizeof(table->counters[0]))
      fail("too many counters: %u", table->info.num_entries);
    table->entries.size = table->info.size;
    optlen = sizeof(table->entries) - sizeof(table->entries.entrytable) +
             table->info.size;
    if (getsockopt(fd, SOL_IP, IPT_SO_GET_ENTRIES, &table->entries, &optlen))
      fail("getsockopt(IPT_SO_GET_ENTRIES)");
    table->replace.valid_hooks = table->info.valid_hooks;
    table->replace.num_entries = table->info.num_entries;
    table->replace.counters = table->counters;
    table->replace.size = table->info.size;
    memcpy(table->replace.hook_entry, table->info.hook_entry,
           sizeof(table->replace.hook_entry));
    memcpy(table->replace.underflow, table->info.underflow,
           sizeof(table->replace.underflow));
    memcpy(table->replace.entrytable, table->entries.entrytable,
           table->info.size);
  }
  close(fd);
}

static void reset_net_namespace(void)
{
  struct ipt_get_entries entries;
  struct ipt_getinfo info;
  socklen_t optlen;
  unsigned i;
  int fd;

  memset(&info, 0, sizeof(info));
  memset(&entries, 0, sizeof(entries));
  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1)
    fail("socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)");
  for (i = 0; i < sizeof(ipv4_tables) / sizeof(ipv4_tables[0]); i++) {
    struct ipt_table_desc* table = &ipv4_tables[i];
    if (table->info.valid_hooks == 0)
      continue;
    strcpy(info.name, table->name);
    optlen = sizeof(info);
    if (getsockopt(fd, SOL_IP, IPT_SO_GET_INFO, &info, &optlen))
      fail("getsockopt(IPT_SO_GET_INFO)");
    if (memcmp(&table->info, &info, sizeof(table->info)) == 0) {
      strcpy(entries.name, table->name);
      entries.size = table->info.size;
      optlen = sizeof(entries) - sizeof(entries.entrytable) + entries.size;
      if (getsockopt(fd, SOL_IP, IPT_SO_GET_ENTRIES, &entries, &optlen))
        fail("getsockopt(IPT_SO_GET_ENTRIES)");
      if (memcmp(&table->entries, &entries, optlen) == 0)
        continue;
    }
    table->replace.num_counters = info.num_entries;
    optlen = sizeof(table->replace) - sizeof(table->replace.entrytable) +
             table->replace.size;
    if (setsockopt(fd, SOL_IP, IPT_SO_SET_REPLACE, &table->replace, optlen))
      fail("setsockopt(IPT_SO_SET_REPLACE)");
  }
  close(fd);
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
  checkpoint_net_namespace();
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
    reset_net_namespace();
  }
}

long r[2];
void test()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  *(uint16_t*)0x2027f000 = 0;
  *(uint64_t*)0x2027f008 = 0x2039cff8;
  syscall(__NR_setsockopt, -1, 1, 0x1a, 0x2027f000, 0x10);
  r[0] = syscall(__NR_socket, 2, 2, 0);
  syscall(__NR_setsockopt, r[0], 0, 0x2c, 0x20b2b000, 0);
  r[1] = syscall(__NR_socket, 2, 0x80005, 0);
  memcpy((void*)0x20013c90, "\x66\x69\x6c\x74\x65\x72\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20013cb0 = 0xe;
  *(uint32_t*)0x20013cb4 = 4;
  *(uint32_t*)0x20013cb8 = 0x298;
  *(uint32_t*)0x20013cbc = -1;
  *(uint32_t*)0x20013cc0 = 0;
  *(uint32_t*)0x20013cc4 = 0;
  *(uint32_t*)0x20013cc8 = 0;
  *(uint32_t*)0x20013ccc = -1;
  *(uint32_t*)0x20013cd0 = -1;
  *(uint32_t*)0x20013cd4 = 0;
  *(uint32_t*)0x20013cd8 = 0;
  *(uint32_t*)0x20013cdc = 0;
  *(uint32_t*)0x20013ce0 = -1;
  *(uint32_t*)0x20013ce4 = 4;
  *(uint64_t*)0x20013ce8 = 0x20001000;
  *(uint8_t*)0x20013cf0 = 0;
  *(uint8_t*)0x20013cf1 = 0;
  *(uint8_t*)0x20013cf2 = 0;
  *(uint8_t*)0x20013cf3 = 0;
  *(uint8_t*)0x20013cf4 = 0;
  *(uint8_t*)0x20013cf5 = 0;
  *(uint8_t*)0x20013cf6 = 0;
  *(uint8_t*)0x20013cf7 = 0;
  *(uint8_t*)0x20013cf8 = 0;
  *(uint8_t*)0x20013cf9 = 0;
  *(uint8_t*)0x20013cfa = 0;
  *(uint8_t*)0x20013cfb = 0;
  *(uint8_t*)0x20013cfc = 0;
  *(uint8_t*)0x20013cfd = 0;
  *(uint8_t*)0x20013cfe = 0;
  *(uint8_t*)0x20013cff = 0;
  *(uint8_t*)0x20013d00 = 0;
  *(uint8_t*)0x20013d01 = 0;
  *(uint8_t*)0x20013d02 = 0;
  *(uint8_t*)0x20013d03 = 0;
  *(uint8_t*)0x20013d04 = 0;
  *(uint8_t*)0x20013d05 = 0;
  *(uint8_t*)0x20013d06 = 0;
  *(uint8_t*)0x20013d07 = 0;
  *(uint8_t*)0x20013d08 = 0;
  *(uint8_t*)0x20013d09 = 0;
  *(uint8_t*)0x20013d0a = 0;
  *(uint8_t*)0x20013d0b = 0;
  *(uint8_t*)0x20013d0c = 0;
  *(uint8_t*)0x20013d0d = 0;
  *(uint8_t*)0x20013d0e = 0;
  *(uint8_t*)0x20013d0f = 0;
  *(uint8_t*)0x20013d10 = 0;
  *(uint8_t*)0x20013d11 = 0;
  *(uint8_t*)0x20013d12 = 0;
  *(uint8_t*)0x20013d13 = 0;
  *(uint8_t*)0x20013d14 = 0;
  *(uint8_t*)0x20013d15 = 0;
  *(uint8_t*)0x20013d16 = 0;
  *(uint8_t*)0x20013d17 = 0;
  *(uint8_t*)0x20013d18 = 0;
  *(uint8_t*)0x20013d19 = 0;
  *(uint8_t*)0x20013d1a = 0;
  *(uint8_t*)0x20013d1b = 0;
  *(uint8_t*)0x20013d1c = 0;
  *(uint8_t*)0x20013d1d = 0;
  *(uint8_t*)0x20013d1e = 0;
  *(uint8_t*)0x20013d1f = 0;
  *(uint8_t*)0x20013d20 = 0;
  *(uint8_t*)0x20013d21 = 0;
  *(uint8_t*)0x20013d22 = 0;
  *(uint8_t*)0x20013d23 = 0;
  *(uint8_t*)0x20013d24 = 0;
  *(uint8_t*)0x20013d25 = 0;
  *(uint8_t*)0x20013d26 = 0;
  *(uint8_t*)0x20013d27 = 0;
  *(uint8_t*)0x20013d28 = 0;
  *(uint8_t*)0x20013d29 = 0;
  *(uint8_t*)0x20013d2a = 0;
  *(uint8_t*)0x20013d2b = 0;
  *(uint8_t*)0x20013d2c = 0;
  *(uint8_t*)0x20013d2d = 0;
  *(uint8_t*)0x20013d2e = 0;
  *(uint8_t*)0x20013d2f = 0;
  *(uint8_t*)0x20013d30 = 0;
  *(uint8_t*)0x20013d31 = 0;
  *(uint8_t*)0x20013d32 = 0;
  *(uint8_t*)0x20013d33 = 0;
  *(uint8_t*)0x20013d34 = 0;
  *(uint8_t*)0x20013d35 = 0;
  *(uint8_t*)0x20013d36 = 0;
  *(uint8_t*)0x20013d37 = 0;
  *(uint8_t*)0x20013d38 = 0;
  *(uint8_t*)0x20013d39 = 0;
  *(uint8_t*)0x20013d3a = 0;
  *(uint8_t*)0x20013d3b = 0;
  *(uint8_t*)0x20013d3c = 0;
  *(uint8_t*)0x20013d3d = 0;
  *(uint8_t*)0x20013d3e = 0;
  *(uint8_t*)0x20013d3f = 0;
  *(uint8_t*)0x20013d40 = 0;
  *(uint8_t*)0x20013d41 = 0;
  *(uint8_t*)0x20013d42 = 0;
  *(uint8_t*)0x20013d43 = 0;
  *(uint32_t*)0x20013d44 = 0;
  *(uint16_t*)0x20013d48 = 0x70;
  *(uint16_t*)0x20013d4a = 0x98;
  *(uint32_t*)0x20013d4c = 0;
  *(uint64_t*)0x20013d50 = 0;
  *(uint64_t*)0x20013d58 = 0;
  *(uint16_t*)0x20013d60 = 0x28;
  memcpy((void*)0x20013d62, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20013d7f = 0;
  *(uint32_t*)0x20013d80 = 0xfffffffe;
  *(uint8_t*)0x20013d88 = 0;
  *(uint8_t*)0x20013d89 = 0;
  *(uint8_t*)0x20013d8a = 0;
  *(uint8_t*)0x20013d8b = 0;
  *(uint8_t*)0x20013d8c = 0;
  *(uint8_t*)0x20013d8d = 0;
  *(uint8_t*)0x20013d8e = 0;
  *(uint8_t*)0x20013d8f = 0;
  *(uint8_t*)0x20013d90 = 0;
  *(uint8_t*)0x20013d91 = 0;
  *(uint8_t*)0x20013d92 = 0;
  *(uint8_t*)0x20013d93 = 0;
  *(uint8_t*)0x20013d94 = 0;
  *(uint8_t*)0x20013d95 = 0;
  *(uint8_t*)0x20013d96 = 0;
  *(uint8_t*)0x20013d97 = 0;
  *(uint8_t*)0x20013d98 = 0;
  *(uint8_t*)0x20013d99 = 0;
  *(uint8_t*)0x20013d9a = 0;
  *(uint8_t*)0x20013d9b = 0;
  *(uint8_t*)0x20013d9c = 0;
  *(uint8_t*)0x20013d9d = 0;
  *(uint8_t*)0x20013d9e = 0;
  *(uint8_t*)0x20013d9f = 0;
  *(uint8_t*)0x20013da0 = 0;
  *(uint8_t*)0x20013da1 = 0;
  *(uint8_t*)0x20013da2 = 0;
  *(uint8_t*)0x20013da3 = 0;
  *(uint8_t*)0x20013da4 = 0;
  *(uint8_t*)0x20013da5 = 0;
  *(uint8_t*)0x20013da6 = 0;
  *(uint8_t*)0x20013da7 = 0;
  *(uint8_t*)0x20013da8 = 0;
  *(uint8_t*)0x20013da9 = 0;
  *(uint8_t*)0x20013daa = 0;
  *(uint8_t*)0x20013dab = 0;
  *(uint8_t*)0x20013dac = 0;
  *(uint8_t*)0x20013dad = 0;
  *(uint8_t*)0x20013dae = 0;
  *(uint8_t*)0x20013daf = 0;
  *(uint8_t*)0x20013db0 = 0;
  *(uint8_t*)0x20013db1 = 0;
  *(uint8_t*)0x20013db2 = 0;
  *(uint8_t*)0x20013db3 = 0;
  *(uint8_t*)0x20013db4 = 0;
  *(uint8_t*)0x20013db5 = 0;
  *(uint8_t*)0x20013db6 = 0;
  *(uint8_t*)0x20013db7 = 0;
  *(uint8_t*)0x20013db8 = 0;
  *(uint8_t*)0x20013db9 = 0;
  *(uint8_t*)0x20013dba = 0;
  *(uint8_t*)0x20013dbb = 0;
  *(uint8_t*)0x20013dbc = 0;
  *(uint8_t*)0x20013dbd = 0;
  *(uint8_t*)0x20013dbe = 0;
  *(uint8_t*)0x20013dbf = 0;
  *(uint8_t*)0x20013dc0 = 0;
  *(uint8_t*)0x20013dc1 = 0;
  *(uint8_t*)0x20013dc2 = 0;
  *(uint8_t*)0x20013dc3 = 0;
  *(uint8_t*)0x20013dc4 = 0;
  *(uint8_t*)0x20013dc5 = 0;
  *(uint8_t*)0x20013dc6 = 0;
  *(uint8_t*)0x20013dc7 = 0;
  *(uint8_t*)0x20013dc8 = 0;
  *(uint8_t*)0x20013dc9 = 0;
  *(uint8_t*)0x20013dca = 0;
  *(uint8_t*)0x20013dcb = 0;
  *(uint8_t*)0x20013dcc = 0;
  *(uint8_t*)0x20013dcd = 0;
  *(uint8_t*)0x20013dce = 0;
  *(uint8_t*)0x20013dcf = 0;
  *(uint8_t*)0x20013dd0 = 0;
  *(uint8_t*)0x20013dd1 = 0;
  *(uint8_t*)0x20013dd2 = 0;
  *(uint8_t*)0x20013dd3 = 0;
  *(uint8_t*)0x20013dd4 = 0;
  *(uint8_t*)0x20013dd5 = 0;
  *(uint8_t*)0x20013dd6 = 0;
  *(uint8_t*)0x20013dd7 = 0;
  *(uint8_t*)0x20013dd8 = 0;
  *(uint8_t*)0x20013dd9 = 0;
  *(uint8_t*)0x20013dda = 0;
  *(uint8_t*)0x20013ddb = 0;
  *(uint32_t*)0x20013ddc = 0;
  *(uint16_t*)0x20013de0 = 0x70;
  *(uint16_t*)0x20013de2 = 0x98;
  *(uint32_t*)0x20013de4 = 0;
  *(uint64_t*)0x20013de8 = 0;
  *(uint64_t*)0x20013df0 = 0;
  *(uint16_t*)0x20013df8 = 0x28;
  memcpy((void*)0x20013dfa, "\x52\x45\x4a\x45\x43\x54\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20013e17 = 0;
  *(uint32_t*)0x20013e18 = 0;
  *(uint32_t*)0x20013e20 = htobe32(-1);
  *(uint8_t*)0x20013e24 = 0xac;
  *(uint8_t*)0x20013e25 = 0x14;
  *(uint8_t*)0x20013e26 = 0;
  *(uint8_t*)0x20013e27 = 0xaa;
  *(uint32_t*)0x20013e28 = htobe32(0);
  *(uint32_t*)0x20013e2c = htobe32(-1);
  memcpy((void*)0x20013e30,
         "\x73\x69\x74\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  memcpy((void*)0x20013e40,
         "\x65\x71\x6c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  *(uint8_t*)0x20013e50 = 0;
  *(uint8_t*)0x20013e51 = 0;
  *(uint8_t*)0x20013e52 = 0;
  *(uint8_t*)0x20013e53 = 0;
  *(uint8_t*)0x20013e54 = 0;
  *(uint8_t*)0x20013e55 = 0;
  *(uint8_t*)0x20013e56 = 0;
  *(uint8_t*)0x20013e57 = 0;
  *(uint8_t*)0x20013e58 = 0;
  *(uint8_t*)0x20013e59 = 0;
  *(uint8_t*)0x20013e5a = 0;
  *(uint8_t*)0x20013e5b = 0;
  *(uint8_t*)0x20013e5c = 0;
  *(uint8_t*)0x20013e5d = 0;
  *(uint8_t*)0x20013e5e = 0;
  *(uint8_t*)0x20013e5f = 0;
  *(uint8_t*)0x20013e60 = 0;
  *(uint8_t*)0x20013e61 = 0;
  *(uint8_t*)0x20013e62 = 0;
  *(uint8_t*)0x20013e63 = 0;
  *(uint8_t*)0x20013e64 = 0;
  *(uint8_t*)0x20013e65 = 0;
  *(uint8_t*)0x20013e66 = 0;
  *(uint8_t*)0x20013e67 = 0;
  *(uint8_t*)0x20013e68 = 0;
  *(uint8_t*)0x20013e69 = 0;
  *(uint8_t*)0x20013e6a = 0;
  *(uint8_t*)0x20013e6b = 0;
  *(uint8_t*)0x20013e6c = 0;
  *(uint8_t*)0x20013e6d = 0;
  *(uint8_t*)0x20013e6e = 0;
  *(uint8_t*)0x20013e6f = 0;
  *(uint16_t*)0x20013e70 = 0;
  *(uint8_t*)0x20013e72 = 0;
  *(uint8_t*)0x20013e73 = 0;
  *(uint32_t*)0x20013e74 = 0;
  *(uint16_t*)0x20013e78 = 0x70;
  *(uint16_t*)0x20013e7a = 0xd0;
  *(uint32_t*)0x20013e7c = 0;
  *(uint64_t*)0x20013e80 = 0;
  *(uint64_t*)0x20013e88 = 0;
  *(uint16_t*)0x20013e90 = 0x60;
  memcpy((void*)0x20013e92, "\x43\x4c\x55\x53\x54\x45\x52\x49\x50\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20013eaf = 0;
  *(uint32_t*)0x20013eb0 = 1;
  *(uint8_t*)0x20013eb4 = 1;
  *(uint8_t*)0x20013eb5 = 0x80;
  *(uint8_t*)0x20013eb6 = 0xc2;
  *(uint8_t*)0x20013eb7 = 0;
  *(uint8_t*)0x20013eb8 = 0;
  *(uint8_t*)0x20013eb9 = 0;
  *(uint16_t*)0x20013eba = 0;
  *(uint16_t*)0x20013ebc = 0;
  *(uint16_t*)0x20013ebe = 0;
  *(uint16_t*)0x20013ec0 = 0;
  *(uint16_t*)0x20013ec2 = 0;
  *(uint16_t*)0x20013ec4 = 0;
  *(uint16_t*)0x20013ec6 = 0;
  *(uint16_t*)0x20013ec8 = 0;
  *(uint16_t*)0x20013eca = 0;
  *(uint16_t*)0x20013ecc = 0;
  *(uint16_t*)0x20013ece = 0;
  *(uint16_t*)0x20013ed0 = 0;
  *(uint16_t*)0x20013ed2 = 0;
  *(uint16_t*)0x20013ed4 = 0;
  *(uint16_t*)0x20013ed6 = 0;
  *(uint16_t*)0x20013ed8 = 0;
  *(uint16_t*)0x20013eda = 0;
  *(uint16_t*)0x20013edc = 0;
  *(uint32_t*)0x20013ee0 = 0;
  *(uint32_t*)0x20013ee4 = 0;
  *(uint64_t*)0x20013ee8 = 0;
  *(uint8_t*)0x20013ef0 = 0;
  *(uint8_t*)0x20013ef1 = 0;
  *(uint8_t*)0x20013ef2 = 0;
  *(uint8_t*)0x20013ef3 = 0;
  *(uint8_t*)0x20013ef4 = 0;
  *(uint8_t*)0x20013ef5 = 0;
  *(uint8_t*)0x20013ef6 = 0;
  *(uint8_t*)0x20013ef7 = 0;
  *(uint8_t*)0x20013ef8 = 0;
  *(uint8_t*)0x20013ef9 = 0;
  *(uint8_t*)0x20013efa = 0;
  *(uint8_t*)0x20013efb = 0;
  *(uint8_t*)0x20013efc = 0;
  *(uint8_t*)0x20013efd = 0;
  *(uint8_t*)0x20013efe = 0;
  *(uint8_t*)0x20013eff = 0;
  *(uint8_t*)0x20013f00 = 0;
  *(uint8_t*)0x20013f01 = 0;
  *(uint8_t*)0x20013f02 = 0;
  *(uint8_t*)0x20013f03 = 0;
  *(uint8_t*)0x20013f04 = 0;
  *(uint8_t*)0x20013f05 = 0;
  *(uint8_t*)0x20013f06 = 0;
  *(uint8_t*)0x20013f07 = 0;
  *(uint8_t*)0x20013f08 = 0;
  *(uint8_t*)0x20013f09 = 0;
  *(uint8_t*)0x20013f0a = 0;
  *(uint8_t*)0x20013f0b = 0;
  *(uint8_t*)0x20013f0c = 0;
  *(uint8_t*)0x20013f0d = 0;
  *(uint8_t*)0x20013f0e = 0;
  *(uint8_t*)0x20013f0f = 0;
  *(uint8_t*)0x20013f10 = 0;
  *(uint8_t*)0x20013f11 = 0;
  *(uint8_t*)0x20013f12 = 0;
  *(uint8_t*)0x20013f13 = 0;
  *(uint8_t*)0x20013f14 = 0;
  *(uint8_t*)0x20013f15 = 0;
  *(uint8_t*)0x20013f16 = 0;
  *(uint8_t*)0x20013f17 = 0;
  *(uint8_t*)0x20013f18 = 0;
  *(uint8_t*)0x20013f19 = 0;
  *(uint8_t*)0x20013f1a = 0;
  *(uint8_t*)0x20013f1b = 0;
  *(uint8_t*)0x20013f1c = 0;
  *(uint8_t*)0x20013f1d = 0;
  *(uint8_t*)0x20013f1e = 0;
  *(uint8_t*)0x20013f1f = 0;
  *(uint8_t*)0x20013f20 = 0;
  *(uint8_t*)0x20013f21 = 0;
  *(uint8_t*)0x20013f22 = 0;
  *(uint8_t*)0x20013f23 = 0;
  *(uint8_t*)0x20013f24 = 0;
  *(uint8_t*)0x20013f25 = 0;
  *(uint8_t*)0x20013f26 = 0;
  *(uint8_t*)0x20013f27 = 0;
  *(uint8_t*)0x20013f28 = 0;
  *(uint8_t*)0x20013f29 = 0;
  *(uint8_t*)0x20013f2a = 0;
  *(uint8_t*)0x20013f2b = 0;
  *(uint8_t*)0x20013f2c = 0;
  *(uint8_t*)0x20013f2d = 0;
  *(uint8_t*)0x20013f2e = 0;
  *(uint8_t*)0x20013f2f = 0;
  *(uint8_t*)0x20013f30 = 0;
  *(uint8_t*)0x20013f31 = 0;
  *(uint8_t*)0x20013f32 = 0;
  *(uint8_t*)0x20013f33 = 0;
  *(uint8_t*)0x20013f34 = 0;
  *(uint8_t*)0x20013f35 = 0;
  *(uint8_t*)0x20013f36 = 0;
  *(uint8_t*)0x20013f37 = 0;
  *(uint8_t*)0x20013f38 = 0;
  *(uint8_t*)0x20013f39 = 0;
  *(uint8_t*)0x20013f3a = 0;
  *(uint8_t*)0x20013f3b = 0;
  *(uint8_t*)0x20013f3c = 0;
  *(uint8_t*)0x20013f3d = 0;
  *(uint8_t*)0x20013f3e = 0;
  *(uint8_t*)0x20013f3f = 0;
  *(uint8_t*)0x20013f40 = 0;
  *(uint8_t*)0x20013f41 = 0;
  *(uint8_t*)0x20013f42 = 0;
  *(uint8_t*)0x20013f43 = 0;
  *(uint32_t*)0x20013f44 = 0;
  *(uint16_t*)0x20013f48 = 0x70;
  *(uint16_t*)0x20013f4a = 0x98;
  *(uint32_t*)0x20013f4c = 0;
  *(uint64_t*)0x20013f50 = 0;
  *(uint64_t*)0x20013f58 = 0;
  *(uint16_t*)0x20013f60 = 0x28;
  memcpy((void*)0x20013f62, "\x52\x45\x4a\x45\x43\x54\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20013f7f = 0;
  *(uint32_t*)0x20013f80 = 0;
  *(uint64_t*)0x20001000 = 0;
  *(uint64_t*)0x20001008 = 0;
  *(uint64_t*)0x20001010 = 0;
  *(uint64_t*)0x20001018 = 0;
  *(uint64_t*)0x20001020 = 0;
  *(uint64_t*)0x20001028 = 0;
  *(uint64_t*)0x20001030 = 0;
  *(uint64_t*)0x20001038 = 0;
  syscall(__NR_setsockopt, r[1], 0, 0x40, 0x20013c90, 0x2f8);
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
