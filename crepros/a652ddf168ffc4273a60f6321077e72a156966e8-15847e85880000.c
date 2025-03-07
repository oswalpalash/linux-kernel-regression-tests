// https://syzkaller.appspot.com/bug?id=a652ddf168ffc4273a60f6321077e72a156966e8
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>
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

static __thread int clone_ongoing;
static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* ctx)
{
  if (__atomic_load_n(&clone_ongoing, __ATOMIC_RELAXED) != 0) {
    exit(sig);
  }
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  int skip = __atomic_load_n(&skip_segv, __ATOMIC_RELAXED) != 0;
  int valid = addr < prog_start || addr > prog_end;
  if (skip && valid) {
    _longjmp(segv_env, 1);
  }
  exit(sig);
}

static void install_segv_handler(void)
{
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_IGN;
  syscall(SYS_rt_sigaction, 0x20, &sa, NULL, 8);
  syscall(SYS_rt_sigaction, 0x21, &sa, NULL, 8);
  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = segv_handler;
  sa.sa_flags = SA_NODEFER | SA_SIGINFO;
  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGBUS, &sa, NULL);
}

#define NONFAILING(...)                                                        \
  ({                                                                           \
    int ok = 1;                                                                \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    if (_setjmp(segv_env) == 0) {                                              \
      __VA_ARGS__;                                                             \
    } else                                                                     \
      ok = 0;                                                                  \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    ok;                                                                        \
  })

struct fs_image_segment {
  void* data;
  uintptr_t size;
  uintptr_t offset;
};
static int setup_loop_device(long unsigned size, long unsigned nsegs,
                             struct fs_image_segment* segs,
                             const char* loopname, int* memfd_p, int* loopfd_p)
{
  int err = 0, loopfd = -1;
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

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  install_segv_handler();
  intptr_t res = 0;
  NONFAILING(memcpy((void*)0x20000080, "./file0\000", 8));
  NONFAILING(syz_mount_image(0, 0x20000080, 0, 0, 0x20000000, 0, 0, 0));
  res = syscall(__NR_pipe2, 0x20000180ul, 0ul);
  if (res != -1) {
    NONFAILING(r[0] = *(uint32_t*)0x20000180);
    NONFAILING(r[1] = *(uint32_t*)0x20000184);
  }
  NONFAILING(memcpy((void*)0x200004c0, "\x15\x00\x00\x00\x61\xff\xff", 7));
  syscall(__NR_write, r[1], 0x200004c0ul, 0x15ul);
  res = syscall(__NR_dup, r[1]);
  if (res != -1)
    r[2] = res;
  NONFAILING(memcpy((void*)0x20000040, "./file0\000", 8));
  NONFAILING(memcpy((void*)0x200001c0, "9p\000", 3));
  NONFAILING(memcpy((void*)0x20000440, "trans=fd,", 9));
  NONFAILING(memcpy((void*)0x20000449, "rfdno", 5));
  NONFAILING(*(uint8_t*)0x2000044e = 0x3d);
  NONFAILING(sprintf((char*)0x2000044f, "0x%016llx", (long long)r[0]));
  NONFAILING(*(uint8_t*)0x20000461 = 0x2c);
  NONFAILING(memcpy((void*)0x20000462, "wfdno", 5));
  NONFAILING(*(uint8_t*)0x20000467 = 0x3d);
  NONFAILING(sprintf((char*)0x20000468, "0x%016llx", (long long)r[2]));
  NONFAILING(*(uint8_t*)0x2000047a = 0x2c);
  NONFAILING(*(uint8_t*)0x2000047b = 0);
  syscall(__NR_mount, 0ul, 0x20000040ul, 0x200001c0ul, 0ul, 0x20000440ul);
  NONFAILING(*(uint32_t*)0x20000100 = 0x18);
  NONFAILING(*(uint32_t*)0x20000104 = 0);
  NONFAILING(*(uint64_t*)0x20000108 = 0);
  NONFAILING(*(uint64_t*)0x20000110 = 0);
  syscall(__NR_write, r[2], 0x20000100ul, 0x18ul);
  NONFAILING(memcpy((void*)0x20000240,
                    "\xb0\x00\x00\x00\x00\x00\x00\x6b\x2e\x7f\xb3\xf3\x73\x25"
                    "\x10\x28\xe4\x79\x55\xa6\x04\xc6\x09\x00\x00\x00\xa7\xa3"
                    "\x5e\x95\x0c\x87\x04\x66\x22\xf3\x47\x49",
                    38));
  syscall(__NR_write, r[2], 0x20000240ul, 0xb0ul);
  NONFAILING(*(uint32_t*)0x200000c0 = 0x14c);
  NONFAILING(*(uint32_t*)0x200000c4 = 5);
  NONFAILING(*(uint64_t*)0x200000c8 = 0);
  NONFAILING(*(uint64_t*)0x200000d0 = 0);
  NONFAILING(*(uint64_t*)0x200000d8 = 0);
  NONFAILING(*(uint64_t*)0x200000e0 = 0);
  NONFAILING(*(uint32_t*)0x200000e8 = 0);
  NONFAILING(*(uint32_t*)0x200000ec = 0);
  syscall(__NR_write, r[2], 0x200000c0ul, 0x137ul);
  NONFAILING(memcpy((void*)0x20000200, "./file0\000", 8));
  NONFAILING(memcpy((void*)0x20000140, "9p\000", 3));
  NONFAILING(memcpy((void*)0x20000340, "trans=fd,", 9));
  NONFAILING(memcpy((void*)0x20000349, "rfdno", 5));
  NONFAILING(*(uint8_t*)0x2000034e = 0x3d);
  NONFAILING(sprintf((char*)0x2000034f, "0x%016llx", (long long)r[0]));
  NONFAILING(*(uint8_t*)0x20000361 = 0x2c);
  NONFAILING(memcpy((void*)0x20000362, "wfdno", 5));
  NONFAILING(*(uint8_t*)0x20000367 = 0x3d);
  NONFAILING(sprintf((char*)0x20000368, "0x%016llx", (long long)r[1]));
  NONFAILING(*(uint8_t*)0x2000037a = 0x2c);
  NONFAILING(*(uint8_t*)0x2000037b = 0);
  syscall(__NR_mount, 0ul, 0x20000200ul, 0x20000140ul, 0ul, 0x20000340ul);
  NONFAILING(memcpy((void*)0x200002c0, "./file0/file0\000", 14));
  syscall(__NR_mkdir, 0x200002c0ul, 0ul);
  return 0;
}
