// https://syzkaller.appspot.com/bug?id=f0d852f7d588e6fdfe7054c82e92aacde61fa227
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if_arp.h>
#include <pthread.h>
#include <sched.h>
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

#include <linux/futex.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/tcp.h>

unsigned long long procid;

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

static void thread_start(void* (*fn)(void*), void* arg)
{
  pthread_t th;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 128 << 10);
  if (pthread_create(&th, &attr, fn, arg))
    exit(1);
  pthread_attr_destroy(&attr);
}

#define BITMASK_LEN(type, bf_len) (type)((1ull << (bf_len)) - 1)

#define BITMASK_LEN_OFF(type, bf_off, bf_len)                                  \
  (type)(BITMASK_LEN(type, (bf_len)) << (bf_off))

#define STORE_BY_BITMASK(type, addr, val, bf_off, bf_len)                      \
  if ((bf_off) == 0 && (bf_len) == 0) {                                        \
    *(type*)(addr) = (type)(val);                                              \
  } else {                                                                     \
    type new_val = *(type*)(addr);                                             \
    new_val &= ~BITMASK_LEN_OFF(type, (bf_off), (bf_len));                     \
    new_val |= ((type)(val)&BITMASK_LEN(type, (bf_len))) << (bf_off);          \
    *(type*)(addr) = new_val;                                                  \
  }

typedef struct {
  int state;
} event_t;

static void event_init(event_t* ev)
{
  ev->state = 0;
}

static void event_reset(event_t* ev)
{
  ev->state = 0;
}

static void event_set(event_t* ev)
{
  if (ev->state)
    exit(1);
  __atomic_store_n(&ev->state, 1, __ATOMIC_RELEASE);
  syscall(SYS_futex, &ev->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG);
}

static void event_wait(event_t* ev)
{
  while (!__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, 0);
}

static int event_isset(event_t* ev)
{
  return __atomic_load_n(&ev->state, __ATOMIC_ACQUIRE);
}

static int event_timedwait(event_t* ev, uint64_t timeout)
{
  uint64_t start = current_time_ms();
  uint64_t now = start;
  for (;;) {
    uint64_t remain = timeout - (now - start);
    struct timespec ts;
    ts.tv_sec = remain / 1000;
    ts.tv_nsec = (remain % 1000) * 1000 * 1000;
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, &ts);
    if (__atomic_load_n(&ev->state, __ATOMIC_RELAXED))
      return 1;
    now = current_time_ms();
    if (now - start > timeout)
      return 0;
  }
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

struct thread_t {
  int created, call;
  event_t ready, done;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    event_wait(&th->ready);
    event_reset(&th->ready);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    event_set(&th->done);
  }
  return 0;
}

static void execute_one(void)
{
  int i, call, thread;
  for (call = 0; call < 33; call++) {
    for (thread = 0; thread < (int)(sizeof(threads) / sizeof(threads[0]));
         thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        event_init(&th->ready);
        event_init(&th->done);
        event_set(&th->done);
        thread_start(thr, th);
      }
      if (!event_isset(&th->done))
        continue;
      event_reset(&th->done);
      th->call = call;
      __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
      event_set(&th->ready);
      event_timedwait(&th->done, 45);
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
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

uint64_t r[10] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                  0xffffffffffffffff};

void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    syscall(__NR_mmap, 0x200be000, 0x3000, 2, 0x100132, -1, 0);
    break;
  case 1:
    res = syscall(__NR_socket, 0x10, 3, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 2:
    *(uint64_t*)0x20000080 = 0x20000000;
    *(uint16_t*)0x20000000 = 0x10;
    *(uint16_t*)0x20000002 = 0;
    *(uint32_t*)0x20000004 = 0;
    *(uint32_t*)0x20000008 = 0;
    *(uint32_t*)0x20000088 = 0xc;
    *(uint64_t*)0x20000090 = 0x20000180;
    *(uint64_t*)0x20000180 = 0x20000100;
    *(uint32_t*)0x20000100 = 0x40;
    *(uint16_t*)0x20000104 = 0x10;
    *(uint16_t*)0x20000106 = 0xe3b;
    *(uint32_t*)0x20000108 = 0;
    *(uint32_t*)0x2000010c = 0;
    *(uint8_t*)0x20000110 = 0;
    *(uint8_t*)0x20000111 = 0;
    *(uint16_t*)0x20000112 = 0;
    *(uint32_t*)0x20000114 = 0;
    *(uint32_t*)0x20000118 = 0x243;
    *(uint32_t*)0x2000011c = 0;
    *(uint16_t*)0x20000120 = 8;
    *(uint16_t*)0x20000122 = 0xa;
    *(uint32_t*)0x20000124 = 6;
    *(uint16_t*)0x20000128 = 0x18;
    *(uint16_t*)0x2000012a = 0x12;
    *(uint16_t*)0x2000012c = 8;
    *(uint16_t*)0x2000012e = 1;
    memcpy((void*)0x20000130, "vti", 4);
    *(uint16_t*)0x20000134 = 0xc;
    *(uint16_t*)0x20000136 = 2;
    *(uint16_t*)0x20000138 = 8;
    *(uint16_t*)0x2000013a = 4;
    *(uint32_t*)0x2000013c = htobe32(0xe0000002);
    *(uint64_t*)0x20000188 = 0x40;
    *(uint64_t*)0x20000098 = 1;
    *(uint64_t*)0x200000a0 = 0;
    *(uint64_t*)0x200000a8 = 0;
    *(uint32_t*)0x200000b0 = 0x4000000;
    syscall(__NR_sendmsg, r[0], 0x20000080, 0x800);
    break;
  case 3:
    *(uint32_t*)0x20001b00 = 9;
    *(uint32_t*)0x20001b04 = 0x1d;
    *(uint32_t*)0x20001b08 = 6;
    *(uint32_t*)0x20001b0c = 9;
    *(uint32_t*)0x20001b10 = 2;
    *(uint32_t*)0x20001b14 = -1;
    *(uint32_t*)0x20001b18 = 0;
    *(uint8_t*)0x20001b1c = 0;
    *(uint8_t*)0x20001b1d = 0;
    *(uint8_t*)0x20001b1e = 0;
    *(uint8_t*)0x20001b1f = 0;
    *(uint8_t*)0x20001b20 = 0;
    *(uint8_t*)0x20001b21 = 0;
    *(uint8_t*)0x20001b22 = 0;
    *(uint8_t*)0x20001b23 = 0;
    *(uint8_t*)0x20001b24 = 0;
    *(uint8_t*)0x20001b25 = 0;
    *(uint8_t*)0x20001b26 = 0;
    *(uint8_t*)0x20001b27 = 0;
    *(uint8_t*)0x20001b28 = 0;
    *(uint8_t*)0x20001b29 = 0;
    *(uint8_t*)0x20001b2a = 0;
    *(uint8_t*)0x20001b2b = 0;
    res = syscall(__NR_bpf, 0, 0x20001b00, 0x269);
    if (res != -1)
      r[1] = res;
    break;
  case 4:
    *(uint32_t*)0x20000080 = r[1];
    *(uint64_t*)0x20000088 = 0x20000000;
    *(uint64_t*)0x20000090 = 0x20000040;
    *(uint64_t*)0x20000098 = 0;
    syscall(__NR_bpf, 2, 0x20000080, 0x20);
    break;
  case 5:
    *(uint32_t*)0x20002540 = r[1];
    *(uint64_t*)0x20002548 = 0x20000340;
    *(uint64_t*)0x20002550 = 0x20002440;
    *(uint64_t*)0x20002558 = 0;
    syscall(__NR_bpf, 2, 0x20002540, 0x20);
    break;
  case 6:
    *(uint32_t*)0x20000800 = r[1];
    *(uint64_t*)0x20000808 = 0x20000640;
    *(uint64_t*)0x20000810 = 0x20000700;
    *(uint64_t*)0x20000818 = 0;
    syscall(__NR_bpf, 2, 0x20000800, 0x20);
    break;
  case 7:
    *(uint32_t*)0x20000040 = r[1];
    *(uint64_t*)0x20000048 = 0x20000280;
    *(uint64_t*)0x20000050 = 0x20000400;
    *(uint64_t*)0x20000058 = 0;
    syscall(__NR_bpf, 2, 0x20000040, 0x20);
    break;
  case 8:
    *(uint32_t*)0x20000180 = r[1];
    *(uint64_t*)0x20000188 = 0x200000c0;
    *(uint64_t*)0x20000190 = 0x200002c0;
    *(uint64_t*)0x20000198 = 0;
    syscall(__NR_bpf, 2, 0x20000180, 0x20);
    break;
  case 9:
    *(uint32_t*)0x20000000 = r[1];
    *(uint64_t*)0x20000008 = 0x20000200;
    *(uint64_t*)0x20000010 = 0x20001440;
    *(uint64_t*)0x20000018 = 0;
    syscall(__NR_bpf, 2, 0x20000000, 0x20);
    break;
  case 10:
    *(uint32_t*)0x20000140 = r[1];
    *(uint64_t*)0x20000148 = 0x20000380;
    *(uint64_t*)0x20000150 = 0x200000c0;
    *(uint64_t*)0x20000158 = 0;
    syscall(__NR_bpf, 2, 0x20000140, 0x20);
    break;
  case 11:
    res = syscall(__NR_socket, 0x18, 1, 1);
    if (res != -1)
      r[2] = res;
    break;
  case 12:
    syscall(__NR_ioctl, r[2], 0x8912, 0x20000280);
    break;
  case 13:
    res = syscall(__NR_socket, 0x10, 3, 0);
    if (res != -1)
      r[3] = res;
    break;
  case 14:
    *(uint64_t*)0x20000180 = 0x20000000;
    *(uint16_t*)0x20000000 = 0x10;
    *(uint16_t*)0x20000002 = 0;
    *(uint32_t*)0x20000004 = 0;
    *(uint32_t*)0x20000008 = 0;
    *(uint32_t*)0x20000188 = 0xc;
    *(uint64_t*)0x20000190 = 0x20000140;
    *(uint64_t*)0x20000140 = 0x200005c0;
    *(uint32_t*)0x200005c0 = 0x3c;
    *(uint16_t*)0x200005c4 = 0x10;
    *(uint16_t*)0x200005c6 = 0x601;
    *(uint32_t*)0x200005c8 = 0;
    *(uint32_t*)0x200005cc = 0;
    *(uint8_t*)0x200005d0 = 0;
    *(uint8_t*)0x200005d1 = 0;
    *(uint16_t*)0x200005d2 = 0;
    *(uint32_t*)0x200005d4 = 0;
    *(uint32_t*)0x200005d8 = 0;
    *(uint32_t*)0x200005dc = 0;
    *(uint16_t*)0x200005e0 = 0x1c;
    *(uint16_t*)0x200005e2 = 0x12;
    *(uint16_t*)0x200005e4 = 0xc;
    *(uint16_t*)0x200005e6 = 1;
    memcpy((void*)0x200005e8, "bond", 5);
    *(uint16_t*)0x200005f0 = 0xc;
    *(uint16_t*)0x200005f2 = 2;
    *(uint16_t*)0x200005f4 = 8;
    *(uint16_t*)0x200005f6 = 0xa;
    *(uint32_t*)0x200005f8 = 0;
    *(uint64_t*)0x20000148 = 0x3c;
    *(uint64_t*)0x20000198 = 1;
    *(uint64_t*)0x200001a0 = 0;
    *(uint64_t*)0x200001a8 = 0;
    *(uint32_t*)0x200001b0 = 0;
    syscall(__NR_sendmsg, r[3], 0x20000180, 0);
    break;
  case 15:
    res = syscall(__NR_socket, 0xa, 0x1000000000002, 0);
    if (res != -1)
      r[4] = res;
    break;
  case 16:
    syscall(__NR_ioctl, r[4], 0x8912, 0x20000280);
    break;
  case 17:
    res = syscall(__NR_socket, 0x26, 5, 0);
    if (res != -1)
      r[5] = res;
    break;
  case 18:
    *(uint16_t*)0x20000000 = 0x26;
    memcpy((void*)0x20000002,
           "\x68\x61\x73\x68\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 14);
    *(uint32_t*)0x20000010 = 0;
    *(uint32_t*)0x20000014 = 0;
    memcpy((void*)0x20000018,
           "\x73\x68\x61\x35\x31\x32\x5f\x6d\x62\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
           64);
    syscall(__NR_bind, r[5], 0x20000000, 0x58);
    break;
  case 19:
    res = syscall(__NR_accept, r[5], 0, 0);
    if (res != -1)
      r[6] = res;
    break;
  case 20:
    *(uint64_t*)0x20001b80 = 0;
    *(uint32_t*)0x20001b88 = 0;
    *(uint64_t*)0x20001b90 = 0x200004c0;
    *(uint64_t*)0x20001b98 = 0;
    *(uint64_t*)0x20001ba0 = 0x20001580;
    *(uint64_t*)0x20001ba8 = 0;
    *(uint32_t*)0x20001bb0 = 0;
    syscall(__NR_sendmmsg, r[6], 0x20001b80, 0x500, 0xff03);
    break;
  case 21:
    *(uint32_t*)0x20000180 = 0x80;
    syscall(__NR_accept, r[6], 0x20000100, 0x20000180);
    break;
  case 22:
    res = syscall(__NR_socket, 2, 2, 0);
    if (res != -1)
      r[7] = res;
    break;
  case 23:
    *(uint16_t*)0x20000000 = 2;
    *(uint16_t*)0x20000002 = htobe16(0);
    *(uint8_t*)0x20000004 = 0xac;
    *(uint8_t*)0x20000005 = 0x14;
    *(uint8_t*)0x20000006 = 0x14;
    *(uint8_t*)0x20000007 = 0xbb;
    *(uint8_t*)0x20000008 = 0;
    *(uint8_t*)0x20000009 = 0;
    *(uint8_t*)0x2000000a = 0;
    *(uint8_t*)0x2000000b = 0;
    *(uint8_t*)0x2000000c = 0;
    *(uint8_t*)0x2000000d = 0;
    *(uint8_t*)0x2000000e = 0;
    *(uint8_t*)0x2000000f = 0;
    syscall(__NR_connect, r[7], 0x20000000, 0x10);
    break;
  case 24:
    syscall(__NR_socket, 0xa, 0x1000000000002, 0);
    break;
  case 25:
    res = syscall(__NR_socket, 0xa, 1, 0);
    if (res != -1)
      r[8] = res;
    break;
  case 26:
    *(uint32_t*)0x20000100 = 2;
    *(uint16_t*)0x20000108 = 2;
    *(uint16_t*)0x2000010a = htobe16(0);
    *(uint32_t*)0x2000010c = htobe32(0xe0000002);
    *(uint8_t*)0x20000110 = 0;
    *(uint8_t*)0x20000111 = 0;
    *(uint8_t*)0x20000112 = 0;
    *(uint8_t*)0x20000113 = 0;
    *(uint8_t*)0x20000114 = 0;
    *(uint8_t*)0x20000115 = 0;
    *(uint8_t*)0x20000116 = 0;
    *(uint8_t*)0x20000117 = 0;
    syscall(__NR_setsockopt, r[8], 0, 0x2a, 0x20000100, 0x88);
    break;
  case 27:
    *(uint64_t*)0x20000040 = 0;
    *(uint64_t*)0x20000048 = 0x2710;
    syscall(__NR_setsockopt, r[7], 1, 0x3d, 0x20000040, 8);
    break;
  case 28:
    *(uint32_t*)0x20008000 = 0;
    *(uint32_t*)0x20008004 = 0;
    *(uint64_t*)0x20008008 = 0x20001fe8;
    *(uint64_t*)0x20008010 = 0x20000000;
    memcpy((void*)0x20000000, "syzkaller", 10);
    *(uint32_t*)0x20008018 = 0;
    *(uint32_t*)0x2000801c = 0xc3;
    *(uint64_t*)0x20008020 = 0x20009f3d;
    *(uint32_t*)0x20008028 = 0;
    *(uint32_t*)0x2000802c = 0;
    *(uint8_t*)0x20008030 = 0;
    *(uint8_t*)0x20008031 = 0;
    *(uint8_t*)0x20008032 = 0;
    *(uint8_t*)0x20008033 = 0;
    *(uint8_t*)0x20008034 = 0;
    *(uint8_t*)0x20008035 = 0;
    *(uint8_t*)0x20008036 = 0;
    *(uint8_t*)0x20008037 = 0;
    *(uint8_t*)0x20008038 = 0;
    *(uint8_t*)0x20008039 = 0;
    *(uint8_t*)0x2000803a = 0;
    *(uint8_t*)0x2000803b = 0;
    *(uint8_t*)0x2000803c = 0;
    *(uint8_t*)0x2000803d = 0;
    *(uint8_t*)0x2000803e = 0;
    *(uint8_t*)0x2000803f = 0;
    *(uint32_t*)0x20008040 = 0;
    *(uint32_t*)0x20008044 = 0;
    syscall(__NR_bpf, 5, 0x20008000, 0x48);
    break;
  case 29:
    res = syscall(__NR_socket, 0xa, 0x1000000000002, 0);
    if (res != -1)
      r[9] = res;
    break;
  case 30:
    syscall(__NR_ioctl, r[9], 0x8912, 0x20000280);
    break;
  case 31:
    *(uint32_t*)0x2000e000 = 1;
    *(uint32_t*)0x2000e004 = 5;
    *(uint64_t*)0x2000e008 = 0x20001fd8;
    *(uint8_t*)0x20001fd8 = 0xb7;
    STORE_BY_BITMASK(uint8_t, 0x20001fd9, 0, 0, 4);
    STORE_BY_BITMASK(uint8_t, 0x20001fd9, 0, 4, 4);
    *(uint16_t*)0x20001fda = 0;
    *(uint32_t*)0x20001fdc = 0;
    *(uint8_t*)0x20001fe0 = 0x15;
    *(uint8_t*)0x20001fe1 = 0;
    *(uint16_t*)0x20001fe2 = 2;
    *(uint32_t*)0x20001fe4 = 0;
    STORE_BY_BITMASK(uint8_t, 0x20001fe8, 7, 0, 3);
    STORE_BY_BITMASK(uint8_t, 0x20001fe8, 0, 3, 2);
    STORE_BY_BITMASK(uint8_t, 0x20001fe8, 0, 5, 3);
    STORE_BY_BITMASK(uint8_t, 0x20001fe9, 0, 0, 4);
    STORE_BY_BITMASK(uint8_t, 0x20001fe9, 0, 4, 4);
    *(uint16_t*)0x20001fea = 0;
    *(uint32_t*)0x20001fec = 0;
    *(uint8_t*)0x20001ff8 = 0x95;
    *(uint8_t*)0x20001ff9 = 0;
    *(uint16_t*)0x20001ffa = 0;
    *(uint32_t*)0x20001ffc = 0;
    *(uint64_t*)0x2000e010 = 0x20003ff6;
    memcpy((void*)0x20003ff6, "GPL", 4);
    *(uint32_t*)0x2000e018 = 5;
    *(uint32_t*)0x2000e01c = 0x437;
    *(uint64_t*)0x2000e020 = 0x2000cf3d;
    *(uint32_t*)0x2000e028 = 0;
    *(uint32_t*)0x2000e02c = 0;
    *(uint8_t*)0x2000e030 = 0;
    *(uint8_t*)0x2000e031 = 0;
    *(uint8_t*)0x2000e032 = 0;
    *(uint8_t*)0x2000e033 = 0;
    *(uint8_t*)0x2000e034 = 0;
    *(uint8_t*)0x2000e035 = 0;
    *(uint8_t*)0x2000e036 = 0;
    *(uint8_t*)0x2000e037 = 0;
    *(uint8_t*)0x2000e038 = 0;
    *(uint8_t*)0x2000e039 = 0;
    *(uint8_t*)0x2000e03a = 0;
    *(uint8_t*)0x2000e03b = 0;
    *(uint8_t*)0x2000e03c = 0;
    *(uint8_t*)0x2000e03d = 0;
    *(uint8_t*)0x2000e03e = 0;
    *(uint8_t*)0x2000e03f = 0;
    *(uint32_t*)0x2000e040 = 0;
    *(uint32_t*)0x2000e044 = 0;
    syscall(__NR_bpf, 5, 0x2000e000, 0x48);
    break;
  case 32:
    *(uint32_t*)0x20000040 = 0;
    syscall(__NR_bpf, 0xb, 0x20000040, 4);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      do_sandbox_none();
    }
  }
  sleep(1000000);
  return 0;
}
