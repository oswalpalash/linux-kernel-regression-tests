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

#define BITMASK_LEN(type, bf_len) (type)((1ull << (bf_len)) - 1)

#define BITMASK_LEN_OFF(type, bf_off, bf_len)                                  \
  (type)(BITMASK_LEN(type, (bf_len)) << (bf_off))

#define STORE_BY_BITMASK(type, addr, val, bf_off, bf_len)                      \
  if ((bf_off) == 0 && (bf_len) == 0) {                                        \
    *(type*)(addr) = (type)(val);                                              \
  } else {                                                                     \
    type new_val = *(type*)(addr);                                             \
    new_val &= ~BITMASK_LEN_OFF(type, (bf_off), (bf_len));                     \
    new_val |= ((type)(val)&BITMASK_LEN(type, (bf_len))) << (bf_off);          \
    *(type*)(addr) = new_val;                                                  \
  }

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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
unsigned long long procid;
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    *(uint32_t*)0x2025c000 = 2;
    *(uint32_t*)0x2025c004 = 0x70;
    *(uint8_t*)0x2025c008 = 0xe3;
    *(uint8_t*)0x2025c009 = 0;
    *(uint8_t*)0x2025c00a = 0;
    *(uint8_t*)0x2025c00b = 0;
    *(uint32_t*)0x2025c00c = 0;
    *(uint64_t*)0x2025c010 = 0;
    *(uint64_t*)0x2025c018 = 0;
    *(uint64_t*)0x2025c020 = 0;
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 0, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 1, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 2, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 3, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 4, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 5, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 6, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 7, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 8, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 9, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 10, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 11, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 12, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 13, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 14, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 15, 2);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 17, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 18, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 19, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 20, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 21, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 22, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 23, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 24, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 25, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 26, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 27, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 28, 1);
    STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 29, 35);
    *(uint32_t*)0x2025c030 = 0;
    *(uint32_t*)0x2025c034 = 0;
    *(uint64_t*)0x2025c038 = 0x20000000;
    *(uint64_t*)0x2025c040 = 0;
    *(uint64_t*)0x2025c048 = 0;
    *(uint64_t*)0x2025c050 = 0;
    *(uint32_t*)0x2025c058 = 0;
    *(uint32_t*)0x2025c05c = 0;
    *(uint64_t*)0x2025c060 = 0;
    *(uint32_t*)0x2025c068 = 0;
    *(uint16_t*)0x2025c06c = 0;
    *(uint16_t*)0x2025c06e = 0;
    res = syscall(__NR_perf_event_open, 0x2025c000, 0, 0, -1, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    syscall(__NR_ioctl, r[0], 0x4c06, r[0]);
    break;
  case 2:
    memcpy((void*)0x20000280, "./file0", 8);
    res = syscall(__NR_creat, 0x20000280, 0);
    if (res != -1)
      r[1] = res;
    break;
  case 3:
    *(uint32_t*)0x20000400 = 0x50;
    *(uint32_t*)0x20000404 = 1;
    *(uint64_t*)0x20000408 = 9;
    *(uint32_t*)0x20000410 = 7;
    *(uint32_t*)0x20000414 = 0x1a;
    *(uint32_t*)0x20000418 = 0xc75;
    *(uint32_t*)0x2000041c = 1;
    *(uint16_t*)0x20000420 = -1;
    *(uint16_t*)0x20000422 = 0xbe0d;
    *(uint32_t*)0x20000424 = 6;
    *(uint32_t*)0x20000428 = 7;
    *(uint32_t*)0x2000042c = 0;
    *(uint32_t*)0x20000430 = 0;
    *(uint32_t*)0x20000434 = 0;
    *(uint32_t*)0x20000438 = 0;
    *(uint32_t*)0x2000043c = 0;
    *(uint32_t*)0x20000440 = 0;
    *(uint32_t*)0x20000444 = 0;
    *(uint32_t*)0x20000448 = 0;
    *(uint32_t*)0x2000044c = 0;
    syscall(__NR_write, r[1], 0x20000400, 0x50);
    break;
  case 4:
    memcpy((void*)0x20000200, "msdos", 6);
    memcpy((void*)0x20000240, "./file0", 8);
    *(uint64_t*)0x20000380 = 0x200002c0;
    *(uint64_t*)0x20000388 = 0;
    *(uint64_t*)0x20000390 = 0x8000;
    memcpy((void*)0x200003c0, "errors=continue", 15);
    *(uint8_t*)0x200003cf = 0x2c;
    memcpy((void*)0x200003d0, "dos1xfloppy", 11);
    *(uint8_t*)0x200003db = 0x2c;
    memcpy((void*)0x200003dc, "umask", 5);
    *(uint8_t*)0x200003e1 = 0x3d;
    *(uint8_t*)0x200003e2 = 0x32;
    *(uint8_t*)0x200003e3 = 0x30;
    *(uint8_t*)0x200003e4 = 0x35;
    *(uint8_t*)0x200003e5 = 0x2c;
    memcpy((void*)0x200003e6, "errors=continue", 15);
    *(uint8_t*)0x200003f5 = 0x2c;
    *(uint8_t*)0x200003f6 = 0;
    syz_mount_image(0x20000200, 0x20000240, 0x7fff, 1, 0x20000380, 0x1000,
                    0x200003c0);
    break;
  case 5:
    memcpy((void*)0x20000040, "./file0", 8);
    memcpy((void*)0x20faffe7, "security.capability", 20);
    memcpy((void*)0x202b2fec, "\x00\x00\x00\x02\x01\x00\x00\x00\x00\x00\x00\x01"
                              "\x04\x00\x00\x00\x00\x00\x00\x00",
           20);
    syscall(__NR_lsetxattr, 0x20000040, 0x20faffe7, 0x202b2fec, 0x14, 0);
    break;
  }
}

void execute_one()
{
  execute(6);
  collide = 1;
  execute(6);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    loop();
  }
}
