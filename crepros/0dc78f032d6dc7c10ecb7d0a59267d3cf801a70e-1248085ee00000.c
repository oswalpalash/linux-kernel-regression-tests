// https://syzkaller.appspot.com/bug?id=0dc78f032d6dc7c10ecb7d0a59267d3cf801a70e
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
      if (current_time_ms() - start < 5 * 1000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

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
    *(uint64_t*)0x20000300 = 0;
    *(uint32_t*)0x20000308 = 0;
    *(uint64_t*)0x20000310 = 0x200002c0;
    *(uint64_t*)0x200002c0 = 0x20000400;
    memcpy((void*)0x20000400,
           "\x54\x00\x00\x00\x30\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x40\x00\x01\x00\x3c\x00\x01\x00\x08\x00\x01\x00"
           "\x69\x66\x65\x00\x2c\x00\x02\x80\x1c\x00\x01\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x0c\x00\x06\x00\x04\x00\x01\x00\x04\x00\x04\x00"
           "\x04\x00\x06\x00\x18\x68\x2c\x99\x4a\xa0\x29\x37\xd9\x71\x9b\x26"
           "\x10\x9f\x55\x75\x5d\x96\x4e\xd1\x53\x86\xea\xc6\xac\x81\xda\x3a"
           "\x7b\x07\x5c\x62\x82\x4d\x14\xd2\xbf\x1d\x4e\x85\x7d\xeb\x41\x68"
           "\xe8\x76\x55\x2c\x9b\xdd\x96\xd0\x81\xbd\xc3\x7c\x65\xf9\x52\xa7"
           "\xf1\x7a\xac\xc3\x9b\x7c\xa7\x78\x0b\xb1\x2b\xcb\x0c\xc6\x13\xfd"
           "\x42\x1c\xcd\x67\x76\xdd\x44\x1d\x4c\x72\x74\x2b\x8c\x52\x7b\xf7"
           "\x30\xc9\x86\x0a\x41\x3a\xf5\x99\x26\x5e\x9e\x7d\x18\x65\x99\x25"
           "\x1d\xc5\x48\x1a\xac\x8c\x89\x3b\x19\xd7\xbc\xb6\xce\x57\x94\xac"
           "\x26\xef\xe5\xfe\xee\xba\x2d\x95\xd9\x00\x82\xd1\x8d\xd3\x29\x50"
           "\x8b\xeb\x59\xa7\xed\xc0\x7e\x47\xec\x8c\x83\xa6\xd6\x38\xf8\x8e",
           240);
    *(uint64_t*)0x200002c8 = 0x54;
    *(uint64_t*)0x20000318 = 1;
    *(uint64_t*)0x20000320 = 0;
    *(uint64_t*)0x20000328 = 0;
    *(uint32_t*)0x20000330 = 0;
    syscall(__NR_sendmsg, r[1], 0x20000300ul, 0ul);
    break;
  case 3:
    *(uint64_t*)0x20000300 = 0;
    *(uint32_t*)0x20000308 = 0;
    *(uint64_t*)0x20000310 = 0x200002c0;
    *(uint64_t*)0x200002c0 = 0x20000000;
    *(uint32_t*)0x20000000 = 0x24;
    *(uint16_t*)0x20000004 = 0x31;
    *(uint16_t*)0x20000006 = 0x101;
    *(uint32_t*)0x20000008 = 0;
    *(uint32_t*)0x2000000c = 0;
    *(uint8_t*)0x20000010 = 0;
    *(uint8_t*)0x20000011 = 0;
    *(uint16_t*)0x20000012 = 0;
    *(uint16_t*)0x20000014 = 0x10;
    *(uint16_t*)0x20000016 = 1;
    *(uint16_t*)0x20000018 = 0xc;
    STORE_BY_BITMASK(uint16_t, , 0x2000001a, 1, 0, 14);
    STORE_BY_BITMASK(uint16_t, , 0x2000001b, 0, 6, 1);
    STORE_BY_BITMASK(uint16_t, , 0x2000001b, 0, 7, 1);
    *(uint16_t*)0x2000001c = 8;
    *(uint16_t*)0x2000001e = 1;
    memcpy((void*)0x20000020, "ife\000", 4);
    *(uint64_t*)0x200002c8 = 0x24;
    *(uint64_t*)0x20000318 = 1;
    *(uint64_t*)0x20000320 = 0;
    *(uint64_t*)0x20000328 = 0;
    *(uint32_t*)0x20000330 = 0;
    syscall(__NR_sendmsg, r[0], 0x20000300ul, 0ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
