// https://syzkaller.appspot.com/bug?id=18cd046b8cd9550398a589c3952ee6a3cd94408e
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
  for (call = 0; call < 5; call++) {
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
      if (current_time_ms() - start < 5 * 1000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    memcpy((void*)0x20000080, "./file0\000", 8);
    syscall(__NR_open, 0x20000080ul, 0x1850c5ul, 0ul);
    break;
  case 1:
    res = syscall(__NR_pipe2, 0x20000240ul, 0ul);
    if (res != -1) {
      r[0] = *(uint32_t*)0x20000240;
      r[1] = *(uint32_t*)0x20000244;
    }
    break;
  case 2:
    res = syscall(__NR_dup, r[1]);
    if (res != -1)
      r[2] = res;
    break;
  case 3:
    memcpy((void*)0x20000000, "./file0\000", 8);
    memcpy((void*)0x20000200, "9p\000", 3);
    memcpy((void*)0x200028c0, "trans=fd,", 9);
    memcpy((void*)0x200028c9, "rfdno", 5);
    *(uint8_t*)0x200028ce = 0x3d;
    sprintf((char*)0x200028cf, "0x%016llx", (long long)r[0]);
    *(uint8_t*)0x200028e1 = 0x2c;
    memcpy((void*)0x200028e2, "wfdno", 5);
    *(uint8_t*)0x200028e7 = 0x3d;
    sprintf((char*)0x200028e8, "0x%016llx", (long long)r[2]);
    *(uint8_t*)0x200028fa = 0x2c;
    memcpy((void*)0x200028fb, "cache=loose", 11);
    *(uint8_t*)0x20002906 = 0x2c;
    *(uint8_t*)0x20002907 = 0x6b;
    syscall(__NR_mount, 0ul, 0x20000000ul, 0x20000200ul, 0ul, 0x200028c0ul);
    break;
  case 4:
    memcpy(
        (void*)0x200003c0,
        "\x3b\x27\xa4\xb4\x6e\xe9\x2b\x4a\x59\x07\x3c\x36\x9a\x5e\x19\xf9\xdb"
        "\x15\x3c\x4f\xdb\xc7\x6a\xa2\xa4\xbb\x9f\x3e\x5e\x1a\xa1\x97\xa9\xe9"
        "\x7d\x10\x16\xc0\x18\x13\x79\x2e\x50\xc2\x69\x2c\x17\x5a\xad\x71\x5d"
        "\x11\x0a\x89\x29\x49\xcc\xc6\xe2\xe5\x4c\x2d\x5c\x8f\x0b\x79\x32\xb6"
        "\x97\x97\xf2\x17\x16\x8b\x0c\x1f\xeb\x12\x8a\xe3\x4f\x0d\xaf\x48\x7a"
        "\x70\xb5\xc1\x17\xac\xd4\x37\x25\xfe\x17\x99\x36\x34\xf1\x69\x5d\xab"
        "\xd7\xf9\x98\xcd\x55\xe9\xd5\xbd\x91\x1e\x86\xaa\x7a\x4a\xd7\x5a\x57"
        "\x4b\xb9\x69\x51\xd6\x01\x8b\x25\xd9\x42\xa9\xbb\x0e\x8d\x10\xc0\x92"
        "\xcd\xcb\x85\x79\x76\x73\x97\x20\x99\xe4\x04\x1a\xaf\x8d\x63\x6f\x66"
        "\xcb\x11\x03\xef\x20\x50\xad\x28\xfa\xba\xed\x33\xd6\x92\x78\x89\xd9"
        "\x7f\x4b\x5c\xe0\xde\x71\xd3\xfd\x83\x29\x80\xf4\xf0\x88\xd0\xd8\x24"
        "\xe2\x05\x49\xb4\xbb\xd9\x06\xff\xa5\x1c\xe9\xde\x54\xd7\x79\xeb\x4d"
        "\xe4\x62\xfa\xac\x20\xa3\xab\x0e\xd9\x93\x43\x73\xca\x22\xce\xa5\x45"
        "\x4f\x4c\x2a\x74\x0c\xd4\x61\xe3\x99\x56\xbb\x5f\x98\xdf\x2a\xeb\xc6"
        "\x0c\xf3\x26\x23\xad\xbf\xfb\xcc\x37\x8f\xa7\x25\x0b\x6a\x3f\xc8\x63"
        "\xda\xdc\xf6\xd4\x43\xcd\x08\xce\xfe\x88\x38\x1b\x8c\xf9\x59\xad\x3e"
        "\x35\x3f\x81\xf8\xb8\x55\xc4\xe7\x0f\x07\x96\xee\xe6\x21\x84\x45\xda"
        "\xd2\x81\x1d\xd6\xb5\x40\xff\x52\xef\xa2\xf1\x67\xdd\x9c\x1b\x8b\x01"
        "\x62\x68\xd3\x7d\xb4\x30\x98\x3f\xef\xc0\x64\x5d\x20\x61\x4c\x8d\xf2"
        "\xeb\x08\x72\xc5\x8e\x09\x66\x4e\x67\x2b\x0b\x6a\x99\x70\xfe\xc1\x99"
        "\x25\x7e\x1c\x60\x6e\xc3\xe3\x64\xc6\x6a\x0f\x4d\x25\x8c\x74\xac\xcd"
        "\x43\xb9\x87\xc7\x56\xd6\x02\xfd\x0c\x9c\x50\x13\xb4\x0f\xe9\x7f\x6a"
        "\x40\xee\x87\x87\xfe\xd3\xaa\x43\xfd\x8d\x84\xe9\x65\x6d\xa8\xb7\x15"
        "\x39\xd1\x6c\x4b\xef\xfb\x62\x3f\xa1\xca\x53\xf9\x65\x4a\x41\x3f\xa9"
        "\xa4\x23\xbc\x54\xb8\x73\x58\x3d\x6d\x49\x70\x05\xe5\x47\x12\xfa\xfc"
        "\x71\x38\x49\x88\xd8\x01\x34\xfb\xf8\x4f\x53\xfd\xd7\x4b\x35\x48\x48"
        "\x00\x6b\x8b\x5b\x67\xe7\xcc\x5a\x47\x24\x75\xd3\xae\x54\x5c\xa1\xfc"
        "\xf7\xb0\xcf\xbe\x97\x11\xb5\x17\xa9\xa1\x38\x8a\xd0\xef\xa2\xa3\xb4"
        "\xe2\x21\x52\x02\x1d\x63\x1b\x73\x1e\x2e\x10\x0a\x98\x31\x11\x1d\xb7"
        "\xac\xce\x94\x8b\xb5\xde\xee\xa2\x60\x46\x3c\x14\x0a\xc9\x29\xe7\x7c"
        "\x58\x40\x27\x76\xca\xf8\x5d\x45\x69\xa7\x5d\xde\x2f\x64\xc4\x49\x15"
        "\x08\xaf\xb5\x41\xed\x9b\x2c\x81\xfc\x95\xc0\x67\x06\x23\x5f\x38\x3e"
        "\x31\xcf\x66\x2c\x95\xb1\xe4\x9c\xfd\x94\x87\x1e\x09\x00\x00\x00\x00"
        "\x00\x00\x00\x19\xb2\x71\x27\x69\x41\x69\x2b\xd0\x23\xdd\x9c\x9d\xba"
        "\xc4\xf7\xdb\x1e\x5c\x00\xd8\xb3\xbe\x7b\x8e\x82\x6a\x6a\xad\xd0\x01"
        "\xed\xd0\xdf\xeb\x00\xf8\x04\x84\x42\xb5\xc4\x84\x56\xfd\x64\x2e\x62"
        "\x9d\xcb\x2f\xf5\x55\x92\x66\x5f\xf4\x91\xcd\x00\x26\x72\xce\x4d\x99"
        "\x9d\xa1\x86\xdb\x2c\x3a\x1f\x8b\x6b\x1f\x7d\x37\x50\xd7\xcd\xb3\x09"
        "\x79\x54\xe6\xe1\x4f\xb2\x18\x3a\xd6\x62\xc6\x3d\x4c\xe8\xb8\x2d\xc2"
        "\x48\x7f\x0f\xe2\xea\x28\x27\xb5\x3a\x7c\x6d\xcc\xed\x87\x8d\x2f\xb2"
        "\x9c\x1d\x3f\xf5\x83\x57\x0e\x7b\xc1\x72\xd1\xa5\xc7\x16\xe0\x44\x7c"
        "\xb0\x8c\xe3\xc4\x68\xff\xdf\x97\x5d\xa3\x72\xf3\xf3\xeb\x45\x5a\xaf"
        "\x58\x22\xbc\x04\xa5\x1b\x6c\xad\x24\xa2\x33\x13\x69\xdf\x81\xc1\x23"
        "\xb0\x09\xa2\x38\x1b\x42\xe9\xae\xb0\x77\xf6\x21\x60\x8d\x81\xc1\x2a"
        "\x5f\x5c\x6c\x29\x5d\x74\xaf\xd4\xdd\x5c\x05\x12\x96\xbe\x0b\x54\xc7"
        "\x0b\xf8\x99\xb3\x47\xc3\x6b\xff\x62\xf3\x13\x07\x99\x83\x40\x9d\x7f"
        "\x9c\xf1\x24\x2c\x91\x79\x85\xc1\xb5\xd0\x73\x6f\xe2\x1f\x85\x14\xf6"
        "\x3d\x03\x69\xa3\x74\xc4\x2d\xa4\x0b\xd5\x14\x0b\xc3\xe6\x02\xd0\x0c"
        "\x3c\xb4\xf8\xe6\x21\x86\x3a\xb4\x74\x22\x77\x8d\x67\xd7\x2d\xe3\x47"
        "\x53\xfd\x72\xce\xf8\x06\x49\xa1\x54\x8e\x4e\x8d\xcb\xcf\xfe\x40\x54"
        "\xcc\x9d\x8a\x1f\x92\x26\x23\xa7\x59\x04\xcb\xda\xac\xde\x76\x81\x31"
        "\xe5\x87\x26\x9a\x4a\x99\xd8\x2f\x70\x09\xc1\xb8\xab\x79\xaa\x23\x2a"
        "\x2f\xd4\x5a\xd7\x1b\x60\x38\x03\x12\x3f\x6b\xa9\x79\xfa\x6a\x87\x52"
        "\x58\x84\xb0\x8d\x72\x1a\x21\x40\x0f\xb1\xf9\x50\xb9\x6e\xad\x82\xf4"
        "\x08\xcc\x43\x88\xd3\xb7\x8f\xb4\x56\x61\x64\x29\xa5\x20\x65\x6d\x5e"
        "\x5a\x87\x6f\xd0\x47\x48\x49\x89\x02\xc8\x6f\x58\xd4\x5f\x4c\x1b\x39"
        "\x19\xeb\x84\x6a\x00\xed\xf0\x7e\x7a\x83\x0b\xf7\x23\xe4\x77\x4f\x08"
        "\x5f\x15\x53\x4d\xd3\xb5\x24\x6c\x0c\x09\x70\xb5\xad\x7b\xb3\x9b\x30"
        "\xb1\x56\xa9\x43\x03\x78\xc5\xb0\xaa\xb1\x26\x1c\x78\xd7\x2a\xc3\x01"
        "\xcd\x55\x2d\x5e\x8d\xd4\xb6\x42\xec\x1d\xc0\x67\x27\x45\xd5\x93\xbb"
        "\x26\xd0\x95\xb5\xb2\x35\x76\xe3\xcf\xd6\xab\x58\x0f\x6e\x09\x41\x9d"
        "\x0f\x0c\x64\x25\x0f\xaf\xaa\x37\x59\xaa\x18\x88\xda\x48\xd8\x9c\x3f"
        "\x7c\x94\x54\xb0\xb3\xd0\xab\x40\x44\x5f\x5b\xed\x44\x93\xef\x43\xab"
        "\x08\xf3\x1b\x13\x45\xac\x4f\xfd\x94\xad\x79\xc9\xee\xe5\x39\x04\xed"
        "\x6f\x57\x28\x17\x15\x31\x90\xd2\xe6\x86\x3f\x2e\x39\x35\x6b\xb9\x99"
        "\x26\x41\x9f\xd3\x14\x34\x1a\x53\x6b\x7e\x76\xca\xe6\x0b\xf7\x75\x0a"
        "\x4c\x29\xe3\xf4\xc7\xf0\x05\x53\x0b\x1d\x4e\xe0\xe2\x5b\x93\xb7\x6f"
        "\xcc\x11\x08\x22\x2f\x0b\x00\xde\x52\xcf\x41\x00\xe9\x7a\xdf\xd7\xb9"
        "\xdb\x13\x70\x58\x6b\xa2\x7e\x1e\x18\x32\x99\xbe\x00\xd0\xdf\x84\xb0"
        "\x4a\xcc\xdf\xf5\xe1\x7f\x02\x04\x61\x39\xf9\x1f\x03\x32\x66\x16\x76"
        "\xff\x50\x6e\x17\x5f\x0c\xb2\x85\x0b\xcc\x9f\x86\x66\xf6\xd1\xf6\x9f"
        "\x8f\x42\x71\xcb\x80\x4a\x79\xfc\xcd\x70\x16\xf0\x49\xd1\xa4\x94\xc2"
        "\x6a\x52\x7c\x43\x7f\xa0\xbe\x6d\x51\xec\x75\x43\xd9\xbd\x7a\x2f\x01"
        "\x61\x94\xeb\xe3\xc9\x90\x80\xa6\xc9\xb5\x11\x98\x63\xdf\xe8\x65\xf8"
        "\xe6\x0c\xae\x29\xf5\x0b\x67\xdb\xfa\xa0\xa3\xc9\x79\x4d\x73\x03\x44"
        "\x85\xca\x16\x13\x34\x4c\x57\x27\x83\xdb\x3d\xfa\xb0\x1b\x28\x08\x9c"
        "\x51\xcd\xa9\x9c\xef\xa4\xc1\xc8\x81\xa2\x9e\x22\x9f\x04\xc7\xe0\xfd"
        "\x04\xdc\x42\x5a\xe8\x41\x78\x52\xe6\xe3\x15\x20\xc6\x20\x7e\x9d\x4e"
        "\x35\x28\x5f\xee\xf2\xa2\xcb\x8a\x3b\xce\xb0\x8a\x16\x6f\xa4\x28\x4a"
        "\x51\x63\x62\x62\x1e\x2c\x06\x73\x1a\x44\x27\x91\xf1\xdb\x06\x3a\x32"
        "\xcf\x1f\x00\x5c\x91\x41\x02\xc7\x27\x3c\xb4\xd7\xab\x1b\xf5\x67\xd7"
        "\x2f\x23\x07\x83\xd2\xea\x99\xc4\x3a\x60\xe8\x72\x91\x32\x44\x1e\xe6"
        "\xc5\x36\x2c\x33\xf9\xb6\x13\xf8\x44\x17\xc3\xc5\x54\x9f\x4e\x3d\x9e"
        "\x73\xc6\xf8\x3f\x16\xc8\xe5\x7a\xe2\x2f\xe5\xf5\x45\x15\xe1\x11\xfe"
        "\x43\xad\x7c\x40\x0d\x21\x42\x81\x45\x2b\xb6\x14\x1c\xec\xad\x84\xb2"
        "\x3a\x69\x5f\x06\x19\x88\xd9\x06\xd0\x3b\xe5\xd8\x95\x84\x63\x4b\x9e"
        "\x9d\x9a\x9b\x07\x2f\x8e\x7c\xbb\x47\xc4\x77\x19\x31\x8a\x20\x01\xca"
        "\xfa\x66\x5d\xe8\xc8\x26\x72\xd1\x93\x77\xea\x11\x5b\xd0\x23\xfc\x19"
        "\x75\xf7\xc5\x96\x64\xbf\xb0\x6f\x66\xa1\xa5\xe3\xf0\x5c\xb2\x83\xfb"
        "\x45\xea\x67\xa2\x72\x7e\xe6\xe1\x0b\xf3\x5b\x31\xfd\xd0\x3d\x43\xec"
        "\x67\xb7\x53\xf6\x73\x7e\x0d\x2f\x4a\x52\x75\x03\x15\x95\x87\x8c\xef"
        "\xc8\xf0\x37\x5b\x95\x66\x82\xa8\xa4\x50\x03\xbd\x05\xd7\xa7\x70\x52"
        "\xae\x6f\x09\x52\xc1\x67\x70\x46\xae\x38\x05\xd6\x67\xc0\x65\x5e\xd1"
        "\xd3\x6c\xb3\x32\x5d\xda\x8b\xca\x5b\x62\x04\x76\x07\x46\x95\xb8\xbf"
        "\x4c\x2a\xa0\xf6\xa4\x5a\x72\xc0\x0f\xfd\x68\x46\xd0\x42\x69\x4d\x83"
        "\x04\x4a\xd8\xd5\x21\x56\x6b\x0e\x74\xe4\x92\x51\xb9\xbe\xaf\x34\x51"
        "\xdf\x47\x80\xd5\x3a\xe8\x54\x1b\x71\x36\x82\x9a\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
        1659);
    syscall(__NR_write, r[2], 0x200003c0ul, 0xfcfbul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
