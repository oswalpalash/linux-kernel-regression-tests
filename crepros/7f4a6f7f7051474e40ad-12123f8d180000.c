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

void execute_one(void)
{
  intptr_t res = 0;
  memcpy((void*)0x20000440, "ext4\000", 5);
  memcpy((void*)0x20000100, "./file1\000", 8);
  memcpy(
      (void*)0x200002c0,
      "\x69\x6e\x6f\x64\x65\x5f\x72\x65\x61\x64\x61\x68\x65\x61\x64\x5f\x62\x6c"
      "\x6b\x73\x3d\x30\x78\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30"
      "\x30\x30\x30\x2c\x62\x6c\x6f\x63\x6b\x5f\x76\x61\x6c\x69\x64\x69\x74\x79"
      "\x2c\x64\x65\x62\x75\x67\x5f\x77\x61\x6e\x74\x5f\x65\x78\x74\x72\x61\x5f"
      "\x69\x73\x69\x7a\x65\x3d\x30\x78\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30"
      "\x30\x30\x30\x30\x36\x36\x2c\x6e\x6f\x6d\x62\x63\x61\x63\x68\x65\x2c\x6d"
      "\x61\x78\x5f\x62\x61\x74\x63\x68\x5f\x74\x69\x6d\x65\x3d\x30\x78\x30\x30"
      "\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x38\x2c\x6e\x6f\x6d"
      "\x62\x63\x61\x63\x68\x65\x2c\x00\xc1\xb1\xe9\x09\x00\xd9\xd7\xe0\xd2\xb5"
      "\xfb\x78\xf6\xb8\x65\xdc\x98\xea\x6e\x8e\x72\xe5\xf9\xf8\x06\x13\x08\xbd"
      "\xa6\x20\xdc\x19\xfc\x79\xf6\xb0\x45\xd5\xa4\x78\xb2\x6a\x4d\xbb\x01\x75"
      "\x0b\x8b\x52\x73\x13\x53\x0d\x9f\xf5\x20\xa6\x9a\xe1\xc1\x0b\xf9\xfd\x3b"
      "\x1b\x9e\x24\x6e\x3c\x97\x90\xe4\x8c\x95\x61\x9d\x32\xea\xf4\xc1\x76\xd0"
      "\x2c\x99\x0e\x3d\xbb\x15\xac\x49\x65\x96\x70\xdc\x1f\x4a\x7f\x79\x59\xea"
      "\x44\x43\xa4\x11\xe9\x11\x81\x1a\xd3\x94\x56\x3d\x12\xba\xb3\xcb\x76\xc0"
      "\xe6\x32\x45\x9f\x5d\x9d\xb6\xf5\x19\x92\xa5\xa4\x05\xba\xcd\x22\xcf\xe7"
      "\x01\x03\xed\x11\x97\x2c\x66\xb0\x80\x78\xbd\xe3\xa8\x4e\x1b\xda\xed\x78"
      "\x7b\xdc\xed\xed\x01\xfb\x37\xb7\x1d\x41\x81\x9f\x5c\x73\x6e\x2d\x79\x8c"
      "\xb2\x4d\x5f\xa8\x0e\x0d\x2b\x51\xb3\xc8\xb3\xa3\x7e\xfa\x3f\x62\x02\xc4"
      "\xa7\x7e\xb2\xbd\x30\x84\xa9\x2e\xbf\xe8\x73\x3c\x11\x7c\x82\xc6\xd4\xeb"
      "\xd0\x9b\xc9\xae\xa7\xf9\x44\x43\xcf\x0c\xfa\xf1\xbe\x71\x77\xe5",
      376);
  memcpy(
      (void*)0x20000900,
      "\x78\x9c\xec\xdb\xcb\x6f\x1b\xc5\x1f\x00\xf0\xef\xae\x93\xf6\xf7\xeb\x83"
      "\x84\x52\x1e\x2d\x05\x0c\x05\x11\xf1\x48\x9a\xb4\x40\x0f\x5c\x40\x20\x71"
      "\x00\x09\x09\x0e\xe5\x18\x92\xb4\x0a\x75\x1b\xd4\x04\x89\x56\x11\x04\x84"
      "\xca\x11\x55\xe2\x8e\x38\x22\xf1\x17\x70\x82\x0b\x02\x4e\x48\x5c\xe1\x8e"
      "\x2a\x55\x28\x97\x16\x4e\x46\x6b\xef\x26\xae\x63\x87\x3c\x9c\xba\xb0\x9f"
      "\x8f\xb4\xed\x8c\x77\x9c\x99\xaf\x67\xc7\x9e\x9d\xb1\x03\x28\xad\x6a\xf6"
      "\x4f\x12\xb1\x2f\x22\x7e\x8d\x88\xa1\x66\xf6\xe6\x02\xd5\xe6\x7f\x37\x96"
      "\x17\xa7\xfe\x5c\x5e\x9c\x4a\xa2\x5e\x7f\xe3\x8f\xa4\x51\xee\xfa\xf2\xe2"
      "\x54\x51\xb4\x78\xde\xde\x3c\x33\x92\x46\xa4\x9f\x24\x6d\x7f\xb0\x69\xfe"
      "\xe2\xa5\xb3\x93\xb5\xda\xcc\x85\x3c\x3f\xb6\x70\xee\xdd\xb1\xf9\x8b\x97"
      "\x9e\x9e\x3d\x37\x79\x66\xe6\xcc\xcc\xf9\x89\x93\x27\x4f\x1c\x1f\x7f\xee"
      "\xd9\x89\x67\x7a\x12\x67\x16\xd7\xf5\xc3\x1f\xcc\x1d\x39\xf4\xca\x5b\x57"
      "\x5e\x9b\x3a\x75\xe5\xed\x1f\xbf\xce\x9a\xb5\x2f\x3f\xdf\x1a\x47\xaf\x54"
      "\xa3\xda\x29\xf4\x86\xc7\x7a\x5d\x59\x9f\xed\x6f\x49\x27\x03\x7d\x6c\x08"
      "\x9b\x52\x89\x88\xac\xbb\x06\x1b\xe3\x7f\x28\x2a\xb1\xda\x79\x43\xf1\xf2"
      "\xc7\x7d\x6d\x1c\xb0\xa3\xea\xf5\x7a\x7d\x77\xf7\xd3\x4b\x75\xe0\x3f\x2c"
      "\x89\x7e\xb7\x00\xe8\x8f\xe2\x83\x3e\xbb\xff\x2d\x8e\x5b\x34\xf5\xb8\x2d"
      "\x5c\x7b\xa1\x79\x03\x94\xc5\x7d\x23\x3f\x9a\x67\x06\x22\xcd\xcb\x0c\xb6"
      "\xdd\xdf\xf6\x52\x35\x22\x4e\x2d\xfd\xf5\x45\x76\xc4\x0e\xad\x43\x00\x00"
      "\xb4\xfa\x36\x9b\xff\x3c\xd5\x69\xfe\x97\xc6\x3d\x2d\xe5\xee\xc8\xf7\x50"
      "\x86\x23\xe2\xce\x88\x38\x10\x11\x77\x45\xc4\xc1\x88\xb8\x3b\xa2\x51\xf6"
      "\xde\x88\xb8\x6f\x93\xf5\x57\xdb\xf2\x6b\xe7\x3f\xe9\xd5\x2d\x05\xb6\x41"
      "\xd9\xfc\xef\xf9\x7c\x6f\xeb\xe6\xf9\x5f\x31\xfb\x8b\xe1\x4a\x9e\xdb\xdf"
      "\x88\x7f\x30\x39\x3d\x5b\x9b\x39\x96\xbf\x26\x23\x31\xb8\x3b\xcb\x8f\xaf"
      "\x53\xc7\x77\x2f\xfd\xf2\x59\xb7\x73\xad\xf3\xbf\xec\xc8\xea\x2f\xe6\x82"
      "\x79\x3b\xae\x0e\xb4\x2d\xd0\x4d\x4f\x2e\x4c\x6e\x27\xe6\x56\xd7\x3e\x8a"
      "\x38\x3c\xd0\x29\xfe\x64\x65\x27\x20\x89\x88\x43\x11\x71\x78\x8b\x75\xcc"
      "\x3e\xf1\xd5\x91\x6e\xe7\xfe\x39\xfe\x75\xf4\x60\x9f\xa9\xfe\x65\xc4\xe3"
      "\xcd\xfe\x5f\x8a\xb6\xf8\x0b\xc9\xfa\xfb\x93\x63\xff\x8b\xda\xcc\xb1\xb1"
      "\xe2\xaa\x58\xeb\xa7\x9f\x2f\xbf\xde\xad\xfe\x6d\xc5\xdf\x03\x59\xff\xef"
      "\xe9\x78\xfd\xaf\xc4\x3f\x9c\xb4\xee\xd7\xce\x6f\xbe\x8e\xcb\xbf\x7d\xda"
      "\xf5\x9e\x66\xab\xd7\xff\xae\xe4\xcd\x46\x7a\x57\xfe\xd8\xfb\x93\x0b\x0b"
      "\x17\xc6\x23\x76\x25\xaf\xae\x7d\x7c\x62\xf5\xb9\x45\xbe\x28\x9f\xc5\x3f"
      "\x72\xb4\xf3\xf8\x3f\x10\xab\xaf\xc4\xfd\x11\x91\x5d\xc4\x0f\x44\xc4\x83"
      "\x11\xf1\x50\xde\xf6\x87\x23\xe2\x91\x88\x38\xba\x4e\xfc\x3f\xbc\xf8\xe8"
      "\x3b\x5b\x8f\x7f\x67\x65\xf1\x4f\x6f\xaa\xff\x37\x9f\xa8\x9c\xfd\xfe\x9b"
      "\x6e\xf5\x6f\xac\xff\x4f\x34\x52\x23\xf9\x23\x1b\x79\xff\xdb\x68\x03\xb7"
      "\xf3\xda\x01\x00\x00\xc0\xbf\x45\xda\xf8\x0e\x7c\x92\x8e\xae\xa4\xd3\x74"
      "\x74\xb4\xf9\x1d\xfe\x83\xb1\x27\xad\xcd\xcd\x2f\x3c\x79\x7a\xee\xbd\xf3"
      "\xd3\xcd\xef\xca\x0f\xc7\x60\x5a\xac\x74\x0d\xb5\xac\x87\x8e\xe7\x6b\xc3"
      "\x45\x7e\xa2\x2d\x7f\x3c\x5f\x37\xfe\xbc\xf2\xff\x46\x7e\x74\x6a\xae\x36"
      "\xdd\xef\xe0\xa1\xe4\xf6\x76\x19\xff\x99\xdf\x2b\xfd\x6e\x1d\xb0\xe3\xfc"
      "\x5e\x0b\xca\xcb\xf8\x87\xf2\x32\xfe\xa1\xbc\x8c\x7f\x28\x2f\xe3\x1f\xca"
      "\xab\xd3\xf8\xff\xb0\x0f\xed\x00\x6e\x3d\x9f\xff\x50\x5e\xc6\x3f\x94\x97"
      "\xf1\x0f\xe5\x65\xfc\x43\x29\x6d\xe7\x77\xfd\x12\x65\x4e\x44\x7a\x5b\x34"
      "\x43\x62\x87\x12\xfd\x7e\x67\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\xe8\x8d\xbf\x03\x00\x00\xff\xff\xf0\x75\xe6\x67",
      1057);
  syz_mount_image(/*fs=*/0x20000440, /*dir=*/0x20000100, /*flags=*/0,
                  /*opts=*/0x200002c0, /*chdir=*/0, /*size=*/0x421,
                  /*img=*/0x20000900);
  memcpy((void*)0x20000200, "./file1\000", 8);
  memcpy((void*)0x20003180, "user.incfs.metadata\000", 20);
  memcpy(
      (void*)0x200031c0,
      "\x16\xee\x88\x96\x15\xe6\x28\xb8\x96\x49\x82\x30\xe9\x0f\xc5\x74\x7a\x4f"
      "\x11\xd8\x0e\x12\xbe\xcf\x22\xb8\xec\x25\xf3\xb5\x8f\x75\x09\x5f\xed\x7e"
      "\x65\x03\x75\x77\x17\x83\x36\x71\x5f\xb4\x5e\x6b\x4e\xd7\x1a\xc2\xb2\x7c"
      "\x2e\x36\xbc\x83\x6b\x70\xf3\xf6\x50\xc4\xfd\x27\xe6\xf4\xbf\x38\xd0\xc8"
      "\xbe\x32\xe2\x8f\xb5\x49\x6a\x9a\x31\xa6\xcc\x5f\x7b\x50\x1f\xce\x16\xaa"
      "\x4f\x67\x08\x57\x9f\xc8\x60\xf5\x79\xf7\x61\x1f\x0b\x61\xfa\xdb\xa1\x85"
      "\xeb\x11\x50\x47\x01\x42\x4a\x33\xf6\xe6\xa9\x2c\x9a\x47\x7e\x12\xb7\xea"
      "\xb0\xb7\xa6\x1c\x09\x2b\x83\xbd\x26\x3c\x6a\x8a\x11\x48\xc3\xdd\x7c\x41"
      "\x51\xfc\xbc\xa0\x1a\xd2\x14\x35\x73\x60\xc1\xc9\x34\x61\xf1\xf5\x08\x12"
      "\x59\xf0\x60\x65\x7b\x05\xa0\x4e\xff\xb6\x72\x96\x17\x8e\x34\xc0\x8b\x98"
      "\x29\x05\x9c\x88\xcb\x9d\x7b\x49\x38\x9d\x9c\xaf\x38\x22\xf3\xb3\x0f\x67"
      "\x7c\xa2\x31\xcc\x63\x58\x17\x1f\xa0\xd5\xff\x96\xc8\x4b\x05\x50\x67\x12"
      "\xee\x37\x9f\x95\xa7\x6b\x77\x30\x89\x97\x59\x48\xe1\x1f\x79\x13\xe1\xd9"
      "\x39\xf0\x9a\x84\x02\xa0\x69\x90\x5f\x89\x8a\x31\x69\x30\xef\xeb\xd7\x0f"
      "\x67\x0f\xb2\xb1\x13\xb3\x93\x8e\x33\x42\xc1\xef\x87\xb6\xcd\xbd\xa1\xa4"
      "\xa5\x45\x48\x7a\xc7\xb9\xc5\xf3\x00\x34\xf8\x5a\xc8\xa2\x00\xa2\x23\x2e"
      "\xc7\xca\x25\x8c\x26\x53\x44\x70\xfb\xf7\x12\x09\x0f\x75\xbc\x34\x7c\xae"
      "\x89\x49\x61\xa8\xa2\x24\x5a\xc1\x13\x10\xfb\xd6\x09\x4c\x27\xcd\xb5\x17"
      "\x22\xc8\x6e\x7a\xad\xc8\xb9\xd6\x65\x77\x93\x75\xed\xb2\x42\xa2\x89\xd2"
      "\x30\x1c\xef\x1c\x5d\x74\x77\x1e\x4b\xd7\xcf\xd9\x8b\x3e\x77\x22\x2e\x17"
      "\x90\xf6\x86\x61\xae\x25\x65\xc1\xfc\xff\x58\x9a\x97\x6c\xd4\x0b\x5f\x17"
      "\x3a\x21\xf8\x35\x04\x5c\xbb\x3f\xbb\xae\x14\xcf\xe0\xe5\x48\x0b\x08\xf1"
      "\x03\xf6\x21\xb8\xdb\xd0\x2a\xbc\x50\x73\x02\xcf\x5c\x10\xdd\xcc\x5a\x9c"
      "\x49\x26\x0f\xe5\x65\xbd\x76\xd8\x88\x57\xb8\x48\xc9\xb6\x7f\x5e\x6a\x6f"
      "\x3a\x91\xfb\x3b\xde\x86\x9b\xf9\x3d\x44\xd2\x29\xaa\x48\xf6\x08\xe6\x62"
      "\x72\x67\x75\x55\xa2\xa3\x44\x34\x92\x9f\x31\x6c\x36\x4f\xba\xc8\x7a\x98"
      "\x71\x78\x6d\x5b\xbb\x86\x35\xf7\x4d\x76\x24\xde\x78\xe9\x38\xcd\x70\x80"
      "\x56\xb0\x13\xab\xd7\x9f\xdd\xe4\x96\xa5\xf0\x42\xe0\x54\x40\x06\x00\xcf"
      "\x88\x31\xa9\x87\xe6\xf3\xb2\xa9\x4f\xa4\x9e\x4d\xfe\xfe\x82\xb6\x8c\x11"
      "\x3f\xc7\x02\x5c\xff\x2e\xe1\x32\x05\xac\xae\xd5\xbe\x28\x7e\xb0\x1f\xa0"
      "\x9b\xe1\xa0\x22\x05\x4d\xf3\xf0\xa6\x72\x34\x94\xa8\xbc\x25\x1e\xe2\x02"
      "\x20\xf4\xd0\x8f\x3e\xfe\x76\x57\x1b\x30\xb8\x25\x9c\x0d\x5c\x96\x63\x4d"
      "\x22\x95\x1a\x8a\x45\x1d\x57\xbf\x68\x2d\x02\x2c\x22\x1f\x42\xa6\x91\x68"
      "\x56\x93\xc3\xe5\xb4\x7b\x01\xa3\x45\x52\x3e\x28\xe5\xa9\x63\x75\x48\x58"
      "\x89\xb2\x8a\xc6\x65\x71\x4d\x46\x83\x3c\x92\xc5\x26\xe0\xb2\x9a\x50\x36"
      "\xf1\x50\xd0\x7c\x1c\x7a\x17\x3d\x9a\x6f\x21\xe9\x7e\xbd\x9d\x0c\xe4\xec"
      "\x8e\x95\x89\x99\xd6\x56\xa0\x52\x34\xd6\x02\x6e\x2a\x42\x06\x9c\x69\xb3"
      "\x7c\xfc\xe8\x54\xba\x1f\xe0\xc2\xc5\xe9\xf5\x20\x3c\xc0\x02\x3e\xa0\x93"
      "\x15\xc5\x37\x7b\xcd\xdc\x45\xe0\x0d\x96\xa6\x9f\x01\x08\xb0\xd2\x0d\x1f"
      "\x27\xcb\x65\xb4\x89\x5c\x5b\xe9\xe7\x96\xd4\xde\xca\xbe\x3a\xfb\x8f\xf7"
      "\x4a\x31\x95\x68\x4d\x1f\x82\x3d\xea\xc2\x47\x49\x8a\xe1\x23\xdd\x6c\xc3"
      "\xf4\xfe\x55\x98\xb4\xd7\xe5\xcd\xce\xf8\xd5\xc9\xb8\x75\xda\x35\x8a\x53"
      "\xa0\x85\xb6\xc9\x11\xd4\x74\x0c\x00\xcf\xda\x9e\xb7\x49\x32\x40\xb9\xbb"
      "\x0c\xa1\x1e\x87\xe8\x1c\x1e\xfd\x82\xe6\x69\xdb\x3c\xc8\x52\x9b\x87\x4d"
      "\x54\x07\x5a\x78\x64\x9e\x0b\x37\x51\xd4\xe6\x14\x14\x2b\x77\xf0\xf4\xe2"
      "\x08\x37\xbf\x3b\xb7\x9f\xb9\xcd\x98\xf8\x40\x0d\x85\x22\xcc\x47\x52\xcc"
      "\x89\x5a\x33\xaa\x8b\x29\xd8\x20\x1d\xf0\x6a\xfc\x09\xf2\x15\x36\x91\x5e"
      "\x36\xcd\xdd\x81\x1e\xa5\xbb\xae\x4d\x46\xaa\x3f\x3d\x69\x3b\xd4\xd1\x55"
      "\x5a\x24\xf6\x8a\xd5\x85\xd5\x3c\x32\xcf\x60\xe7\xf6\xe3\x6d\x2d\x24\x2a"
      "\x2e\x0e\x4b\x8a\xe6\x83\x20\x2e\x14\x4d\xe9\xa7\x6a\x59\x90",
      897);
  syscall(__NR_setxattr, /*path=*/0x20000200ul, /*name=*/0x20003180ul,
          /*val=*/0x200031c0ul, /*size=*/0x381ul, /*flags=*/0ul);
  memcpy((void*)0x20000280, "cgroup.controllers\000", 19);
  res = syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0x20000280ul,
                /*flags=*/0x275aul, /*mode=*/0ul);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000180, "cgroup.controllers\000", 19);
  res = syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0x20000180ul,
                /*flags=*/0x275aul, /*mode=*/0ul);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_dup3, /*oldfd=*/r[1], /*newfd=*/r[0], /*flags=*/0ul);
  if (res != -1)
    r[2] = res;
  memcpy((void*)0x200001c0, "cpuset.effective_cpus\000", 22);
  res = syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0x200001c0ul,
                /*flags=*/0x275aul, /*mode=*/0ul);
  if (res != -1)
    r[3] = res;
  *(uint32_t*)0x200000c0 = 0;
  *(uint32_t*)0x200000c4 = r[3];
  *(uint64_t*)0x200000c8 = 0;
  *(uint64_t*)0x200000d0 = 8;
  *(uint64_t*)0x200000d8 = 4;
  *(uint64_t*)0x200000e0 = 0;
  syscall(__NR_ioctl, /*fd=*/r[2], /*cmd=*/0xc028660f, /*arg=*/0x200000c0ul);
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
  use_temporary_dir();
  loop();
  return 0;
}
