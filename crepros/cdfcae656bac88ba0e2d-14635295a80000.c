// https://syzkaller.appspot.com/bug?id=a3d085c934448d325eba613246a150873080b331
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

uint64_t r[1] = {0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  memcpy((void*)0x200000c0, "nilfs2\000", 7);
  memcpy((void*)0x200001c0, "./file0\000", 8);
  memcpy(
      (void*)0x20000c40,
      "\x00\xfa\xa5\xa3\x01\x00\xb5\x10\x7c\x2f\x3f\xdb\xb7\x25\x34\x4e\x0a\xfc"
      "\x2e\xad\x9e\xf0\x81\xca\xf0\x78\xa5\xde\x52\x9e\x2c\xd3\xc7\xd2\x65\x87"
      "\x2f\xfe\x18\xbb\xfb\x84\x57\x93\xba\xb9\xc8\x9c\x0b\x02\x36\x67\xe2\xad"
      "\x54\x07\x8f\x17\xbc\x2a\x7a\x32\x6f\x70\x2e\xc4\x18\xe4\xca\x1f\x79\x00"
      "\x00\x00\x00\xff\x08\x02\x0b\x55\xc0\x61\x1b\xc5\xd6\x3e\x1a\x67\xa3\xaf"
      "\x83\x41\xe7\xdc\x07\x87\x63\x64\xa3\x5a\xef\x43\x04\x0e\x9f\xa1\x13\x18"
      "\x50\xd7\x57\x8c\x0a\x9f\xe8\xe0\x8b\xc2\xe0\x45\x7f\x87\xe0\x2f\x4a\x6d"
      "\x01\x8b\xdf\x34\x6c\xf2\x18\x88\x5f\x4b\x1b\xfb\xe8\xe6\xd1\xad\xc0\x33"
      "\x64\x86\xfe\x55\xec\x9b\xb4\xc5\x6a\x8b\xe4\x39\xce\x2c\xbe\x2f\x57\xbb"
      "\x66\x09\xff\x03\x00\x00\x7d\xb3\x59\x23\x4d\xc7",
      174);
  memcpy(
      (void*)0x20001740,
      "\x78\x9c\xec\xdd\x4d\x6c\x5c\x47\x01\x00\xe0\x79\x6b\xaf\x1d\x27\x0d\xde"
      "\x14\x87\x9a\x34\xb4\x09\x85\xb6\xfc\xd4\x6e\x6c\x13\x7e\x22\x68\xaa\xe6"
      "\x42\xd4\x54\xdc\x2a\x55\x5c\xa2\xd4\x2d\x11\x6e\x40\xa4\x12\xb4\xea\x21"
      "\xc9\x89\x1b\xad\xaa\x70\xe5\x47\x9c\x7a\xa9\x00\x21\xd1\x0b\x8a\x7a\xe2"
      "\x12\x89\x46\xe2\xd2\x53\xe1\xc0\x81\x28\x48\x95\x38\xb4\x81\xc4\xc8\xeb"
      "\x99\xf5\xee\x64\x97\xb7\x6b\xc7\x5e\xaf\xf7\xfb\xa4\xf1\xec\xbc\x99\xdd"
      "\x99\xb7\x7e\xfb\xf6\xed\x7b\x6f\x66\x02\x30\xb4\x2a\xf5\xbf\x0b\x0b\xd3"
      "\x45\x08\x97\xdf\x79\xf3\xc4\x3f\x1f\xfe\xc7\xc4\xca\x92\x27\x1a\x25\x6a"
      "\xf5\xbf\xa3\x4d\xa9\x6a\x08\xa1\x88\xe9\xd1\xec\xf5\x3e\x18\x59\x8d\x6f"
      "\x7d\xf8\xda\x99\x76\x71\x11\xe6\xea\x7f\x53\x3a\x3c\x73\xa3\xf1\xdc\x3d"
      "\x21\x84\x0b\xe1\x50\xb8\x12\x6a\xe1\xc0\xe5\x6b\x6f\x5c\x9d\x7b\xfa\xd4"
      "\xc5\x93\x97\x0e\xbf\xf7\xd6\xb1\xeb\x9b\xb3\xf6\x00\x00\x30\x5c\xbe\x73"
      "\xe5\xd8\xc2\xfe\xbf\xfd\xe5\xfe\x7d\x37\xdf\x7e\xe0\x78\x18\x6f\x2c\x4f"
      "\xc7\xe7\xb5\x98\xbe\x27\x1e\xf7\x1f\x8f\x07\xfe\xe9\xf8\xbf\x12\x5a\xd3"
      "\x45\x53\x68\x36\x96\x95\x1b\x8d\xa1\x92\x95\x1b\x69\x53\xae\xb9\x9e\x6a"
      "\x56\x6e\xb4\x43\xfd\x63\xd9\xeb\x56\x3b\x94\x1b\x2f\xa9\x7f\xa4\x69\x59"
      "\xbb\xf5\x86\x41\x96\xb6\xe3\x5a\x28\x2a\x33\x2d\xe9\x4a\x65\x66\x66\xf5"
      "\x37\x79\xa8\xff\xae\x1f\x2b\x66\xce\x9d\x5d\x7a\xe1\x7c\x9f\x1a\x0a\xdc"
      "\x75\xff\x7e\x30\x84\x70\x48\x10\x7a\x0b\xc7\x43\xff\xdb\xb0\x2d\xc2\xc7"
      "\x1f\xad\x7e\x90\xfa\xdd\x8e\x75\x86\xe5\xc9\x3e\xef\x80\x00\xa2\xfc\x7a"
      "\xe1\x1d\x2e\xe4\x67\x16\x36\xa6\xf1\x6a\xa3\xdd\xd5\x7f\xe3\xc9\x4a\xfb"
      "\xe7\x77\x2d\x3f\xe3\x00\x6b\xb6\x7a\xfb\x57\xff\x60\xd5\xff\x9b\x8b\x9b"
      "\x5b\x3f\xc3\x65\xa7\x6e\x4d\x69\xbd\xd2\xe7\xe8\x9e\x98\xce\xaf\x23\xe4"
      "\xf7\x2f\xf5\xfa\xf9\x4f\xaf\x97\x5f\x8f\xa8\x76\xd9\xce\x4e\xd7\x11\x06"
      "\xe5\xfa\x42\xa7\x76\x8e\x6c\x71\x3b\xd6\xab\x53\xfb\xf3\xed\x62\xa7\xfa"
      "\x66\x8c\xd3\xfb\xf0\xad\x2c\xbf\xf9\xf3\x93\xff\x4f\x07\xe5\x7f\x0c\xb4"
      "\xf7\x91\xf3\xff\xc2\x8e\x0e\x07\xd7\x36\xf6\xbe\xb7\x65\xfb\x85\xe5\x7e"
      "\xee\x7c\x80\x6d\x2d\xbf\x6f\x6e\x39\x4a\xf9\xf9\x7d\x7d\x79\xfe\x78\x49"
      "\xfe\xae\x96\xfc\x89\x46\xa9\x94\x3f\x51\xf2\xfc\xdd\x25\xf9\x7b\x4a\xf2"
      "\x61\x98\xfd\xfe\xe5\x9f\x85\xd7\x8b\xb5\xdf\xf9\xf9\x6f\xfa\x5e\xcf\x87"
      "\xa5\xf3\x6c\x7b\x63\xfc\x89\x1e\xdb\x93\x9f\x8f\xec\xb5\xfe\xfc\xbe\xdf"
      "\x5e\x6d\xb4\x7e\x57\xf7\x18\x24\x7f\x3c\xfd\xec\xe2\xd7\x9e\x7f\xae\x58"
      "\xbd\xff\xbf\x68\x6c\xff\xb7\xe3\xf6\x7e\x28\xa6\x6b\xf1\xb3\x75\x25\x16"
      "\x48\xe7\x0b\xf3\xf3\xea\x8d\x7b\xff\x6b\xad\xf5\x54\x3a\x94\xbb\x37\x6b"
      "\xcf\xde\x36\xe5\xeb\x8f\xa7\x5a\xcb\x15\x53\x6b\xaf\x13\x9a\xf6\x33\x77"
      "\xb4\x63\xba\xf5\x79\x93\x9d\xca\x1d\x6c\x2d\x57\xcb\xca\x4d\xc4\xb0\x2b"
      "\x6b\x6f\x7e\x7c\xb2\x3b\x7b\x5e\x3a\xfe\x48\xfb\xd5\xf4\x7e\x8d\x66\xeb"
      "\x5b\xcd\xd6\x63\x2c\x6b\x47\xda\xaf\xec\x8b\x71\xde\x0e\x58\x8f\xb4\x3d"
      "\x76\xba\xff\x3f\x6d\x9f\xd3\xa1\x5a\xbc\x70\x76\x69\xf1\xf1\x98\x4e\xdb"
      "\xe9\x9f\x47\xaa\xe3\x2b\xcb\x8f\x6c\x71\xbb\x81\x8d\xeb\xb6\xff\xcf\x74"
      "\x68\xe9\xff\xd3\x38\xcc\x9d\x0e\xd5\x4a\xf3\x7e\x61\x72\x6d\x79\xd1\xbc"
      "\x5f\xa8\x65\xcb\xe7\x3a\x2c\x9f\x8f\xe9\xf4\x3d\xf7\xbd\x91\x89\xfa\xf2"
      "\x99\x33\x3f\x58\x7a\xfe\x6e\xaf\x3c\x0c\xb9\xf3\xaf\xbc\xfa\xfd\xd3\x4b"
      "\x4b\x8b\x3f\xf2\xc0\x03\x0f\x3c\x68\x3c\xe8\xf7\x9e\x09\xd8\x6c\xb3\x2f"
      "\xbf\xf4\xc3\xd9\xf3\xaf\xbc\xfa\xd8\xd9\x97\x4e\xbf\xb8\xf8\xe2\xe2\xb9"
      "\xf9\xa3\x47\xe7\xe7\xe6\x8e\x7e\x7d\x7e\x61\xb6\x7e\x5c\x3f\xdb\x7c\x74"
      "\x0f\xec\x24\x6b\x5f\xfa\xfd\x6e\x09\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\xd0\xad\x1f\x9f\x3c\x71\xed\xaf\xef\x7e\xf5\xfd\xd5\xfe\xff\x6b\xfd\xff"
      "\x52\xff\xff\x74\xe7\x6f\xea\xff\xff\xd3\xac\xff\x7f\xde\x4f\x3e\xf5\x83"
      "\x4f\xfd\x00\xf7\xb5\xc9\xaf\x97\xc9\x06\x58\x1d\xcb\xca\x55\x3b\xf4\x70"
      "\x9f\xca\xea\xd9\x9f\x3d\xef\x53\x31\x6e\xcc\xe3\x17\xfb\xff\xa7\xea\xf2"
      "\x71\x5d\x53\x7b\xee\xcb\x96\xe7\xe3\xf7\xa6\x72\xd9\x70\x02\x77\x8c\x97"
      "\x32\x96\x8d\x41\x92\xcf\x17\xf8\xd9\x18\x5f\x8a\xf1\xaf\xf3\x15\x84\xad"
      "\x54\x4c\xb4\x5f\x1c\xe3\xb2\xf1\xad\xd3\xb6\x9e\x3a\x04\x1b\x97\x62\x30"
      "\xa5\xff\x5b\xda\x1a\xd2\x38\x26\xa9\xff\x77\xd9\xb8\x4e\xfb\x36\xb9\x7d"
      "\x6c\x8e\xad\xe8\x4e\xd8\xef\x75\x04\xda\xfb\xd7\x5d\x1a\xff\xbb\x7e\x9c"
      "\xb0\x0d\xc6\x33\x16\xfa\x15\x26\xb6\x41\x1b\x84\x5e\xc3\xf2\xb2\x59\x3c"
      "\x80\xed\xa1\xdf\xf3\x7f\xa6\xf3\x9e\x29\x3e\xf7\xa7\x6f\xef\x5a\x09\xa9"
      "\xd8\x8d\x27\x5b\xf7\x97\xf9\xf8\xa5\xb0\x11\xdb\x7d\xfe\x49\xf5\xef\xac"
      "\xf9\x3f\x1b\xf3\xdf\x75\xbd\xff\xcb\x66\xcc\xab\xad\xaf\xde\x8f\x7f\x71"
      "\xfd\xfd\xa6\x6a\xc3\x81\x6e\xeb\xcf\xd7\x3f\x8d\x03\x3d\xd5\x5b\xfd\x37"
      "\x63\xfd\x69\x6d\x1e\x09\xdd\xd5\xbf\xfc\xab\xac\xfe\xfc\x82\x50\x97\xfe"
      "\x93\xd5\xbf\xbb\xcb\xfa\xef\x58\xff\x83\xeb\xab\xff\xbf\xb1\xfe\xf4\xb6"
      "\x3d\xfa\x50\xb7\xf5\xaf\xb6\xb8\xa8\xb4\xb6\x23\x3f\x6f\x3c\x59\xb4\x0e"
      "\xa5\xbf\x37\xab\xff\x56\xb6\xfe\x69\x6c\xcf\x9e\xd7\x7f\x9d\x13\x35\xde"
      "\x8e\xf5\xc3\x30\x1b\x94\x79\x66\x7b\xb5\x05\xf3\xff\x5e\xdd\x73\x17\xe6"
      "\xff\xed\x24\xbf\x0f\xe3\x2b\x31\x9d\x76\x84\xe9\x3e\x87\x7c\xbe\x93\x5e"
      "\x8f\x5f\x3e\x19\xe3\xf4\x3d\xb0\x3f\x7b\xfd\xa2\xe4\xfb\xcd\xfc\xbf\x83"
      "\xed\x1b\x31\x2e\xfb\x3c\xa4\xf9\x7f\xd3\xf6\x58\x6b\x93\xae\x34\xa5\xab"
      "\x6d\xde\xdb\x9d\xba\xaf\x81\x41\xf5\x81\xf9\x7f\x05\x61\x68\xc3\xf2\xf2"
      "\xf2\xe6\x9e\xd0\x2a\xd1\xd7\xca\xe9\xfb\xfb\xdf\xef\xdf\x09\xfd\xae\xbf"
      "\xb7\xf7\x7f\xeb\x5b\x9b\xcf\xff\x9b\x1f\xc3\xe7\xf3\xff\xe6\xf9\xf9\xfc"
      "\xbf\x79\xfe\xae\x92\xfc\x7c\x7e\xbd\x3c\x3f\x9f\xff\x37\x7f\x3f\xf3\xf9"
      "\x7f\xf3\xfc\xfb\xb2\xd7\xcd\xe7\x07\x9e\x2e\xc9\xff\x74\xf6\xba\x79\xfe"
      "\x81\x92\xe7\xdf\x5f\x92\x7f\xb0\x24\xff\x33\x25\xf9\x87\xc7\xff\x7f\xfe"
      "\x03\x25\xcf\x7f\xb0\x24\xff\xde\x92\xfc\x87\x4a\xf2\x3f\x57\x92\xff\xf9"
      "\x92\xfc\x87\x4b\xf2\x1f\x2d\xc9\xff\x42\x49\xfe\x4e\x97\xfa\xa3\x0c\xeb"
      "\xfa\xc3\x30\xcb\xfb\xe7\xf9\xfc\xc3\xf0\x48\xd7\x7f\x3a\x7d\xfe\xa7\x4a"
      "\xf2\x81\xc1\xf5\xf3\xb7\x8f\x3c\xf5\xdc\xef\xbe\x5b\x5b\xed\xff\x3f\xd6"
      "\x38\xc3\x90\xae\xe3\x1d\x8f\xe9\x6a\xfc\x8d\xfb\x93\x98\xce\xaf\x7b\x87"
      "\xa6\xf4\x4a\xde\xbb\x31\xfd\xf7\x2c\xbf\xdf\xe7\x9b\x80\x35\xf9\xf8\x19"
      "\xf9\xf7\xfb\x23\x25\xf9\xc0\xe0\x4a\xf7\x79\xf9\x7c\xc3\x10\x2a\xda\x8f"
      "\xd8\xd3\xed\xb8\x55\x9d\x8e\xf3\x19\x2c\x5f\x8c\xf1\x97\x62\xfc\xe5\x18"
      "\x3f\x16\xe3\x99\x18\xcf\xc6\xf8\x48\x8c\xe7\xb6\xa8\x7d\x6c\x8e\xa7\x7e"
      "\xfb\x87\x63\xaf\x17\x6b\xbf\xf7\x27\xb3\xfc\x6e\xef\x27\xcf\xfb\x03\xe5"
      "\xe3\x44\xcd\x77\xd9\x9e\xfc\xfc\x40\xaf\xf7\xb3\xe7\xe3\xf8\xf5\x6a\xa3"
      "\xf5\xaf\xb3\x3b\x18\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x40\xdf\x54\xea\x7f\x17\x16\xa6"
      "\x8b\x10\x2e\xbf\xf3\xe6\x89\x67\x4f\x9d\x9d\x5d\x59\xf2\x44\xa3\x44\xad"
      "\xfe\x77\xb4\x29\x55\x6d\x3c\x2f\x84\xc7\x63\x3c\x12\xe3\x5f\xc6\x07\xb7"
      "\x3e\x7c\xed\x4c\x73\x7c\x3b\xc6\x45\x98\x0b\x45\x28\x1a\xcb\xc3\x33\x37"
      "\x1a\x35\xed\x09\x21\x5c\x08\x87\xc2\x95\x50\x0b\x07\x2e\x5f\x7b\xe3\xea"
      "\xdc\xd3\xa7\x2e\x9e\xbc\x74\xf8\xbd\xb7\x8e\x5d\xdf\xbc\x77\x00\x00\x00"
      "\x00\x76\xbe\xff\x05\x00\x00\xff\xff\xc0\xfb\x09\x2c",
      2623);
  syz_mount_image(/*fs=*/0x200000c0, /*dir=*/0x200001c0, /*flags=*/0x808,
                  /*opts=*/0x20000c40, /*chdir=*/0xfd, /*size=*/0xa3f,
                  /*img=*/0x20001740);
  memcpy((void*)0x20000000, "./file2\000", 8);
  res = syscall(__NR_creat, /*file=*/0x20000000ul, /*mode=*/0x20ul);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000040, "full", 4);
  *(uint8_t*)0x20000044 = 0x20;
  sprintf((char*)0x20000045, "%020llu", (long long)5);
  *(uint8_t*)0x20000059 = 0x20;
  sprintf((char*)0x2000005a, "%020llu", (long long)0xffffffff);
  *(uint8_t*)0x2000006e = 0;
  syscall(__NR_write, /*fd=*/r[0], /*buf=*/0x20000040ul, /*len=*/0x2ful);
}
int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul, /*prot=*/7ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  use_temporary_dir();
  loop();
  return 0;
}
