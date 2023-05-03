// https://syzkaller.appspot.com/bug?id=99a60813b9287a6ef4d1583f9208c6584ffde85a
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/capability.h>
#include <linux/futex.h>

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
  int i;
  for (i = 0; i < 100; i++) {
    if (pthread_create(&th, &attr, fn, arg) == 0) {
      pthread_attr_destroy(&attr);
      return;
    }
    if (errno == EAGAIN) {
      usleep(50);
      continue;
    }
    break;
  }
  exit(1);
}

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

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
  syscall(SYS_futex, &ev->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1000000);
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
    if (__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
      return 1;
    now = current_time_ms();
    if (now - start > timeout)
      return 0;
  }
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
  rlim.rlim_cur = rlim.rlim_max = (200 << 20);
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
  for (i = 0; i < sizeof(sysctls) / sizeof(sysctls[0]); i++)
    write_file(sysctls[i].name, sysctls[i].value);
}

static int wait_for_loop(int pid)
{
  if (pid < 0)
    exit(1);
  int status = 0;
  while (waitpid(-1, &status, __WALL) != pid) {
  }
  return WEXITSTATUS(status);
}

static void drop_caps(void)
{
  struct __user_cap_header_struct cap_hdr = {};
  struct __user_cap_data_struct cap_data[2] = {};
  cap_hdr.version = _LINUX_CAPABILITY_VERSION_3;
  cap_hdr.pid = getpid();
  if (syscall(SYS_capget, &cap_hdr, &cap_data))
    exit(1);
  const int drop = (1 << CAP_SYS_PTRACE) | (1 << CAP_SYS_NICE);
  cap_data[0].effective &= ~drop;
  cap_data[0].permitted &= ~drop;
  cap_data[0].inheritable &= ~drop;
  if (syscall(SYS_capset, &cap_hdr, &cap_data))
    exit(1);
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
  drop_caps();
  if (unshare(CLONE_NEWNET)) {
  }
  loop();
  exit(1);
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

static void loop(void)
{
  int i, call, thread;
  for (call = 0; call < 10; call++) {
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

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[6] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res;
  switch (call) {
  case 0:
    res = syscall(__NR_pipe, 0x20000000ul);
    if (res != -1) {
      r[0] = *(uint32_t*)0x20000000;
      r[1] = *(uint32_t*)0x20000004;
    }
    break;
  case 1:
    res = syscall(__NR_socket, 2ul, 2ul, 0);
    if (res != -1)
      r[2] = res;
    break;
  case 2:
    syscall(__NR_close, r[2]);
    break;
  case 3:
    memcpy((void*)0x20000080, "/dev/net/tun\000", 13);
    res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000080ul, 0x88002ul,
                  0ul);
    if (res != -1)
      r[3] = res;
    break;
  case 4:
    memcpy((void*)0x20000100, "syzkaller1\000\000\000\000\000\000", 16);
    *(uint16_t*)0x20000110 = 0xa732;
    syscall(__NR_ioctl, r[3], 0x400454ca, 0x20000100ul);
    break;
  case 5:
    res = syscall(__NR_socket, 0x10ul, 3ul, 0);
    if (res != -1)
      r[4] = res;
    break;
  case 6:
    *(uint32_t*)0x20000180 = 6;
    *(uint32_t*)0x20000184 = 4;
    *(uint64_t*)0x20000188 = 0x20000200;
    *(uint8_t*)0x20000200 = 0x18;
    STORE_BY_BITMASK(uint8_t, , 0x20000201, 2, 0, 4);
    STORE_BY_BITMASK(uint8_t, , 0x20000201, 0, 4, 4);
    *(uint16_t*)0x20000202 = 0;
    *(uint32_t*)0x20000204 = 0;
    *(uint8_t*)0x20000208 = 0;
    *(uint8_t*)0x20000209 = 0;
    *(uint16_t*)0x2000020a = 0;
    *(uint32_t*)0x2000020c = 0;
    *(uint8_t*)0x20000210 = 0x85;
    *(uint8_t*)0x20000211 = 0;
    *(uint16_t*)0x20000212 = 0;
    *(uint32_t*)0x20000214 = 0x2c;
    *(uint8_t*)0x20000218 = 0x95;
    *(uint8_t*)0x20000219 = 0;
    *(uint16_t*)0x2000021a = 0;
    *(uint32_t*)0x2000021c = 0;
    *(uint64_t*)0x20000190 = 0x200000c0;
    memcpy((void*)0x200000c0, "GPL\000", 4);
    *(uint32_t*)0x20000198 = 4;
    *(uint32_t*)0x2000019c = 0x1000;
    *(uint64_t*)0x200001a0 = 0x2062b000;
    *(uint32_t*)0x200001a8 = 0;
    *(uint32_t*)0x200001ac = 0;
    *(uint8_t*)0x200001b0 = 0;
    *(uint8_t*)0x200001b1 = 0;
    *(uint8_t*)0x200001b2 = 0;
    *(uint8_t*)0x200001b3 = 0;
    *(uint8_t*)0x200001b4 = 0;
    *(uint8_t*)0x200001b5 = 0;
    *(uint8_t*)0x200001b6 = 0;
    *(uint8_t*)0x200001b7 = 0;
    *(uint8_t*)0x200001b8 = 0;
    *(uint8_t*)0x200001b9 = 0;
    *(uint8_t*)0x200001ba = 0;
    *(uint8_t*)0x200001bb = 0;
    *(uint8_t*)0x200001bc = 0;
    *(uint8_t*)0x200001bd = 0;
    *(uint8_t*)0x200001be = 0;
    *(uint8_t*)0x200001bf = 0;
    *(uint32_t*)0x200001c0 = 0;
    *(uint32_t*)0x200001c4 = 0;
    *(uint32_t*)0x200001c8 = -1;
    *(uint32_t*)0x200001cc = 8;
    *(uint64_t*)0x200001d0 = 0;
    *(uint32_t*)0x200001d8 = 0;
    *(uint32_t*)0x200001dc = 0x10;
    *(uint64_t*)0x200001e0 = 0;
    *(uint32_t*)0x200001e8 = 0;
    *(uint32_t*)0x200001ec = 0;
    *(uint32_t*)0x200001f0 = -1;
    res = syscall(__NR_bpf, 5ul, 0x20000180ul, 0x70ul);
    if (res != -1)
      r[5] = res;
    break;
  case 7:
    *(uint64_t*)0x20000140 = 0;
    *(uint32_t*)0x20000148 = 0x1a0;
    *(uint64_t*)0x20000150 = 0x20000080;
    *(uint64_t*)0x20000080 = 0x20000000;
    *(uint32_t*)0x20000000 = 0x34;
    *(uint16_t*)0x20000004 = 0x10;
    *(uint16_t*)0x20000006 = 0x801;
    *(uint32_t*)0x20000008 = 0;
    *(uint32_t*)0x2000000c = 0;
    *(uint8_t*)0x20000010 = 0;
    *(uint8_t*)0x20000011 = 0;
    *(uint16_t*)0x20000012 = 0;
    *(uint32_t*)0x20000014 = 0;
    *(uint32_t*)0x20000018 = 0;
    *(uint32_t*)0x2000001c = 0;
    *(uint16_t*)0x20000020 = 0xc;
    STORE_BY_BITMASK(uint16_t, , 0x20000022, 0x2b, 0, 14);
    STORE_BY_BITMASK(uint16_t, , 0x20000023, 0, 6, 1);
    STORE_BY_BITMASK(uint16_t, , 0x20000023, 1, 7, 1);
    *(uint16_t*)0x20000024 = 8;
    *(uint16_t*)0x20000026 = 1;
    *(uint32_t*)0x20000028 = r[5];
    *(uint16_t*)0x2000002c = 8;
    *(uint16_t*)0x2000002e = 0x1b;
    *(uint32_t*)0x20000030 = 0;
    *(uint64_t*)0x20000088 = 0x34;
    *(uint64_t*)0x20000158 = 1;
    *(uint64_t*)0x20000160 = 0;
    *(uint64_t*)0x20000168 = 0;
    *(uint32_t*)0x20000170 = 0;
    syscall(__NR_sendmsg, r[4], 0x20000140ul, 0ul);
    break;
  case 8:
    syscall(__NR_write, r[1], 0x20000140ul, 0x1000000c8ul);
    break;
  case 9:
    syscall(__NR_splice, r[0], 0ul, r[2], 0ul, 0x10008ul, 0ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0ul);
  do_sandbox_none();
  return 0;
}
