// https://syzkaller.appspot.com/bug?id=1bba967ec4596833317399ba8d6f7d655bd655e8
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
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

#define FS_IOC_SETFLAGS _IOW('f', 2, long)
static void remove_dir(const char* dir)
{
  DIR* dp;
  struct dirent* ep;
  int iter = 0;
retry:
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

static void close_fds()
{
  int fd;
  for (fd = 3; fd < MAX_FDS; fd++)
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

uint64_t r[5] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0x0, 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res;
  switch (call) {
  case 0:
    res = syscall(__NR_socket, 0x10ul, 3ul, 0ul);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    res = syscall(__NR_socket, 0x10ul, 3ul, 0ul);
    if (res != -1)
      r[1] = res;
    break;
  case 2:
    res = syscall(__NR_socket, 0x10ul, 2ul, 0);
    if (res != -1)
      r[2] = res;
    break;
  case 3:
    *(uint64_t*)0x200001c0 = 0;
    *(uint32_t*)0x200001c8 = 0;
    *(uint64_t*)0x200001d0 = 0x20000180;
    *(uint64_t*)0x20000180 = 0;
    *(uint64_t*)0x20000188 = 0;
    *(uint64_t*)0x200001d8 = 1;
    *(uint64_t*)0x200001e0 = 0;
    *(uint64_t*)0x200001e8 = 0;
    *(uint32_t*)0x200001f0 = 0;
    syscall(__NR_sendmsg, r[2], 0x200001c0ul, 0ul);
    break;
  case 4:
    *(uint32_t*)0x20000200 = 0x14;
    res = syscall(__NR_getsockname, r[2], 0x20000100ul, 0x20000200ul);
    if (res != -1)
      r[3] = *(uint32_t*)0x20000104;
    break;
  case 5:
    *(uint64_t*)0x20000240 = 0;
    *(uint32_t*)0x20000248 = 0;
    *(uint64_t*)0x20000250 = 0x20000140;
    *(uint64_t*)0x20000140 = 0x200003c0;
    memcpy((void*)0x200003c0, "\x38\x00\x00\x00\x24\x00\x07\x05\x00\x00\x00\x40"
                              "\x07\xa2\xa3\x00\x05\x00\x00\x00",
           20);
    *(uint32_t*)0x200003d4 = r[3];
    memcpy((void*)0x200003d8, "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00"
                              "\x09\x00\x01\x00\x68\x66\x73\x63\x00\x00\x00\x00"
                              "\x08\x00\x02\x00\x00\x00\x00\x00",
           32);
    *(uint64_t*)0x20000148 = 0x38;
    *(uint64_t*)0x20000258 = 1;
    *(uint64_t*)0x20000260 = 0;
    *(uint64_t*)0x20000268 = 0;
    *(uint32_t*)0x20000270 = 0;
    syscall(__NR_sendmsg, r[1], 0x20000240ul, 0ul);
    break;
  case 6:
    *(uint64_t*)0x20000280 = 0;
    *(uint32_t*)0x20000288 = 0;
    *(uint64_t*)0x20000290 = 0x20000180;
    *(uint64_t*)0x20000180 = 0x200004c0;
    *(uint32_t*)0x200004c0 = 0x34;
    *(uint16_t*)0x200004c4 = 0x2c;
    *(uint16_t*)0x200004c6 = 0xd27;
    *(uint32_t*)0x200004c8 = 0;
    *(uint32_t*)0x200004cc = 0;
    *(uint8_t*)0x200004d0 = 0;
    *(uint8_t*)0x200004d1 = 0;
    *(uint16_t*)0x200004d2 = 0;
    *(uint32_t*)0x200004d4 = r[3];
    *(uint16_t*)0x200004d8 = 0;
    *(uint16_t*)0x200004da = 0;
    *(uint16_t*)0x200004dc = 0;
    *(uint16_t*)0x200004de = 0;
    *(uint16_t*)0x200004e0 = 0xfff1;
    *(uint16_t*)0x200004e2 = 0x10;
    *(uint16_t*)0x200004e4 = 0xa;
    *(uint16_t*)0x200004e6 = 1;
    memcpy((void*)0x200004e8, "route\000", 6);
    *(uint16_t*)0x200004f0 = 4;
    *(uint16_t*)0x200004f2 = 2;
    *(uint64_t*)0x20000188 = 0x34;
    *(uint64_t*)0x20000298 = 1;
    *(uint64_t*)0x200002a0 = 0;
    *(uint64_t*)0x200002a8 = 0;
    *(uint32_t*)0x200002b0 = 0;
    syscall(__NR_sendmsg, r[0], 0x20000280ul, 0ul);
    break;
  case 7:
    res = syscall(__NR_socket, 0x10ul, 3ul, 0ul);
    if (res != -1)
      r[4] = res;
    break;
  case 8:
    *(uint64_t*)0x20000280 = 0;
    *(uint32_t*)0x20000288 = 0;
    *(uint64_t*)0x20000290 = 0x20000180;
    *(uint64_t*)0x20000180 = 0x200002c0;
    *(uint32_t*)0x200002c0 = 0x3c;
    *(uint16_t*)0x200002c4 = 0x2c;
    *(uint16_t*)0x200002c6 = 0xd27;
    *(uint32_t*)0x200002c8 = 0;
    *(uint32_t*)0x200002cc = 0;
    *(uint8_t*)0x200002d0 = 0;
    *(uint8_t*)0x200002d1 = 0;
    *(uint16_t*)0x200002d2 = 0;
    *(uint32_t*)0x200002d4 = r[3];
    *(uint16_t*)0x200002d8 = 0x8100;
    *(uint16_t*)0x200002da = 0;
    *(uint16_t*)0x200002dc = 0;
    *(uint16_t*)0x200002de = 0;
    *(uint16_t*)0x200002e0 = 0;
    *(uint16_t*)0x200002e2 = 0x10;
    *(uint16_t*)0x200002e4 = 0xa;
    *(uint16_t*)0x200002e6 = 1;
    memcpy((void*)0x200002e8, "route\000", 6);
    *(uint16_t*)0x200002f0 = 0xc;
    *(uint16_t*)0x200002f2 = 2;
    *(uint16_t*)0x200002f4 = 8;
    *(uint16_t*)0x200002f6 = 3;
    *(uint32_t*)0x200002f8 = 0;
    *(uint64_t*)0x20000188 = 0x3c;
    *(uint64_t*)0x20000298 = 1;
    *(uint64_t*)0x200002a0 = 0;
    *(uint64_t*)0x200002a8 = 0;
    *(uint32_t*)0x200002b0 = 0;
    syscall(__NR_sendmsg, r[4], 0x20000280ul, 0ul);
    break;
  case 9:
    *(uint64_t*)0x20000280 = 0;
    *(uint32_t*)0x20000288 = 0;
    *(uint64_t*)0x20000290 = 0x20000180;
    *(uint64_t*)0x20000180 = 0x200002c0;
    *(uint32_t*)0x200002c0 = 0x34;
    *(uint16_t*)0x200002c4 = 0x2c;
    *(uint16_t*)0x200002c6 = 0xd27;
    *(uint32_t*)0x200002c8 = 0;
    *(uint32_t*)0x200002cc = 0;
    *(uint8_t*)0x200002d0 = 0;
    *(uint8_t*)0x200002d1 = 0;
    *(uint16_t*)0x200002d2 = 0;
    *(uint32_t*)0x200002d4 = r[3];
    *(uint16_t*)0x200002d8 = 0x8100;
    *(uint16_t*)0x200002da = 0;
    *(uint16_t*)0x200002dc = 0;
    *(uint16_t*)0x200002de = 0;
    *(uint16_t*)0x200002e0 = 0;
    *(uint16_t*)0x200002e2 = 0x10;
    *(uint16_t*)0x200002e4 = 0xa;
    *(uint16_t*)0x200002e6 = 1;
    memcpy((void*)0x200002e8, "route\000", 6);
    *(uint16_t*)0x200002f0 = 4;
    *(uint16_t*)0x200002f2 = 2;
    *(uint64_t*)0x20000188 = 0x34;
    *(uint64_t*)0x20000298 = 1;
    *(uint64_t*)0x200002a0 = 0;
    *(uint64_t*)0x200002a8 = 0;
    *(uint32_t*)0x200002b0 = 0;
    syscall(__NR_sendmsg, r[4], 0x20000280ul, 0ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  use_temporary_dir();
  do_sandbox_none();
  return 0;
}
