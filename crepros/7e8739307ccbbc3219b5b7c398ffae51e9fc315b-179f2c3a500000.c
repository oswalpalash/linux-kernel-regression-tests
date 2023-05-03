// https://syzkaller.appspot.com/bug?id=7e8739307ccbbc3219b5b7c398ffae51e9fc315b
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

#define KMEMLEAK_FILE "/sys/kernel/debug/kmemleak"

static void setup_leak()
{
  if (!write_file(KMEMLEAK_FILE, "scan"))
    exit(1);
  sleep(5);
  if (!write_file(KMEMLEAK_FILE, "scan"))
    exit(1);
  if (!write_file(KMEMLEAK_FILE, "clear"))
    exit(1);
}

static void check_leaks(void)
{
  int fd = open(KMEMLEAK_FILE, O_RDWR);
  if (fd == -1)
    exit(1);
  uint64_t start = current_time_ms();
  if (write(fd, "scan", 4) != 4)
    exit(1);
  sleep(1);
  while (current_time_ms() - start < 4 * 1000)
    sleep(1);
  if (write(fd, "scan", 4) != 4)
    exit(1);
  static char buf[128 << 10];
  ssize_t n = read(fd, buf, sizeof(buf) - 1);
  if (n < 0)
    exit(1);
  int nleaks = 0;
  if (n != 0) {
    sleep(1);
    if (write(fd, "scan", 4) != 4)
      exit(1);
    if (lseek(fd, 0, SEEK_SET) < 0)
      exit(1);
    n = read(fd, buf, sizeof(buf) - 1);
    if (n < 0)
      exit(1);
    buf[n] = 0;
    char* pos = buf;
    char* end = buf + n;
    while (pos < end) {
      char* next = strstr(pos + 1, "unreferenced object");
      if (!next)
        next = end;
      char prev = *next;
      *next = 0;
      fprintf(stderr, "BUG: memory leak\n%s\n", pos);
      *next = prev;
      pos = next;
      nleaks++;
    }
  }
  if (write(fd, "clear", 5) != 5)
    exit(1);
  close(fd);
  if (nleaks)
    exit(1);
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
    check_leaks();
  }
}

void execute_one(void)
{
  memcpy((void*)0x20000000, "nilfs2\000", 7);
  memcpy((void*)0x20000100, "./file0\000", 8);
  *(uint64_t*)0x20000200 = 0x20010000;
  memcpy((void*)0x20010000,
         "\x02\x00\x00\x00\x00\x00\x34\x34\x18\x01\x00\x00\x7a\x4a\x79\x34\x40"
         "\xd1\x97\xc3\x01\x00\x00\x00\x1f\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x10\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00"
         "\x00\x05\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xe0\x01\x00\x00\x00"
         "\x00\x00\x00\x56\x1b\x67\x5f\x00\x00\x00\x00\x57\x1b\x67\x5f\x00\x00"
         "\x00\x00\x57\x1b\x67\x5f\x00\x00\x00\x00\x01\x00\x32\x00\x00\x00\x01"
         "\x00\x56\x1b\x67\x5f\x00\x00\x00\x00\x00\x4e\xed\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x0b\x00\x00\x00\x80\x00\x20\x00\xc0\x00\x10\x00\xe2"
         "\x82\x53\x61\x8b\x3b\x42\x84\x9b\x95\x43\xa5\x40\x24\x81\x3a\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00",
         192);
  *(uint64_t*)0x20000208 = 0xc0;
  *(uint64_t*)0x20000210 = 0x400;
  *(uint64_t*)0x20000218 = 0x20010100;
  memcpy((void*)0x20010100, "\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint64_t*)0x20000220 = 0x20;
  *(uint64_t*)0x20000228 = 0x500;
  *(uint64_t*)0x20000230 = 0;
  *(uint64_t*)0x20000238 = 0;
  *(uint64_t*)0x20000240 = 0x1000;
  *(uint64_t*)0x20000248 = 0;
  *(uint64_t*)0x20000250 = 0;
  *(uint64_t*)0x20000258 = 0x1800;
  *(uint64_t*)0x20000260 = 0;
  *(uint64_t*)0x20000268 = 0;
  *(uint64_t*)0x20000270 = 0x2000;
  *(uint64_t*)0x20000278 = 0;
  *(uint64_t*)0x20000280 = 0;
  *(uint64_t*)0x20000288 = 0x3080;
  *(uint64_t*)0x20000290 = 0;
  *(uint64_t*)0x20000298 = 0;
  *(uint64_t*)0x200002a0 = 0x3100;
  *(uint64_t*)0x200002a8 = 0;
  *(uint64_t*)0x200002b0 = 0;
  *(uint64_t*)0x200002b8 = 0x3380;
  *(uint64_t*)0x200002c0 = 0;
  *(uint64_t*)0x200002c8 = 0;
  *(uint64_t*)0x200002d0 = 0x3400;
  *(uint64_t*)0x200002d8 = 0;
  *(uint64_t*)0x200002e0 = 0;
  *(uint64_t*)0x200002e8 = 0x3480;
  *(uint64_t*)0x200002f0 = 0;
  *(uint64_t*)0x200002f8 = 0;
  *(uint64_t*)0x20000300 = 0x3500;
  *(uint64_t*)0x20000308 = 0;
  *(uint64_t*)0x20000310 = 0;
  *(uint64_t*)0x20000318 = 0x3580;
  *(uint64_t*)0x20000320 = 0;
  *(uint64_t*)0x20000328 = 0;
  *(uint64_t*)0x20000330 = 0x3800;
  *(uint64_t*)0x20000338 = 0;
  *(uint64_t*)0x20000340 = 0;
  *(uint64_t*)0x20000348 = 0x38c0;
  *(uint64_t*)0x20000350 = 0;
  *(uint64_t*)0x20000358 = 0;
  *(uint64_t*)0x20000360 = 0x3a40;
  *(uint64_t*)0x20000368 = 0;
  *(uint64_t*)0x20000370 = 0;
  *(uint64_t*)0x20000378 = 0x3b00;
  *(uint64_t*)0x20000380 = 0;
  *(uint64_t*)0x20000388 = 0;
  *(uint64_t*)0x20000390 = 0x3bc0;
  *(uint64_t*)0x20000398 = 0;
  *(uint64_t*)0x200003a0 = 0;
  *(uint64_t*)0x200003a8 = 0x3c80;
  *(uint64_t*)0x200003b0 = 0;
  *(uint64_t*)0x200003b8 = 0;
  *(uint64_t*)0x200003c0 = 0x3d40;
  *(uint64_t*)0x200003c8 = 0;
  *(uint64_t*)0x200003d0 = 0;
  *(uint64_t*)0x200003d8 = 0x3e00;
  *(uint64_t*)0x200003e0 = 0;
  *(uint64_t*)0x200003e8 = 0;
  *(uint64_t*)0x200003f0 = 0x3ec0;
  *(uint64_t*)0x200003f8 = 0;
  *(uint64_t*)0x20000400 = 0;
  *(uint64_t*)0x20000408 = 0x4000;
  *(uint64_t*)0x20000410 = 0;
  *(uint64_t*)0x20000418 = 0;
  *(uint64_t*)0x20000420 = 0x4800;
  *(uint64_t*)0x20000428 = 0;
  *(uint64_t*)0x20000430 = 0;
  *(uint64_t*)0x20000438 = 0x5820;
  *(uint64_t*)0x20000440 = 0;
  *(uint64_t*)0x20000448 = 0;
  *(uint64_t*)0x20000450 = 0x6000;
  *(uint64_t*)0x20000458 = 0;
  *(uint64_t*)0x20000460 = 0;
  *(uint64_t*)0x20000468 = 0x6800;
  *(uint64_t*)0x20000470 = 0;
  *(uint64_t*)0x20000478 = 0;
  *(uint64_t*)0x20000480 = 0x7000;
  *(uint64_t*)0x20000488 = 0;
  *(uint64_t*)0x20000490 = 0;
  *(uint64_t*)0x20000498 = 0x7800;
  *(uint64_t*)0x200004a0 = 0;
  *(uint64_t*)0x200004a8 = 0;
  *(uint64_t*)0x200004b0 = 0x8000;
  *(uint64_t*)0x200004b8 = 0;
  *(uint64_t*)0x200004c0 = 0;
  *(uint64_t*)0x200004c8 = 0x8800;
  *(uint64_t*)0x200004d0 = 0;
  *(uint64_t*)0x200004d8 = 0;
  *(uint64_t*)0x200004e0 = 0x9000;
  *(uint64_t*)0x200004e8 = 0;
  *(uint64_t*)0x200004f0 = 0;
  *(uint64_t*)0x200004f8 = 0x9800;
  *(uint64_t*)0x20000500 = 0;
  *(uint64_t*)0x20000508 = 0;
  *(uint64_t*)0x20000510 = 0xc800;
  *(uint64_t*)0x20000518 = 0;
  *(uint64_t*)0x20000520 = 0;
  *(uint64_t*)0x20000528 = 0xd000;
  *(uint64_t*)0x20000530 = 0;
  *(uint64_t*)0x20000538 = 0;
  *(uint64_t*)0x20000540 = 0xe080;
  *(uint64_t*)0x20000548 = 0;
  *(uint64_t*)0x20000550 = 0;
  *(uint64_t*)0x20000558 = 0xe100;
  *(uint64_t*)0x20000560 = 0;
  *(uint64_t*)0x20000568 = 0;
  *(uint64_t*)0x20000570 = 0xe380;
  *(uint64_t*)0x20000578 = 0;
  *(uint64_t*)0x20000580 = 0;
  *(uint64_t*)0x20000588 = 0xe400;
  *(uint64_t*)0x20000590 = 0;
  *(uint64_t*)0x20000598 = 0;
  *(uint64_t*)0x200005a0 = 0xe480;
  *(uint64_t*)0x200005a8 = 0;
  *(uint64_t*)0x200005b0 = 0;
  *(uint64_t*)0x200005b8 = 0xe500;
  *(uint64_t*)0x200005c0 = 0;
  *(uint64_t*)0x200005c8 = 0;
  *(uint64_t*)0x200005d0 = 0xe580;
  *(uint64_t*)0x200005d8 = 0;
  *(uint64_t*)0x200005e0 = 0;
  *(uint64_t*)0x200005e8 = 0xe600;
  *(uint64_t*)0x200005f0 = 0;
  *(uint64_t*)0x200005f8 = 0;
  *(uint64_t*)0x20000600 = 0xf000;
  *(uint64_t*)0x20000608 = 0;
  *(uint64_t*)0x20000610 = 0;
  *(uint64_t*)0x20000618 = 0xf0c0;
  *(uint64_t*)0x20000620 = 0;
  *(uint64_t*)0x20000628 = 0;
  *(uint64_t*)0x20000630 = 0xf180;
  *(uint64_t*)0x20000638 = 0;
  *(uint64_t*)0x20000640 = 0;
  *(uint64_t*)0x20000648 = 0xf300;
  *(uint64_t*)0x20000650 = 0;
  *(uint64_t*)0x20000658 = 0;
  *(uint64_t*)0x20000660 = 0xf3c0;
  *(uint64_t*)0x20000668 = 0;
  *(uint64_t*)0x20000670 = 0;
  *(uint64_t*)0x20000678 = 0xf480;
  *(uint64_t*)0x20000680 = 0;
  *(uint64_t*)0x20000688 = 0;
  *(uint64_t*)0x20000690 = 0xf540;
  *(uint64_t*)0x20000698 = 0;
  *(uint64_t*)0x200006a0 = 0;
  *(uint64_t*)0x200006a8 = 0xf600;
  *(uint64_t*)0x200006b0 = 0;
  *(uint64_t*)0x200006b8 = 0;
  *(uint64_t*)0x200006c0 = 0xf6c0;
  *(uint64_t*)0x200006c8 = 0;
  *(uint64_t*)0x200006d0 = 0;
  *(uint64_t*)0x200006d8 = 0xf800;
  *(uint64_t*)0x200006e0 = 0;
  *(uint64_t*)0x200006e8 = 0;
  *(uint64_t*)0x200006f0 = 0x10000;
  *(uint64_t*)0x200006f8 = 0;
  *(uint64_t*)0x20000700 = 0;
  *(uint64_t*)0x20000708 = 0x103c0;
  *(uint64_t*)0x20000710 = 0;
  *(uint64_t*)0x20000718 = 0;
  *(uint64_t*)0x20000720 = 0x10800;
  *(uint64_t*)0x20000728 = 0;
  *(uint64_t*)0x20000730 = 0;
  *(uint64_t*)0x20000738 = 0x11100;
  *(uint64_t*)0x20000740 = 0;
  *(uint64_t*)0x20000748 = 0;
  *(uint64_t*)0x20000750 = 0x11400;
  *(uint64_t*)0x20000758 = 0;
  *(uint64_t*)0x20000760 = 0;
  *(uint64_t*)0x20000768 = 0x11500;
  *(uint64_t*)0x20000770 = 0;
  *(uint64_t*)0x20000778 = 0;
  *(uint64_t*)0x20000780 = 0x11600;
  *(uint64_t*)0x20000788 = 0;
  *(uint64_t*)0x20000790 = 0;
  *(uint64_t*)0x20000798 = 0x11700;
  *(uint64_t*)0x200007a0 = 0;
  *(uint64_t*)0x200007a8 = 0;
  *(uint64_t*)0x200007b0 = 0x11820;
  *(uint64_t*)0x200007b8 = 0;
  *(uint64_t*)0x200007c0 = 0;
  *(uint64_t*)0x200007c8 = 0x12000;
  *(uint64_t*)0x200007d0 = 0;
  *(uint64_t*)0x200007d8 = 0;
  *(uint64_t*)0x200007e0 = 0x12800;
  *(uint64_t*)0x200007e8 = 0;
  *(uint64_t*)0x200007f0 = 0;
  *(uint64_t*)0x200007f8 = 0x13000;
  *(uint64_t*)0x20000800 = 0;
  *(uint64_t*)0x20000808 = 0;
  *(uint64_t*)0x20000810 = 0x13800;
  *(uint64_t*)0x20000818 = 0;
  *(uint64_t*)0x20000820 = 0;
  *(uint64_t*)0x20000828 = 0x14000;
  *(uint64_t*)0x20000830 = 0;
  *(uint64_t*)0x20000838 = 0;
  *(uint64_t*)0x20000840 = 0x14800;
  *(uint64_t*)0x20000848 = 0;
  *(uint64_t*)0x20000850 = 0;
  *(uint64_t*)0x20000858 = 0x14c00;
  *(uint64_t*)0x20000860 = 0;
  *(uint64_t*)0x20000868 = 0;
  *(uint64_t*)0x20000870 = 0x15000;
  *(uint64_t*)0x20000878 = 0;
  *(uint64_t*)0x20000880 = 0;
  *(uint64_t*)0x20000888 = 0xff000;
  *(uint64_t*)0x20000890 = 0;
  *(uint64_t*)0x20000898 = 0;
  *(uint64_t*)0x200008a0 = 0xff100;
  *(uint8_t*)0x20017900 = 0;
  syz_mount_image(0x20000000, 0x20000100, 0x100000, 0x47, 0x20000200, 0,
                  0x20017900);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  setup_leak();
  loop();
  return 0;
}
