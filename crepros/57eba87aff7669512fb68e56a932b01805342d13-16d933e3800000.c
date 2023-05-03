// https://syzkaller.appspot.com/bug?id=57eba87aff7669512fb68e56a932b01805342d13
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/sysmacros.h>
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

static uintptr_t syz_fuseblk_mount(uintptr_t a0, uintptr_t a1, uintptr_t a2,
                                   uintptr_t a3, uintptr_t a4, uintptr_t a5,
                                   uintptr_t a6, uintptr_t a7)
{
  uint64_t target = a0;
  uint64_t blkdev = a1;
  uint64_t mode = a2;
  uint64_t uid = a3;
  uint64_t gid = a4;
  uint64_t maxread = a5;
  uint64_t blksize = a6;
  uint64_t flags = a7;

  int fd = open("/dev/fuse", O_RDWR);
  if (fd == -1)
    return fd;
  if (syscall(SYS_mknodat, AT_FDCWD, blkdev, S_IFBLK, makedev(7, 199)))
    return fd;
  char buf[256];
  sprintf(buf, "fd=%d,user_id=%ld,group_id=%ld,rootmode=0%o", fd, (long)uid,
          (long)gid, (unsigned)mode & ~3u);
  if (maxread != 0)
    sprintf(buf + strlen(buf), ",max_read=%ld", (long)maxread);
  if (blksize != 0)
    sprintf(buf + strlen(buf), ",blksize=%ld", (long)blksize);
  if (mode & 1)
    strcat(buf, ",default_permissions");
  if (mode & 2)
    strcat(buf, ",allow_other");
  syscall(SYS_mount, blkdev, target, "fuseblk", flags, buf);
  return fd;
}

static void execute_one();
extern unsigned long long procid;

void loop()
{
  while (1) {
    execute_one();
  }
}

unsigned long long procid;
void execute_one()
{
  memcpy((void*)0x20000000, "./file0", 8);
  syscall(__NR_creat, 0x20000000, 0);
  memcpy((void*)0x20000080, "./file0", 8);
  memcpy((void*)0x200000c0, "./file1", 8);
  syz_fuseblk_mount(0x20000080, 0x200000c0, 0x4000, 0, 0, 0, 0, 0);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  char* cwd = get_current_dir_name();
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      for (;;) {
        if (chdir(cwd))
          fail("failed to chdir");
        use_temporary_dir();
        loop();
      }
    }
  }
  sleep(1000000);
  return 0;
}
