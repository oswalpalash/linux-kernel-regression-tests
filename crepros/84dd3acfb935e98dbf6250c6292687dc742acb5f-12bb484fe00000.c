// https://syzkaller.appspot.com/bug?id=84dd3acfb935e98dbf6250c6292687dc742acb5f
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
#include <linux/loop.h>

static unsigned long long procid;

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

struct fs_image_segment {
  void* data;
  uintptr_t size;
  uintptr_t offset;
};

#define IMAGE_MAX_SEGMENTS 4096
#define IMAGE_MAX_SIZE (129 << 20)

#define sys_memfd_create 319

static unsigned long fs_image_segment_check(unsigned long size,
                                            unsigned long nsegs, long segments)
{
  unsigned long i;
  struct fs_image_segment* segs = (struct fs_image_segment*)segments;
  if (nsegs > IMAGE_MAX_SEGMENTS)
    nsegs = IMAGE_MAX_SEGMENTS;
  for (i = 0; i < nsegs; i++) {
    if (segs[i].size > IMAGE_MAX_SIZE)
      segs[i].size = IMAGE_MAX_SIZE;
    segs[i].offset %= IMAGE_MAX_SIZE;
    if (segs[i].offset > IMAGE_MAX_SIZE - segs[i].size)
      segs[i].offset = IMAGE_MAX_SIZE - segs[i].size;
    if (size < segs[i].offset + segs[i].offset)
      size = segs[i].offset + segs[i].offset;
  }
  if (size > IMAGE_MAX_SIZE)
    size = IMAGE_MAX_SIZE;
  return size;
}

static long syz_mount_image(volatile long fsarg, volatile long dir,
                            volatile unsigned long size,
                            volatile unsigned long nsegs,
                            volatile long segments, volatile long flags,
                            volatile long optsarg)
{
  char loopname[64], fs[32], opts[256];
  int loopfd, err = 0, res = -1;
  unsigned long i;
  NONFAILING(size = fs_image_segment_check(size, nsegs, segments));
  int memfd = syscall(sys_memfd_create, "syz_mount_image", 0);
  if (memfd == -1) {
    err = errno;
    goto error;
  }
  if (ftruncate(memfd, size)) {
    err = errno;
    goto error_close_memfd;
  }
  for (i = 0; i < nsegs; i++) {
    struct fs_image_segment* segs = (struct fs_image_segment*)segments;
    int res1 = 0;
    NONFAILING(res1 =
                   pwrite(memfd, segs[i].data, segs[i].size, segs[i].offset));
    if (res1 < 0) {
    }
  }
  snprintf(loopname, sizeof(loopname), "/dev/loop%llu", procid);
  loopfd = open(loopname, O_RDWR);
  if (loopfd == -1) {
    err = errno;
    goto error_close_memfd;
  }
  if (ioctl(loopfd, LOOP_SET_FD, memfd)) {
    if (errno != EBUSY) {
      err = errno;
      goto error_close_loop;
    }
    ioctl(loopfd, LOOP_CLR_FD, 0);
    usleep(1000);
    if (ioctl(loopfd, LOOP_SET_FD, memfd)) {
      err = errno;
      goto error_close_loop;
    }
  }
  mkdir((char*)dir, 0777);
  memset(fs, 0, sizeof(fs));
  NONFAILING(strncpy(fs, (char*)fsarg, sizeof(fs) - 1));
  memset(opts, 0, sizeof(opts));
  NONFAILING(strncpy(opts, (char*)optsarg, sizeof(opts) - 32));
  if (strcmp(fs, "iso9660") == 0) {
    flags |= MS_RDONLY;
  } else if (strncmp(fs, "ext", 3) == 0) {
    if (strstr(opts, "errors=panic") || strstr(opts, "errors=remount-ro") == 0)
      strcat(opts, ",errors=continue");
  } else if (strcmp(fs, "xfs") == 0) {
    strcat(opts, ",nouuid");
  }
  if (mount(loopname, (char*)dir, fs, flags, opts)) {
    err = errno;
    goto error_clear_loop;
  }
  res = 0;
error_clear_loop:
  ioctl(loopfd, LOOP_CLR_FD, 0);
error_close_loop:
  close(loopfd);
error_close_memfd:
  close(memfd);
error:
  errno = err;
  return res;
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

static void reset_loop()
{
  char buf[64];
  snprintf(buf, sizeof(buf), "/dev/loop%llu", procid);
  int loopfd = open(buf, O_RDWR);
  if (loopfd != -1) {
    ioctl(loopfd, LOOP_CLR_FD, 0);
    close(loopfd);
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
  int collide = 0;
again:
  for (call = 0; call < 3; call++) {
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
      event_timedwait(&th->done, 45 + (call == 0 ? 100 : 0));
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
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
    reset_loop();
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

uint64_t r[1] = {0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res;
  switch (call) {
  case 0:
    NONFAILING(memcpy((void*)0x200000c0, "minix\000", 6));
    NONFAILING(memcpy((void*)0x20000080, "./file0\000", 8));
    NONFAILING(*(uint64_t*)0x20000000 = 0x20000140);
    NONFAILING(memcpy((void*)0x20000140, "\x60\x00\x84\xe0\x02\x00\x0a\x00\x90"
                                         "\x0c\xda\x40\xff\x1a\xd5\xc9\x8f\x13",
                      18));
    NONFAILING(*(uint64_t*)0x20000008 = 0x12);
    NONFAILING(*(uint64_t*)0x20000010 = 0x400);
    NONFAILING(*(uint64_t*)0x20000018 = 0x20000640);
    NONFAILING(memcpy(
        (void*)0x20000640,
        "\xee\x9f\x84\xa4\x3e\xcc\xaf\x13\x15\xbb\x39\x7d\x00\xc1\xb6\xdd\x82"
        "\x0d\xd1\x39\x16\x18\x1d\xba\x62\x09\x38\xcb\x30\xf1\xc2\x1a\xfe\x3d"
        "\x92\x62\x98\xb2\x58\x34\xe8\x90\x09\x2d\x64\x2c\xaf\xe5\xdb\x21\xba"
        "\x94\x94\xfe\xbc\xf6\x60\x79\xb7\x22\xe7\x3b\xe3\xa8\x70\x8a\x36\x24"
        "\x25\x0f\x9f\x59\xfb\xbe\x94\x29\x97\xc6\xc1\xe8\x3a\x34\xe5\x77\x9e"
        "\x8a\x4f\x40\xfd\xb6\xff\xe5\x9f\xfe\x49\x03\x50\x0d\x3c\xd8\xc8\xf3"
        "\xed\x19\xad\x17\x6d\xaf\x94\xcf\x7f\xca\xf2\x92\x2f\x8a\xab\x3b\xec"
        "\xc6\x0c\x8d\x39\xdd\xf4\x3c\xd6\xd2\xe9\x30\x1d\xa3\xca\xa3\x57\x19"
        "\xf2\x5a\x83\x47\x85\x59\xe0\xe8\x91\xdc\xb7\x56\xee\x22\x96\x4d\x03"
        "\xf3\x75\x61\xf5\x79\x74\x90\xa0\xb9\x35\x06\x48\x28\x22\x07\x5b\xa9"
        "\x6d\xb3\x5a\x44\x0d\x04\x6e\x44\xeb\x4a\x79\x98\x6b\x51\x82\x81\x60"
        "\x90\x03\xb6\xfa\xe0\x4a\x18\xbe\x9a\x83\xce\x7e\xfa\x2a\xb8\x0f\x75"
        "\xc9\x0d\x78\xf0\xb4\x17\x9d\x67\x43\x76\xdb\x1f\x7a\xe8\x74\x56\x41"
        "\xad\xcd\xe0\xd2\x45\x72\x27\x44\xbc\xea\x2f\xac\x94\x45\x01\x4e\xe2"
        "\x29\x39\x85\x11\x95\x73\xff\xcc\x0c\x22\xeb\x07\x69\xde\x1e\x9e\x5d"
        "\xb5\x3a\xa3\x37\x8a\x44\x26\x50\x30\x10\xa7\xc6\x24\x72\x65\x83\x82"
        "\x11\x52\x7a\x01\x32\xcc\xf8\x83\xee\x08\xa6\xea\x1a\x78\x5e\x47\xce"
        "\x16\x66\x3f\x1c\x74\x4a\xf4\x92\x00\xd5\xf3\xbc\x32\x6e\x7d\x90\x89"
        "\xa5\xf1\x7d\x0a\xd0\x5f\x06\xe4\x8c\x20\x5f\x9b\x6d\x08\x7b\xa7\xd3"
        "\xb6\x74\x5e\x65\x70\x0c\x00\x95\x72\xa6\x5b\x4b\xfb\xf6\x68\x78\x21"
        "\xbc\x72\x3b\x61\xb5\xab\xe7\x35\x7e\x68\xa9\xb0\x64\x0b\xfb\x9c\x01"
        "\x00\x00\x80\x00\x00\x00\x00\x0e\x84\x0e\xcf\x37\x9c\x57\x6c\x05\x75"
        "\x7d\x44\xb1\x9d\xe0\xe2\xaa\x36\x4c\x42\x15\x62\x19\xa7\x2c\xc9\x43"
        "\xa6\x0f\x04\x10\x89\x1e\xd1\x34\xf3\x5f\x5f\xe6\xb4\xcf\x3a\xbb\xcb"
        "\x98\x44\x36\x32\x23\x58\xa7\xc9\x39\x37\x24\x86\x0d\x87\x25\x39\x81"
        "\xf9\xe7\x9d\xbe\xb7\x89\x57\x7e\xad\x0e\xde\x31\xe1\x9a\x2b\x11\x4a"
        "\x0e\x4d\x82\xb3\xe1\x02\x3d\xd2\x8b\x63\x82\xe6\x7e\x32\x46\xd2\x91"
        "\x2e\x55\xcf\x99\x28\xda\x32\xc1\x02\x7c\xef\x7c\x6c\x61\x0e\xa8\xfc"
        "\xe1\x49\xa7\xaf\x20\x22\x4b\x4e\x02\x86\x01\x93\x6e\xf9\xa7\x8d\x80"
        "\xff\x55\x67\x08\x4e\xc1\x28\x76\x6e\x82\x2d\x61\x24\x18\x6d\x82\xe7"
        "\x60\x12\x8d\xd7\xde\x26\x53\x93\x9d\x35\x3c\x12\xf9\x6d\xef\xf3\xa2"
        "\x8b\x02\x2c\x78\x4b\xd3\x7d\xf7\xf7\x66\x40\xbc\xbc\xb0\x1a\x46\x76"
        "\xd5\x8b\x3e\x9c\x2b\xaa\xe6\x62\x30\xe5\xf5\x4a\x37\x52\x7e\x81\x29"
        "\xd1\x61\xb0\xc0\x6f\x25\x64\x8c\x55\xa7\xe5\xb2\xdb\x52\x80\x53\xc3"
        "\xe3\x86\x4f\x41\x72\x8b\x79\x35\xe5\x75\x56\x8a\xd1\x14\xeb\x8c\x81"
        "\x1b\xf1\x9e\x07\xa3\x98\xba\xbb\xc6\x4f\xbe\xab\x84\x26\x88\x55\x47"
        "\x83\xed\x15\x51\x94\x9a\x79\x1e\x33\x79\x9e\x59\xa3\x4b\x6b\xda\xbc"
        "\x34\x58\xc3\x79\xc7\x35\x19\x82\x92\xe5\xa2\x72\x18\x74\x49\x24\x9d"
        "\x2c\x8a\x9a\xa5\x8f\x38\x35\xa3\xe1\x71\x60\x83\xbb\x04\x64\xcb\xe1"
        "\x40\xd1\x58\x7a\x21\xe4\xec\x2a\xe1\xf3\xad\x81\x13\x4d\xf5\x59\x03"
        "\xff\xb8\xe1\x73\x64\x63\x52\x91\x5a\x2c\x70\x67\x09\xcf\x46\x53\x89"
        "\x78\x22\x4c\x0d\x6d\xc4\x37\xcb\xfc\x37\xab\xfb\xd1\xb7\x6f\xeb\x5c"
        "\xa3\xae\xb1\xac\x8c\xbd\x40\xd5\xba\x89\x6f\x79\xee\x8f\x76\xb0\x80"
        "\x9f\x59\xb8\x68\x62\x64\x87\x74\xd2\xac\xe9\x8b\x82\x5e\x7a\x46\x5b"
        "\x5d\xd8\x0e\x49\x19\x65\x97\x1e\x77\x97\xab\xa3\x96\x84\x41\xc7\x77"
        "\x17\xa2\x4c\xa4\x1e\xfb\x16\x0c\x03\x0c\xfc\x8e",
        777));
    NONFAILING(*(uint64_t*)0x20000020 = 0x309);
    NONFAILING(*(uint64_t*)0x20000028 = 0x34f9);
    syz_mount_image(0x200000c0, 0x20000080, 0, 2, 0x20000000, 0, 0);
    break;
  case 1:
    NONFAILING(memcpy((void*)0x20000600, "./file0\000", 8));
    res = syscall(__NR_creat, 0x20000600ul, 0ul);
    if (res != -1)
      r[0] = res;
    break;
  case 2:
    NONFAILING(sprintf((char*)0x200004c0, "0x%016llx", (long long)0));
    syscall(__NR_write, r[0], 0x200004c0ul, 0xfdeful);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0ul);
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
