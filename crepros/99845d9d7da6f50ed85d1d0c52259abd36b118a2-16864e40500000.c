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

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0x0};

void execute_one(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x1000000010ul, 0x80002ul, 0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_socket, 0x10ul, 0x803ul, 0);
  if (res != -1)
    r[1] = res;
  *(uint64_t*)0x20000280 = 0;
  *(uint32_t*)0x20000288 = 0;
  *(uint64_t*)0x20000290 = 0x20000180;
  *(uint64_t*)0x20000180 = 0;
  *(uint64_t*)0x20000188 = 0;
  *(uint64_t*)0x20000298 = 1;
  *(uint64_t*)0x200002a0 = 0;
  *(uint64_t*)0x200002a8 = 0;
  *(uint32_t*)0x200002b0 = 0;
  syscall(__NR_sendmsg, r[1], 0x20000280ul, 0ul);
  *(uint32_t*)0x20000200 = 0x14;
  res = syscall(__NR_getsockname, r[1], 0x20000100ul, 0x20000200ul);
  if (res != -1)
    r[2] = *(uint32_t*)0x20000104;
  *(uint64_t*)0x20000240 = 0;
  *(uint32_t*)0x20000248 = 0;
  *(uint64_t*)0x20000250 = 0x20000140;
  *(uint64_t*)0x20000140 = 0x20000080;
  memcpy((void*)0x20000080, "\x38\x00\x00\x00\x24\x00\x07\x05\x00\x00\x00\x40"
                            "\x07\xa2\xa3\x00\x05\x00\x00\x00",
         20);
  *(uint32_t*)0x20000094 = r[2];
  memcpy((void*)0x20000098, "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00"
                            "\x09\x00\x01\x00\x68\x66\x73\x63\x00\x00\x00\x00"
                            "\x08\x00\x02",
         27);
  *(uint64_t*)0x20000148 = 0x38;
  *(uint64_t*)0x20000258 = 1;
  *(uint64_t*)0x20000260 = 0;
  *(uint64_t*)0x20000268 = 0;
  *(uint32_t*)0x20000270 = 0;
  syscall(__NR_sendmsg, -1, 0x20000240ul, 0ul);
  *(uint64_t*)0x20000440 = 0;
  *(uint32_t*)0x20000448 = 0;
  *(uint64_t*)0x20000450 = 0x20000180;
  *(uint64_t*)0x20000180 = 0x20000700;
  *(uint32_t*)0x20000700 = 0x8d4;
  *(uint16_t*)0x20000704 = 0x2c;
  *(uint16_t*)0x20000706 = 0xd27;
  *(uint32_t*)0x20000708 = 0;
  *(uint32_t*)0x2000070c = 0;
  *(uint8_t*)0x20000710 = 0;
  *(uint8_t*)0x20000711 = 0;
  *(uint16_t*)0x20000712 = 0;
  *(uint32_t*)0x20000714 = r[2];
  *(uint16_t*)0x20000718 = 0;
  *(uint16_t*)0x2000071a = 0;
  *(uint16_t*)0x2000071c = 0;
  *(uint16_t*)0x2000071e = 0;
  *(uint16_t*)0x20000720 = 4;
  *(uint16_t*)0x20000722 = 0;
  *(uint16_t*)0x20000724 = 8;
  *(uint16_t*)0x20000726 = 1;
  memcpy((void*)0x20000728, "u32\000", 4);
  *(uint16_t*)0x2000072c = 0x8a8;
  *(uint16_t*)0x2000072e = 2;
  *(uint16_t*)0x20000730 = 0x890;
  *(uint16_t*)0x20000732 = 6;
  *(uint16_t*)0x20000734 = 0x3c;
  *(uint16_t*)0x20000736 = 1;
  *(uint32_t*)0x20000738 = 0;
  *(uint32_t*)0x2000073c = 0;
  *(uint32_t*)0x20000740 = 0;
  *(uint32_t*)0x20000744 = 0;
  *(uint32_t*)0x20000748 = 0;
  *(uint8_t*)0x2000074c = 0;
  *(uint8_t*)0x2000074d = 0;
  *(uint16_t*)0x2000074e = 0;
  *(uint16_t*)0x20000750 = 0;
  *(uint16_t*)0x20000752 = 0;
  *(uint32_t*)0x20000754 = 0;
  *(uint8_t*)0x20000758 = 0;
  *(uint8_t*)0x20000759 = 0;
  *(uint16_t*)0x2000075a = 0;
  *(uint16_t*)0x2000075c = 0;
  *(uint16_t*)0x2000075e = 0;
  *(uint32_t*)0x20000760 = 0;
  *(uint32_t*)0x20000764 = 0;
  *(uint32_t*)0x20000768 = 0;
  *(uint32_t*)0x2000076c = 0;
  *(uint16_t*)0x20000770 = 0x404;
  *(uint16_t*)0x20000772 = 2;
  *(uint32_t*)0x20000774 = 1;
  *(uint32_t*)0x20000778 = 8;
  *(uint32_t*)0x2000077c = 7;
  *(uint32_t*)0x20000780 = 1;
  *(uint32_t*)0x20000784 = 9;
  *(uint32_t*)0x20000788 = 3;
  *(uint32_t*)0x2000078c = 7;
  *(uint32_t*)0x20000790 = 0x76;
  *(uint32_t*)0x20000794 = 0;
  *(uint32_t*)0x20000798 = 0x8001;
  *(uint32_t*)0x2000079c = 0;
  *(uint32_t*)0x200007a0 = 2;
  *(uint32_t*)0x200007a4 = 7;
  *(uint32_t*)0x200007a8 = 0x1000;
  *(uint32_t*)0x200007ac = 7;
  *(uint32_t*)0x200007b0 = 0x7f;
  *(uint32_t*)0x200007b4 = 3;
  *(uint32_t*)0x200007b8 = 0x100;
  *(uint32_t*)0x200007bc = 0x40;
  *(uint32_t*)0x200007c0 = 9;
  *(uint32_t*)0x200007c4 = 4;
  *(uint32_t*)0x200007c8 = 0x964;
  *(uint32_t*)0x200007cc = 3;
  *(uint32_t*)0x200007d0 = 1;
  *(uint32_t*)0x200007d4 = 6;
  *(uint32_t*)0x200007d8 = 0x477c;
  *(uint32_t*)0x200007dc = 7;
  *(uint32_t*)0x200007e0 = 0x2e8;
  *(uint32_t*)0x200007e4 = 0x12b;
  *(uint32_t*)0x200007e8 = 0xcd;
  *(uint32_t*)0x200007ec = 0x400;
  *(uint32_t*)0x200007f0 = 0x101;
  *(uint32_t*)0x200007f4 = 0x7fffffff;
  *(uint32_t*)0x200007f8 = 1;
  *(uint32_t*)0x200007fc = 1;
  *(uint32_t*)0x20000800 = 4;
  *(uint32_t*)0x20000804 = 0xffffff7f;
  *(uint32_t*)0x20000808 = 0x7ff;
  *(uint32_t*)0x2000080c = 0x704e96e4;
  *(uint32_t*)0x20000810 = 0x7a7;
  *(uint32_t*)0x20000814 = 0x7f9;
  *(uint32_t*)0x20000818 = 0x80000000;
  *(uint32_t*)0x2000081c = -1;
  *(uint32_t*)0x20000820 = 0xfffffff8;
  *(uint32_t*)0x20000824 = 0;
  *(uint32_t*)0x20000828 = 1;
  *(uint32_t*)0x2000082c = 0x1000;
  *(uint32_t*)0x20000830 = 0xd9a5;
  *(uint32_t*)0x20000834 = 3;
  *(uint32_t*)0x20000838 = 0x81;
  *(uint32_t*)0x2000083c = 0;
  *(uint32_t*)0x20000840 = 0xffff;
  *(uint32_t*)0x20000844 = 1;
  *(uint32_t*)0x20000848 = 2;
  *(uint32_t*)0x2000084c = 7;
  *(uint32_t*)0x20000850 = 3;
  *(uint32_t*)0x20000854 = 0x1ff;
  *(uint32_t*)0x20000858 = 4;
  *(uint32_t*)0x2000085c = 0;
  *(uint32_t*)0x20000860 = 2;
  *(uint32_t*)0x20000864 = 2;
  *(uint32_t*)0x20000868 = 0x7b5;
  *(uint32_t*)0x2000086c = 5;
  *(uint32_t*)0x20000870 = 1;
  *(uint32_t*)0x20000874 = 0x8821;
  *(uint32_t*)0x20000878 = 5;
  *(uint32_t*)0x2000087c = 6;
  *(uint32_t*)0x20000880 = 0x7fffffff;
  *(uint32_t*)0x20000884 = 0x64276e89;
  *(uint32_t*)0x20000888 = 2;
  *(uint32_t*)0x2000088c = 0x3ff;
  *(uint32_t*)0x20000890 = 0x7fff;
  *(uint32_t*)0x20000894 = 0;
  *(uint32_t*)0x20000898 = 0x100;
  *(uint32_t*)0x2000089c = 6;
  *(uint32_t*)0x200008a0 = 0x3ff;
  *(uint32_t*)0x200008a4 = 2;
  *(uint32_t*)0x200008a8 = 5;
  *(uint32_t*)0x200008ac = 0xb221;
  *(uint32_t*)0x200008b0 = 0;
  *(uint32_t*)0x200008b4 = 0xc9;
  *(uint32_t*)0x200008b8 = 0x7fff;
  *(uint32_t*)0x200008bc = 1;
  *(uint32_t*)0x200008c0 = 6;
  *(uint32_t*)0x200008c4 = 0;
  *(uint32_t*)0x200008c8 = 2;
  *(uint32_t*)0x200008cc = 0xffff9907;
  *(uint32_t*)0x200008d0 = 8;
  *(uint32_t*)0x200008d4 = 0x9b;
  *(uint32_t*)0x200008d8 = 9;
  *(uint32_t*)0x200008dc = 0x57;
  *(uint32_t*)0x200008e0 = 0x1ff;
  *(uint32_t*)0x200008e4 = 6;
  *(uint32_t*)0x200008e8 = 0x20b;
  *(uint32_t*)0x200008ec = 0x48c7;
  *(uint32_t*)0x200008f0 = 0x400;
  *(uint32_t*)0x200008f4 = 0x10001;
  *(uint32_t*)0x200008f8 = 0x31d;
  *(uint32_t*)0x200008fc = 6;
  *(uint32_t*)0x20000900 = 0x20;
  *(uint32_t*)0x20000904 = 0x8d;
  *(uint32_t*)0x20000908 = 9;
  *(uint32_t*)0x2000090c = 0x5c3;
  *(uint32_t*)0x20000910 = 9;
  *(uint32_t*)0x20000914 = 4;
  *(uint32_t*)0x20000918 = 0x9a2;
  *(uint32_t*)0x2000091c = 3;
  *(uint32_t*)0x20000920 = 6;
  *(uint32_t*)0x20000924 = 6;
  *(uint32_t*)0x20000928 = 0x80000001;
  *(uint32_t*)0x2000092c = 0x3f;
  *(uint32_t*)0x20000930 = -1;
  *(uint32_t*)0x20000934 = 0x391;
  *(uint32_t*)0x20000938 = 9;
  *(uint32_t*)0x2000093c = 0x43f2;
  *(uint32_t*)0x20000940 = 0x7fff;
  *(uint32_t*)0x20000944 = 0x3f89;
  *(uint32_t*)0x20000948 = 5;
  *(uint32_t*)0x2000094c = 0xe53;
  *(uint32_t*)0x20000950 = 9;
  *(uint32_t*)0x20000954 = 7;
  *(uint32_t*)0x20000958 = 0x200;
  *(uint32_t*)0x2000095c = 0xff;
  *(uint32_t*)0x20000960 = 0x7f;
  *(uint32_t*)0x20000964 = 6;
  *(uint32_t*)0x20000968 = 0xffffffc1;
  *(uint32_t*)0x2000096c = 0x40;
  *(uint32_t*)0x20000970 = 0x81;
  *(uint32_t*)0x20000974 = 0x1ff;
  *(uint32_t*)0x20000978 = 0x1000;
  *(uint32_t*)0x2000097c = 7;
  *(uint32_t*)0x20000980 = 0x1ff;
  *(uint32_t*)0x20000984 = 0x80ab;
  *(uint32_t*)0x20000988 = 0x8e;
  *(uint32_t*)0x2000098c = 0;
  *(uint32_t*)0x20000990 = 2;
  *(uint32_t*)0x20000994 = 0xfffffffa;
  *(uint32_t*)0x20000998 = 0xaee;
  *(uint32_t*)0x2000099c = 0x7fff;
  *(uint32_t*)0x200009a0 = 7;
  *(uint32_t*)0x200009a4 = 7;
  *(uint32_t*)0x200009a8 = 0xffff;
  *(uint32_t*)0x200009ac = 6;
  *(uint32_t*)0x200009b0 = 4;
  *(uint32_t*)0x200009b4 = 2;
  *(uint32_t*)0x200009b8 = 0xec7;
  *(uint32_t*)0x200009bc = 0x7fff;
  *(uint32_t*)0x200009c0 = 0x7f;
  *(uint32_t*)0x200009c4 = 5;
  *(uint32_t*)0x200009c8 = 0;
  *(uint32_t*)0x200009cc = 2;
  *(uint32_t*)0x200009d0 = 0xfffffffe;
  *(uint32_t*)0x200009d4 = 8;
  *(uint32_t*)0x200009d8 = 1;
  *(uint32_t*)0x200009dc = 0xc0000000;
  *(uint32_t*)0x200009e0 = 0xc6;
  *(uint32_t*)0x200009e4 = 6;
  *(uint32_t*)0x200009e8 = 0x28000;
  *(uint32_t*)0x200009ec = 0;
  *(uint32_t*)0x200009f0 = 6;
  *(uint32_t*)0x200009f4 = 7;
  *(uint32_t*)0x200009f8 = 0x1f;
  *(uint32_t*)0x200009fc = 2;
  *(uint32_t*)0x20000a00 = 1;
  *(uint32_t*)0x20000a04 = 7;
  *(uint32_t*)0x20000a08 = 5;
  *(uint32_t*)0x20000a0c = 0;
  *(uint32_t*)0x20000a10 = 1;
  *(uint32_t*)0x20000a14 = 0;
  *(uint32_t*)0x20000a18 = 0;
  *(uint32_t*)0x20000a1c = 4;
  *(uint32_t*)0x20000a20 = 0x40;
  *(uint32_t*)0x20000a24 = 0xffff;
  *(uint32_t*)0x20000a28 = 0x100;
  *(uint32_t*)0x20000a2c = 4;
  *(uint32_t*)0x20000a30 = 0xf3e8;
  *(uint32_t*)0x20000a34 = 6;
  *(uint32_t*)0x20000a38 = 0x20;
  *(uint32_t*)0x20000a3c = 0x8001;
  *(uint32_t*)0x20000a40 = 1;
  *(uint32_t*)0x20000a44 = 0xfffff30a;
  *(uint32_t*)0x20000a48 = 0x7c4;
  *(uint32_t*)0x20000a4c = 9;
  *(uint32_t*)0x20000a50 = 0x6c68;
  *(uint32_t*)0x20000a54 = 0x7fffffff;
  *(uint32_t*)0x20000a58 = 9;
  *(uint32_t*)0x20000a5c = 9;
  *(uint32_t*)0x20000a60 = 0x7fff;
  *(uint32_t*)0x20000a64 = 5;
  *(uint32_t*)0x20000a68 = 0x56d8;
  *(uint32_t*)0x20000a6c = 0x29d8;
  *(uint32_t*)0x20000a70 = 0;
  *(uint32_t*)0x20000a74 = 8;
  *(uint32_t*)0x20000a78 = 0xd14;
  *(uint32_t*)0x20000a7c = 0;
  *(uint32_t*)0x20000a80 = 0x6c3f6949;
  *(uint32_t*)0x20000a84 = 6;
  *(uint32_t*)0x20000a88 = 0x1ff;
  *(uint32_t*)0x20000a8c = 7;
  *(uint32_t*)0x20000a90 = 2;
  *(uint32_t*)0x20000a94 = 3;
  *(uint32_t*)0x20000a98 = 0x46;
  *(uint32_t*)0x20000a9c = 0x3f;
  *(uint32_t*)0x20000aa0 = 6;
  *(uint32_t*)0x20000aa4 = 5;
  *(uint32_t*)0x20000aa8 = 0xfffffffe;
  *(uint32_t*)0x20000aac = 4;
  *(uint32_t*)0x20000ab0 = 0xff;
  *(uint32_t*)0x20000ab4 = 3;
  *(uint32_t*)0x20000ab8 = 2;
  *(uint32_t*)0x20000abc = 3;
  *(uint32_t*)0x20000ac0 = 3;
  *(uint32_t*)0x20000ac4 = 8;
  *(uint32_t*)0x20000ac8 = 4;
  *(uint32_t*)0x20000acc = 6;
  *(uint32_t*)0x20000ad0 = 4;
  *(uint32_t*)0x20000ad4 = 0xfffffff7;
  *(uint32_t*)0x20000ad8 = 8;
  *(uint32_t*)0x20000adc = 6;
  *(uint32_t*)0x20000ae0 = 0x134;
  *(uint32_t*)0x20000ae4 = 2;
  *(uint32_t*)0x20000ae8 = 0xe8e;
  *(uint32_t*)0x20000aec = 0x800;
  *(uint32_t*)0x20000af0 = 0x7ff;
  *(uint32_t*)0x20000af4 = 6;
  *(uint32_t*)0x20000af8 = 0;
  *(uint32_t*)0x20000afc = 0x1ff;
  *(uint32_t*)0x20000b00 = 0x80000000;
  *(uint32_t*)0x20000b04 = 0x41fa;
  *(uint32_t*)0x20000b08 = 0x1000;
  *(uint32_t*)0x20000b0c = 0xf97f5ac0;
  *(uint32_t*)0x20000b10 = 8;
  *(uint32_t*)0x20000b14 = 6;
  *(uint32_t*)0x20000b18 = 0x40;
  *(uint32_t*)0x20000b1c = 0;
  *(uint32_t*)0x20000b20 = 0x8001;
  *(uint32_t*)0x20000b24 = 0x12b;
  *(uint32_t*)0x20000b28 = 8;
  *(uint32_t*)0x20000b2c = 0;
  *(uint32_t*)0x20000b30 = 9;
  *(uint32_t*)0x20000b34 = 5;
  *(uint32_t*)0x20000b38 = 4;
  *(uint32_t*)0x20000b3c = 0x28;
  *(uint32_t*)0x20000b40 = 2;
  *(uint32_t*)0x20000b44 = 6;
  *(uint32_t*)0x20000b48 = 5;
  *(uint32_t*)0x20000b4c = 0;
  *(uint32_t*)0x20000b50 = 0x7f;
  *(uint32_t*)0x20000b54 = 7;
  *(uint32_t*)0x20000b58 = 1;
  *(uint32_t*)0x20000b5c = 4;
  *(uint32_t*)0x20000b60 = 0xff;
  *(uint32_t*)0x20000b64 = 3;
  *(uint32_t*)0x20000b68 = 6;
  *(uint32_t*)0x20000b6c = 7;
  *(uint32_t*)0x20000b70 = 3;
  *(uint16_t*)0x20000b74 = 0x404;
  *(uint16_t*)0x20000b76 = 3;
  *(uint32_t*)0x20000b78 = 0x7ff;
  *(uint32_t*)0x20000b7c = 5;
  *(uint32_t*)0x20000b80 = 0x10000;
  *(uint32_t*)0x20000b84 = 2;
  *(uint32_t*)0x20000b88 = 0x10001;
  *(uint32_t*)0x20000b8c = 0x7f;
  *(uint32_t*)0x20000b90 = 0xeb;
  *(uint32_t*)0x20000b94 = 8;
  *(uint32_t*)0x20000b98 = 4;
  *(uint32_t*)0x20000b9c = 0x7ae;
  *(uint32_t*)0x20000ba0 = 3;
  *(uint32_t*)0x20000ba4 = 9;
  *(uint32_t*)0x20000ba8 = 9;
  *(uint32_t*)0x20000bac = -1;
  *(uint32_t*)0x20000bb0 = 0x7fe0000;
  *(uint32_t*)0x20000bb4 = 7;
  *(uint32_t*)0x20000bb8 = 9;
  *(uint32_t*)0x20000bbc = 5;
  *(uint32_t*)0x20000bc0 = 4;
  *(uint32_t*)0x20000bc4 = 3;
  *(uint32_t*)0x20000bc8 = 8;
  *(uint32_t*)0x20000bcc = 0xfffffffc;
  *(uint32_t*)0x20000bd0 = 6;
  *(uint32_t*)0x20000bd4 = 0x401;
  *(uint32_t*)0x20000bd8 = 0;
  *(uint32_t*)0x20000bdc = 1;
  *(uint32_t*)0x20000be0 = 2;
  *(uint32_t*)0x20000be4 = 4;
  *(uint32_t*)0x20000be8 = 4;
  *(uint32_t*)0x20000bec = 3;
  *(uint32_t*)0x20000bf0 = 3;
  *(uint32_t*)0x20000bf4 = 0xfffffffc;
  *(uint32_t*)0x20000bf8 = 9;
  *(uint32_t*)0x20000bfc = 0x8f8;
  *(uint32_t*)0x20000c00 = 0x191;
  *(uint32_t*)0x20000c04 = 0xf17c;
  *(uint32_t*)0x20000c08 = 0x80000000;
  *(uint32_t*)0x20000c0c = -1;
  *(uint32_t*)0x20000c10 = 0x1000;
  *(uint32_t*)0x20000c14 = 0x7fff;
  *(uint32_t*)0x20000c18 = 6;
  *(uint32_t*)0x20000c1c = 0x800;
  *(uint32_t*)0x20000c20 = 5;
  *(uint32_t*)0x20000c24 = 0x10000;
  *(uint32_t*)0x20000c28 = 7;
  *(uint32_t*)0x20000c2c = 3;
  *(uint32_t*)0x20000c30 = 0x3f;
  *(uint32_t*)0x20000c34 = 4;
  *(uint32_t*)0x20000c38 = 0x35;
  *(uint32_t*)0x20000c3c = 0x5c79;
  *(uint32_t*)0x20000c40 = 0x9aee;
  *(uint32_t*)0x20000c44 = 2;
  *(uint32_t*)0x20000c48 = 0;
  *(uint32_t*)0x20000c4c = 6;
  *(uint32_t*)0x20000c50 = 0xc1a1;
  *(uint32_t*)0x20000c54 = 0x4f800000;
  *(uint32_t*)0x20000c58 = 0;
  *(uint32_t*)0x20000c5c = 0xfffffffe;
  *(uint32_t*)0x20000c60 = 0x7f;
  *(uint32_t*)0x20000c64 = 0xfff;
  *(uint32_t*)0x20000c68 = 9;
  *(uint32_t*)0x20000c6c = 0xc53;
  *(uint32_t*)0x20000c70 = 8;
  *(uint32_t*)0x20000c74 = 0xe5d;
  *(uint32_t*)0x20000c78 = 2;
  *(uint32_t*)0x20000c7c = 0x80;
  *(uint32_t*)0x20000c80 = 3;
  *(uint32_t*)0x20000c84 = 0xff;
  *(uint32_t*)0x20000c88 = 0;
  *(uint32_t*)0x20000c8c = 6;
  *(uint32_t*)0x20000c90 = 0xffff;
  *(uint32_t*)0x20000c94 = 0;
  *(uint32_t*)0x20000c98 = 2;
  *(uint32_t*)0x20000c9c = 2;
  *(uint32_t*)0x20000ca0 = 4;
  *(uint32_t*)0x20000ca4 = 8;
  *(uint32_t*)0x20000ca8 = 0x81;
  *(uint32_t*)0x20000cac = 4;
  *(uint32_t*)0x20000cb0 = 6;
  *(uint32_t*)0x20000cb4 = 0;
  *(uint32_t*)0x20000cb8 = 0x6739;
  *(uint32_t*)0x20000cbc = 1;
  *(uint32_t*)0x20000cc0 = 0;
  *(uint32_t*)0x20000cc4 = 0x486;
  *(uint32_t*)0x20000cc8 = 0x1f;
  *(uint32_t*)0x20000ccc = 4;
  *(uint32_t*)0x20000cd0 = 0xe7;
  *(uint32_t*)0x20000cd4 = 0;
  *(uint32_t*)0x20000cd8 = 0x1f;
  *(uint32_t*)0x20000cdc = 0;
  *(uint32_t*)0x20000ce0 = 0x63200ec5;
  *(uint32_t*)0x20000ce4 = 5;
  *(uint32_t*)0x20000ce8 = 6;
  *(uint32_t*)0x20000cec = 0xa7bf;
  *(uint32_t*)0x20000cf0 = 7;
  *(uint32_t*)0x20000cf4 = 5;
  *(uint32_t*)0x20000cf8 = 0x40;
  *(uint32_t*)0x20000cfc = 2;
  *(uint32_t*)0x20000d00 = 1;
  *(uint32_t*)0x20000d04 = 0x200;
  *(uint32_t*)0x20000d08 = 0xff;
  *(uint32_t*)0x20000d0c = 0x40;
  *(uint32_t*)0x20000d10 = 0x100;
  *(uint32_t*)0x20000d14 = 9;
  *(uint32_t*)0x20000d18 = 4;
  *(uint32_t*)0x20000d1c = 4;
  *(uint32_t*)0x20000d20 = 0;
  *(uint32_t*)0x20000d24 = 1;
  *(uint32_t*)0x20000d28 = 4;
  *(uint32_t*)0x20000d2c = 0x81;
  *(uint32_t*)0x20000d30 = 0;
  *(uint32_t*)0x20000d34 = 0xffffe4d8;
  *(uint32_t*)0x20000d38 = -1;
  *(uint32_t*)0x20000d3c = 5;
  *(uint32_t*)0x20000d40 = 2;
  *(uint32_t*)0x20000d44 = 0xe6b;
  *(uint32_t*)0x20000d48 = 0x7f;
  *(uint32_t*)0x20000d4c = 0;
  *(uint32_t*)0x20000d50 = 7;
  *(uint32_t*)0x20000d54 = 0x33;
  *(uint32_t*)0x20000d58 = 6;
  *(uint32_t*)0x20000d5c = 7;
  *(uint32_t*)0x20000d60 = 7;
  *(uint32_t*)0x20000d64 = 6;
  *(uint32_t*)0x20000d68 = 5;
  *(uint32_t*)0x20000d6c = 0xfffffff8;
  *(uint32_t*)0x20000d70 = 0x7f;
  *(uint32_t*)0x20000d74 = 0x7fffffff;
  *(uint32_t*)0x20000d78 = 8;
  *(uint32_t*)0x20000d7c = 0x16;
  *(uint32_t*)0x20000d80 = 0x7f;
  *(uint32_t*)0x20000d84 = 0x7ff;
  *(uint32_t*)0x20000d88 = 0x7000000;
  *(uint32_t*)0x20000d8c = 5;
  *(uint32_t*)0x20000d90 = 0x7ff;
  *(uint32_t*)0x20000d94 = 0x20;
  *(uint32_t*)0x20000d98 = 8;
  *(uint32_t*)0x20000d9c = 2;
  *(uint32_t*)0x20000da0 = 0x35;
  *(uint32_t*)0x20000da4 = 0xff;
  *(uint32_t*)0x20000da8 = 8;
  *(uint32_t*)0x20000dac = 0x7ff;
  *(uint32_t*)0x20000db0 = 0x80000001;
  *(uint32_t*)0x20000db4 = 9;
  *(uint32_t*)0x20000db8 = 7;
  *(uint32_t*)0x20000dbc = 7;
  *(uint32_t*)0x20000dc0 = 9;
  *(uint32_t*)0x20000dc4 = 0;
  *(uint32_t*)0x20000dc8 = 0x200;
  *(uint32_t*)0x20000dcc = 0x10000;
  *(uint32_t*)0x20000dd0 = 0x81;
  *(uint32_t*)0x20000dd4 = 4;
  *(uint32_t*)0x20000dd8 = 5;
  *(uint32_t*)0x20000ddc = 7;
  *(uint32_t*)0x20000de0 = 0x1ff;
  *(uint32_t*)0x20000de4 = 7;
  *(uint32_t*)0x20000de8 = 0x7fff;
  *(uint32_t*)0x20000dec = 0x7fffffff;
  *(uint32_t*)0x20000df0 = 9;
  *(uint32_t*)0x20000df4 = 7;
  *(uint32_t*)0x20000df8 = 4;
  *(uint32_t*)0x20000dfc = 0;
  *(uint32_t*)0x20000e00 = 9;
  *(uint32_t*)0x20000e04 = 0x10000;
  *(uint32_t*)0x20000e08 = 4;
  *(uint32_t*)0x20000e0c = 4;
  *(uint32_t*)0x20000e10 = 4;
  *(uint32_t*)0x20000e14 = 0x7fffffff;
  *(uint32_t*)0x20000e18 = 2;
  *(uint32_t*)0x20000e1c = 4;
  *(uint32_t*)0x20000e20 = 1;
  *(uint32_t*)0x20000e24 = 0x400;
  *(uint32_t*)0x20000e28 = 0x80;
  *(uint32_t*)0x20000e2c = -1;
  *(uint32_t*)0x20000e30 = 0x1f;
  *(uint32_t*)0x20000e34 = 0x3f;
  *(uint32_t*)0x20000e38 = 1;
  *(uint32_t*)0x20000e3c = 8;
  *(uint32_t*)0x20000e40 = 0x8000;
  *(uint32_t*)0x20000e44 = 3;
  *(uint32_t*)0x20000e48 = 0x7b3;
  *(uint32_t*)0x20000e4c = 8;
  *(uint32_t*)0x20000e50 = 7;
  *(uint32_t*)0x20000e54 = 5;
  *(uint32_t*)0x20000e58 = 8;
  *(uint32_t*)0x20000e5c = 0x1f;
  *(uint32_t*)0x20000e60 = 1;
  *(uint32_t*)0x20000e64 = 0xff;
  *(uint32_t*)0x20000e68 = 4;
  *(uint32_t*)0x20000e6c = 4;
  *(uint32_t*)0x20000e70 = 0x47;
  *(uint32_t*)0x20000e74 = 7;
  *(uint32_t*)0x20000e78 = 0x1f;
  *(uint32_t*)0x20000e7c = 1;
  *(uint32_t*)0x20000e80 = 0x10000;
  *(uint32_t*)0x20000e84 = 5;
  *(uint32_t*)0x20000e88 = 0;
  *(uint32_t*)0x20000e8c = 0;
  *(uint32_t*)0x20000e90 = 6;
  *(uint32_t*)0x20000e94 = 0x1ff;
  *(uint32_t*)0x20000e98 = 1;
  *(uint32_t*)0x20000e9c = 0x80000000;
  *(uint32_t*)0x20000ea0 = 9;
  *(uint32_t*)0x20000ea4 = 9;
  *(uint32_t*)0x20000ea8 = 0;
  *(uint32_t*)0x20000eac = 0x10000;
  *(uint32_t*)0x20000eb0 = 0;
  *(uint32_t*)0x20000eb4 = 0x4aa5;
  *(uint32_t*)0x20000eb8 = 1;
  *(uint32_t*)0x20000ebc = 0x27a;
  *(uint32_t*)0x20000ec0 = 0xffff062d;
  *(uint32_t*)0x20000ec4 = 8;
  *(uint32_t*)0x20000ec8 = 5;
  *(uint32_t*)0x20000ecc = 0xffffffe1;
  *(uint32_t*)0x20000ed0 = 0x7f;
  *(uint32_t*)0x20000ed4 = 0x8000;
  *(uint32_t*)0x20000ed8 = 9;
  *(uint32_t*)0x20000edc = 7;
  *(uint32_t*)0x20000ee0 = 0xfff;
  *(uint32_t*)0x20000ee4 = 0xfffffffe;
  *(uint32_t*)0x20000ee8 = 4;
  *(uint32_t*)0x20000eec = 0;
  *(uint32_t*)0x20000ef0 = 7;
  *(uint32_t*)0x20000ef4 = 5;
  *(uint32_t*)0x20000ef8 = 0x400;
  *(uint32_t*)0x20000efc = 0;
  *(uint32_t*)0x20000f00 = 5;
  *(uint32_t*)0x20000f04 = 3;
  *(uint32_t*)0x20000f08 = 0x115b;
  *(uint32_t*)0x20000f0c = 0xa4a;
  *(uint32_t*)0x20000f10 = 0x8001;
  *(uint32_t*)0x20000f14 = 0xbf42;
  *(uint32_t*)0x20000f18 = 0xb5a;
  *(uint32_t*)0x20000f1c = 8;
  *(uint32_t*)0x20000f20 = 0x10000;
  *(uint32_t*)0x20000f24 = 0xb4;
  *(uint32_t*)0x20000f28 = 9;
  *(uint32_t*)0x20000f2c = 0xd;
  *(uint32_t*)0x20000f30 = 7;
  *(uint32_t*)0x20000f34 = 0xc1908d00;
  *(uint32_t*)0x20000f38 = 1;
  *(uint32_t*)0x20000f3c = 8;
  *(uint32_t*)0x20000f40 = 4;
  *(uint32_t*)0x20000f44 = 6;
  *(uint32_t*)0x20000f48 = 4;
  *(uint32_t*)0x20000f4c = 0x101;
  *(uint32_t*)0x20000f50 = 0x22;
  *(uint32_t*)0x20000f54 = 0xfffffffc;
  *(uint32_t*)0x20000f58 = 6;
  *(uint32_t*)0x20000f5c = 0x95e5;
  *(uint32_t*)0x20000f60 = 1;
  *(uint32_t*)0x20000f64 = 0x8000;
  *(uint32_t*)0x20000f68 = 1;
  *(uint32_t*)0x20000f6c = 0xa8bc;
  *(uint32_t*)0x20000f70 = 1;
  *(uint32_t*)0x20000f74 = 0x20;
  *(uint16_t*)0x20000f78 = 0xc;
  *(uint16_t*)0x20000f7a = 9;
  *(uint64_t*)0x20000f7c = 1;
  *(uint16_t*)0x20000f84 = 0x3c;
  *(uint16_t*)0x20000f86 = 1;
  *(uint32_t*)0x20000f88 = 0x10001;
  *(uint32_t*)0x20000f8c = 0x10000000;
  *(uint32_t*)0x20000f90 = 0x9e3;
  *(uint32_t*)0x20000f94 = 0x1ff;
  *(uint32_t*)0x20000f98 = 5;
  *(uint8_t*)0x20000f9c = 8;
  *(uint8_t*)0x20000f9d = 2;
  *(uint16_t*)0x20000f9e = 3;
  *(uint16_t*)0x20000fa0 = 0x6f8;
  *(uint16_t*)0x20000fa2 = 0x969d;
  *(uint32_t*)0x20000fa4 = 9;
  *(uint8_t*)0x20000fa8 = 0x1f;
  *(uint8_t*)0x20000fa9 = 0;
  *(uint16_t*)0x20000faa = 3;
  *(uint16_t*)0x20000fac = 0xfff;
  *(uint16_t*)0x20000fae = 7;
  *(uint32_t*)0x20000fb0 = 8;
  *(uint32_t*)0x20000fb4 = 7;
  *(uint32_t*)0x20000fb8 = 2;
  *(uint32_t*)0x20000fbc = 0x20;
  *(uint16_t*)0x20000fc0 = 0x14;
  *(uint16_t*)0x20000fc2 = 5;
  *(uint8_t*)0x20000fc4 = 0;
  *(uint8_t*)0x20000fc5 = 0;
  *(uint8_t*)0x20000fc6 = 0;
  *(uint16_t*)0x20000fc8 = htobe16(0);
  *(uint16_t*)0x20000fca = 0;
  *(uint16_t*)0x20000fcc = 0;
  *(uint16_t*)0x20000fce = 0;
  *(uint32_t*)0x20000fd0 = htobe32(0);
  *(uint64_t*)0x20000188 = 0x8d4;
  *(uint64_t*)0x20000458 = 1;
  *(uint64_t*)0x20000460 = 0;
  *(uint64_t*)0x20000468 = 0;
  *(uint32_t*)0x20000470 = 0;
  syscall(__NR_sendmsg, -1, 0x20000440ul, 0ul);
  syscall(__NR_sendmmsg, r[0], 0x20000200ul, 0x10efe10675dec16ul, 0ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  do_sandbox_none();
  return 0;
}
