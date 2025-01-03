// https://syzkaller.appspot.com/bug?id=4d9a3c81b08691de9279ad898de514723ee70d22
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
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

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
  for (call = 0; call < 1; call++) {
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

void execute_call(int call)
{
  switch (call) {
  case 0:
    *(uint32_t*)0x20000300 = 2;
    *(uint32_t*)0x20000304 = 0x80;
    *(uint8_t*)0x20000308 = 0xd7;
    *(uint8_t*)0x20000309 = 0;
    *(uint8_t*)0x2000030a = 0;
    *(uint8_t*)0x2000030b = 0;
    *(uint32_t*)0x2000030c = 0;
    *(uint64_t*)0x20000310 = 8;
    *(uint64_t*)0x20000318 = 0;
    *(uint64_t*)0x20000320 = 0;
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 0, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 1, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 2, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 3, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 4, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 5, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 6, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 7, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 8, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 9, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 1, 10, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 11, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 12, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 13, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 14, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 15, 2);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 17, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 18, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 19, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 20, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 21, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 22, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 23, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 24, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 25, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 26, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 27, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 28, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 29, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 30, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 31, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 32, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 33, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 34, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 35, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 1, 36, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 1, 37, 1);
    STORE_BY_BITMASK(uint64_t, , 0x20000328, 0, 38, 26);
    *(uint32_t*)0x20000330 = 0;
    *(uint32_t*)0x20000334 = 0;
    *(uint64_t*)0x20000338 = 0;
    *(uint64_t*)0x20000340 = 0;
    *(uint64_t*)0x20000348 = 0;
    *(uint64_t*)0x20000350 = 0;
    *(uint32_t*)0x20000358 = 0;
    *(uint32_t*)0x2000035c = 0;
    *(uint64_t*)0x20000360 = 0;
    *(uint32_t*)0x20000368 = 0;
    *(uint16_t*)0x2000036c = 0;
    *(uint16_t*)0x2000036e = 0;
    *(uint32_t*)0x20000370 = 0;
    *(uint32_t*)0x20000374 = 0;
    *(uint64_t*)0x20000378 = 0;
    syscall(__NR_perf_event_open, 0x20000300ul, 0, 0ul, -1, 0ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  loop();
  return 0;
}
