// https://syzkaller.appspot.com/bug?id=e925f35f91b20fb565507134085b4d65824e5eb0
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/futex.h>
#include <linux/loop.h>
#include <pthread.h>
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

struct thread_t {
  int created, running, call;
  pthread_t th;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;
static int collide;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    while (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE))
      syscall(SYS_futex, &th->running, FUTEX_WAIT, 0, 0);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    __atomic_store_n(&th->running, 0, __ATOMIC_RELEASE);
    syscall(SYS_futex, &th->running, FUTEX_WAKE);
  }
  return 0;
}

static void execute(int num_calls)
{
  int call, thread;
  running = 0;
  for (call = 0; call < num_calls; call++) {
    for (thread = 0; thread < sizeof(threads) / sizeof(threads[0]); thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 128 << 10);
        pthread_create(&th->th, &attr, thr, th);
      }
      if (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE)) {
        th->call = call;
        __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
        __atomic_store_n(&th->running, 1, __ATOMIC_RELEASE);
        syscall(SYS_futex, &th->running, FUTEX_WAKE);
        if (collide && call % 2)
          break;
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 20 * 1000 * 1000;
        syscall(SYS_futex, &th->running, FUTEX_WAIT, 1, &ts);
        if (running)
          usleep((call == num_calls - 1) ? 10000 : 1000);
        break;
      }
    }
  }
}

uint64_t r[1] = {0xffffffffffffffff};
unsigned long long procid;
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    memcpy((void*)0x20000080, "./file0", 8);
    res = syscall(__NR_creat, 0x20000080, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    memcpy((void*)0x20000780, "jfs", 4);
    memcpy((void*)0x20000300, "./file0", 8);
    *(uint64_t*)0x20000700 = 0x20000200;
    *(uint64_t*)0x20000708 = 0;
    *(uint64_t*)0x20000710 = 0x7fff;
    *(uint64_t*)0x20000718 = 0x20000340;
    *(uint64_t*)0x20000720 = 0;
    *(uint64_t*)0x20000728 = 1;
    *(uint64_t*)0x20000730 = 0x20000440;
    *(uint64_t*)0x20000738 = 0;
    *(uint64_t*)0x20000740 = 0;
    *(uint64_t*)0x20000748 = 0x200004c0;
    *(uint64_t*)0x20000750 = 0;
    *(uint64_t*)0x20000758 = 0x3ff;
    *(uint64_t*)0x20000760 = 0x20000580;
    *(uint64_t*)0x20000768 = 0;
    *(uint64_t*)0x20000770 = 7;
    *(uint64_t*)0x20000778 = 0x20000280;
    *(uint64_t*)0x20000780 = 0;
    *(uint64_t*)0x20000788 = 2;
    *(uint64_t*)0x20000790 = 0x20000640;
    *(uint64_t*)0x20000798 = 0;
    *(uint64_t*)0x200007a0 = 0xfffffffffffffff9;
    *(uint64_t*)0x200007a8 = 0x20000680;
    *(uint64_t*)0x200007b0 = 0;
    *(uint64_t*)0x200007b8 = 0xca;
    syz_mount_image(0x20000780, 0x20000300, 0x80000, 8, 0x20000700, 0x1000,
                    0x20000ac0);
    break;
  case 2:
    memcpy((void*)0x20000d00, "./file0", 8);
    syscall(__NR_chown, 0x20000d00, 0, 0);
    break;
  case 3:
    memcpy((void*)0x20712ff8, "./file0", 8);
    memcpy((void*)0x20faffe7, "security.capability", 20);
    memcpy((void*)0x20000100, "\x00\x00\x00\x02\x01\x00\x00\x00\x00\x00\x00\x01"
                              "\x04\x00\x00\x00\x00\x00\x00\x00",
           20);
    syscall(__NR_lsetxattr, 0x20712ff8, 0x20faffe7, 0x20000100, 0x14, 0);
    break;
  case 4:
    *(uint8_t*)0x20000b80 = 0x38;
    *(uint8_t*)0x20000b81 = 0x37;
    *(uint8_t*)0x20000b82 = 0x39;
    syscall(__NR_write, r[0], 0x20000b80, 3);
    break;
  }
}

void loop()
{
  execute(5);
  collide = 1;
  execute(5);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
