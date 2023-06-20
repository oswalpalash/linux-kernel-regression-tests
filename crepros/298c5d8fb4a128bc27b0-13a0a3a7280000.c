// https://syzkaller.appspot.com/bug?id=3c85d768cb12b6bb2e8801f372a7bae618cbe1cb
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

void execute_one(void)
{
  memcpy((void*)0x20000440, "ext4\000", 5);
  memcpy((void*)0x20000180, "./file1\000", 8);
  memcpy((void*)0x200001c0,
         "inode_readahead_blks=0x0000000000000000,nogrpid,debug_want_extra_"
         "isize=0x0000000000000066,dioread_nolock,max_batch_time="
         "0x0000000000000008,resgid=",
         146);
  sprintf((char*)0x20000252, "0x%016llx", (long long)0);
  memcpy((void*)0x20000264,
         "\x2c\x00\x4b\x5f\xd2\x5e\x1b\x08\x9b\x19\xd0\x64\x60\xfc\x5e\x16\x71"
         "\xcf\xa7\xda\xfd\x6e\x9f\x99\x4b\xce\x8c\x62\x9a\x6e\xa5\xc2\x8a\xd3"
         "\x22\x43\xae\x83\x46\xe3\xbd\x4c\x5d\x2b\x57\xbd\x24\xe0\xd2\xc3\x79"
         "\xfc\x2d\x09\xa9\x08\x00\x00\x00\xd9\xcd\xd1\xe3\x65\xac\xf0\x9e\x10"
         "\x4e\xf4\xc9\xa3\xc2\x50\xc7\xc8\xb0\x86\x7d\xc6\xed\xdd\x41\x05\x60"
         "\x7a\x13\xb3\xfa\x17\xa7\x2e\x99\xf7\xc5\x1a\xd5\x33\x60\xbc\x6e\x59"
         "\x8a\xe3\x14\x60\xff\x8d\x16\x67\x14\x0b\x15\xc5\x93\x8f\x54\xbd\x13"
         "\xd3\xd9\x24\x98\x85\x5c\xed\x1e\x4f\x21\x64\x55\x97\x5f\x22\x93",
         135);
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
  syz_mount_image(/*fs=*/0x20000440, /*dir=*/0x20000180, /*flags=*/0,
                  /*opts=*/0x200001c0, /*chdir=*/0xfd, /*size=*/0x421,
                  /*img=*/0x20000900);
  memcpy((void*)0x20000000, "./file1\000", 8);
  memcpy((void*)0x200000c0, "trusted.overlay.upper\000", 22);
  *(uint8_t*)0x20000480 = 0;
  *(uint8_t*)0x20000481 = 0xfb;
  *(uint8_t*)0x20000482 = 0x76;
  *(uint8_t*)0x20000483 = 0;
  *(uint8_t*)0x20000484 = 0;
  memcpy((void*)0x20000485,
         "\xcd\xff\xff\xff\x5b\x8e\xf0\x8d\x43\x4b\x0b\x00\x00\xb2\x52\x00",
         16);
  memcpy(
      (void*)0x20000495,
      "\x21\x64\x1d\x35\xee\xba\x27\x3f\xb1\x7d\x19\x03\x77\x06\xe3\x2a\xbb\xb7"
      "\x20\xe5\x4a\xb3\x74\x5b\x25\x5a\xd6\xc2\xd1\xf6\x92\xa2\xf0\x8f\x01\xa9"
      "\xce\x1d\x0e\x82\xcb\xbe\x6c\x55\x29\xb2\x55\x4f\x38\x49\xf5\x3f\x0c\x1f"
      "\x1f\x51\xad\xf5\x4a\xc8\x01\xcc\x23\xf7\xeb\xd5\x7c\x66\x6b\x5d\x6d\x62"
      "\x6d\x33\x36\xb5\x1f\x40\xb9\x14\xcc\x8d\x2d\x1c\xb7\xdd\x20\x68\xed\x31"
      "\xf1\x3f\x89\xd8\x7f\x7a\x6c\xdf\x95\xc7\xba\x12\x36\x1f\x2d\xaf\x45\x17"
      "\xfd\x48\x3a\xeb\x65\xce\x34\xbe\x1b\x47\xe0\xac\x06\xdc\xb9\x63\xdb\xa6"
      "\x13\x0e\x70\x9e\x15\xdb\x2d\x9c\x2a\xf0\x7c\xcc\x31\x3e\x3b\x30\x77\x61"
      "\x8a\x34\x5f\x40\xd9\xcb\x4a\x93\x2e\x27\x16\x2e\xee\x1d\x96\xcb\x7c\xd6"
      "\x68\xd9\x4c\x0e\x55\x2c\x86\x91\xae\x20\x55\x0d\x9f\x9a\xf3\xe0\xbb\x0b"
      "\x34\x59\x56\x73\xe2\xab\x14\x80\x75\x8d\xaa\x47\x84\xcb\xf6\xc7\xe1\x30"
      "\x3a\xad\x64\x39\xb7\xaf\x5b\xde\x50\x2c\xce\xc4\x2c\x91\x51\x16\x39\x8f"
      "\xa9\x6c\x01\x16\x07\x48\x0a\xbf\x1a\x6e\xb8\x07\xe7\x58\xfc\xd8\x8f\x35"
      "\xfc\x0e\x21\x7f\x3b\xae\x1d\xa7\xae\x06\xdc\x3d\x3e\x37\xf7\xd1\x98\x17"
      "\x47\xaa\x72\x6e\x4e\xb7\x8b\xc2\x76\xf6\x0a\xad\x2b\x4f\x50\xd7\x87\x7f"
      "\x3d\x3d\x32\xa1\x97\x04\xb1\x3c\x07\x8f\x04\x56\xd5\xe2\x24\x6f\x07\xe1"
      "\x77\x8b\xf1\x6c\x36\x54\x06\x6e\x58\x34\x07\xbd\xd0\xc8\x8d\x33\xd8\x3d"
      "\xe8\xb4\x87\x4e\xfb\x8c\x3c\x8c\xee\x11\x9b\x6c\x90\x49\x4f\x6e\x0d\x9e"
      "\x96\xa7\x48\x2b\x7c\x79\x9c\x2f\x83\x4c\x67\x96\xa8\xc4\x36\xec\x13\x6c"
      "\xc3\xfd\xe6\xfb\x3a\xf8\xc4\x51\x82\xaf\x71\x66\x47\xf7\x0b\x17\xde\x5a"
      "\xa8\x8e\x02\x00\x00\x00\xbc\x95\xdc\x58\xc8\x64\x32\xa6\xfa\x0a\x13\xba"
      "\x41\x0c\xa4\x98\xfb\xd5\x6f\x66\x09\x61\xbf\xf7\x7e\xd7\xf3\x34\xa1\xbd"
      "\x9c\x60\xb7\xb1\x8c\xc1\x03\x07\x79\xc0\xbe\xed\x45\xb7\x31\x71\x61\x12"
      "\x4a\xdf\xc8\x35\x74\x1e\xcd\x20\xa0\x53\xdd\x03\xb7\x9a\x81\x58\x02\x53"
      "\x13\x25\x83\x44\xdc\xfd\x97\xb6\x28\x5e\x1f\x38\x40\xf8\x0f\x52\x48\x97"
      "\x93\x61\xf4\xc3\x7f\xc6\x0f\x68\xcb\x8f\xf2\x7d\xc5\xc1\x83\x8a\xa1\x79"
      "\xc2\x94\x18\xe0\x59\x2f\x8a\xb7\xee\x63\x6c\x92\x7c\x00\x90\x61\x9e\x49"
      "\xb2\x45\xf7\xa2\xd0\xbe\x61\xe0\x6c\xac\xb1\x34\x2d\x42\x8e\x54\x36\xe4"
      "\x67\x7e\x36\x53\xbd\x8b\x21\x3e\x45\xf8\x81\xa4\x17\x65\xf1\x05\xc1\xff"
      "\x9a\x7f\x65\x6e\x87\x0d\x2d\xc7\x1c\x97\x74\x98\x86\x7d\xe1\x93\x27\x3b"
      "\x25\x16\xf1\xd4\x1e\x0b\x4e\x15\x59\x1b\x52\xc0\xa3\xd2\x44\x2c\xa3\x7a"
      "\x6c\xf5\xb4\xdf\x03\x36\xb3\x5e\x76\xfc\x9b\xab\x97\xfe\x2b\xd7\x89\x14"
      "\x3b\x72\xc5\xf9\x25\xa6\x3a\xac\x74\x7a\x00\xff\xb1\xff\xa8\x7c\x3e\x66"
      "\x91\x4b\x92\xab\x80\x73\x04\xb5\x7f\x51\x6d\x42\x1b\x7c\xad\x25\xde\x02"
      "\x97\x82\x41\x28\xc6\xc9\x80\x0f\x6b\xbf\xfa\xe7\x33\x1e\x64\xa5\x16\x81"
      "\xdf\x48\x2c\xcc\x3d\xfd\xa2\x89\xc0\x58\xa2\x86\x09\x38\x6e\x64\xe7\x23"
      "\x2c\x20\x7c\x6d\x51\xb2\x1b\x26\x16\x6d\xf5\x52\x2c\xfc\xee\x89\x8e\x85"
      "\xcf\x42\x40\x73\x01\xd3\x6e\x32\xa9\xed\x3d\x57\x12\x1c\xc4\x66\x6d\x75"
      "\x1c\x81\x7b\xe3\x75\x21\x7d\x56\xaf\xa8\xd4\x70\x85\xa3\xc8\xcb\xa6\x35"
      "\x06\x8b\x36\x60\x98\x65\xcf\x15\xec\x84\x4d\xbc\x5d\x71\x8b\xad\x5c\x3a"
      "\xb8\x76\x35\x61\x09\x9a\x18\x7e\x46\x07\x70\x5d\x78\xd4\x84\x2f\xe9\x69"
      "\xe4\x88\x8a\xb1\x42\x90\x15\xfc\xdc\x21\xe6\x6e\x3d\xe4\xa6\x26\xb3\xad"
      "\x05\x18\xb8\x54\x4f\x58\xc6\xb4\xa2\xd0\x38\x9b\xbb\xb0\x5d\x45\x17\x84"
      "\xf8\x38\x85\x78\xfd\xa7\xdd\x83\x67\x92\xbf\xc9\xee\x91\xec\x95\x9e\xfd"
      "\x30\x7a\x2e\xdd\x24\xdf\x7f\x58\xa0\xeb\x99\x9b\x0e\x93\x22\xee\x81\x09"
      "\x1a\x1a\x70\x94\x94\xc9\x70\x68\x51\xd9\x20\x6a\xe2\xb2\x63\xc6\x4c\xf6"
      "\x4b\xe1\x25\x37\x15\xc4\x87\x4b\xde\xdd\x63\x8c\xab\xa3\x4d\xb2\x09\xd3"
      "\xdd\xf8\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00",
      865);
  syscall(__NR_setxattr, /*path=*/0x20000000ul, /*name=*/0x200000c0ul,
          /*val=*/0x20000480ul, /*size=*/0x376ul, /*flags=*/0ul);
  memcpy((void*)0x20000400, "./file1\000", 8);
  syscall(__NR_creat, /*file=*/0x20000400ul, /*mode=*/0ul);
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
