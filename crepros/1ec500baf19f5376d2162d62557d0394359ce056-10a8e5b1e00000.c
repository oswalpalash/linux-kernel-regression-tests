// https://syzkaller.appspot.com/bug?id=1ec500baf19f5376d2162d62557d0394359ce056
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
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
      event_timedwait(&th->done, 45);
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
    memcpy((void*)0x20000080, "/dev/uhid\000", 10);
    res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000080ul, 2ul, 0ul);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    memcpy(
        (void*)0x20000b40,
        "\x0b\x00\x00\x00\x73\x79\x7a\x31\x03\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x8a\x8e\x87\x4b"
        "\x73\xe9\x90\xb4\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x2b\x40\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x73\x8d\x7a\x31"
        "\x00\x00\x00\x00\x00\xff\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x73\x79\x7a\x31\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xcf"
        "\x0a\x00\x00\x00\x00\x00\x00\x00\x00\x0c\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x16\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x3b\x38\xe9\x67\xac\x82\x06\xea\xae"
        "\x86\xb9\x7e\xec\x0b\x2b\xed\x1e\xe2\x33\x64\xb1\x0d\x6a\xad\x51\x02"
        "\x00\x00\x00\xe2\xa5\xdb\x3c\x6a\x0d\xee\x4a\xfc\x66\xd2\x44\x28\x05"
        "\x20\x1c\x39\x38\x9a\x80\x4c\x41\xc2\x99\x3f\xc6\x7e\x8a\x14\x60\x45"
        "\xe1\x4a\x8a\x08\x00\x55\x0e\x6a\x25\xc0\xef\x65\xf6\x00\x00\x00\x00"
        "\x00\x00\x00\x03\x18\x7f\xaf\xa4\xa1\xee\x6e\xce\x53\xc6\x73\x85\xb8"
        "\x83\xa3\x6a\xd2\x4a\x04\x00\x00\x00\x00\x00\x00\x00\x6a\x8a\xb1\x1b"
        "\x0a\x0b\x00\xe7\x7e\x6c\x16\x18\x9c\xfa\x16\xcb\xe0\x1a\x4c\xe4\x11"
        "\x37\x8e\xaa\xb7\x37\x2d\xab\x5e\xef\x84\xc3\x1b\x2d\xad\x86\x8a\x53"
        "\xe6\xf5\xe6\x97\x46\xa7\x1e\xc9\x2d\xca\xa9\xa7\xdf\xab\x39\x42\x86"
        "\xe5\xc8\x1e\xae\x45\xe3\xa2\x5b\x94\x2b\x8d\xa1\x1e\xdb\x57\x8b\x45"
        "\x3a\xca\xc0\x3a\x9d\x34\x48\x00\x00\x00\x00\x83\xd6\xd5\xfe\x4f\x83"
        "\x3d\x4d\x4c\xfb\xee\xf0\xe0\xe6\x2b\xe2\x05\x00\x00\x00\x3c\x32\x98"
        "\x4c\x6c\x4b\x2b\x9c\x33\xd8\xa6\x24\xce\xa9\x5c\x3b\x3c\x6d\xd8\x73"
        "\x56\x9c\xf4\x78\x6f\xc5\x16\x6b\x03\x00\x00\x00\x00\x00\x1f\xf2\x8d"
        "\x3c\xe3\xe3\xb8\xf8\x1e\x34\xcf\x97\xc9\xc8\x41\xcb\x2e\xf0\x81\x07"
        "\xa9\xa9\x65\x49\xe3\xd2\x59\xdf\x17\xe2\x9e\xd6\x4b\xd6\x12\x08\x13"
        "\xf9\xf0\x34\x4e\x13\x95\x06\x70\x1e\x8f\xde\xdb\x06\x00\x9b\x5e\x4d"
        "\x0c\x67\xbd\xa0\xb9\x28\xb7\x32\xcf\xf7\x82\xb0\x68\x40\x75\xf2\xcb"
        "\x78\x51\xef\xdd\x77\x97\xee\x95\xd2\xac\x28\xa8\xca\xbd\x26\xc1\x56"
        "\x82\xaa\x78\xd3\x1a\xec\x95\x6b\xd7\xc2\x78\x06\x40\x34\x34\xb3\xc3"
        "\x0b\x07\x0b\xcc\x82\x66\xe1\x2f\xa6\x66\x02\x05\x62\x56\xf7\x46\x75"
        "\xb7\xcb\x4d\x87\xb2\x70\x8d\x70\xc8\xf3\xdf\x53\xca\xf8\xfe\x18\x0c"
        "\x4d\xea\x3f\x5b\x7a\x87\x1b\x30\xc7\xa5\x75\x3b\x48\xf7\xf0\x91\x92"
        "\xa3\x4b\x0e\xfa\xab\x02\xdc\xa0\x51\x7e\xee\x50\xff\x30\x20\x6f\x78"
        "\xec\x82\xc7\x2f\x33\xe9\x09\xc1\xae\x1f\xe9\x4f\xe0\x75\x9f\x07\xd1"
        "\x64\x02\x59\x3e\x33\x35\xd4\x20\x00\x88\x8c\x90\x5f\xb4\x1d\xbd\xb4"
        "\x20\xea\x9f\x87\xfc\x86\x51\x26\x85\xeb\x72\x61\xe9\xc2\x5e\xdb\x60"
        "\x95\xe9\xec\x2b\x74\x3b\xcf\x0f\x46\x8b\xad\xa0\xb8\x7a\x02\x60\x65"
        "\x30\xc2\x78\xa4\xc6",
        770);
    syscall(__NR_write, r[0], 0x20000b40ul, 0x12eul);
    break;
  case 2:
    *(uint32_t*)0x200016c0 = 8;
    memcpy(
        (void*)0x200016c4,
        "\x46\x5e\xda\x69\x80\x61\xe4\x5e\x48\x36\xa5\x54\xc2\xf3\x8f\x91\x2f"
        "\x9b\xdb\x31\x41\xff\xb6\x6c\x12\xb6\x66\x41\xb3\x10\x3c\xaa\xec\x47"
        "\x54\x28\x38\x98\x6f\xd1\x45\xa8\x31\xb5\xd8\x38\xfc\xcb\xf3\x0f\x21"
        "\xd2\x7a\x07\x39\x2c\xcb\xd7\x4a\x22\x15\x98\x57\xa9\xb8\x80\x8a\x8f"
        "\xe8\x4f\xfb\x6a\xe9\x1e\xf9\x2c\x03\x0e\xb3\xaa\x91\x07\xea\x3a\xed"
        "\x31\x1c\x83\xab\xc6\x4e\xe4\xd1\xbe\xf2\x3b\x07\x0f\xba\xcc\x55\x8b"
        "\x76\xf8\x55\x35\x8d\x3f\x33\x49\x0d\xdd\x87\xe3\xe7\x09\x85\x81\x55"
        "\x9c\xe0\xa2\x89\xd9\x9f\xb4\xd9\x3a\x5e\x5a\x48\x33\xa1\x86\x1e\x4b"
        "\x85\xcb\x46\xc0\xff\x67\x5b\x8e\xaa\x4f\x92\x35\xf5\xe7\x16\xb2\x3b"
        "\x57\xcf\x8c\xa7\x33\x30\xae\xc0\xd3\x27\x4a\x83\x43\x36\xeb\x25\xf7"
        "\xe4\xd7\xdb\x57\xeb\xe4\x84\x25\x5d\x3e\x26\xd2\x99\xd6\xcd\x83\xd8"
        "\xdb\x06\xd1\x9b\xc5\x54\x7e\xb0\x88\x74\xad\x01\xb0\x05\xd1\xf0\xe4"
        "\x2f\x61\xf5\x0a\x8b\x0e\xf7\xe2\xc7\x9c\xeb\x80\xf7\x29\xf9\xd8\x34"
        "\x2a\x83\x4e\x1a\x0d\x91\x9f\x44\x1a\x98\x82\x70\x53\xd2\x05\x1d\xb7"
        "\x27\x15\x64\x7e\xc7\x56\x51\xfc\xef\xa6\xd1\xfa\xdc\x93\xec\xd2\xc1"
        "\x44\x6a\x34\x42\x97\x05\x90\x49\xfa\x53\x5c\x0a\x06\x28\xb2\xf8\x37"
        "\x77\x82\xa2\x5f\x65\xa5\xf0\x3b\x30\xaa\x3d\x48\xcf\x3e\x57\x90\x8a"
        "\xd5\x9a\xa5\x62\x7d\x4e\xe4\xd5\x2d\xdd\x44\xd8\x9b\x83\x35\xa2\xa3"
        "\xe0\x80\x40\xe0\x09\x4f\x8b\x4a\xf4\xe3\x99\x54\xe4\x3a\xab\x48\xbb"
        "\xbd\x96\x35\xc1\x8e\xf4\x77\xd4\x8f\x79\x16\x1b\x73\x49\x48\xe1\xae"
        "\x2a\x38\x7f\x69\xc0\x37\x58\xf8\x17\xf1\xeb\x7c\xa7\x9b\x68\xef\x8e"
        "\x57\x85\x09\x92\xd3\x6a\xb0\x14\x86\x07\x4c\x19\x7e\x41\x82\x47\x15"
        "\x86\x1a\x7f\xf8\xd9\x4d\x9f\x2e\x6f\xb7\xd7\xc3\x1a\x59\xa2\x52\x81"
        "\x2a\xd1\x05\x76\x37\xdb\xfc\x07\x20\xc4\x44\xd5\xf8\x7d\xbc\x16\x35"
        "\x96\x8f\x18\xce\x6b\xa8\xd4\xa9\xea\x89\xd4\x3f\x94\x3b\x98\xe1\xa3"
        "\xcb\x14\xf3\xd1\xc6\x9e\xc4\x8a\x6a\xb7\xf2\x8e\x8b\x9d\xa1\x60\x8e"
        "\xa9\xe7\x12\x6f\x97\x57\xae\x65\xe8\x97\x87\x33\x87\x5d\x19\x00\x93"
        "\x3d\x05\x0f\xa7\x71\x31\xbb\xd7\x69\x22\x0a\x00\xd8\xc9\xba\x03\xbf"
        "\x5c\x75\x6a\x2a\x79\x24\x12\x92\x8f\xf9\x41\x45\x2a\x3d\xae\xbb\x0b"
        "\x23\xed\xc1\xf2\x32\x0f\x83\x26\x15\x03\x96\xa0\x85\x73\x0a\xd5\x7e"
        "\x2f\x9a\xe7\x26\xea\x01\x2c\x59\xa6\xa6\x19\x12\xff\x71\x05\x36\x7b"
        "\xb6\x28\xa5\xf1\xe8\x04\x71\x79\xa2\x60\x23\xe8\x82\xd6\x3a\xdb\x31"
        "\x99\x1b\xee\xde\x69\x40\x48\xba\x9e\xa2\x5e\x88\x26\x50\x7d\xd7\xbb"
        "\x36\x92\x8d\x90\x99\x3c\x69\x90\x0a\xcf\x64\x8a\x9b\xe7\x01\x97\x2b"
        "\x19\x2a\x12\xe7\xe9\x88\xe2\xe3\x85\x00\x24\xa3\x2e\x33\x57\x13\x3f"
        "\x30\xd4\x90\xd0\x7d\x23\x03\x9b\x90\xb4\x15\x65\x62\xd1\x23\xb1\x61"
        "\x10\x0f\x96\xc7\xa0\x65\x8b\x1b\xeb\xa7\x83\x1b\x18\x83\xcd\xa9\xee"
        "\x44\xc4\x98\xc0\xf9\x14\x1d\xc1\x51\x31\xf3\x94\x6e\xa6\x7a\x1a\xb6"
        "\xe6\xcb\xc2\xb1\xae\xb1\x86\x9c\xfa\x62\xc6\xf9\x95\x95\x61\xdc\x7f"
        "\x4d\x01\x1b\x27\x8b\xcf\x33\xa5\xc0\xa1\x4e\xd4\xbd\x48\x78\x26\x00"
        "\x53\x90\xbb\x1b\xdc\x1e\x53\x65\xb1\xd5\xc3\xc2\x70\xb1\xbe\x44\xa7"
        "\xf1\xaf\xe6\xb1\xc4\x01\xa6\xd4\x44\x37\x0d\x7e\x05\x84\xf3\xd7\xed"
        "\xce\xdd\x31\x20\x4c\x1f\x3a\x96\x6e\x6e\x19\xdc\xd0\x68\x00\xa2\xda"
        "\x33\xf1\xde\x49\x06\xcc\x7a\xef\x79\xf0\x40\x55\x86\xa4\x0f\x7b\x02"
        "\x27\xb8\x03\x15\x79\x0b\x5a\x07\x79\xa5\xab\x7d\x9c\xfa\xc9\x55\xee"
        "\x3b\xd2\xad\x94\x1a\x7c\x93\xc7\x5c\x01\x28\x8d\x84\x5d\x4e\x62\xc0"
        "\x62\xb3\xbf\x45\x65\x11\xd4\xc6\x2f\x73\x2a\xf1\xee\x05\x43\xa0\xfc"
        "\xcf\x34\xd9\x2e\x65\x95\x06\x21\x7e\x73\x50\x07\x17\xe3\xcc\x45\x66"
        "\x3f\x90\xfe\xb5\x7a\x55\xc0\x3f\xe3\x98\x7b\x93\xb3\xa9\x9f\x98\xe4"
        "\x4d\xd5\xd6\xa7\x28\x1b\xad\xbd\x62\x0f\xed\x1b\xb2\xb0\x4c\x80\x5f"
        "\x82\x6b\x27\x01\x8e\xbb\x82\xbf\x76\x72\xd3\xe6\xe8\x98\x67\x9a\xab"
        "\x44\xc0\x1d\x8a\x23\xd9\x2d\x09\xc3\x43\xac\x4c\xdc\xe4\x74\x83\x17"
        "\x07\x00\x4a\xc6\x72\xdd\xae\x5b\x59\x43\x37\x0d\xf0\xb2\x29\x2f\x27"
        "\x3c\x29\x9a\xc5\x9d\x3e\xec\xb9\x45\x87\xd3\x97\x75\xee\xe6\x5a\x24"
        "\x06\x71\x16\xb6\x94\x47\x06\x6a\xd6\xa5\xaa\x82\xb6\x2c\xcb\x23\x6a"
        "\x73\xc4\xd6\xaa\xa1\xa1\xea\xa7\xef\x08\x82\xfd\xfe\xf5\xc7\xe2\xe6"
        "\x37\xac\x88\xd3\x0d\xc8\x9a\xfa\x01\x19\xc4\x51\x1a\xf5\xba\x51\x18"
        "\x11\xa2\xc4\x6c\xb8\x11\x1b\x24\x15\xc6\x7e\x22\x04\x1f\x04\xeb\xb0"
        "\x3b\x6e\x0c\xa0\xd7\x24\xe2\x55\xf0\x11\xc1\x9a\x07\xa1\x6d\x76\xd9"
        "\xb5\x2f\xf8\xff\x0d\x4a\xa1\x73\x6c\x35\x06\x75\xec\x15\xbd\xde\xe6"
        "\xab\xfb\x61\xee\x9e\x4e\x4a\xee\x43\x12\x86\x4e\x48\xb1\xed\x9b\x1b"
        "\x8d\x00\x5c\x18\x4c\x0c\x0b\x1e\x64\xfe\x61\xb7\x37\x6b\x70\xbd\xf9"
        "\x18\x11\x18\xa8\xdd\xec\x45\x5c\x24\x49\x73\xbe\x2e\xf3\x14\xea\x12"
        "\x2d\x8b\x0c\x42\x50\x6f\x96\xdf\xeb\x39\x76\x10\x64\x8c\x4e\xf6\xbe"
        "\x8b\x52\x22\x8d\xae\xbc\x9a\xea\x7c\x91\x53\x86\xb7\xa0\x6b\x91\xca"
        "\xf4\xd1\xfb\xec\xab\xbf\xbc\x33\x30\x6c\xe5\x6b\x4b\x4c\x98\x72\x6b"
        "\x60\xae\xc4\xcd\xa8\xa7\xce\x82\x31\x7b\xa8\x4e\x9c\x10\x3e\x58\x82"
        "\xaf\xb6\x39\xc0\xea\x47\xc1\xb0\xe0\xfe\x1f\xdd\xe5\xf8\xd3\x52\xed"
        "\x57\x4f\xba\xe8\x69\xd1\x66\xd4\xac\xd6\x1a\xf3\x43\x82\x3f\xb9\xc8"
        "\x64\xa5\xb9\xa4\x95\x02\xc8\x88\xa0\x46\x1f\xfe\xb1\x9f\x00\xcd\x43"
        "\xad\xe8\x55\x9b\xb7\xb9\xea\x34\x9d\x2e\xb9\xe9\xb2\x8b\xbc\x9c\xa0"
        "\x8e\x77\x60\x67\x5e\xb6\x07\x4c\x79\x13\xba\xe4\x0d\x0d\x8c\x8f\x82"
        "\x4c\xc3\xc7\x73\x7b\x90\xe4\xb2\x7d\xce\xc3\xb3\x96\xde\x8d\x7a\x4b"
        "\x2c\x2f\x63\x15\x9d\xac\xa5\x8a\xca\x8f\xe1\x34\x09\xcf\xe2\xb6\x99"
        "\x62\xc1\x0a\x07\x02\x9e\x1d\x78\xeb\x51\xee\x40\xe8\x09\xbc\x95\x01"
        "\xa8\xe9\xd5\xb9\xa1\x5c\x1e\x5e\xbb\xe4\x5b\x06\xc8\x7c\xd1\xd9\xbb"
        "\x05\xfe\x61\x13\xc3\x61\x10\xba\x9f\x01\x57\x2e\x40\xf7\x97\x9e\x60"
        "\x1d\x33\xd5\xa1\x4f\x59\x8b\x10\xa3\x57\xd6\x4e\x86\x14\x9d\x90\x0a"
        "\x18\x52\xa8\x87\x17\xf6\xc9\x15\xc4\x24\xeb\x4d\xf1\x20\xd2\x5d\xf0"
        "\xde\xa8\x7a\x85\x32\x90\xc5\x1c\x83\xfb\xb5\x79\x37\x1c\x8f\x63\x70"
        "\xca\xe5\x5a\x9c\x33\x5a\x39\x37\xe4\x46\xe7\x42\xc2\x89\x4e\x00\xab"
        "\x47\x87\x98\x58\xed\xd4\x4c\x40\x90\x7f\xcf\xa6\xcd\xfe\x0b\xdd\xd7"
        "\x7d\x35\x75\xf4\x40\xd3\x99\xbc\x87\x74\x1c\xa4\x1e\xd1\xe9\x6c\xbe"
        "\x17\xb5\xd8\xb8\x33\x3b\x4e\xd9\x94\xad\x43\x85\x1d\x76\xf6\x89\xcc"
        "\x43\x18\x8e\x9e\x80\xe4\x65\x19\x7e\x4b\x6f\xff\x40\x19\x59\xe7\xe2"
        "\xe4\x11\x63\x1b\xb4\x82\x1e\x7a\x39\xec\xc7\x7e\x80\x1c\xc7\x02\x5c"
        "\xfe\x30\xa8\xcb\x0d\x8b\x9f\xd2\xcd\x05\xc8\x64\x2e\x73\xa9\x47\xc0"
        "\x79\xbc\x81\x24\xaa\x0b\x02\xe4\x30\xed\x96\xd2\xeb\x0e\x58\x53\x02"
        "\x5f\xfa\x1e\x20\xb7\xb2\x9e\xd4\x65\xa7\xfd\xea\xf0\xbf\x78\x53\x17"
        "\x89\x19\xe9\xa8\x9d\x71\x44\x1a\x79\xb1\xe9\x06\x3b\x7b\xe5\x99\x4a"
        "\xe1\x69\x2c\xe1\xad\xad\x20\xd4\xa5\xb6\x74\x59\xb4\xac\xdf\x66\x36"
        "\xa3\x93\x90\xc4\xc1\x23\x5a\x1d\x54\x83\xc7\x42\x3d\x74\x05\xa5\xe6"
        "\x84\x9f\xf8\x62\x2c\x88\xb3\x2a\xb0\x5a\x53\x6f\x3f\xe2\xbb\x04\x57"
        "\x71\x39\x58\xea\x67\xcd\x7c\x8f\x30\xa7\x43\x87\x8a\xf4\x21\x3d\x68"
        "\x2e\xfb\xa7\x98\x99\xd8\x91\xd0\xb6\x31\x9b\xc5\x03\x21\x19\xd2\x6b"
        "\x3b\x7f\x1b\xe6\xba\x39\x06\xb1\x32\x40\xda\x9c\x24\x33\x7e\x7d\x02"
        "\xbe\x14\xd4\x9d\xaf\xba\xa9\x81\xe8\xd1\xe2\x03\xcf\xc2\xe5\x66\x4c"
        "\xb8\x52\xa6\xfd\x56\x4e\x8b\x83\xac\x01\x3d\xc8\xbe\xdf\x17\xb4\xbb"
        "\xc4\xc2\x91\x9a\x3d\xb4\x91\xc8\xb0\x9b\x5e\x2e\xd2\xa9\xde\x2c\xbc"
        "\x2e\xad\x94\xc3\xf2\xaa\x4b\xb3\x65\xe2\xa4\x02\x11\x70\x99\x84\x38"
        "\x88\xdd\x02\x0b\x30\xd4\x6a\x1c\xba\xea\xdc\x39\xf3\x12\x15\x45\x92"
        "\xa1\xa0\x44\x93\x88\x99\xed\x69\xc6\xd2\x5f\x32\xdf\x2f\xe1\x94\xaa"
        "\x03\xcd\xb2\xd9\x47\xb5\xd9\x3c\xe3\x0b\x5e\x4b\x14\xe4\xad\x69\x31"
        "\x07\x3a\x6c\x61\x22\xc9\xe8\xe8\xdc\x04\x3f\x84\x2a\x3b\xc9\x41\xbc"
        "\xd3\xc6\xcf\xcb\xf7\xd2\xa5\xe2\x57\xa5\x60\xc6\x4f\x95\x78\x1c\xe8"
        "\x19\x51\xa2\x8a\xef\xbc\x6f\xf5\xba\x79\x7f\x2d\xb3\x23\xd0\x80\x51"
        "\x03\xcc\x9e\xc9\x0f\x66\x97\xd2\x60\x61\x52\x14\xe7\x2b\x0d\x47\xc5"
        "\xec\xaa\xdf\x09\xd7\x7c\x07\x68\x2b\xe1\x12\x49\x8f\x01\x3c\x79\x20"
        "\x77\x68\x52\x76\x31\xf0\x3b\x49\x08\x3d\xd4\x9f\x9c\x6f\x46\xd1\xa1"
        "\x04\x17\xc3\x23\x7a\x49\x02\x85\x5f\x12\x5d\xdc\xe9\x4b\xff\x28\x3f"
        "\x06\x4c\x73\x92\xea\x87\x19\x79\xd9\x72\xc7\x9c\x6e\xed\xf6\xd2\x73"
        "\xc8\x59\xde\xd6\xcb\x16\x40\xcf\x2e\x04\x19\x95\xa1\x22\x94\x91\x41"
        "\x63\xcf\xcf\x74\x9f\x07\xe8\xea\x78\x80\x70\x42\x21\x02\x97\x87\xb5"
        "\xa6\x44\x63\x49\x55\x2d\x9d\x12\x98\x49\x92\x47\xe1\x10\x87\x9e\xcb"
        "\xb1\x8d\xd7\x56\xbe\x29\x02\x83\xe8\x83\xf9\x4f\x0c\x7a\x24\x63\x5f"
        "\x4d\xf3\xca\x18\xbf\xa7\x23\x83\xcf\xf5\x46\x14\xcc\xf7\xca\x36\xb7"
        "\x93\xc1\x15\xe6\x52\x17\x16\x2d\xa6\x08\xea\x88\x80\x7a\xaa\x6d\x32"
        "\x66\xed\xe1\xd5\x12\x57\x3f\xbb\x68\xd2\xaf\x37\xb0\x15\xa5\xd5\xd4"
        "\x7d\x9d\x89\x39\x4d\x63\x5a\xf8\x0e\xcc\x78\x1c\x63\xca\xbc\x65\x9d"
        "\x6d\x55\x13\x01\x0f\x66\x4f\x06\x5f\xd3\x8d\x0b\xc7\x42\xca\x67\xc1"
        "\xa0\x38\xe0\x4e\x16\x66\xd1\xe6\xfb\xb0\x8c\x42\xab\x60\xf9\x94\xd0"
        "\x6a\x37\xcc\xdb\x77\xb6\x3b\xf0\x12\x81\x7c\xdb\x2c\xe3\xcb\x9b\x8e"
        "\xdc\xa3\x9f\x6b\xce\xd8\xcb\xc5\x46\x09\xa9\x12\x8e\xa4\xac\x31\xf2"
        "\xd0\xef\x8e\x10\xf7\x6e\x4f\xf3\x76\x83\x34\x14\xa9\x8a\x1f\x5d\x30"
        "\xd3\x16\x3c\xb8\x1d\x2f\xdb\x2b\x08\x81\x90\xec\x28\xb6\x59\xef\x6e"
        "\x13\x48\x32\x3c\xd4\x64\x4f\x3d\x00\x45\xac\xa8\xbe\x52\x77\xe0\xd5"
        "\xed\x26\x20\x18\x49\x5e\xe1\xce\x26\xb9\xd9\xfd\x40\x01\x7d\xbb\x94"
        "\x97\x22\x77\x32\x59\x92\x58\x7e\x44\x33\xa9\x00\x1a\x41\x1b\xf7\xbe"
        "\xc9\x16\xf9\x7f\xce\xf3\x77\x27\xcc\x34\x84\xc8\xb4\x5a\x5b\x82\xef"
        "\x83\x22\xd3\x94\xf1\x98\xb4\x27\x06\xba\x86\xfa\x76\xdc\xb8\x20\x38"
        "\xa3\x43\x5e\x8e\x88\x59\xef\x05\xa4\x01\x55\x65\x8a\x8c\x95\x0b\x1e"
        "\x98\x54\xa5\xfc\x59\x99\x43\x48\xb7\x4d\x73\x48\xd3\xe1\xcb\x67\xe5"
        "\x71\x3c\x43\xfd\xb8\x35\x1a\xe5\xef\x8c\x6e\x37\x41\xd3\x20\xa8\xc4"
        "\x81\xf2\x71\x8d\xd6\x4f\x20\xfd\x9b\x7f\x86\x6b\xfb\x72\xa6\x3f\x13"
        "\x89\x0f\x65\x39\xd5\xac\x92\x2c\xa1\xec\xf8\x35\x4a\x11\x5f\x80\x58"
        "\x48\xb9\x08\x79\xe3\xb3\x69\x61\x76\xf8\x57\xc5\x94\xa4\x45\xd6\x98"
        "\x96\x0b\x55\x9a\x38\xf3\xa0\xc5\xf8\x10\xf5\x1f\xe4\x4b\xa4\x8c\x1c"
        "\x6a\x58\xa1\x92\xad\xd7\xe9\x1a\x20\xf3\x16\x26\xdf\xf9\x96\xa5\xe2"
        "\x44\xb6\xe0\x82\x77\x2c\x23\xdf\xaa\xf3\x73\xda\x1a\xc8\x11\x0e\x54"
        "\xbb\xb8\xd8\x6b\x14\x36\xf2\x79\x2e\x3f\xc4\xf0\xe8\x6f\x59\xeb\x54"
        "\x02\x78\xf8\x94\x5d\x7e\xf5\x2a\xc1\x81\x9d\x78\x06\x7a\x58\xe3\xde"
        "\x2b\x82\x3d\xed\xb6\x53\x91\x02\x61\xd1\xc5\xfd\xac\x8a\xf1\x41\xef"
        "\x3c\x84\xf9\x80\x7a\x9c\x73\xc9\xfa\x33\x00\x9e\xea\xeb\xe1\x87\x93"
        "\x11\x9e\x8a\xa2\x13\x6c\xdc\xe7\x7f\x80\xac\xc9\x78\x82\x93\x92\x28"
        "\xae\x04\x60\x02\xbd\xa3\x92\x14\x97\x12\x05\x26\xcc\xcc\x1e\x7e\xfd"
        "\x38\x85\xa8\x33\xa6\x4e\x75\x9d\x73\x06\xe3\xc6\x01\xc2\x50\x4b\x0a"
        "\xc0\xbd\xbd\x37\x01\x4f\xf6\x5e\xf2\x9b\xd4\xc9\x2e\xe4\x6a\x6c\xd6"
        "\x35\xd9\x23\xd3\x00\x73\x2f\xe5\x6a\x4c\xdf\xb4\xab\xa2\xb0\x40\x07"
        "\xed\x59\x26\xbc\x7e\x29\x77\x3c\xe8\x87\x13\x24\xdd\x85\x37\x6a\xfd"
        "\xf1\x1e\x05\x0a\xd7\x68\x97\x02\x0e\x22\x1e\x52\x1c\x89\x4a\xfd\xfe"
        "\x3d\x8b\x92\x4e\xbc\xcf\xfc\x9f\xa7\xc6\x50\x57\x83\x3d\xdb\x77\x3f"
        "\xff\x6c\x3c\xd8\x6f\x29\xc7\x35\x58\x40\x2e\x0e\x58\x56\xb9\x2b\x28"
        "\xaf\xac\x38\xfb\xa2\x23\x7c\x21\x40\xa3\x7e\x27\x86\xd2\x2c\x79\x9f"
        "\x74\xaa\x13\x09\x65\x5d\x0e\x99\x2f\xc5\xc9\xeb\xa4\x7c\x83\xd0\xf3"
        "\x46\x9e\xae\xee\x4f\xef\x49\x87\xaa\x66\x58\x10\x47\x6f\xdc\x12\x1e"
        "\xbb\x0b\xbf\x37\x3e\xa1\x42\xba\xbc\xf8\xa6\x86\x90\x21\x4f\xa8\x01"
        "\xbb\xd4\x5f\xf3\x44\xea\xc1\xa0\x8a\xba\x15\x6e\xe3\x3e\x46\x33\x5a"
        "\x95\x66\x73\x50\x6f\x32\x2f\xbe\xe6\x2d\x6c\x54\xd9\x50\x2a\x5c\x10"
        "\xc8\x07\x4a\x3f\x8a\x23\xaf\x01\x6d\xa7\x64\xf2\x6e\xda\x79\xf1\x7f"
        "\x01\x60\x91\x01\x26\x48\x81\x85\x72\x9f\xdd\xdd\x9c\x8e\xbf\x4c\xb0"
        "\x58\x89\x2f\x69\x84\x4d\x90\xbb\xca\xf0\x01\x74\x39\x15\xff\x5b\x52"
        "\x61\x87\xc3\xa8\x78\x52\xf7\x34\xf0\xea\xe6\xb8\xb4\x27\xe3\x68\x95"
        "\x69\xa1\x8b\xe3\xf0\xda\x4d\x36\x60\x70\x56\x68\x60\x51\x5b\x9d\xc6"
        "\x69\x8d\x57\x67\x07\xaa\x73\x49\x26\x86\x43\x9d\x7e\xc2\x9c\x30\x0f"
        "\xfa\x64\x81\xf3\xf0\x02\xf1\x2c\x96\x43\x0a\x02\xa7\xff\x59\xcb\x5b"
        "\x96\xa5\x46\x0b\xc4\x5e\x63\x0d\xde\xc7\x43\x56\xf6\x60\xe7\xd8\x85"
        "\x57\x2c\x91\x37\x4f\x12\x35\x86\x2b\x3d\xd1\x6f\xf0\xd7\xe3\xd8\xd0"
        "\xd8\x43\xb4\xf4\x26\xe2\x72\x9a\x17\x24\xa8\x14\x80\x95\x99\xb7\xe4"
        "\x5d\xca\x92\x28\x87\x11\x5a\xbd\xf4\x61\x4b\x42\xa9\x79\x43\xba\x50"
        "\xee\x95\xa8\xae\x89\xeb\xc8\x0a\x0c\xd6\x05\xdd\x74\xb3\xe1\xbc\xa6"
        "\x9f\xd8\xcb\x50\xdb\x47\x25\x93\x98\x09\xe7\xb1\x59\x01\x78\x56\xf6"
        "\xd1\x24\x6c\x35\x2f\x95\x67\xd6\x5e\x01\xce\x3e\x1c\x9b\x76\xea\xe1"
        "\xf4\x5b\x99\x57\x04\x3a\x09\xa5\xec\x2f\x46\x32\xd9\x3b\xdf\xd9\xe1"
        "\x32\x7f\x80\x93\x43\x85\x59\x0e\xb8\x7c\xd3\x4f\x98\x8f\x30\xac\xd4"
        "\x3c\xec\xe7\xf4\x32\xfd\xfe\xc9\xca\x7e\xb6\xb9\xae\x27\xb2\xe5\x05"
        "\xb5\xf4\xda\xc0\x40\xea\x38\x69\xf5\xcb\x8a\x31\xdf\x15\x86\x53\xa9"
        "\x2e\x4d\xb3\xf0\xea\x21\xc6\x96\x36\x22\xd3\xe0\x9f\xbc\x52\xec\x12"
        "\xa6\xf3\x8a\x25\x79\x3e\x50\xa3\xbe\xe1\x2c\xd6\xf5\x39\x21\x64\xe2"
        "\x31\xdd\x6a\x85\x17\xf1\x78\x38\x22\x41\xf8\xe9\x7a\xff\xdc\xfe\xb1"
        "\x32\x46\x75\xd6\xe8\xb4\x74\xec\xcb\xba\x18\x6c\x4b\x83\x22\x9e\xee"
        "\xb2\x8d\x78\x19\x06\xf6\x28\xbd\xf9\xb5\xc2\x07\x3a\x2d\xa7\x8c\x72"
        "\xaa\x0b\x43\xb9\x11\xa8\x25\x64\xd0\x2e\x13\x4d\x16\xb3\x1d\x1e\xd9"
        "\xd5\x4e\x8a\x83\xd2\x24\xdc\x64\x6a\x96\xa9\xa7\xa4\xc9\xfb\xb6\xd8"
        "\x44\x08\x28\x1f\x40\x05\x97\x1e\x3d\xc7\x64\xb0\x01\xda\xbc\x56\x35"
        "\x0d\xd5\x06\x7b\xaf\x9a\x35\x37\x42\x2c\xcb\x49\x34\xc7\x7e\x4e\x92"
        "\xb8\x86\x97\xb6\xd1\xa7\x3e\xe5\x93\x56\xd3\x03\x1a\xff\x46\x2f\x79"
        "\x76\x9b\x77\x73\x16\x2b\x9f\xa8\xd0\x73\x81\x26\xb0\xc5\x3d\x51\xc5"
        "\x1f\x99\x5a\xc5\x76\x5d\xa4\xc0\x2d\x18\x2d\xf7\xee\xc9\xd4\x72\x8d"
        "\x04\x70\x7d\xd9\xfb\x83\x61\x8f\x77\x19\xa9\xe5\xa7\x83\x4f\xe9\x3e"
        "\x3e\x40\x19\xb0\x59\x0d\x70\xfe\x8b\x3b\x4d\xc6\x95\x36\x3e\x1f\xfb"
        "\xc4\x85\x73\x91\x5b\x8b\x0e\x46\x39\xa7\x91\xd4\x74\xba\xe6\xea\x24"
        "\xa6\xf8\xae\x96\xb9\xe7\x71\xc8\x8a\x45\x74\x8b\xed\xb3\x75\x7a\x98"
        "\x93\xef\xee\x3e\xab\x00\x90\xa9\xd4\x7c\x56\x2e\x76\x06\x2f\xa3\x5b"
        "\x44\xd5\xb2\x71\x53\xe6\x92\x29\xf3\xe8\x3e\x3c\x9e\xfd\x83\x60\xee"
        "\x06\xe0\xde\xd8\x2c\x7c\xe4\xe7\x13\x9c\x32\x77\xeb\xda\x90\xdd\x24"
        "\xf9\xb3\xcc\xe7\x76\xc5\x91\x11\x7b\x8b\x06\xc8\xc4\xd5\xb2\x8c\x23"
        "\x3b\x42\x48\x42\x09\x08\x6b\x61\xd2\xf8\x10\x14\x69\xc2\xf1\x50\x04"
        "\xc5\xa1\x3f\x34\xa4\xf8\x61\xd9\x8b\xb4\x97\xa8\xb7\xb8\x3e\x01\x0f"
        "\xb4\x23\x85\xea\xf4\x28\x8d\x3c\xb5\xc6\x33\x6f\x11\x31\x20\x63\xbe"
        "\x83\xb7\x51\x4e\xa5\x83\xa2\x16\xe0\xd9\xf8\x74\xaa\x65\x5c\xaf\x46"
        "\x38\xfe\x02\x9d\xbf\xb9\x9c\xdd\x37\x9c\xf7\x26\xdf\xda\x0e\x8a\x13"
        "\xf0\x1b\x94\x76\x7e\x15\xfe\x34\x45\x7a\x2f\x59\x5b\x46\x90\x7d\x7f"
        "\x8c\xac\x22\xa3\xb0\xe7\x9a\x8f\x80\xb4\x4c\x52\xff\xf3\x79\x31\x77"
        "\xa8\x92\x34\x71\xe9\x1f\x72\x24\x55\xc6\xd0\x11\x55\xb9\x5c\xfc\xf9"
        "\x9e\x5f\x2e\x72\x3a\x46\xc6\x0a\x09\xbb\xd4\xf9\xd4\x3c\x2b\xfe\x72"
        "\x2a\x06\x4a\x25\x87\x5e\xea\xe4\xb2\x6d\xf7\xf3\x66\x6d\x02\x8e\xfa"
        "\x43\xe9\x19\x0b\x9b\x42\x3c\x67\x47\xb2\xfe\x87\xa5\x98\x26\x1c\x19"
        "\xb9\x0e\xce\x0d\x98\x88\x25\x8d\x64\x01\x2f\x55\x80\x49\x4d\xcc\x7f"
        "\xd2\x9c\x95\xdb\x6e\xf1\x32\x69\xe5\xb4\x56\x37\x60\xf6\x87\xc3\xe6"
        "\xe0\x9e\xe4\x6a\xee\x26\x35\x2a\xb9\x24\x95\x12\x79\x8a\xe4\x78\x93"
        "\xec\x04\xaa\xa9\x57\xe2\x35\x92\x0e\xc5\x81\x37\x2d\xdf\xb4\x51\x1d"
        "\x0e\x31\x1d\xc1\xb3\x00\xf7\x70\x26\x01\xde\xaf\xf7\x12\x89\xa3\x76"
        "\x44\x02\xd9\x7e\x65\xdd\xe8\x0b\x1d\x25\xe4\x22\xba\x5d\x4d\x16\xca"
        "\x43\x15\x45\xfe\x16\xd0\xce\x56\x12\xad\x18\x7a\xda\x94\x22\xc4\x18"
        "\x01\x2c\x5c\xa7\x2b\xed\x12\x07\x3c\x91\xc4\xe4\xa2\x57\xa7\xf7\x4b"
        "\x72\xf7\xa4\x1d\x08\xbd\xf0\xed\x99\x03\x78\xe6\x27\x37\xde\x75\xb5"
        "\x3d\x60\xbc\xfb\x52\x81\xc1\xd7\x0a\xdf\x28\x69\xd0\xeb\x1d\xc7\x9c"
        "\xbb\x16\x7f\x64\xa5\x91\x63\x14\xe7\x62\x99\x08\x2f\xa9\xd1\x14\x50"
        "\xa4\x68\x10\x8d\x39\x5a\x8f\x3e\x59\x7c\x6b\x24\x96\x07\x1c\x98\x72"
        "\x7c\x3f\xb0\x71\x39\x4e\xcf\x73\xfe\x11\x1c\x06\x1e\x2a\xc0\xd7\xab"
        "\x94\xd4\x96\x71\x9e\x05\x8f\xfa\xd2\x87\x09\x66\xb2\x5b\xd3\xe1\xd0"
        "\x19\x68\xeb\xde\x7c\x5b\x62\x04\xe3\xf8\x9d\x71\x69\x02\xe5\x75\xa5"
        "\x26\xe3\x92\xea\x7f\x1c\x69\xed\x2d\x84\x03\xc3\xa7\x3e\xf2\xeb\x92"
        "\x77\x5d\xce\xfc\x26\xc5\x7f\x14\x57\x38\x73\xbc\x5a\xf1\xc6\x26\xc7"
        "\x03\x34\x2c\x0c\xf4\x17\xd7\xd9\xec\x34\x9d\x94\x76\xa1\x0f\x37\x1c"
        "\xf4\x50\x9c\x6f\xa5\x3b\xec\xcb\x61\xf8\x43\x91\xd6\xff\xfe\x76\xd4"
        "\x1f\xfa\x4a\xe3\x51\xe8\xb7\xa9\x4d\xf5\xaf\xa5\xde\x25\x8e\x4b\x0a"
        "\x04\x7f\xfa\xa7\x07\x28\x8c\xe4\x24\x9c\x7c\x76\x43\xe7\x01\xc7\x4f"
        "\x76\xd5\xac\xae\xfb\xbc\x0c\x5f\x11\x4d\xc8\x40\x67\x90\x10\xbe\x69"
        "\xb1\xd5\x3e\x39\xe7\x55\x0d\x6d\x53\x96\x0e\x07\x80\xbb\x73\xfb\xf1"
        "\x94\xe5\x13\x6f\x43\x5e\x65\xfd\xe2\xcd\x6e\xe8\xde\x56\x23\x8f\x6d"
        "\x11\x03\x08\x1f\xa8\x0e\x86\xb4\xc2\x4c\x8f\x9b\x22\x6a\x80\x21\x7f"
        "\xbd\xa9\xc2\x8f\xf3\x62\x96\x2b\x99\xfb\x2f\x2d\x2e\x72\x2b\x52\x6c"
        "\x94\x1c\xf0\x0b\x18\x3c\x3a\x1a\xcd\x26\x63\xa9\xbc\x52\x2f\xbc\x09"
        "\x79\x31\xa5\xc5\x53\x6d\xca\xa1\x70\x7a\xe0\x29\xa8\x4e\x59\x6a\x3e"
        "\xf5\x1f\x55\x90\x4a\x31\xc2\x79\xd9\x5b\xc1\x63\x3e\x1d\x2d\x5d\x97"
        "\x65\x30\xb1\xcd\x0d\x95\x2c\x6f\xd5\xe0\x11\x1c\x57\xcd\xef\x49\x60"
        "\xe5\x17\x9c\x31\x59\xab\xb4\xbe\x4b\x74\x76\x1d\xfe\x15\x24\x89\x26"
        "\x8d\xc1\x73\x13\x9e\xcb\x6e\x68\xe1\xa6\xb6\x7a\x6c\x39\x72\x63\xd5"
        "\xb2\xe9\x82\x23\xc2\xf6\xd6\xd4\xcd\xb3\x35\x6c\x5b\xb9\x1f\x38\x60"
        "\x0e\xc9\x3f\x35\xc8\xc9\x11\xac\x16\x17\x32\x0f\xbe\x7c\xec\x35",
        4096);
    *(uint16_t*)0x200026c4 = 0x1000;
    syscall(__NR_write, r[0], 0x200016c0ul, 0x1006ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0ul);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      use_temporary_dir();
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
