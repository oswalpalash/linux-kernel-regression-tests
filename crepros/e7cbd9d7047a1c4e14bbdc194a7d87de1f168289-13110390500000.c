// https://syzkaller.appspot.com/bug?id=e7cbd9d7047a1c4e14bbdc194a7d87de1f168289
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <setjmp.h>
#include <signal.h>
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

static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* ctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  int skip = __atomic_load_n(&skip_segv, __ATOMIC_RELAXED) != 0;
  int valid = addr < prog_start || addr > prog_end;
  if (skip && valid) {
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
  ({                                                                           \
    int ok = 1;                                                                \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    if (_setjmp(segv_env) == 0) {                                              \
      __VA_ARGS__;                                                             \
    } else                                                                     \
      ok = 0;                                                                  \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    ok;                                                                        \
  })

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
  int i = 0;
  for (; i < 100; i++) {
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

struct csum_inet {
  uint32_t acc;
};

static void csum_inet_init(struct csum_inet* csum)
{
  csum->acc = 0;
}

static void csum_inet_update(struct csum_inet* csum, const uint8_t* data,
                             size_t length)
{
  if (length == 0)
    return;
  size_t i = 0;
  for (; i < length - 1; i += 2)
    csum->acc += *(uint16_t*)&data[i];
  if (length & 1)
    csum->acc += le16toh((uint16_t)data[length - 1]);
  while (csum->acc > 0xffff)
    csum->acc = (csum->acc & 0xffff) + (csum->acc >> 16);
}

static uint16_t csum_inet_digest(struct csum_inet* csum)
{
  return ~csum->acc;
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

#define MAX_FDS 30

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
  if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL)) {
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

static void close_fds()
{
  for (int fd = 3; fd < MAX_FDS; fd++)
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

static void loop(void)
{
  int i, call, thread;
  int collide = 0;
again:
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
      if (collide && (call % 2) == 0)
        break;
      event_timedwait(&th->done, 45);
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
  close_fds();
  if (!collide) {
    collide = 1;
    goto again;
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    NONFAILING(memcpy((void*)0x20000040, "cpu.stat\000", 9));
    res = syscall(__NR_openat, 0xffffff9c, 0x20000040ul, 0x275aul, 0ul);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    res = syscall(__NR_socket, 0xaul, 3ul, 7);
    if (res != -1)
      r[1] = res;
    break;
  case 2:
    syscall(__NR_connect, r[1], 0ul, 0ul);
    break;
  case 3:
    syscall(__NR_sendmmsg, r[1], 0ul, 0ul, 0xfe80ul);
    break;
  case 4:
    syscall(__NR_ioctl, r[1], 0x8912, 0ul);
    break;
  case 5:
    syscall(__NR_mmap, 0x20000000ul, 0xb36000ul, 2ul, 0x28011ul, r[0], 0ul);
    break;
  case 6:
    syscall(__NR_connect, -1, 0ul, 0ul);
    break;
  case 7:
    syscall(__NR_sendmmsg, -1, 0ul, 0ul, 0xfe80ul);
    break;
  case 8:
    syscall(__NR_sendmsg, r[0], 0ul, 0x24000045ul);
    break;
  case 9:
    NONFAILING(*(uint8_t*)0x20000080 = 0xaa);
    NONFAILING(*(uint8_t*)0x20000081 = 0xaa);
    NONFAILING(*(uint8_t*)0x20000082 = 0xaa);
    NONFAILING(*(uint8_t*)0x20000083 = 0xaa);
    NONFAILING(*(uint8_t*)0x20000084 = 0xaa);
    NONFAILING(*(uint8_t*)0x20000085 = 0xaa);
    NONFAILING(*(uint8_t*)0x20000086 = 1);
    NONFAILING(*(uint8_t*)0x20000087 = 0x80);
    NONFAILING(*(uint8_t*)0x20000088 = 0xc2);
    NONFAILING(*(uint8_t*)0x20000089 = 0);
    NONFAILING(*(uint8_t*)0x2000008a = 0);
    NONFAILING(*(uint8_t*)0x2000008b = 0);
    NONFAILING(*(uint16_t*)0x2000008c = htobe16(0x800));
    NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x2000008e, 5, 0, 4));
    NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x2000008e, 4, 4, 4));
    NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x2000008f, 0, 0, 2));
    NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x2000008f, 0, 2, 6));
    NONFAILING(*(uint16_t*)0x20000090 = htobe16(0x28));
    NONFAILING(*(uint16_t*)0x20000092 = htobe16(0));
    NONFAILING(*(uint16_t*)0x20000094 = htobe16(0));
    NONFAILING(*(uint8_t*)0x20000096 = 0);
    NONFAILING(*(uint8_t*)0x20000097 = 6);
    NONFAILING(*(uint16_t*)0x20000098 = htobe16(0));
    NONFAILING(*(uint32_t*)0x2000009a = htobe32(0x7f000001));
    NONFAILING(*(uint8_t*)0x2000009e = 0xac);
    NONFAILING(*(uint8_t*)0x2000009f = 0x14);
    NONFAILING(*(uint8_t*)0x200000a0 = 0x14);
    NONFAILING(*(uint8_t*)0x200000a1 = 0xaa);
    NONFAILING(*(uint16_t*)0x200000a2 = htobe16(0));
    NONFAILING(*(uint16_t*)0x200000a4 = htobe16(0x4e24));
    NONFAILING(*(uint32_t*)0x200000a6 = 0x41424344);
    NONFAILING(*(uint32_t*)0x200000aa = 0x41424344);
    NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x200000ae, 0, 0, 1));
    NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x200000ae, 6, 1, 3));
    NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x200000ae, 5, 4, 4));
    NONFAILING(*(uint8_t*)0x200000af = 2);
    NONFAILING(*(uint16_t*)0x200000b0 = htobe16(0));
    NONFAILING(*(uint16_t*)0x200000b2 = htobe16(0));
    NONFAILING(*(uint16_t*)0x200000b4 = htobe16(0));
    struct csum_inet csum_1;
    csum_inet_init(&csum_1);
    NONFAILING(csum_inet_update(&csum_1, (const uint8_t*)0x2000009a, 4));
    NONFAILING(csum_inet_update(&csum_1, (const uint8_t*)0x2000009e, 4));
    uint16_t csum_1_chunk_2 = 0x600;
    csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_2, 2);
    uint16_t csum_1_chunk_3 = 0x1400;
    csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_3, 2);
    NONFAILING(csum_inet_update(&csum_1, (const uint8_t*)0x200000a2, 20));
    NONFAILING(*(uint16_t*)0x200000b2 = csum_inet_digest(&csum_1));
    struct csum_inet csum_2;
    csum_inet_init(&csum_2);
    NONFAILING(csum_inet_update(&csum_2, (const uint8_t*)0x2000008e, 20));
    NONFAILING(*(uint16_t*)0x20000098 = csum_inet_digest(&csum_2));
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  install_segv_handler();
  do_sandbox_none();
  return 0;
}
