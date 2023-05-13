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

#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_openat
#define __NR_openat 295
#endif
#ifndef __NR_write
#define __NR_write 4
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[1] = {0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res;
  switch (call) {
  case 0:
    memcpy((void*)0x20000080, "/dev/uhid\000", 10);
    res = syscall(__NR_openat, 0xffffff9c, 0x20000080, 2, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    memcpy(
        (void*)0x20000900,
        "\x0b\x00\x00\x00\x73\x79\x7a\x31\x19\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x20\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x41\x00\x73\x8d\x7a\x31"
        "\x00\x00\x00\x00\x00\xff\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x73\x79\x7a\x31\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xcf"
        "\x0a\x00\x00\x00\x00\x00\x00\x00\x00\x0c\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x16\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\xfd\xff\x00\x00\x00\x00\x00\x3b\x38\xe9\x67\xac\x82\x06\xea\xae"
        "\x86\xb9\x7e\xec\x0b\x2b\xed\x1e\xe2\x33\x64\xb1\x0d\x6a\xad\x51\x02"
        "\x00\x00\x00\xe2\xa1\xdb\x3c\x6a\x0d\xee\x4a\xfc\x66\xd2\x44\x28\x05"
        "\x20\x1c\x39\x38\x9a\x80\x4c\x41\xc2\x99\x3f\xc6\x7e\x8a\x14\x60\x45"
        "\xe1\x4a\x8a\x08\x00\x55\x0e\x6a\x25\xc0\xef\x65\xf6\xec\x71\xf0\x08"
        "\x42\x54\xd1\x40\x18\x7f\xaf\xa4\xa1\xee\x6e\xce\x53\xc6\x73\x85\xb8"
        "\x83\xa3\x6a\xd2\x4a\x04\x00\x00\x00\x00\x00\x00\x00\x6a\x8a\xb1\x1b"
        "\x0a\x0b\x00\xe7\x7e\x6c\x16\x18\x9c\xfa\x16\xcb\xe0\x1a\x4c\xe4\x11"
        "\x37\x8e\xaa\xb7\x37\x2d\xab\x5e\xef\x84\xc3\x1b\x2d\xad\x86\x8a\x53"
        "\xe6\xf5\xe6\x97\xab\x39\x42\x86\xe5\xc8\x1e\xae\x45\xe3\xa2\x5b\x94"
        "\x2b\x8d\xa1\x1e\xdb\x57\x8b\x45\x3a\xca\xc0\x3a\x9d\x34\x48\x00\x00"
        "\x00\x00\x83\xd6\xd5\xfe\x4f\x83\x3d\x4d\x4c\xfb\xee\xf0\xe0\xe6\x2b"
        "\xe2\x05\x00\x00\x00\x3c\x32\x98\x4c\x24\xce\xa9\x5c\x3b\x3c\x6d\xd8"
        "\x73\x56\x9c\xf4\x78\x6f\xc5\x16\x6b\x03\x00\x00\x00\x00\x00\x1f\xf2"
        "\x8d\x3c\xe3\xe3\xb8\xf8\x1e\x01\xb8\x0b\x84\x09\x40\x0f\x34\xcf\x97"
        "\xc9\xc8\x41\xcb\x2e\xf0\x81\x07\xa9\xa9\x65\x49\xe3\xd2\x59\xdf\x17"
        "\xe2\x9e\xd6\x4b\xd6\x12\x08\x13\xf9\xf0\x34\x4e\x13\x95\x06\x70\x1e"
        "\x8f\xde\xdb\x06\x00\x9b\x5e\x4d\x0c\x67\xbd\xa0\xb9\x28\xb7\x32\xcf"
        "\xf7\x82\xb0\x68\x40\x75\xf2\xcb\x78\x51\xef\xdd\x77\x97\xee\x95\xd2"
        "\xac\x28\xa8\xca\xbd\x26\xc1\x56\x82\xaa\x58\xd3\x1a\xec\x95\x6b\xd7"
        "\xc2\x78\x06\x40\x34\x34\xb3\xc3\x0b\x07\x0b\xcc\x82\x66\xe1\x2f\xa6"
        "\x66\x02\x05\x62\x56\xf7\x46\x75\xb7\xcb\x4d\x87\xb2\x70\x8d\x70\xc8"
        "\xf3\xdf\x53\xca\xf8\xfe\x18\x0c\x4d\xea\x3f\x5b\x7a\x87\x1b\x30\xc7"
        "\xa5\x75\x3b\x48\xf7\xf0\x91\x92\xa3\x4b\x0e\xfa\xab\x02\xdc\xa0\x51"
        "\x7e\xee\x10\xff\x30\x20\x6f\x78\xec\x82\xc7\x2f\xd9\xac\x91\x64\x34"
        "\x12\xb1\xd9\x40\x9e\xcb\x85\xd7\x60\x53\x5e\xb2\x50\x8e\xb9\x85\xfc"
        "\x09\xb3\x92\x08\x5e\x1d\x9a\xaf\x0a\x9c\x86\x5b\x59\xfd\x39\x64\x44"
        "\x39\x73\x2f\x85\x81\x28\xc4\x2d\x19\x94\xbc\x0d\x84\xf4\xef\x8d\xff"
        "\x93\xfa\x5e\x68\x87\xb3\x44\x1e\x32\x6d\xad\x41\x6a\x8a\x56\xdc\x21"
        "\x15\x70\x95\x71\xf8\x84\xfb\xc6\x9a\xa8\x66\x8a\x2f\xec\xae\xe2\x06"
        "\x4c\x81\xfa\x6e\x86\xef\x51\x26\xa6\x74\xab\x8c\xda\xf7\x2a\x8a\xbf"
        "\xaa\x12\xad\x38\x7c\x07\x9f\xb2\x87\x20\x06\xe1\x05\xf7\xc2\x48\xcf"
        "\xcc\x46\x39\x64\xd7\x48\x51\xbc\x25\x48\xae\x53\x33\x3b\x2d\xf9\x9d"
        "\x41\x05\xa5\x79\xf9\x1a\x91\xe2\xc4\x4b\x92\x7f\xa0\x5c\x1c\xd2\x9f"
        "\x3e\x05\xf5\x61\xec\xd4\x9b\xff\x3d\x05\xa6\xd5\x05\x2a\x0b\xe4\x71"
        "\x2e\xb8\x7c\x81\xd4\x8d\x62\x33\xbf\x7c\x1a\x29\xc4\x93\xcd\x5b\x25"
        "\x96\x2a\x15\x3c\x06\xe4\x10\x81\x03\x43\x37\xf1\xf3\xb4\xa2\xb7\x9b"
        "\x45\x1c\x05\x4b\xd4\x00\x00\x00\x00\x00\x00\x00\x88\xab\x9e\xfa\x01"
        "\x00\x00\x00\x00\x00\x00\x00\xdd\x90\x31\xf7\xf6\x9b\xe9\xe3\xc0\x4b"
        "\x9d\x3b\xcf\x26\xcc\x3b\x4d\x74\x6a\x85\x26\xe1\x2c\x95\x36\x2e\x8a"
        "\x33\xee\x46\x17\x7d\x88\x3d\x7e\x5b\x78\xaf\xc7\x29\xe6\x54\x1e\x24"
        "\xb3\x19\xa9\x87\xdc\xfb\xe3\xe2\xa6\x3f\x4c\xa5\x0b\xd4\xe2\xfd\x52"
        "\x46\x41\xe7\xd4\xd0\xae\x8f\xe4\xa2\x6b\x25\x36\xc1\xd5\xef\x95\x0d"
        "\x06\x42\xbb\xb8\xeb\xe9\xba\x98\x60\x16\x40\xe1\xee\xec\xba\x07\x84"
        "\x61\x5e\x38\xd6\xd6\xcb\x64\x88\x78\x16\xf8\x0b\xda\x93\x55\xe3\x3c"
        "\xfa\x31\x12\x84\xd4\x4b\x03\xc2\xc4\x88\x30\x75\x29\x4e\x95\x00\x3a"
        "\xbf\x90\x5f\x9c\x2c\x16\x81\x32\xfc\x39\x34\xe2\x32\xd9\x71\x70\x04"
        "\x71\x8c\x51\xb0\xbe\x4b\x88\xe5\x51\xd2\x0c\xc4\x03\xbc\x28\xd5\xdd"
        "\xfc\xc9\xa5\xc2\xdd\x29\xab\x79\x4b\xab\xe2\xcf\x27\x1d\x8d\x22\xbf"
        "\x6b\x62\x87\xf4\x09\x91\xb1\xad\x7e\x1a\x8d\x5c\x64\xc7\xd7\x23\x93"
        "\x0c\x77\x21\x6a\x00\x00\x00\x00\x00\x00\x00\x00\x00",
        1135);
    syscall(__NR_write, (intptr_t)r[0], 0x20000900, 0x12e);
    break;
  case 2:
    *(uint32_t*)0x20000140 = 0xc;
    *(uint16_t*)0x20000144 = 0xcf;
    memcpy((void*)0x20000146,
           "\x9e\x0a\x10\xe6\x8f\x3f\x4a\xc1\xd2\x67\xfc\x11\x95\x4d\x09\x58"
           "\x00\xc7\xf5\xd5\x56\x46\xee\xeb\x21\x0f\x4c\xff\x6c\x99\xc5\x7b"
           "\xc7\x98\xdd\x96\x11\x32\xe4\x44\xc6\xb6\xc5\x0b\x95\xd2\x61\xf6"
           "\x48\x35\xd9\xcb\x55\x74\xfc\xe6\xb6\x95\xd4\x38\xf7\x19\x00\xbe"
           "\x20\xe0\x22\x06\x11\xd1\x9c\x8d\x00\x38\x83\xfc\xe4\xe8\x7e\x5c"
           "\x31\xa4\xad\xa8\xd1\x54\xcd\xea\x1b\xe3\x72\xc1\xe8\xc2\x5c\xaf"
           "\x2f\x7e\x36\x9c\x41\x63\x3f\xed\x73\x11\x3a\x8e\x34\xe4\xe6\x80"
           "\x28\x26\x14\x53\xef\x82\x74\xf7\x37\xe7\x00\xe4\xc3\x10\xe2\x8c"
           "\xac\x0d\x39\xc5\x59\xf4\x8f\xa9\xc8\xee\x75\xc9\x7d\x7b\x09\x64"
           "\x4b\x98\x56\xe2\x16\xf0\x69\xc3\x6a\x47\x53\x2d\x17\x3c\x7c\x43"
           "\x67\xd7\x63\x3f\x4e\xfb\xa6\x43\x9c\x92\x4e\x4f\xdf\x12\x84\x97"
           "\xd6\xac\xee\x6c\x9a\x67\x87\xb2\x03\xe1\xfc\x6e\x5e\xb7\x7c\xd3"
           "\xc2\x89\x99\x48\x2f\xbf\xd2\x56\x2e\x32\x7c\x93\xb9\x9a\xa1",
           207);
    syscall(__NR_write, (intptr_t)r[0], 0x20000140, 0xd5);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
