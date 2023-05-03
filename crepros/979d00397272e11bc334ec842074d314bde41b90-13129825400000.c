// https://syzkaller.appspot.com/bug?id=979d00397272e11bc334ec842074d314bde41b90
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if_arp.h>
#include <sched.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/tcp.h>

static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* ctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  if (__atomic_load_n(&skip_segv, __ATOMIC_RELAXED) &&
      (addr < prog_start || addr > prog_end)) {
    _longjmp(segv_env, 1);
  }
  exit(sig);
}

static void install_segv_handler(void)
{
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_IGN;
  syscall(SYS_rt_sigaction, 0x20, &sa, NULL, 8);
  syscall(SYS_rt_sigaction, 0x21, &sa, NULL, 8);
  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = segv_handler;
  sa.sa_flags = SA_NODEFER | SA_SIGINFO;
  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGBUS, &sa, NULL);
}

#define NONFAILING(...)                                                        \
  {                                                                            \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    if (_setjmp(segv_env) == 0) {                                              \
      __VA_ARGS__;                                                             \
    }                                                                          \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
  }

static void sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
}

static uint64_t current_time_ms(void)
{
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    exit(1);
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void vsnprintf_check(char* str, size_t size, const char* format,
                            va_list args)
{
  int rv;
  rv = vsnprintf(str, size, format, args);
  if (rv < 0)
    exit(1);
  if ((size_t)rv >= size)
    exit(1);
}

#define COMMAND_MAX_LEN 128
#define PATH_PREFIX                                                            \
  "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin "
#define PATH_PREFIX_LEN (sizeof(PATH_PREFIX) - 1)

static void execute_command(bool panic, const char* format, ...)
{
  va_list args;
  char command[PATH_PREFIX_LEN + COMMAND_MAX_LEN];
  int rv;
  va_start(args, format);
  memcpy(command, PATH_PREFIX, PATH_PREFIX_LEN);
  vsnprintf_check(command + PATH_PREFIX_LEN, COMMAND_MAX_LEN, format, args);
  va_end(args);
  rv = system(command);
  if (rv) {
    if (panic)
      exit(1);
  }
}

#define DEV_IPV4 "172.20.20.%d"
#define DEV_IPV6 "fe80::%02hx"
#define DEV_MAC "aa:aa:aa:aa:aa:%02hx"

static void snprintf_check(char* str, size_t size, const char* format, ...)
{
  va_list args;
  va_start(args, format);
  vsnprintf_check(str, size, format, args);
  va_end(args);
}
static void initialize_netdevices(void)
{
  unsigned i;
  const char* devtypes[] = {"ip6gretap", "bridge", "vcan", "bond", "team"};
  const char* devnames[] = {"lo",
                            "sit0",
                            "bridge0",
                            "vcan0",
                            "tunl0",
                            "gre0",
                            "gretap0",
                            "ip_vti0",
                            "ip6_vti0",
                            "ip6tnl0",
                            "ip6gre0",
                            "ip6gretap0",
                            "erspan0",
                            "bond0",
                            "veth0",
                            "veth1",
                            "team0",
                            "veth0_to_bridge",
                            "veth1_to_bridge",
                            "veth0_to_bond",
                            "veth1_to_bond",
                            "veth0_to_team",
                            "veth1_to_team"};
  const char* devmasters[] = {"bridge", "bond", "team"};
  for (i = 0; i < sizeof(devtypes) / (sizeof(devtypes[0])); i++)
    execute_command(0, "ip link add dev %s0 type %s", devtypes[i], devtypes[i]);
  execute_command(0, "ip link add type veth");
  for (i = 0; i < sizeof(devmasters) / (sizeof(devmasters[0])); i++) {
    execute_command(
        0, "ip link add name %s_slave_0 type veth peer name veth0_to_%s",
        devmasters[i], devmasters[i]);
    execute_command(
        0, "ip link add name %s_slave_1 type veth peer name veth1_to_%s",
        devmasters[i], devmasters[i]);
    execute_command(0, "ip link set %s_slave_0 master %s0", devmasters[i],
                    devmasters[i]);
    execute_command(0, "ip link set %s_slave_1 master %s0", devmasters[i],
                    devmasters[i]);
    execute_command(0, "ip link set veth0_to_%s up", devmasters[i]);
    execute_command(0, "ip link set veth1_to_%s up", devmasters[i]);
  }
  execute_command(0, "ip link set bridge_slave_0 up");
  execute_command(0, "ip link set bridge_slave_1 up");
  for (i = 0; i < sizeof(devnames) / (sizeof(devnames[0])); i++) {
    char addr[32];
    snprintf_check(addr, sizeof(addr), DEV_IPV4, i + 10);
    execute_command(0, "ip -4 addr add %s/24 dev %s", addr, devnames[i]);
    snprintf_check(addr, sizeof(addr), DEV_IPV6, i + 10);
    execute_command(0, "ip -6 addr add %s/120 dev %s", addr, devnames[i]);
    snprintf_check(addr, sizeof(addr), DEV_MAC, i + 10);
    execute_command(0, "ip link set dev %s address %s", devnames[i], addr);
    execute_command(0, "ip link set dev %s up", devnames[i]);
  }
}

static void setup_common()
{
  if (mount(0, "/sys/fs/fuse/connections", "fusectl", 0, 0)) {
  }
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();
  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = 200 << 20;
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 32 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 136 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 256;
  setrlimit(RLIMIT_NOFILE, &rlim);
  if (unshare(CLONE_NEWNS)) {
  }
  if (unshare(CLONE_NEWIPC)) {
  }
  if (unshare(0x02000000)) {
  }
  if (unshare(CLONE_NEWUTS)) {
  }
  if (unshare(CLONE_SYSVSEM)) {
  }
}

int wait_for_loop(int pid)
{
  if (pid < 0)
    exit(1);
  int status = 0;
  while (waitpid(-1, &status, __WALL) != pid) {
  }
  return WEXITSTATUS(status);
}

static int do_sandbox_none(void)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid != 0)
    return wait_for_loop(pid);
  setup_common();
  sandbox_common();
  if (unshare(CLONE_NEWNET)) {
  }
  initialize_netdevices();
  loop();
  exit(1);
}

static void kill_and_wait(int pid, int* status)
{
  kill(-pid, SIGKILL);
  kill(pid, SIGKILL);
  int i;
  for (i = 0; i < 100; i++) {
    if (waitpid(-1, status, WNOHANG | __WALL) == pid)
      return;
    usleep(1000);
  }
  DIR* dir = opendir("/sys/fs/fuse/connections");
  if (dir) {
    for (;;) {
      struct dirent* ent = readdir(dir);
      if (!ent)
        break;
      if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        continue;
      char abort[300];
      snprintf(abort, sizeof(abort), "/sys/fs/fuse/connections/%s/abort",
               ent->d_name);
      int fd = open(abort, O_WRONLY);
      if (fd == -1) {
        continue;
      }
      if (write(fd, abort, 1) < 0) {
      }
      close(fd);
    }
    closedir(dir);
  } else {
  }
  while (waitpid(-1, status, __WALL) != pid) {
  }
}

#define SYZ_HAVE_SETUP_TEST 1
static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
}

#define SYZ_HAVE_RESET_TEST 1
static void reset_test()
{
  int fd;
  for (fd = 3; fd < 30; fd++)
    close(fd);
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter;
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      setup_test();
      execute_one();
      reset_test();
      exit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      if (waitpid(-1, &status, WNOHANG | WAIT_FLAGS) == pid)
        break;
      sleep_ms(1);
      if (current_time_ms() - start < 5 * 1000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}
#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff};

void execute_one(void)
{
  long res = 0;
  res = syscall(__NR_socket, 0xa, 1, 0);
  if (res != -1)
    r[0] = res;
  NONFAILING(*(uint32_t*)0x200000c0 = 1);
  syscall(__NR_setsockopt, r[0], 6, 0x13, 0x200000c0, 0x1d4);
  NONFAILING(*(uint16_t*)0x20000080 = 0xa);
  NONFAILING(*(uint16_t*)0x20000082 = htobe16(0));
  NONFAILING(*(uint32_t*)0x20000084 = 0);
  NONFAILING(*(uint8_t*)0x20000088 = 0);
  NONFAILING(*(uint8_t*)0x20000089 = 0);
  NONFAILING(*(uint8_t*)0x2000008a = 0);
  NONFAILING(*(uint8_t*)0x2000008b = 0);
  NONFAILING(*(uint8_t*)0x2000008c = 0);
  NONFAILING(*(uint8_t*)0x2000008d = 0);
  NONFAILING(*(uint8_t*)0x2000008e = 0);
  NONFAILING(*(uint8_t*)0x2000008f = 0);
  NONFAILING(*(uint8_t*)0x20000090 = 0);
  NONFAILING(*(uint8_t*)0x20000091 = 0);
  NONFAILING(*(uint8_t*)0x20000092 = 0);
  NONFAILING(*(uint8_t*)0x20000093 = 0);
  NONFAILING(*(uint8_t*)0x20000094 = 0);
  NONFAILING(*(uint8_t*)0x20000095 = 0);
  NONFAILING(*(uint8_t*)0x20000096 = 0);
  NONFAILING(*(uint8_t*)0x20000097 = 0);
  NONFAILING(*(uint32_t*)0x20000098 = 0);
  syscall(__NR_connect, r[0], 0x20000080, 0x1c);
  NONFAILING(memcpy((void*)0x20006580, "tls", 4));
  syscall(__NR_setsockopt, r[0], 6, 0x1f, 0x20006580, 4);
  NONFAILING(*(uint16_t*)0x20000100 = 0x303);
  NONFAILING(*(uint16_t*)0x20000102 = 0x33);
  syscall(__NR_setsockopt, r[0], 0x11a, 1, 0x20000100, 0x28);
  NONFAILING(memcpy((void*)0x200002c0, "\xb7", 1));
  NONFAILING(*(uint16_t*)0x20000340 = 0xa);
  NONFAILING(*(uint16_t*)0x20000342 = htobe16(0));
  NONFAILING(*(uint32_t*)0x20000344 = 0);
  NONFAILING(*(uint8_t*)0x20000348 = -1);
  NONFAILING(*(uint8_t*)0x20000349 = 1);
  NONFAILING(*(uint8_t*)0x2000034a = 0);
  NONFAILING(*(uint8_t*)0x2000034b = 0);
  NONFAILING(*(uint8_t*)0x2000034c = 0);
  NONFAILING(*(uint8_t*)0x2000034d = 0);
  NONFAILING(*(uint8_t*)0x2000034e = 0);
  NONFAILING(*(uint8_t*)0x2000034f = 0);
  NONFAILING(*(uint8_t*)0x20000350 = 0);
  NONFAILING(*(uint8_t*)0x20000351 = 0);
  NONFAILING(*(uint8_t*)0x20000352 = 0);
  NONFAILING(*(uint8_t*)0x20000353 = 0);
  NONFAILING(*(uint8_t*)0x20000354 = 0);
  NONFAILING(*(uint8_t*)0x20000355 = 0);
  NONFAILING(*(uint8_t*)0x20000356 = 0);
  NONFAILING(*(uint8_t*)0x20000357 = 1);
  NONFAILING(*(uint32_t*)0x20000358 = 0);
  syscall(__NR_sendto, r[0], 0x200002c0, 1, 0x8000, 0x20000340, 0x1c);
  NONFAILING(*(uint32_t*)0x20000080 = 0x56);
  syscall(__NR_getsockopt, -1, 0x29, 0x36, 0x20000000, 0x20000080);
  NONFAILING(*(uint32_t*)0x20000000 = 0);
  NONFAILING(*(uint16_t*)0x20000004 = 2);
  NONFAILING(*(uint16_t*)0x20000006 = htobe16(0));
  NONFAILING(*(uint32_t*)0x20000008 = htobe32(0));
  NONFAILING(*(uint8_t*)0x2000000c = 0);
  NONFAILING(*(uint8_t*)0x2000000d = 0);
  NONFAILING(*(uint8_t*)0x2000000e = 0);
  NONFAILING(*(uint8_t*)0x2000000f = 0);
  NONFAILING(*(uint8_t*)0x20000010 = 0);
  NONFAILING(*(uint8_t*)0x20000011 = 0);
  NONFAILING(*(uint8_t*)0x20000012 = 0);
  NONFAILING(*(uint8_t*)0x20000013 = 0);
  NONFAILING(*(uint32_t*)0x20000084 = 0);
  NONFAILING(*(uint16_t*)0x20000088 = 0);
  NONFAILING(*(uint32_t*)0x2000008a = 0);
  NONFAILING(*(uint32_t*)0x2000008e = 0);
  NONFAILING(*(uint32_t*)0x20000092 = 0);
  NONFAILING(*(uint32_t*)0x200000c0 = 0x98);
  syscall(__NR_getsockopt, -1, 0x84, 9, 0x20000000, 0x200000c0);
  NONFAILING(memcpy(
      (void*)0x200007c0,
      "\x64\x75\x6d\x6d\x79\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16));
  NONFAILING(*(uint32_t*)0x200007d0 = 0);
  syscall(__NR_ioctl, -1, 0x8933, 0x200007c0);
  syscall(__NR_listen, -1, 0);
  NONFAILING(*(uint32_t*)0x20000000 = 1);
  NONFAILING(*(uint32_t*)0x20000004 = 0);
  NONFAILING(*(uint64_t*)0x20000008 = 0x20001000);
  NONFAILING(*(uint64_t*)0x20000010 = 0x20000100);
  NONFAILING(memcpy((void*)0x20000100, "GPL", 4));
  NONFAILING(*(uint32_t*)0x20000018 = 0);
  NONFAILING(*(uint32_t*)0x2000001c = 0);
  NONFAILING(*(uint64_t*)0x20000020 = 0);
  NONFAILING(*(uint32_t*)0x20000028 = 0);
  NONFAILING(*(uint32_t*)0x2000002c = 0);
  NONFAILING(*(uint8_t*)0x20000030 = 0);
  NONFAILING(*(uint8_t*)0x20000031 = 0);
  NONFAILING(*(uint8_t*)0x20000032 = 0);
  NONFAILING(*(uint8_t*)0x20000033 = 0);
  NONFAILING(*(uint8_t*)0x20000034 = 0);
  NONFAILING(*(uint8_t*)0x20000035 = 0);
  NONFAILING(*(uint8_t*)0x20000036 = 0);
  NONFAILING(*(uint8_t*)0x20000037 = 0);
  NONFAILING(*(uint8_t*)0x20000038 = 0);
  NONFAILING(*(uint8_t*)0x20000039 = 0);
  NONFAILING(*(uint8_t*)0x2000003a = 0);
  NONFAILING(*(uint8_t*)0x2000003b = 0);
  NONFAILING(*(uint8_t*)0x2000003c = 0);
  NONFAILING(*(uint8_t*)0x2000003d = 0);
  NONFAILING(*(uint8_t*)0x2000003e = 0);
  NONFAILING(*(uint8_t*)0x2000003f = 0);
  NONFAILING(*(uint32_t*)0x20000040 = 0);
  NONFAILING(*(uint32_t*)0x20000044 = 0);
  syscall(__NR_bpf, 5, 0x20000000, 0x48);
  syscall(__NR_mmap, 0x20000000, 0xe7e000, 0, 0x40031, -1, 0);
  syscall(__NR_mmap, 0x200be000, 0x3000, 2, 0x100132, -1, 0);
  res = syscall(__NR_socket, 0x10, 3, 0);
  if (res != -1)
    r[1] = res;
  NONFAILING(*(uint64_t*)0x200003c0 = 0x20000040);
  NONFAILING(*(uint16_t*)0x20000040 = 0x10);
  NONFAILING(*(uint16_t*)0x20000042 = 0);
  NONFAILING(*(uint32_t*)0x20000044 = 0);
  NONFAILING(*(uint32_t*)0x20000048 = 0);
  NONFAILING(*(uint32_t*)0x200003c8 = 0xc);
  NONFAILING(*(uint64_t*)0x200003d0 = 0x20000380);
  NONFAILING(*(uint64_t*)0x20000380 = 0x20000300);
  NONFAILING(*(uint16_t*)0x20000300 = r[1]);
  NONFAILING(*(uint64_t*)0x20000388 = 1);
  NONFAILING(*(uint64_t*)0x200003d8 = 1);
  NONFAILING(*(uint64_t*)0x200003e0 = 0);
  NONFAILING(*(uint64_t*)0x200003e8 = 0);
  NONFAILING(*(uint32_t*)0x200003f0 = 0);
  syscall(__NR_sendmsg, r[1], 0x200003c0, 0);
  syscall(__NR_socket, 0xa, 2, 0x88);
  res = syscall(__NR_socket, 0xa, 1, 0);
  if (res != -1)
    r[2] = res;
  NONFAILING(*(uint32_t*)0x20000000 = 0);
  NONFAILING(memcpy(
      (void*)0x20000004,
      "\x62\x70\x71\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16));
  NONFAILING(*(uint32_t*)0x20000014 = 4);
  syscall(__NR_setsockopt, -1, 0, 0x48c, 0x20000000, 8);
  syscall(__NR_ioctl, r[2], 0x8916, 0x20000000);
  syscall(__NR_ioctl, r[2], 0x100000008936, 0x20000000);
  syscall(__NR_socket, 0xa, 2, 0);
  NONFAILING(*(uint32_t*)0x20000000 = 0);
  NONFAILING(*(uint32_t*)0x20000004 = 0);
  NONFAILING(*(uint32_t*)0x20000008 = 0);
  NONFAILING(*(uint32_t*)0x2000000c = 0);
  NONFAILING(*(uint32_t*)0x20000010 = 0);
  NONFAILING(*(uint32_t*)0x20000014 = 0);
  syscall(__NR_ioctl, -1, 0x541b, 0x20000040);
  res = syscall(__NR_socket, 0x26, 5, 0);
  if (res != -1)
    r[3] = res;
  NONFAILING(*(uint16_t*)0x20000000 = 0x26);
  NONFAILING(memcpy((void*)0x20000002,
                    "\x61\x65\x61\x64\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                    14));
  NONFAILING(*(uint32_t*)0x20000010 = 0);
  NONFAILING(*(uint32_t*)0x20000014 = 0);
  NONFAILING(memcpy((void*)0x20000018,
                    "\x70\x63\x72\x79\x70\x74\x28\x70\x63\x72\x79\x70\x74\x28"
                    "\x67\x63\x6d\x5f\x62\x61\x73\x65\x28\x63\x74\x72\x28\x61"
                    "\x65\x73\x2d\x61\x65\x73\x6e\x69\x29\x2c\x67\x68\x61\x73"
                    "\x68\x2d\x67\x65\x6e\x65\x72\x69\x63\x29\x29\x29\x00\x00"
                    "\x00\x00\x00\x00\x00\x00\x00\x00",
                    64));
  syscall(__NR_bind, r[3], 0x20000000, 0x58);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  install_segv_handler();
  do_sandbox_none();
  return 0;
}
