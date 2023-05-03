// https://syzkaller.appspot.com/bug?id=4c4af2385654d032d6480051e771ade1732f2ecc
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

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);

  memcpy((void*)0x20000000, "f2fs\000", 5);
  memcpy((void*)0x20000100, "./file0\000", 8);
  *(uint64_t*)0x20000200 = 0x20010000;
  memcpy((void*)0x20010000,
         "\x10\x20\xf5\xf2\x01\x00\x0b\x00\x09\x00\x00\x00\x03\x00\x00\x00\x0c"
         "\x00\x00\x00\x09\x00\x00\x00\x02\x00\x00\x00\x01\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x40\x00\x00\x00\x00\x00\x00\x0b\x00\x00\x00\x1e\x00\x00"
         "\x00\x02\x00\x00\x00\x02\x00\x00\x00\x02\x00\x00\x00\x02\x00\x00\x00"
         "\x16\x00\x00\x00\x00\x04\x00\x00\x00\x04\x00\x00\x00\x08\x00\x00\x00"
         "\x0c\x00\x00\x00\x10\x00\x00\x00\x14\x00\x00\x03\x00\x00\x00\x01\x00"
         "\x00\x00\x02\x00\x00\x00\x1e\x10\x66\x1c\xcb\xa8\x46\x61\x9d\x3d\x50"
         "\x9c\x42\xfd\x7a\xa1\x00\x00\x00\x00",
         128);
  *(uint64_t*)0x20000208 = 0x80;
  *(uint64_t*)0x20000210 = 0x400;
  *(uint64_t*)0x20000218 = 0;
  *(uint64_t*)0x20000220 = 0;
  *(uint64_t*)0x20000228 = 0x860;
  *(uint64_t*)0x20000230 = 0;
  *(uint64_t*)0x20000238 = 0;
  *(uint64_t*)0x20000240 = 0xa80;
  *(uint64_t*)0x20000248 = 0;
  *(uint64_t*)0x20000250 = 0;
  *(uint64_t*)0x20000258 = 0xb80;
  *(uint64_t*)0x20000260 = 0;
  *(uint64_t*)0x20000268 = 0;
  *(uint64_t*)0x20000270 = 0xc80;
  *(uint64_t*)0x20000278 = 0;
  *(uint64_t*)0x20000280 = 0;
  *(uint64_t*)0x20000288 = 0xec0;
  *(uint64_t*)0x20000290 = 0;
  *(uint64_t*)0x20000298 = 0;
  *(uint64_t*)0x200002a0 = 0x1400;
  *(uint64_t*)0x200002a8 = 0;
  *(uint64_t*)0x200002b0 = 0;
  *(uint64_t*)0x200002b8 = 0x1860;
  *(uint64_t*)0x200002c0 = 0;
  *(uint64_t*)0x200002c8 = 0;
  *(uint64_t*)0x200002d0 = 0x1a80;
  *(uint64_t*)0x200002d8 = 0;
  *(uint64_t*)0x200002e0 = 0;
  *(uint64_t*)0x200002e8 = 0x1b80;
  *(uint64_t*)0x200002f0 = 0;
  *(uint64_t*)0x200002f8 = 0;
  *(uint64_t*)0x20000300 = 0x1c80;
  *(uint64_t*)0x20000308 = 0;
  *(uint64_t*)0x20000310 = 0;
  *(uint64_t*)0x20000318 = 0x1ec0;
  *(uint64_t*)0x20000320 = 0x20010e00;
  memcpy(
      (void*)0x20010e00,
      "\x71\xf4\xab\x0c\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x0b\x00"
      "\x00\x00\x00\x00\x00\x00\x15\x00\x00\x00\x15\x00\x00\x00\x10\x00\x00\x00"
      "\x06\x00\x00\x00\x08\x00\x00\x00\x0a\x00\x00\x00\xff\xff\xff\xff\xff\xff"
      "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x03\x00\x06\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00"
      "\x00\x00\x04\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
      "\xff\xff\xff\xff\xff\xff\xff\xff\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\xc5\x00\x00\x00\x06\x00\x00\x00\x01\x00\x00\x00"
      "\x07\x00\x00\x00\x07\x00\x00\x00\x0a\x00\x00\x00\x40\x00\x00\x00\x40\x00"
      "\x00\x00\xfc\x0f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
      192);
  *(uint64_t*)0x20000328 = 0xc0;
  *(uint64_t*)0x20000330 = 0x400000;
  *(uint64_t*)0x20000338 = 0x20010f00;
  memcpy((void*)0x20010f00,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xeb\xeb\xde\xb8\x00\x00"
         "\x03\x00\x00\x00\x00\x03\x00\x00\x00\x00\x20\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         64);
  *(uint64_t*)0x20000340 = 0x40;
  *(uint64_t*)0x20000348 = 0x400fe0;
  *(uint64_t*)0x20000350 = 0x20011000;
  memcpy((void*)0x20011000,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x04"
         "\x00\x78\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         64);
  *(uint64_t*)0x20000358 = 0x40;
  *(uint64_t*)0x20000360 = 0x4011e0;
  *(uint64_t*)0x20000368 = 0;
  *(uint64_t*)0x20000370 = 0;
  *(uint64_t*)0x20000378 = 0x401240;
  *(uint64_t*)0x20000380 = 0;
  *(uint64_t*)0x20000388 = 0;
  *(uint64_t*)0x20000390 = 0x401280;
  *(uint64_t*)0x20000398 = 0x20011300;
  memcpy((void*)0x20011300,
         "\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x02\x0c\x60\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint64_t*)0x200003a0 = 0x20;
  *(uint64_t*)0x200003a8 = 0x4012e0;
  *(uint64_t*)0x200003b0 = 0x20011400;
  memcpy((void*)0x20011400,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x08\x00\x00\x00\x05\x10\xec\x00\x00\x00\x00",
         32);
  *(uint64_t*)0x200003b8 = 0x20;
  *(uint64_t*)0x200003c0 = 0x401320;
  *(uint64_t*)0x200003c8 = 0;
  *(uint64_t*)0x200003d0 = 0;
  *(uint64_t*)0x200003d8 = 0x401380;
  *(uint64_t*)0x200003e0 = 0;
  *(uint64_t*)0x200003e8 = 0;
  *(uint64_t*)0x200003f0 = 0x4013e0;
  *(uint64_t*)0x200003f8 = 0;
  *(uint64_t*)0x20000400 = 0;
  *(uint64_t*)0x20000408 = 0x402000;
  *(uint64_t*)0x20000410 = 0;
  *(uint64_t*)0x20000418 = 0;
  *(uint64_t*)0x20000420 = 0x402e00;
  *(uint64_t*)0x20000428 = 0;
  *(uint64_t*)0x20000430 = 0;
  *(uint64_t*)0x20000438 = 0x402fe0;
  *(uint64_t*)0x20000440 = 0;
  *(uint64_t*)0x20000448 = 0;
  *(uint64_t*)0x20000450 = 0x403fe0;
  *(uint64_t*)0x20000458 = 0x20011b00;
  memcpy(
      (void*)0x20011b00,
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x71\xf4\xab\x0c"
      "\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x0b\x00\x00\x00\x00\x00"
      "\x00\x00\x15\x00\x00\x00\x15\x00\x00\x00\x10\x00\x00\x00\x06\x00\x00\x00"
      "\x08\x00\x00\x00\x0a\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
      "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x03\x00\x06\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x04\x00"
      "\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
      "\xff\xff\xff\xff\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\xc5\x00\x00\x00\x06\x00\x00\x00\x01\x00\x00\x00\x07\x00\x00\x00"
      "\x07\x00\x00\x00\x0a\x00\x00\x00\x40\x00\x00\x00\x40\x00\x00\x00\xfc\x0f"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00",
      224);
  *(uint64_t*)0x20000460 = 0xe0;
  *(uint64_t*)0x20000468 = 0x404fe0;
  *(uint64_t*)0x20000470 = 0x20011c00;
  memcpy((void*)0x20011c00,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xeb\xeb\xde\xb8",
         32);
  *(uint64_t*)0x20000478 = 0x20;
  *(uint64_t*)0x20000480 = 0x405fe0;
  *(uint64_t*)0x20000488 = 0;
  *(uint64_t*)0x20000490 = 0;
  *(uint64_t*)0x20000498 = 0x5ff000;
  *(uint64_t*)0x200004a0 = 0;
  *(uint64_t*)0x200004a8 = 0;
  *(uint64_t*)0x200004b0 = 0x5ff040;
  *(uint64_t*)0x200004b8 = 0;
  *(uint64_t*)0x200004c0 = 0;
  *(uint64_t*)0x200004c8 = 0x600000;
  *(uint64_t*)0x200004d0 = 0;
  *(uint64_t*)0x200004d8 = 0;
  *(uint64_t*)0x200004e0 = 0x600fe0;
  *(uint64_t*)0x200004e8 = 0;
  *(uint64_t*)0x200004f0 = 0;
  *(uint64_t*)0x200004f8 = 0x6011e0;
  *(uint64_t*)0x20000500 = 0;
  *(uint64_t*)0x20000508 = 0;
  *(uint64_t*)0x20000510 = 0x601240;
  *(uint64_t*)0x20000518 = 0;
  *(uint64_t*)0x20000520 = 0;
  *(uint64_t*)0x20000528 = 0x601280;
  *(uint64_t*)0x20000530 = 0;
  *(uint64_t*)0x20000538 = 0;
  *(uint64_t*)0x20000540 = 0x6012e0;
  *(uint64_t*)0x20000548 = 0;
  *(uint64_t*)0x20000550 = 0;
  *(uint64_t*)0x20000558 = 0x601320;
  *(uint64_t*)0x20000560 = 0;
  *(uint64_t*)0x20000568 = 0;
  *(uint64_t*)0x20000570 = 0x601380;
  *(uint64_t*)0x20000578 = 0;
  *(uint64_t*)0x20000580 = 0;
  *(uint64_t*)0x20000588 = 0x6013e0;
  *(uint64_t*)0x20000590 = 0;
  *(uint64_t*)0x20000598 = 0;
  *(uint64_t*)0x200005a0 = 0x602000;
  *(uint64_t*)0x200005a8 = 0;
  *(uint64_t*)0x200005b0 = 0;
  *(uint64_t*)0x200005b8 = 0x602e00;
  *(uint64_t*)0x200005c0 = 0;
  *(uint64_t*)0x200005c8 = 0;
  *(uint64_t*)0x200005d0 = 0x602fe0;
  *(uint64_t*)0x200005d8 = 0;
  *(uint64_t*)0x200005e0 = 0;
  *(uint64_t*)0x200005e8 = 0x603fe0;
  *(uint64_t*)0x200005f0 = 0;
  *(uint64_t*)0x200005f8 = 0;
  *(uint64_t*)0x20000600 = 0x604fe0;
  *(uint64_t*)0x20000608 = 0;
  *(uint64_t*)0x20000610 = 0;
  *(uint64_t*)0x20000618 = 0x605fe0;
  *(uint64_t*)0x20000620 = 0;
  *(uint64_t*)0x20000628 = 0;
  *(uint64_t*)0x20000630 = 0x7ff000;
  *(uint64_t*)0x20000638 = 0;
  *(uint64_t*)0x20000640 = 0;
  *(uint64_t*)0x20000648 = 0x7ff040;
  *(uint64_t*)0x20000650 = 0x20013000;
  memcpy((void*)0x20013000,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00"
         "\x00\x00\x02\x00\x00\x00\x01\x00\x00\x00\x00\x03\x00\x00\x00\x01",
         33);
  *(uint64_t*)0x20000658 = 0x21;
  *(uint64_t*)0x20000660 = 0xc00000;
  *(uint64_t*)0x20000668 = 0;
  *(uint64_t*)0x20000670 = 0;
  *(uint64_t*)0x20000678 = 0x2000fe0;
  *(uint64_t*)0x20000680 = 0;
  *(uint64_t*)0x20000688 = 0;
  *(uint64_t*)0x20000690 = 0x2001fe0;
  *(uint8_t*)0x20015000 = 0;
  syz_mount_image(0x20000000, 0x20000100, 0, 0x31, 0x20000200, 0, 0x20015000);
  return 0;
}
