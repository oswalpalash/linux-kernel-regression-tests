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
  r[1] = syscall(__NR_socket, 2, 0x200000000000003, 9);
  memcpy((void*)0x203ae000, "\x72\x61\x77\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x203ae020 = 9;
  *(uint32_t*)0x203ae024 = 3;
  *(uint32_t*)0x203ae028 = 0x220;
  *(uint32_t*)0x203ae02c = 0xd0;
  *(uint32_t*)0x203ae030 = -1;
  *(uint32_t*)0x203ae034 = -1;
  *(uint32_t*)0x203ae038 = 0;
  *(uint32_t*)0x203ae03c = -1;
  *(uint32_t*)0x203ae040 = 0x188;
  *(uint32_t*)0x203ae044 = -1;
  *(uint32_t*)0x203ae048 = -1;
  *(uint32_t*)0x203ae04c = 0x188;
  *(uint32_t*)0x203ae050 = -1;
  *(uint32_t*)0x203ae054 = 3;
  *(uint64_t*)0x203ae058 = 0x20161fd0;
  *(uint8_t*)0x203ae060 = 0;
  *(uint8_t*)0x203ae061 = 0;
  *(uint8_t*)0x203ae062 = 0;
  *(uint8_t*)0x203ae063 = 0;
  *(uint8_t*)0x203ae064 = 0;
  *(uint8_t*)0x203ae065 = 0;
  *(uint8_t*)0x203ae066 = 0;
  *(uint8_t*)0x203ae067 = 0;
  *(uint8_t*)0x203ae068 = 0;
  *(uint8_t*)0x203ae069 = 0;
  *(uint8_t*)0x203ae06a = 0;
  *(uint8_t*)0x203ae06b = 0;
  *(uint8_t*)0x203ae06c = 0;
  *(uint8_t*)0x203ae06d = 0;
  *(uint8_t*)0x203ae06e = 0;
  *(uint8_t*)0x203ae06f = 0;
  *(uint8_t*)0x203ae070 = 0;
  *(uint8_t*)0x203ae071 = 0;
  *(uint8_t*)0x203ae072 = 0;
  *(uint8_t*)0x203ae073 = 0;
  *(uint8_t*)0x203ae074 = 0;
  *(uint8_t*)0x203ae075 = 0;
  *(uint8_t*)0x203ae076 = 0;
  *(uint8_t*)0x203ae077 = 0;
  *(uint8_t*)0x203ae078 = 0;
  *(uint8_t*)0x203ae079 = 0;
  *(uint8_t*)0x203ae07a = 0;
  *(uint8_t*)0x203ae07b = 0;
  *(uint8_t*)0x203ae07c = 0;
  *(uint8_t*)0x203ae07d = 0;
  *(uint8_t*)0x203ae07e = 0;
  *(uint8_t*)0x203ae07f = 0;
  *(uint8_t*)0x203ae080 = 0;
  *(uint8_t*)0x203ae081 = 0;
  *(uint8_t*)0x203ae082 = 0;
  *(uint8_t*)0x203ae083 = 0;
  *(uint8_t*)0x203ae084 = 0;
  *(uint8_t*)0x203ae085 = 0;
  *(uint8_t*)0x203ae086 = 0;
  *(uint8_t*)0x203ae087 = 0;
  *(uint8_t*)0x203ae088 = 0;
  *(uint8_t*)0x203ae089 = 0;
  *(uint8_t*)0x203ae08a = 0;
  *(uint8_t*)0x203ae08b = 0;
  *(uint8_t*)0x203ae08c = 0;
  *(uint8_t*)0x203ae08d = 0;
  *(uint8_t*)0x203ae08e = 0;
  *(uint8_t*)0x203ae08f = 0;
  *(uint8_t*)0x203ae090 = 0;
  *(uint8_t*)0x203ae091 = 0;
  *(uint8_t*)0x203ae092 = 0;
  *(uint8_t*)0x203ae093 = 0;
  *(uint8_t*)0x203ae094 = 0;
  *(uint8_t*)0x203ae095 = 0;
  *(uint8_t*)0x203ae096 = 0;
  *(uint8_t*)0x203ae097 = 0;
  *(uint8_t*)0x203ae098 = 0;
  *(uint8_t*)0x203ae099 = 0;
  *(uint8_t*)0x203ae09a = 0;
  *(uint8_t*)0x203ae09b = 0;
  *(uint8_t*)0x203ae09c = 0;
  *(uint8_t*)0x203ae09d = 0;
  *(uint8_t*)0x203ae09e = 0;
  *(uint8_t*)0x203ae09f = 0;
  *(uint8_t*)0x203ae0a0 = 0;
  *(uint8_t*)0x203ae0a1 = 0;
  *(uint8_t*)0x203ae0a2 = 0;
  *(uint8_t*)0x203ae0a3 = 0;
  *(uint8_t*)0x203ae0a4 = 0;
  *(uint8_t*)0x203ae0a5 = 0;
  *(uint8_t*)0x203ae0a6 = 0;
  *(uint8_t*)0x203ae0a7 = 0;
  *(uint8_t*)0x203ae0a8 = 0;
  *(uint8_t*)0x203ae0a9 = 0;
  *(uint8_t*)0x203ae0aa = 0;
  *(uint8_t*)0x203ae0ab = 0;
  *(uint8_t*)0x203ae0ac = 0;
  *(uint8_t*)0x203ae0ad = 0;
  *(uint8_t*)0x203ae0ae = 0;
  *(uint8_t*)0x203ae0af = 0;
  *(uint8_t*)0x203ae0b0 = 0;
  *(uint8_t*)0x203ae0b1 = 0;
  *(uint8_t*)0x203ae0b2 = 0;
  *(uint8_t*)0x203ae0b3 = 0;
  *(uint32_t*)0x203ae0b4 = 0;
  *(uint16_t*)0x203ae0b8 = 0x70;
  *(uint16_t*)0x203ae0ba = 0xd0;
  *(uint32_t*)0x203ae0bc = 0;
  *(uint64_t*)0x203ae0c0 = 0;
  *(uint64_t*)0x203ae0c8 = 0;
  *(uint16_t*)0x203ae0d0 = 0x60;
  memcpy((void*)0x203ae0d2, "\x48\x4d\x41\x52\x4b\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x203ae0ef = 0;
  *(uint32_t*)0x203ae0f0 = htobe32(0);
  *(uint32_t*)0x203ae100 = htobe32(0);
  *(uint32_t*)0x203ae104 = htobe32(0);
  *(uint32_t*)0x203ae108 = htobe32(0);
  *(uint32_t*)0x203ae10c = htobe32(0);
  *(uint16_t*)0x203ae110 = 0;
  *(uint16_t*)0x203ae112 = 0;
  *(uint16_t*)0x203ae114 = 0;
  *(uint16_t*)0x203ae116 = 0;
  *(uint32_t*)0x203ae118 = 0;
  *(uint16_t*)0x203ae11c = 0;
  *(uint32_t*)0x203ae120 = 0;
  *(uint32_t*)0x203ae124 = 3;
  *(uint32_t*)0x203ae128 = 0;
  *(uint8_t*)0x203ae130 = 0;
  *(uint8_t*)0x203ae131 = 0;
  *(uint8_t*)0x203ae132 = 0;
  *(uint8_t*)0x203ae133 = 0;
  *(uint8_t*)0x203ae134 = 0;
  *(uint8_t*)0x203ae135 = 0;
  *(uint8_t*)0x203ae136 = 0;
  *(uint8_t*)0x203ae137 = 0;
  *(uint8_t*)0x203ae138 = 0;
  *(uint8_t*)0x203ae139 = 0;
  *(uint8_t*)0x203ae13a = 0;
  *(uint8_t*)0x203ae13b = 0;
  *(uint8_t*)0x203ae13c = 0;
  *(uint8_t*)0x203ae13d = 0;
  *(uint8_t*)0x203ae13e = 0;
  *(uint8_t*)0x203ae13f = 0;
  *(uint8_t*)0x203ae140 = 0;
  *(uint8_t*)0x203ae141 = 0;
  *(uint8_t*)0x203ae142 = 0;
  *(uint8_t*)0x203ae143 = 0;
  *(uint8_t*)0x203ae144 = 0;
  *(uint8_t*)0x203ae145 = 0;
  *(uint8_t*)0x203ae146 = 0;
  *(uint8_t*)0x203ae147 = 0;
  *(uint8_t*)0x203ae148 = 0;
  *(uint8_t*)0x203ae149 = 0;
  *(uint8_t*)0x203ae14a = 0;
  *(uint8_t*)0x203ae14b = 0;
  *(uint8_t*)0x203ae14c = 0;
  *(uint8_t*)0x203ae14d = 0;
  *(uint8_t*)0x203ae14e = 0;
  *(uint8_t*)0x203ae14f = 0;
  *(uint8_t*)0x203ae150 = 0;
  *(uint8_t*)0x203ae151 = 0;
  *(uint8_t*)0x203ae152 = 0;
  *(uint8_t*)0x203ae153 = 0;
  *(uint8_t*)0x203ae154 = 0;
  *(uint8_t*)0x203ae155 = 0;
  *(uint8_t*)0x203ae156 = 0;
  *(uint8_t*)0x203ae157 = 0;
  *(uint8_t*)0x203ae158 = 0;
  *(uint8_t*)0x203ae159 = 0;
  *(uint8_t*)0x203ae15a = 0;
  *(uint8_t*)0x203ae15b = 0;
  *(uint8_t*)0x203ae15c = 0;
  *(uint8_t*)0x203ae15d = 0;
  *(uint8_t*)0x203ae15e = 0;
  *(uint8_t*)0x203ae15f = 0;
  *(uint8_t*)0x203ae160 = 0;
  *(uint8_t*)0x203ae161 = 0;
  *(uint8_t*)0x203ae162 = 0;
  *(uint8_t*)0x203ae163 = 0;
  *(uint8_t*)0x203ae164 = 0;
  *(uint8_t*)0x203ae165 = 0;
  *(uint8_t*)0x203ae166 = 0;
  *(uint8_t*)0x203ae167 = 0;
  *(uint8_t*)0x203ae168 = 0;
  *(uint8_t*)0x203ae169 = 0;
  *(uint8_t*)0x203ae16a = 0;
  *(uint8_t*)0x203ae16b = 0;
  *(uint8_t*)0x203ae16c = 0;
  *(uint8_t*)0x203ae16d = 0;
  *(uint8_t*)0x203ae16e = 0;
  *(uint8_t*)0x203ae16f = 0;
  *(uint8_t*)0x203ae170 = 0;
  *(uint8_t*)0x203ae171 = 0;
  *(uint8_t*)0x203ae172 = 0;
  *(uint8_t*)0x203ae173 = 0;
  *(uint8_t*)0x203ae174 = 0;
  *(uint8_t*)0x203ae175 = 0;
  *(uint8_t*)0x203ae176 = 0;
  *(uint8_t*)0x203ae177 = 0;
  *(uint8_t*)0x203ae178 = 0;
  *(uint8_t*)0x203ae179 = 0;
  *(uint8_t*)0x203ae17a = 0;
  *(uint8_t*)0x203ae17b = 0;
  *(uint8_t*)0x203ae17c = 0;
  *(uint8_t*)0x203ae17d = 0;
  *(uint8_t*)0x203ae17e = 0;
  *(uint8_t*)0x203ae17f = 0;
  *(uint8_t*)0x203ae180 = 0;
  *(uint8_t*)0x203ae181 = 0;
  *(uint8_t*)0x203ae182 = 0;
  *(uint8_t*)0x203ae183 = 0;
  *(uint32_t*)0x203ae184 = 0;
  *(uint16_t*)0x203ae188 = 0x70;
  *(uint16_t*)0x203ae18a = 0xb8;
  *(uint32_t*)0x203ae18c = 0;
  *(uint64_t*)0x203ae190 = 0;
  *(uint64_t*)0x203ae198 = 0;
  *(uint16_t*)0x203ae1a0 = 0x48;
  memcpy((void*)0x203ae1a2, "\x54\x45\x45\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x203ae1bf = 1;
  *(uint64_t*)0x203ae1c0 = htobe64(0);
  *(uint64_t*)0x203ae1c8 = htobe64(1);
  *(uint8_t*)0x203ae1d0 = 0x73;
  *(uint8_t*)0x203ae1d1 = 0x79;
  *(uint8_t*)0x203ae1d2 = 0x7a;
  *(uint8_t*)0x203ae1d3 = 0;
  *(uint8_t*)0x203ae1d4 = 0;
  *(uint64_t*)0x203ae1e0 = 0;
  *(uint8_t*)0x203ae1e8 = 0;
  *(uint8_t*)0x203ae1e9 = 0;
  *(uint8_t*)0x203ae1ea = 0;
  *(uint8_t*)0x203ae1eb = 0;
  *(uint8_t*)0x203ae1ec = 0;
  *(uint8_t*)0x203ae1ed = 0;
  *(uint8_t*)0x203ae1ee = 0;
  *(uint8_t*)0x203ae1ef = 0;
  *(uint8_t*)0x203ae1f0 = 0;
  *(uint8_t*)0x203ae1f1 = 0;
  *(uint8_t*)0x203ae1f2 = 0;
  *(uint8_t*)0x203ae1f3 = 0;
  *(uint8_t*)0x203ae1f4 = 0;
  *(uint8_t*)0x203ae1f5 = 0;
  *(uint8_t*)0x203ae1f6 = 0;
  *(uint8_t*)0x203ae1f7 = 0;
  *(uint8_t*)0x203ae1f8 = 0;
  *(uint8_t*)0x203ae1f9 = 0;
  *(uint8_t*)0x203ae1fa = 0;
  *(uint8_t*)0x203ae1fb = 0;
  *(uint8_t*)0x203ae1fc = 0;
  *(uint8_t*)0x203ae1fd = 0;
  *(uint8_t*)0x203ae1fe = 0;
  *(uint8_t*)0x203ae1ff = 0;
  *(uint8_t*)0x203ae200 = 0;
  *(uint8_t*)0x203ae201 = 0;
  *(uint8_t*)0x203ae202 = 0;
  *(uint8_t*)0x203ae203 = 0;
  *(uint8_t*)0x203ae204 = 0;
  *(uint8_t*)0x203ae205 = 0;
  *(uint8_t*)0x203ae206 = 0;
  *(uint8_t*)0x203ae207 = 0;
  *(uint8_t*)0x203ae208 = 0;
  *(uint8_t*)0x203ae209 = 0;
  *(uint8_t*)0x203ae20a = 0;
  *(uint8_t*)0x203ae20b = 0;
  *(uint8_t*)0x203ae20c = 0;
  *(uint8_t*)0x203ae20d = 0;
  *(uint8_t*)0x203ae20e = 0;
  *(uint8_t*)0x203ae20f = 0;
  *(uint8_t*)0x203ae210 = 0;
  *(uint8_t*)0x203ae211 = 0;
  *(uint8_t*)0x203ae212 = 0;
  *(uint8_t*)0x203ae213 = 0;
  *(uint8_t*)0x203ae214 = 0;
  *(uint8_t*)0x203ae215 = 0;
  *(uint8_t*)0x203ae216 = 0;
  *(uint8_t*)0x203ae217 = 0;
  *(uint8_t*)0x203ae218 = 0;
  *(uint8_t*)0x203ae219 = 0;
  *(uint8_t*)0x203ae21a = 0;
  *(uint8_t*)0x203ae21b = 0;
  *(uint8_t*)0x203ae21c = 0;
  *(uint8_t*)0x203ae21d = 0;
  *(uint8_t*)0x203ae21e = 0;
  *(uint8_t*)0x203ae21f = 0;
  *(uint8_t*)0x203ae220 = 0;
  *(uint8_t*)0x203ae221 = 0;
  *(uint8_t*)0x203ae222 = 0;
  *(uint8_t*)0x203ae223 = 0;
  *(uint8_t*)0x203ae224 = 0;
  *(uint8_t*)0x203ae225 = 0;
  *(uint8_t*)0x203ae226 = 0;
  *(uint8_t*)0x203ae227 = 0;
  *(uint8_t*)0x203ae228 = 0;
  *(uint8_t*)0x203ae229 = 0;
  *(uint8_t*)0x203ae22a = 0;
  *(uint8_t*)0x203ae22b = 0;
  *(uint8_t*)0x203ae22c = 0;
  *(uint8_t*)0x203ae22d = 0;
  *(uint8_t*)0x203ae22e = 0;
  *(uint8_t*)0x203ae22f = 0;
  *(uint8_t*)0x203ae230 = 0;
  *(uint8_t*)0x203ae231 = 0;
  *(uint8_t*)0x203ae232 = 0;
  *(uint8_t*)0x203ae233 = 0;
  *(uint8_t*)0x203ae234 = 0;
  *(uint8_t*)0x203ae235 = 0;
  *(uint8_t*)0x203ae236 = 0;
  *(uint8_t*)0x203ae237 = 0;
  *(uint8_t*)0x203ae238 = 0;
  *(uint8_t*)0x203ae239 = 0;
  *(uint8_t*)0x203ae23a = 0;
  *(uint8_t*)0x203ae23b = 0;
  *(uint32_t*)0x203ae23c = 0;
  *(uint16_t*)0x203ae240 = 0x70;
  *(uint16_t*)0x203ae242 = 0x98;
  *(uint32_t*)0x203ae244 = 0;
  *(uint64_t*)0x203ae248 = 0;
  *(uint64_t*)0x203ae250 = 0;
  *(uint16_t*)0x203ae258 = 0x28;
  memcpy((void*)0x203ae25a, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x203ae277 = 0;
  *(uint32_t*)0x203ae278 = 0xfffffffe;
  syscall(__NR_setsockopt, r[1], 0, 0x40, 0x203ae000, 0x280);
}

int main()
{
  for (;;) {
    loop();
  }
}
