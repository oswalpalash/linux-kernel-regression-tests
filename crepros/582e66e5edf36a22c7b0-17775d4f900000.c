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
  memcpy((void*)0x20010000,
         "\xed\x79\x7e\x4f\x85\x52\x62\x73\x83\xe3\xe8\xbd\x94\xf9\x32\x66\xa9"
         "\x86\x59\x26\xfc\x25\xed\xc5\xcf\x02\x79\xbb\xc6\xda\x70\x97\x8a\xe5"
         "\xb4\x01\x4a\xd8\x41\x68\x86\x72\x01\xe7\xdb\x0b\x90\xb5\x00\x00\x01"
         "\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x5f\x42\x48\x52"
         "\x66\x53\x5f\x4d\x05\x00\x00\x00\x00\x00\x00\x00\x00\x90\xd0\x01\x00"
         "\x00\x00\x00\x00\x10\x50\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00"
         "\x00\x00\x80\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00"
         "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00\x00"
         "\x10\x00\x00\x00\x10\x00\x00\x81\x00\x00\x00\x05\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x45\x01\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x01\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00\x00\x00\x00\x80\x05"
         "\x00\x00\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xfc\xb5\x93\x27\xa7"
         "\xf7\x49\x02\xbb\x9e\xeb\x47\xff\x81\x4c\x50\x8a\xe5\xb4\x01\x4a\xd8"
         "\x41\x68\x86\x72\x01\xe7\xdb\x0b\x90\xb5",
         299);
  *(uint64_t*)0x20000208 = 0x12b;
  *(uint64_t*)0x20000210 = 0x10000;
  *(uint64_t*)0x20000218 = 0x20010200;
  memcpy((void*)0x20010200, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff"
                            "\xff\xff\xff\xff\xff\xff\xff",
         19);
  *(uint64_t*)0x20000220 = 0x13;
  *(uint64_t*)0x20000228 = 0x10220;
  *(uint64_t*)0x20000230 = 0x20010300;
  memcpy((void*)0x20010300,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00"
         "\x00\x00\xe4\x00\x00\x50\x01\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00"
         "\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00"
         "\x00\x22\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x01\x00"
         "\x00\x10\x00\x00\x02\x00\x01\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x50\x01\x00\x00\x00\x00\xfc\xb5\x93\x27\xa7\xf7\x49\x02\xbb\x9e"
         "\xeb\x47\xff\x81\x4c\x50\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\xd0"
         "\x01\x00\x00\x00\x00\xfc\xb5\x93\x27\xa7\xf7\x49\x02\xbb\x9e\xeb\x47"
         "\xff\x81\x4c\x50\x00\x00\x00\x01\x00\x00\x00\x00\x00\x22\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x10\x00\x00\x02"
         "\x00\x01\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x50\x01\x00\x00"
         "\x00\x00\xfc\xb5\x93\x27\xa7\xf7\x49\x02\xbb\x9e\xeb\x47\xff\x81\x4c"
         "\x50\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\xd0\x01\x00\x00\x00\x00"
         "\xfc\xb5\x93\x27\xa7\xf7\x49\x02\xbb\x9e\xeb\x47\xff\x81\x4c\x50",
         237);
  *(uint64_t*)0x20000238 = 0xed;
  *(uint64_t*)0x20000240 = 0x10320;
  *(uint64_t*)0x20000248 = 0x20010400;
  memcpy(
      (void*)0x20010400,
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x90\xd0\x01\x00\x00\x00"
      "\x00\x05\x00\x00\x00\x00\x00\x00\x00\x00\x10\x50\x01\x00\x00\x00\x00\x05"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x70\xd0\x01\x00\x00\x00\x00\x05\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\xd0\x01\x00\x00\x00\x00\x04\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x80\xd0\x01\x00\x00\x00\x00\x05\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x40\xd0\x01\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x08\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x01\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x50\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x80\x10"
      "\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x60\x10\x00\x00"
      "\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x40\x10\x00\x00\x00\x00"
      "\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x70\x10\x00\x00\x00\x00\x00\x02"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x50\x10\x00\x00\x00\x00\x00\x01\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00\x00\x00\x60\x00\x00\x00"
      "\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x30\x50\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x20\x50\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x40\x50\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00\x10\x50"
      "\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00\x50\x10\x00\x00"
      "\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00"
      "\x00\x00\x60\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10\xd0\x01\x00\x00\x00"
      "\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x50\x01\x00\x00\x00\x00\x04"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x20\xd0\x01\x00\x00\x00\x00\x04\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\xd0\x01\x00\x00\x00\x00\x04\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x30\xd0\x01\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x40\xd0\x01\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x08\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x01\x00",
      629);
  *(uint64_t*)0x20000250 = 0x275;
  *(uint64_t*)0x20000258 = 0x10b20;
  *(uint64_t*)0x20000260 = 0x20013600;
  memcpy(
      (void*)0x20013600,
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02"
      "\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00"
      "\x00\x00\x24\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x01\x00"
      "\x00\x10\x00\x00\x02\x00\x01\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x50\x02\x00\x00\x00\x00\xfc\xb5\x93\x27\xa7\xf7\x49\x02\xbb\x9e\xeb\x47"
      "\xff\x81\x4c\x50\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x50\x04\x00\x00"
      "\x00\x00\xfc\xb5\x93\x27\xa7\xf7\x49\x02\xbb\x9e\xeb\x47\xff\x81\x4c\x50"
      "\x00\x00\x80\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x01\x00\x00\x00\x00\x00\x22\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00"
      "\x00\x00\x01\x00\x00\x10\x00\x00\x02\x00\x01\x00\x01\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x50\x01\x00\x00\x00\x00\xfc\xb5\x93\x27\xa7\xf7\x49\x02"
      "\xbb\x9e\xeb\x47\xff\x81\x4c\x50\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\xd0\x01\x00\x00\x00\x00\xfc\xb5\x93\x27\xa7\xf7\x49\x02\xbb\x9e\xeb\x47"
      "\xff\x81\x4c\x50\x00\x00\x80\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x01\x00\x00\x00\x01\x00\x00\x10\x00\x00\x01\x00\x01\x00\x01\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\xd0\x00\x00\x00\x00\x00\xfc\xb5\x93\x27"
      "\xa7\xf7\x49\x02\xbb\x9e\xeb\x47\xff\x81\x4c\x50\x00\x00\x80\x00\x00\x00"
      "\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00"
      "\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x10"
      "\x00\x00\x01\x00\x01\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x50\x00"
      "\x00\x00\x00\x00\xfc\xb5\x93\x27\xa7\xf7\x49\x02\xbb\x9e\xeb\x47\xff\x81"
      "\x4c\x50\x00\x00\x40\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x01\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x10"
      "\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\xfc\xb5\x93\x27\xa7\xf7"
      "\x49\x02\xbb\x9e\xeb\x47\xff\x81\x4c\x50\x01\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x08\x00\x00\x00\x00\x00\x00\x40\x06\x00\x00\x00\x00\x00\x10"
      "\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\xfc\xb5\x93\x27\xa7\xf7\x49\x02\xbb\x9e\xeb\x47\xff\x81"
      "\x4c\x50\x8a\xe5\xb4\x01\x4a\xd8\x41\x68\x86\x72\x01\xe7\xdb\x0b\x90\xb5"
      "\xe4\xdd\xd8\xa1\xd9\x17\x9b\xd5\x4f\x8e\xb1\x60\x12\x8d\x1e\x1c\x12\x98"
      "\xf4\xad\xad\xd4\xc3\x33\x6b\x60\xe9\x61\x9d\xc8\xea\x60\x8a\xe5\xb4\x01"
      "\x4a\xd8\x41\x68\x86\x72\x01\xe7\xdb\x0b\x90\xb5\x00\x10\x50\x01\x00\x00"
      "\x00\x00\x01\x00\x00\x00\x00\x00\x00\x01\x01\xa2\xac\xa4\x4e\x54\x4e\x5a"
      "\xa6\xdc\x88\x7c\xe6\x9f\xb9\xd0\x05\x00\x00\x00\x00\x00\x00\x00\x03\x00"
      "\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00"
      "\x00\xd8\x01\x00\x00\x00\x00\x00\x00\x00\x39\x0f\x00\x00\x62\x00\x00\x00"
      "\x00\x01\x00\x00\x00\x00\x00\x00\xe4\x00\x00\xd0\x00\x00\x00\x00\x00\xe9"
      "\x0e\x00\x00\x50\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\xe4\x00\x00"
      "\x50\x01\x00\x00\x00\x00\x79\x0e\x00\x00\x70\x00\x00\x00\x00\x01\x00\x00"
      "\x00\x00\x00\x00\xe4\x00\x00\xd0\x01\x00\x00\x00\x00\x09\x0e\x00\x00\x70"
      "\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\xe4\x00\x00\xd0\x01\x00\x00"
      "\x00\x00\x09\x0e\x00\x00\x70\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00"
      "\xe4\x00\x00\xd0\x01\x00\x00\x00\x00\xb9\x0d\x00\x00\x70",
      824);
  *(uint64_t*)0x20000268 = 0x338;
  *(uint64_t*)0x20000270 = 0x1500dc0;
  *(uint64_t*)0x20000278 = 0x20013a00;
  memcpy((void*)0x20013a00,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x02\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00"
         "\x00\x00\x00\x00\x24\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00"
         "\x00\x01\x00\x00\x10\x00\x00\x02\x00\x01\x00\x01\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x50\x02\x00\x00\x00\x00\xfc\xb5\x93\x27\xa7\xf7\x49"
         "\x02\xbb\x9e\xeb\x47\xff\x81\x4c\x50\x00\x00\x00\x02\x00\x00\x00\x00"
         "\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x24"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x10"
         "\x00\x00\x02\x00\x01\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x50"
         "\x02\x00\x00\x00\x00\xfc\xb5\x93\x27\xa7\xf7\x49\x02\xbb\x9e\xeb\x47"
         "\xff\x81\x4c\x50\x00\x00\x00\x02\x00\x00\x00\x00\x02\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x24\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x10\x00\x00\x02\x00\x01"
         "\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x50\x02\x00\x00\x00\x00"
         "\xfc\xb5\x93\x27\xa7\xf7\x49\x02\xbb\x9e\xeb\x47\xff\x81\x4c\x50\x01"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x50\x04\x00\x00\x00\x00\xfc\xb5"
         "\x93\x27\xa7\xf7\x49\x02\xbb\x9e\xeb\x47\xff\x81\x4c\x50\x00\x00\x80"
         "\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00"
         "\x00\x00\x00\x00\x22\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00"
         "\x00\x01\x00\x00\x10\x00\x00\x02\x00\x01\x00\x01\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x50\x01\x00\x00\x00\x00\xfc\xb5\x93\x27\xa7\xf7\x49"
         "\x02\xbb\x9e\xeb\x47\xff\x81\x4c\x50\x01\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\xd0\x01\x00\x00\x00\x00\xfc\xb5\x93\x27\xa7\xf7\x49\x02\xbb"
         "\x9e\xeb\x47\xff\x81\x4c\x50\x00\x00\x80\x00\x00\x00\x00\x00\x02\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x01\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x10\x00\x00"
         "\x01\x00\x01\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\xd0\x00\x00"
         "\x00\x00\x00\xfc\xb5\x93\x27\xa7\xf7\x49\x02\xbb\x9e\xeb\x47\xff\x81"
         "\x4c\x50\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00"
         "\x00\x00\x00\x80\x05\x00\x00\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00"
         "\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\xfc\xb5\x93\x27\xa7\xf7\x49\x02\xbb\x9e\xeb\x47\xff\x81\x4c\x50\x8a"
         "\xe5\xb4\x01\x4a\xd8\x41\x68\x86\x72\x01\xe7\xdb\x0b\x90\xb5",
         576);
  *(uint64_t*)0x20000280 = 0x240;
  *(uint64_t*)0x20000288 = 0x1501dc0;
  *(uint64_t*)0x20000290 = 0;
  *(uint64_t*)0x20000298 = 0;
  *(uint64_t*)0x200002a0 = 0x2500000;
  *(uint64_t*)0x200002a8 = 0;
  *(uint64_t*)0x200002b0 = 0;
  *(uint64_t*)0x200002b8 = 0x2500f40;
  *(uint64_t*)0x200002c0 = 0;
  *(uint64_t*)0x200002c8 = 0;
  *(uint64_t*)0x200002d0 = 0x2503ea0;
  *(uint64_t*)0x200002d8 = 0;
  *(uint64_t*)0x200002e0 = 0;
  *(uint64_t*)0x200002e8 = 0x2505f40;
  *(uint64_t*)0x200002f0 = 0x20015f00;
  memcpy(
      (void*)0x20015f00,
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00\xeb\xca\x00\x7d"
      "\x01\xbc\xba\x47\x8d\xb6\x28\x70\xdc\xb2\xc5\x47\x44\xe6\x8a\xf1\xdf\x22"
      "\x8b\x7d\x4b\xd1\xae\x3e\x07\xf3\xac\xf8\x8a\xe5\xb4\x01\x4a\xd8\x41\x68"
      "\x86\x72\x01\xe7\xdb\x0b\x90\xb5\x00\x70\xd0\x01\x00\x00\x00\x00\x01\x00"
      "\x00\x00\x00\x00\x00\x01\x01\xa2\xac\xa4\x4e\x54\x4e\x5a\xa6\xdc\x88\x7c"
      "\xe6\x9f\xb9\xd0\x05\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00"
      "\x00\x00\x0b\x00\x00\x00\x00\x00\x00\xd0\x00\x00\x00\x00\x00\xc0\x00\x00"
      "\x80\x00\x00\x00\x00\x00\x83\x0f\x00\x00\x18\x00\x00\x00\x00\x00\x50\x01"
      "\x00\x00\x00\x00\xc0\x00\x00\x80\x00\x00\x00\x00\x00\x6b\x0f\x00\x00\x18"
      "\x00\x00\x00\x00\x10\x50\x01\x00\x00\x00\x00\xa9\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x4a\x0f\x00\x00\x21\x00\x00\x00\x00\x00\xd0\x01\x00\x00\x00\x00"
      "\xa9\x00\x00\x00\x00\x00\x00\x00\x00\x29\x0f\x00\x00\x21\x00\x00\x00\x00"
      "\x00\xd0\x01\x00\x00\x00\x00\xc0\x00\x00\x00\x02\x00\x00\x00\x00\x11\x0f"
      "\x00\x00\x18\x00\x00\x00\x00\x40\xd0\x01\x00\x00\x00\x00\xa9\x00\x00\x00"
      "\x00\x00\x00\x00\x00\xf0\x0e\x00\x00\x21\x00\x00\x00\x00\x50\xd0\x01\x00"
      "\x00\x00\x00\xa9\x00\x00\x00\x00\x00\x00\x00\x00\xcf\x0e\x00\x00\x21\x00"
      "\x00\x00\x00\x60\xd0\x01\x00\x00\x00\x00\xa9\x00\x00\x00\x00\x00\x00\x00"
      "\x00\xae\x0e\x00\x00\x21\x00\x00\x00\x00\x70\xd0\x01\x00\x00\x00\x00\xa9"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x8d\x0e\x00\x00\x21\x00\x00\x00\x00\x80"
      "\xd0\x01\x00\x00\x00\x00\xa9\x00\x00\x00\x00\x00\x00\x00\x00\x6c\x0e\x00"
      "\x00\x21\x00\x00\x00\x00\x90\xd0\x01\x00\x00\x00\x00\xa9\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x4b\x0e\x00\x00\x21\x00\x00\x00\x00\x60\xd0\x01\x00\x00"
      "\x00\x00\xa9\x00\x00\x00\x00\x00\x00\x00\x00\x4b\x0e\x00\x00\x21\x00\x00"
      "\x00\x00\x60\xd0\x01\x00\x00\x00\x00\xa9\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x33\x0e\x00\x00\x21\x00\x00\x00\x00\x00\x50\x00\x00\x00\x00\x00\xc0\x00"
      "\x00\x80\x00\x00\x00\x00\x00\xf3\x0e\x00\x00\x18",
      480);
  *(uint64_t*)0x200002f8 = 0x1e0;
  *(uint64_t*)0x20000300 = 0x2506fe0;
  *(uint64_t*)0x20000308 = 0x20016100;
  memcpy(
      (void*)0x20016100,
      "\x01\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x02\x00"
      "\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00"
      "\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00"
      "\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00"
      "\xb0\x01\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x05"
      "\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\xb0\x04\x00"
      "\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x05\x00\x00\x00"
      "\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\xb0\x02\x00\x00\x00\x00"
      "\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00"
      "\x00\x02\x00\x00\x00\x00\x00\x00\x00\xb0\x09\x00\x00\x00\x00\x00\x00\x00"
      "\x01\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x02\x00"
      "\x00\x00\x00\x00\x00\x00\xb0\xf7\xff\xff\xff\xff\xff\xff\xff\x01\x00\x00"
      "\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00"
      "\x00\x00\x00\xb0\x07\x00\x00\x00\x00\x00\x00\x00\x00\x70\x00\x00\x00\x00"
      "\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x24\x00\x00\x00\x00\x00\x00\x00"
      "\x01\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x02\x00"
      "\x00\x00\x00\x00\x00\x00\xb0\x05\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00"
      "\x00\x00\x00\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00"
      "\x00\x00\x00\xb0\x03\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00"
      "\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x22\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x01\x00"
      "\x00\x00\x00\x00\x00\x00\x8f\x27\x94\xf9\xa0\xb4\x13\x50\xde\xb4\x6e\x8a"
      "\x2e\x83\xd8\xce\x1d\x32\x1d\x0b\x9c\xb0\xfc\x57\x60\x3b\xe9\xee\x57\x1c"
      "\x48\x5a\x8a\xe5\xb4\x01\x4a\xd8\x41\x68\x86\x72\x01\xe7\xdb\x0b\x90\xb5"
      "\x00\x80\xd0\x01\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x01\x01\xa2"
      "\xac\xa4\x4e\x54\x4e\x5a\xa6\xdc\x88\x7c\xe6\x9f\xb9\xd0\x05\x00\x00\x00"
      "\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x05\x00\x00\x00\x00\x01"
      "\x00\x00\x00\x00\x00\x00\x00\xcc\x00\x00\xd0\x00\x00\x00\x00\x00\x6b\x0f"
      "\x00\x00\x30\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\xcc\x00\x00\x50"
      "\x01\x00\x00\x00\x00\x3b\x0f\x00\x00\x30\x00\x00\x00\x01\x00\x00\x00\x00"
      "\x00\x00\x00\xcc\x00\x00\xd0\x01\x00\x00\x00\x00\x0b\x0f\x00\x00\x30\x00"
      "\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\xcc\x00\x00\x50\x02\x00\x00\x00"
      "\x00\xdb\x0e\x00\x00\x30\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\xcc"
      "\x00\x00\x50\x04\x00\x00\x00\x00\xab\x0e\x00\x00\x30\x00\x00\x00\x01\x00"
      "\x00\x00\x00\x00\x00\x00\xcc\x00\x00\x50\x04\x00\x00\x00\x00\xab\x0e\x00"
      "\x00\x30\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\xcc\x00\x00\x50\x04"
      "\x00\x00\x00\x00\x7b\x0e\x00\x00\x30",
      657);
  *(uint64_t*)0x20000310 = 0x291;
  *(uint64_t*)0x20000318 = 0x2507e80;
  *(uint64_t*)0x20000320 = 0x20016400;
  memcpy(
      (void*)0x20016400,
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\xd0\x01"
      "\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x01\xa2\xac\xa4\x4e\x54"
      "\x4e\x5a\xa6\xdc\x88\x7c\xe6\x9f\xb9\xd0\x03\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\xd0\x01\x00\x00\x00\x00\x00\x00"
      "\x00\x02\x00\x00\x00\x00\x01\xa2\xac\xa4\x4e\x54\x4e\x5a\xa6\xdc\x88\x7c"
      "\xe6\x9f\xb9\xd0\x03\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00"
      "\x00\x00\x00\x00\xd0\x01\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00"
      "\x01\xa2\xac\xa4\x4e\x54\x4e\x5a\xa6\xdc\x88\x7c\xe6\x9f\xb9\xd0\x03\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\xd0\x01"
      "\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x01\xa2\xac\xa4\x4e\x54"
      "\x4e\x5a\xa6\xdc\x88\x7c\xe6\x9f\xb9\xd0\x03\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x50\x01\x00\x00\x00\x00\x00\x00"
      "\x80\x00\x00\x00\x00\x00\x01\xa2\xac\xa4\x4e\x54\x4e\x5a\xa6\xdc\x88\x7c"
      "\xe6\x9f\xb9\xd0\x03\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x50\x01\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00"
      "\x01\xa2\xac\xa4\x4e\x54\x4e\x5a\xa6\xdc\x88\x7c\xe6\x9f\xb9\xd0\x03\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\xd0\x00"
      "\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x01\xa2\xac\xa4\x4e\x54"
      "\x4e\x5a\xa6\xdc\x88\x7c\xe6\x9f\xb9\xd0\x17\x1a\x20\x4a\x29\x3d\x6e\xfc"
      "\x66\xc5\xe1\x39\x25\xbd\x7d\x86\x2f\x89\x3d\x90\xcb\xdb\x59\x52\x9c\x1c"
      "\x62\x14\xd0\x6b\x4f\x7d\x8a\xe5\xb4\x01\x4a\xd8\x41\x68\x86\x72\x01\xe7"
      "\xdb\x0b\x90\xb5\x00\x90\xd0\x01\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00"
      "\x00\x01\x01\xa2\xac\xa4\x4e\x54\x4e\x5a\xa6\xdc\x88\x7c\xe6\x9f\xb9\xd0"
      "\x05\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x0a\x00"
      "\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x84\x00\x00\x00\x00\x00\x00"
      "\x00\x00\xe4\x0d\x00\x00\xb7\x01\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00"
      "\x84\x00\x00\x00\x00\x00\x00\x00\x00\x2d\x0c\x00\x00\xb7\x01\x00\x00\x05"
      "\x00\x00\x00\x00\x00\x00\x00\x0c\x06\x00\x00\x00\x00\x00\x00\x00\x1c\x0c"
      "\x00\x00\x11\x00\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00\x84\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x65\x0a\x00\x00\xb7\x01\x00\x00\x06\x00\x00\x00\x00"
      "\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\xc5\x09\x00\x00\xa0\x00"
      "\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00\x0c\x06\x00\x00\x00\x00\x00\x00"
      "\x00\xb9\x09\x00\x00\x0c\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00\x54"
      "\xd2\xc2\xbf\x8d\x00\x00\x00\x00\x94\x09\x00\x00\x25\x00\x00\x00\x07\x00"
      "\x00\x00\x00\x00\x00\x00\x84\x00\x00\x00\x00\x00\x00\x00\x00\xdd\x07\x00"
      "\x00\xb7\x01\x00\x00\x09\x00\x00\x00\x00\x00\x00\x00\x84\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x26\x06\x00\x00\xb7\x01\x00\x00\xf7\xff\xff\xff\xff\xff"
      "\xff\xff\x84\x00\x00\x00\x00\x00\x00\x00\x00\x6f\x04\x00\x00\xb7\x01",
      701);
  *(uint64_t*)0x20000328 = 0x2bd;
  *(uint64_t*)0x20000330 = 0x2508ea0;
  *(uint64_t*)0x20000338 = 0x20016700;
  memcpy((void*)0x20016700,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00"
         "\x00\x00\x00\x50\xd0\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x04",
         100);
  *(uint64_t*)0x20000340 = 0x64;
  *(uint64_t*)0x20000348 = 0x2509560;
  *(uint64_t*)0x20000350 = 0x20016800;
  memcpy((void*)0x20016800,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x60\xd0\x01\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x04",
         91);
  *(uint64_t*)0x20000358 = 0x5b;
  *(uint64_t*)0x20000360 = 0x2509720;
  *(uint64_t*)0x20000368 = 0x20016900;
  memcpy((void*)0x20016900, "\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\xed\x41",
         56);
  *(uint64_t*)0x20000370 = 0x38;
  *(uint64_t*)0x20000378 = 0x2509840;
  *(uint64_t*)0x20000380 = 0x20016a00;
  memcpy((void*)0x20016a00,
         "\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x40\xd0\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x04",
         82);
  *(uint64_t*)0x20000388 = 0x52;
  *(uint64_t*)0x20000390 = 0x25098e0;
  *(uint64_t*)0x20000398 = 0x20016b00;
  memcpy((void*)0x20016b00,
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
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x6a"
         "\x31\x5f\x5f\x00\x00\x00\x00\x00\x00\x00\x00\x6a\x31\x5f\x5f\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x6a\x31\x5f\x5f\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x6a\x31\x5f\x5f\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xed\x41",
         288);
  *(uint64_t*)0x200003a0 = 0x120;
  *(uint64_t*)0x200003a8 = 0x25099e0;
  *(uint64_t*)0x200003b0 = 0x20016d00;
  memcpy((void*)0x20016d00,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\xd0\x01\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x09\xaf\x3f\xbe\x4f"
         "\x64\x4e\xed\x88\x00\x0e\xdd\x72\x59\x61\x94",
         113);
  *(uint64_t*)0x200003b8 = 0x71;
  *(uint64_t*)0x200003c0 = 0x2509b60;
  *(uint64_t*)0x200003c8 = 0x20016e00;
  memcpy((void*)0x20016e00, "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000j1__"
                            "\000\000\000\000\000\000\000\000j1__",
         33);
  *(uint64_t*)0x200003d0 = 0x21;
  *(uint64_t*)0x200003d8 = 0x2509c00;
  *(uint64_t*)0x200003e0 = 0x20016f00;
  memcpy((void*)0x20016f00, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x07\x00\x64"
                            "\x65\x66\x61\x75\x6c\x74\x01\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xed\x41",
         72);
  *(uint64_t*)0x200003e8 = 0x48;
  *(uint64_t*)0x200003f0 = 0x2509c80;
  *(uint64_t*)0x200003f8 = 0x20017000;
  memcpy((void*)0x20017000,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x80\xd0\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x05",
         98);
  *(uint64_t*)0x20000400 = 0x62;
  *(uint64_t*)0x20000408 = 0x2509d20;
  *(uint64_t*)0x20000410 = 0x20017100;
  memcpy((void*)0x20017100,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xed\x41",
         63);
  *(uint64_t*)0x20000418 = 0x3f;
  *(uint64_t*)0x20000420 = 0x2509e40;
  *(uint64_t*)0x20000428 = 0x20017200;
  memcpy((void*)0x20017200,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x70\xd0\x01\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x05",
         89);
  *(uint64_t*)0x20000430 = 0x59;
  *(uint64_t*)0x20000438 = 0x2509ee0;
  *(uint64_t*)0x20000440 = 0;
  *(uint64_t*)0x20000448 = 0;
  *(uint64_t*)0x20000450 = 0x4505000;
  *(uint64_t*)0x20000458 = 0;
  *(uint64_t*)0x20000460 = 0;
  *(uint64_t*)0x20000468 = 0x4505f40;
  memcpy((void*)0x20000040, "nossd", 5);
  *(uint8_t*)0x20000045 = 0x2c;
  memcpy((void*)0x20000046, "skip_balance", 12);
  *(uint8_t*)0x20000052 = 0x2c;
  *(uint8_t*)0x20000053 = 0;
  syz_mount_image(0x20000000, 0x20000100, 0xffffffeb, 0x1a, 0x20000200, 0,
                  0x20000040);
  memcpy((void*)0x20000000, "btrfs\000", 6);
  memcpy((void*)0x20000100, "./file0\000", 8);
  *(uint64_t*)0x20000200 = 0x20010000;
  memcpy((void*)0x20010000,
         "\xed\x79\x7e\x4f\x85\x52\x62\x73\x83\xe3\xe8\xbd\x94\xf9\x32\x66\xa9"
         "\x86\x59\x26\xfc\x25\xed\xc5\xcf\x02\x79\xbb\xc6\xda\x70\x97\x8a\xe5"
         "\xb4\x01\x4a\xd8\x41\x68\x86\x72\x01\xe7\xdb\x0b\x90\xb5\x00\x00\x01"
         "\x00\x00\x00\x00\x00\x01\x00\x00\x00\x11\x00\x00\x00\x5f\x42\x48\x52"
         "\x66\x53\x5f\x4d\x05\x00\x00\x00",
         76);
  *(uint64_t*)0x20000208 = 0x4c;
  *(uint64_t*)0x20000210 = 0x10000;
  memcpy((void*)0x20000040, "nossd", 5);
  *(uint8_t*)0x20000045 = 0x2c;
  memcpy((void*)0x20000046, "skip_balance", 12);
  *(uint8_t*)0x20000052 = 0x2c;
  *(uint8_t*)0x20000053 = 0;
  syz_mount_image(0x20000000, 0x20000100, 0, 1, 0x20000200, 0, 0x20000040);
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
