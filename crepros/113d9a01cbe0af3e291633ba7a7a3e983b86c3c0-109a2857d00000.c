// https://syzkaller.appspot.com/bug?id=113d9a01cbe0af3e291633ba7a7a3e983b86c3c0
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/loop.h>

static unsigned long long procid;

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

static void reset_loop()
{
  char buf[64];
  snprintf(buf, sizeof(buf), "/dev/loop%llu", procid);
  int loopfd = open(buf, O_RDWR);
  if (loopfd != -1) {
    ioctl(loopfd, LOOP_CLR_FD, 0);
    close(loopfd);
  }
}

static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  write_file("/proc/self/oom_score_adj", "1000");
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter = 0;
  for (;; iter++) {
    reset_loop();
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      setup_test();
      execute_one();
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

void execute_one(void)
{
  memcpy((void*)0x20000000, "btrfs\000", 6);
  memcpy((void*)0x20000100, "./file0\000", 8);
  *(uint64_t*)0x20000200 = 0x20000a80;
  memcpy(
      (void*)0x20000a80,
      "\x90\xe4\x2e\x85\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf9\x0c\xac\x8b"
      "\x04\x4b\x4f\xa8\x8b\xee\x4b\x8d\x3d\xa8\x8d\xc2\x00\x00\x01\x00\x00\x00"
      "\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x5f\x42\x48\x52\x66\x53\x5f\x4d"
      "\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x50\x00\x00\x00\x00\x00\x00\x10"
      "\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\xd0\x00"
      "\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00\x00\x10\x00"
      "\x00\x61\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x45\x03\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01"
      "\x00\x00\x00\x00\x00\x00\x72\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x10"
      "\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x11\x00\x00\x00"
      "\x1a\x88\x85\xd6\x1a\xee\x4f\xeb\xb6\x9b\xd3\x35\x46\xbd\x0e\x04\xf9\x0c"
      "\xac\x8b\x04\x4b\x4f\xa8\x8b\xee\x4b\x8d\x3d\xa8\x8d\xc2\x53\xa4\x9a\x2c"
      "\xce\xaa\x7e\x66\x45\x87\xf2\x55\x05\x5f\xa5\x85",
      300);
  *(uint64_t*)0x20000208 = 0x12c;
  *(uint64_t*)0x20000210 = 0x10000;
  *(uint8_t*)0x20000040 = 0;
  syz_mount_image(0x20000000, 0x20000100, 0, 1, 0x20000200, 0, 0x20000040);
  *(uint64_t*)0x20000200 = 0x20010000;
  memcpy(
      (void*)0x20010000,
      "\x90\xe4\x2e\x85\x00\x00\x00\x00\x00\x00\x00\x01\xc1\x8d\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf9\x0c\xac\x8b"
      "\x04\x4b\x4f\xa8\x8b\xee\x4b\x8d\x3d\xa8\x8d\xc2\x00\x00\x01\x00\x00\x00"
      "\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x5f\x42\x48\x52\x66\x53\x5f\x4d"
      "\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x50\x00\x00\x00\x00\x00\x00\x10"
      "\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\xd0\x00\x00\x00\x00"
      "\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x10\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00\x61\x00"
      "\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x45\x03\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00"
      "\x00\x00\x00\x72\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00\x00"
      "\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x1a\x88\x85"
      "\xd6\x1a\xee\x4f\xeb\xb6\x9b\xd3\x35\x46\xbd\x0e\x04\xf9\x0c\xac\x8b\x04"
      "\x4b\x4f\xa8\x8b\xee\x4b\x8d\x3d\xa8\x8d\xc2\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
      320);
  *(uint64_t*)0x20000208 = 0x140;
  *(uint64_t*)0x20000210 = 0x10000;
  *(uint64_t*)0x20000218 = 0x20010200;
  memcpy((void*)0x20010200,
         "\000\000\000\000\000\000\000\000\000\000\000\a\000\000\000\000\000"
         "\000\000\a\000\000\000\000\000\000\000\000\000\000\000\000",
         32);
  *(uint64_t*)0x20000220 = 0x20;
  *(uint64_t*)0x20000228 = 0x10220;
  *(uint64_t*)0x20000230 = 0x20010300;
  memcpy((void*)0x20010300,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00"
         "\x00\x00\xe4\x00\x00\x10\x00\x00\x00\x00\x00\x00\x00\x40\x00\x00\x00"
         "\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00"
         "\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00"
         "\x00\x10\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x10\x00\x00\x00\x00\x00\x1a\x88\x85\xd6\x1a\xee\x4f\xeb\xb6\x9b"
         "\xd3\x35\x46\xbd\x0e\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         128);
  *(uint64_t*)0x20000238 = 0x80;
  *(uint64_t*)0x20000240 = 0x10320;
  *(uint64_t*)0x20000248 = 0x20010400;
  memcpy(
      (void*)0x20010400,
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10\x50\x00\x00\x00\x00"
      "\x00\x05\x00\x00\x00\x00\x00\x00\x00\x00\x10\x10\x00\x00\x00\x00\x00\x04"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x20\x50\x00\x00\x00\x00\x00\x05\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x50\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x70\x50\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x80\x50\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x01\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x01\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\xf0\x50\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00\x00\x10\x10"
      "\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\xb0\x50\x00\x00"
      "\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00\x00\xd0\x50\x00\x00\x00\x00"
      "\x00\x06\x00\x00\x00\x00\x00\x00\x00\x00\x00\x51\x00\x00\x00\x00\x00\x06"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x60\x50\x00\x00\x00\x00\x00\x06\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\xd0\x00\x00\x00"
      "\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x50\x00\x00\x00\x00\x00\x07\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x10\x10\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x10\x50\x00\x00\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x00"
      "\xd0\x50\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00\x00\x00\x51"
      "\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00\x00\x60\x50\x00\x00"
      "\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00"
      "\x00\x00\xd0\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x50\x50\x00\x00\x00\x00"
      "\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x10\x10\x00\x00\x00\x00\x00\x04"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x60\x50\x00\x00\x00\x00\x00\x04\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x50\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x70\x50\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x80\x50\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x01\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x01\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
      640);
  *(uint64_t*)0x20000250 = 0x280;
  *(uint64_t*)0x20000258 = 0x10b20;
  *(uint64_t*)0x20000260 = 0;
  *(uint64_t*)0x20000268 = 0;
  *(uint64_t*)0x20000270 = 0x100000;
  *(uint64_t*)0x20000278 = 0x20010800;
  memcpy(
      (void*)0x20010800,
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x19\x00"
      "\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00"
      "\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x01\x00"
      "\x00\x10\x00\x00\x01\x00\x01\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x69\x00\x00\x00\x00\x00\x1a\x88\x85\xd6\x1a\xee\x4f\xeb\xb6\x9b\xd3\x35"
      "\x46\xbd\x0e\x04\x00\x00\x19\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x01\x00\x00\x00\x01\x00\x00\x10\x00\x00\x01\x00\x01\x00\x01\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x50\x00\x00\x00\x00\x00\x1a\x88\x85\xd6"
      "\x1a\xee\x4f\xeb\xb6\x9b\xd3\x35\x46\xbd\x0e\x04\x00\x00\x40\x00\x00\x00"
      "\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00"
      "\x02\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00\x00\x10"
      "\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00"
      "\x00\x00\x00\x00\x1a\x88\x85\xd6\x1a\xee\x4f\xeb\xb6\x9b\xd3\x35\x46\xbd"
      "\x0e\x04\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00"
      "\x00\x00\x72\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00\x00\x10"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x1a\x88\x85\xd6"
      "\x1a\xee\x4f\xeb\xb6\x9b\xd3\x35\x46\xbd\x0e\x04\xf9\x0c\xac\x8b\x04\x4b"
      "\x4f\xa8\x8b\xee\x4b\x8d\x3d\xa8\x8d\xc2\xb7\x58\x48\x58\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\xf9\x0c\xac\x8b\x04\x4b\x4f\xa8\x8b\xee\x4b\x8d"
      "\x3d\xa8\x8d\xc2\x00\x10\x10\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00"
      "\x00\x01\xe1\xee\x0d\x5d\x64\xd8\x4a\x64\xbe\x71\x2c\x76\x36\x49\x6d\x14"
      "\x04\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x04\x00"
      "\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\xd8\x01\x00\x00\x00\x00\x00"
      "\x00\x00\x39\x0f\x00\x00\x62\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00"
      "\xe4\x00\x00\x10\x00\x00\x00\x00\x00\xe9\x0e\x00\x00\x50\x00\x00\x00\x00"
      "\x01\x00\x00\x00\x00\x00\x00\xe4\x00\x00\x50\x00\x00\x00\x00\x00\x99\x0e"
      "\x00\x00\x50\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\xe4\x00\x00\x69"
      "\x00\x00\x00\x00\x00\x49\x0e\x00\x00\x50\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00",
      576);
  *(uint64_t*)0x20000280 = 0x240;
  *(uint64_t*)0x20000288 = 0x100ea0;
  *(uint64_t*)0x20000290 = 0x20010b00;
  memcpy(
      (void*)0x20010b00,
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x19\x00"
      "\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00"
      "\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x01\x00"
      "\x00\x10\x00\x00\x01\x00\x01\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x69\x00\x00\x00\x00\x00\x1a\x88\x85\xd6\x1a\xee\x4f\xeb\xb6\x9b\xd3\x35"
      "\x46\xbd\x0e\x04\x00\x00\x19\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x01\x00\x00\x00\x01\x00\x00\x10\x00\x00\x01\x00\x01\x00\x01\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x50\x00\x00\x00\x00\x00\x1a\x88\x85\xd6"
      "\x1a\xee\x4f\xeb\xb6\x9b\xd3\x35\x46\xbd\x0e\x04\x00\x00\x40\x00\x00\x00"
      "\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00"
      "\x02\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00\x00\x10"
      "\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00"
      "\x00\x00\x00\x00\x1a\x88\x85\xd6\x1a\xee\x4f\xeb\xb6\x9b\xd3\x35\x46\xbd"
      "\x0e\x04\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00"
      "\x00\x00\x72\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00\x00\x10"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x1a\x88\x85\xd6"
      "\x1a\xee\x4f\xeb\xb6\x9b\xd3\x35\x46\xbd\x0e\x04\xf9\x0c\xac\x8b\x04\x4b"
      "\x4f\xa8\x8b\xee\x4b\x8d\x3d\xa8\x8d\xc2\x7d\xe5\x23\xe5\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\xf9\x0c\xac\x8b\x04\x4b\x4f\xa8\x8b\xee\x4b\x8d"
      "\x3d\xa8\x8d\xc2\x00\x20\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x01\xe1\xee\x0d\x5d\x64\xd8\x4a\x64\xbe\x71\x2c\x76\x36\x49\x6d\x14"
      "\x01\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x02\x00"
      "\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\xd8\x01\x00\x00\x00\x00\x00"
      "\x00\x00\x39\x0f\x00\x00\x62\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00"
      "\xe4\x00\x00\x10\x00\x00\x00\x00\x00\xe9\x0e\x00\x00\x50\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00",
      512);
  *(uint64_t*)0x20000298 = 0x200;
  *(uint64_t*)0x200002a0 = 0x101ea0;
  *(uint64_t*)0x200002a8 = 0;
  *(uint64_t*)0x200002b0 = 0;
  *(uint64_t*)0x200002b8 = 0x102f40;
  *(uint64_t*)0x200002c0 = 0;
  *(uint64_t*)0x200002c8 = 0;
  *(uint64_t*)0x200002d0 = 0x103fc0;
  *(uint64_t*)0x200002d8 = 0;
  *(uint64_t*)0x200002e0 = 0;
  *(uint64_t*)0x200002e8 = 0x105000;
  *(uint64_t*)0x200002f0 = 0;
  *(uint64_t*)0x200002f8 = 0;
  *(uint64_t*)0x20000300 = 0x106000;
  *(uint64_t*)0x20000308 = 0;
  *(uint64_t*)0x20000310 = 0;
  *(uint64_t*)0x20000318 = 0x106f00;
  *(uint64_t*)0x20000320 = 0;
  *(uint64_t*)0x20000328 = 0;
  *(uint64_t*)0x20000330 = 0x107fa0;
  *(uint64_t*)0x20000338 = 0;
  *(uint64_t*)0x20000340 = 0;
  *(uint64_t*)0x20000348 = 0x108f00;
  *(uint64_t*)0x20000350 = 0;
  *(uint64_t*)0x20000358 = 0;
  *(uint64_t*)0x20000360 = 0x500000;
  *(uint64_t*)0x20000368 = 0;
  *(uint64_t*)0x20000370 = 0;
  *(uint64_t*)0x20000378 = 0x500560;
  *(uint64_t*)0x20000380 = 0;
  *(uint64_t*)0x20000388 = 0;
  *(uint64_t*)0x20000390 = 0x500720;
  *(uint64_t*)0x20000398 = 0;
  *(uint64_t*)0x200003a0 = 0;
  *(uint64_t*)0x200003a8 = 0x500840;
  *(uint64_t*)0x200003b0 = 0;
  *(uint64_t*)0x200003b8 = 0;
  *(uint64_t*)0x200003c0 = 0x5008e0;
  *(uint64_t*)0x200003c8 = 0;
  *(uint64_t*)0x200003d0 = 0;
  *(uint64_t*)0x200003d8 = 0x5009e0;
  *(uint64_t*)0x200003e0 = 0;
  *(uint64_t*)0x200003e8 = 0;
  *(uint64_t*)0x200003f0 = 0x500b60;
  *(uint64_t*)0x200003f8 = 0;
  *(uint64_t*)0x20000400 = 0;
  *(uint64_t*)0x20000408 = 0x500c80;
  *(uint64_t*)0x20000410 = 0;
  *(uint64_t*)0x20000418 = 0;
  *(uint64_t*)0x20000420 = 0x500d20;
  *(uint64_t*)0x20000428 = 0;
  *(uint64_t*)0x20000430 = 0;
  *(uint64_t*)0x20000438 = 0x500e40;
  *(uint64_t*)0x20000440 = 0;
  *(uint64_t*)0x20000448 = 0;
  *(uint64_t*)0x20000450 = 0x500ee0;
  *(uint64_t*)0x20000458 = 0;
  *(uint64_t*)0x20000460 = 0;
  *(uint64_t*)0x20000468 = 0x501000;
  *(uint64_t*)0x20000470 = 0;
  *(uint64_t*)0x20000478 = 0;
  *(uint64_t*)0x20000480 = 0x501e40;
  *(uint64_t*)0x20000488 = 0;
  *(uint64_t*)0x20000490 = 0;
  *(uint64_t*)0x20000498 = 0x502e60;
  *(uint64_t*)0x200004a0 = 0;
  *(uint64_t*)0x200004a8 = 0;
  *(uint64_t*)0x200004b0 = 0x506000;
  *(uint64_t*)0x200004b8 = 0;
  *(uint64_t*)0x200004c0 = 0;
  *(uint64_t*)0x200004c8 = 0x506fe0;
  *(uint64_t*)0x200004d0 = 0;
  *(uint64_t*)0x200004d8 = 0;
  *(uint64_t*)0x200004e0 = 0x507f60;
  *(uint64_t*)0x200004e8 = 0;
  *(uint64_t*)0x200004f0 = 0;
  *(uint64_t*)0x200004f8 = 0x509000;
  *(uint64_t*)0x20000500 = 0;
  *(uint64_t*)0x20000508 = 0;
  *(uint64_t*)0x20000510 = 0x509f40;
  *(uint64_t*)0x20000518 = 0;
  *(uint64_t*)0x20000520 = 0;
  *(uint64_t*)0x20000528 = 0x50afe0;
  *(uint64_t*)0x20000530 = 0;
  *(uint64_t*)0x20000538 = 0;
  *(uint64_t*)0x20000540 = 0x50be40;
  *(uint64_t*)0x20000548 = 0;
  *(uint64_t*)0x20000550 = 0;
  *(uint64_t*)0x20000558 = 0x50cca0;
  *(uint64_t*)0x20000560 = 0;
  *(uint64_t*)0x20000568 = 0;
  *(uint64_t*)0x20000570 = 0x50e000;
  *(uint64_t*)0x20000578 = 0;
  *(uint64_t*)0x20000580 = 0;
  *(uint64_t*)0x20000588 = 0x50e6e0;
  *(uint64_t*)0x20000590 = 0;
  *(uint64_t*)0x20000598 = 0;
  *(uint64_t*)0x200005a0 = 0x50f560;
  *(uint64_t*)0x200005a8 = 0;
  *(uint64_t*)0x200005b0 = 0;
  *(uint64_t*)0x200005b8 = 0x50f720;
  *(uint64_t*)0x200005c0 = 0;
  *(uint64_t*)0x200005c8 = 0;
  *(uint64_t*)0x200005d0 = 0x50f840;
  *(uint64_t*)0x200005d8 = 0;
  *(uint64_t*)0x200005e0 = 0;
  *(uint64_t*)0x200005e8 = 0x50f8e0;
  *(uint64_t*)0x200005f0 = 0;
  *(uint64_t*)0x200005f8 = 0;
  *(uint64_t*)0x20000600 = 0x50f9e0;
  *(uint64_t*)0x20000608 = 0;
  *(uint64_t*)0x20000610 = 0;
  *(uint64_t*)0x20000618 = 0x50fb60;
  *(uint64_t*)0x20000620 = 0;
  *(uint64_t*)0x20000628 = 0;
  *(uint64_t*)0x20000630 = 0x50fc80;
  *(uint64_t*)0x20000638 = 0;
  *(uint64_t*)0x20000640 = 0;
  *(uint64_t*)0x20000648 = 0x50fd20;
  *(uint64_t*)0x20000650 = 0;
  *(uint64_t*)0x20000658 = 0;
  *(uint64_t*)0x20000660 = 0x50fe40;
  *(uint64_t*)0x20000668 = 0;
  *(uint64_t*)0x20000670 = 0;
  *(uint64_t*)0x20000678 = 0x50fee0;
  *(uint64_t*)0x20000680 = 0;
  *(uint64_t*)0x20000688 = 0;
  *(uint64_t*)0x20000690 = 0x510000;
  *(uint64_t*)0x20000698 = 0;
  *(uint64_t*)0x200006a0 = 0;
  *(uint64_t*)0x200006a8 = 0x510f40;
  *(uint64_t*)0x200006b0 = 0;
  *(uint64_t*)0x200006b8 = 0;
  *(uint64_t*)0x200006c0 = 0x690000;
  *(uint64_t*)0x200006c8 = 0;
  *(uint64_t*)0x200006d0 = 0;
  *(uint64_t*)0x200006d8 = 0x690480;
  syz_mount_image(0, 0, 0x1000000, 0x34, 0x20000200, 0, 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
