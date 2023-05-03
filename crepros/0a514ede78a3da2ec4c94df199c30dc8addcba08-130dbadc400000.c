// https://syzkaller.appspot.com/bug?id=0a514ede78a3da2ec4c94df199c30dc8addcba08
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <linux/futex.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/time.h>
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
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

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

static uint64_t current_time_ms()
{
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    fail("clock_gettime failed");
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void execute_one();
extern unsigned long long procid;

static void loop()
{
  int iter;
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      fail("clone failed");
    if (pid == 0) {
      prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
      setpgrp();
      execute_one();
      int fd;
      for (fd = 3; fd < 30; fd++)
        close(fd);
      doexit(0);
    }

    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      int res = waitpid(-1, &status, __WALL | WNOHANG);
      if (res == pid) {
        break;
      }
      usleep(1000);
      if (current_time_ms() - start < 5 * 1000)
        continue;
      kill(-pid, SIGKILL);
      kill(pid, SIGKILL);
      while (waitpid(-1, &status, __WALL) != pid) {
      }
      break;
    }
  }
}

struct thread_t {
  int created, running, call;
  pthread_t th;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

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
    for (thread = 0; thread < sizeof(threads) / sizeof(threads[0]); thread++) {
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
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 20 * 1000 * 1000;
        syscall(SYS_futex, &th->running, FUTEX_WAIT, 1, &ts);
        if (__atomic_load_n(&running, __ATOMIC_RELAXED))
          usleep((call == num_calls - 1) ? 10000 : 1000);
        break;
      }
    }
  }
}

#ifndef __NR_mkdir
#define __NR_mkdir 39
#endif
#ifndef __NR_pipe2
#define __NR_pipe2 331
#endif
#ifndef __NR_mount
#define __NR_mount 21
#endif
#ifndef __NR_write
#define __NR_write 4
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
unsigned long long procid;
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    memcpy((void*)0x20000000, "./file0", 8);
    syscall(__NR_mkdir, 0x20000000, 0);
    break;
  case 1:
    res = syscall(__NR_pipe2, 0x20000040, 0);
    if (res != -1) {
      r[0] = *(uint32_t*)0x20000040;
      r[1] = *(uint32_t*)0x20000044;
    }
    break;
  case 2:
    memcpy((void*)0x20000100, "./file0", 8);
    memcpy((void*)0x200001c0, "9p", 3);
    memcpy((void*)0x20000200, "trans=fd,", 9);
    memcpy((void*)0x20000209, "rfdno", 5);
    *(uint8_t*)0x2000020e = 0x3d;
    sprintf((char*)0x2000020f, "0x%016llx", (long long)r[0]);
    *(uint8_t*)0x20000221 = 0x2c;
    memcpy((void*)0x20000222, "wfdno", 5);
    *(uint8_t*)0x20000227 = 0x3d;
    sprintf((char*)0x20000228, "0x%016llx", (long long)r[0]);
    *(uint8_t*)0x2000023a = 0x2c;
    *(uint8_t*)0x2000023b = 0;
    syscall(__NR_mount, 0, 0x20000100, 0x200001c0, 0, 0x20000200);
    break;
  case 3:
    memcpy((void*)0x20000240,
           "\x0b\x00\x00\x00\x29\x00\x00\x8f\x47\x8b\x93\x2a\x30\x57\x20\x22"
           "\x34\xae\x5f\xf3\xc8\xef\x13\x6e\x65\x96\xa6\x9a\x6c\xa8\x9d\x03"
           "\x28\xd0\xa4\xa3\xcf\xe0\xc5\xd2\x63\x0e\xfc\x29\xcc\x64\x21\x61"
           "\x3c\xfe\x5a\xa7\x12\x3a\x8e\xe0\xfe\x54\x8f\xbe\x61\xcd\x95\x21"
           "\xdb\x9a\x75\xdd\x97\xed\x3a\xcb\x14\x3a\x5f\xe8\xdd\xfa\x51\x5a"
           "\x6c\x09\x32\xb9\x6e\xcb\xbb\xe1\xc5\x6b\xc8\x45\x01\x41\x47\xf3"
           "\xfd\x63\xbf\x1d\xff\x05\xd7\x92\xa6\xa2\xbf\xdd\x17\x99\x0a\x17"
           "\x83\x32\xa4\x99\xbd\xa6\x9e\x06\x76\x0d\x1d\x13\x72\xba\x5f\x19"
           "\xd0\x7a\x0c\x1f\x7b\xa1\xfb\x33\xa3\x83\xa6\xdb\x8a\x97\xc6\x3c"
           "\xb0\xbb\xbf\x8b\xa0\x1f\x90\x0c\xe9\x3d\xc7\x6c\x6d\xe3\xbf\x0f"
           "\x86\x12\x0e\x41\xdc\x26\xc6\xe6\x9b\x7c\x3e\xc8\x38\xbd\xbe\x9b"
           "\xf5\xd3\xd7\xd6\xa2\x81\x2e\x97\x76\xd5\xb9\x44\xcc\x7f\x10\xeb"
           "\xd6\x93\x34\x3d\xd8\x32\xba\xf4\xaa\x3c\xcb\xaf\xcd\xa3\x43\x6d"
           "\xde\xa7\xbb\x1a\x75\x84\x32\x44\xe6\xa6\xd0\xaa\x02\x82\x6f\xa0"
           "\x84\xd8\xcc\xc3\x2a\x73\x11\x5f\x70\xa0\x1f\x1f\xf2\xe8\x7b\xa9"
           "\x92\x05\x34\x36\x97\x60\x66\x08\x0b",
           249);
    syscall(__NR_write, r[1], 0x20000240, 0xf9);
    break;
  }
}

void execute_one()
{
  execute(4);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      for (;;) {
        loop();
      }
    }
  }
  sleep(1000000);
  return 0;
}
