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
  r[0] = syscall(__NR_socket, 2, 6, 0);
  *(uint32_t*)0x20000000 = 0;
  *(uint16_t*)0x20000008 = 2;
  *(uint16_t*)0x2000000a = 0;
  *(uint32_t*)0x2000000c = htobe32(-1);
  *(uint8_t*)0x20000010 = 0;
  *(uint8_t*)0x20000011 = 0;
  *(uint8_t*)0x20000012 = 0;
  *(uint8_t*)0x20000013 = 0;
  *(uint8_t*)0x20000014 = 0;
  *(uint8_t*)0x20000015 = 0;
  *(uint8_t*)0x20000016 = 0;
  *(uint8_t*)0x20000017 = 0;
  *(uint64_t*)0x20000018 = 0;
  *(uint64_t*)0x20000020 = 0;
  *(uint64_t*)0x20000028 = 0;
  *(uint64_t*)0x20000030 = 0;
  *(uint64_t*)0x20000038 = 0;
  *(uint64_t*)0x20000040 = 0;
  *(uint64_t*)0x20000048 = 0;
  *(uint64_t*)0x20000050 = 0;
  *(uint64_t*)0x20000058 = 0;
  *(uint64_t*)0x20000060 = 0;
  *(uint64_t*)0x20000068 = 0;
  *(uint64_t*)0x20000070 = 0;
  *(uint64_t*)0x20000078 = 0;
  *(uint64_t*)0x20000080 = 0;
  *(uint64_t*)0x20000088 = 2;
  *(uint16_t*)0x20000090 = 2;
  *(uint16_t*)0x20000092 = 0;
  *(uint32_t*)0x20000094 = htobe32(0x7f000001);
  *(uint8_t*)0x20000098 = 0;
  *(uint8_t*)0x20000099 = 0;
  *(uint8_t*)0x2000009a = 0;
  *(uint8_t*)0x2000009b = 0;
  *(uint8_t*)0x2000009c = 0;
  *(uint8_t*)0x2000009d = 0;
  *(uint8_t*)0x2000009e = 0;
  *(uint8_t*)0x2000009f = 0;
  *(uint64_t*)0x200000a0 = 0;
  *(uint64_t*)0x200000a8 = 0;
  *(uint64_t*)0x200000b0 = 0;
  *(uint64_t*)0x200000b8 = 0;
  *(uint64_t*)0x200000c0 = 0;
  *(uint64_t*)0x200000c8 = 0;
  *(uint64_t*)0x200000d0 = 0;
  *(uint64_t*)0x200000d8 = 0;
  *(uint64_t*)0x200000e0 = 0;
  *(uint64_t*)0x200000e8 = 0;
  *(uint64_t*)0x200000f0 = 0;
  *(uint64_t*)0x200000f8 = 0;
  *(uint64_t*)0x20000100 = 0;
  *(uint64_t*)0x20000108 = 0;
  *(uint64_t*)0x20000110 = 0;
  syscall(__NR_setsockopt, r[0], 0, 0x2b, 0x20000000, 0x118);
  r[1] = syscall(__NR_socket, 0xa, 0x801, 0x84);
  memcpy((void*)0x20000ca0, "\x66\x69\x6c\x74\x65\x72\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20000cc0 = 0xe;
  *(uint32_t*)0x20000cc4 = 4;
  *(uint32_t*)0x20000cc8 = 0x300;
  *(uint32_t*)0x20000ccc = -1;
  *(uint32_t*)0x20000cd0 = 0x98;
  *(uint32_t*)0x20000cd4 = 0x188;
  *(uint32_t*)0x20000cd8 = 0x188;
  *(uint32_t*)0x20000cdc = -1;
  *(uint32_t*)0x20000ce0 = -1;
  *(uint32_t*)0x20000ce4 = 0x268;
  *(uint32_t*)0x20000ce8 = 0x268;
  *(uint32_t*)0x20000cec = 0x268;
  *(uint32_t*)0x20000cf0 = -1;
  *(uint32_t*)0x20000cf4 = 4;
  *(uint64_t*)0x20000cf8 = 0x20003fc0;
  *(uint8_t*)0x20000d00 = 0;
  *(uint8_t*)0x20000d01 = 0;
  *(uint8_t*)0x20000d02 = 0;
  *(uint8_t*)0x20000d03 = 0;
  *(uint8_t*)0x20000d04 = 0;
  *(uint8_t*)0x20000d05 = 0;
  *(uint8_t*)0x20000d06 = 0;
  *(uint8_t*)0x20000d07 = 0;
  *(uint8_t*)0x20000d08 = 0;
  *(uint8_t*)0x20000d09 = 0;
  *(uint8_t*)0x20000d0a = 0;
  *(uint8_t*)0x20000d0b = 0;
  *(uint8_t*)0x20000d0c = 0;
  *(uint8_t*)0x20000d0d = 0;
  *(uint8_t*)0x20000d0e = 0;
  *(uint8_t*)0x20000d0f = 0;
  *(uint8_t*)0x20000d10 = 0;
  *(uint8_t*)0x20000d11 = 0;
  *(uint8_t*)0x20000d12 = 0;
  *(uint8_t*)0x20000d13 = 0;
  *(uint8_t*)0x20000d14 = 0;
  *(uint8_t*)0x20000d15 = 0;
  *(uint8_t*)0x20000d16 = 0;
  *(uint8_t*)0x20000d17 = 0;
  *(uint8_t*)0x20000d18 = 0;
  *(uint8_t*)0x20000d19 = 0;
  *(uint8_t*)0x20000d1a = 0;
  *(uint8_t*)0x20000d1b = 0;
  *(uint8_t*)0x20000d1c = 0;
  *(uint8_t*)0x20000d1d = 0;
  *(uint8_t*)0x20000d1e = 0;
  *(uint8_t*)0x20000d1f = 0;
  *(uint8_t*)0x20000d20 = 0;
  *(uint8_t*)0x20000d21 = 0;
  *(uint8_t*)0x20000d22 = 0;
  *(uint8_t*)0x20000d23 = 0;
  *(uint8_t*)0x20000d24 = 0;
  *(uint8_t*)0x20000d25 = 0;
  *(uint8_t*)0x20000d26 = 0;
  *(uint8_t*)0x20000d27 = 0;
  *(uint8_t*)0x20000d28 = 0;
  *(uint8_t*)0x20000d29 = 0;
  *(uint8_t*)0x20000d2a = 0;
  *(uint8_t*)0x20000d2b = 0;
  *(uint8_t*)0x20000d2c = 0;
  *(uint8_t*)0x20000d2d = 0;
  *(uint8_t*)0x20000d2e = 0;
  *(uint8_t*)0x20000d2f = 0;
  *(uint8_t*)0x20000d30 = 0;
  *(uint8_t*)0x20000d31 = 0;
  *(uint8_t*)0x20000d32 = 0;
  *(uint8_t*)0x20000d33 = 0;
  *(uint8_t*)0x20000d34 = 0;
  *(uint8_t*)0x20000d35 = 0;
  *(uint8_t*)0x20000d36 = 0;
  *(uint8_t*)0x20000d37 = 0;
  *(uint8_t*)0x20000d38 = 0;
  *(uint8_t*)0x20000d39 = 0;
  *(uint8_t*)0x20000d3a = 0;
  *(uint8_t*)0x20000d3b = 0;
  *(uint8_t*)0x20000d3c = 0;
  *(uint8_t*)0x20000d3d = 0;
  *(uint8_t*)0x20000d3e = 0;
  *(uint8_t*)0x20000d3f = 0;
  *(uint8_t*)0x20000d40 = 0;
  *(uint8_t*)0x20000d41 = 0;
  *(uint8_t*)0x20000d42 = 0;
  *(uint8_t*)0x20000d43 = 0;
  *(uint8_t*)0x20000d44 = 0;
  *(uint8_t*)0x20000d45 = 0;
  *(uint8_t*)0x20000d46 = 0;
  *(uint8_t*)0x20000d47 = 0;
  *(uint8_t*)0x20000d48 = 0;
  *(uint8_t*)0x20000d49 = 0;
  *(uint8_t*)0x20000d4a = 0;
  *(uint8_t*)0x20000d4b = 0;
  *(uint8_t*)0x20000d4c = 0;
  *(uint8_t*)0x20000d4d = 0;
  *(uint8_t*)0x20000d4e = 0;
  *(uint8_t*)0x20000d4f = 0;
  *(uint8_t*)0x20000d50 = 0;
  *(uint8_t*)0x20000d51 = 0;
  *(uint8_t*)0x20000d52 = 0;
  *(uint8_t*)0x20000d53 = 0;
  *(uint32_t*)0x20000d54 = 0;
  *(uint16_t*)0x20000d58 = 0x70;
  *(uint16_t*)0x20000d5a = 0x98;
  *(uint32_t*)0x20000d5c = 0;
  *(uint64_t*)0x20000d60 = 0;
  *(uint64_t*)0x20000d68 = 0;
  *(uint16_t*)0x20000d70 = 0x28;
  memcpy((void*)0x20000d72, "\x41\x55\x44\x49\x54\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20000d8f = 0;
  *(uint8_t*)0x20000d90 = 0;
  *(uint8_t*)0x20000d98 = 0;
  *(uint8_t*)0x20000d99 = 0;
  *(uint8_t*)0x20000d9a = 0;
  *(uint8_t*)0x20000d9b = 0;
  *(uint8_t*)0x20000d9c = 0;
  *(uint8_t*)0x20000d9d = 0;
  *(uint8_t*)0x20000d9e = 0;
  *(uint8_t*)0x20000d9f = 0;
  *(uint8_t*)0x20000da0 = 0;
  *(uint8_t*)0x20000da1 = 0;
  *(uint8_t*)0x20000da2 = 0;
  *(uint8_t*)0x20000da3 = 0;
  *(uint8_t*)0x20000da4 = 0;
  *(uint8_t*)0x20000da5 = 0;
  *(uint8_t*)0x20000da6 = 0;
  *(uint8_t*)0x20000da7 = 0;
  *(uint8_t*)0x20000da8 = 0;
  *(uint8_t*)0x20000da9 = 0;
  *(uint8_t*)0x20000daa = 0;
  *(uint8_t*)0x20000dab = 0;
  *(uint8_t*)0x20000dac = 0;
  *(uint8_t*)0x20000dad = 0;
  *(uint8_t*)0x20000dae = 0;
  *(uint8_t*)0x20000daf = 0;
  *(uint8_t*)0x20000db0 = 0;
  *(uint8_t*)0x20000db1 = 0;
  *(uint8_t*)0x20000db2 = 0;
  *(uint8_t*)0x20000db3 = 0;
  *(uint8_t*)0x20000db4 = 0;
  *(uint8_t*)0x20000db5 = 0;
  *(uint8_t*)0x20000db6 = 0;
  *(uint8_t*)0x20000db7 = 0;
  *(uint8_t*)0x20000db8 = 0;
  *(uint8_t*)0x20000db9 = 0;
  *(uint8_t*)0x20000dba = 0;
  *(uint8_t*)0x20000dbb = 0;
  *(uint8_t*)0x20000dbc = 0;
  *(uint8_t*)0x20000dbd = 0;
  *(uint8_t*)0x20000dbe = 0;
  *(uint8_t*)0x20000dbf = 0;
  *(uint8_t*)0x20000dc0 = 0;
  *(uint8_t*)0x20000dc1 = 0;
  *(uint8_t*)0x20000dc2 = 0;
  *(uint8_t*)0x20000dc3 = 0;
  *(uint8_t*)0x20000dc4 = 0;
  *(uint8_t*)0x20000dc5 = 0;
  *(uint8_t*)0x20000dc6 = 0;
  *(uint8_t*)0x20000dc7 = 0;
  *(uint8_t*)0x20000dc8 = 0;
  *(uint8_t*)0x20000dc9 = 0;
  *(uint8_t*)0x20000dca = 0;
  *(uint8_t*)0x20000dcb = 0;
  *(uint8_t*)0x20000dcc = 0;
  *(uint8_t*)0x20000dcd = 0;
  *(uint8_t*)0x20000dce = 0;
  *(uint8_t*)0x20000dcf = 0;
  *(uint8_t*)0x20000dd0 = 0;
  *(uint8_t*)0x20000dd1 = 0;
  *(uint8_t*)0x20000dd2 = 0;
  *(uint8_t*)0x20000dd3 = 0;
  *(uint8_t*)0x20000dd4 = 0;
  *(uint8_t*)0x20000dd5 = 0;
  *(uint8_t*)0x20000dd6 = 0;
  *(uint8_t*)0x20000dd7 = 0;
  *(uint8_t*)0x20000dd8 = 0;
  *(uint8_t*)0x20000dd9 = 0;
  *(uint8_t*)0x20000dda = 0;
  *(uint8_t*)0x20000ddb = 0;
  *(uint8_t*)0x20000ddc = 0;
  *(uint8_t*)0x20000ddd = 0;
  *(uint8_t*)0x20000dde = 0;
  *(uint8_t*)0x20000ddf = 0;
  *(uint8_t*)0x20000de0 = 0;
  *(uint8_t*)0x20000de1 = 0;
  *(uint8_t*)0x20000de2 = 0;
  *(uint8_t*)0x20000de3 = 0;
  *(uint8_t*)0x20000de4 = 0;
  *(uint8_t*)0x20000de5 = 0;
  *(uint8_t*)0x20000de6 = 0;
  *(uint8_t*)0x20000de7 = 0;
  *(uint8_t*)0x20000de8 = 0;
  *(uint8_t*)0x20000de9 = 0;
  *(uint8_t*)0x20000dea = 0;
  *(uint8_t*)0x20000deb = 0;
  *(uint32_t*)0x20000dec = 0;
  *(uint16_t*)0x20000df0 = 0xc0;
  *(uint16_t*)0x20000df2 = 0xf0;
  *(uint32_t*)0x20000df4 = 0;
  *(uint64_t*)0x20000df8 = 0;
  *(uint64_t*)0x20000e00 = 0;
  *(uint16_t*)0x20000e08 = 0x28;
  memcpy((void*)0x20000e0a, "\x63\x67\x72\x6f\x75\x70\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20000e27 = 0;
  *(uint32_t*)0x20000e28 = 0;
  *(uint32_t*)0x20000e2c = 0;
  *(uint16_t*)0x20000e30 = 0x28;
  memcpy((void*)0x20000e32, "\x74\x74\x6c\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20000e4f = 0;
  *(uint8_t*)0x20000e50 = 0;
  *(uint8_t*)0x20000e51 = 0;
  *(uint16_t*)0x20000e58 = 0x30;
  memcpy((void*)0x20000e5a, "\x43\x4f\x4e\x4e\x4d\x41\x52\x4b\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20000e77 = 1;
  *(uint32_t*)0x20000e78 = 0;
  *(uint32_t*)0x20000e7c = 0;
  *(uint32_t*)0x20000e80 = 0;
  *(uint8_t*)0x20000e84 = 0;
  *(uint8_t*)0x20000e88 = 0;
  *(uint8_t*)0x20000e89 = 0;
  *(uint8_t*)0x20000e8a = 0;
  *(uint8_t*)0x20000e8b = 0;
  *(uint8_t*)0x20000e8c = 0;
  *(uint8_t*)0x20000e8d = 0;
  *(uint8_t*)0x20000e8e = 0;
  *(uint8_t*)0x20000e8f = 0;
  *(uint8_t*)0x20000e90 = 0;
  *(uint8_t*)0x20000e91 = 0;
  *(uint8_t*)0x20000e92 = 0;
  *(uint8_t*)0x20000e93 = 0;
  *(uint8_t*)0x20000e94 = 0;
  *(uint8_t*)0x20000e95 = 0;
  *(uint8_t*)0x20000e96 = 0;
  *(uint8_t*)0x20000e97 = 0;
  *(uint8_t*)0x20000e98 = 0;
  *(uint8_t*)0x20000e99 = 0;
  *(uint8_t*)0x20000e9a = 0;
  *(uint8_t*)0x20000e9b = 0;
  *(uint8_t*)0x20000e9c = 0;
  *(uint8_t*)0x20000e9d = 0;
  *(uint8_t*)0x20000e9e = 0;
  *(uint8_t*)0x20000e9f = 0;
  *(uint8_t*)0x20000ea0 = 0;
  *(uint8_t*)0x20000ea1 = 0;
  *(uint8_t*)0x20000ea2 = 0;
  *(uint8_t*)0x20000ea3 = 0;
  *(uint8_t*)0x20000ea4 = 0;
  *(uint8_t*)0x20000ea5 = 0;
  *(uint8_t*)0x20000ea6 = 0;
  *(uint8_t*)0x20000ea7 = 0;
  *(uint8_t*)0x20000ea8 = 0;
  *(uint8_t*)0x20000ea9 = 0;
  *(uint8_t*)0x20000eaa = 0;
  *(uint8_t*)0x20000eab = 0;
  *(uint8_t*)0x20000eac = 0;
  *(uint8_t*)0x20000ead = 0;
  *(uint8_t*)0x20000eae = 0;
  *(uint8_t*)0x20000eaf = 0;
  *(uint8_t*)0x20000eb0 = 0;
  *(uint8_t*)0x20000eb1 = 0;
  *(uint8_t*)0x20000eb2 = 0;
  *(uint8_t*)0x20000eb3 = 0;
  *(uint8_t*)0x20000eb4 = 0;
  *(uint8_t*)0x20000eb5 = 0;
  *(uint8_t*)0x20000eb6 = 0;
  *(uint8_t*)0x20000eb7 = 0;
  *(uint8_t*)0x20000eb8 = 0;
  *(uint8_t*)0x20000eb9 = 0;
  *(uint8_t*)0x20000eba = 0;
  *(uint8_t*)0x20000ebb = 0;
  *(uint8_t*)0x20000ebc = 0;
  *(uint8_t*)0x20000ebd = 0;
  *(uint8_t*)0x20000ebe = 0;
  *(uint8_t*)0x20000ebf = 0;
  *(uint8_t*)0x20000ec0 = 0;
  *(uint8_t*)0x20000ec1 = 0;
  *(uint8_t*)0x20000ec2 = 0;
  *(uint8_t*)0x20000ec3 = 0;
  *(uint8_t*)0x20000ec4 = 0;
  *(uint8_t*)0x20000ec5 = 0;
  *(uint8_t*)0x20000ec6 = 0;
  *(uint8_t*)0x20000ec7 = 0;
  *(uint8_t*)0x20000ec8 = 0;
  *(uint8_t*)0x20000ec9 = 0;
  *(uint8_t*)0x20000eca = 0;
  *(uint8_t*)0x20000ecb = 0;
  *(uint8_t*)0x20000ecc = 0;
  *(uint8_t*)0x20000ecd = 0;
  *(uint8_t*)0x20000ece = 0;
  *(uint8_t*)0x20000ecf = 0;
  *(uint8_t*)0x20000ed0 = 0;
  *(uint8_t*)0x20000ed1 = 0;
  *(uint8_t*)0x20000ed2 = 0;
  *(uint8_t*)0x20000ed3 = 0;
  *(uint8_t*)0x20000ed4 = 0;
  *(uint8_t*)0x20000ed5 = 0;
  *(uint8_t*)0x20000ed6 = 0;
  *(uint8_t*)0x20000ed7 = 0;
  *(uint8_t*)0x20000ed8 = 0;
  *(uint8_t*)0x20000ed9 = 0;
  *(uint8_t*)0x20000eda = 0;
  *(uint8_t*)0x20000edb = 0;
  *(uint32_t*)0x20000edc = 0;
  *(uint16_t*)0x20000ee0 = 0x98;
  *(uint16_t*)0x20000ee2 = 0xe0;
  *(uint32_t*)0x20000ee4 = 0;
  *(uint64_t*)0x20000ee8 = 0;
  *(uint64_t*)0x20000ef0 = 0;
  *(uint16_t*)0x20000ef8 = 0x28;
  memcpy((void*)0x20000efa, "\x63\x6f\x6e\x6e\x6c\x61\x62\x65\x6c\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20000f17 = 0;
  *(uint16_t*)0x20000f18 = 0;
  *(uint16_t*)0x20000f1a = 2;
  *(uint16_t*)0x20000f20 = 0x48;
  memcpy((void*)0x20000f22, "\x54\x45\x45\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20000f3f = 1;
  *(uint8_t*)0x20000f40 = 0xfe;
  *(uint8_t*)0x20000f41 = 0x80;
  *(uint8_t*)0x20000f42 = 0;
  *(uint8_t*)0x20000f43 = 0;
  *(uint8_t*)0x20000f44 = 0;
  *(uint8_t*)0x20000f45 = 0;
  *(uint8_t*)0x20000f46 = 0;
  *(uint8_t*)0x20000f47 = 0;
  *(uint8_t*)0x20000f48 = 0;
  *(uint8_t*)0x20000f49 = 0;
  *(uint8_t*)0x20000f4a = 0;
  *(uint8_t*)0x20000f4b = 0;
  *(uint8_t*)0x20000f4c = 0;
  *(uint8_t*)0x20000f4d = 0;
  *(uint8_t*)0x20000f4e = 0;
  *(uint8_t*)0x20000f4f = 0xbb;
  memcpy((void*)0x20000f50,
         "\x76\x63\x61\x6e\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  *(uint64_t*)0x20000f60 = 0;
  *(uint8_t*)0x20000f68 = 0;
  *(uint8_t*)0x20000f69 = 0;
  *(uint8_t*)0x20000f6a = 0;
  *(uint8_t*)0x20000f6b = 0;
  *(uint8_t*)0x20000f6c = 0;
  *(uint8_t*)0x20000f6d = 0;
  *(uint8_t*)0x20000f6e = 0;
  *(uint8_t*)0x20000f6f = 0;
  *(uint8_t*)0x20000f70 = 0;
  *(uint8_t*)0x20000f71 = 0;
  *(uint8_t*)0x20000f72 = 0;
  *(uint8_t*)0x20000f73 = 0;
  *(uint8_t*)0x20000f74 = 0;
  *(uint8_t*)0x20000f75 = 0;
  *(uint8_t*)0x20000f76 = 0;
  *(uint8_t*)0x20000f77 = 0;
  *(uint8_t*)0x20000f78 = 0;
  *(uint8_t*)0x20000f79 = 0;
  *(uint8_t*)0x20000f7a = 0;
  *(uint8_t*)0x20000f7b = 0;
  *(uint8_t*)0x20000f7c = 0;
  *(uint8_t*)0x20000f7d = 0;
  *(uint8_t*)0x20000f7e = 0;
  *(uint8_t*)0x20000f7f = 0;
  *(uint8_t*)0x20000f80 = 0;
  *(uint8_t*)0x20000f81 = 0;
  *(uint8_t*)0x20000f82 = 0;
  *(uint8_t*)0x20000f83 = 0;
  *(uint8_t*)0x20000f84 = 0;
  *(uint8_t*)0x20000f85 = 0;
  *(uint8_t*)0x20000f86 = 0;
  *(uint8_t*)0x20000f87 = 0;
  *(uint8_t*)0x20000f88 = 0;
  *(uint8_t*)0x20000f89 = 0;
  *(uint8_t*)0x20000f8a = 0;
  *(uint8_t*)0x20000f8b = 0;
  *(uint8_t*)0x20000f8c = 0;
  *(uint8_t*)0x20000f8d = 0;
  *(uint8_t*)0x20000f8e = 0;
  *(uint8_t*)0x20000f8f = 0;
  *(uint8_t*)0x20000f90 = 0;
  *(uint8_t*)0x20000f91 = 0;
  *(uint8_t*)0x20000f92 = 0;
  *(uint8_t*)0x20000f93 = 0;
  *(uint8_t*)0x20000f94 = 0;
  *(uint8_t*)0x20000f95 = 0;
  *(uint8_t*)0x20000f96 = 0;
  *(uint8_t*)0x20000f97 = 0;
  *(uint8_t*)0x20000f98 = 0;
  *(uint8_t*)0x20000f99 = 0;
  *(uint8_t*)0x20000f9a = 0;
  *(uint8_t*)0x20000f9b = 0;
  *(uint8_t*)0x20000f9c = 0;
  *(uint8_t*)0x20000f9d = 0;
  *(uint8_t*)0x20000f9e = 0;
  *(uint8_t*)0x20000f9f = 0;
  *(uint8_t*)0x20000fa0 = 0;
  *(uint8_t*)0x20000fa1 = 0;
  *(uint8_t*)0x20000fa2 = 0;
  *(uint8_t*)0x20000fa3 = 0;
  *(uint8_t*)0x20000fa4 = 0;
  *(uint8_t*)0x20000fa5 = 0;
  *(uint8_t*)0x20000fa6 = 0;
  *(uint8_t*)0x20000fa7 = 0;
  *(uint8_t*)0x20000fa8 = 0;
  *(uint8_t*)0x20000fa9 = 0;
  *(uint8_t*)0x20000faa = 0;
  *(uint8_t*)0x20000fab = 0;
  *(uint8_t*)0x20000fac = 0;
  *(uint8_t*)0x20000fad = 0;
  *(uint8_t*)0x20000fae = 0;
  *(uint8_t*)0x20000faf = 0;
  *(uint8_t*)0x20000fb0 = 0;
  *(uint8_t*)0x20000fb1 = 0;
  *(uint8_t*)0x20000fb2 = 0;
  *(uint8_t*)0x20000fb3 = 0;
  *(uint8_t*)0x20000fb4 = 0;
  *(uint8_t*)0x20000fb5 = 0;
  *(uint8_t*)0x20000fb6 = 0;
  *(uint8_t*)0x20000fb7 = 0;
  *(uint8_t*)0x20000fb8 = 0;
  *(uint8_t*)0x20000fb9 = 0;
  *(uint8_t*)0x20000fba = 0;
  *(uint8_t*)0x20000fbb = 0;
  *(uint32_t*)0x20000fbc = 0;
  *(uint16_t*)0x20000fc0 = 0x70;
  *(uint16_t*)0x20000fc2 = 0x98;
  *(uint32_t*)0x20000fc4 = 0;
  *(uint64_t*)0x20000fc8 = 0;
  *(uint64_t*)0x20000fd0 = 0;
  *(uint16_t*)0x20000fd8 = 0x28;
  memcpy((void*)0x20000fda, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20000ff7 = 0;
  *(uint32_t*)0x20000ff8 = 0xfffffffe;
  syscall(__NR_setsockopt, r[1], 0, 0x40, 0x20000ca0, 0x360);
}

int main()
{
  for (;;) {
    loop();
  }
}
