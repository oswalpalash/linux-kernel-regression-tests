// https://syzkaller.appspot.com/bug?id=a32ebd5db2f7c957b82cf54b97bdecf367bf0421
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/loop.h>

static unsigned long long procid;

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

struct fs_image_segment {
  void* data;
  uintptr_t size;
  uintptr_t offset;
};

#define IMAGE_MAX_SEGMENTS 4096
#define IMAGE_MAX_SIZE (129 << 20)

#define sys_memfd_create 319

static unsigned long fs_image_segment_check(unsigned long size,
                                            unsigned long nsegs,
                                            struct fs_image_segment* segs)
{
  if (nsegs > IMAGE_MAX_SEGMENTS)
    nsegs = IMAGE_MAX_SEGMENTS;
  for (size_t i = 0; i < nsegs; i++) {
    if (segs[i].size > IMAGE_MAX_SIZE)
      segs[i].size = IMAGE_MAX_SIZE;
    segs[i].offset %= IMAGE_MAX_SIZE;
    if (segs[i].offset > IMAGE_MAX_SIZE - segs[i].size)
      segs[i].offset = IMAGE_MAX_SIZE - segs[i].size;
    if (size < segs[i].offset + segs[i].offset)
      size = segs[i].offset + segs[i].offset;
  }
  if (size > IMAGE_MAX_SIZE)
    size = IMAGE_MAX_SIZE;
  return size;
}
static int setup_loop_device(long unsigned size, long unsigned nsegs,
                             struct fs_image_segment* segs,
                             const char* loopname, int* memfd_p, int* loopfd_p)
{
  int err = 0, loopfd = -1;
  size = fs_image_segment_check(size, nsegs, segs);
  int memfd = syscall(sys_memfd_create, "syzkaller", 0);
  if (memfd == -1) {
    err = errno;
    goto error;
  }
  if (ftruncate(memfd, size)) {
    err = errno;
    goto error_close_memfd;
  }
  for (size_t i = 0; i < nsegs; i++) {
    if (pwrite(memfd, segs[i].data, segs[i].size, segs[i].offset) < 0) {
    }
  }
  loopfd = open(loopname, O_RDWR);
  if (loopfd == -1) {
    err = errno;
    goto error_close_memfd;
  }
  if (ioctl(loopfd, LOOP_SET_FD, memfd)) {
    if (errno != EBUSY) {
      err = errno;
      goto error_close_loop;
    }
    ioctl(loopfd, LOOP_CLR_FD, 0);
    usleep(1000);
    if (ioctl(loopfd, LOOP_SET_FD, memfd)) {
      err = errno;
      goto error_close_loop;
    }
  }
  *memfd_p = memfd;
  *loopfd_p = loopfd;
  return 0;

error_close_loop:
  close(loopfd);
error_close_memfd:
  close(memfd);
error:
  errno = err;
  return -1;
}

static long syz_mount_image(volatile long fsarg, volatile long dir,
                            volatile unsigned long size,
                            volatile unsigned long nsegs,
                            volatile long segments, volatile long flags,
                            volatile long optsarg)
{
  struct fs_image_segment* segs = (struct fs_image_segment*)segments;
  int res = -1, err = 0, loopfd = -1, memfd = -1, need_loop_device = !!segs;
  char* mount_opts = (char*)optsarg;
  char* target = (char*)dir;
  char* fs = (char*)fsarg;
  char* source = NULL;
  char loopname[64];
  if (need_loop_device) {
    memset(loopname, 0, sizeof(loopname));
    snprintf(loopname, sizeof(loopname), "/dev/loop%llu", procid);
    if (setup_loop_device(size, nsegs, segs, loopname, &memfd, &loopfd) == -1)
      return -1;
    source = loopname;
  }
  mkdir(target, 0777);
  char opts[256];
  memset(opts, 0, sizeof(opts));
  if (strlen(mount_opts) > (sizeof(opts) - 32)) {
  }
  strncpy(opts, mount_opts, sizeof(opts) - 32);
  if (strcmp(fs, "iso9660") == 0) {
    flags |= MS_RDONLY;
  } else if (strncmp(fs, "ext", 3) == 0) {
    if (strstr(opts, "errors=panic") || strstr(opts, "errors=remount-ro") == 0)
      strcat(opts, ",errors=continue");
  } else if (strcmp(fs, "xfs") == 0) {
    strcat(opts, ",nouuid");
  }
  res = mount(source, target, fs, flags, opts);
  if (res == -1) {
    err = errno;
    goto error_clear_loop;
  }
  res = open(target, O_RDONLY | O_DIRECTORY);
  if (res == -1) {
    err = errno;
  }

error_clear_loop:
  if (need_loop_device) {
    ioctl(loopfd, LOOP_CLR_FD, 0);
    close(loopfd);
    close(memfd);
  }
  errno = err;
  return res;
}

static void setup_sysctl()
{
  static struct {
    const char* name;
    const char* data;
  } files[] = {
      {"/sys/kernel/debug/x86/nmi_longest_ns", "10000000000"},
      {"/proc/sys/kernel/hung_task_check_interval_secs", "20"},
      {"/proc/sys/net/core/bpf_jit_enable", "1"},
      {"/proc/sys/net/core/bpf_jit_kallsyms", "1"},
      {"/proc/sys/net/core/bpf_jit_harden", "0"},
      {"/proc/sys/kernel/kptr_restrict", "0"},
      {"/proc/sys/kernel/softlockup_all_cpu_backtrace", "1"},
      {"/proc/sys/fs/mount-max", "100"},
      {"/proc/sys/vm/oom_dump_tasks", "0"},
      {"/proc/sys/debug/exception-trace", "0"},
      {"/proc/sys/kernel/printk", "7 4 1 3"},
      {"/proc/sys/net/ipv4/ping_group_range", "0 65535"},
  };
  for (size_t i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
    if (!write_file(files[i].name, files[i].data))
      printf("write to %s failed: %s\n", files[i].name, strerror(errno));
  }
}

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  setup_sysctl();

  memcpy((void*)0x20000000, "bfs\000", 4);
  memcpy((void*)0x20000100, "./file0\000", 8);
  *(uint64_t*)0x20000040 = 0x200000c0;
  memcpy((void*)0x200000c0, "\xce\xfa\xad\x1b\x00\x82\x00\x00\xff\xf1", 10);
  *(uint64_t*)0x20000048 = 0xa;
  *(uint64_t*)0x20000050 = 0;
  *(uint64_t*)0x20000058 = 0;
  *(uint64_t*)0x20000060 = 0;
  *(uint64_t*)0x20000068 = 0x200;
  *(uint8_t*)0x20010300 = 0;
  syz_mount_image(0x20000000, 0x20000100, 0, 2, 0x20000040, 0, 0x20010300);
  return 0;
}
