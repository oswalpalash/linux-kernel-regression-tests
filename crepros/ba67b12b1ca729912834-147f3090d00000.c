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
      if (current_time_ms() - start < 5000) {
        continue;
      }
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[3] = {0xffffffffffffffff, 0x0, 0xffffffffffffffff};

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
  *(uint32_t*)0x20000200 = 0x14;
  res = syscall(__NR_getsockname, r[0], 0x200002c0ul, 0x20000200ul);
  if (res != -1)
    r[1] = *(uint32_t*)0x200002c4;
  *(uint64_t*)0x20000240 = 0;
  *(uint32_t*)0x20000248 = 0;
  *(uint64_t*)0x20000250 = 0x20000140;
  *(uint64_t*)0x20000140 = 0x200004c0;
  memcpy((void*)0x200004c0, "\x38\x00\x00\x00\x24\x00\xff\xff\xff\x7f\x00\x00"
                            "\x00\x00\x3c\x00\x05\x00\x00\x00",
         20);
  *(uint32_t*)0x200004d4 = r[1];
  memcpy((void*)0x200004d8, "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00"
                            "\x09\x00\x01\x00\x68\x66\x73\x63\x00\x00\x00\x00"
                            "\x08\x00\x02",
         27);
  *(uint64_t*)0x20000148 = 0x38;
  *(uint64_t*)0x20000258 = 1;
  *(uint64_t*)0x20000260 = 0;
  *(uint64_t*)0x20000268 = 0;
  *(uint32_t*)0x20000270 = 0;
  syscall(__NR_sendmsg, -1, 0x20000240ul, 0ul);
  *(uint64_t*)0x20000380 = 0;
  *(uint32_t*)0x20000388 = 0;
  *(uint64_t*)0x20000390 = 0x20000180;
  *(uint64_t*)0x20000180 = 0x20000500;
  *(uint32_t*)0x20000500 = 0x498;
  *(uint16_t*)0x20000504 = 0x2c;
  *(uint16_t*)0x20000506 = 0xd27;
  *(uint32_t*)0x20000508 = 0;
  *(uint32_t*)0x2000050c = 0;
  *(uint8_t*)0x20000510 = 0;
  *(uint8_t*)0x20000511 = 0;
  *(uint16_t*)0x20000512 = 0;
  *(uint32_t*)0x20000514 = r[1];
  *(uint16_t*)0x20000518 = 0;
  *(uint16_t*)0x2000051a = 0;
  *(uint16_t*)0x2000051c = 0;
  *(uint16_t*)0x2000051e = 0;
  *(uint16_t*)0x20000520 = 0xa;
  *(uint16_t*)0x20000522 = 0;
  *(uint16_t*)0x20000524 = 9;
  *(uint16_t*)0x20000526 = 1;
  memcpy((void*)0x20000528, "rsvp\000", 5);
  *(uint16_t*)0x20000530 = 0x468;
  *(uint16_t*)0x20000532 = 2;
  *(uint16_t*)0x20000534 = 8;
  *(uint16_t*)0x20000536 = 2;
  *(uint32_t*)0x20000538 = htobe32(0xe0000001);
  *(uint16_t*)0x2000053c = 0x45c;
  *(uint16_t*)0x2000053e = 5;
  *(uint16_t*)0x20000540 = 8;
  *(uint16_t*)0x20000542 = 5;
  *(uint32_t*)0x20000544 = 3;
  *(uint16_t*)0x20000548 = 8;
  *(uint16_t*)0x2000054a = 5;
  *(uint32_t*)0x2000054c = 3;
  *(uint16_t*)0x20000550 = 8;
  *(uint16_t*)0x20000552 = 5;
  *(uint32_t*)0x20000554 = 0xb39;
  *(uint16_t*)0x20000558 = 0x3c;
  *(uint16_t*)0x2000055a = 1;
  *(uint32_t*)0x2000055c = 1;
  *(uint32_t*)0x20000560 = 2;
  *(uint32_t*)0x20000564 = 5;
  *(uint32_t*)0x20000568 = 4;
  *(uint32_t*)0x2000056c = 0x3e85;
  *(uint8_t*)0x20000570 = 0x7f;
  *(uint8_t*)0x20000571 = 0;
  *(uint16_t*)0x20000572 = 0x3ff;
  *(uint16_t*)0x20000574 = 0x1f;
  *(uint16_t*)0x20000576 = 0xae;
  *(uint32_t*)0x20000578 = 0;
  *(uint8_t*)0x2000057c = 0x8b;
  *(uint8_t*)0x2000057d = 0;
  *(uint16_t*)0x2000057e = 0xa40d;
  *(uint16_t*)0x20000580 = 1;
  *(uint16_t*)0x20000582 = 0xfff;
  *(uint32_t*)0x20000584 = 0x101;
  *(uint32_t*)0x20000588 = 0xa000;
  *(uint32_t*)0x2000058c = 2;
  *(uint32_t*)0x20000590 = 0x10000;
  *(uint16_t*)0x20000594 = 0x404;
  *(uint16_t*)0x20000596 = 2;
  *(uint32_t*)0x20000598 = 0x80;
  *(uint32_t*)0x2000059c = 2;
  *(uint32_t*)0x200005a0 = 0x13a;
  *(uint32_t*)0x200005a4 = 4;
  *(uint32_t*)0x200005a8 = 7;
  *(uint32_t*)0x200005ac = 0x1f;
  *(uint32_t*)0x200005b0 = 0xbbef483;
  *(uint32_t*)0x200005b4 = 0x83d;
  *(uint32_t*)0x200005b8 = 0x81;
  *(uint32_t*)0x200005bc = 3;
  *(uint32_t*)0x200005c0 = 0x100;
  *(uint32_t*)0x200005c4 = 0x9f400000;
  *(uint32_t*)0x200005c8 = 0xd6;
  *(uint32_t*)0x200005cc = 0x24bb;
  *(uint32_t*)0x200005d0 = 6;
  *(uint32_t*)0x200005d4 = 7;
  *(uint32_t*)0x200005d8 = 5;
  *(uint32_t*)0x200005dc = 0x101;
  *(uint32_t*)0x200005e0 = 0xbf68;
  *(uint32_t*)0x200005e4 = 5;
  *(uint32_t*)0x200005e8 = 0x28e;
  *(uint32_t*)0x200005ec = 0x7f;
  *(uint32_t*)0x200005f0 = 0;
  *(uint32_t*)0x200005f4 = 0xb726;
  *(uint32_t*)0x200005f8 = 0x1000;
  *(uint32_t*)0x200005fc = 1;
  *(uint32_t*)0x20000600 = 2;
  *(uint32_t*)0x20000604 = 9;
  *(uint32_t*)0x20000608 = 0x3ff;
  *(uint32_t*)0x2000060c = 7;
  *(uint32_t*)0x20000610 = 0;
  *(uint32_t*)0x20000614 = 3;
  *(uint32_t*)0x20000618 = 0xff;
  *(uint32_t*)0x2000061c = 8;
  *(uint32_t*)0x20000620 = 9;
  *(uint32_t*)0x20000624 = 8;
  *(uint32_t*)0x20000628 = 0x7da1;
  *(uint32_t*)0x2000062c = 0xffff;
  *(uint32_t*)0x20000630 = 6;
  *(uint32_t*)0x20000634 = 1;
  *(uint32_t*)0x20000638 = 0x1f;
  *(uint32_t*)0x2000063c = 8;
  *(uint32_t*)0x20000640 = 2;
  *(uint32_t*)0x20000644 = 2;
  *(uint32_t*)0x20000648 = 1;
  *(uint32_t*)0x2000064c = 0x3ff;
  *(uint32_t*)0x20000650 = 9;
  *(uint32_t*)0x20000654 = 0xea90000;
  *(uint32_t*)0x20000658 = 6;
  *(uint32_t*)0x2000065c = 0x80;
  *(uint32_t*)0x20000660 = 5;
  *(uint32_t*)0x20000664 = 0;
  *(uint32_t*)0x20000668 = 5;
  *(uint32_t*)0x2000066c = 7;
  *(uint32_t*)0x20000670 = 0x3ff;
  *(uint32_t*)0x20000674 = 1;
  *(uint32_t*)0x20000678 = 2;
  *(uint32_t*)0x2000067c = 0x96b3;
  *(uint32_t*)0x20000680 = 2;
  *(uint32_t*)0x20000684 = 7;
  *(uint32_t*)0x20000688 = 0x10001;
  *(uint32_t*)0x2000068c = 2;
  *(uint32_t*)0x20000690 = 6;
  *(uint32_t*)0x20000694 = 4;
  *(uint32_t*)0x20000698 = 4;
  *(uint32_t*)0x2000069c = 0xa3;
  *(uint32_t*)0x200006a0 = 1;
  *(uint32_t*)0x200006a4 = 7;
  *(uint32_t*)0x200006a8 = 4;
  *(uint32_t*)0x200006ac = 0x6f;
  *(uint32_t*)0x200006b0 = 0xf75a;
  *(uint32_t*)0x200006b4 = 0x7fffffff;
  *(uint32_t*)0x200006b8 = 5;
  *(uint32_t*)0x200006bc = 0x1ff;
  *(uint32_t*)0x200006c0 = 9;
  *(uint32_t*)0x200006c4 = 0x3d2;
  *(uint32_t*)0x200006c8 = 0x80000001;
  *(uint32_t*)0x200006cc = 7;
  *(uint32_t*)0x200006d0 = 6;
  *(uint32_t*)0x200006d4 = 9;
  *(uint32_t*)0x200006d8 = 8;
  *(uint32_t*)0x200006dc = 0x7ff;
  *(uint32_t*)0x200006e0 = 0xd21c;
  *(uint32_t*)0x200006e4 = 0x80;
  *(uint32_t*)0x200006e8 = 5;
  *(uint32_t*)0x200006ec = 0x40;
  *(uint32_t*)0x200006f0 = 4;
  *(uint32_t*)0x200006f4 = 5;
  *(uint32_t*)0x200006f8 = 0x6a;
  *(uint32_t*)0x200006fc = 0xcd;
  *(uint32_t*)0x20000700 = 5;
  *(uint32_t*)0x20000704 = 6;
  *(uint32_t*)0x20000708 = 0x81;
  *(uint32_t*)0x2000070c = 0x971;
  *(uint32_t*)0x20000710 = 4;
  *(uint32_t*)0x20000714 = 7;
  *(uint32_t*)0x20000718 = 1;
  *(uint32_t*)0x2000071c = 6;
  *(uint32_t*)0x20000720 = 0;
  *(uint32_t*)0x20000724 = 1;
  *(uint32_t*)0x20000728 = 4;
  *(uint32_t*)0x2000072c = 4;
  *(uint32_t*)0x20000730 = 9;
  *(uint32_t*)0x20000734 = -1;
  *(uint32_t*)0x20000738 = 9;
  *(uint32_t*)0x2000073c = 0x3b08;
  *(uint32_t*)0x20000740 = 0x6c;
  *(uint32_t*)0x20000744 = 0x7cb;
  *(uint32_t*)0x20000748 = 0xffff;
  *(uint32_t*)0x2000074c = 0;
  *(uint32_t*)0x20000750 = 0x3ff;
  *(uint32_t*)0x20000754 = 9;
  *(uint32_t*)0x20000758 = 5;
  *(uint32_t*)0x2000075c = 1;
  *(uint32_t*)0x20000760 = 7;
  *(uint32_t*)0x20000764 = 1;
  *(uint32_t*)0x20000768 = 0xfffffffb;
  *(uint32_t*)0x2000076c = 0x7fff;
  *(uint32_t*)0x20000770 = 0x28;
  *(uint32_t*)0x20000774 = 7;
  *(uint32_t*)0x20000778 = 9;
  *(uint32_t*)0x2000077c = 8;
  *(uint32_t*)0x20000780 = 0x10000;
  *(uint32_t*)0x20000784 = 5;
  *(uint32_t*)0x20000788 = 6;
  *(uint32_t*)0x2000078c = 0x80000000;
  *(uint32_t*)0x20000790 = 0;
  *(uint32_t*)0x20000794 = 9;
  *(uint32_t*)0x20000798 = 2;
  *(uint32_t*)0x2000079c = 0x80000000;
  *(uint32_t*)0x200007a0 = 0x1ff;
  *(uint32_t*)0x200007a4 = 4;
  *(uint32_t*)0x200007a8 = 4;
  *(uint32_t*)0x200007ac = 0x767a;
  *(uint32_t*)0x200007b0 = 0x8001;
  *(uint32_t*)0x200007b4 = 2;
  *(uint32_t*)0x200007b8 = 0x5d;
  *(uint32_t*)0x200007bc = 0x40;
  *(uint32_t*)0x200007c0 = 7;
  *(uint32_t*)0x200007c4 = 8;
  *(uint32_t*)0x200007c8 = 0x7358ae37;
  *(uint32_t*)0x200007cc = 5;
  *(uint32_t*)0x200007d0 = 0x200;
  *(uint32_t*)0x200007d4 = 9;
  *(uint32_t*)0x200007d8 = 0x3f;
  *(uint32_t*)0x200007dc = 0x7fff;
  *(uint32_t*)0x200007e0 = 0xe8cf;
  *(uint32_t*)0x200007e4 = 0x1ff;
  *(uint32_t*)0x200007e8 = 0x80000001;
  *(uint32_t*)0x200007ec = 0x5a7cabff;
  *(uint32_t*)0x200007f0 = 1;
  *(uint32_t*)0x200007f4 = 6;
  *(uint32_t*)0x200007f8 = 2;
  *(uint32_t*)0x200007fc = 0x10001;
  *(uint32_t*)0x20000800 = 7;
  *(uint32_t*)0x20000804 = 9;
  *(uint32_t*)0x20000808 = 7;
  *(uint32_t*)0x2000080c = 0x100;
  *(uint32_t*)0x20000810 = 0;
  *(uint32_t*)0x20000814 = 2;
  *(uint32_t*)0x20000818 = 2;
  *(uint32_t*)0x2000081c = 9;
  *(uint32_t*)0x20000820 = 0x2ae;
  *(uint32_t*)0x20000824 = 0xf9;
  *(uint32_t*)0x20000828 = 0x7fffffff;
  *(uint32_t*)0x2000082c = 0x80000000;
  *(uint32_t*)0x20000830 = 0xc60;
  *(uint32_t*)0x20000834 = 4;
  *(uint32_t*)0x20000838 = 0xd8d;
  *(uint32_t*)0x2000083c = 9;
  *(uint32_t*)0x20000840 = 6;
  *(uint32_t*)0x20000844 = 0xbc;
  *(uint32_t*)0x20000848 = 0x7f;
  *(uint32_t*)0x2000084c = 0x200;
  *(uint32_t*)0x20000850 = 7;
  *(uint32_t*)0x20000854 = 5;
  *(uint32_t*)0x20000858 = 0;
  *(uint32_t*)0x2000085c = 2;
  *(uint32_t*)0x20000860 = 0;
  *(uint32_t*)0x20000864 = 3;
  *(uint32_t*)0x20000868 = 4;
  *(uint32_t*)0x2000086c = 1;
  *(uint32_t*)0x20000870 = 9;
  *(uint32_t*)0x20000874 = 3;
  *(uint32_t*)0x20000878 = 0x1f;
  *(uint32_t*)0x2000087c = 1;
  *(uint32_t*)0x20000880 = 7;
  *(uint32_t*)0x20000884 = 8;
  *(uint32_t*)0x20000888 = 8;
  *(uint32_t*)0x2000088c = 0x400;
  *(uint32_t*)0x20000890 = 0x400;
  *(uint32_t*)0x20000894 = 0x10000;
  *(uint32_t*)0x20000898 = 8;
  *(uint32_t*)0x2000089c = 0xfffffeff;
  *(uint32_t*)0x200008a0 = 0xffff;
  *(uint32_t*)0x200008a4 = 0xff;
  *(uint32_t*)0x200008a8 = 0;
  *(uint32_t*)0x200008ac = 5;
  *(uint32_t*)0x200008b0 = 1;
  *(uint32_t*)0x200008b4 = 0;
  *(uint32_t*)0x200008b8 = 9;
  *(uint32_t*)0x200008bc = 2;
  *(uint32_t*)0x200008c0 = 0x8683;
  *(uint32_t*)0x200008c4 = 6;
  *(uint32_t*)0x200008c8 = 0;
  *(uint32_t*)0x200008cc = 0xde;
  *(uint32_t*)0x200008d0 = 0;
  *(uint32_t*)0x200008d4 = 0xe0;
  *(uint32_t*)0x200008d8 = 6;
  *(uint32_t*)0x200008dc = 0x81;
  *(uint32_t*)0x200008e0 = 3;
  *(uint32_t*)0x200008e4 = 7;
  *(uint32_t*)0x200008e8 = 1;
  *(uint32_t*)0x200008ec = 0x3f;
  *(uint32_t*)0x200008f0 = 0x10000000;
  *(uint32_t*)0x200008f4 = 7;
  *(uint32_t*)0x200008f8 = 4;
  *(uint32_t*)0x200008fc = 6;
  *(uint32_t*)0x20000900 = 6;
  *(uint32_t*)0x20000904 = 1;
  *(uint32_t*)0x20000908 = 0xffffff7f;
  *(uint32_t*)0x2000090c = 0x6f;
  *(uint32_t*)0x20000910 = 3;
  *(uint32_t*)0x20000914 = 4;
  *(uint32_t*)0x20000918 = 0x221;
  *(uint32_t*)0x2000091c = 6;
  *(uint32_t*)0x20000920 = 0x8a1;
  *(uint32_t*)0x20000924 = 0x80000001;
  *(uint32_t*)0x20000928 = 0x4ce;
  *(uint32_t*)0x2000092c = 0x200;
  *(uint32_t*)0x20000930 = 9;
  *(uint32_t*)0x20000934 = 0xa1;
  *(uint32_t*)0x20000938 = 0;
  *(uint32_t*)0x2000093c = 2;
  *(uint32_t*)0x20000940 = 7;
  *(uint32_t*)0x20000944 = 1;
  *(uint32_t*)0x20000948 = 0xfffffffa;
  *(uint32_t*)0x2000094c = 0x123b;
  *(uint32_t*)0x20000950 = -1;
  *(uint32_t*)0x20000954 = 4;
  *(uint32_t*)0x20000958 = 0x400;
  *(uint32_t*)0x2000095c = 3;
  *(uint32_t*)0x20000960 = 0x529;
  *(uint32_t*)0x20000964 = 0x101;
  *(uint32_t*)0x20000968 = 0x53d5;
  *(uint32_t*)0x2000096c = 0x3ff;
  *(uint32_t*)0x20000970 = 0xb9;
  *(uint32_t*)0x20000974 = 1;
  *(uint32_t*)0x20000978 = 3;
  *(uint32_t*)0x2000097c = 0xffff;
  *(uint32_t*)0x20000980 = 8;
  *(uint32_t*)0x20000984 = 2;
  *(uint32_t*)0x20000988 = 4;
  *(uint32_t*)0x2000098c = 0;
  *(uint32_t*)0x20000990 = 4;
  *(uint32_t*)0x20000994 = 0x8001;
  *(uint64_t*)0x20000188 = 0x498;
  *(uint64_t*)0x20000398 = 1;
  *(uint64_t*)0x200003a0 = 0;
  *(uint64_t*)0x200003a8 = 0;
  *(uint32_t*)0x200003b0 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000380ul, 0ul);
  res = syscall(__NR_socket, 0x1000000010ul, 0x80002ul, 0);
  if (res != -1)
    r[2] = res;
  syscall(__NR_sendmmsg, r[2], 0x20000200ul, 0x10efe10675dec16ul, 0ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  do_sandbox_none();
  return 0;
}
