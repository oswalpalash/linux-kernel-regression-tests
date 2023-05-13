// https://syzkaller.appspot.com/bug?id=aeeb0c8dde43b5e032734b2a5573803031be395f
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
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/futex.h>

static unsigned long long procid;

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

static void use_temporary_dir(void)
{
  char tmpdir_template[] = "./syzkaller.XXXXXX";
  char* tmpdir = mkdtemp(tmpdir_template);
  if (!tmpdir)
    exit(1);
  if (chmod(tmpdir, 0777))
    exit(1);
  if (chdir(tmpdir))
    exit(1);
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

#define FS_IOC_SETFLAGS _IOW('f', 2, long)
static void remove_dir(const char* dir)
{
  int iter = 0;
  DIR* dp = 0;
retry:
  while (umount2(dir, MNT_DETACH) == 0) {
  }
  dp = opendir(dir);
  if (dp == NULL) {
    if (errno == EMFILE) {
      exit(1);
    }
    exit(1);
  }
  struct dirent* ep = 0;
  while ((ep = readdir(dp))) {
    if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
      continue;
    char filename[FILENAME_MAX];
    snprintf(filename, sizeof(filename), "%s/%s", dir, ep->d_name);
    while (umount2(filename, MNT_DETACH) == 0) {
    }
    struct stat st;
    if (lstat(filename, &st))
      exit(1);
    if (S_ISDIR(st.st_mode)) {
      remove_dir(filename);
      continue;
    }
    int i;
    for (i = 0;; i++) {
      if (unlink(filename) == 0)
        break;
      if (errno == EPERM) {
        int fd = open(filename, O_RDONLY);
        if (fd != -1) {
          long flags = 0;
          if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == 0) {
          }
          close(fd);
          continue;
        }
      }
      if (errno == EROFS) {
        break;
      }
      if (errno != EBUSY || i > 100)
        exit(1);
      if (umount2(filename, MNT_DETACH))
        exit(1);
    }
  }
  closedir(dp);
  for (int i = 0;; i++) {
    if (rmdir(dir) == 0)
      break;
    if (i < 100) {
      if (errno == EPERM) {
        int fd = open(dir, O_RDONLY);
        if (fd != -1) {
          long flags = 0;
          if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == 0) {
          }
          close(fd);
          continue;
        }
      }
      if (errno == EROFS) {
        break;
      }
      if (errno == EBUSY) {
        if (umount2(dir, MNT_DETACH))
          exit(1);
        continue;
      }
      if (errno == ENOTEMPTY) {
        if (iter < 100) {
          iter++;
          goto retry;
        }
      }
    }
    exit(1);
  }
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
  for (call = 0; call < 2; call++) {
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
  int iter = 0;
  for (;; iter++) {
    char cwdbuf[32];
    sprintf(cwdbuf, "./%d", iter);
    if (mkdir(cwdbuf, 0777))
      exit(1);
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      if (chdir(cwdbuf))
        exit(1);
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
      if (current_time_ms() - start < 5 * 1000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
    remove_dir(cwdbuf);
  }
}

void execute_call(int call)
{
  switch (call) {
  case 0:
    NONFAILING(*(uint32_t*)0x20000040 = 2);
    NONFAILING(*(uint32_t*)0x20000044 = 0x70);
    NONFAILING(*(uint8_t*)0x20000048 = 0x1c);
    NONFAILING(*(uint8_t*)0x20000049 = 1);
    NONFAILING(*(uint8_t*)0x2000004a = 0);
    NONFAILING(*(uint8_t*)0x2000004b = 0);
    NONFAILING(*(uint32_t*)0x2000004c = 0);
    NONFAILING(*(uint64_t*)0x20000050 = 1);
    NONFAILING(*(uint64_t*)0x20000058 = 0x106432);
    NONFAILING(*(uint64_t*)0x20000060 = 0);
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 0, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 1, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 2, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 3, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 4, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 5, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 6, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 7, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 8, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 9, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 10, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 11, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 12, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 13, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 14, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 15, 2));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 17, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 18, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 19, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 20, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 21, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 22, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 23, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 24, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 25, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 26, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 27, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 28, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 29, 35));
    NONFAILING(*(uint32_t*)0x20000070 = 0);
    NONFAILING(*(uint32_t*)0x20000074 = 0);
    NONFAILING(*(uint64_t*)0x20000078 = 0);
    NONFAILING(*(uint64_t*)0x20000080 = 0);
    NONFAILING(*(uint64_t*)0x20000088 = 0);
    NONFAILING(*(uint64_t*)0x20000090 = 0);
    NONFAILING(*(uint32_t*)0x20000098 = 0x20);
    NONFAILING(*(uint32_t*)0x2000009c = 0);
    NONFAILING(*(uint64_t*)0x200000a0 = 0);
    NONFAILING(*(uint32_t*)0x200000a8 = 0);
    NONFAILING(*(uint16_t*)0x200000ac = 0);
    NONFAILING(*(uint16_t*)0x200000ae = 0);
    syscall(__NR_perf_event_open, 0x20000040ul, 0, 0ul, -1, 0ul);
    break;
  case 1:
    NONFAILING(*(uint32_t*)0x20000100 = 1);
    NONFAILING(*(uint32_t*)0x20000104 = 0x70);
    NONFAILING(*(uint8_t*)0x20000108 = 0);
    NONFAILING(*(uint8_t*)0x20000109 = 0);
    NONFAILING(*(uint8_t*)0x2000010a = 0);
    NONFAILING(*(uint8_t*)0x2000010b = 0);
    NONFAILING(*(uint32_t*)0x2000010c = 0);
    NONFAILING(*(uint64_t*)0x20000110 = 0x3c43);
    NONFAILING(*(uint64_t*)0x20000118 = 0);
    NONFAILING(*(uint64_t*)0x20000120 = 0);
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000128, 0, 0, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000128, 0, 1, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000128, 0, 2, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000128, 0, 3, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000128, 0, 4, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000128, 0, 5, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000128, 0, 6, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000128, 0, 7, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000128, 0, 8, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000128, 0, 9, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000128, 0, 10, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000128, 0, 11, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000128, 0, 12, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000128, 0, 13, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000128, 0, 14, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000128, 0, 15, 2));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000128, 0, 17, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000128, 0, 18, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000128, 0, 19, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000128, 0, 20, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000128, 0, 21, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000128, 0, 22, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000128, 0, 23, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000128, 0, 24, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000128, 0, 25, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000128, 0, 26, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000128, 0, 27, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000128, 0, 28, 1));
    NONFAILING(STORE_BY_BITMASK(uint64_t, , 0x20000128, 0, 29, 35));
    NONFAILING(*(uint32_t*)0x20000130 = 0);
    NONFAILING(*(uint32_t*)0x20000134 = 0);
    NONFAILING(*(uint64_t*)0x20000138 = 0);
    NONFAILING(*(uint64_t*)0x20000140 = 0);
    NONFAILING(*(uint64_t*)0x20000148 = 0x8001);
    NONFAILING(*(uint64_t*)0x20000150 = 0);
    NONFAILING(*(uint32_t*)0x20000158 = 0);
    NONFAILING(*(uint32_t*)0x2000015c = 0);
    NONFAILING(*(uint64_t*)0x20000160 = 0);
    NONFAILING(*(uint32_t*)0x20000168 = 0);
    NONFAILING(*(uint16_t*)0x2000016c = 0);
    NONFAILING(*(uint16_t*)0x2000016e = 0);
    syscall(__NR_perf_event_open, 0x20000100ul, 0, 0ul, -1, 0ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  install_segv_handler();
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      use_temporary_dir();
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
