// https://syzkaller.appspot.com/bug?id=dd526205a598fb0f936c2d29e676c8c3e435adb1
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
      if (current_time_ms() - start < 5 * 1000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

void execute_one(void)
{
  memcpy((void*)0x20000000, "btrfs\000", 6);
  memcpy((void*)0x20000100, "./file0\000", 8);
  *(uint64_t*)0x20000200 = 0x20010000;
  memcpy((void*)0x20010000, "\x23\x08\xd3\x8f\x29\x5d\xfe\x36\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\xb6\xc6\xde\x41"
                            "\x0f\x3d\x4d\x56\xa2\x85\x15\x37\x70\x4b\xe2\x59"
                            "\x00\x00\x01\x00\x00\x00\x00\x00\x01\x00\x00\x00"
                            "\x00\x00\x00\x00\x5f\x42\x48\x52\x66\x53\x5f\x4d",
         72);
  *(uint64_t*)0x20000208 = 0x48;
  *(uint64_t*)0x20000210 = 0x10000;
  *(uint64_t*)0x20000218 = 0;
  *(uint64_t*)0x20000220 = 0;
  *(uint64_t*)0x20000228 = 0;
  *(uint64_t*)0x20000230 = 0;
  *(uint64_t*)0x20000238 = 0;
  *(uint64_t*)0x20000240 = 0;
  *(uint8_t*)0x20016800 = 0;
  syz_mount_image(0x20000000, 0x20000100, 0, 3, 0x20000200, 0, 0x20016800);
  memcpy((void*)0x20000000, "btrfs\000", 6);
  memcpy((void*)0x20000100, "./file0\000", 8);
  *(uint64_t*)0x20000200 = 0x20010000;
  memcpy((void*)0x20010000,
         "\x23\x08\xd3\x8f\x29\x5d\xfe\x36\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xb6\xc6"
         "\xde\x41\x0f\x3d\x4d\x56\xa2\x85\x15\x37\x70\x4b\xe2\x59\x00\x00\x01"
         "\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x5f\x42\x48\x52"
         "\x66\x53\x5f\x4d\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x50\x00\x00"
         "\x00\x00\x00\x00\x10\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00"
         "\x00\x00\xd0\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00"
         "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00\x00"
         "\x10\x00\x00\x00\x10\x00\x00\x61\x00\x00\x00\x04\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\xc5\x01\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x01\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x72\x00"
         "\x00\x00\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x61\x2c\xcd\x42\xc9"
         "\xb9\x45\x2f\xac\x27\xd4\x54\x7f\x0c\xc6\xdf\xb6\xc6\xde\x41\x0f\x3d"
         "\x4d\x56\xa2\x85\x15\x37\x70\x4b\xe2\x59",
         299);
  *(uint64_t*)0x20000208 = 0x12b;
  *(uint64_t*)0x20000210 = 0x10000;
  *(uint64_t*)0x20000218 = 0x20010200;
  memcpy((void*)0x20010200, "\000\000\000\000\000\000\000\000\000\000\000\a\000"
                            "\000\000\000\000\000\000\a",
         20);
  *(uint64_t*)0x20000220 = 0x14;
  *(uint64_t*)0x20000228 = 0x10220;
  *(uint64_t*)0x20000230 = 0x20010300;
  memcpy((void*)0x20010300,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00"
         "\x00\x00\xe4\x00\x00\x10\x00\x00\x00\x00\x00\x00\x00\x40\x00\x00\x00"
         "\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00"
         "\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00"
         "\x00\x10\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x10\x00\x00\x00\x00\x00\x61\x2c\xcd\x42\xc9\xb9\x45\x2f\xac\x27"
         "\xd4\x54\x7f\x0c\xc6\xdf",
         108);
  *(uint64_t*)0x20000238 = 0x6c;
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
      "\x00\x00\x01\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x01",
      628);
  *(uint64_t*)0x20000250 = 0x274;
  *(uint64_t*)0x20000258 = 0x10b20;
  *(uint64_t*)0x20000260 = 0x20010800;
  memcpy(
      (void*)0x20010800,
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x19\x00"
      "\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00"
      "\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x01\x00"
      "\x00\x10\x00\x00\x01\x00\x01\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x69\x00\x00\x00\x00\x00\x61\x2c\xcd\x42\xc9\xb9\x45\x2f\xac\x27\xd4\x54"
      "\x7f\x0c\xc6\xdf\x00\x00\x19\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x01\x00\x00\x00\x01\x00\x00\x10\x00\x00\x01\x00\x01\x00\x01\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x50\x00\x00\x00\x00\x00\x61\x2c\xcd\x42"
      "\xc9\xb9\x45\x2f\xac\x27\xd4\x54\x7f\x0c\xc6\xdf\x00\x00\x40\x00\x00\x00"
      "\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00"
      "\x02\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00\x00\x10"
      "\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00"
      "\x00\x00\x00\x00\x61\x2c\xcd\x42\xc9\xb9\x45\x2f\xac\x27\xd4\x54\x7f\x0c"
      "\xc6\xdf\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00"
      "\x00\x00\x72\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00\x00\x10"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x61\x2c\xcd\x42"
      "\xc9\xb9\x45\x2f\xac\x27\xd4\x54\x7f\x0c\xc6\xdf\xb6\xc6\xde\x41\x0f\x3d"
      "\x4d\x56\xa2\x85\x15\x37\x70\x4b\xe2\x59\xe5\xa7\xe3\xc4\xf4\xe7\x39\x76"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\xb6\xc6\xde\x41\x0f\x3d\x4d\x56\xa2\x85\x15\x37"
      "\x70\x4b\xe2\x59\x00\x10\x10\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00"
      "\x00\x01\x23\x2e\x9e\xf5\x2b\xc2\x4c\x8b\x9c\xcb\xe8\xb4\x2d\xd5\x41\x1a"
      "\x04\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x04\x00"
      "\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\xd8\x01\x00\x00\x00\x00\x00"
      "\x00\x00\x39\x0f\x00\x00\x62\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00"
      "\xe4\x00\x00\x10\x00\x00\x00\x00\x00\xe9\x0e\x00\x00\x50\x00\x00\x00\x00"
      "\x01\x00\x00\x00\x00\x00\x00\xe4\x00\x00\x50\x00\x00\x00\x00\x00\x99\x0e"
      "\x00\x00\x50\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\xe4\x00\x00\x69"
      "\x00\x00\x00\x00\x00\x49\x0e\x00\x00\x50",
      550);
  *(uint64_t*)0x20000268 = 0x226;
  *(uint64_t*)0x20000270 = 0x100ea0;
  *(uint64_t*)0x20000278 = 0x20010b00;
  memcpy((void*)0x20010b00,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x19"
         "\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00"
         "\x00\x00\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00"
         "\x00\x01\x00\x00\x10\x00\x00\x01\x00\x01\x00\x01\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x69\x00\x00\x00\x00\x00\x61\x2c\xcd\x42\xc9\xb9\x45"
         "\x2f\xac\x27\xd4\x54\x7f\x0c\xc6\xdf\x00\x00\x19\x00\x00\x00\x00\x00"
         "\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x05"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x10"
         "\x00\x00\x01\x00\x01\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x50"
         "\x00\x00\x00\x00\x00\x61\x2c\xcd\x42\xc9\xb9\x45\x2f\xac\x27\xd4\x54"
         "\x7f\x0c\xc6\xdf\x00\x00\x40\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00\x01\x00\x00"
         "\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00"
         "\x61\x2c\xcd\x42\xc9\xb9\x45\x2f\xac\x27\xd4\x54\x7f\x0c\xc6\xdf\x01"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00"
         "\x72\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00\x00\x10\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x61\x2c\xcd"
         "\x42\xc9\xb9\x45\x2f\xac\x27\xd4\x54\x7f\x0c\xc6\xdf\xb6\xc6\xde\x41"
         "\x0f\x3d\x4d\x56\xa2\x85\x15\x37\x70\x4b\xe2\x59\x5d",
         353);
  *(uint64_t*)0x20000280 = 0x161;
  *(uint64_t*)0x20000288 = 0x101ea0;
  *(uint64_t*)0x20000290 = 0;
  *(uint64_t*)0x20000298 = 0;
  *(uint64_t*)0x200002a0 = 0;
  *(uint64_t*)0x200002a8 = 0;
  *(uint64_t*)0x200002b0 = 0;
  *(uint64_t*)0x200002b8 = 0;
  *(uint64_t*)0x200002c0 = 0;
  *(uint64_t*)0x200002c8 = 0;
  *(uint64_t*)0x200002d0 = 0;
  *(uint64_t*)0x200002d8 = 0;
  *(uint64_t*)0x200002e0 = 0;
  *(uint64_t*)0x200002e8 = 0;
  *(uint64_t*)0x200002f0 = 0x20011800;
  memcpy((void*)0x20011800,
         "\xab\x3b\xb3\x94\x7f\x88\x0b\x8a\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xb6\xc6"
         "\xde\x41\x0f\x3d\x4d\x56\xa2\x85\x15\x37\x70\x4b\xe2\x59\x00\x00\x50"
         "\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x01\x23\x2e\x9e\xf5"
         "\x2b\xc2\x4c\x8b\x9c\xcb\xe8\xb4\x2d\xd5\x41\x1a\x07\x00\x00\x00\x00"
         "\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x0a\x00\x00\x00\x00\x02"
         "\x00\x00\x00\x00\x00\x00\x00\x84\x00\x00\x00\x00\x00\x00\x00\x00\xe4"
         "\x0d\x00\x00\xb7\x01\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x84\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x2d\x0c\x00\x00\xb7\x01\x00\x00\x05\x00"
         "\x00\x00\x00\x00\x00\x00\x0c\x06\x00\x00\x00\x00\x00\x00\x00\x1c\x0c"
         "\x00\x00\x11\x00\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00\x84\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x65\x0a\x00\x00\xb7\x01\x00\x00\x06\x00\x00"
         "\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\xc5\x09\x00"
         "\x00\xa0\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00\x0c\x06\x00\x00"
         "\x00\x00\x00\x00\x00\xb9\x09\x00\x00\x0c\x00\x00\x00\x06\x00\x00\x00"
         "\x00\x00\x00\x00\x54\xd2\xc2\xbf\x8d\x00\x00\x00\x00\x94\x09\x00\x00"
         "\x25\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x84\x00\x00\x00\x00"
         "\x00\x00\x00\x00\xdd\x07\x00\x00\xb7\x01\x00\x00\x09\x00\x00\x00\x00"
         "\x00\x00\x00\x84\x00\x00\x00\x00\x00\x00\x00\x00\x26\x06\x00\x00\xb7"
         "\x01\x00\x00\xf7\xff\xff\xff\xff\xff\xff\xff\x84\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x6f\x04\x00\x00\xb7\x01\x00",
         350);
  *(uint64_t*)0x200002f8 = 0x15e;
  *(uint64_t*)0x20000300 = 0x500000;
  *(uint64_t*)0x20000308 = 0x20011a00;
  memcpy((void*)0x20011a00,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00"
         "\x00\x00\x00\x90\x50\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00"
         "\x00\x00\x00\x00",
         106);
  *(uint64_t*)0x20000310 = 0x6a;
  *(uint64_t*)0x20000318 = 0x500560;
  *(uint64_t*)0x20000320 = 0x20011b00;
  memcpy((void*)0x20011b00,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xa0\x50\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x04\x00\x00",
         93);
  *(uint64_t*)0x20000328 = 0x5d;
  *(uint64_t*)0x20000330 = 0x500720;
  *(uint64_t*)0x20000338 = 0x20011c00;
  memcpy((void*)0x20011c00, "\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\xed\x41",
         56);
  *(uint64_t*)0x20000340 = 0x38;
  *(uint64_t*)0x20000348 = 0x500840;
  *(uint64_t*)0x20000350 = 0x20011d00;
  memcpy((void*)0x20011d00,
         "\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x60\x50\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00",
         84);
  *(uint64_t*)0x20000358 = 0x54;
  *(uint64_t*)0x20000360 = 0x5008e0;
  *(uint64_t*)0x20000368 = 0x20011e00;
  memcpy((void*)0x20011e00,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00\x84"
         "\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x07\x00\x02\x64\x65\x66\x61\x75\x6c\x74\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x02\x00\x2e\x2e\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\xed\x41\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x59"
         "\xbc\x64\x5f\x00\x00\x00\x00\x00\x00\x00\x00\x59\xbc\x64\x5f\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x59\xbc\x64\x5f\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x59\xbc\x64\x5f\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xed\x41\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\xff\xff\xff\xff"
         "\x00\x00\x00",
         309);
  *(uint64_t*)0x20000370 = 0x135;
  *(uint64_t*)0x20000378 = 0x5009e0;
  *(uint64_t*)0x20000380 = 0x20012000;
  memcpy((void*)0x20012000,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\xd0\x50\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x30\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x01\x06\x00\x00\x00\x00\x00\x00\x00\x1d\xb2\xf5\x87\xaa"
         "\xf5\x4e\x74\x8c\xf5\xc3\xa8\x7a\x5d\xfb\x20\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x5a\xbc\x64\x5f\x00\x00\x00\x00\xdf\xea"
         "\xca\x06\x59\xbc\x64\x5f\x00\x00",
         195);
  *(uint64_t*)0x20000388 = 0xc3;
  *(uint64_t*)0x20000390 = 0x500b60;
  *(uint64_t*)0x20000398 = 0x20012100;
  memcpy((void*)0x20012100, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x07\x00\x64"
                            "\x65\x66\x61\x75\x6c\x74\x01\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xed\x41",
         72);
  *(uint64_t*)0x200003a0 = 0x48;
  *(uint64_t*)0x200003a8 = 0x500c80;
  *(uint64_t*)0x200003b0 = 0x20012200;
  memcpy((void*)0x20012200,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x06\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x51\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00",
         101);
  *(uint64_t*)0x200003b8 = 0x65;
  *(uint64_t*)0x200003c0 = 0x500d20;
  *(uint64_t*)0x200003c8 = 0x20012300;
  memcpy((void*)0x20012300,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xed\x41\x00",
         64);
  *(uint64_t*)0x200003d0 = 0x40;
  *(uint64_t*)0x200003d8 = 0x500e40;
  *(uint64_t*)0x200003e0 = 0x20012400;
  memcpy((void*)0x20012400,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10\x50\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x07\x00",
         90);
  *(uint64_t*)0x200003e8 = 0x5a;
  *(uint64_t*)0x200003f0 = 0x500ee0;
  *(uint64_t*)0x200003f8 = 0x20012500;
  memcpy(
      (void*)0x20012500,
      "\x0d\x04\xc4\xb1\xe4\x16\xba\x62\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xb6\xc6\xde\x41"
      "\x0f\x3d\x4d\x56\xa2\x85\x15\x37\x70\x4b\xe2\x59\x00\x10\x50\x00\x00\x00"
      "\x00\x00\x01\x00\x00\x00\x00\x00\x00\x01\x23\x2e\x9e\xf5\x2b\xc2\x4c\x8b"
      "\x9c\xcb\xe8\xb4\x2d\xd5\x41\x1a\x07\x00\x00\x00\x00\x00\x00\x00\x02\x00"
      "\x00\x00\x00\x00\x00\x00\x0e\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00"
      "\x00\xc0\x00\x00\x40\x00\x00\x00\x00\x00\x83\x0f\x00\x00\x18\x00\x00\x00"
      "\x00\x10\x10\x00\x00\x00\x00\x00\xa9\x00\x00\x00\x00\x00\x00\x00\x00\x62"
      "\x0f\x00\x00\x21\x00\x00\x00\x00\x00\x50\x00\x00\x00\x00\x00\xa9\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x41\x0f\x00\x00\x21\x00\x00\x00\x00\x00\x50\x00"
      "\x00\x00\x00\x00\xc0\x00\x00\x19\x00\x00\x00\x00\x00\x29\x0f\x00\x00\x18"
      "\x00\x00\x00\x00\x10\x50\x00\x00\x00\x00\x00\xa9\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x08\x0f\x00\x00\x21\x00\x00\x00\x00\x30\x50\x00\x00\x00\x00\x00"
      "\xa8\x00\x30\x00\x00\x00\x00\x00\x00\xd3\x0e\x00\x00\x35\x00\x00\x00\x00"
      "\x60\x50\x00\x00\x00\x00\x00\xa9\x00\x00\x00\x00\x00\x00\x00\x00\xb2\x0e"
      "\x00\x00\x21\x00\x00\x00\x00\x90\x50\x00\x00\x00\x00\x00\xa9\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x91\x0e\x00\x00\x21\x00\x00\x00\x00\xa0\x50\x00\x00"
      "\x00\x00\x00\xa9\x00\x00\x00\x00\x00\x00\x00\x00\x70\x0e\x00\x00\x21\x00"
      "\x00\x00\x00\xc0\x50\x00\x00\x00\x00\x00\xa9\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x4f\x0e\x00\x00\x21\x00\x00\x00\x00\xd0\x50\x00\x00\x00\x00\x00\xa9"
      "\x01\x00\x00\x00\x00\x00\x00\x00\x2e\x0e\x00\x00\x21\x00\x00\x00\x00\xe0"
      "\x50\x00\x00\x00\x00\x00\xa9\x00\x00\x00\x00\x00\x00\x00\x00\x0d\x0e\x00"
      "\x00\x21\x00\x00\x00\x00\x00\x51\x00\x00\x00\x00\x00\xa9\x00\x00\x00\x00"
      "\x00\x00\x00\x00\xec\x0d\x00\x00\x21\x00\x00\x00\x00\x00\x69\x00\x00\x00"
      "\x00\x00\xc0\x00\x00\x19\x00\x00\x00\x00\x00\xd4\x0d\x00\x00\x18",
      448);
  *(uint64_t*)0x20000400 = 0x1c0;
  *(uint64_t*)0x20000408 = 0x501000;
  *(uint64_t*)0x20000410 = 0x20012700;
  memcpy((void*)0x20012700,
         "\x00\x00\x01\x00\x00\x00\x00\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00"
         "\x01\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00\x02"
         "\x00\x00\x00\x00\x00\x00\x00\xb0\x04\x00\x00\x00\x00\x00\x00\x00\x01"
         "\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00\x02\x00"
         "\x00\x00\x00\x00\x00\x00\xb0\x05\x00\x00\x00\x00\x00\x00\x00\x01\x00"
         "\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00"
         "\x00\x00\x00\x00\x00\xb0\x05\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00"
         "\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00"
         "\x00\x00\x00\x00\xb0\x05\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00"
         "\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00"
         "\x00\x00\x00\xb0\x09\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00"
         "\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00"
         "\x00\x00\xb0\xf7\xff\xff\xff\xff\xff\xff\xff\x01\x00\x00\x00\x00\x00"
         "\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00"
         "\x00\xb0\x07\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00"
         "\x00\x06\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"
         "\xb2\x05\x00\x00\x00\x00\x00\x00\x00\x05\x01\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x00"
         "\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00"
         "\x00\x00\xb0\x02\x00\x00\x00\x00\x00\x00\x00\x00\xc0\x00\x00\x00\x00"
         "\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x05\x00\x00\x00\x00\x00\x00"
         "\x00\x01\x00\x00\x00\x00\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00"
         "\x02\x00\x00\x00\x00\x00\x00\x00\xb0\x01\x00\x00\x00\x00\x00\x00\x00"
         "\x01\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x02"
         "\x00\x00\x00\x00\x00\x00\x00\xb0\x03\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x10\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x02",
         441);
  *(uint64_t*)0x20000418 = 0x1b9;
  *(uint64_t*)0x20000420 = 0x501e40;
  *(uint64_t*)0x20000428 = 0;
  *(uint64_t*)0x20000430 = 0;
  *(uint64_t*)0x20000438 = 0;
  *(uint64_t*)0x20000440 = 0;
  *(uint64_t*)0x20000448 = 0;
  *(uint64_t*)0x20000450 = 0x506000;
  *(uint64_t*)0x20000458 = 0;
  *(uint64_t*)0x20000460 = 0;
  *(uint64_t*)0x20000468 = 0x506fe0;
  *(uint64_t*)0x20000470 = 0;
  *(uint64_t*)0x20000478 = 0;
  *(uint64_t*)0x20000480 = 0x509000;
  *(uint64_t*)0x20000488 = 0;
  *(uint64_t*)0x20000490 = 0;
  *(uint64_t*)0x20000498 = 0x509f40;
  *(uint64_t*)0x200004a0 = 0;
  *(uint64_t*)0x200004a8 = 0;
  *(uint64_t*)0x200004b0 = 0x50afe0;
  *(uint64_t*)0x200004b8 = 0;
  *(uint64_t*)0x200004c0 = 0;
  *(uint64_t*)0x200004c8 = 0x50be40;
  *(uint64_t*)0x200004d0 = 0;
  *(uint64_t*)0x200004d8 = 0;
  *(uint64_t*)0x200004e0 = 0x50cca0;
  *(uint64_t*)0x200004e8 = 0;
  *(uint64_t*)0x200004f0 = 0;
  *(uint64_t*)0x200004f8 = 0x50e000;
  *(uint64_t*)0x20000500 = 0;
  *(uint64_t*)0x20000508 = 0;
  *(uint64_t*)0x20000510 = 0x50e6e0;
  *(uint64_t*)0x20000518 = 0;
  *(uint64_t*)0x20000520 = 0;
  *(uint64_t*)0x20000528 = 0;
  *(uint64_t*)0x20000530 = 0x20015600;
  memcpy((void*)0x20015600,
         "\xb6\xd2\xc9\xc7\x3d\xbd\xed\xa7\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xb6\xc6"
         "\xde\x41\x0f\x3d\x4d\x56\xa2\x85\x15\x37\x70\x4b\xe2\x59\x00\x00\x51"
         "\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x01\x23\x2e\x9e\xf5"
         "\x2b\xc2\x4c\x8b\x9c\xcb\xe8\xb4\x2d\xd5\x41\x1a\x06\x00\x00\x00\x00"
         "\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\xf9\x01\x00\x00\x00\x00\x00\x00\x00\x73"
         "\x0f\x00\x00\x28\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\xcc\x00"
         "\x00\x10\x00\x00\x00\x00\x00\x43\x0f\x00\x00\x30\x00\x00\x00\x01\x00"
         "\x00\x00\x00\x00\x00\x00\xcc\x00\x00\x50\x00\x00\x00\x00\x00\x13\x0f"
         "\x00\x00\x30\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\xcc\x00\x00"
         "\x69\x00\x00\x00\x00\x00\xe3\x0e\x00\x00\x30",
         198);
  *(uint64_t*)0x20000538 = 0xc6;
  *(uint64_t*)0x20000540 = 0x510000;
  *(uint64_t*)0x20000548 = 0x20015700;
  memcpy((void*)0x20015700,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x69\x00\x00\x00\x00\x00\x00\x00"
         "\x19\x00\x00\x00\x00\x00\x23\x2e\x9e\xf5\x2b\xc2\x4c\x8b\x9c\xcb\xe8"
         "\xb4\x2d\xd5\x41\x1a\x03\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x50\x00\x00\x00\x00\x00\x00\x00\x19\x00\x00"
         "\x00\x00\x00\x23\x2e\x9e\xf5\x2b\xc2\x4c\x8b\x9c\xcb\xe8\xb4\x2d\xd5"
         "\x41\x1a\x03\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x00\x40\x00\x00\x00\x00\x00"
         "\x23\x2e\x9e\xf5\x2b\xc2\x4c\x8b\x9c\xcb\xe8\xb4\x2d\xd5\x41\x1a",
         152);
  *(uint64_t*)0x20000550 = 0x98;
  *(uint64_t*)0x20000558 = 0x510f40;
  *(uint8_t*)0x20016800 = 0;
  syz_mount_image(0x20000000, 0x20000100, 0, 0x24, 0x20000200, 0, 0x20016800);
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
