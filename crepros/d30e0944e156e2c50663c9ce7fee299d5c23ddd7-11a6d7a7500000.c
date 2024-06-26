// https://syzkaller.appspot.com/bug?id=d30e0944e156e2c50663c9ce7fee299d5c23ddd7
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

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0x0};

void execute_one(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_socket, 0x10ul, 0x803ul, 0);
  if (res != -1)
    r[2] = res;
  *(uint64_t*)0x20000280 = 0;
  *(uint32_t*)0x20000288 = 0;
  *(uint64_t*)0x20000290 = 0x20000180;
  *(uint64_t*)0x20000180 = 0;
  *(uint64_t*)0x20000188 = 0;
  *(uint64_t*)0x20000298 = 1;
  *(uint64_t*)0x200002a0 = 0;
  *(uint64_t*)0x200002a8 = 0;
  *(uint32_t*)0x200002b0 = 0;
  syscall(__NR_sendmsg, r[2], 0x20000280ul, 0ul);
  *(uint32_t*)0x20000200 = 0x14;
  res = syscall(__NR_getsockname, r[2], 0x20000100ul, 0x20000200ul);
  if (res != -1)
    r[3] = *(uint32_t*)0x20000104;
  *(uint64_t*)0x20000240 = 0;
  *(uint32_t*)0x20000248 = 0;
  *(uint64_t*)0x20000250 = 0x20000140;
  *(uint64_t*)0x20000140 = 0x200003c0;
  memcpy((void*)0x200003c0, "\x38\x00\x00\x00\x24\x00\x07\x05\x00\x00\x00\x40"
                            "\x07\xa2\xa3\x00\x05\x00\x00\x00",
         20);
  *(uint32_t*)0x200003d4 = r[3];
  memcpy((void*)0x200003d8, "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00"
                            "\x09\x00\x01\x00\x68\x66\x73\x63\x00\x00\x00\x00"
                            "\x08\x00\x02",
         27);
  *(uint64_t*)0x20000148 = 0x38;
  *(uint64_t*)0x20000258 = 1;
  *(uint64_t*)0x20000260 = 0;
  *(uint64_t*)0x20000268 = 0;
  *(uint32_t*)0x20000270 = 0;
  syscall(__NR_sendmsg, r[1], 0x20000240ul, 0ul);
  *(uint64_t*)0x200001c0 = 0;
  *(uint32_t*)0x200001c8 = 0;
  *(uint64_t*)0x200001d0 = 0x20000180;
  *(uint64_t*)0x20000180 = 0x20000600;
  *(uint32_t*)0x20000600 = 0xce8;
  *(uint16_t*)0x20000604 = 0x2c;
  *(uint16_t*)0x20000606 = 0xd27;
  *(uint32_t*)0x20000608 = 0;
  *(uint32_t*)0x2000060c = 0;
  *(uint8_t*)0x20000610 = 0;
  *(uint8_t*)0x20000611 = 0;
  *(uint16_t*)0x20000612 = 0;
  *(uint32_t*)0x20000614 = r[3];
  *(uint16_t*)0x20000618 = 0;
  *(uint16_t*)0x2000061a = 0;
  *(uint16_t*)0x2000061c = 0;
  *(uint16_t*)0x2000061e = 0;
  *(uint16_t*)0x20000620 = 0x10;
  *(uint16_t*)0x20000622 = 0;
  *(uint16_t*)0x20000624 = 0xc;
  *(uint16_t*)0x20000626 = 1;
  memcpy((void*)0x20000628, "tcindex\000", 8);
  *(uint16_t*)0x20000630 = 0xcb8;
  *(uint16_t*)0x20000632 = 2;
  *(uint16_t*)0x20000634 = 0xcb4;
  *(uint16_t*)0x20000636 = 6;
  *(uint16_t*)0x20000638 = 0xc;
  *(uint16_t*)0x2000063a = 8;
  *(uint64_t*)0x2000063c = 3;
  *(uint16_t*)0x20000644 = 0xc;
  *(uint16_t*)0x20000646 = 9;
  *(uint64_t*)0x20000648 = 9;
  *(uint16_t*)0x20000650 = 8;
  *(uint16_t*)0x20000652 = 5;
  *(uint32_t*)0x20000654 = 7;
  *(uint16_t*)0x20000658 = 0x3c;
  *(uint16_t*)0x2000065a = 1;
  *(uint32_t*)0x2000065c = 0x1f;
  *(uint32_t*)0x20000660 = 0;
  *(uint32_t*)0x20000664 = 0x1000;
  *(uint32_t*)0x20000668 = 0xd721;
  *(uint32_t*)0x2000066c = 4;
  *(uint8_t*)0x20000670 = 3;
  *(uint8_t*)0x20000671 = 0;
  *(uint16_t*)0x20000672 = 1;
  *(uint16_t*)0x20000674 = 0x200;
  *(uint16_t*)0x20000676 = 0;
  *(uint32_t*)0x20000678 = 1;
  *(uint8_t*)0x2000067c = 4;
  *(uint8_t*)0x2000067d = 0;
  *(uint16_t*)0x2000067e = 2;
  *(uint16_t*)0x20000680 = 0x401;
  *(uint16_t*)0x20000682 = 0x1ff;
  *(uint32_t*)0x20000684 = 0xffffff01;
  *(uint32_t*)0x20000688 = 9;
  *(uint32_t*)0x2000068c = 3;
  *(uint32_t*)0x20000690 = 7;
  *(uint16_t*)0x20000694 = 0xc;
  *(uint16_t*)0x20000696 = 8;
  *(uint64_t*)0x20000698 = 0x240000000;
  *(uint16_t*)0x200006a0 = 0x3c;
  *(uint16_t*)0x200006a2 = 1;
  *(uint32_t*)0x200006a4 = 0xffff;
  *(uint32_t*)0x200006a8 = 8;
  *(uint32_t*)0x200006ac = 0xfffffffd;
  *(uint32_t*)0x200006b0 = 1;
  *(uint32_t*)0x200006b4 = 7;
  *(uint8_t*)0x200006b8 = 6;
  *(uint8_t*)0x200006b9 = 2;
  *(uint16_t*)0x200006ba = 1;
  *(uint16_t*)0x200006bc = 1;
  *(uint16_t*)0x200006be = 0x11f;
  *(uint32_t*)0x200006c0 = 3;
  *(uint8_t*)0x200006c4 = 3;
  *(uint8_t*)0x200006c5 = 1;
  *(uint16_t*)0x200006c6 = 0x101;
  *(uint16_t*)0x200006c8 = 0xfea;
  *(uint16_t*)0x200006ca = 0x8000;
  *(uint32_t*)0x200006cc = 8;
  *(uint32_t*)0x200006d0 = 9;
  *(uint32_t*)0x200006d4 = 8;
  *(uint32_t*)0x200006d8 = 4;
  *(uint16_t*)0x200006dc = 0x404;
  *(uint16_t*)0x200006de = 3;
  *(uint32_t*)0x200006e0 = 0x8000;
  *(uint32_t*)0x200006e4 = 2;
  *(uint32_t*)0x200006e8 = 0x80000000;
  *(uint32_t*)0x200006ec = 7;
  *(uint32_t*)0x200006f0 = 0x101;
  *(uint32_t*)0x200006f4 = 2;
  *(uint32_t*)0x200006f8 = 8;
  *(uint32_t*)0x200006fc = 6;
  *(uint32_t*)0x20000700 = 0xffffff81;
  *(uint32_t*)0x20000704 = 3;
  *(uint32_t*)0x20000708 = 0xfffffffc;
  *(uint32_t*)0x2000070c = 3;
  *(uint32_t*)0x20000710 = 4;
  *(uint32_t*)0x20000714 = 0x7fff;
  *(uint32_t*)0x20000718 = 6;
  *(uint32_t*)0x2000071c = 1;
  *(uint32_t*)0x20000720 = 0x3ff;
  *(uint32_t*)0x20000724 = 1;
  *(uint32_t*)0x20000728 = 0x31d;
  *(uint32_t*)0x2000072c = 8;
  *(uint32_t*)0x20000730 = 4;
  *(uint32_t*)0x20000734 = 0xe7;
  *(uint32_t*)0x20000738 = 1;
  *(uint32_t*)0x2000073c = 5;
  *(uint32_t*)0x20000740 = 9;
  *(uint32_t*)0x20000744 = 0x400;
  *(uint32_t*)0x20000748 = 2;
  *(uint32_t*)0x2000074c = 1;
  *(uint32_t*)0x20000750 = 0x3ff;
  *(uint32_t*)0x20000754 = 0x6d062c52;
  *(uint32_t*)0x20000758 = 2;
  *(uint32_t*)0x2000075c = 5;
  *(uint32_t*)0x20000760 = 2;
  *(uint32_t*)0x20000764 = 0x401;
  *(uint32_t*)0x20000768 = 3;
  *(uint32_t*)0x2000076c = 0;
  *(uint32_t*)0x20000770 = 7;
  *(uint32_t*)0x20000774 = 0x101;
  *(uint32_t*)0x20000778 = 8;
  *(uint32_t*)0x2000077c = 8;
  *(uint32_t*)0x20000780 = -1;
  *(uint32_t*)0x20000784 = 0xdc45;
  *(uint32_t*)0x20000788 = 3;
  *(uint32_t*)0x2000078c = 6;
  *(uint32_t*)0x20000790 = -1;
  *(uint32_t*)0x20000794 = 3;
  *(uint32_t*)0x20000798 = 0xc0;
  *(uint32_t*)0x2000079c = 2;
  *(uint32_t*)0x200007a0 = 0x39e;
  *(uint32_t*)0x200007a4 = 3;
  *(uint32_t*)0x200007a8 = 0x400;
  *(uint32_t*)0x200007ac = 0x7f;
  *(uint32_t*)0x200007b0 = 0x7a2;
  *(uint32_t*)0x200007b4 = 5;
  *(uint32_t*)0x200007b8 = 0x7fff;
  *(uint32_t*)0x200007bc = 0xc1;
  *(uint32_t*)0x200007c0 = 0x8000;
  *(uint32_t*)0x200007c4 = 9;
  *(uint32_t*)0x200007c8 = 0x81;
  *(uint32_t*)0x200007cc = 0x100;
  *(uint32_t*)0x200007d0 = 6;
  *(uint32_t*)0x200007d4 = 8;
  *(uint32_t*)0x200007d8 = 0x336;
  *(uint32_t*)0x200007dc = 0xe153;
  *(uint32_t*)0x200007e0 = 1;
  *(uint32_t*)0x200007e4 = 4;
  *(uint32_t*)0x200007e8 = 8;
  *(uint32_t*)0x200007ec = 0xc;
  *(uint32_t*)0x200007f0 = 0x401;
  *(uint32_t*)0x200007f4 = 0x3ff;
  *(uint32_t*)0x200007f8 = 2;
  *(uint32_t*)0x200007fc = 0x76;
  *(uint32_t*)0x20000800 = 0;
  *(uint32_t*)0x20000804 = 0x7fffffff;
  *(uint32_t*)0x20000808 = 5;
  *(uint32_t*)0x2000080c = 0xc9;
  *(uint32_t*)0x20000810 = 0;
  *(uint32_t*)0x20000814 = 0xe881;
  *(uint32_t*)0x20000818 = 0x81;
  *(uint32_t*)0x2000081c = 0x99;
  *(uint32_t*)0x20000820 = 6;
  *(uint32_t*)0x20000824 = 0x80;
  *(uint32_t*)0x20000828 = 9;
  *(uint32_t*)0x2000082c = 6;
  *(uint32_t*)0x20000830 = 0x890b;
  *(uint32_t*)0x20000834 = 3;
  *(uint32_t*)0x20000838 = 4;
  *(uint32_t*)0x2000083c = 0x1ff;
  *(uint32_t*)0x20000840 = 6;
  *(uint32_t*)0x20000844 = 5;
  *(uint32_t*)0x20000848 = 0xfffffeff;
  *(uint32_t*)0x2000084c = 2;
  *(uint32_t*)0x20000850 = 0x80000001;
  *(uint32_t*)0x20000854 = 6;
  *(uint32_t*)0x20000858 = 0xcf;
  *(uint32_t*)0x2000085c = 5;
  *(uint32_t*)0x20000860 = 0x7ff;
  *(uint32_t*)0x20000864 = 8;
  *(uint32_t*)0x20000868 = 2;
  *(uint32_t*)0x2000086c = 0;
  *(uint32_t*)0x20000870 = 0x81;
  *(uint32_t*)0x20000874 = 4;
  *(uint32_t*)0x20000878 = 0x44485432;
  *(uint32_t*)0x2000087c = 0;
  *(uint32_t*)0x20000880 = 0xff;
  *(uint32_t*)0x20000884 = 9;
  *(uint32_t*)0x20000888 = 1;
  *(uint32_t*)0x2000088c = 0x167;
  *(uint32_t*)0x20000890 = 9;
  *(uint32_t*)0x20000894 = 3;
  *(uint32_t*)0x20000898 = 0x7fffffff;
  *(uint32_t*)0x2000089c = 0xc21f;
  *(uint32_t*)0x200008a0 = 8;
  *(uint32_t*)0x200008a4 = 3;
  *(uint32_t*)0x200008a8 = 8;
  *(uint32_t*)0x200008ac = 3;
  *(uint32_t*)0x200008b0 = 0xffff0001;
  *(uint32_t*)0x200008b4 = 6;
  *(uint32_t*)0x200008b8 = 6;
  *(uint32_t*)0x200008bc = 6;
  *(uint32_t*)0x200008c0 = 6;
  *(uint32_t*)0x200008c4 = 0x8c;
  *(uint32_t*)0x200008c8 = 4;
  *(uint32_t*)0x200008cc = 0xff;
  *(uint32_t*)0x200008d0 = 0xfffffffe;
  *(uint32_t*)0x200008d4 = 9;
  *(uint32_t*)0x200008d8 = 0xffffff80;
  *(uint32_t*)0x200008dc = 2;
  *(uint32_t*)0x200008e0 = 0x15;
  *(uint32_t*)0x200008e4 = 1;
  *(uint32_t*)0x200008e8 = 0x80;
  *(uint32_t*)0x200008ec = 1;
  *(uint32_t*)0x200008f0 = 0x80000000;
  *(uint32_t*)0x200008f4 = 2;
  *(uint32_t*)0x200008f8 = 0x8da;
  *(uint32_t*)0x200008fc = 5;
  *(uint32_t*)0x20000900 = 0x8001;
  *(uint32_t*)0x20000904 = 0x91;
  *(uint32_t*)0x20000908 = 0x80;
  *(uint32_t*)0x2000090c = 0x6a3;
  *(uint32_t*)0x20000910 = 0x7e;
  *(uint32_t*)0x20000914 = 9;
  *(uint32_t*)0x20000918 = 4;
  *(uint32_t*)0x2000091c = -1;
  *(uint32_t*)0x20000920 = 7;
  *(uint32_t*)0x20000924 = 2;
  *(uint32_t*)0x20000928 = 2;
  *(uint32_t*)0x2000092c = 0x3f;
  *(uint32_t*)0x20000930 = 1;
  *(uint32_t*)0x20000934 = 0x1f;
  *(uint32_t*)0x20000938 = 4;
  *(uint32_t*)0x2000093c = 0x80;
  *(uint32_t*)0x20000940 = 0xfffffffa;
  *(uint32_t*)0x20000944 = 0x8001;
  *(uint32_t*)0x20000948 = 4;
  *(uint32_t*)0x2000094c = 0xffff1d75;
  *(uint32_t*)0x20000950 = 0x5e08;
  *(uint32_t*)0x20000954 = -1;
  *(uint32_t*)0x20000958 = 6;
  *(uint32_t*)0x2000095c = 6;
  *(uint32_t*)0x20000960 = 7;
  *(uint32_t*)0x20000964 = 3;
  *(uint32_t*)0x20000968 = 0x5b7;
  *(uint32_t*)0x2000096c = 6;
  *(uint32_t*)0x20000970 = 4;
  *(uint32_t*)0x20000974 = 6;
  *(uint32_t*)0x20000978 = 0xa9ca;
  *(uint32_t*)0x2000097c = 0x81;
  *(uint32_t*)0x20000980 = 0xc9;
  *(uint32_t*)0x20000984 = 0;
  *(uint32_t*)0x20000988 = 6;
  *(uint32_t*)0x2000098c = 5;
  *(uint32_t*)0x20000990 = 5;
  *(uint32_t*)0x20000994 = 0x79ca;
  *(uint32_t*)0x20000998 = 0x10000;
  *(uint32_t*)0x2000099c = 0xf;
  *(uint32_t*)0x200009a0 = 0x1ff800;
  *(uint32_t*)0x200009a4 = 0x1d12370f;
  *(uint32_t*)0x200009a8 = 0x6a;
  *(uint32_t*)0x200009ac = 7;
  *(uint32_t*)0x200009b0 = 2;
  *(uint32_t*)0x200009b4 = 3;
  *(uint32_t*)0x200009b8 = 9;
  *(uint32_t*)0x200009bc = 0xef6f;
  *(uint32_t*)0x200009c0 = 5;
  *(uint32_t*)0x200009c4 = 3;
  *(uint32_t*)0x200009c8 = 0xc58a;
  *(uint32_t*)0x200009cc = 1;
  *(uint32_t*)0x200009d0 = 0x7fff;
  *(uint32_t*)0x200009d4 = 0x800;
  *(uint32_t*)0x200009d8 = 9;
  *(uint32_t*)0x200009dc = 5;
  *(uint32_t*)0x200009e0 = 4;
  *(uint32_t*)0x200009e4 = 2;
  *(uint32_t*)0x200009e8 = 0x5d;
  *(uint32_t*)0x200009ec = 8;
  *(uint32_t*)0x200009f0 = 6;
  *(uint32_t*)0x200009f4 = 8;
  *(uint32_t*)0x200009f8 = 8;
  *(uint32_t*)0x200009fc = 2;
  *(uint32_t*)0x20000a00 = 0xfff;
  *(uint32_t*)0x20000a04 = 7;
  *(uint32_t*)0x20000a08 = 0x81;
  *(uint32_t*)0x20000a0c = 9;
  *(uint32_t*)0x20000a10 = 0x7ff;
  *(uint32_t*)0x20000a14 = 0xa8;
  *(uint32_t*)0x20000a18 = 0xffffff01;
  *(uint32_t*)0x20000a1c = 0x80;
  *(uint32_t*)0x20000a20 = 5;
  *(uint32_t*)0x20000a24 = 0xffff;
  *(uint32_t*)0x20000a28 = 0x401;
  *(uint32_t*)0x20000a2c = 6;
  *(uint32_t*)0x20000a30 = 6;
  *(uint32_t*)0x20000a34 = 0xffff;
  *(uint32_t*)0x20000a38 = 0x20;
  *(uint32_t*)0x20000a3c = 0x7f;
  *(uint32_t*)0x20000a40 = 0x800;
  *(uint32_t*)0x20000a44 = 0x4024;
  *(uint32_t*)0x20000a48 = 6;
  *(uint32_t*)0x20000a4c = 1;
  *(uint32_t*)0x20000a50 = 1;
  *(uint32_t*)0x20000a54 = 5;
  *(uint32_t*)0x20000a58 = 0x380000;
  *(uint32_t*)0x20000a5c = 1;
  *(uint32_t*)0x20000a60 = 0x400;
  *(uint32_t*)0x20000a64 = 0x20;
  *(uint32_t*)0x20000a68 = 5;
  *(uint32_t*)0x20000a6c = 0xcc1;
  *(uint32_t*)0x20000a70 = 2;
  *(uint32_t*)0x20000a74 = 0x51;
  *(uint32_t*)0x20000a78 = 0x100;
  *(uint32_t*)0x20000a7c = 2;
  *(uint32_t*)0x20000a80 = 3;
  *(uint32_t*)0x20000a84 = 0x43;
  *(uint32_t*)0x20000a88 = 6;
  *(uint32_t*)0x20000a8c = 0xfffffffd;
  *(uint32_t*)0x20000a90 = 0xa66;
  *(uint32_t*)0x20000a94 = 8;
  *(uint32_t*)0x20000a98 = 0xfff;
  *(uint32_t*)0x20000a9c = 9;
  *(uint32_t*)0x20000aa0 = 4;
  *(uint32_t*)0x20000aa4 = 8;
  *(uint32_t*)0x20000aa8 = 0x10001;
  *(uint32_t*)0x20000aac = 6;
  *(uint32_t*)0x20000ab0 = 1;
  *(uint32_t*)0x20000ab4 = 5;
  *(uint32_t*)0x20000ab8 = 0x401;
  *(uint32_t*)0x20000abc = 1;
  *(uint32_t*)0x20000ac0 = 3;
  *(uint32_t*)0x20000ac4 = 0x80000000;
  *(uint32_t*)0x20000ac8 = 7;
  *(uint32_t*)0x20000acc = 0xe52;
  *(uint32_t*)0x20000ad0 = 0xcc;
  *(uint32_t*)0x20000ad4 = 4;
  *(uint32_t*)0x20000ad8 = 1;
  *(uint32_t*)0x20000adc = 0x80;
  *(uint16_t*)0x20000ae0 = 0x404;
  *(uint16_t*)0x20000ae2 = 3;
  *(uint32_t*)0x20000ae4 = 0x7ff;
  *(uint32_t*)0x20000ae8 = 0x40;
  *(uint32_t*)0x20000aec = 1;
  *(uint32_t*)0x20000af0 = 5;
  *(uint32_t*)0x20000af4 = 5;
  *(uint32_t*)0x20000af8 = 8;
  *(uint32_t*)0x20000afc = 0;
  *(uint32_t*)0x20000b00 = 6;
  *(uint32_t*)0x20000b04 = 0x8001;
  *(uint32_t*)0x20000b08 = 0xfffffffd;
  *(uint32_t*)0x20000b0c = 0x80000001;
  *(uint32_t*)0x20000b10 = 0;
  *(uint32_t*)0x20000b14 = 4;
  *(uint32_t*)0x20000b18 = 0x7fff;
  *(uint32_t*)0x20000b1c = 0x7f;
  *(uint32_t*)0x20000b20 = 0x49;
  *(uint32_t*)0x20000b24 = 0x80000001;
  *(uint32_t*)0x20000b28 = 0x401;
  *(uint32_t*)0x20000b2c = 5;
  *(uint32_t*)0x20000b30 = 5;
  *(uint32_t*)0x20000b34 = 0x80000000;
  *(uint32_t*)0x20000b38 = 0x3f;
  *(uint32_t*)0x20000b3c = 7;
  *(uint32_t*)0x20000b40 = 5;
  *(uint32_t*)0x20000b44 = 0x5d324f93;
  *(uint32_t*)0x20000b48 = 0x80000000;
  *(uint32_t*)0x20000b4c = 4;
  *(uint32_t*)0x20000b50 = 9;
  *(uint32_t*)0x20000b54 = 1;
  *(uint32_t*)0x20000b58 = 0xfffffeff;
  *(uint32_t*)0x20000b5c = 5;
  *(uint32_t*)0x20000b60 = 0x5b7f;
  *(uint32_t*)0x20000b64 = 0x100;
  *(uint32_t*)0x20000b68 = 0x2b;
  *(uint32_t*)0x20000b6c = 7;
  *(uint32_t*)0x20000b70 = 6;
  *(uint32_t*)0x20000b74 = 0x5bb;
  *(uint32_t*)0x20000b78 = 0x1dd0;
  *(uint32_t*)0x20000b7c = 0x1f;
  *(uint32_t*)0x20000b80 = 0x80000001;
  *(uint32_t*)0x20000b84 = 0x20;
  *(uint32_t*)0x20000b88 = 7;
  *(uint32_t*)0x20000b8c = 0xfffff000;
  *(uint32_t*)0x20000b90 = 1;
  *(uint32_t*)0x20000b94 = 7;
  *(uint32_t*)0x20000b98 = 0x20;
  *(uint32_t*)0x20000b9c = 3;
  *(uint32_t*)0x20000ba0 = 0;
  *(uint32_t*)0x20000ba4 = 0;
  *(uint32_t*)0x20000ba8 = 0xffffff7f;
  *(uint32_t*)0x20000bac = 2;
  *(uint32_t*)0x20000bb0 = 0x40000;
  *(uint32_t*)0x20000bb4 = 8;
  *(uint32_t*)0x20000bb8 = 0xd3;
  *(uint32_t*)0x20000bbc = 0xe6;
  *(uint32_t*)0x20000bc0 = 0xff;
  *(uint32_t*)0x20000bc4 = 6;
  *(uint32_t*)0x20000bc8 = 0x7ff;
  *(uint32_t*)0x20000bcc = 0x40;
  *(uint32_t*)0x20000bd0 = 7;
  *(uint32_t*)0x20000bd4 = 0x10001;
  *(uint32_t*)0x20000bd8 = 0;
  *(uint32_t*)0x20000bdc = 2;
  *(uint32_t*)0x20000be0 = 0xfffffff7;
  *(uint32_t*)0x20000be4 = 3;
  *(uint32_t*)0x20000be8 = 5;
  *(uint32_t*)0x20000bec = 3;
  *(uint32_t*)0x20000bf0 = 0xd8;
  *(uint32_t*)0x20000bf4 = 0x20;
  *(uint32_t*)0x20000bf8 = 1;
  *(uint32_t*)0x20000bfc = 9;
  *(uint32_t*)0x20000c00 = 0xe;
  *(uint32_t*)0x20000c04 = 0xf1f;
  *(uint32_t*)0x20000c08 = 3;
  *(uint32_t*)0x20000c0c = 4;
  *(uint32_t*)0x20000c10 = 0;
  *(uint32_t*)0x20000c14 = 2;
  *(uint32_t*)0x20000c18 = 0xffff;
  *(uint32_t*)0x20000c1c = 3;
  *(uint32_t*)0x20000c20 = 0x3f;
  *(uint32_t*)0x20000c24 = 0x27;
  *(uint32_t*)0x20000c28 = 0x101;
  *(uint32_t*)0x20000c2c = 0x7fffffff;
  *(uint32_t*)0x20000c30 = 4;
  *(uint32_t*)0x20000c34 = -1;
  *(uint32_t*)0x20000c38 = 8;
  *(uint32_t*)0x20000c3c = 6;
  *(uint32_t*)0x20000c40 = 0xfa2;
  *(uint32_t*)0x20000c44 = 9;
  *(uint32_t*)0x20000c48 = 0xffff0000;
  *(uint32_t*)0x20000c4c = 0x4b6b;
  *(uint32_t*)0x20000c50 = 0xffff;
  *(uint32_t*)0x20000c54 = 0xff;
  *(uint32_t*)0x20000c58 = 7;
  *(uint32_t*)0x20000c5c = 0x4544;
  *(uint32_t*)0x20000c60 = 9;
  *(uint32_t*)0x20000c64 = 4;
  *(uint32_t*)0x20000c68 = 0x7fffffff;
  *(uint32_t*)0x20000c6c = 0x400;
  *(uint32_t*)0x20000c70 = 0x80000000;
  *(uint32_t*)0x20000c74 = 6;
  *(uint32_t*)0x20000c78 = 0x18cfe13f;
  *(uint32_t*)0x20000c7c = 0x80000001;
  *(uint32_t*)0x20000c80 = 0x7fff;
  *(uint32_t*)0x20000c84 = 8;
  *(uint32_t*)0x20000c88 = 0x8000;
  *(uint32_t*)0x20000c8c = 4;
  *(uint32_t*)0x20000c90 = 0;
  *(uint32_t*)0x20000c94 = 0x7e;
  *(uint32_t*)0x20000c98 = 1;
  *(uint32_t*)0x20000c9c = 1;
  *(uint32_t*)0x20000ca0 = 3;
  *(uint32_t*)0x20000ca4 = 8;
  *(uint32_t*)0x20000ca8 = 0xab0f;
  *(uint32_t*)0x20000cac = 8;
  *(uint32_t*)0x20000cb0 = 8;
  *(uint32_t*)0x20000cb4 = 0xfffffffe;
  *(uint32_t*)0x20000cb8 = 0;
  *(uint32_t*)0x20000cbc = 3;
  *(uint32_t*)0x20000cc0 = 0x5f17;
  *(uint32_t*)0x20000cc4 = 0xf3500000;
  *(uint32_t*)0x20000cc8 = 0xdf68;
  *(uint32_t*)0x20000ccc = 0xfff;
  *(uint32_t*)0x20000cd0 = 0x80000000;
  *(uint32_t*)0x20000cd4 = 0x3bb89d73;
  *(uint32_t*)0x20000cd8 = 0x800;
  *(uint32_t*)0x20000cdc = 0xa000;
  *(uint32_t*)0x20000ce0 = 9;
  *(uint32_t*)0x20000ce4 = 2;
  *(uint32_t*)0x20000ce8 = 0x4b53;
  *(uint32_t*)0x20000cec = 0x20;
  *(uint32_t*)0x20000cf0 = 7;
  *(uint32_t*)0x20000cf4 = 0;
  *(uint32_t*)0x20000cf8 = 0;
  *(uint32_t*)0x20000cfc = 0x1ff;
  *(uint32_t*)0x20000d00 = 0x3f;
  *(uint32_t*)0x20000d04 = 0;
  *(uint32_t*)0x20000d08 = 5;
  *(uint32_t*)0x20000d0c = 3;
  *(uint32_t*)0x20000d10 = 7;
  *(uint32_t*)0x20000d14 = 0x65e4bae7;
  *(uint32_t*)0x20000d18 = 0xb845;
  *(uint32_t*)0x20000d1c = 0x7ff;
  *(uint32_t*)0x20000d20 = 9;
  *(uint32_t*)0x20000d24 = 9;
  *(uint32_t*)0x20000d28 = 8;
  *(uint32_t*)0x20000d2c = 0x8000;
  *(uint32_t*)0x20000d30 = 2;
  *(uint32_t*)0x20000d34 = 5;
  *(uint32_t*)0x20000d38 = 0xb06e;
  *(uint32_t*)0x20000d3c = 0;
  *(uint32_t*)0x20000d40 = 0x10000;
  *(uint32_t*)0x20000d44 = -1;
  *(uint32_t*)0x20000d48 = 1;
  *(uint32_t*)0x20000d4c = 0xffff;
  *(uint32_t*)0x20000d50 = 4;
  *(uint32_t*)0x20000d54 = 0x7ff;
  *(uint32_t*)0x20000d58 = 0x8000;
  *(uint32_t*)0x20000d5c = 0xc36c;
  *(uint32_t*)0x20000d60 = 6;
  *(uint32_t*)0x20000d64 = 0x7fffffff;
  *(uint32_t*)0x20000d68 = 0x3ff;
  *(uint32_t*)0x20000d6c = 6;
  *(uint32_t*)0x20000d70 = 6;
  *(uint32_t*)0x20000d74 = 0x7fffffff;
  *(uint32_t*)0x20000d78 = 2;
  *(uint32_t*)0x20000d7c = 0x3f;
  *(uint32_t*)0x20000d80 = 3;
  *(uint32_t*)0x20000d84 = 0x7ff;
  *(uint32_t*)0x20000d88 = 0x4fa00955;
  *(uint32_t*)0x20000d8c = 7;
  *(uint32_t*)0x20000d90 = 9;
  *(uint32_t*)0x20000d94 = 0x51d633c7;
  *(uint32_t*)0x20000d98 = 0;
  *(uint32_t*)0x20000d9c = 0x696;
  *(uint32_t*)0x20000da0 = 6;
  *(uint32_t*)0x20000da4 = 0x28;
  *(uint32_t*)0x20000da8 = 0xdf5;
  *(uint32_t*)0x20000dac = 4;
  *(uint32_t*)0x20000db0 = 8;
  *(uint32_t*)0x20000db4 = 0x80000001;
  *(uint32_t*)0x20000db8 = 0;
  *(uint32_t*)0x20000dbc = 0;
  *(uint32_t*)0x20000dc0 = 9;
  *(uint32_t*)0x20000dc4 = 0x5ac;
  *(uint32_t*)0x20000dc8 = 0x7fff;
  *(uint32_t*)0x20000dcc = 0x10000;
  *(uint32_t*)0x20000dd0 = 4;
  *(uint32_t*)0x20000dd4 = 0xb8;
  *(uint32_t*)0x20000dd8 = 7;
  *(uint32_t*)0x20000ddc = 4;
  *(uint32_t*)0x20000de0 = 2;
  *(uint32_t*)0x20000de4 = 0xfffffff7;
  *(uint32_t*)0x20000de8 = 9;
  *(uint32_t*)0x20000dec = 0;
  *(uint32_t*)0x20000df0 = 0x1ff;
  *(uint32_t*)0x20000df4 = 4;
  *(uint32_t*)0x20000df8 = 0x8000;
  *(uint32_t*)0x20000dfc = 1;
  *(uint32_t*)0x20000e00 = 0xfff;
  *(uint32_t*)0x20000e04 = 0x8000;
  *(uint32_t*)0x20000e08 = 0xff;
  *(uint32_t*)0x20000e0c = 7;
  *(uint32_t*)0x20000e10 = 0xfff;
  *(uint32_t*)0x20000e14 = 0x81;
  *(uint32_t*)0x20000e18 = 0xe545;
  *(uint32_t*)0x20000e1c = 0x78083adb;
  *(uint32_t*)0x20000e20 = 0xffff;
  *(uint32_t*)0x20000e24 = 9;
  *(uint32_t*)0x20000e28 = 7;
  *(uint32_t*)0x20000e2c = 8;
  *(uint32_t*)0x20000e30 = 5;
  *(uint32_t*)0x20000e34 = 1;
  *(uint32_t*)0x20000e38 = 0x8000;
  *(uint32_t*)0x20000e3c = 9;
  *(uint32_t*)0x20000e40 = 0x1e7c595e;
  *(uint32_t*)0x20000e44 = 8;
  *(uint32_t*)0x20000e48 = 9;
  *(uint32_t*)0x20000e4c = 0xa9c;
  *(uint32_t*)0x20000e50 = 0;
  *(uint32_t*)0x20000e54 = 0x98b;
  *(uint32_t*)0x20000e58 = 0xfffffff8;
  *(uint32_t*)0x20000e5c = 9;
  *(uint32_t*)0x20000e60 = -1;
  *(uint32_t*)0x20000e64 = 3;
  *(uint32_t*)0x20000e68 = 1;
  *(uint32_t*)0x20000e6c = 9;
  *(uint32_t*)0x20000e70 = 1;
  *(uint32_t*)0x20000e74 = 7;
  *(uint32_t*)0x20000e78 = 0x133a;
  *(uint32_t*)0x20000e7c = 7;
  *(uint32_t*)0x20000e80 = 0x6d1f;
  *(uint32_t*)0x20000e84 = 0x7b9c;
  *(uint32_t*)0x20000e88 = 0xffffc30c;
  *(uint32_t*)0x20000e8c = 5;
  *(uint32_t*)0x20000e90 = 0x2c27;
  *(uint32_t*)0x20000e94 = 6;
  *(uint32_t*)0x20000e98 = 0x134;
  *(uint32_t*)0x20000e9c = 0;
  *(uint32_t*)0x20000ea0 = 9;
  *(uint32_t*)0x20000ea4 = 8;
  *(uint32_t*)0x20000ea8 = 0xffff4983;
  *(uint32_t*)0x20000eac = 0x1000;
  *(uint32_t*)0x20000eb0 = 0x401;
  *(uint32_t*)0x20000eb4 = 6;
  *(uint32_t*)0x20000eb8 = 6;
  *(uint32_t*)0x20000ebc = 6;
  *(uint32_t*)0x20000ec0 = 5;
  *(uint32_t*)0x20000ec4 = 1;
  *(uint32_t*)0x20000ec8 = 4;
  *(uint32_t*)0x20000ecc = 8;
  *(uint32_t*)0x20000ed0 = 0x9d13;
  *(uint32_t*)0x20000ed4 = 0x101;
  *(uint32_t*)0x20000ed8 = 0xfffffc00;
  *(uint32_t*)0x20000edc = 7;
  *(uint32_t*)0x20000ee0 = 0x80000000;
  *(uint16_t*)0x20000ee4 = 0x404;
  *(uint16_t*)0x20000ee6 = 2;
  *(uint32_t*)0x20000ee8 = 3;
  *(uint32_t*)0x20000eec = 0xa711;
  *(uint32_t*)0x20000ef0 = 0;
  *(uint32_t*)0x20000ef4 = 0xced;
  *(uint32_t*)0x20000ef8 = 9;
  *(uint32_t*)0x20000efc = 0xfef2;
  *(uint32_t*)0x20000f00 = 0x3ff;
  *(uint32_t*)0x20000f04 = 0xfffeffff;
  *(uint32_t*)0x20000f08 = 7;
  *(uint32_t*)0x20000f0c = 0x401;
  *(uint32_t*)0x20000f10 = 0x101;
  *(uint32_t*)0x20000f14 = 0x40;
  *(uint32_t*)0x20000f18 = 1;
  *(uint32_t*)0x20000f1c = 7;
  *(uint32_t*)0x20000f20 = 0xbd95f56;
  *(uint32_t*)0x20000f24 = 7;
  *(uint32_t*)0x20000f28 = 9;
  *(uint32_t*)0x20000f2c = 0x401;
  *(uint32_t*)0x20000f30 = 2;
  *(uint32_t*)0x20000f34 = 0x101;
  *(uint32_t*)0x20000f38 = 7;
  *(uint32_t*)0x20000f3c = 7;
  *(uint32_t*)0x20000f40 = 0x7fffffff;
  *(uint32_t*)0x20000f44 = 0x30f;
  *(uint32_t*)0x20000f48 = 0x200;
  *(uint32_t*)0x20000f4c = 0x3f;
  *(uint32_t*)0x20000f50 = 5;
  *(uint32_t*)0x20000f54 = 0x9edf;
  *(uint32_t*)0x20000f58 = 5;
  *(uint32_t*)0x20000f5c = 0x20d;
  *(uint32_t*)0x20000f60 = 0;
  *(uint32_t*)0x20000f64 = 9;
  *(uint32_t*)0x20000f68 = 0;
  *(uint32_t*)0x20000f6c = 8;
  *(uint32_t*)0x20000f70 = 6;
  *(uint32_t*)0x20000f74 = 0x80000000;
  *(uint32_t*)0x20000f78 = 0x10000;
  *(uint32_t*)0x20000f7c = 0x200;
  *(uint32_t*)0x20000f80 = 8;
  *(uint32_t*)0x20000f84 = 6;
  *(uint32_t*)0x20000f88 = 4;
  *(uint32_t*)0x20000f8c = 0x727;
  *(uint32_t*)0x20000f90 = 4;
  *(uint32_t*)0x20000f94 = 5;
  *(uint32_t*)0x20000f98 = 9;
  *(uint32_t*)0x20000f9c = 8;
  *(uint32_t*)0x20000fa0 = 0xeb;
  *(uint32_t*)0x20000fa4 = 0x80;
  *(uint32_t*)0x20000fa8 = 0;
  *(uint32_t*)0x20000fac = 6;
  *(uint32_t*)0x20000fb0 = 0x100;
  *(uint32_t*)0x20000fb4 = 6;
  *(uint32_t*)0x20000fb8 = 0xffffff9e;
  *(uint32_t*)0x20000fbc = 0x3f;
  *(uint32_t*)0x20000fc0 = 5;
  *(uint32_t*)0x20000fc4 = 0;
  *(uint32_t*)0x20000fc8 = 0x8e2;
  *(uint32_t*)0x20000fcc = 0xfffffffc;
  *(uint32_t*)0x20000fd0 = 0x34;
  *(uint32_t*)0x20000fd4 = 3;
  *(uint32_t*)0x20000fd8 = 0x7fff;
  *(uint32_t*)0x20000fdc = 0x101;
  *(uint32_t*)0x20000fe0 = 9;
  *(uint32_t*)0x20000fe4 = 6;
  *(uint32_t*)0x20000fe8 = 0xe48d;
  *(uint32_t*)0x20000fec = 5;
  *(uint32_t*)0x20000ff0 = 4;
  *(uint32_t*)0x20000ff4 = 0;
  *(uint32_t*)0x20000ff8 = 0;
  *(uint32_t*)0x20000ffc = 3;
  *(uint32_t*)0x20001000 = 0x34f;
  *(uint32_t*)0x20001004 = 0x40;
  *(uint32_t*)0x20001008 = 5;
  *(uint32_t*)0x2000100c = 0x10001;
  *(uint32_t*)0x20001010 = 4;
  *(uint32_t*)0x20001014 = 0x7fff;
  *(uint32_t*)0x20001018 = 0x80;
  *(uint32_t*)0x2000101c = 0x968;
  *(uint32_t*)0x20001020 = 1;
  *(uint32_t*)0x20001024 = 0x7f;
  *(uint32_t*)0x20001028 = 1;
  *(uint32_t*)0x2000102c = 0x1ff;
  *(uint32_t*)0x20001030 = 0x7f;
  *(uint32_t*)0x20001034 = 0x3ff;
  *(uint32_t*)0x20001038 = 0x800;
  *(uint32_t*)0x2000103c = 0;
  *(uint32_t*)0x20001040 = 0x3d;
  *(uint32_t*)0x20001044 = 5;
  *(uint32_t*)0x20001048 = 6;
  *(uint32_t*)0x2000104c = 0x86;
  *(uint32_t*)0x20001050 = 0x3f;
  *(uint32_t*)0x20001054 = 1;
  *(uint32_t*)0x20001058 = 0x80000000;
  *(uint32_t*)0x2000105c = 5;
  *(uint32_t*)0x20001060 = 8;
  *(uint32_t*)0x20001064 = 1;
  *(uint32_t*)0x20001068 = 3;
  *(uint32_t*)0x2000106c = 5;
  *(uint32_t*)0x20001070 = 0xd6;
  *(uint32_t*)0x20001074 = 0x800;
  *(uint32_t*)0x20001078 = 1;
  *(uint32_t*)0x2000107c = 4;
  *(uint32_t*)0x20001080 = 4;
  *(uint32_t*)0x20001084 = 1;
  *(uint32_t*)0x20001088 = 9;
  *(uint32_t*)0x2000108c = 8;
  *(uint32_t*)0x20001090 = 0;
  *(uint32_t*)0x20001094 = 0;
  *(uint32_t*)0x20001098 = 4;
  *(uint32_t*)0x2000109c = 5;
  *(uint32_t*)0x200010a0 = 0;
  *(uint32_t*)0x200010a4 = 0;
  *(uint32_t*)0x200010a8 = 4;
  *(uint32_t*)0x200010ac = 0xbd2;
  *(uint32_t*)0x200010b0 = 5;
  *(uint32_t*)0x200010b4 = 0xefc0;
  *(uint32_t*)0x200010b8 = 0xfad;
  *(uint32_t*)0x200010bc = 2;
  *(uint32_t*)0x200010c0 = 0x50000000;
  *(uint32_t*)0x200010c4 = 4;
  *(uint32_t*)0x200010c8 = 5;
  *(uint32_t*)0x200010cc = 0xf4;
  *(uint32_t*)0x200010d0 = 5;
  *(uint32_t*)0x200010d4 = 0x80;
  *(uint32_t*)0x200010d8 = 1;
  *(uint32_t*)0x200010dc = 0x3f;
  *(uint32_t*)0x200010e0 = 2;
  *(uint32_t*)0x200010e4 = 8;
  *(uint32_t*)0x200010e8 = 0x3ff;
  *(uint32_t*)0x200010ec = 7;
  *(uint32_t*)0x200010f0 = 6;
  *(uint32_t*)0x200010f4 = 0xffff7459;
  *(uint32_t*)0x200010f8 = 6;
  *(uint32_t*)0x200010fc = 0x7fff;
  *(uint32_t*)0x20001100 = 3;
  *(uint32_t*)0x20001104 = 0x10000;
  *(uint32_t*)0x20001108 = 1;
  *(uint32_t*)0x2000110c = 0xeb3bca3;
  *(uint32_t*)0x20001110 = 3;
  *(uint32_t*)0x20001114 = 0x20;
  *(uint32_t*)0x20001118 = 0x40;
  *(uint32_t*)0x2000111c = 2;
  *(uint32_t*)0x20001120 = 1;
  *(uint32_t*)0x20001124 = 0x200;
  *(uint32_t*)0x20001128 = 2;
  *(uint32_t*)0x2000112c = 0x80000001;
  *(uint32_t*)0x20001130 = 5;
  *(uint32_t*)0x20001134 = 0x40;
  *(uint32_t*)0x20001138 = 0x400000;
  *(uint32_t*)0x2000113c = 0x8000;
  *(uint32_t*)0x20001140 = 0x643;
  *(uint32_t*)0x20001144 = 0x40;
  *(uint32_t*)0x20001148 = 7;
  *(uint32_t*)0x2000114c = 0;
  *(uint32_t*)0x20001150 = 0x5a4;
  *(uint32_t*)0x20001154 = 0x7fff;
  *(uint32_t*)0x20001158 = 6;
  *(uint32_t*)0x2000115c = 0;
  *(uint32_t*)0x20001160 = -1;
  *(uint32_t*)0x20001164 = 0x7f;
  *(uint32_t*)0x20001168 = 0xfff;
  *(uint32_t*)0x2000116c = 0x81;
  *(uint32_t*)0x20001170 = 9;
  *(uint32_t*)0x20001174 = 0xce54;
  *(uint32_t*)0x20001178 = 0x84000000;
  *(uint32_t*)0x2000117c = 0x40000000;
  *(uint32_t*)0x20001180 = 0x10001;
  *(uint32_t*)0x20001184 = 5;
  *(uint32_t*)0x20001188 = 0x871;
  *(uint32_t*)0x2000118c = -1;
  *(uint32_t*)0x20001190 = 0xfff;
  *(uint32_t*)0x20001194 = 4;
  *(uint32_t*)0x20001198 = 0xfff;
  *(uint32_t*)0x2000119c = 0x80000001;
  *(uint32_t*)0x200011a0 = 0x33;
  *(uint32_t*)0x200011a4 = 7;
  *(uint32_t*)0x200011a8 = 8;
  *(uint32_t*)0x200011ac = 4;
  *(uint32_t*)0x200011b0 = 0x62;
  *(uint32_t*)0x200011b4 = 0x400;
  *(uint32_t*)0x200011b8 = 4;
  *(uint32_t*)0x200011bc = 0xff;
  *(uint32_t*)0x200011c0 = 0x800;
  *(uint32_t*)0x200011c4 = 0x7fff;
  *(uint32_t*)0x200011c8 = 4;
  *(uint32_t*)0x200011cc = 0x9d7e;
  *(uint32_t*)0x200011d0 = 0x81;
  *(uint32_t*)0x200011d4 = 0xc00;
  *(uint32_t*)0x200011d8 = 8;
  *(uint32_t*)0x200011dc = 0x10000;
  *(uint32_t*)0x200011e0 = 0x3f;
  *(uint32_t*)0x200011e4 = 0x84;
  *(uint32_t*)0x200011e8 = 0xfff;
  *(uint32_t*)0x200011ec = 0;
  *(uint32_t*)0x200011f0 = 0xec2;
  *(uint32_t*)0x200011f4 = 0;
  *(uint32_t*)0x200011f8 = 0;
  *(uint32_t*)0x200011fc = 0x7ff;
  *(uint32_t*)0x20001200 = 2;
  *(uint32_t*)0x20001204 = 0x133;
  *(uint32_t*)0x20001208 = 0x8000;
  *(uint32_t*)0x2000120c = 0;
  *(uint32_t*)0x20001210 = 0x80;
  *(uint32_t*)0x20001214 = 0xf2;
  *(uint32_t*)0x20001218 = 1;
  *(uint32_t*)0x2000121c = 5;
  *(uint32_t*)0x20001220 = 0x4945;
  *(uint32_t*)0x20001224 = 0x1f;
  *(uint32_t*)0x20001228 = 7;
  *(uint32_t*)0x2000122c = 0x7fff;
  *(uint32_t*)0x20001230 = 0x81;
  *(uint32_t*)0x20001234 = 1;
  *(uint32_t*)0x20001238 = 4;
  *(uint32_t*)0x2000123c = 0x8000;
  *(uint32_t*)0x20001240 = 0x10001;
  *(uint32_t*)0x20001244 = 1;
  *(uint32_t*)0x20001248 = 0x6a67;
  *(uint32_t*)0x2000124c = 8;
  *(uint32_t*)0x20001250 = 0x3ff;
  *(uint32_t*)0x20001254 = 0xec7;
  *(uint32_t*)0x20001258 = 6;
  *(uint32_t*)0x2000125c = 2;
  *(uint32_t*)0x20001260 = 0x1788d8b4;
  *(uint32_t*)0x20001264 = 0xf7;
  *(uint32_t*)0x20001268 = 3;
  *(uint32_t*)0x2000126c = 0xfffffffb;
  *(uint32_t*)0x20001270 = 0x10001;
  *(uint32_t*)0x20001274 = 5;
  *(uint32_t*)0x20001278 = 9;
  *(uint32_t*)0x2000127c = 0x80;
  *(uint32_t*)0x20001280 = 0xffffff80;
  *(uint32_t*)0x20001284 = 0xde6;
  *(uint32_t*)0x20001288 = 5;
  *(uint32_t*)0x2000128c = 0x56d;
  *(uint32_t*)0x20001290 = 1;
  *(uint32_t*)0x20001294 = 0xdc;
  *(uint32_t*)0x20001298 = 0xba6;
  *(uint32_t*)0x2000129c = 0x159b3756;
  *(uint32_t*)0x200012a0 = 0x1000;
  *(uint32_t*)0x200012a4 = 0;
  *(uint32_t*)0x200012a8 = 6;
  *(uint32_t*)0x200012ac = 0x8000;
  *(uint32_t*)0x200012b0 = 0;
  *(uint32_t*)0x200012b4 = 1;
  *(uint32_t*)0x200012b8 = 0xbe3;
  *(uint32_t*)0x200012bc = 0x630;
  *(uint32_t*)0x200012c0 = 1;
  *(uint32_t*)0x200012c4 = 3;
  *(uint32_t*)0x200012c8 = 2;
  *(uint32_t*)0x200012cc = 2;
  *(uint32_t*)0x200012d0 = 7;
  *(uint32_t*)0x200012d4 = 0x81;
  *(uint32_t*)0x200012d8 = 0x80;
  *(uint32_t*)0x200012dc = 0xdfab;
  *(uint32_t*)0x200012e0 = 0x1000;
  *(uint32_t*)0x200012e4 = 0x39;
  *(uint64_t*)0x20000188 = 0xce8;
  *(uint64_t*)0x200001d8 = 1;
  *(uint64_t*)0x200001e0 = 0;
  *(uint64_t*)0x200001e8 = 0;
  *(uint32_t*)0x200001f0 = 0;
  syscall(__NR_sendmsg, r[0], 0x200001c0ul, 0ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  do_sandbox_none();
  return 0;
}
