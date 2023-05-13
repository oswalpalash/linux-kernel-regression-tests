// https://syzkaller.appspot.com/bug?id=5b9d1e3232dc19d61832a76821bc5fc9b914b4cd
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <linux/net.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
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
#include <stdint.h>
#include <string.h>

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

static uint64_t current_time_ms()
{
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    fail("clock_gettime failed");
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

#define XT_TABLE_SIZE 1536
#define XT_MAX_ENTRIES 10

struct xt_counters {
  uint64_t pcnt, bcnt;
};

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
  void* entrytable[XT_TABLE_SIZE / sizeof(void*)];
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
  char entrytable[XT_TABLE_SIZE];
};

struct ipt_table_desc {
  const char* name;
  struct ipt_getinfo info;
  struct ipt_replace replace;
};

static struct ipt_table_desc ipv4_tables[] = {
    {.name = "filter"}, {.name = "nat"},      {.name = "mangle"},
    {.name = "raw"},    {.name = "security"},
};

static struct ipt_table_desc ipv6_tables[] = {
    {.name = "filter"}, {.name = "nat"},      {.name = "mangle"},
    {.name = "raw"},    {.name = "security"},
};

#define IPT_BASE_CTL 64
#define IPT_SO_SET_REPLACE (IPT_BASE_CTL)
#define IPT_SO_GET_INFO (IPT_BASE_CTL)
#define IPT_SO_GET_ENTRIES (IPT_BASE_CTL + 1)

struct arpt_getinfo {
  char name[32];
  unsigned int valid_hooks;
  unsigned int hook_entry[3];
  unsigned int underflow[3];
  unsigned int num_entries;
  unsigned int size;
};

struct arpt_get_entries {
  char name[32];
  unsigned int size;
  void* entrytable[XT_TABLE_SIZE / sizeof(void*)];
};

struct arpt_replace {
  char name[32];
  unsigned int valid_hooks;
  unsigned int num_entries;
  unsigned int size;
  unsigned int hook_entry[3];
  unsigned int underflow[3];
  unsigned int num_counters;
  struct xt_counters* counters;
  char entrytable[XT_TABLE_SIZE];
};

struct arpt_table_desc {
  const char* name;
  struct arpt_getinfo info;
  struct arpt_replace replace;
};

static struct arpt_table_desc arpt_tables[] = {
    {.name = "filter"},
};

#define ARPT_BASE_CTL 96
#define ARPT_SO_SET_REPLACE (ARPT_BASE_CTL)
#define ARPT_SO_GET_INFO (ARPT_BASE_CTL)
#define ARPT_SO_GET_ENTRIES (ARPT_BASE_CTL + 1)

static void checkpoint_iptables(struct ipt_table_desc* tables, int num_tables,
                                int family, int level)
{
  struct ipt_get_entries entries;
  socklen_t optlen;
  int fd, i;

  fd = socket(family, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1)
    fail("socket(%d, SOCK_STREAM, IPPROTO_TCP)", family);
  for (i = 0; i < num_tables; i++) {
    struct ipt_table_desc* table = &tables[i];
    strcpy(table->info.name, table->name);
    strcpy(table->replace.name, table->name);
    optlen = sizeof(table->info);
    if (getsockopt(fd, level, IPT_SO_GET_INFO, &table->info, &optlen)) {
      switch (errno) {
      case EPERM:
      case ENOENT:
      case ENOPROTOOPT:
        continue;
      }
      fail("getsockopt(IPT_SO_GET_INFO)");
    }
    if (table->info.size > sizeof(table->replace.entrytable))
      fail("table size is too large: %u", table->info.size);
    if (table->info.num_entries > XT_MAX_ENTRIES)
      fail("too many counters: %u", table->info.num_entries);
    memset(&entries, 0, sizeof(entries));
    strcpy(entries.name, table->name);
    entries.size = table->info.size;
    optlen = sizeof(entries) - sizeof(entries.entrytable) + table->info.size;
    if (getsockopt(fd, level, IPT_SO_GET_ENTRIES, &entries, &optlen))
      fail("getsockopt(IPT_SO_GET_ENTRIES)");
    table->replace.valid_hooks = table->info.valid_hooks;
    table->replace.num_entries = table->info.num_entries;
    table->replace.size = table->info.size;
    memcpy(table->replace.hook_entry, table->info.hook_entry,
           sizeof(table->replace.hook_entry));
    memcpy(table->replace.underflow, table->info.underflow,
           sizeof(table->replace.underflow));
    memcpy(table->replace.entrytable, entries.entrytable, table->info.size);
  }
  close(fd);
}

static void reset_iptables(struct ipt_table_desc* tables, int num_tables,
                           int family, int level)
{
  struct xt_counters counters[XT_MAX_ENTRIES];
  struct ipt_get_entries entries;
  struct ipt_getinfo info;
  socklen_t optlen;
  int fd, i;

  fd = socket(family, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1)
    fail("socket(%d, SOCK_STREAM, IPPROTO_TCP)", family);
  for (i = 0; i < num_tables; i++) {
    struct ipt_table_desc* table = &tables[i];
    if (table->info.valid_hooks == 0)
      continue;
    memset(&info, 0, sizeof(info));
    strcpy(info.name, table->name);
    optlen = sizeof(info);
    if (getsockopt(fd, level, IPT_SO_GET_INFO, &info, &optlen))
      fail("getsockopt(IPT_SO_GET_INFO)");
    if (memcmp(&table->info, &info, sizeof(table->info)) == 0) {
      memset(&entries, 0, sizeof(entries));
      strcpy(entries.name, table->name);
      entries.size = table->info.size;
      optlen = sizeof(entries) - sizeof(entries.entrytable) + entries.size;
      if (getsockopt(fd, level, IPT_SO_GET_ENTRIES, &entries, &optlen))
        fail("getsockopt(IPT_SO_GET_ENTRIES)");
      if (memcmp(table->replace.entrytable, entries.entrytable,
                 table->info.size) == 0)
        continue;
    }
    table->replace.num_counters = info.num_entries;
    table->replace.counters = counters;
    optlen = sizeof(table->replace) - sizeof(table->replace.entrytable) +
             table->replace.size;
    if (setsockopt(fd, level, IPT_SO_SET_REPLACE, &table->replace, optlen))
      fail("setsockopt(IPT_SO_SET_REPLACE)");
  }
  close(fd);
}

static void checkpoint_arptables(void)
{
  struct arpt_get_entries entries;
  socklen_t optlen;
  unsigned i;
  int fd;

  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1)
    fail("socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)");
  for (i = 0; i < sizeof(arpt_tables) / sizeof(arpt_tables[0]); i++) {
    struct arpt_table_desc* table = &arpt_tables[i];
    strcpy(table->info.name, table->name);
    strcpy(table->replace.name, table->name);
    optlen = sizeof(table->info);
    if (getsockopt(fd, SOL_IP, ARPT_SO_GET_INFO, &table->info, &optlen)) {
      switch (errno) {
      case EPERM:
      case ENOENT:
      case ENOPROTOOPT:
        continue;
      }
      fail("getsockopt(ARPT_SO_GET_INFO)");
    }
    if (table->info.size > sizeof(table->replace.entrytable))
      fail("table size is too large: %u", table->info.size);
    if (table->info.num_entries > XT_MAX_ENTRIES)
      fail("too many counters: %u", table->info.num_entries);
    memset(&entries, 0, sizeof(entries));
    strcpy(entries.name, table->name);
    entries.size = table->info.size;
    optlen = sizeof(entries) - sizeof(entries.entrytable) + table->info.size;
    if (getsockopt(fd, SOL_IP, ARPT_SO_GET_ENTRIES, &entries, &optlen))
      fail("getsockopt(ARPT_SO_GET_ENTRIES)");
    table->replace.valid_hooks = table->info.valid_hooks;
    table->replace.num_entries = table->info.num_entries;
    table->replace.size = table->info.size;
    memcpy(table->replace.hook_entry, table->info.hook_entry,
           sizeof(table->replace.hook_entry));
    memcpy(table->replace.underflow, table->info.underflow,
           sizeof(table->replace.underflow));
    memcpy(table->replace.entrytable, entries.entrytable, table->info.size);
  }
  close(fd);
}

static void reset_arptables()
{
  struct xt_counters counters[XT_MAX_ENTRIES];
  struct arpt_get_entries entries;
  struct arpt_getinfo info;
  socklen_t optlen;
  unsigned i;
  int fd;

  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1)
    fail("socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)");
  for (i = 0; i < sizeof(arpt_tables) / sizeof(arpt_tables[0]); i++) {
    struct arpt_table_desc* table = &arpt_tables[i];
    if (table->info.valid_hooks == 0)
      continue;
    memset(&info, 0, sizeof(info));
    strcpy(info.name, table->name);
    optlen = sizeof(info);
    if (getsockopt(fd, SOL_IP, ARPT_SO_GET_INFO, &info, &optlen))
      fail("getsockopt(ARPT_SO_GET_INFO)");
    if (memcmp(&table->info, &info, sizeof(table->info)) == 0) {
      memset(&entries, 0, sizeof(entries));
      strcpy(entries.name, table->name);
      entries.size = table->info.size;
      optlen = sizeof(entries) - sizeof(entries.entrytable) + entries.size;
      if (getsockopt(fd, SOL_IP, ARPT_SO_GET_ENTRIES, &entries, &optlen))
        fail("getsockopt(ARPT_SO_GET_ENTRIES)");
      if (memcmp(table->replace.entrytable, entries.entrytable,
                 table->info.size) == 0)
        continue;
    }
    table->replace.num_counters = info.num_entries;
    table->replace.counters = counters;
    optlen = sizeof(table->replace) - sizeof(table->replace.entrytable) +
             table->replace.size;
    if (setsockopt(fd, SOL_IP, ARPT_SO_SET_REPLACE, &table->replace, optlen))
      fail("setsockopt(ARPT_SO_SET_REPLACE)");
  }
  close(fd);
}

static void checkpoint_net_namespace(void)
{
  checkpoint_arptables();
  checkpoint_iptables(ipv4_tables, sizeof(ipv4_tables) / sizeof(ipv4_tables[0]),
                      AF_INET, SOL_IP);
  checkpoint_iptables(ipv6_tables, sizeof(ipv6_tables) / sizeof(ipv6_tables[0]),
                      AF_INET6, SOL_IPV6);
}

static void reset_net_namespace(void)
{
  reset_arptables();
  reset_iptables(ipv4_tables, sizeof(ipv4_tables) / sizeof(ipv4_tables[0]),
                 AF_INET, SOL_IP);
  reset_iptables(ipv6_tables, sizeof(ipv6_tables) / sizeof(ipv6_tables[0]),
                 AF_INET6, SOL_IPV6);
}

static void test();

void loop()
{
  int iter;
  checkpoint_net_namespace();
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      fail("loop fork failed");
    if (pid == 0) {
      prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
      setpgrp();
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
    reset_net_namespace();
  }
}

long r[2];
void test()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 2, 2, 0);
  *(uint32_t*)0x20bc9f70 = 7;
  *(uint16_t*)0x20bc9f78 = 2;
  *(uint16_t*)0x20bc9f7a = 0;
  *(uint32_t*)0x20bc9f7c = htobe32(0xe0000002);
  *(uint8_t*)0x20bc9f80 = 0;
  *(uint8_t*)0x20bc9f81 = 0;
  *(uint8_t*)0x20bc9f82 = 0;
  *(uint8_t*)0x20bc9f83 = 0;
  *(uint8_t*)0x20bc9f84 = 0;
  *(uint8_t*)0x20bc9f85 = 0;
  *(uint8_t*)0x20bc9f86 = 0;
  *(uint8_t*)0x20bc9f87 = 0;
  *(uint64_t*)0x20bc9f88 = 0;
  *(uint64_t*)0x20bc9f90 = 0;
  *(uint64_t*)0x20bc9f98 = 0;
  *(uint64_t*)0x20bc9fa0 = 0;
  *(uint64_t*)0x20bc9fa8 = 0;
  *(uint64_t*)0x20bc9fb0 = 0;
  *(uint64_t*)0x20bc9fb8 = 0;
  *(uint64_t*)0x20bc9fc0 = 0;
  *(uint64_t*)0x20bc9fc8 = 0;
  *(uint64_t*)0x20bc9fd0 = 0;
  *(uint64_t*)0x20bc9fd8 = 0;
  *(uint64_t*)0x20bc9fe0 = 0;
  *(uint64_t*)0x20bc9fe8 = 0;
  *(uint64_t*)0x20bc9ff0 = 0;
  *(uint64_t*)0x20bc9ff8 = 0;
  syscall(__NR_setsockopt, r[0], 0, 0x2a, 0x20bc9f70, 0x90);
  r[1] = syscall(__NR_socket, 2, 0x200000000000003, 1);
  memcpy((void*)0x20157d80, "\x72\x61\x77\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20157da0 = 9;
  *(uint32_t*)0x20157da4 = 3;
  *(uint32_t*)0x20157da8 = 0x220;
  *(uint32_t*)0x20157dac = 0xd0;
  *(uint32_t*)0x20157db0 = -1;
  *(uint32_t*)0x20157db4 = -1;
  *(uint32_t*)0x20157db8 = 0;
  *(uint32_t*)0x20157dbc = -1;
  *(uint32_t*)0x20157dc0 = 0x188;
  *(uint32_t*)0x20157dc4 = -1;
  *(uint32_t*)0x20157dc8 = -1;
  *(uint32_t*)0x20157dcc = 0x188;
  *(uint32_t*)0x20157dd0 = -1;
  *(uint32_t*)0x20157dd4 = 3;
  *(uint64_t*)0x20157dd8 = 0x20161fd0;
  *(uint8_t*)0x20157de0 = 0;
  *(uint8_t*)0x20157de1 = 0;
  *(uint8_t*)0x20157de2 = 0;
  *(uint8_t*)0x20157de3 = 0;
  *(uint8_t*)0x20157de4 = 0;
  *(uint8_t*)0x20157de5 = 0;
  *(uint8_t*)0x20157de6 = 0;
  *(uint8_t*)0x20157de7 = 0;
  *(uint8_t*)0x20157de8 = 0;
  *(uint8_t*)0x20157de9 = 0;
  *(uint8_t*)0x20157dea = 0;
  *(uint8_t*)0x20157deb = 0;
  *(uint8_t*)0x20157dec = 0;
  *(uint8_t*)0x20157ded = 0;
  *(uint8_t*)0x20157dee = 0;
  *(uint8_t*)0x20157def = 0;
  *(uint8_t*)0x20157df0 = 0;
  *(uint8_t*)0x20157df1 = 0;
  *(uint8_t*)0x20157df2 = 0;
  *(uint8_t*)0x20157df3 = 0;
  *(uint8_t*)0x20157df4 = 0;
  *(uint8_t*)0x20157df5 = 0;
  *(uint8_t*)0x20157df6 = 0;
  *(uint8_t*)0x20157df7 = 0;
  *(uint8_t*)0x20157df8 = 0;
  *(uint8_t*)0x20157df9 = 0;
  *(uint8_t*)0x20157dfa = 0;
  *(uint8_t*)0x20157dfb = 0;
  *(uint8_t*)0x20157dfc = 0;
  *(uint8_t*)0x20157dfd = 0;
  *(uint8_t*)0x20157dfe = 0;
  *(uint8_t*)0x20157dff = 0;
  *(uint8_t*)0x20157e00 = 0;
  *(uint8_t*)0x20157e01 = 0;
  *(uint8_t*)0x20157e02 = 0;
  *(uint8_t*)0x20157e03 = 0;
  *(uint8_t*)0x20157e04 = 0;
  *(uint8_t*)0x20157e05 = 0;
  *(uint8_t*)0x20157e06 = 0;
  *(uint8_t*)0x20157e07 = 0;
  *(uint8_t*)0x20157e08 = 0;
  *(uint8_t*)0x20157e09 = 0;
  *(uint8_t*)0x20157e0a = 0;
  *(uint8_t*)0x20157e0b = 0;
  *(uint8_t*)0x20157e0c = 0;
  *(uint8_t*)0x20157e0d = 0;
  *(uint8_t*)0x20157e0e = 0;
  *(uint8_t*)0x20157e0f = 0;
  *(uint8_t*)0x20157e10 = 0;
  *(uint8_t*)0x20157e11 = 0;
  *(uint8_t*)0x20157e12 = 0;
  *(uint8_t*)0x20157e13 = 0;
  *(uint8_t*)0x20157e14 = 0;
  *(uint8_t*)0x20157e15 = 0;
  *(uint8_t*)0x20157e16 = 0;
  *(uint8_t*)0x20157e17 = 0;
  *(uint8_t*)0x20157e18 = 0;
  *(uint8_t*)0x20157e19 = 0;
  *(uint8_t*)0x20157e1a = 0;
  *(uint8_t*)0x20157e1b = 0;
  *(uint8_t*)0x20157e1c = 0;
  *(uint8_t*)0x20157e1d = 0;
  *(uint8_t*)0x20157e1e = 0;
  *(uint8_t*)0x20157e1f = 0;
  *(uint8_t*)0x20157e20 = 0;
  *(uint8_t*)0x20157e21 = 0;
  *(uint8_t*)0x20157e22 = 0;
  *(uint8_t*)0x20157e23 = 0;
  *(uint8_t*)0x20157e24 = 0;
  *(uint8_t*)0x20157e25 = 0;
  *(uint8_t*)0x20157e26 = 0;
  *(uint8_t*)0x20157e27 = 0;
  *(uint8_t*)0x20157e28 = 0;
  *(uint8_t*)0x20157e29 = 0;
  *(uint8_t*)0x20157e2a = 0;
  *(uint8_t*)0x20157e2b = 0;
  *(uint8_t*)0x20157e2c = 0;
  *(uint8_t*)0x20157e2d = 0;
  *(uint8_t*)0x20157e2e = 0;
  *(uint8_t*)0x20157e2f = 0;
  *(uint8_t*)0x20157e30 = 0;
  *(uint8_t*)0x20157e31 = 0;
  *(uint8_t*)0x20157e32 = 0;
  *(uint8_t*)0x20157e33 = 0;
  *(uint32_t*)0x20157e34 = 0;
  *(uint16_t*)0x20157e38 = 0x70;
  *(uint16_t*)0x20157e3a = 0xd0;
  *(uint32_t*)0x20157e3c = 0;
  *(uint64_t*)0x20157e40 = 0;
  *(uint64_t*)0x20157e48 = 0;
  *(uint16_t*)0x20157e50 = 0x60;
  memcpy((void*)0x20157e52, "\x48\x4d\x41\x52\x4b\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20157e6f = 0;
  *(uint32_t*)0x20157e70 = htobe32(0);
  *(uint32_t*)0x20157e80 = htobe32(0);
  *(uint32_t*)0x20157e84 = htobe32(0);
  *(uint32_t*)0x20157e88 = htobe32(0);
  *(uint32_t*)0x20157e8c = htobe32(0);
  *(uint16_t*)0x20157e90 = 0;
  *(uint16_t*)0x20157e92 = 0;
  *(uint16_t*)0x20157e94 = 0;
  *(uint16_t*)0x20157e96 = 0;
  *(uint32_t*)0x20157e98 = 0x5fe3;
  *(uint16_t*)0x20157e9c = 0;
  *(uint32_t*)0x20157ea0 = 0;
  *(uint32_t*)0x20157ea4 = 3;
  *(uint32_t*)0x20157ea8 = 0;
  *(uint8_t*)0x20157eb0 = 0;
  *(uint8_t*)0x20157eb1 = 0;
  *(uint8_t*)0x20157eb2 = 0;
  *(uint8_t*)0x20157eb3 = 0;
  *(uint8_t*)0x20157eb4 = 0;
  *(uint8_t*)0x20157eb5 = 0;
  *(uint8_t*)0x20157eb6 = 0;
  *(uint8_t*)0x20157eb7 = 0;
  *(uint8_t*)0x20157eb8 = 0;
  *(uint8_t*)0x20157eb9 = 0;
  *(uint8_t*)0x20157eba = 0;
  *(uint8_t*)0x20157ebb = 0;
  *(uint8_t*)0x20157ebc = 0;
  *(uint8_t*)0x20157ebd = 0;
  *(uint8_t*)0x20157ebe = 0;
  *(uint8_t*)0x20157ebf = 0;
  *(uint8_t*)0x20157ec0 = 0;
  *(uint8_t*)0x20157ec1 = 0;
  *(uint8_t*)0x20157ec2 = 0;
  *(uint8_t*)0x20157ec3 = 0;
  *(uint8_t*)0x20157ec4 = 0;
  *(uint8_t*)0x20157ec5 = 0;
  *(uint8_t*)0x20157ec6 = 0;
  *(uint8_t*)0x20157ec7 = 0;
  *(uint8_t*)0x20157ec8 = 0;
  *(uint8_t*)0x20157ec9 = 0;
  *(uint8_t*)0x20157eca = 0;
  *(uint8_t*)0x20157ecb = 0;
  *(uint8_t*)0x20157ecc = 0;
  *(uint8_t*)0x20157ecd = 0;
  *(uint8_t*)0x20157ece = 0;
  *(uint8_t*)0x20157ecf = 0;
  *(uint8_t*)0x20157ed0 = 0;
  *(uint8_t*)0x20157ed1 = 0;
  *(uint8_t*)0x20157ed2 = 0;
  *(uint8_t*)0x20157ed3 = 0;
  *(uint8_t*)0x20157ed4 = 0;
  *(uint8_t*)0x20157ed5 = 0;
  *(uint8_t*)0x20157ed6 = 0;
  *(uint8_t*)0x20157ed7 = 0;
  *(uint8_t*)0x20157ed8 = 0;
  *(uint8_t*)0x20157ed9 = 0;
  *(uint8_t*)0x20157eda = 0;
  *(uint8_t*)0x20157edb = 0;
  *(uint8_t*)0x20157edc = 0;
  *(uint8_t*)0x20157edd = 0;
  *(uint8_t*)0x20157ede = 0;
  *(uint8_t*)0x20157edf = 0;
  *(uint8_t*)0x20157ee0 = 0;
  *(uint8_t*)0x20157ee1 = 0;
  *(uint8_t*)0x20157ee2 = 0;
  *(uint8_t*)0x20157ee3 = 0;
  *(uint8_t*)0x20157ee4 = 0;
  *(uint8_t*)0x20157ee5 = 0;
  *(uint8_t*)0x20157ee6 = 0;
  *(uint8_t*)0x20157ee7 = 0;
  *(uint8_t*)0x20157ee8 = 0;
  *(uint8_t*)0x20157ee9 = 0;
  *(uint8_t*)0x20157eea = 0;
  *(uint8_t*)0x20157eeb = 0;
  *(uint8_t*)0x20157eec = 0;
  *(uint8_t*)0x20157eed = 0;
  *(uint8_t*)0x20157eee = 0;
  *(uint8_t*)0x20157eef = 0;
  *(uint8_t*)0x20157ef0 = 0;
  *(uint8_t*)0x20157ef1 = 0;
  *(uint8_t*)0x20157ef2 = 0;
  *(uint8_t*)0x20157ef3 = 0;
  *(uint8_t*)0x20157ef4 = 0;
  *(uint8_t*)0x20157ef5 = 0;
  *(uint8_t*)0x20157ef6 = 0;
  *(uint8_t*)0x20157ef7 = 0;
  *(uint8_t*)0x20157ef8 = 0;
  *(uint8_t*)0x20157ef9 = 0;
  *(uint8_t*)0x20157efa = 0;
  *(uint8_t*)0x20157efb = 0;
  *(uint8_t*)0x20157efc = 0;
  *(uint8_t*)0x20157efd = 0;
  *(uint8_t*)0x20157efe = 0;
  *(uint8_t*)0x20157eff = 0;
  *(uint8_t*)0x20157f00 = 0;
  *(uint8_t*)0x20157f01 = 0;
  *(uint8_t*)0x20157f02 = 0;
  *(uint8_t*)0x20157f03 = 0;
  *(uint32_t*)0x20157f04 = 0;
  *(uint16_t*)0x20157f08 = 0x70;
  *(uint16_t*)0x20157f0a = 0xb8;
  *(uint32_t*)0x20157f0c = 0;
  *(uint64_t*)0x20157f10 = 0;
  *(uint64_t*)0x20157f18 = 0;
  *(uint16_t*)0x20157f20 = 0x48;
  memcpy((void*)0x20157f22, "\x54\x45\x45\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20157f3f = 1;
  *(uint8_t*)0x20157f40 = -1;
  *(uint8_t*)0x20157f41 = 1;
  *(uint8_t*)0x20157f42 = 0;
  *(uint8_t*)0x20157f43 = 0;
  *(uint8_t*)0x20157f44 = 0;
  *(uint8_t*)0x20157f45 = 0;
  *(uint8_t*)0x20157f46 = 0;
  *(uint8_t*)0x20157f47 = 0;
  *(uint8_t*)0x20157f48 = 0;
  *(uint8_t*)0x20157f49 = 0;
  *(uint8_t*)0x20157f4a = 0;
  *(uint8_t*)0x20157f4b = 0;
  *(uint8_t*)0x20157f4c = 0;
  *(uint8_t*)0x20157f4d = 0;
  *(uint8_t*)0x20157f4e = 0;
  *(uint8_t*)0x20157f4f = 1;
  memcpy((void*)0x20157f50,
         "\x67\x72\x65\x74\x61\x70\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  *(uint64_t*)0x20157f60 = 0;
  *(uint8_t*)0x20157f68 = 0;
  *(uint8_t*)0x20157f69 = 0;
  *(uint8_t*)0x20157f6a = 0;
  *(uint8_t*)0x20157f6b = 0;
  *(uint8_t*)0x20157f6c = 0;
  *(uint8_t*)0x20157f6d = 0;
  *(uint8_t*)0x20157f6e = 0;
  *(uint8_t*)0x20157f6f = 0;
  *(uint8_t*)0x20157f70 = 0;
  *(uint8_t*)0x20157f71 = 0;
  *(uint8_t*)0x20157f72 = 0;
  *(uint8_t*)0x20157f73 = 0;
  *(uint8_t*)0x20157f74 = 0;
  *(uint8_t*)0x20157f75 = 0;
  *(uint8_t*)0x20157f76 = 0;
  *(uint8_t*)0x20157f77 = 0;
  *(uint8_t*)0x20157f78 = 0;
  *(uint8_t*)0x20157f79 = 0;
  *(uint8_t*)0x20157f7a = 0;
  *(uint8_t*)0x20157f7b = 0;
  *(uint8_t*)0x20157f7c = 0;
  *(uint8_t*)0x20157f7d = 0;
  *(uint8_t*)0x20157f7e = 0;
  *(uint8_t*)0x20157f7f = 0;
  *(uint8_t*)0x20157f80 = 0;
  *(uint8_t*)0x20157f81 = 0;
  *(uint8_t*)0x20157f82 = 0;
  *(uint8_t*)0x20157f83 = 0;
  *(uint8_t*)0x20157f84 = 0;
  *(uint8_t*)0x20157f85 = 0;
  *(uint8_t*)0x20157f86 = 0;
  *(uint8_t*)0x20157f87 = 0;
  *(uint8_t*)0x20157f88 = 0;
  *(uint8_t*)0x20157f89 = 0;
  *(uint8_t*)0x20157f8a = 0;
  *(uint8_t*)0x20157f8b = 0;
  *(uint8_t*)0x20157f8c = 0;
  *(uint8_t*)0x20157f8d = 0;
  *(uint8_t*)0x20157f8e = 0;
  *(uint8_t*)0x20157f8f = 0;
  *(uint8_t*)0x20157f90 = 0;
  *(uint8_t*)0x20157f91 = 0;
  *(uint8_t*)0x20157f92 = 0;
  *(uint8_t*)0x20157f93 = 0;
  *(uint8_t*)0x20157f94 = 0;
  *(uint8_t*)0x20157f95 = 0;
  *(uint8_t*)0x20157f96 = 0;
  *(uint8_t*)0x20157f97 = 0;
  *(uint8_t*)0x20157f98 = 0;
  *(uint8_t*)0x20157f99 = 0;
  *(uint8_t*)0x20157f9a = 0;
  *(uint8_t*)0x20157f9b = 0;
  *(uint8_t*)0x20157f9c = 0;
  *(uint8_t*)0x20157f9d = 0;
  *(uint8_t*)0x20157f9e = 0;
  *(uint8_t*)0x20157f9f = 0;
  *(uint8_t*)0x20157fa0 = 0;
  *(uint8_t*)0x20157fa1 = 0;
  *(uint8_t*)0x20157fa2 = 0;
  *(uint8_t*)0x20157fa3 = 0;
  *(uint8_t*)0x20157fa4 = 0;
  *(uint8_t*)0x20157fa5 = 0;
  *(uint8_t*)0x20157fa6 = 0;
  *(uint8_t*)0x20157fa7 = 0;
  *(uint8_t*)0x20157fa8 = 0;
  *(uint8_t*)0x20157fa9 = 0;
  *(uint8_t*)0x20157faa = 0;
  *(uint8_t*)0x20157fab = 0;
  *(uint8_t*)0x20157fac = 0;
  *(uint8_t*)0x20157fad = 0;
  *(uint8_t*)0x20157fae = 0;
  *(uint8_t*)0x20157faf = 0;
  *(uint8_t*)0x20157fb0 = 0;
  *(uint8_t*)0x20157fb1 = 0;
  *(uint8_t*)0x20157fb2 = 0;
  *(uint8_t*)0x20157fb3 = 0;
  *(uint8_t*)0x20157fb4 = 0;
  *(uint8_t*)0x20157fb5 = 0;
  *(uint8_t*)0x20157fb6 = 0;
  *(uint8_t*)0x20157fb7 = 0;
  *(uint8_t*)0x20157fb8 = 0;
  *(uint8_t*)0x20157fb9 = 0;
  *(uint8_t*)0x20157fba = 0;
  *(uint8_t*)0x20157fbb = 0;
  *(uint32_t*)0x20157fbc = 0;
  *(uint16_t*)0x20157fc0 = 0x70;
  *(uint16_t*)0x20157fc2 = 0x98;
  *(uint32_t*)0x20157fc4 = 0;
  *(uint64_t*)0x20157fc8 = 0;
  *(uint64_t*)0x20157fd0 = 0;
  *(uint16_t*)0x20157fd8 = 0x28;
  memcpy((void*)0x20157fda, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20157ff7 = 0;
  *(uint32_t*)0x20157ff8 = 0xfffffffe;
  syscall(__NR_setsockopt, r[1], 0, 0x40, 0x20157d80, 0x280);
}

int main()
{
  for (;;) {
    loop();
  }
}
