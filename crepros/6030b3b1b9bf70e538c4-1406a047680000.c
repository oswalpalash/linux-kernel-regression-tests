// https://syzkaller.appspot.com/bug?id=82148e8173e51abf3a46e231a7e233950c815ca7
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

void execute_one(void)
{
  memcpy((void*)0x20000000, "hfsplus\000", 8);
  memcpy((void*)0x200000c0, "./bus\000", 6);
  memcpy(
      (void*)0x20000e40,
      "\x78\x9c\xec\xdd\x4f\x68\x1c\xd7\x1d\x07\xf0\xef\xac\x57\xb2\xd6\x05\x47"
      "\x49\xe4\xc4\x2d\x81\x88\x18\xd2\x52\x51\x5b\x7f\x50\x5a\xf5\x52\xb7\x94"
      "\xa2\x43\x28\x21\x3d\xf4\x2c\x6c\x39\x16\x5e\x2b\x41\x52\x8a\x12\x4a\x71"
      "\xff\xd1\x6b\x0f\xa1\xe7\xf4\xa0\x5b\x4f\x85\xde\x0d\xe9\xb9\xbd\xe5\xaa"
      "\x63\xa0\x90\x4b\x0e\x45\x37\x95\x99\x9d\x95\xd6\x92\xb5\x91\x2c\xcb\xbb"
      "\xaa\x3f\x1f\xf1\xf6\xbd\xa7\x37\xef\xcd\x6f\x7f\x3b\x3b\xfb\x47\x88\x09"
      "\xf0\xdc\x5a\x9c\x4a\xf3\x61\x8a\x2c\x4e\xbd\xbd\x59\xf6\xb7\xb7\xe6\xda"
      "\xdb\x5b\x73\x17\xeb\xe1\x76\x92\xb2\xdd\x48\x9a\x9d\x2a\xc5\x6a\x52\x7c"
      "\x96\xdc\x4c\xa7\xe4\x9b\xe5\x2f\xeb\xed\x8b\xa3\xf6\xf3\xc9\xca\xc2\xbb"
      "\x9f\x7f\xb5\xfd\x45\xa7\xd7\xac\x4b\xb5\x7d\x23\x63\x47\xcf\x3b\x9e\x07"
      "\x75\xc9\x64\x92\x0b\x75\x7d\xd8\xc8\x13\xad\x77\xeb\xc8\xf5\x8e\xab\xd8"
      "\xcb\x4c\x99\xb0\x6b\xdd\xc4\xc1\xa0\xed\x1e\xf2\xe0\x24\xd3\x4f\xf9\xbc"
      "\x05\x86\x41\xd1\x79\xdd\x3c\x64\x3c\xb9\x94\x64\xac\x7e\x1f\x90\xfa\xec"
      "\xd0\x78\xb6\xd1\x3d\x7d\x27\x3a\xcb\x01\x00\x00\xc0\x39\xf5\xc2\x4e\x76"
      "\xb2\x99\xcb\x83\x8e\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xce\x93\xfa"
      "\xfa\xff\x45\x5d\x1a\x9d\xba\x59\x4c\xa6\xe8\x5e\xff\x7f\xb4\x1e\x4b\xdd"
      "\x3e\xd7\x1e\x0e\x3a\x00\x00\x00\x00\x00\x00\x00\x00\x78\x0a\x5e\xdf\xc9"
      "\x4e\x36\x73\xb9\xdb\xdf\x2d\xaa\xbf\xf9\xbf\x51\x75\x26\xaa\xdb\x6f\xe4"
      "\xc3\xac\x67\x39\x6b\xb9\x9e\xcd\x2c\x65\x23\x1b\x59\xcb\x4c\x92\xf1\x9e"
      "\x85\x46\x37\x97\x36\x36\xd6\x66\x8e\x31\x73\xf6\xb1\x33\x67\x9f\xcd\xfd"
      "\x05\x00\x00\x00\x00\x00\x00\x80\xff\x53\xbf\xcb\xe2\xfe\xdf\xff\x01\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x60\x18\x14\xc9\x85\x4e\x55\x95\x89"
      "\x4e\xbd\x5b\x8c\xa7\xd1\x4c\x32\x96\x64\xb4\xdc\xee\x41\xf2\xef\x6e\xfb"
      "\x3c\x7b\x38\xe8\x00\x00\x00\x00\xe0\x19\x78\x61\x27\x3b\xd9\xcc\xe5\x6e"
      "\x7f\xb7\xa8\x3e\xf3\xbf\x52\x7d\xee\x1f\xcb\x87\x59\xcd\x46\x56\xb2\x91"
      "\x76\x96\x73\xbb\xfa\x4e\xa0\xf3\xa9\xbf\xb1\xbd\x35\xd7\xde\xde\x9a\xbb"
      "\x5f\x96\xc3\xeb\xfe\xe5\xbf\xbb\x95\x63\x86\x51\xad\x98\xce\x77\x0f\x8f"
      "\xdf\xf3\xd5\x6a\x8b\x56\xee\x64\x25\x23\x49\xae\xe7\x56\xde\x4f\x3b\xb7"
      "\xd3\xa8\x66\x96\xae\x76\xe3\x79\x7c\x5c\xbf\xfd\xb2\x5c\xfb\x47\xb5\x63"
      "\x46\x76\xbb\xae\xcb\x7b\xfe\xe7\xba\x1e\x0e\xe3\x55\x46\x46\xaa\x8c\x94"
      "\x39\x9a\xae\x63\x2b\xb3\xf1\x62\xff\x4c\xfc\xf8\xcb\x53\xed\x69\x26\x8d"
      "\xbd\x6f\x7e\x26\xce\x20\xe7\x97\xea\xba\x9b\xeb\x8b\x27\x8a\xf6\x2c\x1d"
      "\xcc\xc4\x6c\xcf\xd1\xf7\x4a\xff\x4c\x24\xdf\xfe\xc7\xdf\x7e\x79\xb7\xbd"
      "\x7a\xef\xee\x9d\xf5\xa9\xe1\x39\x8c\x9e\xd0\xc1\x4c\xcc\xf5\x64\xe2\xd5"
      "\xe7\x2a\x13\xd3\x55\x26\xae\xec\xf5\x17\xf3\xb3\xfc\x22\x53\x99\xcc\x3b"
      "\x59\xcb\x4a\x7e\x95\xa5\x6c\x64\x39\x93\xf9\x69\xd5\x5a\xaa\x8f\xeb\xf2"
      "\x76\xbc\x7f\xa6\x6e\x3e\xd2\x7b\xe7\xeb\x22\x19\xad\x1f\x97\xce\x59\xf4"
      "\x64\x31\xbd\x51\xce\xed\x7e\xed\x9b\xdb\x59\xce\x5b\xd5\xcf\x6c\x66\xf2"
      "\xfd\xcc\x67\x3e\x0b\x3d\x8f\xf0\x95\x63\x3c\xeb\x1b\x27\x7b\xd6\x5f\xfb"
      "\x4e\xdd\x68\x25\xf9\x53\x5d\x0f\x87\x32\xaf\x2f\xf6\xe4\xb5\xf7\x9c\x3b"
      "\x5e\x8d\xf5\xfe\x66\x3f\x4b\x2f\x3d\xfd\x73\x63\xf3\x5b\x75\xa3\xdc\xc7"
      "\xef\xeb\x7a\x38\x1c\xcc\xc4\x4c\x4f\x26\x5e\xee\x9f\x89\xbf\x56\xef\x13"
      "\xd6\xdb\xab\xf7\xd6\xee\x2e\x7d\x70\xcc\xfd\xbd\x59\xd7\xe5\x01\xfb\xc7"
      "\xa1\x7a\x65\x2e\x8f\x97\x97\xca\x07\xab\xea\x3d\x7a\x74\x94\x63\x2f\x97"
      "\x63\xad\x83\x63\x33\xd5\xd8\xc4\xde\xbc\xc6\xa1\xb1\x2b\x7b\x63\xe3\xb9"
      "\x9c\x95\xfc\x3c\xef\x1f\xf1\x4c\x1d\xad\xdf\xc3\x1d\x5e\x69\xb6\x1a\x7b"
      "\xf5\xb1\x63\x73\xd5\xd8\xd5\x9e\xb1\xd6\xde\xd8\xfe\xfb\x2d\x00\x86\xde"
      "\xa5\xef\x5e\x1a\x6d\xfd\xa7\xf5\xaf\xd6\xa7\xad\x3f\xb4\xee\xb6\xde\x1e"
      "\xfb\xc9\xc5\x1f\x5c\x7c\x6d\x34\x23\xff\x1c\xf9\x61\x73\xfa\xc2\x9b\x8d"
      "\xd7\x8a\xbf\xe7\xd3\xfc\x66\xff\xf3\x3f\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0\xe4\xd6"
      "\x3f\xfa\xf8\xde\x52\xbb\xbd\xbc\x76\x66\x8d\xee\xe5\x9c\xfa\x6f\xdc\xbd"
      "\x02\xd5\xd9\xc7\xf3\xac\x1a\xaf\x3f\xd1\xac\x32\x07\x43\x10\xbc\xc6\x73"
      "\xdf\x18\xf0\x89\x09\x38\x73\x37\x36\xee\x7f\x70\x63\xfd\xa3\x8f\xbf\xb7"
      "\x72\x7f\xe9\xbd\xe5\xf7\x96\x57\x47\xe6\xe7\x17\xa6\x17\xe6\xdf\x9a\xbb"
      "\x71\x67\xa5\xbd\x3c\xdd\xb9\x1d\x74\x94\xc0\x59\xd8\x7f\xd1\x3f\x38\x32"
      "\x3a\x98\x80\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\xaf\xd5\xff\xdf\x00"
      "\x46\xea\xad\x4e\xf7\xef\x04\x03\xbe\x8b\x00\x00\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xc0\x39\xb7\x38\x95"
      "\xe6\xc3\x14\x99\x99\xbe\x3e\x5d\xf6\xb7\xb7\xe6\xda\x65\xe9\xb6\xf7\xb7"
      "\x6c\x26\x69\x24\x29\x7e\x9d\x14\x9f\x25\x37\xd3\x29\x19\xef\x59\xae\x38"
      "\x6a\x3f\x9f\xac\x2c\xbc\xfb\xf9\x57\xdb\x5f\xec\xaf\xd5\xec\x6e\xdf\xe8"
      "\x37\xaf\xbf\x89\xba\x7e\x50\x97\x4c\x26\xb9\x50\xd7\xa7\xf0\xc8\x7a\xb7"
      "\x4e\xbd\x5e\xb1\x77\x0f\xcb\x84\x5d\xeb\x26\x0e\x06\xed\x7f\x01\x00\x00"
      "\xff\xff\x68\xe6\x06\x48",
      1554);
  syz_mount_image(/*fs=*/0x20000000, /*dir=*/0x200000c0, /*flags=*/0x200008,
                  /*opts=*/0x20000180, /*chdir=*/0x81, /*size=*/0x612,
                  /*img=*/0x20000e40);
  memcpy((void*)0x20000080, "./bus\000", 6);
  memcpy((void*)0x200001c0,
         "./"
         "file2aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\000",
         252);
  syscall(__NR_symlink, /*old=*/0x20000080ul, /*new=*/0x200001c0ul);
  memcpy((void*)0x20000b40,
         "./"
         "file2aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\000",
         252);
  syscall(__NR_creat, /*file=*/0x20000b40ul, /*mode=*/0ul);
  memcpy((void*)0x20000b00, "./bus\000", 6);
  memcpy((void*)0x20000300, "./file2\000", 8);
  syscall(__NR_rename, /*old=*/0x20000b00ul, /*new=*/0x20000300ul);
  memcpy((void*)0x20000000, "hfsplus\000", 8);
  memcpy((void*)0x20000100, "./file1\000", 8);
  memcpy(
      (void*)0x20003180,
      "\x78\x9c\xec\xdd\x4d\x6c\x1c\x57\x1d\x00\xf0\xff\xac\x77\xd7\xde\x80\x52"
      "\xa7\x4d\xd2\x80\x2a\x61\x35\x52\x41\x58\x24\x8e\x2d\xa7\x18\x21\x35\x20"
      "\x04\x96\xa8\x50\x29\x07\xc4\x71\x95\x38\xcd\xca\x1b\xb7\xb2\xb7\xc8\x89"
      "\x2a\x48\xf9\x96\xb8\x70\xe8\x9d\x22\xe1\x5b\x2f\x20\x71\x0f\x2a\xe2\x08"
      "\x9c\x7a\xf5\xb1\x12\x12\x97\x9e\x02\x48\x2c\x9a\x8f\xfd\xb2\x1d\x7b\xd7"
      "\x75\xbc\xb6\xfa\xfb\x45\xb3\xef\xcd\xbc\x8f\x79\xef\xbf\x33\x3b\x3b\xb3"
      "\x8a\x1c\xc0\x27\xd6\xf2\x6c\x94\x1f\x46\x12\xcb\xb3\x2f\x6f\xa6\xeb\xdb"
      "\x5b\x0b\xcd\xed\xad\x85\xbb\x9d\x7c\x44\x4c\x46\x44\x29\xa2\x9c\x27\x91"
      "\xac\x45\x24\xef\x47\xdc\x88\x7c\x89\xcf\xa4\x1b\x8b\xee\x92\xc7\xed\xe7"
      "\x9d\xc6\xd2\xab\x1f\x7c\xb4\xfd\x61\xbe\x56\x2e\x96\xac\x7e\x69\xbf\x76"
      "\xbb\x5c\x2f\xed\xb1\xf1\x41\xb1\xc4\x4c\x44\x4c\x14\xe9\xc7\x30\xd0\xdf"
      "\xcd\x1d\xfd\x55\x47\xee\x2e\xe9\xce\x30\x0d\xd8\xe5\x4e\xe0\x60\xdc\x2a"
      "\x11\xd1\x1e\xf0\x83\x8b\xbd\x92\x03\x0d\x7f\xde\x02\x27\x56\x92\x5f\x37"
      "\x77\x9d\xd0\xd3\x11\x67\x22\x62\xaa\xf8\x1e\x90\x5f\x15\xf3\x6b\xf6\xa9"
      "\xf6\x60\xdc\x03\x00\x00\x00\x80\x63\xf0\xd4\xaf\xb3\x5b\xf8\xb3\xe3\x1e"
      "\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x9c\x26\xc5\xdf\xff\x4f\x8a\xa5"
      "\xd4\xc9\xcf\x44\xd2\xf9\xfb\xff\xd5\x62\x5b\x14\xf9\x53\xed\xe1\xb8\x07"
      "\x00\x00\x00\x00\x00\x00\x00\x00\xa3\xfb\xf6\xa7\x77\x6c\xf8\xdc\xa3\x78"
      "\x14\x9b\x71\xb6\xb3\xde\x4e\xb2\xdf\xfc\x9f\xcf\x56\xce\x67\xaf\x9f\x8a"
      "\x37\x63\x23\x56\x62\x3d\xae\xc4\x66\xd4\xa3\x15\xad\x58\x8f\x6b\x11\x31"
      "\x9d\x95\x57\xb2\xd7\xea\x66\xbd\xd5\x5a\xbf\x36\x44\xcb\xf9\x6e\xcb\xe8"
      "\x6b\x39\x3f\xe4\x0c\x6a\x87\x9f\x3c\x00\x00\x00\x00\x00\x00\x00\x9c\x16"
      "\xe5\xd1\x9b\xfc\x24\x96\x7b\xbf\xff\x03\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\xc0\x49\x90\x44\x4c\xe4\x49\xb6\x9c\xef\xe4\xa7\xa3\x54\x8e\x88"
      "\xa9\x88\xa8\xa6\xf5\x1e\x44\xfc\xbd\x93\x3f\x79\xbe\x1a\xf1\xdb\x3f\xf7"
      "\x6f\x68\xff\xb7\x9d\xd9\x55\xf3\xe1\x71\x0e\x0b\x00\x00\x00\xc6\xe4\xa9"
      "\x47\xf1\x28\x36\xe3\x6c\x67\xbd\x9d\x64\xf7\xfc\x17\xb3\xfb\xfe\xa9\x78"
      "\x33\xd6\xa2\x15\x8d\x68\x45\x33\x56\xe2\x56\xf6\x2c\x20\xbf\xeb\x2f\x6d"
      "\x6f\x2d\x34\xb7\xb7\x16\xee\xa6\xcb\xee\x7e\xbf\xf6\xaf\x91\x86\x91\xf5"
      "\x18\xf9\xb3\x87\xbd\xf7\x7c\x29\xab\x51\x9b\x8c\x68\x64\x5b\xae\xc4\xcd"
      "\x78\x3d\x9a\x71\x2b\x4a\x59\xcb\xd4\xa5\xce\x78\xf6\x1e\xd7\xdb\xe9\x98"
      "\x92\x97\x0a\x43\x8e\xec\x56\x91\xa6\x33\xff\x4d\x54\xf6\xa8\xf1\xad\xb7"
      "\x46\x9a\xea\x01\x92\xa1\x6b\x4e\x67\x11\x49\x47\x94\x47\x64\xae\x68\x9b"
      "\x46\xe3\xdc\xfe\x91\x18\xf1\xdd\xe9\xec\xe9\x76\xb1\xa7\x6b\x51\xea\x3e"
      "\xf9\x39\x7f\x94\x31\xdf\xcc\x93\x17\x7f\x9f\xa7\xe9\x7c\x7e\x31\x52\x4c"
      "\x9e\xb4\x9d\x91\x98\xef\x3b\xfa\x2e\xee\x1f\x89\x88\xcf\xff\xe9\xbd\xef"
      "\xdf\x69\xae\xad\x4e\xde\xde\x98\x3d\x39\x53\x3a\x48\xdf\x23\xbe\xc9\xbe"
      "\x27\x68\x3b\x23\xb1\xd0\x17\x89\x67\x87\x88\xc4\x77\x7f\x15\xb1\x7a\xe7"
      "\x54\x45\xe2\x31\xe6\xb2\x48\x5c\xe8\xae\x2f\xc7\x37\xe3\x7b\x31\x1b\x33"
      "\xf1\x4a\xac\x47\x23\x7e\x18\xf5\x68\xc5\x4a\xcc\xc4\x37\xa2\x1e\x13\x51"
      "\x9f\xc8\xa7\x9c\xbe\x4e\xef\x1f\xa9\x1b\x03\x6b\xaf\x1c\x34\x92\x6a\xf6"
      "\xbe\x54\x8a\x4f\xd1\xe1\xc7\xd4\x8a\x7a\x3c\x9f\xb5\x3d\x1b\x8d\xf8\x4e"
      "\xbc\x1e\xb7\x62\x25\xae\x67\xff\xe6\xe3\x5a\xbc\x18\x8b\xb1\x18\x4b\x7d"
      "\xef\xf0\x85\x21\xce\xfa\xd2\x68\x9f\xb4\x97\xbf\xd0\x77\xa4\xfd\x32\x22"
      "\x6a\xef\x0d\xd7\xf0\x28\xed\xfd\x3c\x3b\xdd\x7a\xae\x7b\x75\xea\x3f\xea"
      "\xe7\xb2\xf3\xe0\xdc\xc0\x96\x5e\x94\x9e\x3e\xfa\xeb\x51\xf9\xb3\x45\x26"
      "\xdd\xc7\x4f\x8b\xf4\x24\x78\xbb\x48\x07\xaf\x12\x9d\xd1\x3d\xb3\x7f\x24"
      "\x7e\x97\x7d\xac\x6c\x34\xd7\x56\xd7\xef\xd4\xdf\x18\x72\x8f\x2f\x14\x69"
      "\x7a\x1e\xfd\xfc\x44\x5d\x25\xd2\xe3\xe5\xe9\xf4\xcd\xca\xd6\x06\x8f\x8e"
      "\xb4\xec\x99\x9d\x65\x53\x79\xbc\xaa\xc5\x2f\x2e\x79\xd9\xe0\x15\x37\x2d"
      "\xbb\xd0\x2d\x3b\xe8\x4c\xad\x16\x07\xeb\xee\x9e\xe6\xb3\x9e\x9e\xdd\xb3"
      "\x6c\x21\x2b\xbb\xd4\x57\x56\xeb\x96\x5d\x89\x9b\x37\xf2\xef\x5b\x3d\x53"
      "\xc7\x19\x54\x00\x86\x77\xe6\x8b\x67\xaa\xb5\x7f\xd6\xfe\x56\x7b\xb7\xf6"
      "\xb3\xda\x9d\xda\xcb\x53\x5f\x9f\xfc\xf2\xe4\x73\xd5\xa8\xfc\xa5\xf2\x95"
      "\xf2\xdc\xc4\x0b\xa5\xe7\x92\x3f\xc6\xbb\xf1\xa3\xde\xfd\x3f\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x70\x78\x1b\xf7\xee\xaf\xd6\x9b\xcd\x95\xf5\x1d\x99\x76\xbb\xfd"
      "\xe3\xc7\x14\x1d\x53\x26\xe2\xa0\x3a\x95\x38\xb8\xce\x93\xc9\x54\x23\x22"
      "\xcb\x94\x3b\x99\xd1\xfa\x99\x1c\xaa\x72\xb5\xf7\xee\xbc\xf4\x87\x8f\x33"
      "\xe6\xca\xa8\xad\x22\x8e\x24\x50\xe5\xe2\x20\xbb\x77\x7f\xf5\xdf\xed\x76"
      "\x7b\x4c\x07\xd2\x40\xa6\xb2\xcf\x31\xdf\xcb\xb4\x0b\x63\x1d\xea\x21\x32"
      "\xff\x69\x1f\x5d\x87\x63\xfe\x60\x02\x9e\xb8\xab\xad\xbb\x6f\x5c\xdd\xb8"
      "\x77\xff\x4b\x8d\xbb\xf5\xd7\x56\x5e\x5b\x59\x5b\x5a\x5c\x5c\x9a\x5b\x5a"
      "\xbc\xfe\x8f\xab\xb7\x1b\xcd\x95\xb9\xfc\x75\xdc\xa3\x04\x9e\x84\xfe\xef"
      "\xdb\x00\x00\x00\x00\x00\x00\x00\x00\x00\xc0\xe9\x70\x3c\xff\x93\x67\x4f"
      "\xff\x3b\xe6\xa9\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\xa7\xd4\xf2\xec\x64\x91\xbb\x32\x97\xbe"
      "\x6e\x6f\x2d\x34\xd3\xa5\x93\xef\x56\xcc\xaa\x95\x22\x22\x79\x2b\x22\x79"
      "\x3f\xe2\x46\xe4\x4b\x4c\xf7\x75\x97\x3c\x6e\x3f\xef\x34\x96\x5e\xfd\xe0"
      "\xa3\xed\x0f\xf3\xb5\x72\xb1\x64\xf5\x4b\x03\xed\x2a\x87\x99\xc5\x83\x62"
      "\x89\x99\x88\x98\x28\xd2\x7e\x53\xa3\xf6\xf7\xd7\x5e\x7f\x37\x8b\xf4\x50"
      "\x23\xcb\x24\xdd\x19\xa6\x01\xbb\xdc\x09\x1c\x8c\xdb\xff\x03\x00\x00\xff"
      "\xff\xcd\xa3\x15\xb9",
      1697);
  syz_mount_image(/*fs=*/0x20000000, /*dir=*/0x20000100, /*flags=*/0x412,
                  /*opts=*/0x200003c0, /*chdir=*/1, /*size=*/0x6a1,
                  /*img=*/0x20003180);
  memcpy((void*)0x20000040, "./file1\000", 8);
  syscall(__NR_creat, /*file=*/0x20000040ul, /*mode=*/0ul);
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
