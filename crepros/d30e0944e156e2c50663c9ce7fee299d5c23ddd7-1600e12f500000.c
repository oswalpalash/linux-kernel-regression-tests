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

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

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
  res = syscall(__NR_getsockname, r[1], 0x200002c0ul, 0x20000200ul);
  if (res != -1)
    r[2] = *(uint32_t*)0x200002c4;
  *(uint64_t*)0x20000240 = 0;
  *(uint32_t*)0x20000248 = 0;
  *(uint64_t*)0x20000250 = 0x20000140;
  *(uint64_t*)0x20000140 = 0x200003c0;
  memcpy((void*)0x200003c0, "\x38\x00\x00\x00\x24\x00\xff\xff\xff\x7f\x00\x00"
                            "\x00\x00\x3c\x00\x05\x00\x00\x00",
         20);
  *(uint32_t*)0x200003d4 = r[2];
  memcpy((void*)0x200003d8, "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00"
                            "\x09\x00\x01\x00\x68\x66\x73\x63\x00\x00\x00\x00"
                            "\x08\x00\x02",
         27);
  *(uint64_t*)0x20000148 = 0x38;
  *(uint64_t*)0x20000258 = 1;
  *(uint64_t*)0x20000260 = 0;
  *(uint64_t*)0x20000268 = 0;
  *(uint32_t*)0x20000270 = 0;
  syscall(__NR_sendmsg, -1, 0x20000240ul, 0ul);
  *(uint64_t*)0x20000300 = 0;
  *(uint32_t*)0x20000308 = 0;
  *(uint64_t*)0x20000310 = 0x20000180;
  *(uint64_t*)0x20000180 = 0x20000580;
  *(uint32_t*)0x20000580 = 0xb50;
  *(uint16_t*)0x20000584 = 0x2c;
  *(uint16_t*)0x20000586 = 0xd27;
  *(uint32_t*)0x20000588 = 0;
  *(uint32_t*)0x2000058c = 0;
  *(uint8_t*)0x20000590 = 0;
  *(uint8_t*)0x20000591 = 0;
  *(uint16_t*)0x20000592 = 0;
  *(uint32_t*)0x20000594 = r[2];
  *(uint16_t*)0x20000598 = 0;
  *(uint16_t*)0x2000059a = 0;
  *(uint16_t*)0x2000059c = 0;
  *(uint16_t*)0x2000059e = 0;
  *(uint16_t*)0x200005a0 = 0x10;
  *(uint16_t*)0x200005a2 = 0;
  *(uint16_t*)0x200005a4 = 0xc;
  *(uint16_t*)0x200005a6 = 1;
  memcpy((void*)0x200005a8, "tcindex\000", 8);
  *(uint16_t*)0x200005b0 = 0xb20;
  *(uint16_t*)0x200005b2 = 2;
  *(uint16_t*)0x200005b4 = 0x8a8;
  *(uint16_t*)0x200005b6 = 6;
  *(uint16_t*)0x200005b8 = 0x404;
  *(uint16_t*)0x200005ba = 3;
  *(uint32_t*)0x200005bc = 0xee;
  *(uint32_t*)0x200005c0 = 8;
  *(uint32_t*)0x200005c4 = 0x28;
  *(uint32_t*)0x200005c8 = 0x525;
  *(uint32_t*)0x200005cc = 0x10001;
  *(uint32_t*)0x200005d0 = 0x8000;
  *(uint32_t*)0x200005d4 = 0xd98;
  *(uint32_t*)0x200005d8 = 7;
  *(uint32_t*)0x200005dc = 0x3a;
  *(uint32_t*)0x200005e0 = 8;
  *(uint32_t*)0x200005e4 = 4;
  *(uint32_t*)0x200005e8 = 6;
  *(uint32_t*)0x200005ec = 0x531e4700;
  *(uint32_t*)0x200005f0 = 0x1000;
  *(uint32_t*)0x200005f4 = 9;
  *(uint32_t*)0x200005f8 = 1;
  *(uint32_t*)0x200005fc = 4;
  *(uint32_t*)0x20000600 = 3;
  *(uint32_t*)0x20000604 = 0x1000;
  *(uint32_t*)0x20000608 = 2;
  *(uint32_t*)0x2000060c = 0x8001;
  *(uint32_t*)0x20000610 = 0x101;
  *(uint32_t*)0x20000614 = 4;
  *(uint32_t*)0x20000618 = 0xc28;
  *(uint32_t*)0x2000061c = 8;
  *(uint32_t*)0x20000620 = 0;
  *(uint32_t*)0x20000624 = 2;
  *(uint32_t*)0x20000628 = 0x101;
  *(uint32_t*)0x2000062c = 0x80000001;
  *(uint32_t*)0x20000630 = 0x81;
  *(uint32_t*)0x20000634 = 0x6e39;
  *(uint32_t*)0x20000638 = 3;
  *(uint32_t*)0x2000063c = 0x80000000;
  *(uint32_t*)0x20000640 = 0xdc7;
  *(uint32_t*)0x20000644 = 7;
  *(uint32_t*)0x20000648 = 0xbd;
  *(uint32_t*)0x2000064c = 2;
  *(uint32_t*)0x20000650 = 0x81;
  *(uint32_t*)0x20000654 = 0x487f3857;
  *(uint32_t*)0x20000658 = 5;
  *(uint32_t*)0x2000065c = 0x8000;
  *(uint32_t*)0x20000660 = 3;
  *(uint32_t*)0x20000664 = 6;
  *(uint32_t*)0x20000668 = 0x80;
  *(uint32_t*)0x2000066c = 0x78a6;
  *(uint32_t*)0x20000670 = 0x3f;
  *(uint32_t*)0x20000674 = 0;
  *(uint32_t*)0x20000678 = 9;
  *(uint32_t*)0x2000067c = 0x15e;
  *(uint32_t*)0x20000680 = 5;
  *(uint32_t*)0x20000684 = 0x40;
  *(uint32_t*)0x20000688 = 3;
  *(uint32_t*)0x2000068c = 0xffff;
  *(uint32_t*)0x20000690 = 9;
  *(uint32_t*)0x20000694 = 7;
  *(uint32_t*)0x20000698 = 9;
  *(uint32_t*)0x2000069c = 0x8001;
  *(uint32_t*)0x200006a0 = 6;
  *(uint32_t*)0x200006a4 = 1;
  *(uint32_t*)0x200006a8 = 6;
  *(uint32_t*)0x200006ac = 8;
  *(uint32_t*)0x200006b0 = 1;
  *(uint32_t*)0x200006b4 = 0xdc5;
  *(uint32_t*)0x200006b8 = 0x1ff;
  *(uint32_t*)0x200006bc = 0x200;
  *(uint32_t*)0x200006c0 = 0x7fffffff;
  *(uint32_t*)0x200006c4 = 0x401;
  *(uint32_t*)0x200006c8 = 0x8001;
  *(uint32_t*)0x200006cc = 0;
  *(uint32_t*)0x200006d0 = 0x247;
  *(uint32_t*)0x200006d4 = 0;
  *(uint32_t*)0x200006d8 = 8;
  *(uint32_t*)0x200006dc = 3;
  *(uint32_t*)0x200006e0 = 3;
  *(uint32_t*)0x200006e4 = 0x80;
  *(uint32_t*)0x200006e8 = 0xfffffffd;
  *(uint32_t*)0x200006ec = 0xd0aa;
  *(uint32_t*)0x200006f0 = 0xfffffff2;
  *(uint32_t*)0x200006f4 = 0x81;
  *(uint32_t*)0x200006f8 = 2;
  *(uint32_t*)0x200006fc = 5;
  *(uint32_t*)0x20000700 = 6;
  *(uint32_t*)0x20000704 = 0x1ff;
  *(uint32_t*)0x20000708 = 0xfffffc01;
  *(uint32_t*)0x2000070c = 8;
  *(uint32_t*)0x20000710 = 0x7f;
  *(uint32_t*)0x20000714 = 0;
  *(uint32_t*)0x20000718 = 0x76;
  *(uint32_t*)0x2000071c = 0x80000000;
  *(uint32_t*)0x20000720 = 0x8001;
  *(uint32_t*)0x20000724 = 0xfff;
  *(uint32_t*)0x20000728 = 0x101;
  *(uint32_t*)0x2000072c = 0;
  *(uint32_t*)0x20000730 = 0x81;
  *(uint32_t*)0x20000734 = 4;
  *(uint32_t*)0x20000738 = 7;
  *(uint32_t*)0x2000073c = 0x80000001;
  *(uint32_t*)0x20000740 = 1;
  *(uint32_t*)0x20000744 = 0xef29;
  *(uint32_t*)0x20000748 = 1;
  *(uint32_t*)0x2000074c = 1;
  *(uint32_t*)0x20000750 = 3;
  *(uint32_t*)0x20000754 = 8;
  *(uint32_t*)0x20000758 = 0x8e7;
  *(uint32_t*)0x2000075c = 0xffffff28;
  *(uint32_t*)0x20000760 = 0x20;
  *(uint32_t*)0x20000764 = 0;
  *(uint32_t*)0x20000768 = 1;
  *(uint32_t*)0x2000076c = 7;
  *(uint32_t*)0x20000770 = 8;
  *(uint32_t*)0x20000774 = 0x7f;
  *(uint32_t*)0x20000778 = 5;
  *(uint32_t*)0x2000077c = 5;
  *(uint32_t*)0x20000780 = 0x20;
  *(uint32_t*)0x20000784 = 2;
  *(uint32_t*)0x20000788 = 9;
  *(uint32_t*)0x2000078c = 5;
  *(uint32_t*)0x20000790 = 0x800;
  *(uint32_t*)0x20000794 = 4;
  *(uint32_t*)0x20000798 = 3;
  *(uint32_t*)0x2000079c = 6;
  *(uint32_t*)0x200007a0 = 0x1f;
  *(uint32_t*)0x200007a4 = 0x3800000;
  *(uint32_t*)0x200007a8 = 3;
  *(uint32_t*)0x200007ac = 2;
  *(uint32_t*)0x200007b0 = 1;
  *(uint32_t*)0x200007b4 = 0xfffffc00;
  *(uint32_t*)0x200007b8 = 0xbf31;
  *(uint32_t*)0x200007bc = 1;
  *(uint32_t*)0x200007c0 = 9;
  *(uint32_t*)0x200007c4 = 5;
  *(uint32_t*)0x200007c8 = 9;
  *(uint32_t*)0x200007cc = 9;
  *(uint32_t*)0x200007d0 = 2;
  *(uint32_t*)0x200007d4 = 3;
  *(uint32_t*)0x200007d8 = 3;
  *(uint32_t*)0x200007dc = 0;
  *(uint32_t*)0x200007e0 = 2;
  *(uint32_t*)0x200007e4 = 0xce4f;
  *(uint32_t*)0x200007e8 = 5;
  *(uint32_t*)0x200007ec = 4;
  *(uint32_t*)0x200007f0 = 0x20;
  *(uint32_t*)0x200007f4 = 0xfffffffc;
  *(uint32_t*)0x200007f8 = 0x40cdcc8e;
  *(uint32_t*)0x200007fc = 6;
  *(uint32_t*)0x20000800 = 0;
  *(uint32_t*)0x20000804 = 7;
  *(uint32_t*)0x20000808 = 0;
  *(uint32_t*)0x2000080c = 0x7fffffff;
  *(uint32_t*)0x20000810 = 5;
  *(uint32_t*)0x20000814 = 4;
  *(uint32_t*)0x20000818 = 0;
  *(uint32_t*)0x2000081c = 0x10000;
  *(uint32_t*)0x20000820 = 0;
  *(uint32_t*)0x20000824 = 7;
  *(uint32_t*)0x20000828 = 0xa30;
  *(uint32_t*)0x2000082c = 6;
  *(uint32_t*)0x20000830 = 3;
  *(uint32_t*)0x20000834 = 7;
  *(uint32_t*)0x20000838 = 5;
  *(uint32_t*)0x2000083c = 0x7fcdaddd;
  *(uint32_t*)0x20000840 = 7;
  *(uint32_t*)0x20000844 = 0x426d;
  *(uint32_t*)0x20000848 = 0xbd;
  *(uint32_t*)0x2000084c = 0x3ff;
  *(uint32_t*)0x20000850 = 0x20;
  *(uint32_t*)0x20000854 = 2;
  *(uint32_t*)0x20000858 = 0x32a9;
  *(uint32_t*)0x2000085c = 0x800;
  *(uint32_t*)0x20000860 = 3;
  *(uint32_t*)0x20000864 = 0x7fff;
  *(uint32_t*)0x20000868 = 0xffff0001;
  *(uint32_t*)0x2000086c = 3;
  *(uint32_t*)0x20000870 = 0x523;
  *(uint32_t*)0x20000874 = 0xc5;
  *(uint32_t*)0x20000878 = 0x734451bb;
  *(uint32_t*)0x2000087c = 0xfffff000;
  *(uint32_t*)0x20000880 = 0;
  *(uint32_t*)0x20000884 = 0x4f3;
  *(uint32_t*)0x20000888 = 0xff;
  *(uint32_t*)0x2000088c = 0xc0000000;
  *(uint32_t*)0x20000890 = 0x90;
  *(uint32_t*)0x20000894 = 9;
  *(uint32_t*)0x20000898 = 0x1f;
  *(uint32_t*)0x2000089c = 0x7fff;
  *(uint32_t*)0x200008a0 = 0x1000;
  *(uint32_t*)0x200008a4 = 0xa5;
  *(uint32_t*)0x200008a8 = 0x80000001;
  *(uint32_t*)0x200008ac = 0x40;
  *(uint32_t*)0x200008b0 = 0x80000001;
  *(uint32_t*)0x200008b4 = 2;
  *(uint32_t*)0x200008b8 = 9;
  *(uint32_t*)0x200008bc = 0x200;
  *(uint32_t*)0x200008c0 = 9;
  *(uint32_t*)0x200008c4 = 8;
  *(uint32_t*)0x200008c8 = 0;
  *(uint32_t*)0x200008cc = 0xfffffffb;
  *(uint32_t*)0x200008d0 = 0x80;
  *(uint32_t*)0x200008d4 = 0xf0b;
  *(uint32_t*)0x200008d8 = 1;
  *(uint32_t*)0x200008dc = 0x2c;
  *(uint32_t*)0x200008e0 = 0x10000;
  *(uint32_t*)0x200008e4 = 4;
  *(uint32_t*)0x200008e8 = 0x200;
  *(uint32_t*)0x200008ec = 0x200;
  *(uint32_t*)0x200008f0 = 1;
  *(uint32_t*)0x200008f4 = 0x54;
  *(uint32_t*)0x200008f8 = 2;
  *(uint32_t*)0x200008fc = 8;
  *(uint32_t*)0x20000900 = 7;
  *(uint32_t*)0x20000904 = 9;
  *(uint32_t*)0x20000908 = 6;
  *(uint32_t*)0x2000090c = 6;
  *(uint32_t*)0x20000910 = 0xff;
  *(uint32_t*)0x20000914 = 0xf64;
  *(uint32_t*)0x20000918 = 3;
  *(uint32_t*)0x2000091c = 3;
  *(uint32_t*)0x20000920 = 0x8b;
  *(uint32_t*)0x20000924 = 0x100;
  *(uint32_t*)0x20000928 = 8;
  *(uint32_t*)0x2000092c = 0x12;
  *(uint32_t*)0x20000930 = 0x99;
  *(uint32_t*)0x20000934 = 0xa4e;
  *(uint32_t*)0x20000938 = 0x77cc;
  *(uint32_t*)0x2000093c = 1;
  *(uint32_t*)0x20000940 = 0xdc81;
  *(uint32_t*)0x20000944 = 1;
  *(uint32_t*)0x20000948 = 2;
  *(uint32_t*)0x2000094c = 0x400;
  *(uint32_t*)0x20000950 = 7;
  *(uint32_t*)0x20000954 = 9;
  *(uint32_t*)0x20000958 = 0x20;
  *(uint32_t*)0x2000095c = 3;
  *(uint32_t*)0x20000960 = 0x334;
  *(uint32_t*)0x20000964 = 7;
  *(uint32_t*)0x20000968 = 5;
  *(uint32_t*)0x2000096c = 0x6e;
  *(uint32_t*)0x20000970 = 7;
  *(uint32_t*)0x20000974 = 8;
  *(uint32_t*)0x20000978 = 0x7fff;
  *(uint32_t*)0x2000097c = 1;
  *(uint32_t*)0x20000980 = 0x10000;
  *(uint32_t*)0x20000984 = 0x972f;
  *(uint32_t*)0x20000988 = 0;
  *(uint32_t*)0x2000098c = 2;
  *(uint32_t*)0x20000990 = 0x200;
  *(uint32_t*)0x20000994 = 0;
  *(uint32_t*)0x20000998 = 0x100010;
  *(uint32_t*)0x2000099c = 0xfffffffd;
  *(uint32_t*)0x200009a0 = 9;
  *(uint32_t*)0x200009a4 = 0x4a800000;
  *(uint32_t*)0x200009a8 = 0x1000;
  *(uint32_t*)0x200009ac = 9;
  *(uint32_t*)0x200009b0 = 3;
  *(uint32_t*)0x200009b4 = 9;
  *(uint32_t*)0x200009b8 = 0xf0a;
  *(uint16_t*)0x200009bc = 0x404;
  *(uint16_t*)0x200009be = 2;
  *(uint32_t*)0x200009c0 = 1;
  *(uint32_t*)0x200009c4 = 0x40;
  *(uint32_t*)0x200009c8 = 4;
  *(uint32_t*)0x200009cc = 0x5d70;
  *(uint32_t*)0x200009d0 = 0x7f;
  *(uint32_t*)0x200009d4 = 9;
  *(uint32_t*)0x200009d8 = 9;
  *(uint32_t*)0x200009dc = 3;
  *(uint32_t*)0x200009e0 = 0xfffffff9;
  *(uint32_t*)0x200009e4 = 1;
  *(uint32_t*)0x200009e8 = 0x80;
  *(uint32_t*)0x200009ec = 5;
  *(uint32_t*)0x200009f0 = 6;
  *(uint32_t*)0x200009f4 = 4;
  *(uint32_t*)0x200009f8 = 5;
  *(uint32_t*)0x200009fc = 0x81;
  *(uint32_t*)0x20000a00 = 0x34;
  *(uint32_t*)0x20000a04 = 0x400;
  *(uint32_t*)0x20000a08 = 0x101;
  *(uint32_t*)0x20000a0c = 0xfff;
  *(uint32_t*)0x20000a10 = 0xffffa65f;
  *(uint32_t*)0x20000a14 = 0x5f5dee2c;
  *(uint32_t*)0x20000a18 = 9;
  *(uint32_t*)0x20000a1c = 4;
  *(uint32_t*)0x20000a20 = 0x100;
  *(uint32_t*)0x20000a24 = 0x401;
  *(uint32_t*)0x20000a28 = 0x100;
  *(uint32_t*)0x20000a2c = 1;
  *(uint32_t*)0x20000a30 = 2;
  *(uint32_t*)0x20000a34 = 0xff;
  *(uint32_t*)0x20000a38 = 0x81;
  *(uint32_t*)0x20000a3c = 0x9a09;
  *(uint32_t*)0x20000a40 = 0x9b;
  *(uint32_t*)0x20000a44 = 7;
  *(uint32_t*)0x20000a48 = 5;
  *(uint32_t*)0x20000a4c = 3;
  *(uint32_t*)0x20000a50 = 9;
  *(uint32_t*)0x20000a54 = 0x9c46;
  *(uint32_t*)0x20000a58 = 0x80;
  *(uint32_t*)0x20000a5c = 0x80;
  *(uint32_t*)0x20000a60 = 0x80;
  *(uint32_t*)0x20000a64 = 0x80000001;
  *(uint32_t*)0x20000a68 = 0xfffffffc;
  *(uint32_t*)0x20000a6c = 0xfffff001;
  *(uint32_t*)0x20000a70 = 0;
  *(uint32_t*)0x20000a74 = 0xf6b;
  *(uint32_t*)0x20000a78 = 0x3f;
  *(uint32_t*)0x20000a7c = -1;
  *(uint32_t*)0x20000a80 = 7;
  *(uint32_t*)0x20000a84 = 6;
  *(uint32_t*)0x20000a88 = 4;
  *(uint32_t*)0x20000a8c = 0x7fffffff;
  *(uint32_t*)0x20000a90 = 5;
  *(uint32_t*)0x20000a94 = 0x8000;
  *(uint32_t*)0x20000a98 = 0x32;
  *(uint32_t*)0x20000a9c = 0x67c2;
  *(uint32_t*)0x20000aa0 = 9;
  *(uint32_t*)0x20000aa4 = 0xcf80;
  *(uint32_t*)0x20000aa8 = 0;
  *(uint32_t*)0x20000aac = 5;
  *(uint32_t*)0x20000ab0 = 5;
  *(uint32_t*)0x20000ab4 = 9;
  *(uint32_t*)0x20000ab8 = 1;
  *(uint32_t*)0x20000abc = 7;
  *(uint32_t*)0x20000ac0 = 0x10001;
  *(uint32_t*)0x20000ac4 = 0x200;
  *(uint32_t*)0x20000ac8 = 1;
  *(uint32_t*)0x20000acc = 0xff;
  *(uint32_t*)0x20000ad0 = 5;
  *(uint32_t*)0x20000ad4 = 3;
  *(uint32_t*)0x20000ad8 = 3;
  *(uint32_t*)0x20000adc = 0x741a;
  *(uint32_t*)0x20000ae0 = 8;
  *(uint32_t*)0x20000ae4 = 6;
  *(uint32_t*)0x20000ae8 = 2;
  *(uint32_t*)0x20000aec = 0x945;
  *(uint32_t*)0x20000af0 = 0x6d0;
  *(uint32_t*)0x20000af4 = 7;
  *(uint32_t*)0x20000af8 = 7;
  *(uint32_t*)0x20000afc = 0x80000001;
  *(uint32_t*)0x20000b00 = 8;
  *(uint32_t*)0x20000b04 = 2;
  *(uint32_t*)0x20000b08 = 5;
  *(uint32_t*)0x20000b0c = 6;
  *(uint32_t*)0x20000b10 = 0x40;
  *(uint32_t*)0x20000b14 = 0x80;
  *(uint32_t*)0x20000b18 = 0x7fffffff;
  *(uint32_t*)0x20000b1c = 0x81;
  *(uint32_t*)0x20000b20 = 4;
  *(uint32_t*)0x20000b24 = 0x7fff;
  *(uint32_t*)0x20000b28 = 0x200;
  *(uint32_t*)0x20000b2c = 4;
  *(uint32_t*)0x20000b30 = 0x7ec;
  *(uint32_t*)0x20000b34 = 0xfffffff9;
  *(uint32_t*)0x20000b38 = 0;
  *(uint32_t*)0x20000b3c = 0xca5;
  *(uint32_t*)0x20000b40 = 8;
  *(uint32_t*)0x20000b44 = 0xfffffff7;
  *(uint32_t*)0x20000b48 = 5;
  *(uint32_t*)0x20000b4c = 0x24;
  *(uint32_t*)0x20000b50 = 1;
  *(uint32_t*)0x20000b54 = 5;
  *(uint32_t*)0x20000b58 = 4;
  *(uint32_t*)0x20000b5c = 0xa8;
  *(uint32_t*)0x20000b60 = 0x23bd;
  *(uint32_t*)0x20000b64 = 0;
  *(uint32_t*)0x20000b68 = 0x401;
  *(uint32_t*)0x20000b6c = 0x80000000;
  *(uint32_t*)0x20000b70 = 9;
  *(uint32_t*)0x20000b74 = 0xffff;
  *(uint32_t*)0x20000b78 = 0x40;
  *(uint32_t*)0x20000b7c = 2;
  *(uint32_t*)0x20000b80 = 8;
  *(uint32_t*)0x20000b84 = 0x200;
  *(uint32_t*)0x20000b88 = 0x3b67;
  *(uint32_t*)0x20000b8c = 2;
  *(uint32_t*)0x20000b90 = 0x80000001;
  *(uint32_t*)0x20000b94 = 7;
  *(uint32_t*)0x20000b98 = 0x1a13;
  *(uint32_t*)0x20000b9c = 0xffff1cb3;
  *(uint32_t*)0x20000ba0 = 8;
  *(uint32_t*)0x20000ba4 = 0xffff;
  *(uint32_t*)0x20000ba8 = 9;
  *(uint32_t*)0x20000bac = 0xfff;
  *(uint32_t*)0x20000bb0 = 0xf1b;
  *(uint32_t*)0x20000bb4 = 3;
  *(uint32_t*)0x20000bb8 = 8;
  *(uint32_t*)0x20000bbc = 9;
  *(uint32_t*)0x20000bc0 = 0x401;
  *(uint32_t*)0x20000bc4 = 0x5f0216ce;
  *(uint32_t*)0x20000bc8 = 5;
  *(uint32_t*)0x20000bcc = 0xcb;
  *(uint32_t*)0x20000bd0 = 8;
  *(uint32_t*)0x20000bd4 = 0x2bb8c572;
  *(uint32_t*)0x20000bd8 = 4;
  *(uint32_t*)0x20000bdc = 0x80;
  *(uint32_t*)0x20000be0 = 1;
  *(uint32_t*)0x20000be4 = 5;
  *(uint32_t*)0x20000be8 = 9;
  *(uint32_t*)0x20000bec = 0x3cb9;
  *(uint32_t*)0x20000bf0 = 3;
  *(uint32_t*)0x20000bf4 = 0x93d;
  *(uint32_t*)0x20000bf8 = 0x57f4ab31;
  *(uint32_t*)0x20000bfc = 0;
  *(uint32_t*)0x20000c00 = 0;
  *(uint32_t*)0x20000c04 = 0x3a0c;
  *(uint32_t*)0x20000c08 = 0xdf0d;
  *(uint32_t*)0x20000c0c = 0x20000;
  *(uint32_t*)0x20000c10 = 2;
  *(uint32_t*)0x20000c14 = 1;
  *(uint32_t*)0x20000c18 = 0x8001;
  *(uint32_t*)0x20000c1c = 0x8001;
  *(uint32_t*)0x20000c20 = 2;
  *(uint32_t*)0x20000c24 = 0;
  *(uint32_t*)0x20000c28 = 4;
  *(uint32_t*)0x20000c2c = 0x400;
  *(uint32_t*)0x20000c30 = 0xfffffffb;
  *(uint32_t*)0x20000c34 = 0xff;
  *(uint32_t*)0x20000c38 = 0x3ff;
  *(uint32_t*)0x20000c3c = 0xc9;
  *(uint32_t*)0x20000c40 = 0;
  *(uint32_t*)0x20000c44 = 7;
  *(uint32_t*)0x20000c48 = 0;
  *(uint32_t*)0x20000c4c = 0x80000001;
  *(uint32_t*)0x20000c50 = 7;
  *(uint32_t*)0x20000c54 = 2;
  *(uint32_t*)0x20000c58 = 5;
  *(uint32_t*)0x20000c5c = 3;
  *(uint32_t*)0x20000c60 = 0xfffffff9;
  *(uint32_t*)0x20000c64 = 0x40;
  *(uint32_t*)0x20000c68 = 0x7f;
  *(uint32_t*)0x20000c6c = 0;
  *(uint32_t*)0x20000c70 = 0;
  *(uint32_t*)0x20000c74 = 0x101;
  *(uint32_t*)0x20000c78 = 9;
  *(uint32_t*)0x20000c7c = 8;
  *(uint32_t*)0x20000c80 = 1;
  *(uint32_t*)0x20000c84 = 0xffff;
  *(uint32_t*)0x20000c88 = 0x3f;
  *(uint32_t*)0x20000c8c = 2;
  *(uint32_t*)0x20000c90 = -1;
  *(uint32_t*)0x20000c94 = 0x33701a89;
  *(uint32_t*)0x20000c98 = 0x60;
  *(uint32_t*)0x20000c9c = 3;
  *(uint32_t*)0x20000ca0 = 0x1f;
  *(uint32_t*)0x20000ca4 = 0x10000000;
  *(uint32_t*)0x20000ca8 = 0x1f;
  *(uint32_t*)0x20000cac = 9;
  *(uint32_t*)0x20000cb0 = 0x6c;
  *(uint32_t*)0x20000cb4 = 5;
  *(uint32_t*)0x20000cb8 = 0xfff;
  *(uint32_t*)0x20000cbc = 6;
  *(uint32_t*)0x20000cc0 = 1;
  *(uint32_t*)0x20000cc4 = 0x100;
  *(uint32_t*)0x20000cc8 = 2;
  *(uint32_t*)0x20000ccc = 8;
  *(uint32_t*)0x20000cd0 = 0;
  *(uint32_t*)0x20000cd4 = 7;
  *(uint32_t*)0x20000cd8 = 0x401;
  *(uint32_t*)0x20000cdc = 9;
  *(uint32_t*)0x20000ce0 = 9;
  *(uint32_t*)0x20000ce4 = 0xd9b9;
  *(uint32_t*)0x20000ce8 = 0xade2;
  *(uint32_t*)0x20000cec = 0x763;
  *(uint32_t*)0x20000cf0 = 1;
  *(uint32_t*)0x20000cf4 = 0x3ff;
  *(uint32_t*)0x20000cf8 = 3;
  *(uint32_t*)0x20000cfc = 0x79de;
  *(uint32_t*)0x20000d00 = 0x7fffffff;
  *(uint32_t*)0x20000d04 = 0x25f;
  *(uint32_t*)0x20000d08 = 3;
  *(uint32_t*)0x20000d0c = 0x1553;
  *(uint32_t*)0x20000d10 = 4;
  *(uint32_t*)0x20000d14 = 0;
  *(uint32_t*)0x20000d18 = 0x401;
  *(uint32_t*)0x20000d1c = 1;
  *(uint32_t*)0x20000d20 = 0x36;
  *(uint32_t*)0x20000d24 = 0xfffff001;
  *(uint32_t*)0x20000d28 = 2;
  *(uint32_t*)0x20000d2c = 0x8001;
  *(uint32_t*)0x20000d30 = 3;
  *(uint32_t*)0x20000d34 = 9;
  *(uint32_t*)0x20000d38 = 8;
  *(uint32_t*)0x20000d3c = 9;
  *(uint32_t*)0x20000d40 = 0x7ff;
  *(uint32_t*)0x20000d44 = 3;
  *(uint32_t*)0x20000d48 = 9;
  *(uint32_t*)0x20000d4c = 0x3ec4;
  *(uint32_t*)0x20000d50 = 0xffff;
  *(uint32_t*)0x20000d54 = 0xec;
  *(uint32_t*)0x20000d58 = 0x81;
  *(uint32_t*)0x20000d5c = 2;
  *(uint32_t*)0x20000d60 = 0x10001;
  *(uint32_t*)0x20000d64 = 6;
  *(uint32_t*)0x20000d68 = 0xff;
  *(uint32_t*)0x20000d6c = 0x8e;
  *(uint32_t*)0x20000d70 = 0xbf5;
  *(uint32_t*)0x20000d74 = 0x81;
  *(uint32_t*)0x20000d78 = 0;
  *(uint32_t*)0x20000d7c = 0x8001;
  *(uint32_t*)0x20000d80 = 5;
  *(uint32_t*)0x20000d84 = 0x81;
  *(uint32_t*)0x20000d88 = 0x80000000;
  *(uint32_t*)0x20000d8c = 0x7ff;
  *(uint32_t*)0x20000d90 = 5;
  *(uint32_t*)0x20000d94 = 8;
  *(uint32_t*)0x20000d98 = 0x1f;
  *(uint32_t*)0x20000d9c = 0x100;
  *(uint32_t*)0x20000da0 = 5;
  *(uint32_t*)0x20000da4 = 7;
  *(uint32_t*)0x20000da8 = 7;
  *(uint32_t*)0x20000dac = 8;
  *(uint32_t*)0x20000db0 = 0x7fff;
  *(uint32_t*)0x20000db4 = 0;
  *(uint32_t*)0x20000db8 = 0xff;
  *(uint32_t*)0x20000dbc = 0x401;
  *(uint16_t*)0x20000dc0 = 0x3c;
  *(uint16_t*)0x20000dc2 = 1;
  *(uint32_t*)0x20000dc4 = 8;
  *(uint32_t*)0x20000dc8 = 8;
  *(uint32_t*)0x20000dcc = 2;
  *(uint32_t*)0x20000dd0 = 0x60f;
  *(uint32_t*)0x20000dd4 = 0x8001;
  *(uint8_t*)0x20000dd8 = 6;
  *(uint8_t*)0x20000dd9 = 0;
  *(uint16_t*)0x20000dda = 0;
  *(uint16_t*)0x20000ddc = 9;
  *(uint16_t*)0x20000dde = 4;
  *(uint32_t*)0x20000de0 = 0x7ff;
  *(uint8_t*)0x20000de4 = 0;
  *(uint8_t*)0x20000de5 = 1;
  *(uint16_t*)0x20000de6 = 8;
  *(uint16_t*)0x20000de8 = 4;
  *(uint16_t*)0x20000dea = 5;
  *(uint32_t*)0x20000dec = 0x10001;
  *(uint32_t*)0x20000df0 = 0;
  *(uint32_t*)0x20000df4 = 0x34cd;
  *(uint32_t*)0x20000df8 = 6;
  *(uint16_t*)0x20000dfc = 0xc;
  *(uint16_t*)0x20000dfe = 8;
  *(uint64_t*)0x20000e00 = 0x163;
  *(uint16_t*)0x20000e08 = 0x3c;
  *(uint16_t*)0x20000e0a = 1;
  *(uint32_t*)0x20000e0c = 6;
  *(uint32_t*)0x20000e10 = 4;
  *(uint32_t*)0x20000e14 = 1;
  *(uint32_t*)0x20000e18 = 1;
  *(uint32_t*)0x20000e1c = 9;
  *(uint8_t*)0x20000e20 = 7;
  *(uint8_t*)0x20000e21 = 1;
  *(uint16_t*)0x20000e22 = 8;
  *(uint16_t*)0x20000e24 = 0xfffe;
  *(uint16_t*)0x20000e26 = 6;
  *(uint32_t*)0x20000e28 = 6;
  *(uint8_t*)0x20000e2c = 4;
  *(uint8_t*)0x20000e2d = 2;
  *(uint16_t*)0x20000e2e = 7;
  *(uint16_t*)0x20000e30 = 2;
  *(uint16_t*)0x20000e32 = 5;
  *(uint32_t*)0x20000e34 = 9;
  *(uint32_t*)0x20000e38 = 0xfffffffd;
  *(uint32_t*)0x20000e3c = 4;
  *(uint32_t*)0x20000e40 = 6;
  *(uint16_t*)0x20000e44 = 8;
  *(uint16_t*)0x20000e46 = 5;
  *(uint32_t*)0x20000e48 = 0x10000;
  *(uint16_t*)0x20000e4c = 8;
  *(uint16_t*)0x20000e4e = 5;
  *(uint32_t*)0x20000e50 = 7;
  *(uint16_t*)0x20000e54 = 8;
  *(uint16_t*)0x20000e56 = 5;
  *(uint32_t*)0x20000e58 = 0xfff;
  *(uint16_t*)0x20000e5c = 8;
  *(uint16_t*)0x20000e5e = 4;
  *(uint32_t*)0x20000e60 = 0;
  *(uint16_t*)0x20000e64 = 0x25c;
  *(uint16_t*)0x20000e66 = 7;
  *(uint16_t*)0x20000e68 = 0xfc;
  STORE_BY_BITMASK(uint16_t, , 0x20000e6a, 4, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000e6b, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000e6b, 0, 7, 1);
  *(uint16_t*)0x20000e6c = 7;
  *(uint16_t*)0x20000e6e = 1;
  memcpy((void*)0x20000e70, "xt\000", 3);
  *(uint16_t*)0x20000e74 = 0xd4;
  STORE_BY_BITMASK(uint16_t, , 0x20000e76, 2, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000e77, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000e77, 1, 7, 1);
  *(uint16_t*)0x20000e78 = 0x2a;
  *(uint16_t*)0x20000e7a = 6;
  *(uint16_t*)0x20000e7c = 5;
  memcpy((void*)0x20000e7e, "security\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000",
         32);
  *(uint8_t*)0x20000e9e = 0;
  *(uint16_t*)0x20000ea0 = 0x400;
  *(uint16_t*)0x20000ea4 = 8;
  *(uint16_t*)0x20000ea6 = 3;
  *(uint32_t*)0x20000ea8 = 3;
  *(uint16_t*)0x20000eac = 0x24;
  *(uint16_t*)0x20000eae = 1;
  memcpy((void*)0x20000eb0, "raw\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000",
         32);
  *(uint16_t*)0x20000ed0 = 8;
  *(uint16_t*)0x20000ed2 = 3;
  *(uint32_t*)0x20000ed4 = 0x3fa2;
  *(uint16_t*)0x20000ed8 = 8;
  *(uint16_t*)0x20000eda = 2;
  *(uint32_t*)0x20000edc = 3;
  *(uint16_t*)0x20000ee0 = 8;
  *(uint16_t*)0x20000ee2 = 3;
  *(uint32_t*)0x20000ee4 = 0x8000;
  *(uint16_t*)0x20000ee8 = 0x2a;
  *(uint16_t*)0x20000eea = 6;
  *(uint16_t*)0x20000eec = 5;
  memcpy((void*)0x20000eee, "security\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000",
         32);
  *(uint8_t*)0x20000f0e = 5;
  *(uint16_t*)0x20000f10 = 3;
  *(uint16_t*)0x20000f14 = 8;
  *(uint16_t*)0x20000f16 = 3;
  *(uint32_t*)0x20000f18 = 0x10001;
  *(uint16_t*)0x20000f1c = 0x24;
  *(uint16_t*)0x20000f1e = 1;
  memcpy((void*)0x20000f20, "security\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000",
         32);
  *(uint16_t*)0x20000f40 = 8;
  *(uint16_t*)0x20000f42 = 2;
  *(uint32_t*)0x20000f44 = 0;
  *(uint16_t*)0x20000f48 = 4;
  *(uint16_t*)0x20000f4a = 6;
  *(uint16_t*)0x20000f4c = 0xc;
  *(uint16_t*)0x20000f4e = 7;
  *(uint32_t*)0x20000f50 = 0;
  *(uint32_t*)0x20000f54 = 0;
  *(uint16_t*)0x20000f58 = 0xc;
  *(uint16_t*)0x20000f5a = 8;
  *(uint32_t*)0x20000f5c = 0;
  *(uint32_t*)0x20000f60 = 0;
  *(uint16_t*)0x20000f64 = 0x58;
  STORE_BY_BITMASK(uint16_t, , 0x20000f66, 0x10, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000f67, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000f67, 0, 7, 1);
  *(uint16_t*)0x20000f68 = 8;
  *(uint16_t*)0x20000f6a = 1;
  memcpy((void*)0x20000f6c, "ipt\000", 4);
  *(uint16_t*)0x20000f70 = 0x30;
  STORE_BY_BITMASK(uint16_t, , 0x20000f72, 2, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000f73, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000f73, 1, 7, 1);
  *(uint16_t*)0x20000f74 = 0x2a;
  *(uint16_t*)0x20000f76 = 6;
  *(uint16_t*)0x20000f78 = 0x200;
  memcpy((void*)0x20000f7a, "nat\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000",
         32);
  *(uint8_t*)0x20000f9a = 0xb3;
  *(uint16_t*)0x20000f9c = 0xc30;
  *(uint16_t*)0x20000fa0 = 4;
  *(uint16_t*)0x20000fa2 = 6;
  *(uint16_t*)0x20000fa4 = 0xc;
  *(uint16_t*)0x20000fa6 = 7;
  *(uint32_t*)0x20000fa8 = 1;
  *(uint32_t*)0x20000fac = 1;
  *(uint16_t*)0x20000fb0 = 0xc;
  *(uint16_t*)0x20000fb2 = 8;
  *(uint32_t*)0x20000fb4 = 0;
  *(uint32_t*)0x20000fb8 = 0;
  *(uint16_t*)0x20000fbc = 0xc0;
  STORE_BY_BITMASK(uint16_t, , 0x20000fbe, 2, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000fbf, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000fbf, 0, 7, 1);
  *(uint16_t*)0x20000fc0 = 0xd;
  *(uint16_t*)0x20000fc2 = 1;
  memcpy((void*)0x20000fc4, "connmark\000", 9);
  *(uint16_t*)0x20000fd0 = 0x90;
  STORE_BY_BITMASK(uint16_t, , 0x20000fd2, 2, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000fd3, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000fd3, 1, 7, 1);
  *(uint16_t*)0x20000fd4 = 0x1c;
  *(uint16_t*)0x20000fd6 = 1;
  *(uint32_t*)0x20000fd8 = 4;
  *(uint32_t*)0x20000fdc = 0x8d;
  *(uint32_t*)0x20000fe0 = 3;
  *(uint32_t*)0x20000fe4 = 9;
  *(uint32_t*)0x20000fe8 = 0x328000;
  *(uint16_t*)0x20000fec = 2;
  *(uint16_t*)0x20000ff0 = 0x1c;
  *(uint16_t*)0x20000ff2 = 1;
  *(uint32_t*)0x20000ff4 = 9;
  *(uint32_t*)0x20000ff8 = 0x81;
  *(uint32_t*)0x20000ffc = 0x78000002;
  *(uint32_t*)0x20001000 = 0xafd;
  *(uint32_t*)0x20001004 = 0x7ff;
  *(uint16_t*)0x20001008 = 0x7b4;
  *(uint16_t*)0x2000100c = 0x1c;
  *(uint16_t*)0x2000100e = 1;
  *(uint32_t*)0x20001010 = 0x20;
  *(uint32_t*)0x20001014 = 6;
  *(uint32_t*)0x20001018 = 2;
  *(uint32_t*)0x2000101c = 6;
  *(uint32_t*)0x20001020 = 8;
  *(uint16_t*)0x20001024 = 0xff;
  *(uint16_t*)0x20001028 = 0x1c;
  *(uint16_t*)0x2000102a = 1;
  *(uint32_t*)0x2000102c = 0x1ff;
  *(uint32_t*)0x20001030 = 1;
  *(uint32_t*)0x20001034 = 8;
  *(uint32_t*)0x20001038 = 8;
  *(uint32_t*)0x2000103c = 0x10001;
  *(uint16_t*)0x20001040 = 9;
  *(uint16_t*)0x20001044 = 0x1c;
  *(uint16_t*)0x20001046 = 1;
  *(uint32_t*)0x20001048 = 8;
  *(uint32_t*)0x2000104c = 9;
  *(uint32_t*)0x20001050 = 6;
  *(uint32_t*)0x20001054 = 8;
  *(uint32_t*)0x20001058 = 0x7f;
  *(uint16_t*)0x2000105c = 0x782;
  *(uint16_t*)0x20001060 = 4;
  *(uint16_t*)0x20001062 = 6;
  *(uint16_t*)0x20001064 = 0xc;
  *(uint16_t*)0x20001066 = 7;
  *(uint32_t*)0x20001068 = 0;
  *(uint32_t*)0x2000106c = 0;
  *(uint16_t*)0x20001070 = 0xc;
  *(uint16_t*)0x20001072 = 8;
  *(uint32_t*)0x20001074 = 0;
  *(uint32_t*)0x20001078 = 0;
  *(uint16_t*)0x2000107c = 0x44;
  STORE_BY_BITMASK(uint16_t, , 0x2000107e, 0x20, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000107f, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000107f, 0, 7, 1);
  *(uint16_t*)0x20001080 = 0xf;
  *(uint16_t*)0x20001082 = 1;
  memcpy((void*)0x20001084, "tunnel_key\000", 11);
  *(uint16_t*)0x20001090 = 0x14;
  STORE_BY_BITMASK(uint16_t, , 0x20001092, 2, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20001093, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20001093, 1, 7, 1);
  *(uint16_t*)0x20001094 = 6;
  *(uint16_t*)0x20001096 = 9;
  *(uint16_t*)0x20001098 = htobe16(0x4e24);
  *(uint16_t*)0x2000109c = 8;
  *(uint16_t*)0x2000109e = 7;
  *(uint32_t*)0x200010a0 = 5;
  *(uint16_t*)0x200010a4 = 4;
  *(uint16_t*)0x200010a6 = 6;
  *(uint16_t*)0x200010a8 = 0xc;
  *(uint16_t*)0x200010aa = 7;
  *(uint32_t*)0x200010ac = 0;
  *(uint32_t*)0x200010b0 = 0;
  *(uint16_t*)0x200010b4 = 0xc;
  *(uint16_t*)0x200010b6 = 8;
  *(uint32_t*)0x200010b8 = 7;
  *(uint32_t*)0x200010bc = 0;
  *(uint16_t*)0x200010c0 = 8;
  *(uint16_t*)0x200010c2 = 1;
  *(uint32_t*)0x200010c4 = 0x9d9;
  *(uint16_t*)0x200010c8 = 8;
  *(uint16_t*)0x200010ca = 5;
  *(uint16_t*)0x200010cc = 9;
  *(uint16_t*)0x200010ce = 0xd;
  *(uint64_t*)0x20000188 = 0xb50;
  *(uint64_t*)0x20000318 = 1;
  *(uint64_t*)0x20000320 = 0;
  *(uint64_t*)0x20000328 = 0;
  *(uint32_t*)0x20000330 = 0;
  syscall(__NR_sendmsg, -1, 0x20000300ul, 0ul);
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
