// https://syzkaller.appspot.com/bug?id=63ba1a37eb575a6c6371e5d4ab1d64147a98ba18
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <fcntl.h>
#include <linux/capability.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
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
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

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

#define BITMASK_LEN(type, bf_len) (type)((1ull << (bf_len)) - 1)

#define BITMASK_LEN_OFF(type, bf_off, bf_len)                                  \
  (type)(BITMASK_LEN(type, (bf_len)) << (bf_off))

#define STORE_BY_BITMASK(type, addr, val, bf_off, bf_len)                      \
  if ((bf_off) == 0 && (bf_len) == 0) {                                        \
    *(type*)(addr) = (type)(val);                                              \
  } else {                                                                     \
    type new_val = *(type*)(addr);                                             \
    new_val &= ~BITMASK_LEN_OFF(type, (bf_off), (bf_len));                     \
    new_val |= ((type)(val)&BITMASK_LEN(type, (bf_len))) << (bf_off);          \
    *(type*)(addr) = new_val;                                                  \
  }

struct csum_inet {
  uint32_t acc;
};

static void csum_inet_init(struct csum_inet* csum)
{
  csum->acc = 0;
}

static void csum_inet_update(struct csum_inet* csum, const uint8_t* data,
                             size_t length)
{
  if (length == 0)
    return;

  size_t i;
  for (i = 0; i < length - 1; i += 2)
    csum->acc += *(uint16_t*)&data[i];

  if (length & 1)
    csum->acc += (uint16_t)data[length - 1];

  while (csum->acc > 0xffff)
    csum->acc = (csum->acc & 0xffff) + (csum->acc >> 16);
}

static uint16_t csum_inet_digest(struct csum_inet* csum)
{
  return ~csum->acc;
}

static void use_temporary_dir()
{
  char tmpdir_template[] = "./syzkaller.XXXXXX";
  char* tmpdir = mkdtemp(tmpdir_template);
  if (!tmpdir)
    fail("failed to mkdtemp");
  if (chmod(tmpdir, 0777))
    fail("failed to chmod");
  if (chdir(tmpdir))
    fail("failed to chdir");
}

static uintptr_t syz_open_dev(uintptr_t a0, uintptr_t a1, uintptr_t a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf));
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();

  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = 128 << 20;
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 8 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);

#define CLONE_NEWCGROUP 0x02000000

  unshare(CLONE_NEWNS);
  unshare(CLONE_NEWIPC);
  unshare(CLONE_NEWCGROUP);
  unshare(CLONE_NEWNET);
  unshare(CLONE_NEWUTS);
  unshare(CLONE_SYSVSEM);
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
    close(fd);
    return false;
  }
  close(fd);
  return true;
}

static int real_uid;
static int real_gid;
__attribute__((aligned(64 << 10))) static char sandbox_stack[1 << 20];

static int namespace_sandbox_proc(void* arg)
{
  sandbox_common();

  write_file("/proc/self/setgroups", "deny");
  if (!write_file("/proc/self/uid_map", "0 %d 1\n", real_uid))
    fail("write of /proc/self/uid_map failed");
  if (!write_file("/proc/self/gid_map", "0 %d 1\n", real_gid))
    fail("write of /proc/self/gid_map failed");

  if (mkdir("./syz-tmp", 0777))
    fail("mkdir(syz-tmp) failed");
  if (mount("", "./syz-tmp", "tmpfs", 0, NULL))
    fail("mount(tmpfs) failed");
  if (mkdir("./syz-tmp/newroot", 0777))
    fail("mkdir failed");
  if (mkdir("./syz-tmp/newroot/dev", 0700))
    fail("mkdir failed");
  if (mount("/dev", "./syz-tmp/newroot/dev", NULL,
            MS_BIND | MS_REC | MS_PRIVATE, NULL))
    fail("mount(dev) failed");
  if (mkdir("./syz-tmp/newroot/proc", 0700))
    fail("mkdir failed");
  if (mount(NULL, "./syz-tmp/newroot/proc", "proc", 0, NULL))
    fail("mount(proc) failed");
  if (mkdir("./syz-tmp/pivot", 0777))
    fail("mkdir failed");
  if (syscall(SYS_pivot_root, "./syz-tmp", "./syz-tmp/pivot")) {
    if (chdir("./syz-tmp"))
      fail("chdir failed");
  } else {
    if (chdir("/"))
      fail("chdir failed");
    if (umount2("./pivot", MNT_DETACH))
      fail("umount failed");
  }
  if (chroot("./newroot"))
    fail("chroot failed");
  if (chdir("/"))
    fail("chdir failed");

  struct __user_cap_header_struct cap_hdr = {};
  struct __user_cap_data_struct cap_data[2] = {};
  cap_hdr.version = _LINUX_CAPABILITY_VERSION_3;
  cap_hdr.pid = getpid();
  if (syscall(SYS_capget, &cap_hdr, &cap_data))
    fail("capget failed");
  cap_data[0].effective &= ~(1 << CAP_SYS_PTRACE);
  cap_data[0].permitted &= ~(1 << CAP_SYS_PTRACE);
  cap_data[0].inheritable &= ~(1 << CAP_SYS_PTRACE);
  if (syscall(SYS_capset, &cap_hdr, &cap_data))
    fail("capset failed");

  loop();
  doexit(1);
}

static int do_sandbox_namespace(int executor_pid, bool enable_tun)
{
  int pid;

  real_uid = getuid();
  real_gid = getgid();
  mprotect(sandbox_stack, 4096, PROT_NONE);
  pid =
      clone(namespace_sandbox_proc, &sandbox_stack[sizeof(sandbox_stack) - 64],
            CLONE_NEWUSER | CLONE_NEWPID, NULL);
  if (pid < 0)
    fail("sandbox clone failed");
  return pid;
}

#ifndef __NR_write
#define __NR_write 4
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#ifndef __NR_fcntl
#define __NR_fcntl 55
#endif
#ifndef __NR_ioctl
#define __NR_ioctl 54
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[3];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 2, 1, 0);
  memcpy((void*)0x20af1ff3, "/dev/net/tun", 13);
  r[1] = syz_open_dev(0x20af1ff3, 0, 0xe6009f61);
  r[2] = syscall(__NR_fcntl, r[1], 0, r[1]);
  *(uint8_t*)0x20ca1000 = 0x73;
  *(uint8_t*)0x20ca1001 = 0x79;
  *(uint8_t*)0x20ca1002 = 0x7a;
  *(uint8_t*)0x20ca1003 = 0x30;
  *(uint8_t*)0x20ca1004 = 0;
  *(uint32_t*)0x20ca1010 = 5;
  syscall(__NR_ioctl, r[1], 0x400454ca, 0x20ca1000);
  *(uint8_t*)0x2062ffe0 = 0x73;
  *(uint8_t*)0x2062ffe1 = 0x79;
  *(uint8_t*)0x2062ffe2 = 0x7a;
  *(uint8_t*)0x2062ffe3 = 0x30;
  *(uint8_t*)0x2062ffe4 = 0;
  *(uint16_t*)0x2062fff0 = 0x301;
  syscall(__NR_ioctl, r[0], 0x8914, 0x2062ffe0);
  *(uint16_t*)0x20ab08cd = 0;
  *(uint16_t*)0x20ab08cf = htobe16(0xd);
  STORE_BY_BITMASK(uint8_t, 0x20ab08d1, 0x1c, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x20ab08d1, 4, 4, 4);
  STORE_BY_BITMASK(uint8_t, 0x20ab08d2, 5, 0, 2);
  STORE_BY_BITMASK(uint8_t, 0x20ab08d2, 0, 2, 6);
  *(uint16_t*)0x20ab08d3 = htobe16(0x80);
  *(uint16_t*)0x20ab08d5 = htobe16(0x66);
  *(uint16_t*)0x20ab08d7 = htobe16(0x800);
  *(uint8_t*)0x20ab08d9 = -1;
  *(uint8_t*)0x20ab08da = 0x32;
  *(uint16_t*)0x20ab08db = 0;
  *(uint32_t*)0x20ab08dd = htobe32(0x7f000001);
  *(uint32_t*)0x20ab08e1 = htobe32(-1);
  *(uint8_t*)0x20ab08e5 = 0x44;
  *(uint8_t*)0x20ab08e6 = 0x1c;
  *(uint8_t*)0x20ab08e7 = 0;
  STORE_BY_BITMASK(uint8_t, 0x20ab08e8, 3, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x20ab08e8, 0x7a, 4, 4);
  *(uint32_t*)0x20ab08e9 = htobe32(0xe0000001);
  *(uint32_t*)0x20ab08ed = htobe32(0);
  *(uint32_t*)0x20ab08f1 = htobe32(0);
  *(uint32_t*)0x20ab08f5 = htobe32(0x80);
  *(uint32_t*)0x20ab08f9 = htobe32(0xe0000002);
  *(uint32_t*)0x20ab08fd = htobe32(6);
  *(uint8_t*)0x20ab0901 = 7;
  *(uint8_t*)0x20ab0902 = 7;
  *(uint8_t*)0x20ab0903 = 9;
  *(uint32_t*)0x20ab0904 = htobe32(0xe0000001);
  *(uint8_t*)0x20ab0908 = 7;
  *(uint8_t*)0x20ab0909 = 0xb;
  *(uint8_t*)0x20ab090a = 1;
  *(uint32_t*)0x20ab090b = htobe32(0x7fffffff);
  *(uint8_t*)0x20ab090f = 0xac;
  *(uint8_t*)0x20ab0910 = 0x14;
  *(uint8_t*)0x20ab0911 = 0;
  *(uint8_t*)0x20ab0912 = 0xc;
  *(uint8_t*)0x20ab0913 = 0x89;
  *(uint8_t*)0x20ab0914 = 0x2b;
  *(uint8_t*)0x20ab0915 = 0;
  *(uint32_t*)0x20ab0916 = htobe32(0);
  *(uint32_t*)0x20ab091a = htobe32(-1);
  *(uint8_t*)0x20ab091e = 0xac;
  *(uint8_t*)0x20ab091f = 0x14;
  *(uint8_t*)0x20ab0920 = 0;
  *(uint8_t*)0x20ab0921 = 0xaa;
  *(uint32_t*)0x20ab0922 = htobe32(0);
  *(uint32_t*)0x20ab0926 = htobe32(0);
  *(uint8_t*)0x20ab092a = 0xac;
  *(uint8_t*)0x20ab092b = 0x14;
  *(uint8_t*)0x20ab092c = 0;
  *(uint8_t*)0x20ab092d = 0xaa;
  *(uint32_t*)0x20ab092e = htobe32(0xe0000001);
  *(uint8_t*)0x20ab0932 = 0xac;
  *(uint8_t*)0x20ab0933 = 0x14;
  *(uint8_t*)0x20ab0934 = 0;
  *(uint8_t*)0x20ab0935 = 0xd;
  *(uint32_t*)0x20ab0936 = htobe32(-1);
  *(uint32_t*)0x20ab093a = htobe32(0xe0000001);
  *(uint8_t*)0x20ab093e = 0;
  *(uint16_t*)0x20ab0941 = htobe16(0x4e21);
  *(uint16_t*)0x20ab0943 = htobe16(0x4e23);
  *(uint8_t*)0x20ab0945 = 4;
  STORE_BY_BITMASK(uint8_t, 0x20ab0946, 1, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x20ab0946, 0x80, 4, 4);
  *(uint16_t*)0x20ab0947 = 0;
  STORE_BY_BITMASK(uint8_t, 0x20ab0949, 0, 0, 1);
  STORE_BY_BITMASK(uint8_t, 0x20ab0949, 0xe, 1, 4);
  STORE_BY_BITMASK(uint8_t, 0x20ab0949, 0x4f, 5, 3);
  memcpy((void*)0x20ab094a, "\xf1\x26\x6c", 3);
  *(uint8_t*)0x20ab094d = 0;
  memcpy((void*)0x20ab094e, "\x73\x99\xda", 3);
  struct csum_inet csum_1;
  csum_inet_init(&csum_1);
  csum_inet_update(&csum_1, (const uint8_t*)0x20ab08dd, 4);
  csum_inet_update(&csum_1, (const uint8_t*)0x20ab08e1, 4);
  uint16_t csum_1_chunk_2 = 0x2100;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_2, 2);
  uint16_t csum_1_chunk_3 = 0x1000;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_3, 2);
  csum_inet_update(&csum_1, (const uint8_t*)0x20ab0941, 16);
  *(uint16_t*)0x20ab0947 = csum_inet_digest(&csum_1);
  struct csum_inet csum_2;
  csum_inet_init(&csum_2);
  csum_inet_update(&csum_2, (const uint8_t*)0x20ab08d1, 112);
  *(uint16_t*)0x20ab08db = csum_inet_digest(&csum_2);
  syscall(__NR_write, r[2], 0x20ab08cd, 0x84);
}

int main()
{
  use_temporary_dir();
  int pid = do_sandbox_namespace(0, false);
  int status = 0;
  while (waitpid(pid, &status, __WALL) != pid) {
  }
  return 0;
}
