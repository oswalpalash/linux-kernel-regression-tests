// https://syzkaller.appspot.com/bug?id=ddc83e209f712ce63078e146f7c0fe63e1edbc2f
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
  syscall(__NR_mmap, 0x20000000, 0xe79000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 0xa, 2, 0);
  *(uint16_t*)0x20e78fe4 = 0xa;
  *(uint16_t*)0x20e78fe6 = htobe16(0x4e20);
  *(uint32_t*)0x20e78fe8 = 0xfff;
  *(uint8_t*)0x20e78fec = 0;
  *(uint8_t*)0x20e78fed = 0;
  *(uint8_t*)0x20e78fee = 0;
  *(uint8_t*)0x20e78fef = 0;
  *(uint8_t*)0x20e78ff0 = 0;
  *(uint8_t*)0x20e78ff1 = 0;
  *(uint8_t*)0x20e78ff2 = 0;
  *(uint8_t*)0x20e78ff3 = 0;
  *(uint8_t*)0x20e78ff4 = 0;
  *(uint8_t*)0x20e78ff5 = 0;
  *(uint8_t*)0x20e78ff6 = -1;
  *(uint8_t*)0x20e78ff7 = -1;
  *(uint8_t*)0x20e78ff8 = 0xac;
  *(uint8_t*)0x20e78ff9 = 0x14;
  *(uint8_t*)0x20e78ffa = 0;
  *(uint8_t*)0x20e78ffb = 0xaa;
  *(uint32_t*)0x20e78ffc = 0;
  syscall(__NR_bind, r[0], 0x20e78fe4, 0x1c);
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
}

int main()
{
  for (;;) {
    loop();
  }
}
