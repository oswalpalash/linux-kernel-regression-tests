// https://syzkaller.appspot.com/bug?id=47918174613db46c53bdf674594b8eb5db1881c5
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
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
  return munmap(dest, max_destlen);
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

static void reset_loop_device(const char* loopname)
{
  int loopfd = open(loopname, O_RDWR);
  if (loopfd == -1) {
    return;
  }
  if (ioctl(loopfd, LOOP_CLR_FD, 0)) {
  }
  close(loopfd);
}

static long syz_mount_image(volatile long fsarg, volatile long dir,
                            volatile long flags, volatile long optsarg,
                            volatile long change_dir,
                            volatile unsigned long size, volatile long image)
{
  unsigned char* data = (unsigned char*)image;
  int res = -1, err = 0, need_loop_device = !!size;
  char* mount_opts = (char*)optsarg;
  char* target = (char*)dir;
  char* fs = (char*)fsarg;
  char* source = NULL;
  char loopname[64];
  if (need_loop_device) {
    int loopfd;
    memset(loopname, 0, sizeof(loopname));
    snprintf(loopname, sizeof(loopname), "/dev/loop%llu", procid);
    if (setup_loop_device(data, size, loopname, &loopfd) == -1)
      return -1;
    close(loopfd);
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
  if (need_loop_device)
    reset_loop_device(loopname);
  errno = err;
  return res;
}

#define FS_IOC_SETFLAGS _IOW('f', 2, long)
static void remove_dir(const char* dir)
{
  int iter = 0;
  DIR* dp = 0;
retry:
  const int umount_flags = MNT_FORCE | UMOUNT_NOFOLLOW;
  while (umount2(dir, umount_flags) == 0) {
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
    while (umount2(filename, umount_flags) == 0) {
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
      if (umount2(filename, umount_flags))
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
        if (umount2(dir, umount_flags))
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  NONFAILING(memcpy((void*)0x200004c0, "ext4\000", 5));
  NONFAILING(memcpy((void*)0x20000500, "./bus\000", 6));
  NONFAILING(memcpy(
      (void*)0x20000a40,
      "\x78\x9c\xec\xdd\xc1\x4f\x1b\xd9\x19\x00\xf0\x6f\x06\x08\x84\x90\x40\xda"
      "\x1c\xda\xaa\x6d\xd2\x34\x6d\x5a\x45\xb1\xc1\x49\x50\x94\x53\x7a\x69\x55"
      "\x45\x91\xaa\x46\x3d\xf5\x90\x50\x70\x10\xc2\xc6\x08\x9b\x34\xd0\x1c\xc8"
      "\xff\x50\xa9\x91\x7a\x6a\xff\x84\x1e\x2a\xf5\x50\x29\xa7\xbd\xef\x6d\xf7"
      "\xb6\x97\xec\x61\xa5\xec\x6e\xb4\xab\xb0\xd2\x1e\xbc\x9a\xb1\x21\x84\x60"
      "\x60\x37\x04\x4b\xf8\xf7\x93\x9e\x66\xde\x3c\xe3\xef\x3d\xac\x79\xcf\x7c"
      "\x80\x5f\x00\x3d\xeb\x5c\x44\xac\x45\xc4\xb1\x88\xb8\x17\x11\xa3\xed\xeb"
      "\x49\xbb\xc4\xcd\x56\xc9\x1e\xf7\xf2\xc5\xa3\xe9\xf5\x17\x8f\xa6\x93\x68"
      "\x36\xef\x7c\x96\xe4\xed\xd9\xb5\xd8\xf2\x35\x99\x13\xed\xe7\x1c\x8a\x88"
      "\x3f\xfe\x2e\xe2\x2f\xc9\x9b\x71\xeb\x2b\xab\xf3\x53\x95\x4a\x79\xa9\x5d"
      "\x2f\x36\xaa\x8b\xc5\xfa\xca\xea\xe5\xb9\xea\xd4\x6c\x79\xb6\xbc\x50\x2a"
      "\x4d\x4e\x4c\x8e\x5f\xbf\x72\xad\x74\x60\x63\x3d\x5b\xfd\xcf\xf3\xdf\xce"
      "\xdd\xfa\xd3\xff\xff\xf7\x93\x67\xef\xaf\xfd\xfa\x6f\x59\xb7\x46\xda\x6d"
      "\x5b\xc7\x71\x90\x5a\x43\x1f\xd8\x8c\x93\xe9\x8f\x88\x5b\xef\x22\x58\x17"
      "\xf4\xb5\xc7\x73\xac\xdb\x1d\xe1\x3b\x49\x23\xe2\x7b\x11\x71\x3e\xbf\xff"
      "\x47\xa3\x2f\x7f\x35\x01\x80\xa3\xac\xd9\x1c\x8d\xe6\xe8\xd6\x3a\x00\x70"
      "\xd4\xa5\x79\x0e\x2c\x49\x0b\xed\x5c\xc0\x48\xa4\x69\xa1\xd0\xca\xe1\x9d"
      "\x89\xe1\xb4\x52\xab\x37\x2e\xdd\xaf\x2d\x2f\xcc\xb4\x72\x65\x63\x31\x90"
      "\xde\x9f\xab\x94\xc7\xdb\xb9\xc2\xb1\x18\x48\xb2\xfa\x44\x7e\xfe\xaa\x5e"
      "\xda\x56\xbf\x12\x11\xa7\x23\xe2\xef\x83\xc7\xf3\x7a\x61\xba\x56\x99\xe9"
      "\xe6\x1b\x1f\x00\xe8\x61\x27\xb6\xad\xff\x5f\x0e\xb6\xd6\x7f\x00\xe0\x88"
      "\x1b\xea\x76\x07\x00\x80\x43\x67\xfd\x07\x80\xde\x63\xfd\x07\x80\xde\x63"
      "\xfd\x07\x80\xde\x63\xfd\x07\x80\xde\x63\xfd\x07\x80\xde\x63\xfd\x07\x80"
      "\x9e\xf2\x87\xdb\xb7\xb3\xd2\x5c\x6f\x7f\xfe\xf5\xcc\x83\x95\xe5\xf9\xda"
      "\x83\xcb\x33\xe5\xfa\x7c\xa1\xba\x3c\x5d\x98\xae\x2d\x2d\x16\x66\x6b\xb5"
      "\xd9\xfc\x33\x7b\xaa\x7b\x3d\x5f\xa5\x56\x5b\x9c\xb8\x1a\xcb\x0f\x8b\x8d"
      "\x72\xbd\x51\xac\xaf\xac\xde\xad\xd6\x96\x17\x1a\x77\xf3\xcf\xf5\xbe\x5b"
      "\x1e\x38\x94\x51\x01\x00\xbb\x39\x7d\xf6\xe9\x87\x49\x44\xac\xdd\x38\x9e"
      "\x97\xd8\xb2\x97\x83\xb5\x1a\x8e\xb6\xb4\xdb\x1d\x00\xba\xa6\xaf\xdb\x1d"
      "\x00\xba\xc6\x6e\x5f\xd0\xbb\xfc\x8c\x0f\xec\xb0\x45\xef\x6b\x3a\xfe\x89"
      "\xd0\x93\x83\xef\x0b\x70\x38\x2e\xfe\x50\xfe\x1f\x7a\x95\xfc\x3f\xf4\x2e"
      "\xf9\x7f\xe8\x5d\xf2\xff\xd0\xbb\x9a\xcd\xc4\x9e\xff\x00\xd0\x63\xe4\xf8"
      "\x01\xbf\xff\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x6f\x6f\x24\x2f\x49\x5a\x68\xef"
      "\x05\x3e\x12\x69\x5a\x28\x44\x9c\x8c\x88\xb1\x18\x48\xee\xcf\x55\xca\xe3"
      "\x11\x71\x2a\x22\x3e\x18\x1c\x18\xcc\xea\x13\xdd\xee\x34\x00\xf0\x96\xd2"
      "\x4f\x92\xf6\xfe\x5f\x17\x47\x2f\x8c\x6c\x6f\x3d\x96\x7c\x35\x98\x1f\x23"
      "\xe2\xaf\xff\xbc\xf3\x8f\x87\x53\x8d\xc6\xd2\x44\x76\xfd\xf3\xcd\xeb\x8d"
      "\x27\xed\xeb\xa5\x6e\xf4\x1f\x00\xd8\xcb\xc6\x3a\xbd\xb1\x8e\x6f\x78\xf9"
      "\xe2\xd1\xf4\x46\x39\xcc\xfe\x3c\xff\x4d\x6b\x73\xd1\x2c\xee\x7a\xbb\xb4"
      "\x5a\xfa\xa3\x3f\x3f\x0e\xc5\x40\x44\x0c\x7f\x91\xb4\xeb\x2d\xd9\xfb\x95"
      "\xbe\x03\x88\xbf\xf6\x38\x22\x7e\xb0\xd3\xf8\x93\x3c\x37\x32\xd6\xde\xf9"
      "\x74\x7b\xfc\x2c\xf6\xc9\x43\x8d\x9f\xbe\x16\x3f\xcd\xdb\x5a\xc7\xec\x7b"
      "\xf1\xfd\x03\xe8\x0b\xf4\x9a\xa7\xd9\xfc\x73\x73\xa7\xfb\x2f\x8d\x73\xf9"
      "\x71\xe7\xfb\x7f\x28\x9f\xa1\xde\xde\xc6\xfc\xb7\xfe\xc6\xfc\x97\x6e\xce"
      "\x7f\x7d\x1d\xe6\xbf\x73\xfb\x8d\x71\xf5\xbd\xdf\x77\x6c\x7b\x1c\xf1\xa3"
      "\xfe\x9d\xe2\x27\x9b\xf1\x93\x0e\xf1\x2f\xec\x33\xfe\x47\x3f\xfe\xe9\xf9"
      "\x4e\x6d\xcd\x7f\x45\x5c\x8c\x9d\xe3\x6f\x8d\x55\x6c\x54\x17\x8b\xf5\x95"
      "\xd5\xcb\x73\xd5\xa9\xd9\xf2\x6c\x79\xa1\x54\x9a\x9c\x98\x1c\xbf\x7e\xe5"
      "\x5a\xa9\x98\xe7\xa8\x8b\x1b\x99\xea\x37\x7d\x7a\xe3\xd2\xa9\xdd\xc6\x3f"
      "\xdc\x21\xfe\xd0\x1e\xe3\xff\xc5\x3e\xc7\xff\xef\xaf\xef\xfd\xf9\x67\xbb"
      "\xc4\xff\xd5\xcf\x77\x7e\xfd\xcf\xec\x12\x3f\x5b\x13\x7f\xb9\xcf\xf8\x53"
      "\xc3\xff\xed\xb8\x7d\x77\x16\x7f\xa6\xc3\xf8\xf7\x7a\xfd\x2f\xed\x33\xfe"
      "\xb3\x8f\x57\x67\xf6\xf9\x50\x00\xe0\x10\xd4\x57\x56\xe7\xa7\x2a\x95\xf2"
      "\x92\x13\x27\x4e\x9c\x6c\x9e\x74\x7b\x66\x02\xde\xb5\x57\x37\x7d\xb7\x7b"
      "\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x74\x72\x18\xff\x4e\xd4"
      "\xed\x31\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x70\x74\x7d\x13\x00\x00\xff\xff\x2c\x8f"
      "\xd9\x41",
      1190));
  NONFAILING(syz_mount_image(/*fs=*/0x200004c0, /*dir=*/0x20000500, /*flags=*/0,
                             /*opts=*/0x20000240, /*chdir=*/0x21,
                             /*size=*/0x4a6, /*img=*/0x20000a40));
  NONFAILING(memcpy((void*)0x200016c0, "./bus\000", 6));
  NONFAILING(syz_mount_image(/*fs=*/0, /*dir=*/0x200016c0, /*flags=*/0,
                             /*opts=*/0, /*chdir=*/0, /*size=*/0, /*img=*/0));
  NONFAILING(memcpy((void*)0x20000040, "./bus/file0\000", 12));
  syscall(__NR_mkdirat, /*fd=*/0xffffff9c, /*path=*/0x20000040ul, /*mode=*/0ul);
  NONFAILING(memcpy((void*)0x20000340, "./bus/file0\000", 12));
  NONFAILING(memcpy((void*)0x20000400, "security.apparmor\000", 18));
  NONFAILING(memcpy((void*)0x200003c0, "overlay\000", 8));
  syscall(__NR_setxattr, /*path=*/0x20000340ul, /*name=*/0x20000400ul,
          /*val=*/0x200003c0ul, /*size=*/0xe407ul, /*flags=*/0ul);
  NONFAILING(memcpy((void*)0x200001c0, "cgroup.controllers\000", 19));
  res = syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0x200001c0ul,
                /*flags=*/0x275aul, /*mode=*/0ul);
  if (res != -1)
    r[0] = res;
  NONFAILING(memcpy((void*)0x200002c0, "cpuacct.usage_percpu_sys\000", 25));
  res = syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0x200002c0ul,
                /*flags=*/0x275aul, /*mode=*/0ul);
  if (res != -1)
    r[1] = res;
  NONFAILING(*(uint32_t*)0x20000080 = 0);
  NONFAILING(*(uint32_t*)0x20000084 = r[1]);
  NONFAILING(*(uint64_t*)0x20000088 = 7);
  NONFAILING(*(uint64_t*)0x20000090 = 0);
  NONFAILING(*(uint64_t*)0x20000098 = 0);
  NONFAILING(*(uint64_t*)0x200000a0 = 0);
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0xc028660f, /*arg=*/0x20000080ul);
}
int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  install_segv_handler();
  use_temporary_dir();
  loop();
  return 0;
}
