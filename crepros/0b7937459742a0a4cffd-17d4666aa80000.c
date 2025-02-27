// https://syzkaller.appspot.com/bug?id=813569ceafccff50448ffda6320265e8e2d00dc3
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

#ifndef __NR_copy_file_range
#define __NR_copy_file_range 326
#endif
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

static void use_temporary_dir(void)
{
  char tmpdir_template[] = "./syzkaller.XXXXXX";
  char* tmpdir = mkdtemp(tmpdir_template);
  if (!tmpdir)
    exit(1);
  if (chmod(tmpdir, 0777))
    exit(1);
  if (chdir(tmpdir))
    exit(1);
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

#define FS_IOC_SETFLAGS _IOW('f', 2, long)
static void remove_dir(const char* dir)
{
  int iter = 0;
  DIR* dp = 0;
retry:
  while (umount2(dir, MNT_DETACH | UMOUNT_NOFOLLOW) == 0) {
  }
  dp = opendir(dir);
  if (dp == NULL) {
    if (errno == EMFILE) {
      exit(1);
    }
    exit(1);
  }
  struct dirent* ep = 0;
  while ((ep = readdir(dp))) {
    if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
      continue;
    char filename[FILENAME_MAX];
    snprintf(filename, sizeof(filename), "%s/%s", dir, ep->d_name);
    while (umount2(filename, MNT_DETACH | UMOUNT_NOFOLLOW) == 0) {
    }
    struct stat st;
    if (lstat(filename, &st))
      exit(1);
    if (S_ISDIR(st.st_mode)) {
      remove_dir(filename);
      continue;
    }
    int i;
    for (i = 0;; i++) {
      if (unlink(filename) == 0)
        break;
      if (errno == EPERM) {
        int fd = open(filename, O_RDONLY);
        if (fd != -1) {
          long flags = 0;
          if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == 0) {
          }
          close(fd);
          continue;
        }
      }
      if (errno == EROFS) {
        break;
      }
      if (errno != EBUSY || i > 100)
        exit(1);
      if (umount2(filename, MNT_DETACH | UMOUNT_NOFOLLOW))
        exit(1);
    }
  }
  closedir(dp);
  for (int i = 0;; i++) {
    if (rmdir(dir) == 0)
      break;
    if (i < 100) {
      if (errno == EPERM) {
        int fd = open(dir, O_RDONLY);
        if (fd != -1) {
          long flags = 0;
          if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == 0) {
          }
          close(fd);
          continue;
        }
      }
      if (errno == EROFS) {
        break;
      }
      if (errno == EBUSY) {
        if (umount2(dir, MNT_DETACH | UMOUNT_NOFOLLOW))
          exit(1);
        continue;
      }
      if (errno == ENOTEMPTY) {
        if (iter < 100) {
          iter++;
          goto retry;
        }
      }
    }
    exit(1);
  }
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
  if (symlink("/dev/binderfs", "./binderfs")) {
  }
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
    char cwdbuf[32];
    sprintf(cwdbuf, "./%d", iter);
    if (mkdir(cwdbuf, 0777))
      exit(1);
    reset_loop();
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      if (chdir(cwdbuf))
        exit(1);
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
    remove_dir(cwdbuf);
  }
}

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    memcpy((void*)0x20000000, "udf\000", 4);
    memcpy((void*)0x20000080, "./file0\000", 8);
    memcpy((void*)0x200001c0, "fileset", 7);
    *(uint8_t*)0x200001c7 = 0x3d;
    sprintf((char*)0x200001c8, "%020llu", (long long)0x10001);
    *(uint8_t*)0x200001dc = 0x2c;
    memcpy((void*)0x200001dd, "longad", 6);
    *(uint8_t*)0x200001e3 = 0x2c;
    memcpy((void*)0x200001e4, "iocharset", 9);
    *(uint8_t*)0x200001ed = 0x3d;
    memcpy((void*)0x200001ee, "maciceland", 10);
    *(uint8_t*)0x200001f8 = 0x2c;
    memcpy((void*)0x200001f9, "adinicb", 7);
    *(uint8_t*)0x20000200 = 0x2c;
    memcpy((void*)0x20000201, "gid", 3);
    *(uint8_t*)0x20000204 = 0x3d;
    sprintf((char*)0x20000205, "%020llu", (long long)0);
    *(uint8_t*)0x20000219 = 0x2c;
    memcpy((void*)0x2000021a, "umask", 5);
    *(uint8_t*)0x2000021f = 0x3d;
    sprintf((char*)0x20000220, "%023llo", (long long)0x56b);
    *(uint8_t*)0x20000237 = 0x2c;
    memcpy((void*)0x20000238, "uid=forget", 10);
    *(uint8_t*)0x20000242 = 0x2c;
    *(uint8_t*)0x20000243 = 0;
    memcpy(
        (void*)0x20001040,
        "\x78\x9c\xec\xdd\x4f\x6c\x1c\xd7\x7d\x07\xf0\xdf\x1b\x92\x22\x29\xb7"
        "\x15\x13\x3b\x8a\xdd\xc6\xc5\xa6\x2d\x52\x99\xb1\x5c\xfd\x8b\xa9\x58"
        "\x85\xbb\xaa\x69\xb6\x01\x64\x99\x08\xc5\xdc\x02\x70\x45\x52\xea\xc2"
        "\x14\x49\x90\x54\x23\x1b\x69\xc1\xf4\xd2\x43\x0f\x01\x8a\xa2\x87\x9c"
        "\x08\xb4\x46\x81\x14\x0d\x8c\xa6\x08\x7a\x64\x5b\x17\x48\x2e\x3e\x14"
        "\x39\xf5\x44\xb4\xb0\x11\x14\x3d\xb0\x45\x80\x9c\x02\x16\x33\xfb\x96"
        "\x5c\x52\x94\xa5\x8a\xa4\x44\x5a\x9f\x8f\x4d\x7d\x77\x67\xde\x9b\x79"
        "\x6f\x66\x3c\x23\x0b\x7a\xf3\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x88\xf8\xdd\xd7\x2f\x9f\x39\x9b\x1e\x77\x2b\x00"
        "\x80\x47\xe9\xea\xd8\x57\xcf\x9c\xf3\xfc\x07\x80\x27\xca\x35\xff\xff"
        "\x0f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x87\x5d\x8a\x22\x9e"
        "\x8e\x14\xf3\x57\xd7\xd3\x44\xf5\xbd\xa5\xef\x4a\x73\xf6\xf6\x9d\xf1"
        "\xe1\x91\xdd\xab\xf5\xa7\xaa\x66\x57\x55\xbe\xfc\xe9\x3b\x7b\xee\xfc"
        "\x85\x2f\xbd\x3c\x74\xb1\x9d\x1f\x5f\x7f\xbf\x3d\x17\x6f\x8e\x5d\xbb"
        "\x5c\x7b\x6d\xee\xd6\xfc\xc2\xf4\xe2\xe2\xf4\x54\x6d\x7c\xb6\x39\x39"
        "\x37\x35\xfd\xc0\x5b\xd8\x6b\xfd\x9d\x06\xab\x03\x50\xbb\xf5\xd6\xed"
        "\xa9\x1b\x37\x16\x6b\xe7\x5e\x3a\xbf\x6d\xf5\x9d\x81\x8f\x7a\x9f\x3a"
        "\x39\x70\x69\xe8\x85\xd3\xcf\xb7\xcb\x8e\x0f\x8f\x8c\x8c\x75\x94\xe9"
        "\xee\x79\xe8\xbd\xdf\xe5\x5e\x23\x3c\x8e\x45\x11\xa7\x23\xc5\x8b\xdf"
        "\xfb\x49\x6a\x44\x44\x11\x7b\x3f\x16\xf7\xb9\x76\x0e\x5a\x7f\xd5\x89"
        "\xc1\xaa\x13\xe3\xc3\x23\x55\x47\x66\x9a\x8d\xd9\xa5\x72\xe5\x68\xfb"
        "\x40\x14\x11\xb5\x8e\x4a\xf5\xf6\x31\x7a\x04\xe7\x62\x4f\xea\x11\xcb"
        "\x65\xf3\xcb\x06\x0f\x96\xdd\x1b\x9b\x6f\x2c\x34\xae\xcf\x4c\xd7\x46"
        "\x1b\x0b\x4b\xcd\xa5\xe6\xdc\xec\x68\x6a\xb5\xb6\xec\x4f\x2d\x8a\xb8"
        "\x98\x22\x56\x22\x62\xad\xf7\xee\xcd\xf5\x44\x11\xdd\x91\xe2\x3b\x27"
        "\xd6\xd3\xf5\x88\xe8\x6a\x1f\x87\x2f\x56\x03\x83\xef\xdd\x8e\xe2\x00"
        "\xfb\xf8\x00\xca\x76\xd6\x7a\x22\x56\x8a\x23\x70\xce\x0e\xb1\xde\x28"
        "\xe2\x8d\x48\xf1\xd3\xf7\x4f\xc5\x64\x79\xcc\xf2\x4f\x7c\x21\xe2\x8d"
        "\x32\x7f\x10\xf1\x6e\x99\xaf\x46\xa4\xf2\xc2\xb8\x10\xf1\xe1\x2e\xd7"
        "\x11\x47\x53\x77\x14\xf1\x67\xe5\xf9\xbf\xb4\x9e\xa6\xaa\xfb\x41\xfb"
        "\xbe\x72\xe5\x6b\xb5\xaf\xcc\xde\x98\xeb\x28\xdb\xbe\xaf\x1c\xd1\xe7"
        "\x43\xff\x8e\x7c\x34\x0e\xf9\xbd\xa9\x2f\x8a\x68\x54\x77\xfc\xf5\xf4"
        "\xf0\xbf\xd9\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x60"
        "\xbf\xf5\x47\x11\xcf\x45\x8a\xd7\xff\xed\x0f\xab\x71\xc5\x51\x8d\x4b"
        "\x3f\x71\x69\xe8\xf7\x06\x7e\xb1\x73\xcc\xf8\xb3\xf7\xd9\x4e\x59\xf6"
        "\xa5\x88\x58\x2e\x1e\x6c\x4c\xee\xb1\x3c\x84\x78\x34\x8d\xa6\xf4\x98"
        "\xc7\x12\x3f\xc9\xfa\xa2\x88\x3f\xca\xe3\xff\xbe\xf5\xb8\x1b\x03\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0\x44\x2b\xe2"
        "\xc7\x91\xe2\x95\x0f\x4e\xa5\x95\xe8\x9c\x53\xbc\x39\x7b\xb3\x76\xad"
        "\x71\x7d\xa6\x35\x2b\x6c\x7b\xee\xdf\xf6\x9c\xe9\x1b\x1b\x1b\x1b\xb5"
        "\xd4\xca\x7a\xce\x89\x9c\xcb\x39\x57\x72\xae\xe6\x5c\xcb\x19\x45\xae"
        "\x9f\xb3\x9e\x73\x22\xe7\x72\xce\x95\x9c\xab\x39\xd7\x72\x46\x57\xae"
        "\x9f\xb3\x9e\x73\x22\xe7\x72\xce\x95\x9c\xab\x39\xd7\x72\x46\x77\xae"
        "\x9f\xb3\x9e\x73\x22\xe7\x72\xce\x95\x9c\xab\x39\xd7\x72\xc6\x21\x99"
        "\xbb\x17\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xe0\x93\xa4"
        "\x88\x22\x7e\x1e\x29\xbe\xfd\x8d\xf5\x14\x29\x22\xea\x11\x13\xd1\xca"
        "\xd5\xde\xc7\xdd\x3a\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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
        "\x00\xa0\xd4\x9b\x8a\xf8\x7e\xa4\xa8\xfd\x7e\x7d\x73\x59\x77\x44\xa4"
        "\xea\xdf\x96\x53\xe5\x2f\x17\xa2\x7e\xac\xcc\x4f\x47\x7d\xa8\xcc\x57"
        "\xa3\x7e\x39\x67\xa3\xca\xee\xfa\xb7\x1e\x43\xfb\xd9\x9b\x9e\x54\xc4"
        "\x8f\x22\x45\x6f\xdf\x7b\x9b\x27\x3c\x9f\xff\x9e\xd6\xb7\xcd\xcb\x20"
        "\xde\xfd\xe6\xd6\xb7\x5f\xee\x6e\x65\x57\x7b\xe5\xc0\x47\xbd\x4f\x9d"
        "\x3c\x71\x69\x68\xe4\x57\x9f\xbd\xd7\xe7\xb4\x5b\x03\x06\xaf\x34\x67"
        "\x6f\xdf\xa9\x8d\x0f\x8f\x8c\x8c\x75\x2c\xee\xce\x7b\xff\x74\xc7\xb2"
        "\x81\xbc\xdf\x62\x7f\xba\x4e\x44\x2c\xbe\xfd\xce\x5b\x8d\x99\x99\xe9"
        "\x85\x87\xff\x50\x5e\x02\x7b\xa8\xee\x83\x0f\x87\xf5\x43\x74\x1f\x8a"
        "\x66\x3c\x9e\xbe\xf3\x04\x28\x9f\xff\x1f\x46\x8a\xdf\xfa\xe0\xdf\xdb"
        "\x0f\xfc\xf6\xf3\xff\x17\x5a\xdf\x36\x9f\xf0\xf1\xb3\x3f\xde\x7a\xfe"
        "\xbf\xb2\x73\x43\x07\xf4\xfc\x7f\xba\x63\xd9\x2b\xf9\x77\x23\x3d\xdd"
        "\x11\x7d\x4b\xb7\xe6\x7b\x4e\x46\xf4\x2d\xbe\xfd\xce\xe9\xe6\xad\xc6"
        "\xcd\xe9\x9b\xd3\xb3\x17\xce\x9c\xf9\xf2\xd0\xd0\x97\xcf\x9f\xe9\x39"
        "\x16\xd1\x77\xa3\x39\x33\xdd\xf1\x69\xcf\x87\x0a\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\xe0\xd1\x4a\x45\xfc\x4e\xa4\x68\xfc\x68"
        "\x3d\xd5\x22\xe2\x4e\x35\x5e\x6b\xe0\xd2\xd0\x0b\xa7\x9f\xef\x8a\xae"
        "\x6a\xbc\xd5\xb6\x71\x5b\x6f\x8e\x5d\xbb\x5c\x7b\x6d\xee\xd6\xfc\xc2"
        "\xf4\xe2\xe2\xf4\x54\x6d\x7c\xb6\x39\x39\x37\x35\xfd\xa0\xbb\xeb\xab"
        "\x86\x7b\x8d\x0f\x8f\x1c\x48\x67\xee\xab\xff\x80\xdb\xdf\xdf\xf7\xda"
        "\xdc\xfc\xdb\x0b\xcd\x9b\x7f\xb0\xb4\xeb\xfa\xe3\x7d\x97\xaf\x2f\x2e"
        "\x2d\x34\x26\x77\x5f\x1d\xfd\x51\x44\xd4\x3b\x97\x0c\x56\x0d\x1e\x1f"
        "\x1e\xa9\x1a\x3d\xd3\x6c\xcc\x56\x55\x47\x77\x1d\x4c\xf7\xff\xd7\x93"
        "\x8a\xf8\x8f\x48\x31\x79\xa1\x96\x3e\x9f\x97\xe5\xf1\x7f\x3b\x47\xf8"
        "\x6f\x1b\xff\xbf\xbc\x73\x43\x07\x34\xfe\xef\x53\x1d\xcb\xca\x7d\xa6"
        "\x54\xc4\xcf\x22\xc5\x6f\xfe\xf9\xb3\xf1\xf9\xaa\x9d\xc7\xe3\xae\x63"
        "\x96\xcb\xfd\x75\xa4\x18\xbc\xf8\xb9\x5c\x2e\x8e\x95\xe5\xda\x6d\x68"
        "\xbd\x57\xa0\x35\x32\xb0\x2c\xfb\x3f\x91\xe2\xef\x7f\xbe\xbd\x6c\x7b"
        "\x3c\xe4\xd3\x5b\x65\xcf\x3e\xf0\x81\x3d\x22\xca\xf3\x7f\x22\x52\x7c"
        "\xff\x4f\xbf\x1b\xbf\x96\x97\x6d\x7f\xff\xc3\xee\xe7\xff\xf8\xce\x0d"
        "\x1d\xd0\xf9\x7f\xa6\x63\xd9\xf1\x6d\xef\x2b\xd8\x73\xd7\xc9\xe7\xff"
        "\x74\xa4\x78\xf5\xe9\xf7\xe2\xd7\xf3\xb2\x8f\x7b\xff\x47\xfb\xdd\x1b"
        "\xa7\x72\xe1\xcd\xf7\x73\x1c\xd0\xf9\xff\x4c\xc7\xb2\x81\xbc\xdf\xdf"
        "\xd8\x9f\xae\x03\x00\x00\x00\x00\x00\x00\x00\x00\x1c\x69\x3d\xa9\x88"
        "\xbf\x89\x14\xcf\x8f\x74\xa7\x97\xf3\xb2\x07\xf9\xfb\x7f\x53\x3b\x37"
        "\x74\x40\x7f\xff\xeb\xb3\x1d\xcb\xa6\xf6\x67\xbe\xa2\xfb\x7e\xd8\xf3"
        "\x41\x05\x00\x00\x00\x80\x43\xa2\x27\x15\xf1\xe3\x48\x71\x73\xe9\xbd"
        "\xcd\x31\xd4\xdb\xc7\x7f\x77\x8c\xff\xfc\xed\xad\xf1\x9f\xc3\x69\xc7"
        "\xda\xea\xcf\xf9\x7e\xa9\x7a\x6f\xc0\x7e\xfe\xf9\x5f\xa7\x81\xbc\xdf"
        "\x89\xbd\x77\x1b\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x0e\x95\x94\x8a\x78\x39\xcf\xa7\x3e\x71\x9f\xf9\xd4\x57\x23"
        "\xc5\xeb\xff\xf5\x62\x2e\x97\x4e\x96\xe5\xda\xf3\xc0\x0f\x54\xbf\xf6"
        "\x5d\x9d\x9b\x3d\x7d\x79\x66\x66\x6e\xb2\xb1\xd4\xb8\x3e\x33\x5d\x1b"
        "\x9b\x6f\x4c\x4e\x97\x75\x9f\x89\x14\xeb\x7f\xf5\xb9\x5c\xb7\xa8\xe6"
        "\x57\x6f\xcf\x37\xdf\x9a\xe3\x7d\x6b\x2e\xf6\x85\x48\x31\xf2\xb7\xed"
        "\xb2\xad\xb9\xd8\xdb\x73\x93\x3f\xb3\x55\xf6\x6c\x59\xf6\x53\x91\xe2"
        "\x3f\xff\x6e\x5b\xd9\xcd\xd7\x0c\x7c\x66\xab\xec\xb9\xb2\xec\x5f\x46"
        "\x8a\xaf\xff\xe3\xf6\xed\xb6\xe7\xbc\x3e\xb9\x55\xf6\x7c\x59\xf6\xbb"
        "\x91\xe2\x87\x5f\xaf\xb5\xcb\x1e\x2f\xcb\xb6\xdf\x8f\xfa\xd9\xad\xb2"
        "\x2f\x4d\xce\x15\xfb\x7f\x52\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x78\xe2\xf4\xa4\x22\xfe\x24\x52\xfc\xf7\xad\x95"
        "\xcd\xb1\xfc\x79\xfe\xff\x9e\x8e\xaf\x95\x77\xbf\xd9\x31\xdf\xff\x0e"
        "\x77\xaa\x79\xfe\x07\xaa\xf9\xff\xef\xf5\xf9\x61\xe6\xff\x1f\xd8\x9f"
        "\x6e\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xc0\x91"
        "\x92\xa2\x88\x77\x22\xc5\xfc\xd5\xf5\xb4\xda\x5b\x7e\x6f\xe9\xbb\xd2"
        "\x9c\xbd\x7d\x67\x7c\x78\x64\xf7\x6a\xfd\xa9\xaa\xd9\x55\x95\x2f\x7f"
        "\xfa\xce\x9e\x3b\x7f\xe1\x4b\x2f\x0f\x5d\x6c\xe7\xc7\xd7\xdf\x6f\xcf"
        "\xc5\x9b\x63\xd7\x2e\xd7\x5e\x9b\xbb\x35\xbf\x30\xbd\xb8\x38\x3d\x55"
        "\x1b\x9f\x6d\x4e\xce\x4d\x4d\x3f\xf0\x16\xf6\x5a\x7f\xa7\xc1\xea\x00"
        "\xd4\x6e\xbd\x75\x7b\xea\xc6\x8d\xc5\xda\xb9\x97\xce\x6f\x5b\x7d\x67"
        "\xe0\xa3\xde\xa7\x4e\x0e\x5c\x1a\x7a\xe1\xf4\xf3\xed\xb2\xe3\xc3\x23"
        "\x23\x63\x1d\x65\xba\x7b\x1e\x7a\xef\x77\x49\xf7\x58\x7e\x2c\x8a\xf8"
        "\x8b\x48\xf1\xe2\xf7\x7e\x92\xfe\xa9\x37\xa2\x88\xbd\x1f\x8b\xfb\x5c"
        "\x3b\x07\xad\xbf\xea\xc4\x60\xd5\x89\xf1\xe1\x91\xaa\x23\x33\xcd\xc6"
        "\xec\x52\xb9\x72\xb4\x7d\x20\x8a\x88\x5a\x47\xa5\x7a\xfb\x18\x3d\x82"
        "\x73\xb1\x27\xf5\x88\xe5\xb2\xf9\x65\x83\x07\xcb\xee\x8d\xcd\x37\x16"
        "\x1a\xd7\x67\xa6\x6b\xa3\x8d\x85\xa5\xe6\x52\x73\x6e\x76\x34\xb5\x5a"
        "\x5b\xf6\xa7\x16\x45\x5c\x4c\x11\x2b\x11\xb1\xd6\x7b\xf7\xe6\x7a\xa2"
        "\x88\xb7\x22\xc5\x77\x4e\xac\xa7\x7f\xee\x8d\xe8\x6a\x1f\x87\x2f\x5e"
        "\x1d\xfb\xea\x99\x73\xf7\x6e\x47\x71\x80\x7d\x7c\x00\x65\x3b\x6b\x3d"
        "\x11\x2b\xc5\x11\x38\x67\x87\x58\x6f\x14\xf1\x0f\x91\xe2\xa7\xef\x9f"
        "\x8a\x7f\xe9\x8d\xe8\x8e\xd6\x4f\x7c\x21\xe2\x8d\x32\x7f\x10\xf1\x6e"
        "\xb4\xce\x77\x2a\x2f\x8c\x0b\x11\x1f\xee\x72\x1d\x71\x34\x75\x47\x11"
        "\xff\x5b\x9e\xff\x4b\xeb\xe9\xfd\xde\xf2\x7e\xd0\xbe\xaf\x5c\xf9\x5a"
        "\xed\x2b\xb3\x37\xe6\x3a\xca\xb6\xef\x2b\x47\xfe\xf9\xf0\x28\x1d\xf2"
        "\x7b\x53\x5f\x14\xf1\xc3\xea\x8e\xbf\x9e\xfe\xd5\x7f\xd7\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x87\x48\x11\xbf\x12\x29\x5e"
        "\xf9\xe0\x54\xaa\xc6\x07\x6f\x8e\x29\x6e\xce\xde\xac\x5d\x6b\x5c\x9f"
        "\x69\x0d\xeb\x6b\x8f\xfd\x6b\x8f\x99\xde\xd8\xd8\xd8\xa8\xa5\x56\xd6"
        "\x73\x4e\xe4\x5c\xce\xb9\x92\x73\x35\xe7\x5a\xce\x28\x72\xfd\x9c\xf5"
        "\x32\xfb\x36\x36\x26\xf2\xf7\xe5\x9c\x2b\x39\x57\x73\xae\xe5\x8c\xae"
        "\x5c\x3f\x67\x3d\xe7\x44\xce\xe5\x9c\x2b\x39\x57\x73\xae\xe5\x8c\xee"
        "\x5c\x3f\x67\x3d\xe7\x44\xce\xe5\x9c\x2b\x39\x57\x73\xae\xe5\x8c\x43"
        "\x32\x76\x0f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\xf8\x64\x29\xaa\x7f\x52\x7c\xfb\x1b\xeb\x69\xa3\xb7\x35\xbf\xf4\x44"
        "\xb4\x72\xd5\x7c\xa0\x9f\x78\xff\x17\x00\x00\xff\xff\xd4\x41\xfa\x1d",
        3077);
    syz_mount_image(/*fs=*/0x20000000, /*dir=*/0x20000080, /*flags=*/0x2010008,
                    /*opts=*/0x200001c0, /*chdir=*/5, /*size=*/0xc05,
                    /*img=*/0x20001040);
    break;
  case 1:
    memcpy((void*)0x20000080, "./file1\000", 8);
    res =
        syscall(__NR_open, /*file=*/0x20000080ul, /*flags=*/0ul, /*mode=*/0ul);
    if (res != -1)
      r[0] = res;
    break;
  case 2:
    memcpy((void*)0x20000040, "./bus\000", 6);
    res = syscall(__NR_creat, /*file=*/0x20000040ul, /*mode=*/0ul);
    if (res != -1)
      r[1] = res;
    break;
  case 3:
    syscall(__NR_copy_file_range, /*fd_in=*/r[0], /*off_in=*/0ul,
            /*fd_out=*/r[1], /*off_out=*/0ul, /*len=*/0x40000000000004ul,
            /*flags=*/0ul);
    break;
  case 4:
    memcpy((void*)0x20002000, "./bus\000", 6);
    res = syscall(__NR_open, /*file=*/0x20002000ul, /*flags=*/0x143142ul,
                  /*mode=*/0ul);
    if (res != -1)
      r[2] = res;
    break;
  case 5:
    memcpy((void*)0x20002000, "./bus\000", 6);
    res = syscall(__NR_open, /*file=*/0x20002000ul, /*flags=*/0x143042ul,
                  /*mode=*/0x99ul);
    if (res != -1)
      r[3] = res;
    break;
  case 6:
    syscall(__NR_lseek, /*fd=*/r[2], /*offset=*/0ul, /*whence=*/2ul);
    break;
  case 7:
    syscall(__NR_sendfile, /*fdout=*/r[2], /*fdin=*/r[3], /*off=*/0ul,
            /*count=*/0x1000000201005ul);
    break;
  case 8:
    syscall(__NR_sendfile, /*fdout=*/r[1], /*fdin=*/r[3], /*off=*/0ul,
            /*count=*/0x100000001ul);
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
      use_temporary_dir();
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
