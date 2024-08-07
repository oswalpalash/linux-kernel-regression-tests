// https://syzkaller.appspot.com/bug?id=deb061ff946b02c2f4821f91683d89a68b2f45f8
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
  for (call = 0; call < 4; call++) {
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
      if (call == 2)
        break;
      event_timedwait(&th->done,
                      50 + (call == 0 ? 4000 : 0) + (call == 2 ? 4000 : 0));
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

void execute_call(int call)
{
  switch (call) {
  case 0:
    memcpy((void*)0x20000000, "hfsplus\000", 8);
    memcpy((void*)0x200000c0, "./bus\000", 6);
    memcpy(
        (void*)0x20000bc0,
        "\x78\x9c\xec\xdd\x4d\x68\x1b\xd9\x1d\x00\xf0\xff\xc8\xb2\x6c\xb9\xe0"
        "\xf5\xee\x26\xbb\xa1\x2c\xd4\x6c\x60\x5b\x6a\x9a\xd8\x16\xde\xd6\xbd"
        "\x34\x2d\xa5\xf8\xb0\x94\x65\x7b\xe8\xd9\xc4\xca\x5a\x44\xf1\x2e\xb6"
        "\xb6\x38\xa1\x14\xf7\x8b\x5e\x7b\x08\xed\x35\x3d\xf8\xd6\x53\xa1\x77"
        "\x43\x7a\x6e\x6f\xb9\xfa\x18\x28\xe4\x92\x43\xf1\xcd\x65\x46\x4f\xb6"
        "\xfc\xfd\x1d\xc9\xed\xef\x67\x9e\xde\x7b\xf3\x66\xde\xfc\xf5\x97\x66"
        "\x34\x92\x30\x0a\xe0\xff\xd6\xdc\x44\x94\x37\x22\x8b\xb9\x89\x4f\x56"
        "\xf3\xfe\xe6\x7a\xad\xb9\xb9\x5e\x1b\x4a\xc3\xcd\x88\xc8\xdb\xa5\x88"
        "\x72\xbb\x8a\x6c\x29\x22\x7b\x1e\x71\x2f\xda\x25\xb2\x54\x52\x7b\xfb"
        "\xd0\xfd\x3c\x6d\xcc\x7e\xf6\xe2\xf5\xe6\xcb\x76\xaf\x9c\x4a\xb1\x5d"
        "\x29\x86\x77\xb6\x3f\xa7\xb5\x54\x62\x3c\x22\x06\x52\x7d\xd0\xe0\xb9"
        "\xe6\xbb\x9f\xea\xe1\x73\x87\xb7\x9b\xa1\x3c\x61\xb7\x3b\x89\x83\x5e"
        "\xdb\x3e\x60\xed\x2c\x9b\x5f\xf0\xb8\x05\xfa\x41\xd6\x7e\xdd\x3c\x60"
        "\x2c\x62\x24\xbd\xf6\x15\xd7\x04\xe9\xec\x50\x7a\xb3\xd1\x5d\xbe\x33"
        "\x9d\xe5\x00\x00\x00\xe0\x9a\x7a\x6b\x2b\xb6\x62\x35\x46\x7b\x1d\x07"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x5c\x27\xe9\xf7\xff\xb3\x54"
        "\x4a\x9d\xf6\x78\x64\x9d\xdf\xff\xaf\xa4\x65\x91\xda\xd7\xda\x46\xaf"
        "\x03\x00\x00\x00\x00\x00\x00\x00\x80\x4b\xf0\x8d\xad\xd8\x8a\xd5\x18"
        "\xed\xf4\xb7\xb3\xe2\x3b\xff\x0f\x8b\xce\x8d\xe2\xf6\x6b\xf1\x55\xac"
        "\x44\x3d\x96\xe3\x4e\xac\xc6\x7c\xb4\xa2\x15\xcb\x31\x15\x11\x63\x5d"
        "\x13\x55\x56\xe7\x5b\xad\xe5\xa9\x53\x6c\x39\x7d\xe8\x96\xd3\x87\x45"
        "\xb7\x7d\x25\xf7\x19\x00\x00\x00\x00\x00\x00\x00\xfe\x07\xfd\x26\xe6"
        "\x76\xbf\xff\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80"
        "\xde\x1a\xee\x34\x06\x22\x22\x4b\xe5\x46\xa7\x3d\x16\xa5\x72\x5a\xa9"
        "\x92\xaf\xb4\x16\xf1\xaf\x4e\xfb\x3a\x1a\x4a\xf5\x46\x8f\xe3\x00\x00"
        "\x00\x80\x37\xe1\xad\xad\xd8\x8a\xd5\x18\xed\xf4\xb7\xb3\xe2\x3d\xff"
        "\x7b\xc5\xfb\xfe\xe1\xf8\x2a\x96\xa2\x15\x8d\x68\x45\x33\xea\xb1\x50"
        "\x7c\x16\xd0\x7e\xd7\x5f\xda\x5c\xaf\x35\x37\xd7\x6b\x8f\xf2\x72\x70"
        "\xde\x3f\xfd\x67\xbb\x70\xca\x30\x8a\x19\xd3\x67\x0f\x87\xef\xf9\x56"
        "\xb1\x46\x35\x1e\x44\xa3\x58\x72\x27\xee\xc7\x17\xd1\x8c\x85\x28\x15"
        "\x5b\xe6\x6e\x75\xe2\x39\x3c\xae\x5f\xbf\xca\xe7\xfe\x41\x72\xca\xc8"
        "\x16\x52\x9d\xdf\xf3\x3f\xa6\xba\x3f\x8c\x15\x19\x19\xdc\xc9\xc8\x64"
        "\x8a\x2d\xcf\xc6\xdb\xc7\x67\xe2\x87\xaf\x2e\xb4\xa7\xa9\x28\xed\x7c"
        "\xf2\x73\xe3\x0a\x72\x3e\x92\xea\x6c\x5f\xdd\x7b\xfb\x33\x31\xdd\xf5"
        "\xec\x7b\xef\xf8\x4c\x44\x7c\xf3\xef\x7f\xfd\xf9\x62\x73\xe9\xe1\xe2"
        "\x83\x95\xfe\xb9\x47\xe7\xb5\x3f\x13\xb5\xae\x4c\xbc\x7f\x86\x4c\x4c"
        "\x5c\xfb\x54\x4c\x6e\x47\x94\xb2\xdd\xa7\xe9\x5c\xfc\x24\x7e\x16\x13"
        "\x31\x1e\x9f\xc6\x72\x34\xe2\x17\x31\x1f\xad\xa8\xc7\x78\xfc\xb8\x68"
        "\xcd\xa7\x15\xf3\xdb\xb1\xe3\x33\x75\x6f\x4f\xef\xd3\x93\x22\xa9\xa4"
        "\xc7\xa5\x7d\x16\x2d\xc5\xcd\x9d\x91\x93\x63\xfa\xb0\xd8\x76\x34\x1a"
        "\xf1\xd3\xf8\x22\x16\xa2\x1e\x1f\x17\x7f\xd3\x31\x15\xdf\x8d\x99\x98"
        "\x89\xd9\xae\x47\xf8\xe6\x29\x8e\xfa\xd2\xd9\x8e\xfa\xdb\xdf\x4a\x8d"
        "\x6a\x44\xfc\x21\xd5\xfd\x21\xcf\xeb\xdb\x5d\x79\xed\x3e\xe7\x8e\x15"
        "\x63\xdd\x4b\x76\xb3\xf4\xce\xe5\x9f\x1b\xcb\x5f\x4f\x67\xf8\xbc\xfc"
        "\x36\xd5\xfd\x61\x7f\x26\xa6\xba\x32\xf1\xee\xf1\x99\xf8\x4b\x71\x9d"
        "\xb0\xd2\x5c\x7a\xb8\xbc\x38\xff\xe5\x29\xf7\xf7\x51\xaa\xf3\xe3\xe8"
        "\xf7\x7d\xf5\x2a\x91\x3f\x5f\xde\xc9\x1f\xac\xa2\xd7\xc9\xc9\xee\xf3"
        "\xf9\xdd\x03\x63\xed\x7c\x55\xd2\x37\x2e\xed\xb1\xd2\x81\xb1\x9b\x3b"
        "\x63\x27\x1d\xa9\x95\x74\x0d\x77\x70\xa6\xe9\x62\xec\xfd\x43\xc7\x6a"
        "\xc5\xd8\xad\xae\xb1\xc3\xae\xb7\x00\xe8\x7b\x23\xdf\x1e\xa9\x54\xff"
        "\x5d\xfd\x67\xf5\x59\xf5\x77\xd5\xc5\xea\x27\xc3\x3f\x1a\xfa\xde\xd0"
        "\x07\x95\x18\xfc\xc7\xe0\xf7\xcb\x93\x03\x1f\x95\x3e\xc8\xfe\x16\xcf"
        "\xe2\x57\xbb\xef\xff\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\xf3\x5b\x79\xfc\xe4"
        "\xe1\x7c\xb3\x59\x5f\xbe\xb2\x46\xe7\xe7\x6f\xf2\x25\x7f\x1e\x88\x23"
        "\x56\xee\xfc\x8e\xd6\xd5\xc7\x73\xcd\x1b\xd1\x1f\x61\x68\xec\x69\x0c"
        "\x45\x5f\x84\x71\x99\x8d\x1e\x9f\x98\x80\x2b\x77\xb7\xf5\xe8\xcb\xbb"
        "\x2b\x8f\x9f\x7c\xa7\xf1\x68\xfe\xf3\xfa\xe7\xf5\xa5\xc1\x99\x99\xd9"
        "\xc9\xd9\x99\x8f\x6b\x77\x1f\x34\x9a\xf5\xc9\xf6\x6d\xaf\xa3\x04\xae"
        "\x42\xd7\x85\x25\x00\x00\x00\x00\x00\x00\x00\x00\x00\x70\x4d\x1c\xff"
        "\x6f\x00\x83\x69\xad\xbd\x43\x59\x79\xff\x92\x13\xfe\x65\x05\x00\x00"
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
        "\x00\x00\x00\x00\xe0\x02\xe6\x26\xa2\xbc\x11\x59\x4c\x4d\xde\x99\xcc"
        "\xfb\x9b\xeb\xb5\x66\x5e\x22\xe6\xa2\x5d\x77\x94\x23\xa2\x14\x11\xd9"
        "\x2f\x23\xb2\xe7\x11\xf7\xa2\x5d\x62\xac\x6b\xba\xec\xa8\xfd\x3c\x6d"
        "\xcc\x7e\xf6\xe2\xf5\xe6\xcb\xdd\xb9\xca\x9d\xf5\x4b\xc7\x6d\x77\x94"
        "\x81\x3d\xbd\xb5\x54\x62\x3c\x0d\x8d\x9f\x75\xbe\x38\x7a\xbe\xfb\x17"
        "\x9e\x2f\xdb\xb9\x87\x79\xc2\x6e\x77\x12\x07\xbd\xf6\xdf\x00\x00\x00"
        "\xff\xff\x03\xab\x07\x44",
        1553);
    syz_mount_image(0x20000000, 0x200000c0, 0x200008, 0x20000900, 4, 0x611,
                    0x20000bc0);
    break;
  case 1:
    memcpy((void*)0x20000000, "./file2\000", 8);
    syscall(__NR_unlink, 0x20000000ul);
    break;
  case 2:
    memcpy((void*)0x200000c0, "msdos\000", 6);
    memcpy((void*)0x20000100, ".\000", 2);
    *(uint8_t*)0x200007c0 = -1;
    memcpy(
        (void*)0x200007c1,
        "\xee\xa7\x3c\x3c\xa0\x47\x34\x9a\xb6\x6d\x52\x9f\xb1\xe2\xbd\xc6\x18"
        "\x75\xc6\xcc\xeb\xea\x30\xba\x1b\x89\x77\xc6\x32\xdd\xcb\xe0\xed\x04"
        "\xf9\x86\xb6\x8e\xe6\x00\x04\x4b\xd7\x5d\x3c\xfd\x10\xcc\x2e\xc8\x8c"
        "\xcf\xf0\x20\x44\x7c\xd9\xca\xde\xd4\xab\xf6\x57\x67\xe9\xdb\xbd\x7b"
        "\xd8\x41\xa8\xce\x3e\xb4\x4c\x71\x9a\x2e\x6c\x34\x1b\xf6\x8b\x2a\x0a"
        "\xf0\x4f\x1c\xc9\x51\xb1\x90\xc7\xfd\x36\xff\xdb\xd0\x62\x2c\xdc\x9b"
        "\x7c\x9a\x55\xb3\xf4\x65\xde\xf5\x74\x08\x14\x39\x5f\x09\x5f\xc4\x4a"
        "\x4c\xf6\xfe\x14\xb0\xd3\x1f\x38\x42\x14\xd5\x9a\x01\x05\xd5\x77\x55"
        "\x72\x15\x47\x39\x73\xf3\xb7\x01\x78\xf4\x9f\x0e\x48\xdb\xe1\x38\x56"
        "\x2f\x4d\x21\xca\xd5\x9f\xaf\xb7\xfb\xaf\x76\x46\x1c\x56\xfd\xd2\x09"
        "\x33\x29\xe2\xec\xe7\xcf\x96\x43\x5b\xca\x11\xa9\x0b\xa7\xf3\x1e\x69"
        "\xc4\xb7\x3b\xa3\xd1\x72\x90\x6f\xfc\x90\xf5\xde\x13\x4a\x69\x6a\x73"
        "\x1d\x20\xa9\xcf\x70\x00\x9a\x31\xf8\x3d\x4b\x77\x4b\x0d\x99\xde\x69"
        "\xf1\x4d\xb9\xcd\xbd\xe3\x83\x24\x51\x37\x98\xbb\xa5",
        234);
    *(uint16_t*)0x200008ab = -1;
    *(uint64_t*)0x200008ad = -1;
    sprintf((char*)0x200008b5, "%023llo", (long long)-1);
    sprintf((char*)0x200008cc, "%023llo", (long long)-1);
    sprintf((char*)0x200008e3, "%023llo", (long long)-1);
    memcpy((void*)0x200008fa,
           "\x01\xe6\x76\xf4\xfc\x19\x04\xef\xc2\x97\xa3\xba\xa0\x83\xd7\xd5"
           "\x30\x48\x01\x4c\x35\x42\x9e\x76\x3c\x6f\x76\x63\x7c\xfd\x7d\x74"
           "\x91\xfe\x31\x36\x2d\x2a\xf6\xbc\xf1\x76\xcf\xa2\x49\xa2\xb3\x6a"
           "\x99\xee\x6b\x55\x42\xc2\x27\x63\xa0\x68\x88\x67\xeb\x2a\xc0\x9c"
           "\x50\x78\xd6\x12\xc0\x13\x3e\xf8\xa8\xf4\x00\x74\xa8\xe5\x40\xe5"
           "\xb2\x62\xac\x88\x9e\xa4\x4b\xc2\xa2\xbf\x39\xdd\x3d\xdf\xd9\x62"
           "\xda\x62\x4b\x3b\x25\xa0\x36\xd7\xe9\x46\x64\x08\x6a\x77\xf4\x9d"
           "\x81\xcf\x96\x30\x79\x4a\x2a\x15\xd9\xe3\x04\x2b\x8e\x67\xe8\x1d"
           "\xec\x65\x08\x51\xa4\x72\xe1\xd5\xc1\x4d\x7f\xf4",
           140);
    *(uint8_t*)0x20000986 = 0;
    syz_mount_image(0x200000c0, 0x20000100, 0x1a484bc, 0x200007c0, 1, 0,
                    0x20000000);
    break;
  case 3:
    memcpy((void*)0x20000140, "./file2\000", 8);
    syscall(__NR_openat, 0xffffff9c, 0x20000140ul, 0x6a142ul, 0ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  use_temporary_dir();
  loop();
  return 0;
}
