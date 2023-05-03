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

#ifndef __NR_getsockname
#define __NR_getsockname 367
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_sendmmsg
#define __NR_sendmmsg 345
#endif
#ifndef __NR_sendmsg
#define __NR_sendmsg 370
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10, 0x803, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000280 = 0;
  *(uint32_t*)0x20000284 = 0;
  *(uint32_t*)0x20000288 = 0x20000180;
  *(uint32_t*)0x20000180 = 0;
  *(uint32_t*)0x20000184 = 0;
  *(uint32_t*)0x2000028c = 1;
  *(uint32_t*)0x20000290 = 0;
  *(uint32_t*)0x20000294 = 0;
  *(uint32_t*)0x20000298 = 0;
  syscall(__NR_sendmsg, (intptr_t)r[0], 0x20000280, 0);
  *(uint32_t*)0x200002c0 = 0x14;
  res = syscall(__NR_getsockname, (intptr_t)r[0], 0x20000100, 0x200002c0);
  if (res != -1)
    r[1] = *(uint32_t*)0x20000104;
  *(uint32_t*)0x20000240 = 0;
  *(uint32_t*)0x20000244 = 0;
  *(uint32_t*)0x20000248 = 0x20000140;
  *(uint32_t*)0x20000140 = 0x200014c0;
  memcpy((void*)0x200014c0, "\x38\x00\x00\x00\x24\x00\x07\x05\x00\x00\x00\x40"
                            "\x07\xa2\xa3\x00\x05\x00\x00\x00",
         20);
  *(uint32_t*)0x200014d4 = r[1];
  memcpy((void*)0x200014d8, "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00"
                            "\x09\x00\x01\x00\x68\x66\x73\x63\x00\x00\x00\x00"
                            "\x08\x00\x02",
         27);
  *(uint32_t*)0x20000144 = 0x38;
  *(uint32_t*)0x2000024c = 1;
  *(uint32_t*)0x20000250 = 0;
  *(uint32_t*)0x20000254 = 0;
  *(uint32_t*)0x20000258 = 0;
  syscall(__NR_sendmsg, -1, 0x20000240, 0);
  *(uint32_t*)0x20000380 = 0;
  *(uint32_t*)0x20000384 = 0;
  *(uint32_t*)0x20000388 = 0x20000180;
  *(uint32_t*)0x20000180 = 0x20003d40;
  memcpy(
      (void*)0x20003d40,
      "\\\'\000\000,\000\'\r\000\000\000\000\000\000\000\000\000\000\000\000",
      20);
  *(uint32_t*)0x20003d54 = r[1];
  memcpy(
      (void*)0x20003d58,
      "\x00\x00\x00\x00\x00\x00\x00\x00\x0c\x00\x00\x00\x06\x00\x05\x00\x00\x00"
      "\x00\x00\x0a\x00\x01\x00\x72\x73\x76\x70\x36\x00\x00\x00\x24\x27\x02\x00"
      "\x4c\x12\x06\x00\x48\x00\x0d\x00\x09\x00\x01\x00\x6d\x70\x6c\x73\x00\x00"
      "\x00\x00\x0c\x00\x02\x80\x08\x00\x05\x00\x17\xca\x03\x00\x13\x00\x06\x00"
      "\xaf\x71\xea\x2a\x54\xbb\xe5\x05\x3c\x38\x19\x88\xc0\x7e\x4b\x00\x0c\x00"
      "\x07\x00\x01\x00\x00\x00\x01\x00\x00\x00\x0c\x00\x08\x00\x02\x00\x00\x00"
      "\x01\x00\x00\x00\xac\x00\x1e\x00\x0b\x00\x01\x00\x73\x69\x6d\x70\x6c\x65"
      "\x00\x00\x70\x00\x02\x80\x0b\x00\x03\x00\x66\x6c\x6f\x77\x65\x72\x00\x00"
      "\x0b\x00\x03\x00\x66\x6c\x6f\x77\x65\x72\x00\x00\x18\x00\x02\x00\x02\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x20\x04\x00\x00\x00\xda\x0e\x00\x00"
      "\x0b\x00\x03\x00\x66\x6c\x6f\x77\x65\x72\x00\x00\x18\x00\x02\x00\x04\x00"
      "\x00\x00\x08\x00\x00\x00\x02\x00\x00\x00\x00\x01\x00\x00\x07\x00\x00\x00"
      "\x0b\x00\x03\x00\x66\x6c\x6f\x77\x65\x72\x00\x00\x0b\x00\x03\x00\x66\x6c"
      "\x6f\x77\x65\x72\x00\x00\x12\x00\x06\x00\x9f\x12\xdc\x7e\x65\x3e\x55\xfa"
      "\x71\x84\xbe\x81\x07\x92\x00\x00\x0c\x00\x07\x00\x00\x00\x00\x00\x01\x00"
      "\x00\x00\x0c\x00\x08\x00\x02\x00\x00\x00\x01\x00\x00\x00\x54\x11\x02\x00"
      "\x07\x00\x01\x00\x78\x74\x00\x00\x2c\x11\x02\x80\x2a\x10\x06\x00\xfa\xf5"
      "\x6e\x61\x74\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0\x00\x00\x00"
      "\xd1\x2e\xb3\x9b\x80\x68\xea\x6c\x32\x6b\x7e\xba\x5a\xce\x51\x95\x6a\x81"
      "\x63\xbf\xf5\xb0\x71\xd4\x65\x2f\x66\x16\x82\xf3\x09\x43\xd0\x52\x76\xbb"
      "\xef\x81\xf1\xf5\x2c\xba\xab\x7e\x51\xf4\x0f\x8e\x9b\x4f\x2a\x54\xaf\x6e"
      "\x75\x65\x60\xae\x1b\x00\x86\xd9\x09\x6b\x5e\xc2\xeb\xdf\x1f\x23\x43\xcd"
      "\xeb\xc2\x93\x13\x76\xbe\x3a\x0c\x36\xed\x2d\x06\x03\x96\xb8\x0a\x6e\x23"
      "\x82\x5b\xce\x6d\xd4\x52\xb6\xd4\x24\x9a\xde\x5a\xeb\xbc\x46\xe1\xd1\x26"
      "\x2b\x37\xa5\xed\x3a\x5e\x7c\x2e\x4b\x42\xb7\x2a\x70\xb9\xa7\xef\xec\x12"
      "\x4b\xad\x7f\x57\xac\x23\x47\x1f\xf3\x0b\x73\x03\x5b\x5f\xe5\x8b\xb5\x32"
      "\xc9\x41\x03\x84\x85\x1c\xeb\x06\xf6\x56\x5a\x97\x34\xe3\xe2\x29\xcc\xf3"
      "\x23\x65\xb7\xf4\x11\xe0\x58\x0e\xcc\x8c\x09\xd7\x96\xff\x8e\x32\x3c\xf7"
      "\x60\x2c\xa2\x7c\xa1\xc8\xaf\xb7\x20\x79\xe6\x89\xd9\xef\x6f\xa0\x3e\x1c"
      "\x1b\xd6\x4e\x7b\x3b\xad\x36\x07\xb1\xda\x8c\xdc\x85\x03\xbe\xa3\xaf\xba"
      "\x07\x3b\x6e\xe9\xc4\xa6\x0c\x03\xa2\xa6\xd8\xcb\xcc\xb1\xab\x36\xbf\x8e"
      "\x2a\xb2\x79\x3c\x02\x6e\xff\x2e\x71\x47\xf4\x86\x9f\xb1\x09\x23\x37\xa7"
      "\x9d\x83\xe8\x6e\xe4\x6e\x13\xae\x32\x95\x19\xf4\xa7\xf7\x7f\x74\x41\x65"
      "\x3d\x65\xcb\x03\x1e\x38\xf5\x9e\x0a\x40\x31\x3c\x0e\x84\x43\x6c\x7b\xb4"
      "\xbe\xb6\x84\xe2\xa9\xf4\xae\x63\xba\x6c\x93\x45\x88\xec\x29\x6a\x53\x0b"
      "\x9f\x54\x52\x52\x62\x1d\x28\xec\x9a\xc3\x8a\x3a\x4d\xf9\x9e\x4a\xc1\x14"
      "\x7f\x03\x2a\x5b\x6b\x9c\xe6\x34\x88\x46\x57\x54\x72\xc8\xc3\x61\x9a\x35"
      "\xca\x77\xfc\x41\xd2\x04\xc8\xf0\xba\xbf\x2a\x2d\x45\x8d\xab\x72\x90\x75"
      "\x4e\x60\xe3\xe3\xc8\xc2\x0a\x23\x69\xa9\xe8\x23\x12\xca\x78\x03\xff\xe7"
      "\xd3\x71\x73\xf2\x5c\x48\x28\x3d\xaf\xf2\xfd\xde\xc6\x1e\x27\x69\x6d\xe1"
      "\xe7\x6d\xed\x2c\xa6\x42\x27\xa9\x58\x45\x5a\x63\x09\xa3\x3b\x69\x5f\xc6"
      "\xc5\x99\xd4\x7f\x97\x72\xea\xe5\xb5\x61\x86\x58\x61\xf5\xc2\x9f\x00\x00"
      "\xf4\x77\xcb\x19\x06\x1c\xf3\x84\x9e\x34\x9c\x23\xa4\x46\xff\xeb\xb2\x3e"
      "\x16\x25\x30\xfb\x10\x95\xa7\xc8\x92\x91\x26\xa3\x11\x7b\xb4\x06\xbf\x2d"
      "\x3f\x22\x50\x07\x28\xae\xd1\xb2\xae\x2e\x3e\x3f\x3d\xdc\x5d\xff\xcb\xad"
      "\xb2\x4e\xcc\xe3\xba\xd4\x28\x1a\xa1\x7e\x87\x60\x27\xdf\x5e\x95\x0f\x3d"
      "\xe3\x36\xe1\x3e\xf7\x89\x85\xb5\x5d\x52\xfa\x77\x5c\xe6\x85\xe1\x83\x3f"
      "\x35\xa4\x63\x30\xb8\xe6\x53\xc0\xed\xd1\xd9\xb7\x8f\xad\x65\x7f\xc0\xb6"
      "\xac\xde\x9b\x14\x15\xa8\x37\xd1\xac\x8e\x44\x6c\x39\x44\xd4\xb0\xe6\x55"
      "\xa5\x89\x9f\xcc\x07\x52\x11\x3f\xd3\xae\x09\xa6\x0c\xbc\x4f\xc0\xa6\xa6"
      "\x78\xf4\xe7\x1e\x08\xd4\x15\x7a\x2d\x1d\xa9\x16\x00\xda\x85\x55\x99\x6b"
      "\xe8\x15\x93\x7a\x5c\xc9\x9a\xe2\x6f\xae\x81\x5f\x69\x29\x83\x25\x64\x43"
      "\xc9\x84\xa1\xfd\xe3\x7b\xfb\x2d\xd3\x19\x85\x24\x41\x8e\xec\x3b\x3e\x1d"
      "\x46\xf5\xa4\xb5\x76\x65\xd6\xd2\x05\xda\x75\xe0\x65\xd8\xa5\x03\xc0\xa5"
      "\xbe\xaa\x66\x24\x66\x82\x81\xf8\x98\x53\x92\x5f\x61\x73\x43\x26\xcd\x42"
      "\x6a\xf3\xba\xf0\xd9\x4b\xef\x92\x21\x47\xcd\x9b\x12\x46\x26\x9c\xb6\xb2"
      "\x27\x99\xd2\xfc\x86\x01\x85\x9b\x08\xa3\xef\x24\x85\x09\xb3\x30\x63\x96"
      "\xf1\xb7\xd4\x19\x11\xaa\x73\xf1\xd9\x39\x66\x70\x5b\xc7\x5b\x4a\x34\x3a"
      "\x49\x2f\x6c\x62\x68\xe7\x26\x80\x4f\x4e\xb7\x7f\xa4\x71\x50\x91\xd3\x61"
      "\x29\x07\x4d\x33\x2e\xc3\x0b\x83\x0d\xb6\x3e\x63\x61\x07\x81\x41\x60\xe2"
      "\x1b\xb7\xb3\xa9\xa5\x27\x7b\xee\xb7\x0a\x44\x6d\x71\xab\xc5\xdf\xd4\x6f"
      "\x76\x67\x4b\xb0\xc3\xeb\x15\x8e\x88\x13\xe8\x99\x4a\x5a\xa3\xfe\x62\x27"
      "\xaa\x33\xf6\x56\x1e\x4a\xba\x91\xd5\xc3\x61\x2b\xed\xf4\x8d\xdd\x29\xa1"
      "\x2c\xf7\xbd\x5e\xb8\x27\xf0\x9b\xab\xf5\x75\x5c\x48\x4b\xa7\xa8\x2e\x99"
      "\x57\xf3\x58\x47\x30\x77\x98\xd4\x04\xbd\x76\x92\x3d\x33\x59\x86\xf8\x51"
      "\x2f\xa5\xd5\xea\x3b\xdb\xa4\xa7\x86\x1f\x9d\x65\xc5\x46\x9a\xe9\x49\x69"
      "\xc3\xad\x19\x1a\xaa\xac\x6e\x03\xc1\x19\x57\xb1\x13\xbf\xc5\x34\x2c\xa1"
      "\x5e\x59\xe2\xd3\xc4\xd2\x39\xf1\xff\x3e\x99\x4f\xce\x44\xdb\xc5\x71\x96"
      "\x2f\xec\x80\x00\x62\x38\xfa\xfe\x51\x00\x3c\x59\xe6\x15\x35\xb7\x2a\x07"
      "\x21\xe8\x92\x58\x6e\xf0\x5e\xb3\x7b\x1f\x43\x5a\x68\x12\x63\x8b\xce\xaf"
      "\xc3\xb4\x76\xf2\x3a\x12\x45\x4b\x14\xb2\xf3\xbc\x7c\x98\x12\x0f\x87\xf8"
      "\xa1\x7f\xa3\xe2\x59\xfd\x29\xcc\x8f\x31\x0d\x99\x18\x3d\x84\x68\xec\x7c"
      "\xf2\x6e\xb9\x5b\x60\xbd\x3d\x24\x74\xec\xf4\xd8\x21\x39\xe1\xcf\xba\xb8"
      "\x93\x5d\x78\xf9\x21\x6a\x40\x2e\x05\x4d\xea\xbc\x2f\x9f\xed\x91\x9e\xf7"
      "\xd4\xfa\x0b\x82\x4c\xa7\x96\x37\x67\x46\xed\x6c\x7c\x04\xfd\x35\x87\x19"
      "\xb3\x4b\xc7\xd1\x19\x3a\x01\x06\x1c\x3d\x8e\xb7\x0a\x39\xe4\x3c\xc4\xcc"
      "\xfb\xc7\xf3\x14\x9c\xe7\xd3\x67\x31\x14\x7a\x3b\x52\xff\xbf\x7a\x56\xb3"
      "\x94\x09\xef\x3d\xc4\x1d\xf9\x30\xc5\x6f\x98\x95\xf0\xa0\x4e\x3e\xfc\xb6"
      "\x8b\xfa\x54\xda\x25\xfc\x9e\xf6\x03\x4d\x9a\x77\xa3\x27\xaa\x15\x2f\x62"
      "\x7c\x56\xb4\xf8\x71\xdf\xa7\x57\x57\xf6\x6a\x13\xfa\x59\xc8\x1e\x71\x51"
      "\x0f\x7e\x07\x8c\xf8\xcf\x61\x97\xc1\x2a\x89\x09\x5e\xd2\x0a\x28\x77\x52"
      "\xcf\x9c\x2a\xb1\x7e\xf3\xd2\x39\x59\xd3\x83\x81\xd0\x04\x8f\xae\xd3\xdb"
      "\xdd\x63\x72\x0b\xe7\x26\x3f\xfe\x17\x0e\xa2\xe6\x93\x3f\xd8\x6d\x10\xe3"
      "\x17\x82\xd5\x03\x9a\x89\xa7\x6f\xf4\x49\xbf\xf8\xa7\xb4\x4d\xcd\x33\x3d"
      "\x8c\x3f\x81\x4f\x06\x3b\x6e\x6f\xcb\xe3\x09\x15\x8a\x2b\x6f\xfb\x8c\xbf"
      "\x55\x69\x91\x32\x62\xf8\x52\xa3\x3b\xc8\x70\x78\xf5\xbd\x40\x1e\x95\xf5"
      "\x8a\x83\x26\x75\x98\x28\x06\x9a\x8c\x16\x08\xcd\x4e\x53\x8f\xbd\xf3\xfa"
      "\xce\xc3\x4e\xf7\x49\x2f\x3b\xc5\x95\xe2\x27\x34\x92\xb2\x83\x87\x5e\x47"
      "\x51\x3b\xb1\x2b\xb0\x0a\x58\x83\xdd\x84\x55\xd2\x8f\x54\xb0\xea\x8f\xc6"
      "\xdd\x8d\x89\xf6\x42\xd8\x1d\x5f\xed\xbc\xd4\x1d\xf9\x14\x06\x4f\x00\x84"
      "\x37\x0d\xb5\xbc\xee\xb7\xf5\x9e\x43\xb5\x37\x80\xd3\x7f\x6e\xdf\x53\xf6"
      "\xe7\x38\xbd\xd4\x01\x59\x84\xf8\xaf\x2f\xb2\x66\x4e\xc4\x6a\xbf\x61\x27"
      "\x61\x7c\x34\xe3\x58\xa7\xbc\x69\x8f\x4a\xc2\x99\xd1\xc2\x9d\x39\xd4\x7c"
      "\x02\x07\x3a\x99\xb1\x92\x3e\x87\x04\x20\x38\x64\x30\xb6\x2e\x57\xd0\x4f"
      "\x3c\xaa\x50\x33\x60\xf9\x66\xe2\x8b\x6d\x36\x2b\x65\xe0\x7b\x46\x09\x8b"
      "\x09\x86\xe0\x98\x92\xef\x25\x42\x2a\x05\x96\x24\x51\x33\x3c\x55\x29\x28"
      "\x73\x32\xe3\xa5\x5f\x8b\xdc\x6c\xe8\x60\x16\x83\x74\xb0\x78\x8e\x62\x47"
      "\xd6\xc4\x77\x89\x59\xe8\x67\x2f\x72\xe7\xa6\x45\xa7\xf9\xd9\xeb\x86\x09"
      "\xe5\xa8\xb8\x1b\x40\x4b\x95\xd7\xbd\x00\xf5\xed\x9b\xef\x9d\x7c\x9f\x89"
      "\xa9\xd8\xe4\x12\x5e\x1e\xb4\xcb\x82\x08\x58\x14\x4d\xcc\xc3\xa4\x46\x1a"
      "\xa7\xfa\xc3\x17\x83\xe8\xe3\x75\xb7\x18\xd8\x67\x6a\xcf\xd1\xd8\x90\x05"
      "\xf0\xcf\xe7\x8a\xc1\xc0\xc4\x66\x6d\xe2\x5f\xfd\xad\x38\xb9\xcf\x54\xf1"
      "\xf6\xed\xf9\x20\xc4\x8b\x51\x2f\x45\x70\x7d\xbf\xea\x49\x1c\x24\x0a\xf2"
      "\x78\xbc\xfb\xcd\x5b\xf4\x86\x19\x1d\xbb\x0f\xac\x33\x18\x87\x06\x47\xea"
      "\xfc\xd5\x11\xf1\x40\x7d\xb2\xbe\x70\x30\x6e\x75\xb5\x75\x9c\x9c\xf7\x60"
      "\x4d\x29\x51\xc6\x3e\xc2\x56\x07\xf5\x7a\x07\x2f\x32\x29\xee\x8d\xdb\xcf"
      "\x9d\x90\x1a\x5b\x24\xb2\xad\x22\x93\x66\xc7\x6a\xe7\x45\xc3\xc6\xbd\x8c"
      "\x29\xa1\x56\x67\xca\x1f\xa0\x88\x46\xa9\xfb\x2d\x41\x96\x94\xdc\x44\xea"
      "\xe4\x6a\xb4\x69\x4e\x28\x06\x2e\x96\x3b\xbd\x8c\x39\xec\xaf\xf8\x87\x47"
      "\x91\xd2\x56\x59\xe7\xdf\xeb\x6b\x32\x4b\x1f\x4f\x3a\x32\xf2\x59\xf4\x17"
      "\x2d\xdd\xf3\x3b\x9b\x5a\xd3\xe4\xef\xe5\xbd\x80\xb9\x2b\x2e\x41\x70\x01"
      "\x72\x22\x58\x39\xa1\x7b\x3b\x4f\x2c\x2b\xaa\xe4\x17\xa2\x7d\xc9\x8a\x9e"
      "\x8b\x00\x3e\xda\x12\x11\x00\xfc\x0f\x11\x45\xa1\x49\x23\x9c\x5c\xf2\x4b"
      "\xcb\x2e\x60\x3c\x9e\x22\x2e\x3a\xa5\x5d\xf6\x2e\x52\x04\x6c\xa2\x7b\x96"
      "\x4b\x00\x21\x6b\x03\x6d\x1c\xbd\xf1\xbe\xf4\xf9\x5b\x27\xb1\x77\x7d\x41"
      "\x97\x18\x54\x01\xce\x46\x99\x53\xed\x72\x24\x8b\xf4\xb6\xcb\xdc\xda\xb9"
      "\xff\xd0\x04\x07\xdd\x8a\x71\x55\xff\x9d\xd4\x7d\xc5\x77\x80\x83\x3b\x54"
      "\x59\xfb\x60\xea\x37\xb9\x18\x6b\x40\x9e\xfd\x78\xae\x09\xff\x6b\x98\x23"
      "\x60\x79\x61\x44\xa0\x04\xe3\xe5\x1f\xf6\x14\xaa\x7b\x64\x16\xc5\x4a\xe9"
      "\xff\x9a\x8e\x38\x9d\x82\x6a\x36\x3a\xf3\xdc\x06\x25\xf9\xf8\xdc\xea\x35"
      "\xc5\xfe\xfa\xda\x40\x59\x93\x8d\x16\xe5\xc5\xe8\x75\xd6\xf3\x81\x21\x37"
      "\xbd\x16\x94\x32\x73\x42\x90\xb6\x65\x31\xaa\xf1\xae\x19\xa5\x98\x71\x74"
      "\xd1\x16\x10\x07\x26\xab\x3a\x5a\x74\x50\x72\xd6\x6f\xd8\xbb\xcb\x8c\xd6"
      "\xb4\x61\x49\xd2\x27\x88\x86\x47\x3c\x0d\x7e\x96\xc0\xf4\x99\xef\xa9\x9e"
      "\xaf\x7d\xe6\xd4\x87\x8d\xd9\xbb\x5f\x5a\x4b\x86\x41\x07\x88\xf7\x07\xf3"
      "\xb3\x79\x10\xdd\x0d\xa5\xa1\xea\x1e\x3e\xa4\x50\x0c\x1f\xfd\xed\x0f\x86"
      "\xe8\x2d\x6b\x01\x6d\x1b\xd2\x72\x3e\xd8\x8d\x00\xa7\xac\xec\x51\x3c\x2f"
      "\x5b\xc9\x26\x8f\x20\x76\x3e\x42\xc8\x2f\x53\xe0\x61\x82\x4b\x9a\x57\xfd"
      "\x55\x59\x2e\x57\x9c\x97\x67\x04\x02\xaa\x80\xaf\xe8\x00\xb5\x32\x9c\xfe"
      "\x92\xa1\xd9\x75\x06\xb7\x90\x55\xa1\x59\x12\x1a\xe7\x67\x2b\x3f\xcd\x10"
      "\x03\x5a\xae\x80\x78\x6b\x84\x43\xf6\x12\xe1\x4f\xba\xfb\x4f\x6b\x3f\x24"
      "\xc5\x41\xe9\x71\xb7\xab\x4d\x84\x83\x4a\x41\x86\xa6\xba\x26\x7d\x92\xc0"
      "\xcf\xd0\x2b\x16\xfc\x56\xd8\xd3\xa8\xb6\xe7\xdf\x27\xa6\x51\x92\x1c\x5e"
      "\x8a\x4d\xee\xe2\x82\xb9\x59\xee\xd1\xa6\x8a\x65\x10\xbc\xa4\x3e\x7b\x99"
      "\xae\x2e\x5f\xb6\xb2\x67\x9f\x07\x51\x3c\x0e\xcb\xf9\xcd\xd5\x0d\x83\xb9"
      "\x9c\xaf\x9c\x32\xe0\x5a\xd7\xed\x92\x4d\x5d\x6a\xd7\xbb\xd6\x6e\xf6\xb7"
      "\x0c\x72\x67\x02\x87\x12\xf2\x5d\x8c\xac\xdb\x70\x23\x23\xae\xa9\xe6\x01"
      "\x8d\xf7\x8d\xef\xd2\x1f\xb3\x59\xc4\xc1\x61\xbc\x3b\x6f\xab\x81\xca\xfb"
      "\x49\x3e\x94\x24\x91\xb8\x48\xdd\xfe\x23\xe4\x34\x6d\xcb\xc0\x11\xbb\xd8"
      "\xb1\xff\xa5\xa4\xf7\xee\x04\xb9\xb6\xc8\x49\x3a\x29\xe8\x16\x5c\x62\xac"
      "\xaa\xb5\x15\x46\xe0\xd7\x47\xf4\xea\x65\xb1\xdc\xca\xcd\xf2\x63\x86\xc7"
      "\x0d\xaf\x0f\x2f\xd6\x66\x2f\xaf\xb1\x66\x4f\x82\xc3\x72\x6b\x5c\x16\x3b"
      "\x79\x43\x5d\xf2\x05\xe3\x1c\x5a\x63\xee\x3b\x4f\x77\xe4\xb2\xb7\xdb\x33"
      "\x79\x42\x69\x88\xa4\x4e\xa7\xd6\xca\x5a\x5d\xf7\xc1\xae\x8c\xdc\x3c\x50"
      "\x8c\x70\xe3\x81\x86\x96\xf5\xbf\xe3\xbe\x96\x12\x5b\x84\x3d\x40\xa0\xcd"
      "\xbb\x08\x60\x30\x85\x59\x64\x8f\x0e\x02\x0c\x81\xb1\x6c\x50\xf8\xd5\x61"
      "\x7d\x3f\x21\xd4\x53\x2e\x12\x01\xce\xea\xe1\x76\x96\x0c\xb4\x07\xf6\xc5"
      "\xa0\xc1\xfb\x74\x2e\x22\x44\x4e\x5a\x44\x76\x23\x4c\x32\xe0\x49\x12\x55"
      "\x00\x38\x1b\x7c\x0c\xe2\xdc\xde\xe1\x0a\x66\x4e\xe7\x22\x7e\x64\xbd\x73"
      "\x85\x15\xed\x9b\xa0\x83\x41\xa6\x1d\x6a\xe4\x2c\xf5\x0d\x9b\xfd\xd5\xb2"
      "\x97\xf4\x0f\x9c\x4c\x71\xaf\xa6\xd1\x57\x70\xce\xa1\xb6\x65\x37\xa6\x0e"
      "\x27\x0f\x22\x4c\x10\x64\x7b\x36\x1c\xf2\xc9\x24\xd0\x3b\xe9\x85\x67\x4e"
      "\xae\xc9\xe7\xad\xed\xa2\x99\x4d\x83\xc9\x1f\x0f\xab\x60\x56\xc9\xa7\x49"
      "\x04\x35\x31\x84\x32\x71\x2a\xb7\xb1\x40\xb9\xe9\x59\xa9\x32\x68\xa5\x00"
      "\x6b\x4f\x66\xc7\xe4\x6a\x8c\x6c\xc4\xf7\x3b\xd8\x00\x07\x51\xbe\x1b\x8a"
      "\x9f\xf2\x63\x0f\xe6\xc0\x37\x8e\xf0\xed\x8d\x5b\xd9\x83\xf3\xdb\xd5\x18"
      "\x0e\xc1\x22\x82\x00\x81\x05\x46\x3f\xea\x7b\x3e\x3c\x03\xe3\x0c\x45\x93"
      "\xe4\x9c\x83\xb6\x50\xdc\x88\xc5\xd8\xb5\x49\x0c\x8c\xea\xcd\x5d\xb9\x3a"
      "\x45\xb5\xef\xd8\x9d\xb4\x0e\xe6\x55\x2f\xce\xd1\x4d\x10\x70\xd1\x8c\x99"
      "\x81\xdd\x3a\xab\xae\x87\xd2\x93\xb6\xf3\xad\xf2\x45\x7f\x3f\xc6\x85\xab"
      "\x5b\x72\x40\xf9\xe1\xed\x31\x2d\x06\x51\xe1\x4a\x31\x68\x4b\xd7\xfc\x70"
      "\x6f\x14\xa4\x45\x6e\x74\xb4\xb5\xb0\x5e\x9b\x26\xe8\x36\xcf\xc4\x64\x51"
      "\x9e\x5e\xc9\xa2\x6b\x7c\x24\x17\x22\xf9\x80\xf7\x43\x08\xc5\x26\xa7\xe6"
      "\x01\xa2\xaa\xa2\x92\x2d\x97\xf3\x4a\x07\x1d\xaf\x1b\x69\xa9\x95\xa2\xf6"
      "\xcf\x42\xb6\x7c\x27\xcf\xeb\x5a\x7e\x90\x12\x5c\x8f\xa9\x6d\xde\x7e\x53"
      "\xa9\x23\x37\x1b\x3f\x50\x22\xf3\x2b\xd6\x23\x64\x18\x40\x9f\xcd\xcc\x61"
      "\x8e\x7e\xe1\xcf\xc5\x09\x2c\x38\x0b\xcd\xc9\x38\x50\x54\x72\xbe\x1e\x6c"
      "\x3f\xc0\x3c\x7f\x6f\x71\x13\xfe\x5f\x3e\x8c\xff\x63\xcd\x3d\x0a\x16\x08"
      "\x31\x47\x51\x55\x71\x40\x4f\x2b\x3e\x00\xc4\x34\xfa\x56\x98\xf5\xdb\x12"
      "\xc2\x7d\xee\xfd\x9d\xc3\x32\xbb\x26\x63\xdb\xba\xae\x47\x4a\x67\x09\xd9"
      "\xd0\x0e\xe7\x2b\x9f\x3e\x7f\x0a\xe3\xe5\x38\xd7\xef\x1b\x53\x67\x72\xc5"
      "\xfa\x88\x72\xd3\x82\x83\x3e\x3b\xf1\xb1\xc8\x0a\xe6\xc4\x7d\xd0\x28\x75"
      "\xc4\x8d\x70\xe3\xc0\x67\xea\xbe\xa8\x49\xd6\x93\x8e\xfb\x0b\x32\x8f\xc4"
      "\x4b\xbd\xf4\xfc\x7d\x38\x13\xc6\x9d\x25\x42\xb8\xdb\x78\x86\x63\x56\x51"
      "\xa9\xd4\x8e\x6b\x43\xec\x88\x79\x55\xc4\xf6\x3c\xbb\xba\xa1\x54\xae\x16"
      "\x2d\xb4\x5a\x0c\xd8\x8f\x7b\xa4\x60\x9d\x89\x1c\x8d\x5a\x8e\x8f\x5a\x50"
      "\xbb\x18\xca\xd2\x03\xdf\x95\xe5\x59\x67\x6c\x8c\x91\x15\x68\x51\xbf\x73"
      "\x12\x0b\x27\x69\xb8\x27\x99\x2d\x2b\xb0\xf3\x5e\x4a\x0d\x67\x26\x8a\x4c"
      "\xbc\x9d\xa6\xe4\x60\x40\xb4\xed\xb2\xd5\x33\xea\xd5\xd8\xb8\x7b\x9a\x2e"
      "\xef\x07\x86\xdd\x44\xc3\xfd\xd8\x3c\x3b\x53\x54\xf2\x8f\x9c\xb0\x7e\xc1"
      "\x0c\x91\x55\x77\x0d\x18\x91\xd6\x5d\xd7\xfa\x22\x9b\xc3\x25\x13\xb4\xcd"
      "\xf0\x10\xaf\xe8\xc2\x57\xf3\x12\x3c\x0c\x82\x1d\x22\xc9\x71\x6c\xd0\x7e"
      "\xa1\xfa\x4a\x20\xca\xa2\xce\x57\x94\x91\x62\x1d\xb1\x11\x14\x50\x69\xdd"
      "\x8e\x58\xaf\x4e\xb1\x35\x81\x66\x4e\xeb\x29\x13\xb2\x5c\x29\x09\xf3\xe5"
      "\x66\x8b\xb8\x94\xd5\x5f\xcb\x3e\x79\x97\xf3\xcc\x71\x02\xc9\xd1\x93\x5b"
      "\xcb\x85\x0f\xec\x00\xb1\x50\x70\x7a\x69\x8f\x4a\x34\xe6\xa1\xd6\xc8\x40"
      "\x0a\x0b\x13\x7e\x52\x91\xe2\x2d\xe3\x94\x05\x98\xfb\xcc\x3f\x18\x49\xaf"
      "\x42\x53\x2d\x75\xcd\x2a\xcd\x19\x11\x1c\x26\x4e\x4a\x41\xe3\xf4\xc2\xf0"
      "\x9d\x1c\x31\x0f\xb5\x2f\xd4\x7d\x3f\x64\x84\xb9\xaf\xe2\xaa\x3f\x87\x6d"
      "\x23\x8e\xdf\x9a\xc2\x1c\xc7\x1d\x6f\x7c\x54\x2a\xdd\x05\xfe\xdd\x3c\xa1"
      "\xd5\xb4\x5d\x6d\x9f\xd5\x4a\x15\x93\x5f\xb1\xa5\x30\x6e\xb9\x10\x11\xdf"
      "\x6b\xe3\x15\x93\xce\x14\xde\x8f\xc9\x62\x43\x12\xa1\xb2\x41\x80\xee\x06"
      "\x06\xce\x59\x68\x3a\xf5\x3a\x05\x37\x0a\xff\xb7\x4a\x75\x3c\x85\xb9\x16"
      "\xae\x5c\xdf\x72\x91\x53\xd6\xa5\xbf\xf4\xb4\x37\x80\x8f\x5d\x16\xd2\x99"
      "\x69\xcc\x5c\xf5\x59\x67\x28\x76\x2b\x48\xaf\x38\xab\xc2\x68\x96\xd4\x0a"
      "\x25\x33\x76\xbd\xef\xf0\x9d\xf5\x4f\x61\x9a\xc1\x28\x99\x80\x65\x66\x91"
      "\x8f\xd8\xe5\x12\x0a\x30\xe0\x06\x60\xb8\x38\xe3\xf3\x83\xc0\xcd\x6c\x03"
      "\x99\x68\x72\xec\x02\x67\xcf\xe5\x0c\x77\xed\x36\x88\x32\x5f\x67\x8d\x3c"
      "\x1f\x54\x38\x0d\xe8\xc1\xab\x41\xdf\xef\x44\xcc\x3e\xd2\x36\xe9\x77\x68"
      "\x5a\x16\xd8\x91\x62\xdc\xf8\xee\x9f\x6a\x0c\x45\xf9\x81\x4a\x52\xd2\x8f"
      "\xda\xc0\x94\xd7\x52\xd8\x24\x50\x05\xc3\x2f\x98\xd6\xcf\x0b\x71\x66\x11"
      "\x47\xdb\x3e\x69\x49\x9f\x7e\xb5\x5e\x1f\x38\xfa\x9d\x36\xdf\x4c\x22\xb1"
      "\x58\x98\xdb\x80\x53\xf2\xaa\xc7\x81\x66\x0f\xed\x2f\xe9\xcb\x38\x3f\x80"
      "\xb7\xf5\xab\xc3\x7f\x0c\xca\x64\x76\x1e\xe5\x49\x3d\x55\xc7\x97\x0e\x4a"
      "\x7e\x07\x77\x07\x74\x17\x72\xb1\x37\x1a\x84\xe1\x42\x4e\xb3\x4f\xd8\x81"
      "\x61\x7b\xdd\xc7\x66\x25\x78\x85\x9a\xc8\xe6\xbb\xd4\x75\x32\xa1\x51\x79"
      "\x78\xfc\x9d\x13\x2c\xec\x27\x5c\x0f\xe9\xc2\x59\x27\x8f\xd0\x96\xae\xbe"
      "\xb9\x01\xcc\xa4\xf4\x9c\x20\x16\x6d\xc1\x69\x8e\x5b\xa9\x8c\xd9\xe0\x80"
      "\x71\x41\x8f\x04\x21\xd6\xaf\x4c\x0e\x78\xf9\x6e\x0a\xe2\xec\xdb\x9b\x36"
      "\xa6\x4d\x3e\xc9\x69\xe5\x11\xe6\xc2\xb7\x06\x85\x99\x6a\x51\x76\x94\x6b"
      "\x75\x69\xf0\x7f\x9a\xe3\x49\x80\x06\x06\x46\x0e\xd5\xad\xcc\x50\x02\x38"
      "\x8c\x00\xbf\xba\x89\xd8\x71\x55\x3d\x47\x2c\xea\x05\xf5\x81\x99\x67\x21"
      "\x0d\x21\xf1\x4b\x3d\xa6\xd7\xd7\x40\x87\x61\xa7\xa6\xdc\x3e\x0e\x10\xa3"
      "\x2a\x8b\x14\x5b\x8f\x8c\x61\xda\x50\x47\xf2\x5e\xb1\x51\x5d\x93\x4a\x44"
      "\x01\x09\x3e\xb7\xee\xd3\xb3\xfe\xa0\x1e\x01\x54\x38\xfc\x56\xa9\x6a\x0d"
      "\xa9\xc6\xa1\x17\xcc\x16\xf4\x75\xa2\x96\xbc\xd6\x42\xd1\x93\x20\xef\xd5"
      "\x14\x34\x69\xdc\x6c\xa1\x86\xfa\xb0\x92\x06\xc7\xfd\xc6\x52\x4c\x85\x91"
      "\x82\xa3\x17\xa6\x83\x04\x94\x05\x4e\xd3\xd1\x75\x4b\x75\x79\xe7\x7d\xa1"
      "\x98\x0a\xbf\x30\x8c\x9f\x8f\xf0\x0f\x94\x35\x0a\x79\xb1\xce\x4c\x41\xc7"
      "\x49\x5e\xbb\xf8\x71\x07\x52\xa8\xfe\x13\x87\xff\x5b\x8e\xec\x55\xcd\x06"
      "\xe0\xbb\xaf\x45\x0f\xe2\x60\x61\xf1\xb8\x9d\xe3\x77\x28\x0e\x01\xeb\xff"
      "\xcc\x1f\x93\x89\x19\xc4\x62\x76\x2e\x0a\x50\xd2\xd9\x60\x75\x58\x9e\x85"
      "\xc5\x82\x3c\xf8\x65\x51\x5d\xa4\x7e\xcb\x55\xf5\x2e\x8d\x1c\x3b\x43\x4e"
      "\xf6\xa7\x9d\xe0\xbe\xe4\x2f\xfe\xdc\xad\x29\x4a\x1a\xcf\x22\x68\x4f\x8b"
      "\xb1\xe6\x4b\x39\xad\xf0\x6e\x9a\x54\xca\xf6\xd4\x96\x72\x68\xae\xfe\x20"
      "\x8a\x73\xfb\x3f\xcc\x34\x91\x3c\x95\x8a\x01\xf4\x0d\x4d\xdd\x2e\x15\x07"
      "\x46\x3d\x3e\xc3\x0a\xbe\x72\xd4\x4d\xae\xda\xf7\x5d\xdb\x47\xe4\x68\x12"
      "\xed\x3e\xc6\xcd\x15\xa5\x83\xbe\x35\x34\xee\x12\x21\xca\x73\x02\xd1\x97"
      "\x76\x39\x1a\x56\xd4\xc4\x94\x3a\x4e\xca\x71\x25\x52\x17\x91\x27\x0f\x4d"
      "\x84\xe8\x39\x51\x6a\xd9\x78\x17\x6c\x34\x63\x06\xaf\x91\x2c\x1d\x0f\x61"
      "\xda\x4a\x13\x0a\x7f\xd9\xfb\xc0\x99\xb5\x42\x0e\x9a\x56\x3e\xb4\xbd\x95"
      "\x88\xbf\xbb\x21\xac\x56\x12\xf4\xd4\xe5\x62\x3e\xce\x26\x1c\x36\xd3\x3b"
      "\x0e\x30\xd3\x62\x93\x5c\x73\xc2\xd6\xcb\xee\xc6\x3c\x2b\x8b\x24\x07\xdb"
      "\xc3\x75\x8f\x37\x75\xcc\x76\x86\x20\xda\x42\xe2\x23\x10\xcb\x0d\x88\x21"
      "\x25\x66\x18\x4e\x21\xea\x91\x47\xd6\x5d\x1a\x64\x25\xcf\xdb\x9a\xf8\x36"
      "\xf4\x4b\xbc\x3f\xda\x2a\xd8\xe7\x0c\xc5\x04\xb8\x14\xf6\x7c\x52\xa9\x62"
      "\xf3\x96\xc7\x4e\x85\xa2\x77\x48\xad\x2d\xaf\x6d\x8a\x08\xc7\x85\x23\xf4"
      "\x65\x69\xe9\x3c\xa0\x22\x42\xa3\xa6\x25\x3b\x8c\x15\xd3\xfd\xa6\x94\x91"
      "\x53\xb5\x00\x2e\xc2\x22\xc1\x00\x33\xe3\x4e\xb5\x2e\x8b\xc5\xf7\x97\x88"
      "\x8d\xc8\x11\x4b\xae\x31\xf4\x81\x66\xa0\x3c\xa6\x44\x6f\xd0\x33\x86\x95"
      "\x9b\x25\x7c\x9a\x5d\xbe\x98\xf3\xc3\x62\xd2\x43\x49\xba\xf6\xd9\xf1\xd1"
      "\xcb\x35\x1d\xb3\xfe\xa7\x52\x66\x67\xfe\xae\x2e\x2e\x4b\x39\xed\xe0\x68"
      "\xbb\x17\x5f\xfb\xb7\xc0\x3b\x20\x87\x70\x9a\xd1\x6a\x94\x60\x60\x94\xdd"
      "\xe8\xe3\x7d\x38\xf0\x20\xdb\x6d\x2f\xe6\xb1\xe3\x36\xe6\x8f\x42\x96\x2b"
      "\x0a\x58\xcd\x8b\xef\x80\x60\x3f\x87\x45\x67\x6f\x6d\xef\x06\x6e\x4c\xb6"
      "\xf6\x81\xb6\x18\x92\x40\xf4\x84\xcf\x7c\x00\x00\x24\x00\x01\x00\x72\x61"
      "\x77\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xd5\x00\x06\x00\x06\x00"
      "\x72\x61\x77\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x01"
      "\xc2\x35\x16\x5f\x78\xc5\xc0\x44\xad\xd1\x56\x20\x0f\xf0\xcf\x7b\xe2\x79"
      "\xf8\xbb\xd9\x63\x93\x90\x8b\xad\xc8\xcc\x2f\x8b\xe7\xec\xa8\xc5\x27\x97"
      "\x24\x7c\x96\xc7\xfc\xd5\x25\x63\xbb\x24\x68\xc6\x33\x2e\x99\x8e\xab\x48"
      "\x3b\x7e\x31\xa5\xde\xc2\x07\xd8\xad\x92\xf8\x9e\xd1\x30\xb7\x81\x9c\x00"
      "\x57\x3d\x67\x42\x69\x99\xc3\xd0\x18\xf0\x30\x2a\xce\x47\x77\x4a\x38\x54"
      "\x24\xb3\xe4\x75\xfd\x1f\x47\x1a\xd3\x10\xf0\xe8\xed\x85\xeb\xd1\xcc\x6e"
      "\xae\xe6\x3a\x51\xc9\x88\x2d\x1d\xb1\xae\x55\x00\xe9\x2f\x70\xdd\xd0\xd5"
      "\xda\xf7\xe9\x7f\x81\xaf\x2f\x26\x59\x03\x83\x51\xa0\x08\xc5\xd7\xf8\xe9"
      "\xf7\xe8\x0c\xc9\x30\xcd\x6c\x37\x1e\x0e\x4c\x62\xe8\x98\xc5\x86\xb3\x5c"
      "\x2b\x4f\x75\x4a\x00\x72\xa9\x2c\x34\x00\x00\x00\x04\x00\x06\x00\x0c\x00"
      "\x07\x00\x01\x00\x00\x00\x01\x00\x00\x00\x0c\x00\x08\x00\x02\x00\x00\x00"
      "\x00\x00\x00\x00\x14\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x08\x00\x01\x00\x04\x00\x03\x00\x14\x00\x03\x00"
      "\xff\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x9c\x14"
      "\x05\x00\x3c\x00\x01\x00\x03\x00\x00\x00\x07\x00\x00\x00\xff\x01\x00\x00"
      "\x03\x00\x00\x00\x00\x08\x00",
      4795);
  *(uint32_t*)0x20000184 = 0x275c;
  *(uint32_t*)0x2000038c = 1;
  *(uint32_t*)0x20000390 = 0;
  *(uint32_t*)0x20000394 = 0;
  *(uint32_t*)0x20000398 = 0;
  syscall(__NR_sendmsg, -1, 0x20000380, 0);
  res = syscall(__NR_socket, 0x10, 0x80002, 0);
  if (res != -1)
    r[2] = res;
  syscall(__NR_sendmmsg, (intptr_t)r[2], 0x20000200, 0x924926d3, 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000, 0x1000, 0, 0x32, -1, 0);
  syscall(__NR_mmap, 0x20000000, 0x1000000, 7, 0x32, -1, 0);
  syscall(__NR_mmap, 0x21000000, 0x1000, 0, 0x32, -1, 0);
  do_sandbox_none();
  return 0;
}
