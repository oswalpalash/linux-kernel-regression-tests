// https://syzkaller.appspot.com/bug?id=75100d84503cde672c1c1071efecd5c0fa1da69c
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <errno.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/futex.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/if_arp.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <signal.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/wait.h>
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

static void vsnprintf_check(char* str, size_t size, const char* format,
                            va_list args)
{
  int rv;

  rv = vsnprintf(str, size, format, args);
  if (rv < 0)
    fail("tun: snprintf failed");
  if ((size_t)rv >= size)
    fail("tun: string '%s...' doesn't fit into buffer", str);
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
      fail("command '%s' failed: %d", &command[0], rv);
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

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();

  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = 160 << 20;
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 8 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 136 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);

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
    fail("sandbox fork failed");
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

  sandbox_common();
  if (unshare(CLONE_NEWNET)) {
  }
  initialize_netdevices();
  loop();
  doexit(1);
}

static void execute_one();
extern unsigned long long procid;

static void loop()
{
  int iter;
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      fail("clone failed");
    if (pid == 0) {
      prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
      setpgrp();
      execute_one();
      int fd;
      for (fd = 3; fd < 30; fd++)
        close(fd);
      doexit(0);
    }

    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      int res = waitpid(-1, &status, __WALL | WNOHANG);
      if (res == pid) {
        break;
      }
      usleep(1000);
      if (current_time_ms() - start < 5 * 1000)
        continue;
      kill(-pid, SIGKILL);
      kill(pid, SIGKILL);
      while (waitpid(-1, &status, __WALL) != pid) {
      }
      break;
    }
  }
}

struct thread_t {
  int created, running, call;
  pthread_t th;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    while (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE))
      syscall(SYS_futex, &th->running, FUTEX_WAIT, 0, 0);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    __atomic_store_n(&th->running, 0, __ATOMIC_RELEASE);
    syscall(SYS_futex, &th->running, FUTEX_WAKE);
  }
  return 0;
}

static void execute(int num_calls)
{
  int call, thread;
  running = 0;
  for (call = 0; call < num_calls; call++) {
    for (thread = 0; thread < sizeof(threads) / sizeof(threads[0]); thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 128 << 10);
        pthread_create(&th->th, &attr, thr, th);
      }
      if (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE)) {
        th->call = call;
        __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
        __atomic_store_n(&th->running, 1, __ATOMIC_RELEASE);
        syscall(SYS_futex, &th->running, FUTEX_WAKE);
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 20 * 1000 * 1000;
        syscall(SYS_futex, &th->running, FUTEX_WAIT, 1, &ts);
        if (__atomic_load_n(&running, __ATOMIC_RELAXED))
          usleep((call == num_calls - 1) ? 10000 : 1000);
        break;
      }
    }
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    res = syscall(__NR_socket, 0xa, 2, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    *(uint16_t*)0x20000080 = 0xa;
    *(uint16_t*)0x20000082 = htobe16(0);
    *(uint32_t*)0x20000084 = 0;
    *(uint8_t*)0x20000088 = 0xfe;
    *(uint8_t*)0x20000089 = 0x80;
    *(uint8_t*)0x2000008a = 0;
    *(uint8_t*)0x2000008b = 0;
    *(uint8_t*)0x2000008c = 0;
    *(uint8_t*)0x2000008d = 0;
    *(uint8_t*)0x2000008e = 0;
    *(uint8_t*)0x2000008f = 0;
    *(uint8_t*)0x20000090 = 0;
    *(uint8_t*)0x20000091 = 0;
    *(uint8_t*)0x20000092 = 0;
    *(uint8_t*)0x20000093 = 0;
    *(uint8_t*)0x20000094 = 0;
    *(uint8_t*)0x20000095 = 0;
    *(uint8_t*)0x20000096 = 0;
    *(uint8_t*)0x20000097 = 0xbb;
    *(uint32_t*)0x20000098 = 0xb;
    syscall(__NR_connect, r[0], 0x20000080, 0x1c);
    break;
  case 2:
    *(uint16_t*)0x200000c0 = 0x27;
    *(uint32_t*)0x200000c4 = 0;
    *(uint32_t*)0x200000c8 = 0;
    *(uint32_t*)0x200000cc = 5;
    *(uint8_t*)0x200000d0 = 0x51;
    *(uint8_t*)0x200000d1 = 1;
    memcpy((void*)0x200000d2,
           "\xfc\xc6\x13\xf7\xf6\xca\x68\x50\x77\xa4\xd8\xb9\x15\xb8\xee\xe6"
           "\x43\x48\xa6\xc7\xef\x28\xc8\x45\x06\xff\x2c\xed\x97\xa6\x75\xc6"
           "\x16\x47\x1c\xbc\x55\xd6\x18\x58\x4e\x73\x6d\xc9\xf3\x77\x87\x43"
           "\x81\x91\xc0\x05\xbc\xf7\xeb\x05\xea\x5c\x79\x25\x12\xc5\x9a",
           63);
    *(uint64_t*)0x20000118 = 0xb;
    syscall(__NR_recvfrom, r[0], 0x20000000, 0x36, 0x40000000, 0x200000c0,
            0x80);
    break;
  case 3:
    res = syscall(__NR_socket, 0x18, 1, 1);
    if (res != -1)
      r[1] = res;
    break;
  case 4:
    *(uint16_t*)0x205fafd2 = 0x18;
    *(uint32_t*)0x205fafd4 = 1;
    *(uint32_t*)0x205fafd8 = 0;
    *(uint32_t*)0x205fafdc = r[0];
    *(uint16_t*)0x205fafe0 = 2;
    *(uint16_t*)0x205fafe2 = htobe16(0);
    *(uint32_t*)0x205fafe4 = htobe32(0xe0000002);
    *(uint8_t*)0x205fafe8 = 0;
    *(uint8_t*)0x205fafe9 = 0;
    *(uint8_t*)0x205fafea = 0;
    *(uint8_t*)0x205fafeb = 0;
    *(uint8_t*)0x205fafec = 0;
    *(uint8_t*)0x205fafed = 0;
    *(uint8_t*)0x205fafee = 0;
    *(uint8_t*)0x205fafef = 0;
    *(uint32_t*)0x205faff0 = 4;
    *(uint32_t*)0x205faff4 = 0;
    *(uint32_t*)0x205faff8 = 0;
    *(uint32_t*)0x205faffc = 0;
    syscall(__NR_connect, r[1], 0x205fafd2, 0x2e);
    break;
  case 5:
    *(uint64_t*)0x20005fc0 = 0x20005680;
    *(uint16_t*)0x20005680 = 0x1f;
    *(uint8_t*)0x20005682 = 0;
    *(uint8_t*)0x20005683 = 0;
    *(uint8_t*)0x20005684 = 0;
    *(uint8_t*)0x20005685 = 0;
    *(uint8_t*)0x20005686 = 0;
    *(uint8_t*)0x20005687 = 0;
    *(uint32_t*)0x20005fc8 = 0x80;
    *(uint64_t*)0x20005fd0 = 0x20005b00;
    *(uint64_t*)0x20005fd8 = 0;
    *(uint64_t*)0x20005fe0 = 0;
    *(uint64_t*)0x20005fe8 = 0;
    *(uint32_t*)0x20005ff0 = 0;
    *(uint32_t*)0x20005ff8 = 0;
    *(uint64_t*)0x20006000 = 0x20005b80;
    *(uint16_t*)0x20005b80 = 0x1f;
    *(uint16_t*)0x20005b82 = 0;
    *(uint8_t*)0x20005b84 = 0;
    *(uint8_t*)0x20005b85 = 0;
    *(uint8_t*)0x20005b86 = 0;
    *(uint8_t*)0x20005b87 = 0;
    *(uint8_t*)0x20005b88 = 0;
    *(uint8_t*)0x20005b89 = 0;
    *(uint16_t*)0x20005b8a = 0;
    *(uint8_t*)0x20005b8c = 0;
    *(uint32_t*)0x20006008 = 0x80;
    *(uint64_t*)0x20006010 = 0x20005c40;
    *(uint64_t*)0x20006018 = 0x1f4;
    *(uint64_t*)0x20006020 = 0x20005c80;
    *(uint64_t*)0x20006028 = 0x3a00;
    *(uint32_t*)0x20006030 = 0;
    *(uint32_t*)0x20006038 = 0;
    syscall(__NR_sendmmsg, r[1], 0x20005fc0, 0x3e8, 0);
    break;
  case 6:
    syscall(__NR_sendmmsg, r[0], 0x20001b00, 0xb8, 0);
    break;
  }
}

void execute_one()
{
  execute(7);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    do_sandbox_none();
  }
}
