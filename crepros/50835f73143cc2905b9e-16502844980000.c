// https://syzkaller.appspot.com/bug?id=c124ed5f4fb2b852b116d34bc4a60011225aaabd
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
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

#ifndef __NR_close_range
#define __NR_close_range 436
#endif
#ifndef __NR_creat
#define __NR_creat 8
#endif
#ifndef __NR_fsconfig
#define __NR_fsconfig 431
#endif
#ifndef __NR_fsmount
#define __NR_fsmount 432
#endif
#ifndef __NR_fsopen
#define __NR_fsopen 430
#endif
#ifndef __NR_ioctl
#define __NR_ioctl 54
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_openat
#define __NR_openat 322
#endif
#ifndef __NR_socket
#define __NR_socket 281
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

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
      if (current_time_ms() - start < 15000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  memcpy((void*)0x20000000, "tmpfs\000", 6);
  res = syscall(__NR_fsopen, /*type=*/0x20000000, /*flags=*/0);
  if (res != -1)
    r[0] = res;
  syscall(__NR_fsconfig, /*fd=*/(intptr_t)r[0], /*cmd=*/6, /*key=*/0,
          /*value=*/0, /*aux=*/0);
  res = syscall(__NR_fsmount, /*fs_fd=*/(intptr_t)r[0], /*flags=*/0,
                /*attr_flags=*/0);
  if (res != -1)
    r[1] = res;
  syscall(__NR_close_range, /*fd=*/(intptr_t)r[0], /*max_fd=*/-1, /*flags=*/0);
  syscall(__NR_socket, /*domain=*/0x10, /*type=*/3, /*proto=*/0xc);
  memcpy((void*)0x20000540, "./file0\000", 8);
  syscall(__NR_creat, /*file=*/0x20000540, /*mode=*/0);
  memcpy(
      (void*)0x20000580,
      "\xbf\x25\xd3\xad\x4a\xb5\x1f\x69\xec\x68\xce\xc9\x50\x0e\xdb\x07\x00\xb1"
      "\x50\x3b\xc4\x06\x6f\xde\xac\x96\xb4\x40\x69\x24\xb0\xac\xaf\x8e\x95\xb7"
      "\x43\x4e\x31\xf2\x06\xd9\x0f\x2c\x03\x63\x97\xe0\xd5\x81\x2a\x95\x90\x79"
      "\xf1\xa5\x03\x5e\x62\x5e\x19\xec\x7f\x3c\x6e\xb1\xfc\x6a\x86\x3a\xf4\xf8"
      "\xba\xe1\x1d\x73\x03\x91\xa0\x57\x64\x29\x38\xeb\x59\xa5\x80\x08\x8c\xf1"
      "\xd2\x14\xcf\x24\x94\xfa\xa7\x95\x9e\x94\x93\x65\xdf\x53\xde\xaf\x18\x9c"
      "\xbd\x7d\x3f\xa6\x77\xe8\x3a\x0b\x32\x71\xa8\xdd\x8d\x42\xe6\xbb\x1a\xc5"
      "\x3c\x02\x30\x11\x96\x28\x0e\xc3\x09\x54\x9b\x9b\x6e\x8f\x1d\x0f\x61\x37"
      "\xbb\x61\x1d\x20\x52\x33\xcf\x08\x89\x01\x3a\x8c\x67\xb1\x1a\xf8\x54\x08"
      "\x85\xd9\x15\xb8\x3f\xf0\x3b\xc9\x17\x0d\x43\xff\xe6\xb4\x0d\xf1\x88\xcf"
      "\xf0\x0f\x44\x2a\xa9\xa8\x66\x03\x64\xc7\xcd\xec\xf7\x4b\x2f\x60\x1d\xcc"
      "\x98\xe0\x61\x02\x0e\x80\xfb\x6e\x49\x8b\x77\x4d\xab\xc7\x4f\x30\x11\x43"
      "\xd7\x80\x58\x31\x8e\x56\xf0\xb8\xbc\xf3\x03\x7d\x9b\x7a\x9f\x15\x25\x30"
      "\xbd\x75\x64\x90\x1e\xa5\x59\xf0\xb7\x9a\xfb\x3d\x00\x1a\x2c\x7e\x6d\x1a"
      "\xf1\x19\x8c\x00",
      256);
  syscall(__NR_ioctl, /*fd=*/(intptr_t)r[1], /*cmd=*/0x41009432,
          /*arg=*/0x20000580);
  memcpy((void*)0x20000000, "/sys/module/nf_conntrack_irc", 28);
  res = syscall(__NR_openat, /*fd=*/0xffffff9c, /*dir=*/0x20000000, /*flags=*/0,
                /*mode=*/0);
  if (res != -1)
    r[2] = res;
  syscall(__NR_close_range, /*fd=*/(intptr_t)r[2], /*max_fd=*/-1, /*flags=*/0);
  memcpy((void*)0x20000000, "./file0\000", 8);
  syscall(
      __NR_openat, /*fd=*/0xffffff9c, /*file=*/0x20000000,
      /*flags=O_NONBLOCK|O_NOFOLLOW|O_CREAT|O_CLOEXEC|O_APPEND|0x2*/ 0x88c42,
      /*mode=*/0);
  syscall(__NR_ioctl, /*fd=*/(intptr_t)r[2], /*cmd=*/0x81009431, /*arg=*/0);
}
int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000, /*len=*/0x1000, /*prot=*/0,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32, /*fd=*/-1,
          /*offset=*/0);
  syscall(__NR_mmap, /*addr=*/0x20000000, /*len=*/0x1000000,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32, /*fd=*/-1,
          /*offset=*/0);
  syscall(__NR_mmap, /*addr=*/0x21000000, /*len=*/0x1000, /*prot=*/0,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32, /*fd=*/-1,
          /*offset=*/0);
  loop();
  return 0;
}
