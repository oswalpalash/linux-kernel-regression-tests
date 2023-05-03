// https://syzkaller.appspot.com/bug?id=99845d9d7da6f50ed85d1d0c52259abd36b118a2
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/capability.h>

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

#define MAX_FDS 30

static void setup_common()
{
  if (mount(0, "/sys/fs/fuse/connections", "fusectl", 0, 0)) {
  }
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();
  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = (200 << 20);
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 32 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 136 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 256;
  setrlimit(RLIMIT_NOFILE, &rlim);
  if (unshare(CLONE_NEWNS)) {
  }
  if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL)) {
  }
  if (unshare(CLONE_NEWIPC)) {
  }
  if (unshare(0x02000000)) {
  }
  if (unshare(CLONE_NEWUTS)) {
  }
  if (unshare(CLONE_SYSVSEM)) {
  }
  typedef struct {
    const char* name;
    const char* value;
  } sysctl_t;
  static const sysctl_t sysctls[] = {
      {"/proc/sys/kernel/shmmax", "16777216"},
      {"/proc/sys/kernel/shmall", "536870912"},
      {"/proc/sys/kernel/shmmni", "1024"},
      {"/proc/sys/kernel/msgmax", "8192"},
      {"/proc/sys/kernel/msgmni", "1024"},
      {"/proc/sys/kernel/msgmnb", "1024"},
      {"/proc/sys/kernel/sem", "1024 1048576 500 1024"},
  };
  unsigned i;
  for (i = 0; i < sizeof(sysctls) / sizeof(sysctls[0]); i++)
    write_file(sysctls[i].name, sysctls[i].value);
}

static int wait_for_loop(int pid)
{
  if (pid < 0)
    exit(1);
  int status = 0;
  while (waitpid(-1, &status, __WALL) != pid) {
  }
  return WEXITSTATUS(status);
}

static void drop_caps(void)
{
  struct __user_cap_header_struct cap_hdr = {};
  struct __user_cap_data_struct cap_data[2] = {};
  cap_hdr.version = _LINUX_CAPABILITY_VERSION_3;
  cap_hdr.pid = getpid();
  if (syscall(SYS_capget, &cap_hdr, &cap_data))
    exit(1);
  const int drop = (1 << CAP_SYS_PTRACE) | (1 << CAP_SYS_NICE);
  cap_data[0].effective &= ~drop;
  cap_data[0].permitted &= ~drop;
  cap_data[0].inheritable &= ~drop;
  if (syscall(SYS_capset, &cap_hdr, &cap_data))
    exit(1);
}

static int do_sandbox_none(void)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid != 0)
    return wait_for_loop(pid);
  setup_common();
  sandbox_common();
  drop_caps();
  if (unshare(CLONE_NEWNET)) {
  }
  loop();
  exit(1);
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

static void close_fds()
{
  for (int fd = 3; fd < MAX_FDS; fd++)
    close(fd);
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
      close_fds();
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

void execute_one(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 0x803ul, 0);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000280 = 0;
  *(uint32_t*)0x20000288 = 0;
  *(uint64_t*)0x20000290 = 0x20000180;
  *(uint64_t*)0x20000180 = 0;
  *(uint64_t*)0x20000188 = 0;
  *(uint64_t*)0x20000298 = 1;
  *(uint64_t*)0x200002a0 = 0;
  *(uint64_t*)0x200002a8 = 0;
  *(uint32_t*)0x200002b0 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000280ul, 0ul);
  *(uint32_t*)0x200002c0 = 0x14;
  res = syscall(__NR_getsockname, r[0], 0x20000100ul, 0x200002c0ul);
  if (res != -1)
    r[1] = *(uint32_t*)0x20000104;
  *(uint64_t*)0x20000240 = 0;
  *(uint32_t*)0x20000248 = 0;
  *(uint64_t*)0x20000250 = 0x20000140;
  *(uint64_t*)0x20000140 = 0x200014c0;
  memcpy((void*)0x200014c0, "\x38\x00\x00\x00\x24\x00\x07\x05\x00\x00\x00\x40"
                            "\x07\xa2\xa3\x00\x05\x00\x00\x00",
         20);
  *(uint32_t*)0x200014d4 = r[1];
  memcpy((void*)0x200014d8, "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00"
                            "\x09\x00\x01\x00\x68\x66\x73\x63\x00\x00\x00\x00"
                            "\x08\x00\x02",
         27);
  *(uint64_t*)0x20000148 = 0x38;
  *(uint64_t*)0x20000258 = 1;
  *(uint64_t*)0x20000260 = 0;
  *(uint64_t*)0x20000268 = 0;
  *(uint32_t*)0x20000270 = 0;
  syscall(__NR_sendmsg, -1, 0x20000240ul, 0ul);
  *(uint64_t*)0x20000200 = 0;
  *(uint32_t*)0x20000208 = 0;
  *(uint64_t*)0x20000210 = 0x20000180;
  *(uint64_t*)0x20000180 = 0x20001500;
  memcpy((void*)0x20001500, "\x7c\x0c\x00\x00\x2c\x00\x27\x0d\x00\x00\x00\x8c"
                            "\xbb\x7e\x84\xa7\x9b\x02\x89\x0d",
         20);
  *(uint32_t*)0x20001514 = r[1];
  memcpy(
      (void*)0x20001518,
      "\x00\x00\x00\x00\x00\x00\x00\x00\x0e\x00\x00\x00\x0a\x00\x01\x00\x62\x61"
      "\x73\x69\x63\x00\x00\x00\x5c\x00\x02\x00\x58\x00\x03\x00\x54\x00\x01\x00"
      "\x09\x00\x01\x00\x76\x6c\x61\x6e\x00\x00\x00\x00\x28\x00\x02\x80\x06\x00"
      "\x03\x00\x00\x00\x00\x00\x1c\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x04\x00"
      "\x06\x00\x0c\x00\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0c\x00\x08\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x0a\x00\x01\x00\x72\x6f\x75\x74\x65\x00"
      "\x00\x00\xe4\x0b\x02\x00\x08\x00\x03\x00\xbd\x00\x00\x00\x08\x00\x02\x00"
      "\x96\x00\x00\x00\xb8\x01\x06\x00\x5c\x01\x07\x00\x0b\x00\x01\x00\x73\x61"
      "\x6d\x70\x6c\x65\x00\x00\x6c\x00\x02\x80\x08\x00\x04\x00\x85\xd0\x0b\x24"
      "\x08\x00\x05\x00\x00\x00\x00\x00\x08\x00\x03\x00\xff\xff\xff\xff\x18\x00"
      "\x02\x00\x03\x00\x00\x00\x05\x00\x00\x00\x05\x00\x00\x00\x06\x00\x00\x00"
      "\xff\x07\x00\x00\x08\x00\x03\x00\x07\x00\x00\x00\x08\x00\x05\x00\x90\x04"
      "\x00\x00\x18\x00\x02\x00\xff\x03\x00\x00\xff\x03\x00\x00\xfd\xff\xff\xdf"
      "\x00\x00\x00\x00\x03\x00\x00\x00\x08\x00\x05\x00\x00\x00\x00\x00\x08\x00"
      "\x05\x00\xff\x07\x00\x00\xc6\x00\x06\x00\x0d\x4b\x76\xd2\x04\x64\x9d\xfa"
      "\x87\xc5\xf9\xa8\x06\x1a\x23\x67\x38\x38\x0e\x51\x79\x00\xc4\xb8\xd4\x95"
      "\xdc\x26\x02\x14\x7c\x8c\xba\xd0\x77\x9d\x2f\x6b\x74\x36\xef\xc9\x07\x31"
      "\x8c\x72\xa8\x86\x38\xc0\x59\x11\x49\x9f\x37\xbd\x90\xb8\x0b\x61\xca\x4a"
      "\x3a\x1c\xd6\x40\x55\x8c\xe7\xf8\x0b\xb3\xc7\xe3\x79\x64\x40\x85\x15\x32"
      "\x37\xcc\xde\xbf\x90\x20\x0e\xb9\x68\x9d\xc8\x6a\x76\x9e\xdd\x85\xea\x34"
      "\xa0\xdb\xf1\xca\x84\xda\xf6\x41\xe2\xb5\xdb\xdf\x4b\x9f\xfc\xfa\x6e\xed"
      "\xa3\x5b\xac\xe5\xf3\x4e\xdd\x5d\x3c\x8f\x3a\x4b\x84\xf7\x4c\xf1\xdc\x9a"
      "\x1d\x4d\x48\xfd\xa3\x30\x56\x37\xff\x42\xc5\xf2\x75\x39\x92\xb1\xc3\x57"
      "\xe0\x7d\x02\x63\xa3\xf0\xfe\xfa\xb4\x50\x2f\x77\x2f\x07\x34\xb1\x59\x13"
      "\x04\xcd\x49\xdb\xe6\xd0\x1a\x08\x88\x63\xc1\xec\x82\xb7\x3c\x9c\x46\x09"
      "\x6d\x2a\x61\xc3\xc7\x70\x00\x00\x0c\x00\x07\x00\x00\x00\x00\x00\x01\x00"
      "\x00\x00\x0c\x00\x08\x00\x01\x00\x00\x00\x46\x31\xaf\x10\x58\x00\x12\x00"
      "\x0d\x00\x01\x00\x63\x6f\x6e\x6e\x6d\x61\x72\x6b\x00\x00\x00\x00\x20\x00"
      "\x02\x80\x1c\x00\x01\x00\x01\x01\x00\x00\x09\x00\x00\x00\x06\x00\x00\x00"
      "\xe8\x0a\xea\x7c\x09\x00\x00\x00\x04\x00\x00\x00\x0c\x00\x06\x00\x8e\x74"
      "\x5a\xf1\x8b\x1b\x11\xc6\x0c\x00\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x0c\x00\x08\x00\x00\x00\x00\x00\x03\x00\x00\x00\x8c\x08\x05\x00\x04\x04"
      "\x03\x00\x01\x00\x00\x00\x06\x00\x00\x00\xb6\x00\x00\x00\x21\x00\x00\x00"
      "\x00\x00\x00\x80\x07\x00\x00\x00\x01\x00\x00\x00\x05\x00\x00\x00\xeb\x00"
      "\x00\x00\x08\x00\x00\x00\x07\x00\x00\x00\x03\x00\x00\x00\x01\x04\x00\x00"
      "\xe0\xaf\x00\x00\x00\x01\x00\x00\x03\x00\x00\x00\x90\x06\x00\x00\x07\x00"
      "\x00\x00\x09\x00\x00\x00\x1b\x98\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x20\x00\x00\x00\xff\x7f\x00\x00\xc6\x04\x00\x00\x09\x00"
      "\x00\x00\x00\x80\xff\xff\x00\x01\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00"
      "\x07\x00\x00\x00\xd8\xda\x00\x00\xff\x7f\x00\x00\x01\x00\x00\x00\x02\x00"
      "\x00\x00\x04\x00\x00\x00\x06\x00\x00\x00\x02\x00\x00\x00\x7f\x00\x00\x00"
      "\x02\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00\x03\x00\x00\x00\x00\x00"
      "\x00\x00\xff\xff\xff\x7f\x00\x00\x00\x00\x3f\x00\x00\x00\x01\x00\x00\x00"
      "\x3f\x00\x00\x00\x00\x00\x00\x80\x1f\x00\x00\x00\x02\x00\x00\x00\x4a\x07"
      "\x00\x00\x42\x8e\x00\x00\x04\x00\x00\x00\xf8\xff\xff\xff\x00\x00\x02\x00"
      "\x07\x00\x00\x00\xff\x03\x00\x00\x01\x00\x00\x00\x00\x01\x00\x00\x07\x00"
      "\x00\x00\x99\x4a\xff\xff\x09\x00\x00\x00\x02\x00\x00\x00\xff\x03\x00\x00"
      "\x00\x00\x00\x00\x02\x00\x00\x00\xff\xff\x00\x00\x08\x00\x00\x00\xff\xff"
      "\xff\x7f\x05\x06\x00\x00\x81\x00\x00\x00\xff\x01\x00\x00\x07\x00\x00\x00"
      "\x00\x00\x00\x00\x03\x00\x00\x00\x02\x00\x00\x00\x00\x10\x00\x00\xff\xff"
      "\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x09\x00\x00\x00\x00\x10\x00\x00"
      "\x07\x00\x00\x00\x01\x80\x00\x00\x2b\x02\x00\x00\x01\x00\x00\x00\x05\x00"
      "\x00\x00\x01\x00\x00\x00\x4d\x01\x00\x00\x40\x00\x00\x00\x05\x00\x00\x00"
      "\x09\x00\x00\x00\x8b\x00\x00\x00\xe2\x09\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x09\x00\x00\x00\x01\x04\x00\x00\x01\x00\x00\x80\xff\x00\x00\x00"
      "\x03\x00\x00\x00\x00\x80\x00\x00\x01\x04\x00\x00\x00\x00\x01\x00\x05\x00"
      "\x00\x00\x7a\xff\xff\xff\x00\x00\x12\x00\xff\x03\x00\x00\xca\x00\x00\x00"
      "\x08\x00\x00\x00\x06\x00\x00\x00\x00\x08\x00\x00\xff\xff\xff\x7f\xff\x7f"
      "\x00\x00\x02\x00\x00\x00\x01\x00\x00\x00\x5f\x4f\x00\x00\x04\x00\x00\x00"
      "\x03\x00\x00\x00\x01\x01\x00\x00\x00\x00\x01\x00\x04\x00\x00\x00\xfd\x03"
      "\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x57\x0a\x00\x00\x00\x00\x00\x00"
      "\x08\x00\x00\x00\xfa\xff\xff\xff\xff\x01\x00\x00\x41\xd2\x00\x00\x05\x00"
      "\x00\x00\xff\xff\xff\x7f\x06\x00\x00\x00\x02\x00\x00\x00\x01\x00\xff\xff"
      "\x00\x00\xe0\xff\x7f\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x80\x00\x00"
      "\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\x7f\x09\x00\x00\x00"
      "\xd9\x0d\x00\x00\x05\x00\x00\x00\x03\x00\x00\x00\x09\x00\x00\x00\x08\x00"
      "\x00\x00\x09\x00\x00\x00\x06\x00\x00\x00\x05\x00\x00\x00\x01\x01\x00\x00"
      "\x81\x00\x00\x00\x09\x00\x00\x00\x04\x00\x00\x00\x05\x00\x00\x00\x00\x00"
      "\x00\x80\xff\x07\x00\x00\x00\x00\x00\x00\x07\x00\x00\x00\x09\x00\x00\x00"
      "\x00\x00\x00\x80\xc3\x07\x00\x00\x00\x00\x01\x00\xfb\xff\xff\xff\x04\x00"
      "\x00\x00\x01\x00\x00\x00\x89\x00\x00\x00\x05\x00\x00\x00\x06\x00\x00\x00"
      "\x81\xff\xff\xff\x08\xcf\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x20\x00"
      "\x00\x00\x09\x00\x00\x00\x00\x00\x00\x00\x05\x00\x00\x00\x00\x01\x00\x00"
      "\x03\x00\x00\x00\x19\x09\x00\x00\x59\x06\x00\x00\x0c\x7d\x2d\x40\x01\x00"
      "\x00\x80\x01\x00\x00\x80\x00\x10\x00\x00\x01\x00\x00\x00\x56\x57\x00\x00"
      "\x00\x00\x00\x00\xaf\xd9\x00\x00\x07\x00\x00\x00\x50\x00\x00\x00\x00\x80"
      "\x00\x00\x01\x80\x00\x00\xc1\xff\xff\xff\x05\x00\x00\x00\x06\x00\x00\x00"
      "\x3e\xfc\xff\xff\x75\x06\x00\x00\x64\x02\x00\x00\xb3\xdb\x00\x00\x07\x00"
      "\x00\x00\x09\x00\x00\x00\x00\x08\x00\x00\x7f\x00\x00\x00\x00\x04\x00\x00"
      "\x2b\x00\x00\x00\x02\x00\x00\x00\xfe\xff\xff\xff\x00\x00\x00\x00\x04\x00"
      "\x00\x00\x00\x00\x00\x02\x57\x0a\x00\x00\x90\x8b\x00\x00\x01\x80\x00\x00"
      "\x08\x00\x00\x00\x08\x00\x00\x00\xff\x01\x00\x00\xfb\xff\xff\xff\x01\xf8"
      "\xff\xff\x07\x00\x00\x00\x00\x00\x80\x00\x06\x00\x00\x00\x01\x00\x00\x00"
      "\xff\xff\xff\xff\x78\x0b\x00\x00\x08\x00\x00\x00\x07\x00\x00\x00\x05\x00"
      "\x00\x00\x00\x08\x00\x00\xff\xff\x00\x00\x08\x00\x00\x00\x02\x00\x00\x00"
      "\x06\x00\x00\x00\x08\x00\x00\x00\x00\x00\x00\x00\x07\x00\x00\x00\x05\x00"
      "\x00\x00\x00\xf8\xff\xff\x50\x0f\x00\x00\x1f\x00\x00\x00\x07\x00\x00\x00"
      "\x05\x00\x00\x00\x09\x00\x00\x00\x00\x04\x00\x00\x00\x08\x00\x00\x3f\x00"
      "\x00\x00\x02\x00\x00\x00\x01\x80\x00\x00\x03\x00\x00\x00\x94\x00\x00\x00"
      "\x3c\x00\x01\x00\x06\x00\x00\x00\x01\x00\x00\x00\x03\x00\x00\x00\x02\x00"
      "\x00\x00\x06\x00\x00\x00\x90\x02\x00\x00\x8a\x00\x04\x00\x00\x04\x00\x00"
      "\x02\x00\x03\x00\x02\x00\x91\x89\x00\x00\x01\x80\x09\x00\x00\x00\x01\x00"
      "\x00\x00\x01\x00\x00\x00\x04\x04\x02\x00\xff\x0f\x00\x00\x08\x00\x00\x00"
      "\x07\x00\x00\x00\xf3\x03\x00\x00\x3f\x00\x00\x00\xff\x7f\x00\x00\x00\x60"
      "\x00\x00\x00\x10\x00\x00\x06\x00\x00\x00\x81\x00\x00\x00\x02\x00\x00\x00"
      "\x00\x00\x01\x00\x09\x00\x00\x00\x81\x00\x00\x00\x00\x00\x00\x00\x04\x00"
      "\x00\x00\x03\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x05\x00\x00\x00"
      "\x01\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\xc3\x0c\x00\x00\x00\x08"
      "\x00\x00\x05\x00\x00\x00\x05\x00\x00\x00\x07\x00\x00\x00\xff\x01\x00\x00"
      "\x03\x00\x00\x00\x7d\x00\x00\x00\x78\x00\x00\x00\x01\x01\x00\x00\x08\x00"
      "\x00\x00\x09\x00\x00\x00\x80\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00"
      "\x05\x00\x00\x00\x01\x80\x00\x00\x00\x10\x00\x00\x1f\x00\x00\x00\x18\x00"
      "\x00\x00\x08\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00"
      "\x01\x00\x00\x80\x07\x00\x00\x00\x08\x00\x00\x00\x15\x97\xff\xff\x07\x00"
      "\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00\xda\x6a\x00\x00\xff\xff\xff\xff"
      "\x07\x00\x00\x00\x05\x00\x00\x00\x03\x00\x00\x00\x00\x01\x00\x00\x07\x00"
      "\x00\x00\x01\x00\x00\x00\x80\x00\x00\x00\x02\x00\x00\x00\x04\x00\x00\x00"
      "\x00\x10\x00\x00\x07\x00\x00\x00\x40\x00\x00\x00\x02\x00\x00\x00\x04\x00"
      "\x00\x00\xfc\x00\x00\x00\xff\x01\x00\x00\xff\xff\x00\x00\x00\x80\x00\x00"
      "\x08\x00\x00\x00\x03\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\xfc\xff"
      "\xff\xff\xcd\x4f\x00\x00\x01\x04\x00\x00\x06\x00\x00\x00\xff\xff\xff\xff"
      "\xff\xff\xff\xff\x05\x00\x00\x00\x03\x00\x00\x00\xc1\x17\x00\x00\x09\x00"
      "\x00\x00\x06\x00\x00\x00\x01\x00\x01\x00\x08\x00\x00\x00\x00\x00\x00\x30"
      "\x01\x01\x00\x00\x00\x04\x00\x00\x7c\x00\x00\x00\x01\x00\x00\x00\x48\x00"
      "\x00\x00\x04\x00\x00\x00\x08\x00\x00\x00\x07\x00\x00\x00\xff\x00\x00\x00"
      "\xc8\xdd\x50\x6b\x01\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x02\x00"
      "\x00\x00\x04\x00\x00\x00\x02\x00\x00\x00\xff\xff\xff\x7f\x07\x00\x00\x00"
      "\x00\x80\x00\x00\x01\x00\x01\x00\x06\x00\x00\x00\x09\x00\x00\x00\x00\x00"
      "\x00\x80\x07\x00\x00\x00\x01\x00\x00\x00\x03\x00\x00\x00\x9d\x0b\x00\x00"
      "\x00\x02\x00\x00\xfc\x00\x00\x00\x03\x00\x00\x00\x5c\x00\x00\x00\x05\x00"
      "\x00\x00\x03\x00\x00\x00\x80\xff\xff\xff\x07\x00\x00\x00\x06\x00\x00\x00"
      "\x04\x00\x00\x00\xaf\x02\x00\x00\x03\x00\x00\x00\x04\x00\x00\x00\x87\x00"
      "\x00\x00\x51\x00\x00\x00\x05\x00\x00\x00\xf7\xff\xff\xff\x05\x00\x00\x00"
      "\x08\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x09\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x80\x00\x00\x01\x00\x00\x80\xa7\xcd\x00\x00\x01\x00\x00\x00"
      "\x08\x00\x00\x00\x06\x00\x00\x00\x02\x00\x00\x00\xff\x03\x00\x00\x01\x04"
      "\x00\x00\x06\x00\x00\x00\xff\x07\x00\x00\x01\x00\x00\x00\x07\x00\x00\x00"
      "\xff\x0f\x00\x00\x08\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x46\x00"
      "\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00\x07\x00\x00\x00\x09\x00\x00\x00"
      "\xff\x01\x00\x00\x07\x00\x00\x00\x08\x00\x00\x00\x12\xf7\xff\xff\x07\x00"
      "\x00\x00\x82\x00\x00\x00\xff\x01\x00\x00\x03\x00\x00\x00\xe5\x04\x00\x00"
      "\xff\x07\x00\x00\x7f\x02\x00\x00\x3f\x00\x00\x00\x03\x00\x00\x00\x43\x1a"
      "\x00\x00\x29\x00\x00\x00\x80\xff\xff\xff\xfe\xff\xff\xff\x00\x04\x00\x00"
      "\x20\x00\x00\x00\x08\x00\x00\x00\x04\x00\x00\x00\x01\x00\x00\x00\x00\x00"
      "\x00\x00\x06\x00\x00\x00\x3f\x00\x00\x00\x06\x00\x00\x00\x92\x09\x00\x00"
      "\x00\x80\x00\x00\xff\x03\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\xff\x0f\x00\x00\x00\x02\x00\x00\x00\x04\x00\x00\x7f\x00\x00\x00"
      "\x00\x00\x00\x00\x29\x00\x00\x00\x40\x00\x00\x00\x00\x08\x00\x00\x00\x00"
      "\x00\x00\xff\xff\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\xe3\x25\x00\x00"
      "\x01\x00\x00\x00\xff\x7f\x00\x00\xc1\xff\xff\xff\xd9\x17\x00\x00\x80\xff"
      "\xff\xff\xb9\x00\x00\x00\x00\x00\x00\x00\xfc\xff\x01\x00\x08\x00\x00\x00"
      "\xa8\x07\x00\x00\x80\x00\x00\x00\x06\x00\x00\x00\x08\x00\x00\x00\x02\x00"
      "\x00\x00\x02\x00\x00\x00\x01\x00\x00\x00\xf1\x00\x00\x00\x06\x00\x00\x00"
      "\xf4\x03\x00\x00\x0d\x00\x00\x00\xfb\xff\xff\xff\x02\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x01\x00\x81\x00\x00\x00\x00\x00\x00\x00\x3f\x00\x00\x00"
      "\x00\x00\x00\x00\xe3\x0b\x00\x00\x03\x00\x00\x00\xff\x0f\x00\x00\x7f\x00"
      "\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00\xff\x7f\x00\x00\x07\x00\x00\x00"
      "\x3f\x00\x00\x00\xff\xff\xff\x7f\x00\x08\x00\x00\x06\x00\x00\x00\x01\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x20\x00\x00\x00\x16\x0a\x00\x00"
      "\x00\x00\x00\x00\x05\x00\x00\x00\x3c\x00\x01\x00\x04\x00\x00\x00\x00\x00"
      "\x00\x00\xff\x00\x00\x00\xd4\xe0\x27\x41\x07\x00\x00\x00\x5d\x00\x02\x00"
      "\x03\x00\x1e\xf7\x80\x00\x00\x00\x01\x01\x07\x00\x03\x00\x07\x00\xab\x0e"
      "\x00\x00\xff\xff\xff\xff\xb5\x00\x00\x00\x03\x00\x00\x00\x08\x00\x05\x00"
      "\x4c\x06\x00\x00\x08\x00\x01\x00\x03\x00\x05\x00\x08\x00\x01\x00\x0f",
      2789);
  *(uint64_t*)0x20000188 = 0xc7c;
  *(uint64_t*)0x20000218 = 1;
  *(uint64_t*)0x20000220 = 0;
  *(uint64_t*)0x20000228 = 0;
  *(uint32_t*)0x20000230 = 0;
  syscall(__NR_sendmsg, -1, 0x20000200ul, 0ul);
  res = syscall(__NR_socket, 0x1000000010ul, 0x80002ul, 0);
  if (res != -1)
    r[2] = res;
  syscall(__NR_sendmmsg, r[2], 0x20000200ul, 0x4924924924926d3ul, 0ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  do_sandbox_none();
  return 0;
}
