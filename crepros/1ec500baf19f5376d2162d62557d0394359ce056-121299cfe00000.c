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
        "\x00\x00\x00\x00\x00\x2b\x40\xde\xff\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x73\x8d\x7a\x31"
        "\x00\x00\x00\x00\x00\xff\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x73\x79\x7a\x31\x00\x00\x00\x00"
        "\x00\x00\x00\x20\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xcf"
        "\x0a\x00\x00\x00\x00\x00\x00\x00\x00\x0c\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x16\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x3b\x38\xe9\x67\xac\x82\x06\xea\xae"
        "\x86\xb9\x7e\xec\x0b\x2b\xed\x1e\xe2\x33\x64\xb1\x0d\x6a\xad\x51\x02"
        "\x00\x00\x00\xe2\xa5\xdb\x3c\x6a\x0d\xee\x4a\xfc\x66\xd2\x44\xfb\x3d"
        "\xa2\x68\xd5\x47\xc2\x6e\xd1\x8e\x6f\x24\x03\x01\xb6\x92\x28\x05\x20"
        "\x1c\x39\x38\x9a\x80\x4c\x41\xc2\x99\x3f\xc6\x7e\x8a\x14\x60\x45\xe1"
        "\x4a\x8a\x08\x00\x7a\xbe\x87\x0c\x06\x51\xb1\x55\x0e\x6a\x25\xc0\xef"
        "\x65\xf6\x00\x00\x00\x00\x00\x00\x00\x03\x18\x7f\xaf\xa4\xa1\xee\x6e"
        "\xce\x53\xc6\x73\x85\xb8\x83\xa3\x6a\xd2\x4a\x04\x00\x00\x00\x00\x00"
        "\x00\x00\x6a\x8a\xb1\x1b\x0a\x0b\x00\xe7\x7e\x6c\x16\x18\x9c\xfa\x16"
        "\xcb\xe0\x1a\x4c\xe4\x11\x37\x8e\xaa\xb7\x37\x2d\xab\x5e\xef\x84\xc3"
        "\x1b\x2d\xad\x86\x8a\x53\xe6\xf5\xe6\x97\x46\xa7\x86\xe5\xc8\x1e\xae"
        "\x45\xe3\xa2\x5b\x94\x2b\x8d\xa1\x1e\xdb\x57\x8b\x45\x3a\xca\xc0\x3a"
        "\x9d\x34\x48\x00\x00\x00\x00\x83\xd6\xd5\xfe\x4f\x83\x3d\x4d\x4c\xfb"
        "\xee\xf0\xe0\xe6\x2b\x7b\x09\x4b\x2b\x9c\x33\xd8\xa6\x24\xce\x00\x00"
        "\x00\x1f\xf2\x8d\x3c\xe3\xe3\xb8\xf8\x1e\x34\xcf\x97\xc9\xc8\x41\xcb"
        "\x2e\xf0\x81\x07\xa9\xa9\x65\x49\xe3\xd2\x59\xdf\x17\xe2\x9e\xd6\x4b"
        "\xd6\x12\x08\x13\xf9\xf0\x34\x4e\x13\x95\x06\x70\x1e\x8f\xde\xdb\x06"
        "\x00\x9b\x5e\x4d\x0c\x67\xbd\xa0\xb9\x28\xb7\x32\xcf\xf7\x82\xb0\x68"
        "\x40\x75\xf2\xcb\x78\x51\xef\xdd\x77\x97\xee\x95\xd2\xac\x28\xa8\xca"
        "\xbd\x26\xc1\x56\x82\xaa\x78\xd3\x1a\xec\x95\x6b\xd7\xc2\x78\x06\x40"
        "\x34\x34\xb3\xc3\x0b\x07\x0b\xcc\x82\x66\xe1\x2f\xa6\x66\x02\x05\x62"
        "\x56\xf7\x46\x75\xb7\xcb\x4d\x87\xb2\x70\x8d\x70\xc8\xf3\xdf\x53\xca"
        "\xf8\xfe\x18\x0c\x4d\xea\x3f\x5b\x7a\x92\xa3\x4b\x0e\xfa\xab\x02\xdc"
        "\xa0\x51\x7e\xee\x50\xff\x30\x20\x6f\x78\xec\x82\xc7\x2f\x33\xe9\x09"
        "\xc1\xae\x1f\xe9\x4f\xe0\x75\x9f\x77\x25\xb5\x66\x4f\xbe\xe8\x07\xd1"
        "\x64\x02\x6f\x3e\x33\x35\xf4\x20\x00\x88\x8c\x90\x5f\xb4\x1d\xbd\xb4"
        "\x20\xea\x9f\x87\xfc\x86\x51\x26\x85\xeb\x72\x61\xe9\xc2\x5e\xdb\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
        743);
    syscall(__NR_write, r[0], 0x20000b40ul, 0x12eul);
    break;
  case 2:
    *(uint16_t*)0x20000080 = 0x11;
    memcpy((void*)0x20000082,
           "\x00\x00\x01\x00\x00\x00\x00\x00\x08\xfc\x9d\x71\xfc\x00\x00\x00"
           "\x00\x00\x00\x00\xf8\xff\xff\x00\x2e\x0b\x38\x36\x00\x54\x04\xb0"
           "\xd6\x30\x1a\x4c\xe8\x75\xf2\xe3\xff\x5f\x16\x3e\xe3\x40\xb7\x67"
           "\x95\x00\x80\x00\xf8\x00\x00\x00\x00\x01\x04\x00\x3c\x58\x11\x03"
           "\x9e\x15\x77\x50\x27\xec\xce\x66\xfd\x79\x2b\xbf\x0e\x5b\xf5\xff"
           "\x9b\x08\x16\xf3\xf6\xdb\x1c\x00\x01\x00\x00\x00\x00\x00\x00\x00"
           "\x49\x74\x00\x00\x00\x00\x00\x00\x00\x06\xad\x8e\x5e\xcc\x32\x6d"
           "\x3a\x09\xff\x42\xc6\x54\x00\x00\x00\x00\x00\x00\x00\x00",
           126);
    syscall(__NR_bind, -1, 0x20000080ul, 0x80ul);
    break;
  case 3:
    *(uint32_t*)0x200000c0 = 0xc;
    syscall(__NR_write, r[0], 0x200000c0ul, 0x235ul);
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
