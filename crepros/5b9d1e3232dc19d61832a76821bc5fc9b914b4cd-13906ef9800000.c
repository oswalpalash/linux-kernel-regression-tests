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
  r[0] = syscall(__NR_socket, 2, 3, 3);
  memcpy((void*)0x20002d08, "\x66\x69\x6c\x74\x65\x72\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20002d28 = 0xe;
  *(uint32_t*)0x20002d2c = 4;
  *(uint32_t*)0x20002d30 = 0x298;
  *(uint32_t*)0x20002d34 = -1;
  *(uint32_t*)0x20002d38 = 0;
  *(uint32_t*)0x20002d3c = 0;
  *(uint32_t*)0x20002d40 = 0;
  *(uint32_t*)0x20002d44 = -1;
  *(uint32_t*)0x20002d48 = -1;
  *(uint32_t*)0x20002d4c = 0;
  *(uint32_t*)0x20002d50 = 0;
  *(uint32_t*)0x20002d54 = 0;
  *(uint32_t*)0x20002d58 = -1;
  *(uint32_t*)0x20002d5c = 4;
  *(uint64_t*)0x20002d60 = 0x20001000;
  *(uint8_t*)0x20002d68 = 0;
  *(uint8_t*)0x20002d69 = 0;
  *(uint8_t*)0x20002d6a = 0;
  *(uint8_t*)0x20002d6b = 0;
  *(uint8_t*)0x20002d6c = 0;
  *(uint8_t*)0x20002d6d = 0;
  *(uint8_t*)0x20002d6e = 0;
  *(uint8_t*)0x20002d6f = 0;
  *(uint8_t*)0x20002d70 = 0;
  *(uint8_t*)0x20002d71 = 0;
  *(uint8_t*)0x20002d72 = 0;
  *(uint8_t*)0x20002d73 = 0;
  *(uint8_t*)0x20002d74 = 0;
  *(uint8_t*)0x20002d75 = 0;
  *(uint8_t*)0x20002d76 = 0;
  *(uint8_t*)0x20002d77 = 0;
  *(uint8_t*)0x20002d78 = 0;
  *(uint8_t*)0x20002d79 = 0;
  *(uint8_t*)0x20002d7a = 0;
  *(uint8_t*)0x20002d7b = 0;
  *(uint8_t*)0x20002d7c = 0;
  *(uint8_t*)0x20002d7d = 0;
  *(uint8_t*)0x20002d7e = 0;
  *(uint8_t*)0x20002d7f = 0;
  *(uint8_t*)0x20002d80 = 0;
  *(uint8_t*)0x20002d81 = 0;
  *(uint8_t*)0x20002d82 = 0;
  *(uint8_t*)0x20002d83 = 0;
  *(uint8_t*)0x20002d84 = 0;
  *(uint8_t*)0x20002d85 = 0;
  *(uint8_t*)0x20002d86 = 0;
  *(uint8_t*)0x20002d87 = 0;
  *(uint8_t*)0x20002d88 = 0;
  *(uint8_t*)0x20002d89 = 0;
  *(uint8_t*)0x20002d8a = 0;
  *(uint8_t*)0x20002d8b = 0;
  *(uint8_t*)0x20002d8c = 0;
  *(uint8_t*)0x20002d8d = 0;
  *(uint8_t*)0x20002d8e = 0;
  *(uint8_t*)0x20002d8f = 0;
  *(uint8_t*)0x20002d90 = 0;
  *(uint8_t*)0x20002d91 = 0;
  *(uint8_t*)0x20002d92 = 0;
  *(uint8_t*)0x20002d93 = 0;
  *(uint8_t*)0x20002d94 = 0;
  *(uint8_t*)0x20002d95 = 0;
  *(uint8_t*)0x20002d96 = 0;
  *(uint8_t*)0x20002d97 = 0;
  *(uint8_t*)0x20002d98 = 0;
  *(uint8_t*)0x20002d99 = 0;
  *(uint8_t*)0x20002d9a = 0;
  *(uint8_t*)0x20002d9b = 0;
  *(uint8_t*)0x20002d9c = 0;
  *(uint8_t*)0x20002d9d = 0;
  *(uint8_t*)0x20002d9e = 0;
  *(uint8_t*)0x20002d9f = 0;
  *(uint8_t*)0x20002da0 = 0;
  *(uint8_t*)0x20002da1 = 0;
  *(uint8_t*)0x20002da2 = 0;
  *(uint8_t*)0x20002da3 = 0;
  *(uint8_t*)0x20002da4 = 0;
  *(uint8_t*)0x20002da5 = 0;
  *(uint8_t*)0x20002da6 = 0;
  *(uint8_t*)0x20002da7 = 0;
  *(uint8_t*)0x20002da8 = 0;
  *(uint8_t*)0x20002da9 = 0;
  *(uint8_t*)0x20002daa = 0;
  *(uint8_t*)0x20002dab = 0;
  *(uint8_t*)0x20002dac = 0;
  *(uint8_t*)0x20002dad = 0;
  *(uint8_t*)0x20002dae = 0;
  *(uint8_t*)0x20002daf = 0;
  *(uint8_t*)0x20002db0 = 0;
  *(uint8_t*)0x20002db1 = 0;
  *(uint8_t*)0x20002db2 = 0;
  *(uint8_t*)0x20002db3 = 0;
  *(uint8_t*)0x20002db4 = 0;
  *(uint8_t*)0x20002db5 = 0;
  *(uint8_t*)0x20002db6 = 0;
  *(uint8_t*)0x20002db7 = 0;
  *(uint8_t*)0x20002db8 = 0;
  *(uint8_t*)0x20002db9 = 0;
  *(uint8_t*)0x20002dba = 0;
  *(uint8_t*)0x20002dbb = 0;
  *(uint32_t*)0x20002dbc = 0;
  *(uint16_t*)0x20002dc0 = 0x70;
  *(uint16_t*)0x20002dc2 = 0x98;
  *(uint32_t*)0x20002dc4 = 0;
  *(uint64_t*)0x20002dc8 = 0;
  *(uint64_t*)0x20002dd0 = 0;
  *(uint16_t*)0x20002dd8 = 0x28;
  memcpy((void*)0x20002dda, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20002df7 = 0;
  *(uint32_t*)0x20002df8 = 0xfffffffe;
  *(uint8_t*)0x20002e00 = 0;
  *(uint8_t*)0x20002e01 = 0;
  *(uint8_t*)0x20002e02 = 0;
  *(uint8_t*)0x20002e03 = 0;
  *(uint8_t*)0x20002e04 = 0;
  *(uint8_t*)0x20002e05 = 0;
  *(uint8_t*)0x20002e06 = 0;
  *(uint8_t*)0x20002e07 = 0;
  *(uint8_t*)0x20002e08 = 0;
  *(uint8_t*)0x20002e09 = 0;
  *(uint8_t*)0x20002e0a = 0;
  *(uint8_t*)0x20002e0b = 0;
  *(uint8_t*)0x20002e0c = 0;
  *(uint8_t*)0x20002e0d = 0;
  *(uint8_t*)0x20002e0e = 0;
  *(uint8_t*)0x20002e0f = 0;
  *(uint8_t*)0x20002e10 = 0;
  *(uint8_t*)0x20002e11 = 0;
  *(uint8_t*)0x20002e12 = 0;
  *(uint8_t*)0x20002e13 = 0;
  *(uint8_t*)0x20002e14 = 0;
  *(uint8_t*)0x20002e15 = 0;
  *(uint8_t*)0x20002e16 = 0;
  *(uint8_t*)0x20002e17 = 0;
  *(uint8_t*)0x20002e18 = 0;
  *(uint8_t*)0x20002e19 = 0;
  *(uint8_t*)0x20002e1a = 0;
  *(uint8_t*)0x20002e1b = 0;
  *(uint8_t*)0x20002e1c = 0;
  *(uint8_t*)0x20002e1d = 0;
  *(uint8_t*)0x20002e1e = 0;
  *(uint8_t*)0x20002e1f = 0;
  *(uint8_t*)0x20002e20 = 0;
  *(uint8_t*)0x20002e21 = 0;
  *(uint8_t*)0x20002e22 = 0;
  *(uint8_t*)0x20002e23 = 0;
  *(uint8_t*)0x20002e24 = 0;
  *(uint8_t*)0x20002e25 = 0;
  *(uint8_t*)0x20002e26 = 0;
  *(uint8_t*)0x20002e27 = 0;
  *(uint8_t*)0x20002e28 = 0;
  *(uint8_t*)0x20002e29 = 0;
  *(uint8_t*)0x20002e2a = 0;
  *(uint8_t*)0x20002e2b = 0;
  *(uint8_t*)0x20002e2c = 0;
  *(uint8_t*)0x20002e2d = 0;
  *(uint8_t*)0x20002e2e = 0;
  *(uint8_t*)0x20002e2f = 0;
  *(uint8_t*)0x20002e30 = 0;
  *(uint8_t*)0x20002e31 = 0;
  *(uint8_t*)0x20002e32 = 0;
  *(uint8_t*)0x20002e33 = 0;
  *(uint8_t*)0x20002e34 = 0;
  *(uint8_t*)0x20002e35 = 0;
  *(uint8_t*)0x20002e36 = 0;
  *(uint8_t*)0x20002e37 = 0;
  *(uint8_t*)0x20002e38 = 0;
  *(uint8_t*)0x20002e39 = 0;
  *(uint8_t*)0x20002e3a = 0;
  *(uint8_t*)0x20002e3b = 0;
  *(uint8_t*)0x20002e3c = 0;
  *(uint8_t*)0x20002e3d = 0;
  *(uint8_t*)0x20002e3e = 0;
  *(uint8_t*)0x20002e3f = 0;
  *(uint8_t*)0x20002e40 = 0;
  *(uint8_t*)0x20002e41 = 0;
  *(uint8_t*)0x20002e42 = 0;
  *(uint8_t*)0x20002e43 = 0;
  *(uint8_t*)0x20002e44 = 0;
  *(uint8_t*)0x20002e45 = 0;
  *(uint8_t*)0x20002e46 = 0;
  *(uint8_t*)0x20002e47 = 0;
  *(uint8_t*)0x20002e48 = 0;
  *(uint8_t*)0x20002e49 = 0;
  *(uint8_t*)0x20002e4a = 0;
  *(uint8_t*)0x20002e4b = 0;
  *(uint8_t*)0x20002e4c = 0;
  *(uint8_t*)0x20002e4d = 0;
  *(uint8_t*)0x20002e4e = 0;
  *(uint8_t*)0x20002e4f = 0;
  *(uint8_t*)0x20002e50 = 0;
  *(uint8_t*)0x20002e51 = 0;
  *(uint8_t*)0x20002e52 = 0;
  *(uint8_t*)0x20002e53 = 0;
  *(uint32_t*)0x20002e54 = 0;
  *(uint16_t*)0x20002e58 = 0x70;
  *(uint16_t*)0x20002e5a = 0x98;
  *(uint32_t*)0x20002e5c = 0;
  *(uint64_t*)0x20002e60 = 0;
  *(uint64_t*)0x20002e68 = 0;
  *(uint16_t*)0x20002e70 = 0x28;
  memcpy((void*)0x20002e72, "\x52\x45\x4a\x45\x43\x54\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20002e8f = 0;
  *(uint32_t*)0x20002e90 = 0;
  *(uint32_t*)0x20002e98 = htobe32(-1);
  *(uint8_t*)0x20002e9c = 0xac;
  *(uint8_t*)0x20002e9d = 0x14;
  *(uint8_t*)0x20002e9e = 0;
  *(uint8_t*)0x20002e9f = 0xaa;
  *(uint32_t*)0x20002ea0 = htobe32(0);
  *(uint32_t*)0x20002ea4 = htobe32(-1);
  memcpy((void*)0x20002ea8,
         "\x73\x69\x74\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  memcpy((void*)0x20002eb8,
         "\x65\x71\x6c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  *(uint8_t*)0x20002ec8 = 0;
  *(uint8_t*)0x20002ec9 = 0;
  *(uint8_t*)0x20002eca = 0;
  *(uint8_t*)0x20002ecb = 0;
  *(uint8_t*)0x20002ecc = 0;
  *(uint8_t*)0x20002ecd = 0;
  *(uint8_t*)0x20002ece = 0;
  *(uint8_t*)0x20002ecf = 0;
  *(uint8_t*)0x20002ed0 = 0;
  *(uint8_t*)0x20002ed1 = 0;
  *(uint8_t*)0x20002ed2 = 0;
  *(uint8_t*)0x20002ed3 = 0;
  *(uint8_t*)0x20002ed4 = 0;
  *(uint8_t*)0x20002ed5 = 0;
  *(uint8_t*)0x20002ed6 = 0;
  *(uint8_t*)0x20002ed7 = 0;
  *(uint8_t*)0x20002ed8 = 0;
  *(uint8_t*)0x20002ed9 = 0;
  *(uint8_t*)0x20002eda = 0;
  *(uint8_t*)0x20002edb = 0;
  *(uint8_t*)0x20002edc = 0;
  *(uint8_t*)0x20002edd = 0;
  *(uint8_t*)0x20002ede = 0;
  *(uint8_t*)0x20002edf = 0;
  *(uint8_t*)0x20002ee0 = 0;
  *(uint8_t*)0x20002ee1 = 0;
  *(uint8_t*)0x20002ee2 = 0;
  *(uint8_t*)0x20002ee3 = 0;
  *(uint8_t*)0x20002ee4 = 0;
  *(uint8_t*)0x20002ee5 = 0;
  *(uint8_t*)0x20002ee6 = 0;
  *(uint8_t*)0x20002ee7 = 0;
  *(uint16_t*)0x20002ee8 = 0;
  *(uint8_t*)0x20002eea = 0;
  *(uint8_t*)0x20002eeb = 0;
  *(uint32_t*)0x20002eec = 0;
  *(uint16_t*)0x20002ef0 = 0x70;
  *(uint16_t*)0x20002ef2 = 0xd0;
  *(uint32_t*)0x20002ef4 = 0;
  *(uint64_t*)0x20002ef8 = 0;
  *(uint64_t*)0x20002f00 = 0;
  *(uint16_t*)0x20002f08 = 0x60;
  memcpy((void*)0x20002f0a, "\x43\x4c\x55\x53\x54\x45\x52\x49\x50\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20002f27 = 0;
  *(uint32_t*)0x20002f28 = 3;
  *(uint8_t*)0x20002f2c = 1;
  *(uint8_t*)0x20002f2d = 0x80;
  *(uint8_t*)0x20002f2e = 0xc2;
  *(uint8_t*)0x20002f2f = 0;
  *(uint8_t*)0x20002f30 = 0;
  *(uint8_t*)0x20002f31 = 0;
  *(uint16_t*)0x20002f32 = 0;
  *(uint16_t*)0x20002f34 = 2;
  *(uint16_t*)0x20002f36 = 4;
  *(uint16_t*)0x20002f38 = 2;
  *(uint16_t*)0x20002f3a = 0;
  *(uint16_t*)0x20002f3c = 0;
  *(uint16_t*)0x20002f3e = 0;
  *(uint16_t*)0x20002f40 = 0;
  *(uint16_t*)0x20002f42 = 0;
  *(uint16_t*)0x20002f44 = 0;
  *(uint16_t*)0x20002f46 = 0;
  *(uint16_t*)0x20002f48 = 0;
  *(uint16_t*)0x20002f4a = 0;
  *(uint16_t*)0x20002f4c = 0;
  *(uint16_t*)0x20002f4e = 0;
  *(uint16_t*)0x20002f50 = 0;
  *(uint16_t*)0x20002f52 = 0;
  *(uint16_t*)0x20002f54 = 0;
  *(uint32_t*)0x20002f58 = 0;
  *(uint32_t*)0x20002f5c = 0;
  *(uint64_t*)0x20002f60 = 0;
  *(uint8_t*)0x20002f68 = 0;
  *(uint8_t*)0x20002f69 = 0;
  *(uint8_t*)0x20002f6a = 0;
  *(uint8_t*)0x20002f6b = 0;
  *(uint8_t*)0x20002f6c = 0;
  *(uint8_t*)0x20002f6d = 0;
  *(uint8_t*)0x20002f6e = 0;
  *(uint8_t*)0x20002f6f = 0;
  *(uint8_t*)0x20002f70 = 0;
  *(uint8_t*)0x20002f71 = 0;
  *(uint8_t*)0x20002f72 = 0;
  *(uint8_t*)0x20002f73 = 0;
  *(uint8_t*)0x20002f74 = 0;
  *(uint8_t*)0x20002f75 = 0;
  *(uint8_t*)0x20002f76 = 0;
  *(uint8_t*)0x20002f77 = 0;
  *(uint8_t*)0x20002f78 = 0;
  *(uint8_t*)0x20002f79 = 0;
  *(uint8_t*)0x20002f7a = 0;
  *(uint8_t*)0x20002f7b = 0;
  *(uint8_t*)0x20002f7c = 0;
  *(uint8_t*)0x20002f7d = 0;
  *(uint8_t*)0x20002f7e = 0;
  *(uint8_t*)0x20002f7f = 0;
  *(uint8_t*)0x20002f80 = 0;
  *(uint8_t*)0x20002f81 = 0;
  *(uint8_t*)0x20002f82 = 0;
  *(uint8_t*)0x20002f83 = 0;
  *(uint8_t*)0x20002f84 = 0;
  *(uint8_t*)0x20002f85 = 0;
  *(uint8_t*)0x20002f86 = 0;
  *(uint8_t*)0x20002f87 = 0;
  *(uint8_t*)0x20002f88 = 0;
  *(uint8_t*)0x20002f89 = 0;
  *(uint8_t*)0x20002f8a = 0;
  *(uint8_t*)0x20002f8b = 0;
  *(uint8_t*)0x20002f8c = 0;
  *(uint8_t*)0x20002f8d = 0;
  *(uint8_t*)0x20002f8e = 0;
  *(uint8_t*)0x20002f8f = 0;
  *(uint8_t*)0x20002f90 = 0;
  *(uint8_t*)0x20002f91 = 0;
  *(uint8_t*)0x20002f92 = 0;
  *(uint8_t*)0x20002f93 = 0;
  *(uint8_t*)0x20002f94 = 0;
  *(uint8_t*)0x20002f95 = 0;
  *(uint8_t*)0x20002f96 = 0;
  *(uint8_t*)0x20002f97 = 0;
  *(uint8_t*)0x20002f98 = 0;
  *(uint8_t*)0x20002f99 = 0;
  *(uint8_t*)0x20002f9a = 0;
  *(uint8_t*)0x20002f9b = 0;
  *(uint8_t*)0x20002f9c = 0;
  *(uint8_t*)0x20002f9d = 0;
  *(uint8_t*)0x20002f9e = 0;
  *(uint8_t*)0x20002f9f = 0;
  *(uint8_t*)0x20002fa0 = 0;
  *(uint8_t*)0x20002fa1 = 0;
  *(uint8_t*)0x20002fa2 = 0;
  *(uint8_t*)0x20002fa3 = 0;
  *(uint8_t*)0x20002fa4 = 0;
  *(uint8_t*)0x20002fa5 = 0;
  *(uint8_t*)0x20002fa6 = 0;
  *(uint8_t*)0x20002fa7 = 0;
  *(uint8_t*)0x20002fa8 = 0;
  *(uint8_t*)0x20002fa9 = 0;
  *(uint8_t*)0x20002faa = 0;
  *(uint8_t*)0x20002fab = 0;
  *(uint8_t*)0x20002fac = 0;
  *(uint8_t*)0x20002fad = 0;
  *(uint8_t*)0x20002fae = 0;
  *(uint8_t*)0x20002faf = 0;
  *(uint8_t*)0x20002fb0 = 0;
  *(uint8_t*)0x20002fb1 = 0;
  *(uint8_t*)0x20002fb2 = 0;
  *(uint8_t*)0x20002fb3 = 0;
  *(uint8_t*)0x20002fb4 = 0;
  *(uint8_t*)0x20002fb5 = 0;
  *(uint8_t*)0x20002fb6 = 0;
  *(uint8_t*)0x20002fb7 = 0;
  *(uint8_t*)0x20002fb8 = 0;
  *(uint8_t*)0x20002fb9 = 0;
  *(uint8_t*)0x20002fba = 0;
  *(uint8_t*)0x20002fbb = 0;
  *(uint32_t*)0x20002fbc = 0;
  *(uint16_t*)0x20002fc0 = 0x70;
  *(uint16_t*)0x20002fc2 = 0x98;
  *(uint32_t*)0x20002fc4 = 0;
  *(uint64_t*)0x20002fc8 = 0;
  *(uint64_t*)0x20002fd0 = 0;
  *(uint16_t*)0x20002fd8 = 0x28;
  memcpy((void*)0x20002fda, "\x52\x45\x4a\x45\x43\x54\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20002ff7 = 0;
  *(uint32_t*)0x20002ff8 = 0;
  *(uint64_t*)0x20001000 = 0;
  *(uint64_t*)0x20001008 = 0;
  *(uint64_t*)0x20001010 = 0;
  *(uint64_t*)0x20001018 = 0;
  *(uint64_t*)0x20001020 = 0;
  *(uint64_t*)0x20001028 = 0;
  *(uint64_t*)0x20001030 = 0;
  *(uint64_t*)0x20001038 = 0;
  syscall(__NR_setsockopt, r[0], 0, 0x40, 0x20002d08, 0x2f8);
  r[1] = syscall(__NR_socket, 2, 1, 0);
  *(uint32_t*)0x200f8ff4 = htobe32(0xe0000001);
  *(uint32_t*)0x200f8ff8 = htobe32(0x7f000001);
  *(uint32_t*)0x200f8ffc = htobe32(-1);
  syscall(__NR_setsockopt, r[1], 0, 0x27, 0x200f8ff4, 0xc);
}

int main()
{
  for (;;) {
    loop();
  }
}
