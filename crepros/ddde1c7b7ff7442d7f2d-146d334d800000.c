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
  syscall(__NR_mmap, 0x20000000, 0xfe5000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 0xa, 0x80001, 0);
  *(uint32_t*)0x20000fac = 0;
  *(uint16_t*)0x20000fb4 = 0xa;
  *(uint16_t*)0x20000fb6 = htobe16(0x4e20);
  *(uint32_t*)0x20000fb8 = 0;
  *(uint8_t*)0x20000fbc = 0xfe;
  *(uint8_t*)0x20000fbd = 0x80;
  *(uint8_t*)0x20000fbe = 0;
  *(uint8_t*)0x20000fbf = 0;
  *(uint8_t*)0x20000fc0 = 0;
  *(uint8_t*)0x20000fc1 = 0;
  *(uint8_t*)0x20000fc2 = 0;
  *(uint8_t*)0x20000fc3 = 0;
  *(uint8_t*)0x20000fc4 = 0;
  *(uint8_t*)0x20000fc5 = 0;
  *(uint8_t*)0x20000fc6 = 0;
  *(uint8_t*)0x20000fc7 = 0;
  *(uint8_t*)0x20000fc8 = 0;
  *(uint8_t*)0x20000fc9 = 0;
  *(uint8_t*)0x20000fca = 0;
  *(uint8_t*)0x20000fcb = 0xaa;
  *(uint32_t*)0x20000fcc = 0;
  *(uint64_t*)0x20000fd4 = 0;
  *(uint64_t*)0x20000fdc = 0;
  *(uint64_t*)0x20000fe4 = 0;
  *(uint64_t*)0x20000fec = 0;
  *(uint64_t*)0x20000ff4 = 0;
  *(uint64_t*)0x20000ffc = 0;
  *(uint64_t*)0x20001004 = 0;
  *(uint64_t*)0x2000100c = 0;
  *(uint64_t*)0x20001014 = 0;
  *(uint64_t*)0x2000101c = 0;
  *(uint64_t*)0x20001024 = 0;
  *(uint64_t*)0x2000102c = 0;
  *(uint16_t*)0x20001034 = 0xa;
  *(uint16_t*)0x20001036 = htobe16(0x4e20);
  *(uint32_t*)0x20001038 = 0;
  *(uint8_t*)0x2000103c = 0;
  *(uint8_t*)0x2000103d = 0;
  *(uint8_t*)0x2000103e = 0;
  *(uint8_t*)0x2000103f = 0;
  *(uint8_t*)0x20001040 = 0;
  *(uint8_t*)0x20001041 = 0;
  *(uint8_t*)0x20001042 = 0;
  *(uint8_t*)0x20001043 = 0;
  *(uint8_t*)0x20001044 = 0;
  *(uint8_t*)0x20001045 = 0;
  *(uint8_t*)0x20001046 = 0;
  *(uint8_t*)0x20001047 = 0;
  *(uint8_t*)0x20001048 = 0;
  *(uint8_t*)0x20001049 = 0;
  *(uint8_t*)0x2000104a = 0;
  *(uint8_t*)0x2000104b = 0;
  *(uint32_t*)0x2000104c = 0;
  *(uint64_t*)0x20001054 = 0;
  *(uint64_t*)0x2000105c = 0;
  *(uint64_t*)0x20001064 = 0;
  *(uint64_t*)0x2000106c = 0;
  *(uint64_t*)0x20001074 = 0;
  *(uint64_t*)0x2000107c = 0;
  *(uint64_t*)0x20001084 = 0;
  *(uint64_t*)0x2000108c = 0;
  *(uint64_t*)0x20001094 = 0;
  *(uint64_t*)0x2000109c = 0;
  *(uint64_t*)0x200010a4 = 0;
  *(uint64_t*)0x200010ac = 0;
  syscall(__NR_setsockopt, r[0], 0x29, 0x2f, 0x20000fac, 0x108);
  r[1] = syscall(__NR_socket, 0xa, 3, 0x87);
  memcpy((void*)0x20010c40, "\x66\x69\x6c\x74\x65\x72\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20010c60 = 0xe;
  *(uint32_t*)0x20010c64 = 4;
  *(uint32_t*)0x20010c68 = 0x360;
  *(uint32_t*)0x20010c6c = -1;
  *(uint32_t*)0x20010c70 = 0;
  *(uint32_t*)0x20010c74 = 0x1a0;
  *(uint32_t*)0x20010c78 = 0xd0;
  *(uint32_t*)0x20010c7c = -1;
  *(uint32_t*)0x20010c80 = -1;
  *(uint32_t*)0x20010c84 = 0x290;
  *(uint32_t*)0x20010c88 = 0x290;
  *(uint32_t*)0x20010c8c = 0x290;
  *(uint32_t*)0x20010c90 = -1;
  *(uint32_t*)0x20010c94 = 4;
  *(uint64_t*)0x20010c98 = 0x2000b000;
  *(uint8_t*)0x20010ca0 = 0;
  *(uint8_t*)0x20010ca1 = 0;
  *(uint8_t*)0x20010ca2 = 0;
  *(uint8_t*)0x20010ca3 = 0;
  *(uint8_t*)0x20010ca4 = 0;
  *(uint8_t*)0x20010ca5 = 0;
  *(uint8_t*)0x20010ca6 = 0;
  *(uint8_t*)0x20010ca7 = 0;
  *(uint8_t*)0x20010ca8 = 0;
  *(uint8_t*)0x20010ca9 = 0;
  *(uint8_t*)0x20010caa = 0;
  *(uint8_t*)0x20010cab = 0;
  *(uint8_t*)0x20010cac = 0;
  *(uint8_t*)0x20010cad = 0;
  *(uint8_t*)0x20010cae = 0;
  *(uint8_t*)0x20010caf = 0;
  *(uint8_t*)0x20010cb0 = 0;
  *(uint8_t*)0x20010cb1 = 0;
  *(uint8_t*)0x20010cb2 = 0;
  *(uint8_t*)0x20010cb3 = 0;
  *(uint8_t*)0x20010cb4 = 0;
  *(uint8_t*)0x20010cb5 = 0;
  *(uint8_t*)0x20010cb6 = 0;
  *(uint8_t*)0x20010cb7 = 0;
  *(uint8_t*)0x20010cb8 = 0;
  *(uint8_t*)0x20010cb9 = 0;
  *(uint8_t*)0x20010cba = 0;
  *(uint8_t*)0x20010cbb = 0;
  *(uint8_t*)0x20010cbc = 0;
  *(uint8_t*)0x20010cbd = 0;
  *(uint8_t*)0x20010cbe = 0;
  *(uint8_t*)0x20010cbf = 0;
  *(uint8_t*)0x20010cc0 = 0;
  *(uint8_t*)0x20010cc1 = 0;
  *(uint8_t*)0x20010cc2 = 0;
  *(uint8_t*)0x20010cc3 = 0;
  *(uint8_t*)0x20010cc4 = 0;
  *(uint8_t*)0x20010cc5 = 0;
  *(uint8_t*)0x20010cc6 = 0;
  *(uint8_t*)0x20010cc7 = 0;
  *(uint8_t*)0x20010cc8 = 0;
  *(uint8_t*)0x20010cc9 = 0;
  *(uint8_t*)0x20010cca = 0;
  *(uint8_t*)0x20010ccb = 0;
  *(uint8_t*)0x20010ccc = 0;
  *(uint8_t*)0x20010ccd = 0;
  *(uint8_t*)0x20010cce = 0;
  *(uint8_t*)0x20010ccf = 0;
  *(uint8_t*)0x20010cd0 = 0;
  *(uint8_t*)0x20010cd1 = 0;
  *(uint8_t*)0x20010cd2 = 0;
  *(uint8_t*)0x20010cd3 = 0;
  *(uint8_t*)0x20010cd4 = 0;
  *(uint8_t*)0x20010cd5 = 0;
  *(uint8_t*)0x20010cd6 = 0;
  *(uint8_t*)0x20010cd7 = 0;
  *(uint8_t*)0x20010cd8 = 0;
  *(uint8_t*)0x20010cd9 = 0;
  *(uint8_t*)0x20010cda = 0;
  *(uint8_t*)0x20010cdb = 0;
  *(uint8_t*)0x20010cdc = 0;
  *(uint8_t*)0x20010cdd = 0;
  *(uint8_t*)0x20010cde = 0;
  *(uint8_t*)0x20010cdf = 0;
  *(uint8_t*)0x20010ce0 = 0;
  *(uint8_t*)0x20010ce1 = 0;
  *(uint8_t*)0x20010ce2 = 0;
  *(uint8_t*)0x20010ce3 = 0;
  *(uint8_t*)0x20010ce4 = 0;
  *(uint8_t*)0x20010ce5 = 0;
  *(uint8_t*)0x20010ce6 = 0;
  *(uint8_t*)0x20010ce7 = 0;
  *(uint8_t*)0x20010ce8 = 0;
  *(uint8_t*)0x20010ce9 = 0;
  *(uint8_t*)0x20010cea = 0;
  *(uint8_t*)0x20010ceb = 0;
  *(uint8_t*)0x20010cec = 0;
  *(uint8_t*)0x20010ced = 0;
  *(uint8_t*)0x20010cee = 0;
  *(uint8_t*)0x20010cef = 0;
  *(uint8_t*)0x20010cf0 = 0;
  *(uint8_t*)0x20010cf1 = 0;
  *(uint8_t*)0x20010cf2 = 0;
  *(uint8_t*)0x20010cf3 = 0;
  *(uint8_t*)0x20010cf4 = 0;
  *(uint8_t*)0x20010cf5 = 0;
  *(uint8_t*)0x20010cf6 = 0;
  *(uint8_t*)0x20010cf7 = 0;
  *(uint8_t*)0x20010cf8 = 0;
  *(uint8_t*)0x20010cf9 = 0;
  *(uint8_t*)0x20010cfa = 0;
  *(uint8_t*)0x20010cfb = 0;
  *(uint8_t*)0x20010cfc = 0;
  *(uint8_t*)0x20010cfd = 0;
  *(uint8_t*)0x20010cfe = 0;
  *(uint8_t*)0x20010cff = 0;
  *(uint8_t*)0x20010d00 = 0;
  *(uint8_t*)0x20010d01 = 0;
  *(uint8_t*)0x20010d02 = 0;
  *(uint8_t*)0x20010d03 = 0;
  *(uint8_t*)0x20010d04 = 0;
  *(uint8_t*)0x20010d05 = 0;
  *(uint8_t*)0x20010d06 = 0;
  *(uint8_t*)0x20010d07 = 0;
  *(uint8_t*)0x20010d08 = 0;
  *(uint8_t*)0x20010d09 = 0;
  *(uint8_t*)0x20010d0a = 0;
  *(uint8_t*)0x20010d0b = 0;
  *(uint8_t*)0x20010d0c = 0;
  *(uint8_t*)0x20010d0d = 0;
  *(uint8_t*)0x20010d0e = 0;
  *(uint8_t*)0x20010d0f = 0;
  *(uint8_t*)0x20010d10 = 0;
  *(uint8_t*)0x20010d11 = 0;
  *(uint8_t*)0x20010d12 = 0;
  *(uint8_t*)0x20010d13 = 0;
  *(uint8_t*)0x20010d14 = 0;
  *(uint8_t*)0x20010d15 = 0;
  *(uint8_t*)0x20010d16 = 0;
  *(uint8_t*)0x20010d17 = 0;
  *(uint8_t*)0x20010d18 = 0;
  *(uint8_t*)0x20010d19 = 0;
  *(uint8_t*)0x20010d1a = 0;
  *(uint8_t*)0x20010d1b = 0;
  *(uint8_t*)0x20010d1c = 0;
  *(uint8_t*)0x20010d1d = 0;
  *(uint8_t*)0x20010d1e = 0;
  *(uint8_t*)0x20010d1f = 0;
  *(uint8_t*)0x20010d20 = 0;
  *(uint8_t*)0x20010d21 = 0;
  *(uint8_t*)0x20010d22 = 0;
  *(uint8_t*)0x20010d23 = 0;
  *(uint8_t*)0x20010d24 = 0;
  *(uint8_t*)0x20010d25 = 0;
  *(uint8_t*)0x20010d26 = 0;
  *(uint8_t*)0x20010d27 = 0;
  *(uint32_t*)0x20010d28 = 0;
  *(uint16_t*)0x20010d2c = 0xa8;
  *(uint16_t*)0x20010d2e = 0xd0;
  *(uint32_t*)0x20010d30 = 0;
  *(uint64_t*)0x20010d38 = 0;
  *(uint64_t*)0x20010d40 = 0;
  *(uint16_t*)0x20010d48 = 0x28;
  memcpy((void*)0x20010d4a, "\x52\x45\x4a\x45\x43\x54\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20010d67 = 0;
  *(uint32_t*)0x20010d68 = 0;
  *(uint8_t*)0x20010d70 = 0;
  *(uint8_t*)0x20010d71 = 0;
  *(uint8_t*)0x20010d72 = 0;
  *(uint8_t*)0x20010d73 = 0;
  *(uint8_t*)0x20010d74 = 0;
  *(uint8_t*)0x20010d75 = 0;
  *(uint8_t*)0x20010d76 = 0;
  *(uint8_t*)0x20010d77 = 0;
  *(uint8_t*)0x20010d78 = 0;
  *(uint8_t*)0x20010d79 = 0;
  *(uint8_t*)0x20010d7a = -1;
  *(uint8_t*)0x20010d7b = -1;
  *(uint8_t*)0x20010d7c = 0xac;
  *(uint8_t*)0x20010d7d = 0x14;
  *(uint8_t*)0x20010d7e = 0;
  *(uint8_t*)0x20010d7f = 0xaa;
  *(uint8_t*)0x20010d80 = 0xfe;
  *(uint8_t*)0x20010d81 = 0x80;
  *(uint8_t*)0x20010d82 = 0;
  *(uint8_t*)0x20010d83 = 0;
  *(uint8_t*)0x20010d84 = 0;
  *(uint8_t*)0x20010d85 = 0;
  *(uint8_t*)0x20010d86 = 0;
  *(uint8_t*)0x20010d87 = 0;
  *(uint8_t*)0x20010d88 = 0;
  *(uint8_t*)0x20010d89 = 0;
  *(uint8_t*)0x20010d8a = 0;
  *(uint8_t*)0x20010d8b = 0;
  *(uint8_t*)0x20010d8c = 0;
  *(uint8_t*)0x20010d8d = 0;
  *(uint8_t*)0x20010d8e = 0;
  *(uint8_t*)0x20010d8f = 0xbb;
  *(uint32_t*)0x20010d90 = htobe32(0);
  *(uint32_t*)0x20010d94 = htobe32(0);
  *(uint32_t*)0x20010d98 = htobe32(0);
  *(uint32_t*)0x20010d9c = htobe32(0xff000000);
  *(uint32_t*)0x20010da0 = htobe32(0);
  *(uint32_t*)0x20010da4 = htobe32(0);
  *(uint32_t*)0x20010da8 = htobe32(0);
  *(uint32_t*)0x20010dac = htobe32(0);
  *(uint8_t*)0x20010db0 = 0x73;
  *(uint8_t*)0x20010db1 = 0x79;
  *(uint8_t*)0x20010db2 = 0x7a;
  *(uint8_t*)0x20010db3 = 0;
  *(uint8_t*)0x20010db4 = 0;
  memcpy((void*)0x20010dc0,
         "\xdd\x13\x59\x6b\x28\x24\x8e\x27\x9b\xfd\x57\x25\x4f\x84\xcf\xd0",
         16);
  *(uint8_t*)0x20010dd0 = 0;
  *(uint8_t*)0x20010dd1 = 0;
  *(uint8_t*)0x20010dd2 = 0;
  *(uint8_t*)0x20010dd3 = 0;
  *(uint8_t*)0x20010dd4 = 0;
  *(uint8_t*)0x20010dd5 = 0;
  *(uint8_t*)0x20010dd6 = 0;
  *(uint8_t*)0x20010dd7 = 0;
  *(uint8_t*)0x20010dd8 = 0;
  *(uint8_t*)0x20010dd9 = 0;
  *(uint8_t*)0x20010dda = 0;
  *(uint8_t*)0x20010ddb = 0;
  *(uint8_t*)0x20010ddc = 0;
  *(uint8_t*)0x20010ddd = 0;
  *(uint8_t*)0x20010dde = 0;
  *(uint8_t*)0x20010ddf = 0;
  *(uint8_t*)0x20010de0 = -1;
  *(uint8_t*)0x20010de1 = 0;
  *(uint8_t*)0x20010de2 = 0;
  *(uint8_t*)0x20010de3 = 0;
  *(uint8_t*)0x20010de4 = 0;
  *(uint8_t*)0x20010de5 = 0;
  *(uint8_t*)0x20010de6 = 0;
  *(uint8_t*)0x20010de7 = 0;
  *(uint8_t*)0x20010de8 = 0;
  *(uint8_t*)0x20010de9 = 0;
  *(uint8_t*)0x20010dea = 0;
  *(uint8_t*)0x20010deb = 0;
  *(uint8_t*)0x20010dec = 0;
  *(uint8_t*)0x20010ded = 0;
  *(uint8_t*)0x20010dee = 0;
  *(uint8_t*)0x20010def = 0;
  *(uint16_t*)0x20010df0 = 0x87;
  *(uint8_t*)0x20010df2 = 0;
  *(uint8_t*)0x20010df3 = 5;
  *(uint8_t*)0x20010df4 = 0x6e;
  *(uint32_t*)0x20010df8 = 0;
  *(uint16_t*)0x20010dfc = 0xa8;
  *(uint16_t*)0x20010dfe = 0xd0;
  *(uint32_t*)0x20010e00 = 0;
  *(uint64_t*)0x20010e08 = 0;
  *(uint64_t*)0x20010e10 = 0;
  *(uint16_t*)0x20010e18 = 0x28;
  memcpy((void*)0x20010e1a, "\x4e\x46\x51\x55\x45\x55\x45\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20010e37 = 1;
  *(uint16_t*)0x20010e38 = 0;
  *(uint16_t*)0x20010e3a = 8;
  *(uint8_t*)0x20010e40 = -1;
  *(uint8_t*)0x20010e41 = 1;
  *(uint8_t*)0x20010e42 = 0;
  *(uint8_t*)0x20010e43 = 0;
  *(uint8_t*)0x20010e44 = 0;
  *(uint8_t*)0x20010e45 = 0;
  *(uint8_t*)0x20010e46 = 0;
  *(uint8_t*)0x20010e47 = 0;
  *(uint8_t*)0x20010e48 = 0;
  *(uint8_t*)0x20010e49 = 0;
  *(uint8_t*)0x20010e4a = 0;
  *(uint8_t*)0x20010e4b = 0;
  *(uint8_t*)0x20010e4c = 0;
  *(uint8_t*)0x20010e4d = 0;
  *(uint8_t*)0x20010e4e = 0;
  *(uint8_t*)0x20010e4f = 1;
  *(uint8_t*)0x20010e50 = 0xfe;
  *(uint8_t*)0x20010e51 = 0x80;
  *(uint8_t*)0x20010e52 = 0;
  *(uint8_t*)0x20010e53 = 0;
  *(uint8_t*)0x20010e54 = 0;
  *(uint8_t*)0x20010e55 = 0;
  *(uint8_t*)0x20010e56 = 0;
  *(uint8_t*)0x20010e57 = 0;
  *(uint8_t*)0x20010e58 = 0;
  *(uint8_t*)0x20010e59 = 0;
  *(uint8_t*)0x20010e5a = 0;
  *(uint8_t*)0x20010e5b = 0;
  *(uint8_t*)0x20010e5c = 0;
  *(uint8_t*)0x20010e5d = 0;
  *(uint8_t*)0x20010e5e = 0;
  *(uint8_t*)0x20010e5f = 0xbb;
  *(uint32_t*)0x20010e60 = htobe32(0);
  *(uint32_t*)0x20010e64 = htobe32(0);
  *(uint32_t*)0x20010e68 = htobe32(0);
  *(uint32_t*)0x20010e6c = htobe32(0);
  *(uint32_t*)0x20010e70 = htobe32(0);
  *(uint32_t*)0x20010e74 = htobe32(0);
  *(uint32_t*)0x20010e78 = htobe32(0);
  *(uint32_t*)0x20010e7c = htobe32(0);
  *(uint8_t*)0x20010e80 = 0x73;
  *(uint8_t*)0x20010e81 = 0x79;
  *(uint8_t*)0x20010e82 = 0x7a;
  *(uint8_t*)0x20010e83 = 0;
  *(uint8_t*)0x20010e84 = 0;
  memcpy((void*)0x20010e90,
         "\x34\x45\x56\x59\x41\x58\x95\x32\xe5\xa0\x69\x58\xc0\xf0\xd2\xc4",
         16);
  *(uint8_t*)0x20010ea0 = 0;
  *(uint8_t*)0x20010ea1 = 0;
  *(uint8_t*)0x20010ea2 = 0;
  *(uint8_t*)0x20010ea3 = 0;
  *(uint8_t*)0x20010ea4 = 0;
  *(uint8_t*)0x20010ea5 = 0;
  *(uint8_t*)0x20010ea6 = 0;
  *(uint8_t*)0x20010ea7 = 0;
  *(uint8_t*)0x20010ea8 = 0;
  *(uint8_t*)0x20010ea9 = 0;
  *(uint8_t*)0x20010eaa = 0;
  *(uint8_t*)0x20010eab = 0;
  *(uint8_t*)0x20010eac = 0;
  *(uint8_t*)0x20010ead = 0;
  *(uint8_t*)0x20010eae = 0;
  *(uint8_t*)0x20010eaf = 0;
  *(uint8_t*)0x20010eb0 = 0;
  *(uint8_t*)0x20010eb1 = 0;
  *(uint8_t*)0x20010eb2 = 0;
  *(uint8_t*)0x20010eb3 = 0;
  *(uint8_t*)0x20010eb4 = 0;
  *(uint8_t*)0x20010eb5 = 0;
  *(uint8_t*)0x20010eb6 = 0;
  *(uint8_t*)0x20010eb7 = 0;
  *(uint8_t*)0x20010eb8 = 0;
  *(uint8_t*)0x20010eb9 = 0;
  *(uint8_t*)0x20010eba = 0;
  *(uint8_t*)0x20010ebb = 0;
  *(uint8_t*)0x20010ebc = 0;
  *(uint8_t*)0x20010ebd = 0;
  *(uint8_t*)0x20010ebe = 0;
  *(uint8_t*)0x20010ebf = 0;
  *(uint16_t*)0x20010ec0 = 0;
  *(uint8_t*)0x20010ec2 = 0;
  *(uint8_t*)0x20010ec3 = 0;
  *(uint8_t*)0x20010ec4 = 0;
  *(uint32_t*)0x20010ec8 = 0;
  *(uint16_t*)0x20010ecc = 0xa8;
  *(uint16_t*)0x20010ece = 0xf0;
  *(uint32_t*)0x20010ed0 = 0;
  *(uint64_t*)0x20010ed8 = 0;
  *(uint64_t*)0x20010ee0 = 0;
  *(uint16_t*)0x20010ee8 = 0x48;
  memcpy((void*)0x20010eea, "\x54\x45\x45\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20010f07 = 1;
  *(uint8_t*)0x20010f08 = 0xac;
  *(uint8_t*)0x20010f09 = 0x14;
  *(uint8_t*)0x20010f0a = 0;
  *(uint8_t*)0x20010f0b = 0xaa;
  *(uint8_t*)0x20010f18 = 0x73;
  *(uint8_t*)0x20010f19 = 0x79;
  *(uint8_t*)0x20010f1a = 0x7a;
  *(uint8_t*)0x20010f1b = 0;
  *(uint8_t*)0x20010f1c = 0;
  *(uint64_t*)0x20010f28 = 0;
  *(uint8_t*)0x20010f30 = 0;
  *(uint8_t*)0x20010f31 = 0;
  *(uint8_t*)0x20010f32 = 0;
  *(uint8_t*)0x20010f33 = 0;
  *(uint8_t*)0x20010f34 = 0;
  *(uint8_t*)0x20010f35 = 0;
  *(uint8_t*)0x20010f36 = 0;
  *(uint8_t*)0x20010f37 = 0;
  *(uint8_t*)0x20010f38 = 0;
  *(uint8_t*)0x20010f39 = 0;
  *(uint8_t*)0x20010f3a = 0;
  *(uint8_t*)0x20010f3b = 0;
  *(uint8_t*)0x20010f3c = 0;
  *(uint8_t*)0x20010f3d = 0;
  *(uint8_t*)0x20010f3e = 0;
  *(uint8_t*)0x20010f3f = 0;
  *(uint8_t*)0x20010f40 = 0;
  *(uint8_t*)0x20010f41 = 0;
  *(uint8_t*)0x20010f42 = 0;
  *(uint8_t*)0x20010f43 = 0;
  *(uint8_t*)0x20010f44 = 0;
  *(uint8_t*)0x20010f45 = 0;
  *(uint8_t*)0x20010f46 = 0;
  *(uint8_t*)0x20010f47 = 0;
  *(uint8_t*)0x20010f48 = 0;
  *(uint8_t*)0x20010f49 = 0;
  *(uint8_t*)0x20010f4a = 0;
  *(uint8_t*)0x20010f4b = 0;
  *(uint8_t*)0x20010f4c = 0;
  *(uint8_t*)0x20010f4d = 0;
  *(uint8_t*)0x20010f4e = 0;
  *(uint8_t*)0x20010f4f = 0;
  *(uint8_t*)0x20010f50 = 0;
  *(uint8_t*)0x20010f51 = 0;
  *(uint8_t*)0x20010f52 = 0;
  *(uint8_t*)0x20010f53 = 0;
  *(uint8_t*)0x20010f54 = 0;
  *(uint8_t*)0x20010f55 = 0;
  *(uint8_t*)0x20010f56 = 0;
  *(uint8_t*)0x20010f57 = 0;
  *(uint8_t*)0x20010f58 = 0;
  *(uint8_t*)0x20010f59 = 0;
  *(uint8_t*)0x20010f5a = 0;
  *(uint8_t*)0x20010f5b = 0;
  *(uint8_t*)0x20010f5c = 0;
  *(uint8_t*)0x20010f5d = 0;
  *(uint8_t*)0x20010f5e = 0;
  *(uint8_t*)0x20010f5f = 0;
  *(uint8_t*)0x20010f60 = 0;
  *(uint8_t*)0x20010f61 = 0;
  *(uint8_t*)0x20010f62 = 0;
  *(uint8_t*)0x20010f63 = 0;
  *(uint8_t*)0x20010f64 = 0;
  *(uint8_t*)0x20010f65 = 0;
  *(uint8_t*)0x20010f66 = 0;
  *(uint8_t*)0x20010f67 = 0;
  *(uint8_t*)0x20010f68 = 0;
  *(uint8_t*)0x20010f69 = 0;
  *(uint8_t*)0x20010f6a = 0;
  *(uint8_t*)0x20010f6b = 0;
  *(uint8_t*)0x20010f6c = 0;
  *(uint8_t*)0x20010f6d = 0;
  *(uint8_t*)0x20010f6e = 0;
  *(uint8_t*)0x20010f6f = 0;
  *(uint8_t*)0x20010f70 = 0;
  *(uint8_t*)0x20010f71 = 0;
  *(uint8_t*)0x20010f72 = 0;
  *(uint8_t*)0x20010f73 = 0;
  *(uint8_t*)0x20010f74 = 0;
  *(uint8_t*)0x20010f75 = 0;
  *(uint8_t*)0x20010f76 = 0;
  *(uint8_t*)0x20010f77 = 0;
  *(uint8_t*)0x20010f78 = 0;
  *(uint8_t*)0x20010f79 = 0;
  *(uint8_t*)0x20010f7a = 0;
  *(uint8_t*)0x20010f7b = 0;
  *(uint8_t*)0x20010f7c = 0;
  *(uint8_t*)0x20010f7d = 0;
  *(uint8_t*)0x20010f7e = 0;
  *(uint8_t*)0x20010f7f = 0;
  *(uint8_t*)0x20010f80 = 0;
  *(uint8_t*)0x20010f81 = 0;
  *(uint8_t*)0x20010f82 = 0;
  *(uint8_t*)0x20010f83 = 0;
  *(uint8_t*)0x20010f84 = 0;
  *(uint8_t*)0x20010f85 = 0;
  *(uint8_t*)0x20010f86 = 0;
  *(uint8_t*)0x20010f87 = 0;
  *(uint8_t*)0x20010f88 = 0;
  *(uint8_t*)0x20010f89 = 0;
  *(uint8_t*)0x20010f8a = 0;
  *(uint8_t*)0x20010f8b = 0;
  *(uint8_t*)0x20010f8c = 0;
  *(uint8_t*)0x20010f8d = 0;
  *(uint8_t*)0x20010f8e = 0;
  *(uint8_t*)0x20010f8f = 0;
  *(uint8_t*)0x20010f90 = 0;
  *(uint8_t*)0x20010f91 = 0;
  *(uint8_t*)0x20010f92 = 0;
  *(uint8_t*)0x20010f93 = 0;
  *(uint8_t*)0x20010f94 = 0;
  *(uint8_t*)0x20010f95 = 0;
  *(uint8_t*)0x20010f96 = 0;
  *(uint8_t*)0x20010f97 = 0;
  *(uint8_t*)0x20010f98 = 0;
  *(uint8_t*)0x20010f99 = 0;
  *(uint8_t*)0x20010f9a = 0;
  *(uint8_t*)0x20010f9b = 0;
  *(uint8_t*)0x20010f9c = 0;
  *(uint8_t*)0x20010f9d = 0;
  *(uint8_t*)0x20010f9e = 0;
  *(uint8_t*)0x20010f9f = 0;
  *(uint8_t*)0x20010fa0 = 0;
  *(uint8_t*)0x20010fa1 = 0;
  *(uint8_t*)0x20010fa2 = 0;
  *(uint8_t*)0x20010fa3 = 0;
  *(uint8_t*)0x20010fa4 = 0;
  *(uint8_t*)0x20010fa5 = 0;
  *(uint8_t*)0x20010fa6 = 0;
  *(uint8_t*)0x20010fa7 = 0;
  *(uint8_t*)0x20010fa8 = 0;
  *(uint8_t*)0x20010fa9 = 0;
  *(uint8_t*)0x20010faa = 0;
  *(uint8_t*)0x20010fab = 0;
  *(uint8_t*)0x20010fac = 0;
  *(uint8_t*)0x20010fad = 0;
  *(uint8_t*)0x20010fae = 0;
  *(uint8_t*)0x20010faf = 0;
  *(uint8_t*)0x20010fb0 = 0;
  *(uint8_t*)0x20010fb1 = 0;
  *(uint8_t*)0x20010fb2 = 0;
  *(uint8_t*)0x20010fb3 = 0;
  *(uint8_t*)0x20010fb4 = 0;
  *(uint8_t*)0x20010fb5 = 0;
  *(uint8_t*)0x20010fb6 = 0;
  *(uint8_t*)0x20010fb7 = 0;
  *(uint32_t*)0x20010fb8 = 0;
  *(uint16_t*)0x20010fbc = 0xa8;
  *(uint16_t*)0x20010fbe = 0xd0;
  *(uint32_t*)0x20010fc0 = 0;
  *(uint64_t*)0x20010fc8 = 0;
  *(uint64_t*)0x20010fd0 = 0;
  *(uint16_t*)0x20010fd8 = 0x28;
  memcpy((void*)0x20010fda, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20010ff7 = 0;
  *(uint32_t*)0x20010ff8 = 0xfffffffe;
  syscall(__NR_setsockopt, r[1], 0x29, 0x40, 0x20010c40, 0x3c0);
}

int main()
{
  for (;;) {
    loop();
  }
}
