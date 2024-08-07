// https://syzkaller.appspot.com/bug?id=11f60158f34305e6ff1e0db2205247afbc9f9604
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
  syscall(__NR_mmap, 0x20000000, 0xe7a000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 0xa, 2, 0);
  r[1] = syscall(__NR_socket, 0x18, 1, 1);
  *(uint16_t*)0x2014cfd2 = 0x18;
  *(uint32_t*)0x2014cfd4 = 1;
  *(uint32_t*)0x2014cfd8 = 0;
  *(uint32_t*)0x2014cfdc = r[0];
  *(uint16_t*)0x2014cfe0 = 2;
  *(uint16_t*)0x2014cfe2 = htobe16(0x4e22);
  *(uint32_t*)0x2014cfe4 = htobe32(-1);
  *(uint8_t*)0x2014cfe8 = 0;
  *(uint8_t*)0x2014cfe9 = 0;
  *(uint8_t*)0x2014cfea = 0;
  *(uint8_t*)0x2014cfeb = 0;
  *(uint8_t*)0x2014cfec = 0;
  *(uint8_t*)0x2014cfed = 0;
  *(uint8_t*)0x2014cfee = 0;
  *(uint8_t*)0x2014cfef = 0;
  *(uint32_t*)0x2014cff0 = 4;
  *(uint32_t*)0x2014cff4 = 0;
  *(uint32_t*)0x2014cff8 = 3;
  *(uint32_t*)0x2014cffc = 0;
  syscall(__NR_connect, r[1], 0x2014cfd2, 0x2e);
  *(uint64_t*)0x203a6000 = 0x201bc000;
  *(uint32_t*)0x203a6008 = 0x32;
  *(uint64_t*)0x203a6010 = 0x20baffe0;
  *(uint64_t*)0x203a6018 = 2;
  *(uint64_t*)0x203a6020 = 0x20e78000;
  *(uint64_t*)0x203a6028 = 0x80;
  *(uint32_t*)0x203a6030 = 0;
  *(uint32_t*)0x203a6038 = 1;
  *(uint64_t*)0x203a603c = 0x20e78ff2;
  *(uint32_t*)0x203a6044 = 0xe;
  *(uint64_t*)0x203a604c = 0x20e78ff0;
  *(uint64_t*)0x203a6054 = 1;
  *(uint64_t*)0x203a605c = 0x20e78000;
  *(uint64_t*)0x203a6064 = 0x20;
  *(uint32_t*)0x203a606c = 1;
  *(uint32_t*)0x203a6074 = 0x1f;
  *(uint64_t*)0x203a6078 = 0x200fe000;
  *(uint32_t*)0x203a6080 = 0x10;
  *(uint64_t*)0x203a6088 = 0x20e78fb0;
  *(uint64_t*)0x203a6090 = 5;
  *(uint64_t*)0x203a6098 = 0x20e77f00;
  *(uint64_t*)0x203a60a0 = 0x30;
  *(uint32_t*)0x203a60a8 = 0;
  *(uint32_t*)0x203a60b0 = 0x81;
  *(uint64_t*)0x203a60b4 = 0x20e78ff0;
  *(uint32_t*)0x203a60bc = 0x10;
  *(uint64_t*)0x203a60c4 = 0x20e78fe0;
  *(uint64_t*)0x203a60cc = 2;
  *(uint64_t*)0x203a60d4 = 0x20e78000;
  *(uint64_t*)0x203a60dc = 0x80;
  *(uint32_t*)0x203a60e4 = 0x801;
  *(uint32_t*)0x203a60ec = 1;
  *(uint64_t*)0x203a60f0 = 0x20000000;
  *(uint32_t*)0x203a60f8 = 0;
  *(uint64_t*)0x203a6100 = 0x20a73000;
  *(uint64_t*)0x203a6108 = 4;
  *(uint64_t*)0x203a6110 = 0x20e77eb0;
  *(uint64_t*)0x203a6118 = 0x40;
  *(uint32_t*)0x203a6120 = 0x8000;
  *(uint32_t*)0x203a6128 = 2;
  *(uint16_t*)0x201bc000 = 0x18;
  *(uint32_t*)0x201bc002 = 1;
  *(uint32_t*)0x201bc006 = 0;
  *(uint32_t*)0x201bc00a = r[1];
  *(uint16_t*)0x201bc00e = 3;
  *(uint16_t*)0x201bc010 = 0;
  *(uint16_t*)0x201bc012 = 3;
  *(uint16_t*)0x201bc014 = 3;
  *(uint16_t*)0x201bc016 = 0xa;
  *(uint16_t*)0x201bc018 = htobe16(0x4e20);
  *(uint32_t*)0x201bc01a = -1;
  *(uint8_t*)0x201bc01e = 0;
  *(uint8_t*)0x201bc01f = 0;
  *(uint8_t*)0x201bc020 = 0;
  *(uint8_t*)0x201bc021 = 0;
  *(uint8_t*)0x201bc022 = 0;
  *(uint8_t*)0x201bc023 = 0;
  *(uint8_t*)0x201bc024 = 0;
  *(uint8_t*)0x201bc025 = 0;
  *(uint8_t*)0x201bc026 = 0;
  *(uint8_t*)0x201bc027 = 0;
  *(uint8_t*)0x201bc028 = 0;
  *(uint8_t*)0x201bc029 = 0;
  *(uint8_t*)0x201bc02a = 0;
  *(uint8_t*)0x201bc02b = 0;
  *(uint8_t*)0x201bc02c = 0;
  *(uint8_t*)0x201bc02d = 0;
  *(uint32_t*)0x201bc02e = 0xb35;
  *(uint64_t*)0x20baffe0 = 0x20e78fc4;
  *(uint64_t*)0x20baffe8 = 0;
  *(uint64_t*)0x20bafff0 = 0x20487000;
  *(uint64_t*)0x20bafff8 = 0;
  *(uint64_t*)0x20e78000 = 0x10;
  *(uint32_t*)0x20e78008 = 1;
  *(uint32_t*)0x20e7800c = 0x80;
  *(uint64_t*)0x20e78010 = 0x10;
  *(uint32_t*)0x20e78018 = 0x114;
  *(uint32_t*)0x20e7801c = 9;
  *(uint64_t*)0x20e78020 = 0x10;
  *(uint32_t*)0x20e78028 = 0;
  *(uint32_t*)0x20e7802c = 2;
  *(uint64_t*)0x20e78030 = 0x10;
  *(uint32_t*)0x20e78038 = 0x109;
  *(uint32_t*)0x20e7803c = 0x140;
  *(uint64_t*)0x20e78040 = 0x10;
  *(uint32_t*)0x20e78048 = 0x10f;
  *(uint32_t*)0x20e7804c = 2;
  *(uint64_t*)0x20e78050 = 0x10;
  *(uint32_t*)0x20e78058 = 0x117;
  *(uint32_t*)0x20e7805c = 0x61;
  *(uint64_t*)0x20e78060 = 0x10;
  *(uint32_t*)0x20e78068 = 0x10f;
  *(uint32_t*)0x20e7806c = 7;
  *(uint64_t*)0x20e78070 = 0x10;
  *(uint32_t*)0x20e78078 = 0xff;
  *(uint32_t*)0x20e7807c = 0xfffffffc;
  *(uint16_t*)0x20e78ff2 = 0x1f;
  *(uint16_t*)0x20e78ff4 = 0x3f;
  *(uint8_t*)0x20e78ff6 = 9;
  *(uint8_t*)0x20e78ff7 = 0x5b;
  *(uint8_t*)0x20e78ff8 = 0x1f;
  *(uint8_t*)0x20e78ff9 = 0x64;
  *(uint8_t*)0x20e78ffa = 0;
  *(uint8_t*)0x20e78ffb = 0x20;
  *(uint16_t*)0x20e78ffc = 0;
  *(uint8_t*)0x20e78ffe = 7;
  *(uint64_t*)0x20e78ff0 = 0x20e78000;
  *(uint64_t*)0x20e78ff8 = 0;
  *(uint64_t*)0x20e78000 = 0x10;
  *(uint32_t*)0x20e78008 = 0x117;
  *(uint32_t*)0x20e7800c = 0x6c1;
  *(uint64_t*)0x20e78010 = 0x10;
  *(uint32_t*)0x20e78018 = 0x119;
  *(uint32_t*)0x20e7801c = 2;
  *(uint16_t*)0x200fe000 = 0x27;
  *(uint32_t*)0x200fe004 = 6;
  *(uint32_t*)0x200fe008 = 0x3ff;
  *(uint32_t*)0x200fe00c = 2;
  *(uint64_t*)0x20e78fb0 = 0x204db000;
  *(uint64_t*)0x20e78fb8 = 0;
  *(uint64_t*)0x20e78fc0 = 0x20e78f1c;
  *(uint64_t*)0x20e78fc8 = 0;
  *(uint64_t*)0x20e78fd0 = 0x20652000;
  *(uint64_t*)0x20e78fd8 = 0;
  *(uint64_t*)0x20e78fe0 = 0x20b92000;
  *(uint64_t*)0x20e78fe8 = 0;
  *(uint64_t*)0x20e78ff0 = 0x20e78000;
  *(uint64_t*)0x20e78ff8 = 0;
  *(uint64_t*)0x20e77f00 = 0x10;
  *(uint32_t*)0x20e77f08 = 0x13f;
  *(uint32_t*)0x20e77f0c = 9;
  *(uint64_t*)0x20e77f10 = 0x10;
  *(uint32_t*)0x20e77f18 = 0x116;
  *(uint32_t*)0x20e77f1c = 0;
  *(uint64_t*)0x20e77f20 = 0x10;
  *(uint32_t*)0x20e77f28 = 0x10d;
  *(uint32_t*)0x20e77f2c = 0x7fffffff;
  *(uint16_t*)0x20e78ff0 = 2;
  *(uint16_t*)0x20e78ff2 = htobe16(0x4e20);
  *(uint32_t*)0x20e78ff4 = htobe32(0);
  *(uint8_t*)0x20e78ff8 = 0;
  *(uint8_t*)0x20e78ff9 = 0;
  *(uint8_t*)0x20e78ffa = 0;
  *(uint8_t*)0x20e78ffb = 0;
  *(uint8_t*)0x20e78ffc = 0;
  *(uint8_t*)0x20e78ffd = 0;
  *(uint8_t*)0x20e78ffe = 0;
  *(uint8_t*)0x20e78fff = 0;
  *(uint64_t*)0x20e78fe0 = 0x2073f000;
  *(uint64_t*)0x20e78fe8 = 0;
  *(uint64_t*)0x20e78ff0 = 0x207db000;
  *(uint64_t*)0x20e78ff8 = 0;
  *(uint64_t*)0x20e78000 = 0x10;
  *(uint32_t*)0x20e78008 = 0x19f;
  *(uint32_t*)0x20e7800c = 0x1000;
  *(uint64_t*)0x20e78010 = 0x10;
  *(uint32_t*)0x20e78018 = 0x1ff;
  *(uint32_t*)0x20e7801c = -1;
  *(uint64_t*)0x20e78020 = 0x10;
  *(uint32_t*)0x20e78028 = 0xff;
  *(uint32_t*)0x20e7802c = 0xae;
  *(uint64_t*)0x20e78030 = 0x10;
  *(uint32_t*)0x20e78038 = 0x11b;
  *(uint32_t*)0x20e7803c = 6;
  *(uint64_t*)0x20e78040 = 0x10;
  *(uint32_t*)0x20e78048 = 0x110;
  *(uint32_t*)0x20e7804c = 1;
  *(uint64_t*)0x20e78050 = 0x10;
  *(uint32_t*)0x20e78058 = 0x13f;
  *(uint32_t*)0x20e7805c = 5;
  *(uint64_t*)0x20e78060 = 0x10;
  *(uint32_t*)0x20e78068 = 0xff;
  *(uint32_t*)0x20e7806c = 0xffffff80;
  *(uint64_t*)0x20e78070 = 0x10;
  *(uint32_t*)0x20e78078 = 0x13d;
  *(uint32_t*)0x20e7807c = 0x10001;
  *(uint64_t*)0x20a73000 = 0x20e78fce;
  *(uint64_t*)0x20a73008 = 0;
  *(uint64_t*)0x20a73010 = 0x20e78f6a;
  *(uint64_t*)0x20a73018 = 0;
  *(uint64_t*)0x20a73020 = 0x20e78000;
  *(uint64_t*)0x20a73028 = 0;
  *(uint64_t*)0x20a73030 = 0x20e78000;
  *(uint64_t*)0x20a73038 = 0;
  *(uint64_t*)0x20e77eb0 = 0x10;
  *(uint32_t*)0x20e77eb8 = 0x117;
  *(uint32_t*)0x20e77ebc = 3;
  *(uint64_t*)0x20e77ec0 = 0x10;
  *(uint32_t*)0x20e77ec8 = 0x11;
  *(uint32_t*)0x20e77ecc = 0x800;
  *(uint64_t*)0x20e77ed0 = 0x10;
  *(uint32_t*)0x20e77ed8 = 0x113;
  *(uint32_t*)0x20e77edc = 0x20;
  *(uint64_t*)0x20e77ee0 = 0x10;
  *(uint32_t*)0x20e77ee8 = 0x11f;
  *(uint32_t*)0x20e77eec = 0xfffff0f5;
  syscall(__NR_sendmmsg, r[0], 0x203a6000, 5, 0x4000000);
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
