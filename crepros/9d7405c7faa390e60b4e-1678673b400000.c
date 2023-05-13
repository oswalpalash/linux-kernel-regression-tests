// https://syzkaller.appspot.com/bug?id=b7575418eb496c7def8d76caff2c1c8755682e90
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if_arp.h>
#include <sched.h>
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
#include <unistd.h>

#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/tcp.h>

unsigned long long procid;

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
  char netdevsim[16];
  sprintf(netdevsim, "netdevsim%d", (int)procid);
  const char* devtypes[] = {"ip6gretap", "ip6erspan", "bridge", "vcan",
                            "bond",      "team",      "dummy",  "nlmon",
                            "caif",      "batadv"};
  const char* devmasters[] = {"bridge", "bond", "team"};
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
                            "veth1_to_team",
                            "veth0_to_hsr",
                            "veth1_to_hsr",
                            "hsr0",
                            "ip6erspan0",
                            "dummy0",
                            "nlmon0",
                            "vxcan1",
                            "caif0",
                            "batadv0",
                            netdevsim};
  unsigned i;
  for (i = 0; i < sizeof(devtypes) / (sizeof(devtypes[0])); i++)
    execute_command(0, "ip link add dev %s0 type %s", devtypes[i], devtypes[i]);
  execute_command(0, "ip link add dev vxcan1 type vxcan");
  execute_command(0, "ip link add dev %s type netdevsim", netdevsim);
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
  }
  execute_command(0, "ip link set bridge_slave_0 up");
  execute_command(0, "ip link set bridge_slave_1 up");
  execute_command(
      0, "ip link add name hsr_slave_0 type veth peer name veth0_to_hsr");
  execute_command(
      0, "ip link add name hsr_slave_1 type veth peer name veth1_to_hsr");
  execute_command(
      0, "ip link add dev hsr0 type hsr slave1 hsr_slave_0 slave2 hsr_slave_1");
  execute_command(0, "ip link set hsr_slave_0 up");
  execute_command(0, "ip link set hsr_slave_1 up");
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
static void initialize_netdevices_init(void)
{
  int pid = procid;
  execute_command(0, "ip link set dev nr%d address bb:bb:bb:bb:bb:00:%02hx",
                  pid, pid);
  execute_command(0, "ip -4 addr add 172.30.00.%d/24 dev nr%d", pid + 1, pid);
  execute_command(0, "ip -6 addr add fe88::00:%02hx/120 dev nr%d", pid + 1,
                  pid);
  execute_command(0, "ip link set dev nr%d up", pid);
  execute_command(0, "ip link set dev rose%d address bb:bb:bb:01:%02hx", pid,
                  pid);
  execute_command(0, "ip -4 addr add 172.30.01.%d/24 dev rose%d", pid + 1, pid);
  execute_command(0, "ip -6 addr add fe88::01:%02hx/120 dev rose%d", pid + 1,
                  pid);
}

static bool write_file(const char* file, const char* what, ...)
{
  char buf[1024];
  va_list args;
  va_start(args, what);
  vsnprintf(buf, sizeof(buf), what, args);
  va_end(args);
  buf[sizeof(buf) - 1] = 0;
  int len = strlen(buf);
  int fd = open(file, O_WRONLY | O_CLOEXEC);
  if (fd == -1)
    return false;
  if (write(fd, buf, len) != len) {
    int err = errno;
    close(fd);
    errno = err;
    return false;
  }
  close(fd);
  return true;
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
  typedef struct {
    const char* name;
    const char* value;
  } sysctl_t;
  static const sysctl_t sysctls[] = {
      {"/proc/sys/kernel/shmmax", "16777216"},
      {"/proc/sys/kernel/shmall", "536870912"},
      {"/proc/sys/kernel/shmmni", "1024"},
      {"/proc/sys/kernel/msgmax", "8192"},
      {"/proc/sys/kernel/msgmni", "1024"},
      {"/proc/sys/kernel/msgmnb", "1024"},
      {"/proc/sys/kernel/sem", "1024 1048576 500 1024"},
  };
  unsigned i;
  for (i = 0; i < sizeof(sysctls) / sizeof(sysctls[0]); i++) {
    if (!write_file(sysctls[i].name, sysctls[i].value)) {
    }
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
  initialize_netdevices_init();
  if (unshare(CLONE_NEWNET)) {
  }
  initialize_netdevices();
  loop();
  exit(1);
}

uint64_t r[2] = {0xffffffffffffffff, 0x0};

void loop(void)
{
  long res = 0;
  res = syscall(__NR_socket, 0x11, 0x100000000000003, 0x300);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000200, "batadv0\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16);
  *(uint32_t*)0x20000210 = 0;
  res = syscall(__NR_ioctl, r[0], 0x8933, 0x20000200);
  if (res != -1)
    r[1] = *(uint32_t*)0x20000210;
  *(uint32_t*)0x20000100 = 4;
  syscall(__NR_setsockopt, r[0], 0x107, 0x14, 0x20000100, 4);
  *(uint16_t*)0x20000040 = 0x11;
  *(uint16_t*)0x20000042 = htobe16(0);
  *(uint32_t*)0x20000044 = r[1];
  *(uint16_t*)0x20000048 = 1;
  *(uint8_t*)0x2000004a = 0;
  *(uint8_t*)0x2000004b = 6;
  memcpy((void*)0x2000004c, "\x32\xca\xe4\x78\x3d\x32", 6);
  *(uint8_t*)0x20000052 = 0;
  *(uint8_t*)0x20000053 = 0;
  syscall(__NR_bind, r[0], 0x20000040, 0x14);
  memcpy((void*)0x20000180,
         "\x05\x03\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\xc5\x2c", 14);
  syscall(__NR_sendto, r[0], 0x20000180, 0xe, 0, 0, 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  do_sandbox_none();
  return 0;
}
