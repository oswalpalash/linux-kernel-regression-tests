// https://syzkaller.appspot.com/bug?id=a3d085c934448d325eba613246a150873080b331
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/futex.h>
#include <linux/loop.h>

#ifndef __NR_memfd_create
#define __NR_memfd_create 319
#endif

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

static void thread_start(void* (*fn)(void*), void* arg)
{
  pthread_t th;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 128 << 10);
  int i = 0;
  for (; i < 100; i++) {
    if (pthread_create(&th, &attr, fn, arg) == 0) {
      pthread_attr_destroy(&attr);
      return;
    }
    if (errno == EAGAIN) {
      usleep(50);
      continue;
    }
    break;
  }
  exit(1);
}

typedef struct {
  int state;
} event_t;

static void event_init(event_t* ev)
{
  ev->state = 0;
}

static void event_reset(event_t* ev)
{
  ev->state = 0;
}

static void event_set(event_t* ev)
{
  if (ev->state)
    exit(1);
  __atomic_store_n(&ev->state, 1, __ATOMIC_RELEASE);
  syscall(SYS_futex, &ev->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1000000);
}

static void event_wait(event_t* ev)
{
  while (!__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, 0);
}

static int event_isset(event_t* ev)
{
  return __atomic_load_n(&ev->state, __ATOMIC_ACQUIRE);
}

static int event_timedwait(event_t* ev, uint64_t timeout)
{
  uint64_t start = current_time_ms();
  uint64_t now = start;
  for (;;) {
    uint64_t remain = timeout - (now - start);
    struct timespec ts;
    ts.tv_sec = remain / 1000;
    ts.tv_nsec = (remain % 1000) * 1000 * 1000;
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, &ts);
    if (__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
      return 1;
    now = current_time_ms();
    if (now - start > timeout)
      return 0;
  }
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

//% This code is derived from puff.{c,h}, found in the zlib development. The
//% original files come with the following copyright notice:

//% Copyright (C) 2002-2013 Mark Adler, all rights reserved
//% version 2.3, 21 Jan 2013
//% This software is provided 'as-is', without any express or implied
//% warranty.  In no event will the author be held liable for any damages
//% arising from the use of this software.
//% Permission is granted to anyone to use this software for any purpose,
//% including commercial applications, and to alter it and redistribute it
//% freely, subject to the following restrictions:
//% 1. The origin of this software must not be misrepresented; you must not
//%    claim that you wrote the original software. If you use this software
//%    in a product, an acknowledgment in the product documentation would be
//%    appreciated but is not required.
//% 2. Altered source versions must be plainly marked as such, and must not be
//%    misrepresented as being the original software.
//% 3. This notice may not be removed or altered from any source distribution.
//% Mark Adler    madler@alumni.caltech.edu

//% BEGIN CODE DERIVED FROM puff.{c,h}

#define MAXBITS 15
#define MAXLCODES 286
#define MAXDCODES 30
#define MAXCODES (MAXLCODES + MAXDCODES)
#define FIXLCODES 288

struct puff_state {
  unsigned char* out;
  unsigned long outlen;
  unsigned long outcnt;
  const unsigned char* in;
  unsigned long inlen;
  unsigned long incnt;
  int bitbuf;
  int bitcnt;
  jmp_buf env;
};
static int puff_bits(struct puff_state* s, int need)
{
  long val = s->bitbuf;
  while (s->bitcnt < need) {
    if (s->incnt == s->inlen)
      longjmp(s->env, 1);
    val |= (long)(s->in[s->incnt++]) << s->bitcnt;
    s->bitcnt += 8;
  }
  s->bitbuf = (int)(val >> need);
  s->bitcnt -= need;
  return (int)(val & ((1L << need) - 1));
}
static int puff_stored(struct puff_state* s)
{
  s->bitbuf = 0;
  s->bitcnt = 0;
  if (s->incnt + 4 > s->inlen)
    return 2;
  unsigned len = s->in[s->incnt++];
  len |= s->in[s->incnt++] << 8;
  if (s->in[s->incnt++] != (~len & 0xff) ||
      s->in[s->incnt++] != ((~len >> 8) & 0xff))
    return -2;
  if (s->incnt + len > s->inlen)
    return 2;
  if (s->outcnt + len > s->outlen)
    return 1;
  for (; len--; s->outcnt++, s->incnt++) {
    if (s->in[s->incnt])
      s->out[s->outcnt] = s->in[s->incnt];
  }
  return 0;
}
struct puff_huffman {
  short* count;
  short* symbol;
};
static int puff_decode(struct puff_state* s, const struct puff_huffman* h)
{
  int first = 0;
  int index = 0;
  int bitbuf = s->bitbuf;
  int left = s->bitcnt;
  int code = first = index = 0;
  int len = 1;
  short* next = h->count + 1;
  while (1) {
    while (left--) {
      code |= bitbuf & 1;
      bitbuf >>= 1;
      int count = *next++;
      if (code - count < first) {
        s->bitbuf = bitbuf;
        s->bitcnt = (s->bitcnt - len) & 7;
        return h->symbol[index + (code - first)];
      }
      index += count;
      first += count;
      first <<= 1;
      code <<= 1;
      len++;
    }
    left = (MAXBITS + 1) - len;
    if (left == 0)
      break;
    if (s->incnt == s->inlen)
      longjmp(s->env, 1);
    bitbuf = s->in[s->incnt++];
    if (left > 8)
      left = 8;
  }
  return -10;
}
static int puff_construct(struct puff_huffman* h, const short* length, int n)
{
  int len;
  for (len = 0; len <= MAXBITS; len++)
    h->count[len] = 0;
  int symbol;
  for (symbol = 0; symbol < n; symbol++)
    (h->count[length[symbol]])++;
  if (h->count[0] == n)
    return 0;
  int left = 1;
  for (len = 1; len <= MAXBITS; len++) {
    left <<= 1;
    left -= h->count[len];
    if (left < 0)
      return left;
  }
  short offs[MAXBITS + 1];
  offs[1] = 0;
  for (len = 1; len < MAXBITS; len++)
    offs[len + 1] = offs[len] + h->count[len];
  for (symbol = 0; symbol < n; symbol++)
    if (length[symbol] != 0)
      h->symbol[offs[length[symbol]]++] = symbol;
  return left;
}
static int puff_codes(struct puff_state* s, const struct puff_huffman* lencode,
                      const struct puff_huffman* distcode)
{
  static const short lens[29] = {3,  4,  5,  6,   7,   8,   9,   10,  11, 13,
                                 15, 17, 19, 23,  27,  31,  35,  43,  51, 59,
                                 67, 83, 99, 115, 131, 163, 195, 227, 258};
  static const short lext[29] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2,
                                 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
  static const short dists[30] = {
      1,    2,    3,    4,    5,    7,    9,    13,    17,    25,
      33,   49,   65,   97,   129,  193,  257,  385,   513,   769,
      1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};
  static const short dext[30] = {0, 0, 0,  0,  1,  1,  2,  2,  3,  3,
                                 4, 4, 5,  5,  6,  6,  7,  7,  8,  8,
                                 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};
  int symbol;
  do {
    symbol = puff_decode(s, lencode);
    if (symbol < 0)
      return symbol;
    if (symbol < 256) {
      if (s->outcnt == s->outlen)
        return 1;
      if (symbol)
        s->out[s->outcnt] = symbol;
      s->outcnt++;
    } else if (symbol > 256) {
      symbol -= 257;
      if (symbol >= 29)
        return -10;
      int len = lens[symbol] + puff_bits(s, lext[symbol]);
      symbol = puff_decode(s, distcode);
      if (symbol < 0)
        return symbol;
      unsigned dist = dists[symbol] + puff_bits(s, dext[symbol]);
      if (dist > s->outcnt)
        return -11;
      if (s->outcnt + len > s->outlen)
        return 1;
      while (len--) {
        if (dist <= s->outcnt && s->out[s->outcnt - dist])
          s->out[s->outcnt] = s->out[s->outcnt - dist];
        s->outcnt++;
      }
    }
  } while (symbol != 256);
  return 0;
}
static int puff_fixed(struct puff_state* s)
{
  static int virgin = 1;
  static short lencnt[MAXBITS + 1], lensym[FIXLCODES];
  static short distcnt[MAXBITS + 1], distsym[MAXDCODES];
  static struct puff_huffman lencode, distcode;
  if (virgin) {
    lencode.count = lencnt;
    lencode.symbol = lensym;
    distcode.count = distcnt;
    distcode.symbol = distsym;
    short lengths[FIXLCODES];
    int symbol;
    for (symbol = 0; symbol < 144; symbol++)
      lengths[symbol] = 8;
    for (; symbol < 256; symbol++)
      lengths[symbol] = 9;
    for (; symbol < 280; symbol++)
      lengths[symbol] = 7;
    for (; symbol < FIXLCODES; symbol++)
      lengths[symbol] = 8;
    puff_construct(&lencode, lengths, FIXLCODES);
    for (symbol = 0; symbol < MAXDCODES; symbol++)
      lengths[symbol] = 5;
    puff_construct(&distcode, lengths, MAXDCODES);
    virgin = 0;
  }
  return puff_codes(s, &lencode, &distcode);
}
static int puff_dynamic(struct puff_state* s)
{
  static const short order[19] = {16, 17, 18, 0, 8,  7, 9,  6, 10, 5,
                                  11, 4,  12, 3, 13, 2, 14, 1, 15};
  int nlen = puff_bits(s, 5) + 257;
  int ndist = puff_bits(s, 5) + 1;
  int ncode = puff_bits(s, 4) + 4;
  if (nlen > MAXLCODES || ndist > MAXDCODES)
    return -3;
  short lengths[MAXCODES];
  int index;
  for (index = 0; index < ncode; index++)
    lengths[order[index]] = puff_bits(s, 3);
  for (; index < 19; index++)
    lengths[order[index]] = 0;
  short lencnt[MAXBITS + 1], lensym[MAXLCODES];
  struct puff_huffman lencode = {lencnt, lensym};
  int err = puff_construct(&lencode, lengths, 19);
  if (err != 0)
    return -4;
  index = 0;
  while (index < nlen + ndist) {
    int symbol;
    int len;
    symbol = puff_decode(s, &lencode);
    if (symbol < 0)
      return symbol;
    if (symbol < 16)
      lengths[index++] = symbol;
    else {
      len = 0;
      if (symbol == 16) {
        if (index == 0)
          return -5;
        len = lengths[index - 1];
        symbol = 3 + puff_bits(s, 2);
      } else if (symbol == 17)
        symbol = 3 + puff_bits(s, 3);
      else
        symbol = 11 + puff_bits(s, 7);
      if (index + symbol > nlen + ndist)
        return -6;
      while (symbol--)
        lengths[index++] = len;
    }
  }
  if (lengths[256] == 0)
    return -9;
  err = puff_construct(&lencode, lengths, nlen);
  if (err && (err < 0 || nlen != lencode.count[0] + lencode.count[1]))
    return -7;
  short distcnt[MAXBITS + 1], distsym[MAXDCODES];
  struct puff_huffman distcode = {distcnt, distsym};
  err = puff_construct(&distcode, lengths + nlen, ndist);
  if (err && (err < 0 || ndist != distcode.count[0] + distcode.count[1]))
    return -8;
  return puff_codes(s, &lencode, &distcode);
}
static int puff(unsigned char* dest, unsigned long* destlen,
                const unsigned char* source, unsigned long sourcelen)
{
  struct puff_state s = {
      .out = dest,
      .outlen = *destlen,
      .outcnt = 0,
      .in = source,
      .inlen = sourcelen,
      .incnt = 0,
      .bitbuf = 0,
      .bitcnt = 0,
  };
  int err;
  if (setjmp(s.env) != 0)
    err = 2;
  else {
    int last;
    do {
      last = puff_bits(&s, 1);
      int type = puff_bits(&s, 2);
      err = type == 0 ? puff_stored(&s)
                      : (type == 1 ? puff_fixed(&s)
                                   : (type == 2 ? puff_dynamic(&s) : -1));
      if (err != 0)
        break;
    } while (!last);
  }
  *destlen = s.outcnt;
  return err;
}

//% END CODE DERIVED FROM puff.{c,h}

#define ZLIB_HEADER_WIDTH 2

static int puff_zlib_to_file(const unsigned char* source,
                             unsigned long sourcelen, int dest_fd)
{
  if (sourcelen < ZLIB_HEADER_WIDTH)
    return 0;
  source += ZLIB_HEADER_WIDTH;
  sourcelen -= ZLIB_HEADER_WIDTH;
  const unsigned long max_destlen = 132 << 20;
  void* ret = mmap(0, max_destlen, PROT_WRITE | PROT_READ,
                   MAP_PRIVATE | MAP_ANON, -1, 0);
  if (ret == MAP_FAILED)
    return -1;
  unsigned char* dest = (unsigned char*)ret;
  unsigned long destlen = max_destlen;
  int err = puff(dest, &destlen, source, sourcelen);
  if (err) {
    munmap(dest, max_destlen);
    errno = -err;
    return -1;
  }
  if (write(dest_fd, dest, destlen) != (ssize_t)destlen) {
    munmap(dest, max_destlen);
    return -1;
  }
  return munmap(dest, destlen);
}

static int setup_loop_device(unsigned char* data, unsigned long size,
                             const char* loopname, int* loopfd_p)
{
  int err = 0, loopfd = -1;
  int memfd = syscall(__NR_memfd_create, "syzkaller", 0);
  if (memfd == -1) {
    err = errno;
    goto error;
  }
  if (puff_zlib_to_file(data, size, memfd)) {
    err = errno;
    goto error_close_memfd;
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
  close(memfd);
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
                            volatile long flags, volatile long optsarg,
                            volatile long change_dir,
                            volatile unsigned long size, volatile long image)
{
  unsigned char* data = (unsigned char*)image;
  int res = -1, err = 0, loopfd = -1, need_loop_device = !!size;
  char* mount_opts = (char*)optsarg;
  char* target = (char*)dir;
  char* fs = (char*)fsarg;
  char* source = NULL;
  char loopname[64];
  if (need_loop_device) {
    memset(loopname, 0, sizeof(loopname));
    snprintf(loopname, sizeof(loopname), "/dev/loop%llu", procid);
    if (setup_loop_device(data, size, loopname, &loopfd) == -1)
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
    bool has_remount_ro = false;
    char* remount_ro_start = strstr(opts, "errors=remount-ro");
    if (remount_ro_start != NULL) {
      char after = *(remount_ro_start + strlen("errors=remount-ro"));
      char before = remount_ro_start == opts ? '\0' : *(remount_ro_start - 1);
      has_remount_ro = ((before == '\0' || before == ',') &&
                        (after == '\0' || after == ','));
    }
    if (strstr(opts, "errors=panic") || !has_remount_ro)
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

struct thread_t {
  int created, call;
  event_t ready, done;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    event_wait(&th->ready);
    event_reset(&th->ready);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    event_set(&th->done);
  }
  return 0;
}

static void execute_one(void)
{
  int i, call, thread;
  for (call = 0; call < 9; call++) {
    for (thread = 0; thread < (int)(sizeof(threads) / sizeof(threads[0]));
         thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        event_init(&th->ready);
        event_init(&th->done);
        event_set(&th->done);
        thread_start(thr, th);
      }
      if (!event_isset(&th->done))
        continue;
      event_reset(&th->done);
      th->call = call;
      __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
      event_set(&th->ready);
      event_timedwait(&th->done, 50 + (call == 0 ? 4000 : 0));
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
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
      if (current_time_ms() - start < 5000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    memcpy((void*)0x20000ec0, "nilfs2\000", 7);
    memcpy((void*)0x200000c0, "./file1\000", 8);
    *(uint8_t*)0x20000040 = 0;
    *(uint8_t*)0x20000041 = 0;
    *(uint8_t*)0x20000042 = -1;
    *(uint8_t*)0x20000043 = -1;
    sprintf((char*)0x20000044, "%020llu", (long long)-1);
    sprintf((char*)0x20000058, "%020llu", (long long)0);
    *(uint32_t*)0x2000006c = 0;
    sprintf((char*)0x20000070, "%020llu", (long long)0);
    sprintf((char*)0x20000084, "0x%016llx", (long long)-1);
    memcpy(
        (void*)0x20001e40,
        "\x78\x9c\xec\xdd\x4f\x6c\x1c\x57\x19\x00\xf0\x37\xeb\xbf\x89\xdd\x78"
        "\xdd\x16\x9a\xb6\x34\x09\x2d\x25\x6d\x00\x27\x75\x72\x28\xb7\x54\x8a"
        "\x40\xaa\xaa\xaa\x17\xee\xad\xd2\xa4\x8d\x70\x4b\x20\xe5\xd0\xaa\x51"
        "\x5c\x4e\xa9\xc4\xa1\xa8\x2a\x87\x22\x0e\x45\xed\x0d\x29\x1c\x90\x68"
        "\x85\x84\x2a\x24\x24\xfe\xf4\xc0\x15\x0e\xa8\x82\x0b\x08\x05\x29\x52"
        "\x25\x44\xa4\xd8\xc8\xf6\x7b\xeb\xf5\xb3\x27\xbb\x1e\xaf\x67\x6d\xef"
        "\xef\x27\x7d\x7e\xfb\xe6\xcd\xce\xf7\x8d\xd7\x71\xe6\xad\x77\xdf\x06"
        "\x60\x60\x35\x96\xbf\x9e\x3a\x75\xb0\x08\xe1\x9d\x8f\xde\x3e\xf3\xcd"
        "\x2b\x0b\xbf\x5a\xda\x76\xb8\xb5\xc7\x91\xe5\xaf\x45\xec\x35\x43\x08"
        "\x23\x6d\xfd\x22\x3b\xde\xa7\x71\xc3\xad\x1b\xaf\x9f\x5d\x6a\x17\xb2"
        "\x36\x84\xd9\xa5\x7b\x0d\xa7\xf1\xf0\xf4\xf5\xd6\x7d\x27\x42\x08\xf3"
        "\xe1\x48\xf8\x38\x34\xc3\xe1\x63\xcd\xcf\xae\x0e\x3d\x75\xfe\x83\x77"
        "\x3f\x39\x7a\xf9\xe2\x93\x2f\x6e\xcb\xc9\x03\x00\xc0\x80\xb9\xf6\xc7"
        "\xb9\xbf\x3e\xfa\xcf\x3f\x7c\x75\xfa\xe6\xb5\x43\xa7\xc3\x58\x6b\x7b"
        "\xba\x3e\x6f\xc6\xfe\x44\xbc\xee\x3f\x11\xaf\xef\xd3\x75\x7f\x23\xac"
        "\xed\x17\x6d\xd1\x6e\x34\xdb\x6f\x28\x46\x23\xdb\x6f\x28\xdb\x6f\x38"
        "\xcb\x33\x5c\x92\x6f\x24\x3b\xce\x48\xc9\x7e\xa3\x1d\xf2\x0d\xb5\x6d"
        "\xdb\xe8\x3c\x01\x00\x00\x60\x37\x4a\xf3\xda\x66\x28\x1a\x33\x6b\xfa"
        "\x8d\xc6\xcc\xcc\xca\xbc\x7f\xc9\xa7\x53\xa3\xc5\xcc\xcb\x17\xe6\xce"
        "\x5f\xea\x53\xa1\x00\x00\x00\x40\x65\x9f\x5d\x59\x7e\xd1\xad\xd8\x15"
        "\x31\xbe\x03\x6a\x10\x42\x08\x21\x84\x10\x42\x08\xb1\x1b\x63\x71\xaa"
        "\xdf\xcf\x40\x00\x00\x00\x00\x83\x26\xad\x3b\xd0\x5a\x1f\x2c\x37\x9f"
        "\xaf\x2c\xb0\x35\xad\xa3\x35\xbb\xcb\x7f\xfd\x89\xc6\xc6\xf7\x87\x1e"
        "\xe8\xf4\xf3\xb7\xdd\x3f\x6f\x75\xff\xfb\x93\x7f\x73\xf9\xdf\x7f\xc3"
        "\x6f\x1c\x00\x00\xaa\xdb\xab\x57\x93\xe9\xbc\xd2\x75\xf4\xc4\x9b\xfb"
        "\x96\xfb\xf9\x3a\x82\x43\xd9\xfd\x36\x7b\xfd\xdf\xc8\x8e\x33\xbc\xc9"
        "\x3a\xcb\xd6\x15\xdc\x2d\xeb\x0d\x96\xd5\x99\x7f\x5f\x77\xaa\xb2\xfa"
        "\x37\xfb\x38\xf6\x4b\x59\xfd\xf9\x7a\x98\x3b\x55\x59\xfd\xf9\x3a\x9d"
        "\x3b\x55\x59\xfd\x63\x35\xd7\x51\x55\x59\xfd\xe3\xbb\xe4\x77\x50\x59"
        "\x8d\xfb\x6a\xae\xa3\xaa\xb2\xfa\xf7\xd7\x5c\x47\x55\x65\xf5\x4f\xd4"
        "\x5c\x47\x55\x65\xf5\x4f\xd6\x5c\x47\x55\x65\xf5\xdf\x51\x73\x1d\x55"
        "\x95\xd5\x7f\xa0\xe6\x3a\xaa\x2a\xab\x7f\xb7\xbc\xac\xb6\xac\xfe\x66"
        "\xcd\x75\x54\x55\x56\xff\x74\xcd\x75\x54\x55\x56\xff\x9d\x35\xd7\x51"
        "\x55\x59\xfd\x77\xd5\x5c\x47\x55\x65\xf5\xdf\x5d\x73\x1d\xfd\xf2\x40"
        "\x6c\xd3\xf7\xe1\x50\x36\xde\x9a\x3f\x6f\x30\xa7\xdb\x2d\x73\x3c\x00"
        "\x00\x00\x18\x74\xff\xb3\xfe\x9f\xe8\x61\x1c\xde\x01\x35\x08\x21\xb6"
        "\x1c\x43\x3b\xa0\x06\x21\x84\x10\x42\x08\xd1\xe3\xb8\xd2\xef\x27\x20"
        "\x00\x00\x00\x80\xbe\x4b\xef\x0b\x48\xef\x7a\x5f\x8c\xd2\xf8\x50\x87"
        "\xf1\xe1\x0e\xe3\x23\xb7\x1d\x1f\x6a\xbd\x0f\xb8\xec\xfe\x63\x1d\xc6"
        "\xd7\xfb\x6f\xc7\x3d\x00\x00\x00\x60\xaf\xf9\xf5\xd5\xf3\xf7\xbe\x55"
        "\xac\xbe\xcf\x7f\xab\xeb\xe1\x8d\xc7\x36\xad\xbf\xb4\xd9\x75\x8c\xf2"
        "\xf5\x08\x37\x9b\x7f\xab\xeb\x9e\x6d\x35\xff\x6e\x59\xb7\x0c\x00\x00"
        "\x80\xc1\x52\x7c\xe3\xe3\x85\x63\x67\xde\x7b\x65\xfa\xe6\xb5\x43\xa7"
        "\xdb\x66\xbf\x0b\x71\xbe\x9b\xd6\x01\x1d\x8e\xcf\x0d\x7c\x18\xfb\xe9"
        "\x75\x01\x93\x59\xbf\x48\x73\xe8\xd3\x6b\xf3\x34\x4a\xf6\xcb\x9f\x1f"
        "\xb8\xa3\xec\x78\xcf\x6c\xf1\x44\x01\x00\x00\x60\x80\xa5\xf9\x7b\x33"
        "\x14\x8d\x99\xb6\x79\x77\x33\x34\x1a\x33\x33\xab\xf3\xf1\x83\x61\xa4"
        "\x38\x7f\x61\xee\xdc\x89\xd8\x4f\x9f\xcf\xf2\xfb\xa9\xf1\x10\xc2\xdc"
        "\xb9\xc7\x6a\xae\x1b\x00\x00\x00\xe8\xde\xea\x7c\xbf\xd8\x17\x36\x98"
        "\xff\xa7\xcf\xf1\x3d\x18\x46\x8b\x99\x97\x2f\xcc\x9d\xbf\xb4\xd2\x9f"
        "\x6c\x6d\x1f\x69\xb4\x3f\x2f\x30\xb5\xba\x7d\xf9\xf9\x82\xc7\x5a\xc7"
        "\x5b\xbb\x7d\xb6\x64\xfb\xc9\xd8\x4f\x9f\xdf\xf9\xe2\xd4\xbe\xe5\xed"
        "\x33\x67\xbf\x33\xf7\x7c\x8f\xcf\x1d\x00\x00\x00\x06\xc5\xa5\x57\x5f"
        "\xfb\xf6\x73\x73\x73\xe7\xbe\xe7\x86\x1b\x25\x37\xae\x6c\xea\x5e\x47"
        "\xe2\x4f\xd6\x0e\x29\xde\x8d\x8a\x37\xfa\xfc\x8b\x09\x00\x00\xe8\xb9"
        "\x0f\xfe\xf1\xf6\x9f\xbe\x7f\x72\xf2\x37\x2b\xef\xff\x5f\x5d\xff\x2e"
        "\xbd\xff\x3f\xcd\xe6\x9a\x71\x6d\xbf\x3f\xc7\x1d\xd2\xeb\x04\xd2\xfb"
        "\x00\xd6\xbd\x5f\xff\xd9\xb5\x79\xa6\xca\xf6\xbb\xb8\x76\xbf\x66\xb6"
        "\xdf\x50\x8c\xb1\xac\xee\xf1\xb6\xe3\xb4\x2f\xba\x97\xee\x37\x5d\x96"
        "\xaf\xb9\xf6\x38\xa3\x25\xf9\x26\xb2\x7c\x93\xed\xf9\x36\x58\xa7\x60"
        "\x38\xdb\x3f\xe5\x3b\x90\x6d\xcf\xd7\x27\x4c\xfb\x4d\x65\xdb\xf3\x75"
        "\x18\x87\xb3\x1c\x45\x96\xff\xc1\x6c\xff\x1f\x07\x00\x00\x00\x58\x75"
        "\xfc\x95\x97\x2e\x1e\xbf\xf4\xea\x6b\x5f\xbb\xf0\xd2\x73\x2f\x9c\x7b"
        "\xe1\xdc\xcb\x27\x4f\xcc\x7e\x7d\xf6\xf1\xd9\xd9\x53\xb3\xc7\x97\x5f"
        "\xd7\x7f\xbc\xfd\xd5\xfd\x00\x00\x00\xc0\x6e\xb4\xfa\xa2\xdf\x7e\x57"
        "\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x83"
        "\xab\x8e\x8f\x13\xeb\xf7\x39\x02\x00\x00\xc0\xa0\xfb\xcf\x95\x10\xc2"
        "\xbc\x10\x42\x08\x21\x84\x10\xa2\x9e\x88\x9f\x40\xdd\xc3\x63\x16\x7d"
        "\x3f\x27\x21\x06\x21\xfe\xd6\xf3\x7f\xbb\x75\xc7\xe2\x62\xfe\x49\xf3"
        "\x00\x00\x00\x00\xdb\xeb\xd6\x8d\xd7\xcf\xb6\xb7\xeb\xcc\x17\x3d\xcd"
        "\xd7\x3a\x5a\x73\xa5\x59\x88\x79\x53\xfb\xbb\x87\x7f\xfa\xf0\x52\xa4"
        "\xdd\xae\x3f\xb1\xf6\xf9\x92\xfd\x3d\xad\x86\x41\x57\xf7\xcf\xff\xed"
        "\xf3\x0f\xf7\x39\xff\x06\x06\x3c\xff\xfb\x6f\xf4\x36\xff\x78\xba\xd1"
        "\xf5\xef\xbf\xc6\xda\x03\x9c\xae\x96\xf7\xcb\x3f\xfa\xd7\x23\xed\xf9"
        "\xef\x1b\xee\x32\x7f\x7e\xfe\xcf\x54\xcb\x7f\x34\xcb\x7f\x34\x74\x97"
        "\x7f\xf1\xbd\x2c\xff\xb3\xd5\xf2\x3f\x92\xe5\xdf\xdf\x65\xfe\x75\xe7"
        "\x7f\xb1\x5a\xfe\x47\x63\xfe\x83\xa9\x9e\x87\xba\xcd\xbf\xf6\xf1\x1f"
        "\x8b\x6d\x3a\x8f\x7d\x5d\xe6\x3f\x96\x9d\xff\xf3\xa1\xdb\xfc\xd9\xf9"
        "\x37\xbb\x4c\x98\xf9\x4a\xcc\x0f\x00\x83\xa8\xd1\xef\x02\xb6\x49\xba"
        "\x4a\x48\xd7\xd1\x13\xb1\x9f\xce\x37\xcd\x6c\xf2\x57\x3f\x6c\xf6\xfa"
        "\xbf\x91\x1d\x67\x83\x19\x53\x25\xe9\xb8\xe9\x3a\xe8\x9e\xd8\x4f\xd7"
        "\x4b\x93\x59\xde\x64\xb3\xf5\x4f\x64\xc7\xbb\xa3\x62\x9d\xb9\xdd\xf2"
        "\xaa\x92\xb2\xfa\x7b\xf5\x38\x6e\xb7\xb2\xfa\x47\x6a\xae\xa3\xaa\xb2"
        "\xfa\x47\x6b\xae\xa3\xaa\xb2\xfa\xc7\x6a\xae\xa3\xaa\xb2\xfa\xc7\x6b"
        "\xae\xa3\xaa\xb2\xfa\xbb\x9d\x87\xf6\x5b\x59\xfd\xbb\xe5\x79\xe5\xb2"
        "\xfa\x27\x6a\xae\xa3\xaa\xb2\xfa\x27\x6b\xae\xa3\xaa\xb2\xfa\x37\xfb"
        "\xff\x78\xbf\x94\xd5\x7f\xa0\xe6\x3a\xaa\x2a\xab\x7f\xea\x36\xf7\xd9"
        "\x49\xff\xb7\x95\xd5\x5f\xf1\x69\xb5\xda\x95\xd5\x3f\x5d\x73\x1d\x55"
        "\x95\xd5\x7f\x67\xcd\x75\x54\x55\x56\xff\x5d\x35\xd7\x51\x55\x59\xfd"
        "\x77\xd7\x5c\x47\xbf\xdc\x1f\xdb\xb2\xf9\x70\x9a\x7f\x4e\xc5\xb1\xd4"
        "\x6f\x66\xfd\xb1\x0d\xbe\x97\x6d\xfd\xc5\x68\xdb\xce\x03\x00\x00\x00"
        "\x28\xf7\x6f\xeb\xff\x09\x21\xc4\xd6\xa2\xb1\x03\x6a\x10\x62\xc0\x63"
        "\xa9\xe9\x77\x0d\x83\x1a\xdf\xfd\x4b\xff\x6b\xa8\x39\x8a\x95\x1b\x7d"
        "\xaf\x43\x88\xed\x8d\xc5\xc5\xfe\xd7\x20\x7a\x1e\xfe\x24\x3f\xd8\xb6"
        "\xf7\xdd\xcc\x00\xec\x54\x7e\xff\x0f\x36\x8f\xff\x60\xf3\xf8\x0f\x36"
        "\x8f\x3f\xb7\x93\x5e\xc3\x5f\x64\xfd\x64\xa8\xc3\xf8\x70\x87\xf1\x91"
        "\x0e\xe3\xa3\xd9\x78\xfe\xf3\x3a\xd6\x61\xfc\xae\xec\xb8\xf9\x1b\x11"
        "\xee\xee\x30\xfe\xb9\x0e\xe3\x07\x3a\x8c\xdf\xd3\x61\xfc\x60\x87\xf1"
        "\x7b\x3b\x8c\xdf\xd7\x61\xfc\xfe\x0e\xe3\x00\x00\x00\x0c\x86\xcf\xc7"
        "\xd6\xfc\x10\x00\x00\x00\xf6\xae\xcb\x3f\xff\xf0\x87\xbf\x3c\xfa\xec"
        "\x8d\xe9\x9b\xd7\x0e\x9d\x0e\xa3\xeb\xd6\x9d\x3f\x11\xfb\x63\xf1\x6f"
        "\xeb\x57\x63\x3f\x5f\xf7\x3e\x19\x89\x7f\xf3\xff\x41\xec\xff\x2c\xb6"
        "\xbf\x8d\xed\xdf\xb3\xfd\xbd\xfe\x04\x00\x00\x00\xb6\x5f\xfa\x9c\x18"
        "\x7f\xff\x07\x00\x00\x80\xbd\x2b\x7d\x4e\xa9\xf9\x3f\x00\x00\x00\xec"
        "\x5d\xd3\xb1\x35\xff\x07\x00\x00\x80\xbd\xeb\xce\xd8\x9a\xff\x03\x00"
        "\x00\xc0\x1e\x56\x8c\x6f\xbc\x39\xb6\xe9\x79\x81\x07\x63\xdb\xed\xba"
        "\x7e\x00\xc0\xce\xf7\x85\xd8\x3e\x10\xdb\x43\xb1\x3d\x1c\xdb\x2f\xc6"
        "\x36\x5d\x07\x3c\x14\xdb\x2f\xd5\x54\x1f\x00\xd0\x3b\x3f\xf9\xd6\x9b"
        "\x8f\xbf\x55\xac\xae\xf7\x7f\x32\x1b\xbf\x15\xb7\xa7\x76\x9d\xf9\x95"
        "\x67\x0a\x8a\xc6\xda\x95\xfc\xf7\xc5\x76\x7f\x6c\x1f\xee\xb2\x9e\xfc"
        "\xf3\x00\xba\xcd\x9f\x1c\xe8\x32\xcf\x76\xe5\x9f\xda\x62\x7e\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x60\xef\x68\x2c\x7f\x3d\x75\xea\x60\x11\xc2\x3b\x1f"
        "\xbd\x7d\x66\xea\xf2\x99\x17\x96\xb6\x1d\x6e\xed\x71\x64\xf9\x6b\x11"
        "\x7b\xcd\x10\xc2\x48\xeb\x7e\x69\x74\xb5\xff\x8b\xb8\xe3\xad\x1b\xaf"
        "\x9f\x5d\x6a\x17\x62\xbb\x18\xdb\x22\xcc\x86\x22\x14\xad\xf1\xf0\xf4"
        "\xf5\x56\xa6\x89\x10\xc2\x7c\x38\x12\x3e\x0e\xcd\x70\xf8\x58\xf3\xb3"
        "\xab\x43\x4f\x9d\xff\xe0\xdd\x4f\x8e\x5e\xbe\xf8\xe4\x8b\xdb\xf8\x2d"
        "\x00\x00\x00\x80\x3d\xef\xff\x01\x00\x00\xff\xff\x3f\x2a\x37\xce",
        3875);
    syz_mount_image(/*fs=*/0x20000ec0, /*dir=*/0x200000c0, /*flags=*/0x2008054,
                    /*opts=*/0x20000040, /*chdir=*/1, /*size=*/0xf23,
                    /*img=*/0x20001e40);
    break;
  case 1:
    memcpy((void*)0x20000000, "./bus\000", 6);
    res = syscall(__NR_open, /*file=*/0x20000000ul, /*flags=*/0x60142ul,
                  /*mode=*/0ul);
    if (res != -1)
      r[0] = res;
    break;
  case 2:
    memcpy((void*)0x20007f80, "./bus\000", 6);
    res = syscall(__NR_open, /*file=*/0x20007f80ul, /*flags=*/0x145142ul,
                  /*mode=*/0ul);
    if (res != -1)
      r[1] = res;
    break;
  case 3:
    syscall(__NR_ftruncate, /*fd=*/r[1], /*len=*/0x2007ffbul);
    break;
  case 4:
    syscall(__NR_sendfile, /*fdout=*/r[0], /*fdin=*/r[1], /*off=*/0ul,
            /*count=*/0x1000000201005ul);
    break;
  case 5:
    memcpy((void*)0x20004280, "./file0\000", 8);
    res = syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0x20004280ul,
                  /*flags=*/0ul, /*mode=*/0ul);
    if (res != -1)
      r[2] = res;
    break;
  case 6:
    syscall(__NR_lseek, /*fd=*/r[2], /*offset=*/0x101ul, /*whence=*/0ul);
    break;
  case 7:
    syscall(__NR_getdents64, /*fd=*/r[2], /*ent=*/0x9999999999999999ul,
            /*count=*/0x29ul);
    break;
  case 8:
    memcpy((void*)0x20000280, "cgroup.controllers\000", 19);
    syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0x20000280ul,
            /*flags=*/0x275aul, /*mode=*/0ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul, /*prot=*/7ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
