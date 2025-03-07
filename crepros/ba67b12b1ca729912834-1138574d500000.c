// https://syzkaller.appspot.com/bug?id=efc234d0d3f0d6b900fe67b5334dc76cde55dbd4
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

uint64_t r[3] = {0xffffffffffffffff, 0x0, 0xffffffffffffffff};

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
  *(uint32_t*)0x20000180 = 0x200004c0;
  *(uint32_t*)0x200004c0 = 0x84;
  *(uint16_t*)0x200004c4 = 0x2c;
  *(uint16_t*)0x200004c6 = 0xd27;
  *(uint32_t*)0x200004c8 = 0;
  *(uint32_t*)0x200004cc = 0;
  *(uint8_t*)0x200004d0 = 0;
  *(uint8_t*)0x200004d1 = 0;
  *(uint16_t*)0x200004d2 = 0;
  *(uint32_t*)0x200004d4 = r[1];
  *(uint16_t*)0x200004d8 = 0;
  *(uint16_t*)0x200004da = 0;
  *(uint16_t*)0x200004dc = 0;
  *(uint16_t*)0x200004de = 0;
  *(uint16_t*)0x200004e0 = 0xc;
  *(uint16_t*)0x200004e2 = 0;
  *(uint16_t*)0x200004e4 = 9;
  *(uint16_t*)0x200004e6 = 1;
  memcpy((void*)0x200004e8, "rsvp\000", 5);
  *(uint16_t*)0x200004f0 = 0x54;
  *(uint16_t*)0x200004f2 = 2;
  *(uint16_t*)0x200004f4 = 8;
  *(uint16_t*)0x200004f6 = 3;
  *(uint32_t*)0x200004f8 = htobe32(-1);
  *(uint16_t*)0x200004fc = 8;
  *(uint16_t*)0x200004fe = 2;
  *(uint32_t*)0x20000500 = htobe32(0x7f000001);
  *(uint16_t*)0x20000504 = 0x40;
  *(uint16_t*)0x20000506 = 5;
  *(uint16_t*)0x20000508 = 0x3c;
  *(uint16_t*)0x2000050a = 1;
  *(uint32_t*)0x2000050c = 0xffffff55;
  *(uint32_t*)0x20000510 = 0;
  *(uint32_t*)0x20000514 = 0;
  *(uint32_t*)0x20000518 = 0;
  *(uint32_t*)0x2000051c = 0;
  *(uint8_t*)0x20000520 = 0;
  *(uint8_t*)0x20000521 = 0;
  *(uint16_t*)0x20000522 = 0;
  *(uint16_t*)0x20000524 = 0;
  *(uint16_t*)0x20000526 = 0;
  *(uint32_t*)0x20000528 = 0;
  *(uint8_t*)0x2000052c = 0;
  *(uint8_t*)0x2000052d = 0;
  *(uint16_t*)0x2000052e = 0;
  *(uint16_t*)0x20000530 = 0;
  *(uint16_t*)0x20000532 = 0;
  *(uint32_t*)0x20000534 = 0x7ff;
  *(uint32_t*)0x20000538 = 0;
  *(uint32_t*)0x2000053c = 0;
  *(uint32_t*)0x20000540 = 0;
  *(uint32_t*)0x20000184 = 0x84;
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
