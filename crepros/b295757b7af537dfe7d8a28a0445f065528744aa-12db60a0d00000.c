// https://syzkaller.appspot.com/bug?id=b295757b7af537dfe7d8a28a0445f065528744aa
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

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

static void kill_and_wait(int pid, int* status)
{
  kill(-pid, SIGKILL);
  kill(pid, SIGKILL);
  for (int i = 0; i < 100; i++) {
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

static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  write_file("/proc/self/oom_score_adj", "1000");
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
  for (call = 0; call < 4; call++) {
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
      event_timedwait(&th->done, 50);
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
  int iter = 0;
  for (;; iter++) {
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      setup_test();
      execute_one();
      exit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      if (waitpid(-1, &status, WNOHANG | WAIT_FLAGS) == pid)
        break;
      sleep_ms(1);
      if (current_time_ms() - start < 5000) {
        continue;
      }
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    res = syscall(__NR_socket, 0x10ul, 0x80002ul, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    NONFAILING(*(uint64_t*)0x20002980 = 0);
    NONFAILING(*(uint32_t*)0x20002988 = 0);
    NONFAILING(*(uint64_t*)0x20002990 = 0x20002940);
    NONFAILING(*(uint64_t*)0x20002940 = 0x20000800);
    NONFAILING(memcpy((void*)0x20000800,
                      "\x98\x00\x00\x00\x30\x00\x3b\x05\x00\x00\x00\x00\x00\x00"
                      "\x00\x00\x00\x00\x00\x00\x84\x00\x01\x00\x50\x00\x01\x00"
                      "\x0d\x00\x01\x00\x63\x6f\x6e\x6e\x6d\x61\x72\x6b\x00\x00"
                      "\x00\x00\x20\x00\x02\x80\x1c\x00\x01\x00\x01\x00\x00\x00"
                      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                      "\x00\x00\x00\x00\x00\x00\x04\x00\x06\x00\x0c\x00\x07\x00"
                      "\x00\x00\x00\x00\x00\x00\x00\x00\x0c\x00\x08\x00\x00\x00"
                      "\x00\x00\x00\x00\x00\x00\x30\x00\x02\x00\x09\x00\x01",
                      111));
    NONFAILING(*(uint64_t*)0x20002948 = 0x98);
    NONFAILING(*(uint64_t*)0x20002998 = 1);
    NONFAILING(*(uint64_t*)0x200029a0 = 0);
    NONFAILING(*(uint64_t*)0x200029a8 = 0);
    NONFAILING(*(uint32_t*)0x200029b0 = 0);
    syscall(__NR_sendmsg, r[0], 0x20002980ul, 0ul);
    break;
  case 2:
    res = syscall(__NR_socket, 0x10ul, 0x80002ul, 0);
    if (res != -1)
      r[1] = res;
    break;
  case 3:
    NONFAILING(*(uint64_t*)0x20002980 = 0);
    NONFAILING(*(uint32_t*)0x20002988 = 0);
    NONFAILING(*(uint64_t*)0x20002990 = 0x20002940);
    NONFAILING(*(uint64_t*)0x20002940 = 0x20000800);
    NONFAILING(*(uint32_t*)0x20000800 = 0x98);
    NONFAILING(*(uint16_t*)0x20000804 = 0x30);
    NONFAILING(*(uint16_t*)0x20000806 = 0x53b);
    NONFAILING(*(uint32_t*)0x20000808 = 0);
    NONFAILING(*(uint32_t*)0x2000080c = 0);
    NONFAILING(*(uint8_t*)0x20000810 = 0);
    NONFAILING(*(uint8_t*)0x20000811 = 0);
    NONFAILING(*(uint16_t*)0x20000812 = 0);
    NONFAILING(*(uint16_t*)0x20000814 = 0x84);
    NONFAILING(*(uint16_t*)0x20000816 = 1);
    NONFAILING(*(uint16_t*)0x20000818 = 0x50);
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x2000081a, 1, 0, 14));
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x2000081b, 0, 6, 1));
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x2000081b, 0, 7, 1));
    NONFAILING(*(uint16_t*)0x2000081c = 0xd);
    NONFAILING(*(uint16_t*)0x2000081e = 1);
    NONFAILING(memcpy((void*)0x20000820, "connmark\000", 9));
    NONFAILING(*(uint16_t*)0x2000082c = 0x20);
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x2000082e, 2, 0, 14));
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x2000082f, 0, 6, 1));
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x2000082f, 1, 7, 1));
    NONFAILING(*(uint16_t*)0x20000830 = 0x1c);
    NONFAILING(*(uint16_t*)0x20000832 = 1);
    NONFAILING(*(uint32_t*)0x20000834 = 1);
    NONFAILING(*(uint32_t*)0x20000838 = 0);
    NONFAILING(*(uint32_t*)0x2000083c = 0);
    NONFAILING(*(uint32_t*)0x20000840 = 0);
    NONFAILING(*(uint32_t*)0x20000844 = 0);
    NONFAILING(*(uint16_t*)0x20000848 = 0);
    NONFAILING(*(uint16_t*)0x2000084c = 4);
    NONFAILING(*(uint16_t*)0x2000084e = 6);
    NONFAILING(*(uint16_t*)0x20000850 = 0xc);
    NONFAILING(*(uint16_t*)0x20000852 = 7);
    NONFAILING(*(uint32_t*)0x20000854 = 0);
    NONFAILING(*(uint32_t*)0x20000858 = 0);
    NONFAILING(*(uint16_t*)0x2000085c = 0xc);
    NONFAILING(*(uint16_t*)0x2000085e = 8);
    NONFAILING(*(uint32_t*)0x20000860 = 0);
    NONFAILING(*(uint32_t*)0x20000864 = 0);
    NONFAILING(*(uint16_t*)0x20000868 = 0x30);
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x2000086a, 2, 0, 14));
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x2000086b, 0, 6, 1));
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x2000086b, 0, 7, 1));
    NONFAILING(*(uint16_t*)0x2000086c = 9);
    NONFAILING(*(uint16_t*)0x2000086e = 1);
    NONFAILING(memcpy((void*)0x20000870, "csum\000", 5));
    NONFAILING(*(uint16_t*)0x20000878 = 4);
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x2000087a, 2, 0, 14));
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x2000087b, 0, 6, 1));
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x2000087b, 1, 7, 1));
    NONFAILING(*(uint16_t*)0x2000087c = 4);
    NONFAILING(*(uint16_t*)0x2000087e = 6);
    NONFAILING(*(uint16_t*)0x20000880 = 0xc);
    NONFAILING(*(uint16_t*)0x20000882 = 7);
    NONFAILING(*(uint32_t*)0x20000884 = 0);
    NONFAILING(*(uint32_t*)0x20000888 = 0);
    NONFAILING(*(uint16_t*)0x2000088c = 0xc);
    NONFAILING(*(uint16_t*)0x2000088e = 8);
    NONFAILING(*(uint32_t*)0x20000890 = 0);
    NONFAILING(*(uint32_t*)0x20000894 = 0);
    NONFAILING(*(uint64_t*)0x20002948 = 0x98);
    NONFAILING(*(uint64_t*)0x20002998 = 1);
    NONFAILING(*(uint64_t*)0x200029a0 = 0);
    NONFAILING(*(uint64_t*)0x200029a8 = 0);
    NONFAILING(*(uint32_t*)0x200029b0 = 0);
    syscall(__NR_sendmsg, r[1], 0x20002980ul, 0ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  install_segv_handler();
  loop();
  return 0;
}
