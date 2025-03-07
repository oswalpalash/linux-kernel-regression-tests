// https://syzkaller.appspot.com/bug?id=b96d0a0eaa2196cfa0667e1359772777ee06c80a
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
  for (call = 0; call < 6; call++) {
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
    *(uint64_t*)0x200000c0 = 0x20000100;
    *(uint16_t*)0x20000100 = 0xa;
    *(uint16_t*)0x20000102 = 8;
    *(uint32_t*)0x20000104 = 0;
    *(uint32_t*)0x20000108 = 0;
    *(uint32_t*)0x200000c8 = 0x293;
    *(uint64_t*)0x200000d0 = 0x20000040;
    *(uint64_t*)0x20000040 = 0;
    *(uint64_t*)0x20000048 = 0;
    *(uint64_t*)0x200000d8 = 1;
    *(uint64_t*)0x200000e0 = 0;
    *(uint64_t*)0x200000e8 = 0;
    *(uint32_t*)0x200000f0 = 0;
    syscall(__NR_sendmsg, -1, 0x200000c0ul, 0ul);
    break;
  case 1:
    *(uint64_t*)0x20000280 = 0;
    *(uint32_t*)0x20000288 = 0;
    *(uint64_t*)0x20000290 = 0x20000240;
    *(uint64_t*)0x20000240 = 0x20000100;
    memcpy((void*)0x20000100,
           "\x14\x00\x00\x00\x10\x00\x00\x3f\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x10\x00\x0a\x20\x00\x00\x00\x00\x0a\x01\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x09\x00\x01\x00\x73\x79\x7a\x30"
           "\x00\x00\x00\x00\x38\x00\x00\x00\x12\x0a\x01\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x04\x00\x04\x80\x09\x00\x02\x00"
           "\x02\x67\x00\x00\x00\x00\x00\x00\x09\x00\x01\x00\x73\x79\x7a\x30"
           "\x00\x00\x00\x00\x08\x00\x03",
           103);
    *(uint64_t*)0x20000248 = 1;
    *(uint64_t*)0x20000298 = 1;
    *(uint64_t*)0x200002a0 = 0;
    *(uint64_t*)0x200002a8 = 0;
    *(uint32_t*)0x200002b0 = 0;
    syscall(__NR_sendmsg, -1, 0x20000280ul, 0ul);
    break;
  case 2:
    res = syscall(__NR_socket, 0x10ul, 3ul, 0xcul);
    if (res != -1)
      r[0] = res;
    break;
  case 3:
    *(uint64_t*)0x200001c0 = 0;
    *(uint32_t*)0x200001c8 = 0x241;
    *(uint64_t*)0x200001d0 = 0x20000080;
    *(uint64_t*)0x20000080 = 0;
    *(uint64_t*)0x20000088 = 0;
    *(uint64_t*)0x200001d8 = 8;
    *(uint64_t*)0x200001e0 = 0x6cedff7f;
    *(uint64_t*)0x200001e8 = 0;
    *(uint32_t*)0x200001f0 = 0;
    syscall(__NR_sendmsg, r[0], 0x200001c0ul, 0ul);
    break;
  case 4:
    res = syscall(__NR_socket, 0x10ul, 3ul, 0xcul);
    if (res != -1)
      r[1] = res;
    break;
  case 5:
    *(uint64_t*)0x200006c0 = 0;
    *(uint32_t*)0x200006c8 = 0;
    *(uint64_t*)0x200006d0 = 0x20000680;
    *(uint64_t*)0x20000680 = 0x20002a00;
    *(uint32_t*)0x20002a00 = 0x14;
    *(uint16_t*)0x20002a04 = 0x10;
    *(uint16_t*)0x20002a06 = 1;
    *(uint32_t*)0x20002a08 = 0;
    *(uint32_t*)0x20002a0c = 0;
    *(uint8_t*)0x20002a10 = 0;
    *(uint8_t*)0x20002a11 = 0;
    *(uint16_t*)0x20002a12 = htobe16(0xa);
    *(uint32_t*)0x20002a14 = 0x68;
    *(uint8_t*)0x20002a18 = 2;
    *(uint8_t*)0x20002a19 = 0xa;
    *(uint16_t*)0x20002a1a = 0x401;
    *(uint32_t*)0x20002a1c = 0;
    *(uint32_t*)0x20002a20 = 0;
    *(uint8_t*)0x20002a24 = 0;
    *(uint8_t*)0x20002a25 = 0;
    *(uint16_t*)0x20002a26 = htobe16(6);
    *(uint16_t*)0x20002a28 = 9;
    *(uint16_t*)0x20002a2a = 1;
    memcpy((void*)0x20002a2c, "syz1\000", 5);
    *(uint16_t*)0x20002a34 = 9;
    *(uint16_t*)0x20002a36 = 1;
    memcpy((void*)0x20002a38, "syz0\000", 5);
    *(uint16_t*)0x20002a40 = 9;
    *(uint16_t*)0x20002a42 = 1;
    memcpy((void*)0x20002a44, "syz0\000", 5);
    *(uint16_t*)0x20002a4c = 9;
    *(uint16_t*)0x20002a4e = 1;
    memcpy((void*)0x20002a50, "syz0\000", 5);
    *(uint16_t*)0x20002a58 = 0xc;
    STORE_BY_BITMASK(uint16_t, , 0x20002a5a, 4, 0, 14);
    STORE_BY_BITMASK(uint16_t, , 0x20002a5b, 1, 6, 1);
    STORE_BY_BITMASK(uint16_t, , 0x20002a5b, 0, 7, 1);
    *(uint64_t*)0x20002a5c = htobe64(3);
    *(uint16_t*)0x20002a64 = 8;
    STORE_BY_BITMASK(uint16_t, , 0x20002a66, 2, 0, 14);
    STORE_BY_BITMASK(uint16_t, , 0x20002a67, 1, 6, 1);
    STORE_BY_BITMASK(uint16_t, , 0x20002a67, 0, 7, 1);
    *(uint32_t*)0x20002a68 = htobe32(2);
    *(uint16_t*)0x20002a6c = 8;
    STORE_BY_BITMASK(uint16_t, , 0x20002a6e, 2, 0, 14);
    STORE_BY_BITMASK(uint16_t, , 0x20002a6f, 1, 6, 1);
    STORE_BY_BITMASK(uint16_t, , 0x20002a6f, 0, 7, 1);
    *(uint32_t*)0x20002a70 = htobe32(1);
    *(uint16_t*)0x20002a74 = 8;
    STORE_BY_BITMASK(uint16_t, , 0x20002a76, 2, 0, 14);
    STORE_BY_BITMASK(uint16_t, , 0x20002a77, 1, 6, 1);
    STORE_BY_BITMASK(uint16_t, , 0x20002a77, 0, 7, 1);
    *(uint32_t*)0x20002a78 = htobe32(0);
    *(uint32_t*)0x20002a7c = 0x14;
    *(uint8_t*)0x20002a80 = 2;
    *(uint8_t*)0x20002a81 = 0xa;
    *(uint16_t*)0x20002a82 = 0x401;
    *(uint32_t*)0x20002a84 = 0;
    *(uint32_t*)0x20002a88 = 0;
    *(uint8_t*)0x20002a8c = 0;
    *(uint8_t*)0x20002a8d = 0;
    *(uint16_t*)0x20002a8e = htobe16(0);
    *(uint32_t*)0x20002a90 = 0x14;
    *(uint16_t*)0x20002a94 = 0x11;
    *(uint16_t*)0x20002a96 = 1;
    *(uint32_t*)0x20002a98 = 0;
    *(uint32_t*)0x20002a9c = 0;
    *(uint8_t*)0x20002aa0 = 0;
    *(uint8_t*)0x20002aa1 = 0;
    *(uint16_t*)0x20002aa2 = htobe16(0xa);
    *(uint64_t*)0x20000688 = 0xa4;
    *(uint64_t*)0x200006d8 = 1;
    *(uint64_t*)0x200006e0 = 0;
    *(uint64_t*)0x200006e8 = 0;
    *(uint32_t*)0x200006f0 = 0;
    syscall(__NR_sendmsg, r[1], 0x200006c0ul, 0x24000800ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      use_temporary_dir();
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
