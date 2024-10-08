// https://syzkaller.appspot.com/bug?id=b96d0a0eaa2196cfa0667e1359772777ee06c80a
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

unsigned long long procid;

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
    if (__atomic_load_n(&ev->state, __ATOMIC_RELAXED))
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
  DIR* dp;
  struct dirent* ep;
  int iter = 0;
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
  int i;
  for (i = 0;; i++) {
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res;
  switch (call) {
  case 0:
    res = syscall(__NR_socket, 0x10ul, 3ul, 0xcul);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    NONFAILING(*(uint64_t*)0x20000280 = 0);
    NONFAILING(*(uint32_t*)0x20000288 = 0x2d3);
    NONFAILING(*(uint64_t*)0x20000290 = 0x20000240);
    NONFAILING(*(uint64_t*)0x20000240 = 0x200002c0);
    NONFAILING(memcpy(
        (void*)0x200002c0,
        "\x14\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00"
        "\x00\x00\x0a\x20\x00\x00\x00\x00\x0a\x01\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x09\x00\x01\x00\x73\x79\x7a\x30\x00\x00\x00"
        "\x00\x70\x00\x00\x00\x12\x0a\x01\x00\x00\x1f\x34\x01\x04\x00\x00\x00"
        "\x00\x00\x00\x00\x04\x00\x04\x80\x09\x00\x02\x00\x73\x79\x7a\x30\x00"
        "\x00\x00\x00\x09\x00\x01\x00\x73\x79\x7a\x30\x00\x00\x00\x00\x08\x00"
        "\x03\x40\x00\x00\x00\x00\x22\x00\x02\x00\xb7\x63\x12\xc4\x11\x0d\xd9"
        "\xb1\x73\x79\xe7\x28\x14\x7a\xfe\x6a\xb9\xf3\x2a\xa9\xd1\x00\x04\x80"
        "\x00\x00\x04\x00\x06\x14\x00\x04\x80\x09\x00\x01\x00\x59\x79\x7a\x30"
        "\x00\x00\x00\x00\x00\x00\x14\x00\x00\x00\x11\x00\x6d\x30\xa1\x6e\x90"
        "\x01\x6c\x47\x8a\x50\x48\x3a\x73\x64\x5a\x6d\xc4\xb5\x34\x5d\x2c\x0d"
        "\xb7\x4c\x9f\x89\x67\x8f\xbc\x9c\xc6\xfb\x9e\x8e\x31\xfa\xd7\xe7\x58"
        "\x0e\x6a\xe4\x35\x8d\x8f\x95\xa2\x73\x5d\x57\xdc\x6e\x3a\xf5\xcf\x3e"
        "\x09\xaa\x68\xb1\x91\xfe\xa4\x67\x66\xe9\xa1\xb3\xb4\xc9\xb0\x75\xf7"
        "\x4c\x98\x1f\x80\x44\x53\x89\x8a\x0b\x8f\x9d\x08\x1e\xf6\x43\x7d\x40"
        "\x51\xa3\x1e\xef\xf8\x78\x44\x21\xe4\x78\xa6\xe0\x4e\x0f\xb3\x69\xef"
        "\x95\xf2\x89\xff\x35\xa2\x43\x34\xba\x0d\x5a\xc0\x37\x02\x8b\x66\x59"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
        299));
    NONFAILING(*(uint64_t*)0x20000248 = 0xb8);
    NONFAILING(*(uint64_t*)0x20000298 = 1);
    NONFAILING(*(uint64_t*)0x200002a0 = 0);
    NONFAILING(*(uint64_t*)0x200002a8 = 0);
    NONFAILING(*(uint32_t*)0x200002b0 = 0);
    syscall(__NR_sendmsg, r[0], 0x20000280ul, 0ul);
    break;
  case 2:
    res = syscall(__NR_socket, 0x10ul, 3ul, 0xcul);
    if (res != -1)
      r[1] = res;
    break;
  case 3:
    NONFAILING(*(uint64_t*)0x20000300 = 0);
    NONFAILING(*(uint32_t*)0x20000308 = 0);
    NONFAILING(*(uint64_t*)0x20000310 = 0x20000280);
    NONFAILING(*(uint64_t*)0x20000280 = 0x20000380);
    NONFAILING(*(uint32_t*)0x20000380 = 0x14);
    NONFAILING(*(uint16_t*)0x20000384 = 0x10);
    NONFAILING(*(uint16_t*)0x20000386 = 1);
    NONFAILING(*(uint32_t*)0x20000388 = 0);
    NONFAILING(*(uint32_t*)0x2000038c = 0);
    NONFAILING(*(uint8_t*)0x20000390 = 0);
    NONFAILING(*(uint8_t*)0x20000391 = 0);
    NONFAILING(*(uint16_t*)0x20000392 = htobe16(0xa));
    NONFAILING(*(uint32_t*)0x20000394 = 0x5c);
    NONFAILING(*(uint8_t*)0x20000398 = 2);
    NONFAILING(*(uint8_t*)0x20000399 = 0xa);
    NONFAILING(*(uint16_t*)0x2000039a = 0x101);
    NONFAILING(*(uint32_t*)0x2000039c = 0);
    NONFAILING(*(uint32_t*)0x200003a0 = 0);
    NONFAILING(*(uint8_t*)0x200003a4 = 0);
    NONFAILING(*(uint8_t*)0x200003a5 = 0);
    NONFAILING(*(uint16_t*)0x200003a6 = htobe16(8));
    NONFAILING(*(uint16_t*)0x200003a8 = 9);
    NONFAILING(*(uint16_t*)0x200003aa = 1);
    NONFAILING(memcpy((void*)0x200003ac, "syz0\000", 5));
    NONFAILING(*(uint16_t*)0x200003b4 = 9);
    NONFAILING(*(uint16_t*)0x200003b6 = 1);
    NONFAILING(memcpy((void*)0x200003b8, "syz1\000", 5));
    NONFAILING(*(uint16_t*)0x200003c0 = 9);
    NONFAILING(*(uint16_t*)0x200003c2 = 1);
    NONFAILING(memcpy((void*)0x200003c4, "syz0\000", 5));
    NONFAILING(*(uint16_t*)0x200003cc = 9);
    NONFAILING(*(uint16_t*)0x200003ce = 1);
    NONFAILING(memcpy((void*)0x200003d0, "syz0\000", 5));
    NONFAILING(*(uint16_t*)0x200003d8 = 0xc);
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x200003da, 4, 0, 14));
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x200003db, 1, 6, 1));
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x200003db, 0, 7, 1));
    NONFAILING(*(uint64_t*)0x200003dc = htobe64(5));
    NONFAILING(*(uint16_t*)0x200003e4 = 0xc);
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x200003e6, 4, 0, 14));
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x200003e7, 1, 6, 1));
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x200003e7, 0, 7, 1));
    NONFAILING(*(uint64_t*)0x200003e8 = htobe64(5));
    NONFAILING(*(uint32_t*)0x200003f0 = 0x14);
    NONFAILING(*(uint16_t*)0x200003f4 = 0x11);
    NONFAILING(*(uint16_t*)0x200003f6 = 1);
    NONFAILING(*(uint32_t*)0x200003f8 = 0);
    NONFAILING(*(uint32_t*)0x200003fc = 0);
    NONFAILING(*(uint8_t*)0x20000400 = 0);
    NONFAILING(*(uint8_t*)0x20000401 = 0);
    NONFAILING(*(uint16_t*)0x20000402 = htobe16(0xa));
    NONFAILING(*(uint64_t*)0x20000288 = 0x84);
    NONFAILING(*(uint64_t*)0x20000318 = 1);
    NONFAILING(*(uint64_t*)0x20000320 = 0);
    NONFAILING(*(uint64_t*)0x20000328 = 0);
    NONFAILING(*(uint32_t*)0x20000330 = 0x30000041);
    syscall(__NR_sendmsg, r[1], 0x20000300ul, 0x40040ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
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
