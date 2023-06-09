// https://syzkaller.appspot.com/bug?id=4dd134118111ccc7f7ec35a0977b0d5a6e2dec5d
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/loop.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

extern unsigned long long procid;

struct fs_image_segment {
  void* data;
  uintptr_t size;
  uintptr_t offset;
};

#define IMAGE_MAX_SEGMENTS 4096
#define IMAGE_MAX_SIZE (32 << 20)

#define SYZ_memfd_create 319

static uintptr_t syz_mount_image(uintptr_t fs, uintptr_t dir, uintptr_t size,
                                 uintptr_t nsegs, uintptr_t segments,
                                 uintptr_t flags, uintptr_t opts)
{
  char loopname[64];
  int loopfd, err = 0, res = -1;
  uintptr_t i;
  struct fs_image_segment* segs = (struct fs_image_segment*)segments;

  if (nsegs > IMAGE_MAX_SEGMENTS)
    nsegs = IMAGE_MAX_SEGMENTS;
  for (i = 0; i < nsegs; i++) {
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
  int memfd = syscall(SYZ_memfd_create, "syz_mount_image", 0);
  if (memfd == -1) {
    err = errno;
    goto error;
  }
  if (ftruncate(memfd, size)) {
    err = errno;
    goto error_close_memfd;
  }
  for (i = 0; i < nsegs; i++) {
    if (pwrite(memfd, segs[i].data, segs[i].size, segs[i].offset) < 0) {
    }
  }
  snprintf(loopname, sizeof(loopname), "/dev/loop%llu", procid);
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
  mkdir((char*)dir, 0777);
  if (strcmp((char*)fs, "iso9660") == 0)
    flags |= MS_RDONLY;
  if (mount(loopname, (char*)dir, (char*)fs, flags, (char*)opts)) {
    err = errno;
    goto error_clear_loop;
  }
  res = 0;
error_clear_loop:
  ioctl(loopfd, LOOP_CLR_FD, 0);
error_close_loop:
  close(loopfd);
error_close_memfd:
  close(memfd);
error:
  errno = err;
  return res;
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
  memcpy((void*)0x20000100, "ext3", 5);
  memcpy((void*)0x20000200, "//file0/../..", 14);
  memcpy((void*)0x20000000, "i_version", 9);
  *(uint8_t*)0x20000009 = 0x2c;
  memcpy((void*)0x2000000a, "nouid32", 7);
  *(uint8_t*)0x20000011 = 0x2c;
  memcpy((void*)0x20000012, "debug_want_extra_isize", 22);
  *(uint8_t*)0x20000028 = 0x3d;
  *(uint8_t*)0x20000029 = 0x38;
  *(uint8_t*)0x2000002a = 0x35;
  *(uint8_t*)0x2000002b = 0x33;
  *(uint8_t*)0x2000002c = 0x36;
  *(uint8_t*)0x2000002d = 0x30;
  *(uint8_t*)0x2000002e = 0x35;
  *(uint8_t*)0x2000002f = 0x39;
  *(uint8_t*)0x20000030 = 0x2c;
  memcpy((void*)0x20000031, "nobh", 4);
  *(uint8_t*)0x20000035 = 0x2c;
  memcpy((void*)0x20000036, "block_validity", 14);
  *(uint8_t*)0x20000044 = 0x2c;
  memcpy((void*)0x20000045, "mblk_io_submit", 14);
  *(uint8_t*)0x20000053 = 0x2c;
  *(uint8_t*)0x20000054 = 0;
  syz_mount_image(0x20000100, 0x20000200, 0xfffffffffffffffe, 0, 0x20000140,
                  0x22, 0x20000000);
  memcpy((void*)0x20000640, "//file0", 8);
  syscall(__NR_mkdir, 0x20000640, 0x20);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    loop();
  }
}
