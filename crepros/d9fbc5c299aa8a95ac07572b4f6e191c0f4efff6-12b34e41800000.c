// https://syzkaller.appspot.com/bug?id=2b6a5e7ed9c189aadc974fc5ff168b131c005947
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <errno.h>
#include <linux/futex.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <signal.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/time.h>
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
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <sys/stat.h>

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

static void exitf(const char* msg, ...)
{
  int e = errno;
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit(kRetryStatus);
}

static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* uctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  if (__atomic_load_n(&skip_segv, __ATOMIC_RELAXED) &&
      (addr < prog_start || addr > prog_end)) {
    _longjmp(segv_env, 1);
  }
  doexit(sig);
}

static void install_segv_handler()
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

#define NONFAILING(...)                                                \
  {                                                                    \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);               \
    if (_setjmp(segv_env) == 0) {                                      \
      __VA_ARGS__;                                                     \
    }                                                                  \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);               \
  }

static uint64_t current_time_ms()
{
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    fail("clock_gettime failed");
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void use_temporary_dir()
{
  char tmpdir_template[] = "./syzkaller.XXXXXX";
  char* tmpdir = mkdtemp(tmpdir_template);
  if (!tmpdir)
    fail("failed to mkdtemp");
  if (chmod(tmpdir, 0777))
    fail("failed to chmod");
  if (chdir(tmpdir))
    fail("failed to chdir");
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();

  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = 128 << 20;
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 8 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);

#define CLONE_NEWCGROUP 0x02000000

  unshare(CLONE_NEWNS);
  unshare(CLONE_NEWIPC);
  unshare(CLONE_NEWCGROUP);
  unshare(CLONE_NEWNET);
  unshare(CLONE_NEWUTS);
  unshare(CLONE_SYSVSEM);
}

static int do_sandbox_none(int executor_pid, bool enable_tun)
{
  unshare(CLONE_NEWPID);
  int pid = fork();
  if (pid < 0)
    fail("sandbox fork failed");
  if (pid)
    return pid;

  sandbox_common();

  loop();
  doexit(1);
}

static void remove_dir(const char* dir)
{
  DIR* dp;
  struct dirent* ep;
  int iter = 0;
retry:
  dp = opendir(dir);
  if (dp == NULL) {
    if (errno == EMFILE) {
      exitf("opendir(%s) failed due to NOFILE, exiting");
    }
    exitf("opendir(%s) failed", dir);
  }
  while ((ep = readdir(dp))) {
    if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
      continue;
    char filename[FILENAME_MAX];
    snprintf(filename, sizeof(filename), "%s/%s", dir, ep->d_name);
    struct stat st;
    if (lstat(filename, &st))
      exitf("lstat(%s) failed", filename);
    if (S_ISDIR(st.st_mode)) {
      remove_dir(filename);
      continue;
    }
    int i;
    for (i = 0;; i++) {
      if (unlink(filename) == 0)
        break;
      if (errno == EROFS) {
        break;
      }
      if (errno != EBUSY || i > 100)
        exitf("unlink(%s) failed", filename);
      if (umount2(filename, MNT_DETACH))
        exitf("umount(%s) failed", filename);
    }
  }
  closedir(dp);
  int i;
  for (i = 0;; i++) {
    if (rmdir(dir) == 0)
      break;
    if (i < 100) {
      if (errno == EROFS) {
        break;
      }
      if (errno == EBUSY) {
        if (umount2(dir, MNT_DETACH))
          exitf("umount(%s) failed", dir);
        continue;
      }
      if (errno == ENOTEMPTY) {
        if (iter < 100) {
          iter++;
          goto retry;
        }
      }
    }
    exitf("rmdir(%s) failed", dir);
  }
}

static void test();

void loop()
{
  int iter;
  for (iter = 0;; iter++) {
    char cwdbuf[256];
    sprintf(cwdbuf, "./%d", iter);
    if (mkdir(cwdbuf, 0777))
      fail("failed to mkdir");
    int pid = fork();
    if (pid < 0)
      fail("loop fork failed");
    if (pid == 0) {
      prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
      setpgrp();
      if (chdir(cwdbuf))
        fail("failed to chdir");
      test();
      doexit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      int res = waitpid(-1, &status, __WALL | WNOHANG);
      if (res == pid)
        break;
      usleep(1000);
      if (current_time_ms() - start > 5 * 1000) {
        kill(-pid, SIGKILL);
        kill(pid, SIGKILL);
        while (waitpid(-1, &status, __WALL) != pid) {
        }
        break;
      }
    }
    remove_dir(cwdbuf);
  }
}

struct thread_t {
  int created, running, call;
  pthread_t th;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;
static int collide;

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
    for (thread = 0; thread < sizeof(threads) / sizeof(threads[0]);
         thread++) {
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
        if (collide && call % 2)
          break;
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 20 * 1000 * 1000;
        syscall(SYS_futex, &th->running, FUTEX_WAIT, 1, &ts);
        if (running)
          usleep((call == num_calls - 1) ? 10000 : 1000);
        break;
      }
    }
  }
}

long r[5];
uint64_t procid;
void execute_call(int call)
{
  switch (call) {
  case 0:
    syscall(__NR_mmap, 0x20000000, 0xfff000, 0x3, 0x32, 0xffffffff,
            0x0);
    break;
  case 1:
    NONFAILING(memcpy((void*)0x20000000, "./file0", 8));
    syscall(__NR_open, 0x20000000, 0x82000, 0x1);
    break;
  case 2:
    NONFAILING(*(uint32_t*)0x20000000 = 0x19980330);
    NONFAILING(*(uint32_t*)0x20000004 = 0x0);
    NONFAILING(*(uint32_t*)0x20b2d000 = 0x0);
    NONFAILING(*(uint32_t*)0x20b2d004 = 0x0);
    NONFAILING(*(uint32_t*)0x20b2d008 = 0x0);
    NONFAILING(*(uint32_t*)0x20b2d00c = 0x0);
    NONFAILING(*(uint32_t*)0x20b2d010 = 0x0);
    NONFAILING(*(uint32_t*)0x20b2d014 = 0x0);
    syscall(__NR_capset, 0x20000000, 0x20b2d000);
    break;
  case 3:
    NONFAILING(memcpy((void*)0x20000ff7, "/dev/kvm", 9));
    r[0] =
        syscall(__NR_openat, 0xffffffffffffff9c, 0x20000ff7, 0x0, 0x0);
    break;
  case 4:
    NONFAILING(*(uint32_t*)0x209ff000 = 0x81);
    NONFAILING(*(uint32_t*)0x209ff004 = 0xbef);
    NONFAILING(*(uint64_t*)0x209ff008 = 0x20021000);
    NONFAILING(*(uint64_t*)0x209ff010 = 0x205af000);
    NONFAILING(*(uint8_t*)0x209ff018 = 0xe3);
    NONFAILING(*(uint8_t*)0x209ff019 = 0x0);
    NONFAILING(*(uint8_t*)0x209ff01a = 0x0);
    NONFAILING(*(uint8_t*)0x209ff01b = 0x0);
    NONFAILING(*(uint8_t*)0x209ff01c = 0x0);
    NONFAILING(*(uint8_t*)0x209ff01d = 0x0);
    NONFAILING(*(uint8_t*)0x209ff01e = 0x0);
    NONFAILING(*(uint8_t*)0x209ff01f = 0x0);
    NONFAILING(*(uint8_t*)0x209ff020 = 0x0);
    NONFAILING(*(uint8_t*)0x209ff021 = 0x0);
    NONFAILING(*(uint8_t*)0x209ff022 = 0x0);
    NONFAILING(*(uint8_t*)0x209ff023 = 0x0);
    NONFAILING(*(uint8_t*)0x209ff024 = 0x0);
    NONFAILING(*(uint8_t*)0x209ff025 = 0x0);
    NONFAILING(*(uint8_t*)0x209ff026 = 0x0);
    NONFAILING(*(uint8_t*)0x209ff027 = 0x0);
    NONFAILING(*(uint8_t*)0x209ff028 = 0x0);
    NONFAILING(*(uint8_t*)0x209ff029 = 0x0);
    NONFAILING(*(uint8_t*)0x209ff02a = 0x0);
    NONFAILING(*(uint8_t*)0x209ff02b = 0x0);
    NONFAILING(*(uint8_t*)0x209ff02c = 0x0);
    NONFAILING(*(uint8_t*)0x209ff02d = 0x0);
    NONFAILING(*(uint8_t*)0x209ff02e = 0x0);
    NONFAILING(*(uint8_t*)0x209ff02f = 0x0);
    NONFAILING(*(uint8_t*)0x209ff030 = 0x0);
    NONFAILING(*(uint8_t*)0x209ff031 = 0x0);
    NONFAILING(*(uint8_t*)0x209ff032 = 0x0);
    NONFAILING(*(uint8_t*)0x209ff033 = 0x0);
    NONFAILING(*(uint8_t*)0x209ff034 = 0x0);
    NONFAILING(*(uint8_t*)0x209ff035 = 0x0);
    NONFAILING(*(uint8_t*)0x209ff036 = 0x0);
    NONFAILING(*(uint8_t*)0x209ff037 = 0x0);
    NONFAILING(memcpy(
        (void*)0x20021000,
        "\x58\x82\xa9\xd3\xe5\x65\x41\xe5\x7f\x43\x7c\x7d\x4f\x2d\x47"
        "\x66\xc8\x11\xdd\xb6\x83\x0c\xb0\x5a\x2c\x38\x64\x6b\xd1\x93"
        "\xbd\x81\x89\xe5\xb9\x9a\xe5\xf0\xfe\xec\x40\x85\x30\x01\xf9"
        "\xda\x41\xc9\x6b\x08\x86\x8c\x40\xb2\x13\xc5\x27\xab\x2c\x43"
        "\x1a\x93\x35\x68\x8c\xdf\x9d\x84\xf1\x9c\x0e\x44\x5d\xe5\x03"
        "\x67\x14\x9b\x3e\xf9\x0e\x1a\x67\x3a\x72\x06\xb7\xaa\x80\xc3"
        "\xe4\x3d\x87\x62\xed\xad\x45\xd0\x0c\x91\x33\xd9\x16\x7d\x20"
        "\x5d\x45\xd8\x59\xe9\x5a\x2f\x9c\x72\x5c\xb2\x62\xa6\x3c\xf0"
        "\xbd\xcb\xbd\x30\x20\x7d\x3c\xd8\xa4\x0d\x80\xbf\x80\x45\xdb"
        "\xba\x0f\xf0\xd8\x44\x84\xab\x29\xcd\x8b\xf0\xe2\x3c\x60\x3a"
        "\x0b\xdd\x64\x7b\x3d\xcf\x01\x34\x30\xd5\x0d\x07\xad\x96\xb7"
        "\xad\xd6\x2b\x1a\xc6\x3a\xc0\x73\x92\x26\x73\xab\x2c\x72\x1c"
        "\x02\xfc\x19\xae\x00\xba\x1a\x3c\xf6\xb0\x91\x1c\xf9\x02\x49"
        "\x9e\x68\xf1\x28\x3f\xbd\x78\x6f\xa5\xa0\x51\x06\x24\x78\x60"
        "\x1f\x30\x42\xdf\x30\x2f\x3c\xce\xe9\x8a\xfc\x92\x52\x28\x22"
        "\xf1\x94",
        227));
    syscall(__NR_ioctl, 0xffffffff, 0x4038ae7a, 0x209ff000);
    break;
  case 5:
    syscall(__NR_setsockopt, 0xffffffff, 0x107, 0x1, 0x204d8ff0, 0x0);
    break;
  case 6:
    NONFAILING(*(uint32_t*)0x2001d000 = 0x2);
    NONFAILING(*(uint32_t*)0x2001d004 = 0x78);
    NONFAILING(*(uint8_t*)0x2001d008 = 0xe3);
    NONFAILING(*(uint8_t*)0x2001d009 = 0x0);
    NONFAILING(*(uint8_t*)0x2001d00a = 0x0);
    NONFAILING(*(uint8_t*)0x2001d00b = 0x0);
    NONFAILING(*(uint32_t*)0x2001d00c = 0x0);
    NONFAILING(*(uint64_t*)0x2001d010 = 0x0);
    NONFAILING(*(uint64_t*)0x2001d018 = 0x0);
    NONFAILING(*(uint64_t*)0x2001d020 = 0x0);
    NONFAILING(*(uint8_t*)0x2001d028 = 0xfe);
    NONFAILING(*(uint8_t*)0x2001d029 = 0x0);
    NONFAILING(*(uint8_t*)0x2001d02a = 0x0);
    NONFAILING(*(uint8_t*)0x2001d02b = 0x0);
    NONFAILING(*(uint32_t*)0x2001d02c = 0x0);
    NONFAILING(*(uint32_t*)0x2001d030 = 0x0);
    NONFAILING(*(uint32_t*)0x2001d034 = 0x0);
    NONFAILING(*(uint64_t*)0x2001d038 = 0x0);
    NONFAILING(*(uint64_t*)0x2001d040 = 0x0);
    NONFAILING(*(uint64_t*)0x2001d048 = 0x0);
    NONFAILING(*(uint64_t*)0x2001d050 = 0x0);
    NONFAILING(*(uint64_t*)0x2001d058 = 0x0);
    NONFAILING(*(uint32_t*)0x2001d060 = 0x0);
    NONFAILING(*(uint64_t*)0x2001d068 = 0x0);
    NONFAILING(*(uint32_t*)0x2001d070 = 0x0);
    NONFAILING(*(uint16_t*)0x2001d074 = 0x0);
    NONFAILING(*(uint16_t*)0x2001d076 = 0x0);
    r[1] = syscall(__NR_perf_event_open, 0x2001d000, 0x0,
                   0xffffffffffffffff, 0xffffffff, 0x0);
    break;
  case 7:
    NONFAILING(*(uint32_t*)0x20db7ff0 = 0x0);
    NONFAILING(*(uint16_t*)0x20db7ff4 = 0x1);
    NONFAILING(*(uint16_t*)0x20db7ff6 = 0x6);
    NONFAILING(*(uint8_t*)0x20db7ff8 = 0xbb);
    NONFAILING(*(uint8_t*)0x20db7ff9 = 0xbb);
    NONFAILING(*(uint8_t*)0x20db7ffa = 0xbb);
    NONFAILING(*(uint8_t*)0x20db7ffb = 0xbb);
    NONFAILING(*(uint8_t*)0x20db7ffc = 0xbb);
    NONFAILING(*(uint8_t*)0x20db7ffd = 0x0 + procid * 0x1ul);
    NONFAILING(*(uint8_t*)0x20db7ffe = 0x0);
    NONFAILING(*(uint8_t*)0x20db7fff = 0x0);
    syscall(__NR_setsockopt, 0xffffffff, 0x107, 0x1, 0x20db7ff0, 0x10);
    break;
  case 8:
    syscall(__NR_close, 0xffffffff);
    break;
  case 9:
    NONFAILING(*(uint16_t*)0x207ec000 = 0xffff);
    NONFAILING(*(uint16_t*)0x207ec002 = 0x0);
    NONFAILING(*(uint16_t*)0x207ec004 = 0x4);
    NONFAILING(*(uint32_t*)0x207ec008 = 0x0);
    NONFAILING(*(uint32_t*)0x207ec00c = 0x1fd);
    NONFAILING(*(uint32_t*)0x207ec010 = 0x0);
    NONFAILING(*(uint32_t*)0x207ec014 = 0x1);
    NONFAILING(*(uint32_t*)0x207ec018 = 0xfffffff8);
    NONFAILING(*(uint32_t*)0x207ec01c = 0x0);
    syscall(__NR_setsockopt, 0xffffffff, 0x84, 0xa, 0x207ec000, 0x20);
    break;
  case 10:
    r[2] = syscall(__NR_ioctl, r[0], 0xae01, 0x0);
    break;
  case 11:
    syscall(__NR_ioctl, 0xffffffff, 0xae60);
    break;
  case 12:
    r[3] = syscall(__NR_ioctl, r[2], 0xae41, 0x0);
    break;
  case 13:
    syscall(__NR_clock_gettime, 0x0, 0x20000000);
    break;
  case 14:
    NONFAILING(*(uint32_t*)0x20001000 = 0x10005);
    NONFAILING(*(uint32_t*)0x20001004 = 0x0);
    NONFAILING(*(uint64_t*)0x20001008 = 0x0);
    NONFAILING(*(uint64_t*)0x20001010 = 0x2000);
    NONFAILING(*(uint64_t*)0x20001018 = 0x20000000);
    syscall(__NR_ioctl, r[2], 0x4020ae46, 0x20001000);
    break;
  case 15:
    syscall(__NR_ioctl, r[3], 0xaeb7);
    break;
  case 16:
    syscall(__NR_ioctl, r[1], 0x541b, 0x2069dffc);
    break;
  case 17:
    NONFAILING(*(uint32_t*)0x202ccffc = 0xe8);
    if (syscall(__NR_getsockopt, 0xffffffff, 0x29, 0x23, 0x2070bf18,
                0x202ccffc) != -1) {
      NONFAILING(r[4] = *(uint32_t*)0x2070bf4c);
    }
    break;
  case 18:
    syscall(__NR_setfsuid, r[4]);
    break;
  case 19:
    syscall(__NR_ioctl, r[3], 0xae80, 0x0);
    break;
  case 20:
    syscall(__NR_ioctl, r[3], 0xae80, 0x0);
    break;
  }
}

void test()
{
  memset(r, -1, sizeof(r));
  execute(21);
  collide = 1;
  execute(21);
}

int main()
{
  int i;
  for (i = 0; i < 8; i++) {
    if (fork() == 0) {
      procid = i;
      install_segv_handler();
      use_temporary_dir();
      int pid = do_sandbox_none(i, false);
      int status = 0;
      while (waitpid(pid, &status, __WALL) != pid) {
      }
      return 0;
    }
  }
  sleep(1000000);
  return 0;
}
