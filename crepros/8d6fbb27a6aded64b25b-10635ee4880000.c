// https://syzkaller.appspot.com/bug?id=ad9be3e2f95396e26ee0db3cdd30e41d00a577fc
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
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

#ifndef __NR_memfd_create
#define __NR_memfd_create 319
#endif

static unsigned long long procid;

struct fs_image_segment {
  void* data;
  uintptr_t size;
  uintptr_t offset;
};

#define IMAGE_MAX_SEGMENTS 4096
#define IMAGE_MAX_SIZE (129 << 20)

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
  int memfd = syscall(__NR_memfd_create, "syzkaller", 0);
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
                            volatile long optsarg, volatile long change_dir)
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
    goto error_clear_loop;
  }
  if (change_dir) {
    res = chdir(target);
    if (res == -1) {
      err = errno;
    }
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

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);

  memcpy((void*)0x20000000, "ntfs3\000", 6);
  memcpy((void*)0x20000100, "./file0\000", 8);
  *(uint64_t*)0x20000200 = 0x20010000;
  memcpy(
      (void*)0x20010000,
      "\xeb\x52\x90\x4e\x54\x46\x53\x20\x20\x20\x20\x00\x02\x08\x00\x00\x00\x00"
      "\x00\x00\x00\xf8\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x80\x00\x80\x00\xff\x0f\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00"
      "\x00\x00\xff\x00\x00\x00\x00\x00\x00\x00\xf6\x00\x00\x00\x01",
      69);
  *(uint64_t*)0x20000208 = 0x45;
  *(uint64_t*)0x20000210 = 0;
  *(uint64_t*)0x20000218 = 0x20010120;
  memcpy(
      (void*)0x20010120,
      "\x46\x49\x4c\x45\x30\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00"
      "\x01\x00\x38\x00\x01\x00\x98\x01\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00"
      "\x00\x00\x10\x00\x00\x00\x60\x00\x00\x00\x00\x00\x18\x00\x00\x00\x00\x00"
      "\x48\x00\x00\x00\x18\x00\x00\x00\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab"
      "\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2"
      "\x4e\xc4\xd8\x01\x06\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x30\x00\x00\x00\x68\x00\x00\x00\x00\x00"
      "\x18\x00\x00\x00\x02\x00\x4a\x00\x00\x00\x18\x00\x01\x00\x05\x00\x00\x00"
      "\x00\x00\x05\x00\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4"
      "\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01"
      "\x00\x70\x00\x00\x00\x00\x00\x00\x00\x6c\x00\x00\x00\x00\x00\x00\x06\x00"
      "\x00\x00\x00\x00\x00\x00\x04\x03\x24\x00\x4d\x00\x46\x00\x54\x00\x00\x00"
      "\x00\x00\x00\x00\x80\x00\x00\x00\x48\x00\x00\x00\x01\x00\x40\x00\x00\x00"
      "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x3e\x00\x00\x00\x00\x00\x00\x00"
      "\x40\x00\x00\x00\x00\x00\x00\x00\x00\xf0\x03\x00\x00\x00\x00\x00\x00\xf0"
      "\x03\x00\x00\x00\x00\x00\x00\xf0\x03\x00\x00\x00\x00\x00\x11\x3f\x04\x00"
      "\x00\x00\x00\x00\xb0\x00\x00\x00\x48\x00\x00\x00\x01\x00\x40\x00\x00\x00"
      "\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x40\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x20\x00"
      "\x00\x00\x00\x00\x00\x00\x20\x00\x00\x00\x00\x00\x00\x00\x11\x01\x02",
      395);
  *(uint64_t*)0x20000220 = 0x18b;
  *(uint64_t*)0x20000228 = 0x4000;
  *(uint64_t*)0x20000230 = 0x200102e0;
  memcpy(
      (void*)0x200102e0,
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xb0\x00"
      "\x00\x00\x48\x00\x00\x00\x01\x00\x40\x00\x00\x00\x03\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x40\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00\x00\x00\x00\x00"
      "\x08\x00\x00\x00\x00\x00\x00\x00\x11\x01\x02\x00\x00\x00\x00\x00\xff\xff"
      "\xff\xff\x00\x00\x03\x00\x46\x49\x4c\x45\x30\x00\x03\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x01\x00\x01\x00\x38\x00\x01\x00\x58\x01\x00\x00\x00\x04"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x01\x00\x00\x00"
      "\x02\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x60\x00\x00\x00\x00\x00"
      "\x18\x00\x00\x00\x00\x00\x48\x00\x00\x00\x18\x00\x00\x00\x00\xab\x2b\xd2"
      "\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4"
      "\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x06\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x30\x00\x00\x00"
      "\x70\x00\x00\x00\x00\x00\x18\x00\x00\x00\x02\x00\x52\x00\x00\x00\x18\x00"
      "\x01\x00\x05\x00\x00\x00\x00\x00\x05\x00\x00\xab\x2b\xd2\x4e\xc4\xd8\x01"
      "\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab"
      "\x2b\xd2\x4e\xc4\xd8\x01\x00\x10\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00"
      "\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00\x08\x03\x24\x00\x4d\x00"
      "\x46\x00\x54\x00\x4d\x00\x69\x00\x72\x00\x72\x00\x00\x00\x00\x00\x00\x00"
      "\x80\x00\x00\x00\x48",
      365);
  *(uint64_t*)0x20000238 = 0x16d;
  *(uint64_t*)0x20000240 = 0x43a0;
  *(uint64_t*)0x20000248 = 0x200104c0;
  memcpy(
      (void*)0x200104c0,
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x46\x49\x4c\x45"
      "\x30\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x01\x00\x38\x00"
      "\x01\x00\x58\x01\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x03\x00\x00\x00\x02\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x10\x00"
      "\x00\x00\x60\x00\x00\x00\x00\x00\x18\x00\x00\x00\x00\x00\x48\x00\x00\x00"
      "\x18\x00\x00\x00\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4"
      "\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01"
      "\x06\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00\x00\x00\x00\x00\x00\x96\x00"
      "\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x30\x00\x00\x00\x70\x00\x00\x00\x00\x00\x18\x00\x00\x00"
      "\x02\x00\x52\x00\x00\x00\x18\x00\x01\x00\x05\x00\xe2\x00\x00\x00\x05\x00"
      "\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab"
      "\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\x00\x04\x00"
      "\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00"
      "\x00\x00\x08\x03\x24\x80\x4c\x00\x6f\x00\x67\x00\x46\x00\x69\x00\x6c\x00"
      "\x65\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x48\x00\x00\x00\x01\x00"
      "\x40\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x3f\x00\x00\xa6"
      "\x22\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00"
      "\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00",
      364);
  *(uint64_t*)0x20000250 = 0x16c;
  *(uint64_t*)0x20000258 = 0x47e0;
  *(uint64_t*)0x20000260 = 0x20010660;
  memcpy(
      (void*)0x20010660,
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x46\x49\x4c\x45"
      "\x30\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x01\x00\x38\x00"
      "\x01\x00\xe0\x01\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x06\x00\x00\x00\x03\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x10\x00"
      "\x00\x00\x48\x00\x00\x00\x00\x00\x18\x00\x00\x00\x00\x00\x30\x00\x00\x00"
      "\x18\x00\x00\x00\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4"
      "\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01"
      "\x06\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x30\x00"
      "\x00\x00\x68\x00\x00\x00\x00\x00\x18\x00\x00\x00\x01\x00\x50\x00\x00\x00"
      "\x18\x00\x01\x00\x05\x00\x00\x00\x00\x00\x05\x00\x00\xab\x2b\xd2\x4e\xc4"
      "\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01"
      "\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00\x07\x03\x24\x00"
      "\x56\x00\x6f\x00\x6c\x00\x75\x00\x6d\x00\x65\x00\x50\x00\x00\x00\x80\x00"
      "\x00\x00\x00\x00\x18\x00\x00\x00\x02\x00\x64\x00\x00\x00\x18\x00\x00\x00"
      "\x01\x00\x04\x80\x48\x00\x00\x00\x54\x00\x00\x00\x00\x00\x00\x00\x14\x00"
      "\x00\x00\x02\x00\x34\x00\x02\x00\x00\x00\x00\x00\x14\x00\x9f\x01\x12\x00"
      "\x01\x01\x00\x00\x00\x00\x00\x05\x12\x00\x00\x00\x00\x00\x18\x00\x9f\x01"
      "\x12\x00\x01\x02\x00\x00\x00\x00\x00\x05\x20\x00\x00\x00\x20\x02\x00\x00"
      "\x01\x01\x00\x00\x00\x00\x00\x05\x12\x00\x00\x00\x01\x02\x00\x00\x00\x00"
      "\x00\x05\x20\x00\x00\x00\x20\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x30\x00\x00\x00\x00\x00\x18\x00\x00\x00\x04\x00\x12\x00\x00\x00\x18\x00"
      "\x00\x00\x73\x00\x79\x00\x7a\x00\x6b\x00\x61\x00\x6c\x00\x6c\x00\x65\x00"
      "\x72\x00\x00\x00\x00\x00\x00\x00\x70\x00\x00\x00\x28\x00\x00\x00\x00\x00"
      "\x18\x00\x00\x00\x05\x00\x0c\x00\x00\x00\x18\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x03\x01\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x18",
      485);
  *(uint64_t*)0x20000268 = 0x1e5;
  *(uint64_t*)0x20000270 = 0x4be0;
  *(uint64_t*)0x20000278 = 0x20010880;
  memcpy(
      (void*)0x20010880,
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x04\x00\x46\x49\x4c\x45"
      "\x30\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x04\x00\x01\x00\x38\x00"
      "\x01\x00\xc0\x01\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x04\x00\x00\x00\x04\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x10\x00"
      "\x00\x00\x48\x00\x00\x00\x00\x00\x18\x00\x00\x00\x00\x00\x30\x00\x00\x00"
      "\x18\x00\x00\x00\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4"
      "\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01"
      "\x06\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x30\x00"
      "\x00\x00\x70\x00\x00\x00\x00\x00\x18\x00\x00\x00\x02\x00\x52\x00\x00\x00"
      "\x18\x00\x01\x00\x05\x00\x00\x00\x00\x00\x05\x00\x00\xab\x2b\xd2\x4e\xc4"
      "\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01"
      "\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\x10\x00\x00\x00\x00\x00\x00\x00\x0a"
      "\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00\x08\x03\x24\x00"
      "\x41\x00\x74\x00\x74\x00\x72\x00\x44\x00\x65\x00\x66\x00\x00\x00\x00\x00"
      "\x00\x00\x50\x00\x00\x00\x80\x00\x00\x00\x00\x00\x18\x00\x00\x00\x03\x00"
      "\x64\x00\x00\x00\x18\x00\x00\x00\x01\x00\x04\x80\x48\x00\x00\x00\x54\x00"
      "\x00\x00\x00\x00\x00\x00\x14\x00\x00\x00\x02\x00\x34\x00\x02\x00\x00\x00"
      "\x00\x00\x14\x00\x89\x00\x12\x00\x01\x01\x00\x00\x00\x00\x00\x05\x12\x00"
      "\x00\x00\x00\x00\x18\x00\x89\x00\x12\x00\x01\x02\x00\x00\x00\x00\x00\x05"
      "\x20\x00\x00\x00\x20\x02\x00\x00\x01\x01\x00\x00\x00\x00\x00\x05\x12\x00"
      "\x00\x00\x01\x02\x00\x00\x00\x00\x00\x05\x20\x00\x00\x00\x20\x02\x00\x00"
      "\x00\x00\x00\x00\x80\x00\x00\x00\x48\x00\x00\x00\x01\x00\x40\x00\x00\x00"
      "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x40\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x00\x0a"
      "\x00\x00\x00\x00\x00\x00\x00\x0a\x00\x00\x00\x00\x00\x00\x11\x01\x46",
      467);
  *(uint64_t*)0x20000280 = 0x1d3;
  *(uint64_t*)0x20000288 = 0x4fe0;
  *(uint64_t*)0x20000290 = 0x20010a80;
  memcpy(
      (void*)0x20010a80,
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x46\x49\x4c\x45"
      "\x30\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x05\x00\x01\x00\x38\x00"
      "\x03\x00\x00\x02\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x06\x00\x00\x00\x05\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x10\x00"
      "\x00\x00\x48\x00\x00\x00\x00\x00\x18\x00\x00\x00\x00\x00\x30\x00\x00\x00"
      "\x18\x00\x00\x00\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x13\x2d\x87\xd2\x4e\xc4"
      "\xd8\x01\x13\x2d\x87\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01"
      "\x26\x00\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x30\x00"
      "\x00\x00\x60\x00\x00\x00\x00\x00\x18\x00\x00\x00\x01\x00\x44\x00\x00\x00"
      "\x18\x00\x01\x00\x05\x00\x00\x00\x00\x00\x05\x00\x00\xab\x2b\xd2\x4e\xc4"
      "\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01"
      "\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x06\x00\x00\x10\x00\x00\x00\x00\x01\x03\x2e\x00"
      "\x00\x00\x00\x00\x50\x00\x00\x00\x48\x00\x00\x00\x01\x00\x40\x00\x00\x00"
      "\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"
      "\x40\x00\x00\x00\x00\x00\x00\x00\x00\x20\x00\x00\x00\x00\x00\x00\x2c\x10"
      "\x00\x00\x00\x00\x00\x00\x2c\x10\x00\x00\x00\x00\x00\x00\x11\x02\x43\x00"
      "\x00\x00\x00\x00\x90\x00\x00\x00\x58\x00\x00\x00\x00\x04\x18\x00\x00\x00"
      "\x03\x00\x38\x00\x00\x00\x20\x00\x00\x00\x24\x00\x49\x00\x33\x00\x30\x00"
      "\x30\x00\x00\x00\x01\x00\x00\x00\x00\x10\x00\x00\x01",
      373);
  *(uint64_t*)0x20000298 = 0x175;
  *(uint64_t*)0x200002a0 = 0x53e0;
  *(uint64_t*)0x200002a8 = 0x20010ca0;
  memcpy(
      (void*)0x20010ca0,
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x46\x49\x4c\x45"
      "\x30\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x06\x00\x01\x00\x38\x00"
      "\x01\x00\x50\x01\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x03\x00\x00\x00\x06\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x10\x00"
      "\x00\x00\x60\x00\x00\x00\x00\x00\x18\x00\x00\x00\x00\x00\x48\x00\x00\x00"
      "\x18\x00\x00\x00\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4"
      "\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01"
      "\x06\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x30\x00\x00\x00\x68\x00\x00\x00\x00\x00\x18\x00\x00\x00"
      "\x02\x00\x50\x00\x00\x00\x18\x00\x01\x00\x05\x00\x00\x00\x00\x00\x05\x00"
      "\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab"
      "\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\x10\x00\x00"
      "\x00\x00\x00\x00\x40\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00"
      "\x00\x00\x07\x03\x24\x00\x42\x00\x69\x00\x74\x00\x6d\x00\x61\x00\x70\x00"
      "\x80\x00\x00\x00\x48\x00\x00\x00\x01\x00\x40\x00\x00\x00\x01\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x40\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x40\x00\x00\x00\x00\x00"
      "\x00\x00\x40\x00\x00\x00\x00\x00\x00\x00\x11\x01\x47",
      355);
  *(uint64_t*)0x200002b0 = 0x163;
  *(uint64_t*)0x200002b8 = 0x57e0;
  *(uint64_t*)0x200002c0 = 0x20011040;
  memcpy(
      (void*)0x20011040,
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x46\x49\x4c\x45"
      "\x30\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x01\x00\x38\x00"
      "\x01\x00\x78\x01\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x04\x00\x00\x00\x08\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x10\x00"
      "\x00\x00\x60\x00\x00\x00\x00\x00\x18\x00\x00\x00\x00\x00\x48\x00\x00\x00"
      "\x18\x00\x00\x00\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4"
      "\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01"
      "\x06\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x30\x00\x00\x00\x70\x00\x00\x00\x00\x00\x18\x00\x00\x00"
      "\x03\x00\x52\x00\x00\x00\x18\x00\x01\x00\x05\x00\x00\x00\x00\x00\x05\x00"
      "\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab"
      "\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00"
      "\x00\x00\x08\x03\x24\x00\x42\x00\x61\x00\x64\x00\x43\x00\x6c\x00\x75\x00"
      "\x73\x00\xff\xff\x00\x00\x00\x00\x80\x00\x00\x00\x18\x00\x00\x00\x00\x00"
      "\x18\x00\x00\x00\x02\x00\x00\x00\x00\x00\x18\x00\x00\x00\x80\x00\x00\x00"
      "\x50\x00\x00\x00\x01\x04\x40\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\xfe\x01\x00\x00\x00\x00\x00\x00\x48\x00\x00\x00\x00\x00\x00\x00"
      "\x00\xf0\x1f\x00\x00\x00\x00\x00\x00\xf0\x1f\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x24\x00\x42\x00\x61\x00\x64\x00\x02\xff\x01",
      395);
  *(uint64_t*)0x200002c8 = 0x18b;
  *(uint64_t*)0x200002d0 = 0x5fe0;
  *(uint64_t*)0x200002d8 = 0x20000340;
  memcpy(
      (void*)0x20000340,
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x46\x49\x4c\x45"
      "\x30\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x09\x00\x01\x00\x38\x00"
      "\x09\x00\x00\x03\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x05\x00\x00\x00\x09\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x10\x00"
      "\x00\x00\x60\x00\x00\x00\x00\x00\x18\x00\x00\x00\x00\x00\x48\x00\x00\x00"
      "\x18\x00\x00\x00\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4"
      "\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01"
      "\x06\x00\x00\x20\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x30\x00\x00\x00\x68\x00\x00\x00\x00\x00\x18\x00\x00\x00"
      "\x01\x00\x50\x00\x00\x00\x18\x00\x01\x00\x05\x00\x00\x00\x00\x00\x05\x00"
      "\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab"
      "\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x20\x00\x00"
      "\x00\x00\x07\x03\x24\x00\x53\x00\x65\x00\x63\x00\x75\x00\x72\x00\x65\x00"
      "\x80\x00\x00\x00\x50\x00\x00\x00\x01\x04\x40\x00\x00\x00\x02\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x40\x00\x00\x00\x00\x00\x00\x00\x48\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x10\x04\x00\x00\x00\x00\x00\x70\x01\x04\x00\x00\x00"
      "\x00\x00\x70\x01\x04\x00\x00\x00\x00\x00\x24\x00\x53\x00\x44\x00\x53\x00"
      "\x11\x41\x48\x00\x00\x00\x00\x00\x90\x00\x00\x00\xe0\x00\x00\x00\x00\x04"
      "\x18\x00\x00\x00\x03\x00\xc0\x00\x00\x00\x20\x00\x00\x00\x24\x00\x53\x00"
      "\x44\x00\x48\x00\x00\x00\x00\x00\x12\x00\x00\x00\x00\x10\x00\x00\x01\x00"
      "\x00\x00\x10\x00\x00\x00\xb0\x00\x00\x00\xb0\x00\x00\x00\x00\x00\x00\x00"
      "\x18\x00\x14\x00\x00\x00\x00\x00\x30\x00\x08\x00\x00\x00\x00\x00\x51\x24"
      "\xb3\x00\x01\x01\x00\x00\x51\x24\xb3\x00\x01\x01\x00\x00\x80\x00\x00\x00"
      "\x00\x00\x00\x00\x7c\x00\x00\x00\x49\x00\x49\x00\x18\x00\x14\x00\x00\x00"
      "\x00\x00\x30\x00\x08\x00\x00\x00\x00\x00\xa0\xdd\xde\x42\x02\x01\x00\x00"
      "\xa0\xdd\xde\x42\x02\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x64\x00"
      "\x00\x00\x49\x00\x49\x00\x18\x00\x14\x00\x00\x00\x00\x00\x30\x00\x08\x00"
      "\x00\x00\x03\x00\xf0\x12\x03\xf8\x00\x01\x00\x00\xf0\x12\x03\xf8\x00\x01"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x7c\x00\x00\x00\x49\x00\x49\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x02\x00\x00\x00\x90\x00"
      "\x00\x00\xc8\x00\x00\x00\x00\x04\x18\x00\x00\x00\x04\x00\xa8\x00\x00\x00"
      "\x20\x00\x00\x00\x24\x00\x53\x00\x49\x00\x49\x00\x00\x00\x00\x00\x10\x00"
      "\x00\x00\x00\x10\x00\x00\x01\x00\x00\x00\x10\x00\x00\x00\x98\x00\x00\x00"
      "\x98\x00\x00\x00\x00\x00\x00\x00\x14\x00\x14\x00\x00\x00\x00\x00\x28\x00"
      "\x04\x00\x00\x00\x00\x00\x00\x01\x00\x00\xf0\x12\x03\xf8\x00\x01\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x7c\x00\x00\x00\x14\x00\x14\x00\x00\x00"
      "\x00\x00\x28\x00\x04\x00\x00\x00\x00\x00\x01\x01\x00\x00\x51\x24\xb3\x00"
      "\x01\x01\x00\x00\x80\x00\x00\x00\x00\x00\x00\x00\x7c\x00\x00\x00\x14\x00"
      "\x14\x00\x00\x00\x00\x00\x28\x00\x04\x00\x00\x00\x00\x00\x02\x01\x00\x00"
      "\xa0\xdd\xde\x42\x02\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x64\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x02",
      789);
  *(uint64_t*)0x200002e0 = 0x315;
  *(uint64_t*)0x200002e8 = 0x63e0;
  *(uint64_t*)0x200002f0 = 0x20011520;
  memcpy(
      (void*)0x20011520,
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x46\x49\x4c\x45"
      "\x30\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0a\x00\x01\x00\x38\x00"
      "\x01\x00\x98\x01\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x04\x00\x00\x00\x0a\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x10\x00"
      "\x00\x00\x60\x00\x00\x00\x00\x00\x18\x00\x00\x00\x00\x00\x48\x00\x00\x00"
      "\x18\x00\x00\x00\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4"
      "\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01"
      "\x06\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x30\x00\x00\x00\x68\x00\x00\x00\x00\x00\x18\x00\x00\x00"
      "\x03\x00\x50\x00\x00\x00\x18\x00\x01\x00\x05\x00\x00\x00\x00\x00\x05\x00"
      "\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\xab"
      "\x2b\xd2\x4e\xc4\xd8\x01\x00\xab\x2b\xd2\x4e\xc4\xd8\x01\x00\x00\x02\x00"
      "\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00"
      "\x00\x00\x07\x03\x24\x00\x55\x00\x70\x00\x43\x00\x61\x00\x73\x00\x65\x00"
      "\x80\x00\x00\x00\x48\x00\x00\x00\x01\x00\x40\x00\x00\x00\x01\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x1f\x00\x00\x00\x00\x00\x00\x00\x40\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00"
      "\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x21\x20\x89",
      355);
  *(uint64_t*)0x200002f8 = 0x163;
  *(uint64_t*)0x20000300 = 0x67e0;
  *(uint64_t*)0x20000308 = 0x2001b200;
  memset((void*)0x2001b200, 16, 1);
  *(uint64_t*)0x20000310 = 1;
  *(uint64_t*)0x20000318 = 0x46080;
  *(uint64_t*)0x20000320 = 0;
  *(uint64_t*)0x20000328 = 0;
  *(uint64_t*)0x20000330 = 0xfffe0;
  syz_mount_image(0x20000000, 0x20000100, 0, 0xd, 0x20000200, 0, 0x20003580, 0);
  return 0;
}
