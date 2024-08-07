// https://syzkaller.appspot.com/bug?id=2a3c5f6dc8ee7d04e8531e92b2bcef857aba69d3
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
#define IMAGE_MAX_SIZE (129 << 20)

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
  memcpy((void*)0x20000080, "btrfs", 6);
  memcpy((void*)0x20000100, "./file0", 8);
  *(uint64_t*)0x20000200 = 0x20000340;
  memcpy((void*)0x20000340,
         "\x8d\xa4\x36\x3a\x00\x00\x00\x00\x00\x00\x00\x00\x00\x15\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xec\xf6"
         "\xf2\xa3\x29\x97\x48\xae\xb8\x1e\x1b\x00\x92\x0e\xfd\x9a\x00\x00\x01"
         "\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x5f\x42\x48\x52"
         "\x66\x53\x5f\x4d\xd5\xf4\x5b\x0a\xdb\x28\x71\x9b\x57\xa4\xbf\x4d\x55"
         "\x59\x9e\xc8\x48\x9a\x6c\x33\x4d\xab\xa1\x59\x70\xbb\x7c\x81\x01\xe1"
         "\x01\xc9\xe4\x41\x69\xd0\x00\x41\x2d\x68\xd7\x32\xea\x99\x8e\xe0\xca"
         "\xbf\x13\x2f\x26\x7a\xc9\x48\x78\x3c\x1c\xc5\x4e\x65\x34\xd4\x7c\xe1"
         "\xe2\xd3\x96\xa7\xad\x6d\x43\x82\xd2\x77\x89\x43\x73\xd4\x02\xd9\xb5"
         "\x33\xe1\x82\x89\xee\x97\xd4\x07\xdc\xf4\x9c\x27\x63\xbe\x84\x09\xa8"
         "\xb1\xca\x26\x0f\x2e\x26\x36\x0c\x8a\x75\x69\x4d\x32\xc0\xda\x67\xf2"
         "\xaf\x29\xe3\x45\xd0\xa3\x70\xee\xa0\xe3\x3f\x83\x62\x11\x0f",
         202);
  *(uint64_t*)0x20000208 = 0xca;
  *(uint64_t*)0x20000210 = 0x10000;
  *(uint8_t*)0x20000140 = 0;
  syz_mount_image(0x20000080, 0x20000100, 0, 1, 0x20000200, 0, 0x20000140);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      for (;;) {
        loop();
      }
    }
  }
  sleep(1000000);
  return 0;
}
