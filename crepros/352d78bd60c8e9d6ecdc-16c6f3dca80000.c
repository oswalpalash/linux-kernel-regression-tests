// https://syzkaller.appspot.com/bug?id=47fa0c6211916f91a75d56a8298470f14c7fe032
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
  memcpy((void*)0x20000480,
         "inode_readahead_blks=0x0000000000000000,nogrpid,debug_want_extra_"
         "isize=0x0000000000000066,dioread_nolock,max_batch_time="
         "0x0000000000000008,resgid=",
         146);
  sprintf((char*)0x20000512, "0x%016llx", (long long)0);
  memcpy(
      (void*)0x20000524,
      "\x2c\x00\x4b\x5f\xd2\x5e\x1b\x08\x9b\x19\xd0\x64\x60\xfc\x5e\x16\x71\xec"
      "\x2a\xb1\x24\x75\xad\x82\x91\x12\xc3\xd8\x39\xd9\xcf\xa7\xda\xfd\x6e\x9f"
      "\x99\x4b\xce\x8c\x62\x9a\x6e\xa5\xc2\x8a\xd3\x22\x43\xae\x83\x46\xe3\xbd"
      "\x4c\x5d\x2b\x57\xbd\x24\xe0\xd2\xc3\x79\xfc\x2d\x09\xa9\x08\x00\xe0\x00"
      "\xd9\x3a\xe0\x00\x00\x00\x00\x00\x00\x4e\xf4\xc9\xa3\xc2\x50\xc7\xc8\xb0"
      "\x86\x7d\xc6\xed\xdd\x41\x05\x60\x7a\x13\xb3\xfa\x17\xa7\x2e\x99\xf7\xc5"
      "\x1a\xd5\x33\x60\xbc\x6e\x59\x8a\xe3\x14\x60\xff\x8d\x16\x67\x14\x0b\x15"
      "\xc5\x93\x8f\x54\xbd\x13\xd3\xd9\x24\x98\x85\x5c\xed\x1e\x4f\x21\x64\x55"
      "\x97\x5f\x22\x93\x35\x94\x10\x6f\xab\xc6\xb0\x15\x28\x2f\xda\xb6\x38\xd6"
      "\xe2\xc7\xaf\xd1\xee\xf1\x8b\xda\x4a\x2d\x0a\x7b\x0a\x41\x0e\x31\x2d\x3d"
      "\xc1\xeb\xb9\x3b\x13\x20\xec\x72\x55\x1b\xad\xf4\x7f\x7a\xa4\x06\xb1\x86"
      "\x39\x63\xdd\x8c\x91\x83\x48\x50\x40\x07\x38\x1c\xe7\xb0\x04\x00\x00\x00"
      "\x75\xe0\xb5\x1b\xba\x6d\x0c\x2a\x2f\x8c\x5f\x54\x57\x4d\xf1\x36\x9b\x10"
      "\xd5\x6a\xbe\xda\x6e\x9d\xdc\xcc\xaa\x86\x30\x8f\x13\xaa\x8c\x69\x5e\xef"
      "\x32\x08\x56\xeb\x2d\x44\xdc\xa0\xe3\xa1\x40\xdd\xff\x99\xd6\xbf\x00\x6c"
      "\xa4\x4f\x76\x12\x00\x93\x13\x03\xcc\xfc\x21\x91\x3f\x23\x6c\xfe\xe5\x4d"
      "\x6f\xe1\x9e\x5c\x1a\x19\x93\x38\xd4\x77\xd2\x23\x7c",
      301);
  memcpy(
      (void*)0x200008c0,
      "\x78\x9c\xec\xdc\xcb\x6f\x1b\x45\x18\x00\xf0\x6f\xd7\x71\x0a\x7d\x90\x50"
      "\xca\xa3\xa5\x80\xa1\x20\x22\x1e\x49\x93\x16\xe8\x81\x0b\x08\x24\x90\x40"
      "\x42\x82\x43\x39\x86\x24\xad\x4a\xdd\x06\x35\x41\xa2\x55\x80\x80\x50\x39"
      "\xa2\x4a\xdc\x11\x47\x24\xfe\x02\x4e\x70\x41\xc0\x09\x89\x2b\xdc\x51\xa5"
      "\x0a\xe5\xd2\xc2\xc9\x68\xed\xdd\xc4\x75\xe2\x90\x87\x13\x87\xfa\xf7\x93"
      "\xb6\x9d\xf1\x8e\x33\xf3\x79\x76\x9c\xd9\x19\x3b\x01\xf4\xac\x4a\xf6\x4f"
      "\x12\xb1\x37\x22\x7e\x8f\x88\x81\x46\xf6\xe6\x02\x95\xc6\x7f\x37\x16\xe6"
      "\x26\xfe\x5e\x98\x9b\x48\xa2\x56\x7b\xf3\xaf\xa4\x5e\xee\xfa\xc2\xdc\x44"
      "\x51\xb4\x78\xde\x9e\x3c\x33\x94\x46\xa4\x9f\x25\x2d\x3f\xb0\x61\xe6\xe2"
      "\xa5\xb3\xe3\xd5\xea\xd4\x85\x3c\x3f\x32\x7b\xee\xbd\x91\x99\x8b\x97\x9e"
      "\x3e\x73\x6e\xfc\xf4\xd4\xe9\xa9\xf3\x63\x27\x4e\x1c\x3f\x36\xfa\xdc\xb3"
      "\x63\xcf\x74\x24\xce\x2c\xae\xeb\x87\x3e\x9a\x3e\x7c\xf0\x95\xb7\xaf\xbc"
      "\x3e\x71\xf2\xca\x3b\x3f\x7f\x9b\x35\x6b\x6f\x7e\xbe\x39\x8e\x4e\xa9\x44"
      "\x65\xa5\xd0\xeb\x1e\xeb\x74\x65\x5d\xb6\xaf\x29\x9d\xf4\x75\xb1\x21\xac"
      "\x4b\x29\x22\xb2\xee\x2a\xd7\xc7\xff\x40\x94\x62\xa9\xf3\x06\xe2\xe5\x4f"
      "\xbb\xda\x38\x60\x4b\xd5\x6a\xb5\xda\xae\xf6\xa7\xe7\x6b\xc0\x2d\x2c\x89"
      "\x6e\xb7\x00\xe8\x8e\xe2\x17\x7d\x76\xff\x5b\x1c\xdb\x34\xf5\xd8\x11\xae"
      "\xbd\xd0\xb8\x01\xca\xe2\xbe\x91\x1f\x8d\x33\x7d\x91\xe6\x65\xca\x2d\xf7"
      "\xb7\x9d\x54\x89\x88\x93\xf3\xff\x7c\x95\x1d\xb1\x45\xeb\x10\x00\x00\xcd"
      "\xbe\xcf\xe6\x3f\x4f\xd5\xe7\x1d\xaf\xf6\x47\x34\xcd\xff\xd2\xb8\xa7\xa9"
      "\xdc\x1d\xf9\x1e\xca\x60\x44\xdc\x19\x11\xfb\x23\xe2\xae\x88\x38\x10\x11"
      "\x77\x47\xd4\xcb\xde\x1b\x11\xf7\xad\xb3\xfe\x4a\x4b\x7e\xf9\xfc\x27\xbd"
      "\xba\xa1\xc0\xd6\x28\x9b\xff\x3d\x9f\xef\x6d\xdd\x3c\xff\x2b\x66\x7f\x31"
      "\x58\xca\x73\xfb\xea\xf1\x97\x93\x53\x67\xaa\x53\x47\xf3\xd7\x64\x28\xca"
      "\xbb\xb2\xfc\xe8\x2a\x75\xfc\xf0\xd2\x6f\x5f\xb4\x3b\xd7\x3c\xff\xcb\x8e"
      "\xac\xfe\x62\x2e\x98\xb7\xe3\x6a\x5f\xcb\x02\xdd\xe4\xf8\xec\xf8\x66\x62"
      "\x6e\x76\xed\x93\x88\x43\x7d\x2b\xc5\x9f\x2c\xee\x04\x24\x11\x71\x30\x22"
      "\x0e\x6d\xb0\x8e\x33\x4f\x7c\x73\xb8\xdd\xb9\xff\x8e\x7f\x15\x1d\xd8\x67"
      "\xaa\x7d\x1d\xf1\x78\xa3\xff\xe7\xa3\x25\xfe\x42\xb2\xfa\xfe\xe4\xc8\x6d"
      "\x51\x9d\x3a\x3a\x52\x5c\x15\xcb\xfd\xf2\xeb\xe5\x37\xda\xd5\xbf\xa9\xf8"
      "\x3b\x20\xeb\xff\xdd\x2b\x5e\xff\x8b\xf1\x0f\x26\xcd\xfb\xb5\x33\xeb\xaf"
      "\xe3\xf2\x1f\x9f\xb7\xbd\xa7\xd9\xe8\xf5\xdf\x9f\xbc\x55\x4f\xf7\xe7\x8f"
      "\x7d\x30\x3e\x3b\x7b\x61\x34\xa2\x3f\x79\x6d\xf9\xe3\x63\x4b\xcf\x2d\xf2"
      "\x45\xf9\x2c\xfe\xa1\x23\x2b\x8f\xff\xfd\xb1\xf4\x4a\xdc\x1f\x11\xd9\x45"
      "\xfc\x40\x44\x3c\x18\x11\x0f\xe5\x6d\x7f\x38\x22\x1e\x89\x88\x23\xab\xc4"
      "\xff\xd3\x8b\x8f\xbe\xbb\xf1\xf8\xb7\x56\x16\xff\xe4\xba\xfa\x7f\xfd\x89"
      "\xd2\xd9\x1f\xbf\x6b\x57\xff\xf2\xf8\x3f\x8c\xe5\xfd\x7f\xbc\x9e\x1a\xca"
      "\x1f\x59\xcb\xfb\xdf\x5a\x1b\xb8\x99\xd7\x0e\x00\x00\x00\xfe\x2f\xd2\xfa"
      "\x67\xe0\x93\x74\x78\x31\x9d\xa6\xc3\xc3\x8d\xcf\xf0\x1f\x88\xdd\x69\x75"
      "\x7a\x66\xf6\xc9\x53\xd3\xef\x9f\x9f\x6c\x7c\x56\x7e\x30\xca\x69\xb1\xd2"
      "\x35\xd0\xb4\x1e\x3a\x9a\xaf\x0d\x17\xf9\xb1\x96\xfc\xb1\x7c\xdd\xf8\xcb"
      "\xd2\xed\xf5\xfc\xf0\xc4\x74\x75\xb2\xdb\xc1\x43\x8f\xdb\xd3\x66\xfc\x67"
      "\xfe\x2c\x75\xbb\x75\xc0\x96\xf3\x7d\x2d\xe8\x5d\xc6\x3f\xf4\x2e\xe3\x1f"
      "\x7a\x97\xf1\x0f\xbd\xcb\xf8\x87\xde\xb5\xd2\xf8\xff\xb8\x0b\xed\x00\xb6"
      "\x5f\x63\xfc\xb7\xfb\x53\x75\xc0\xad\xcc\xfc\x1f\x7a\x97\xf1\x0f\xbd\xcb"
      "\xf8\x87\x9e\xb4\x99\xef\xf5\x4b\x2c\x25\xca\x3b\xa3\x19\xdb\x98\x88\x74"
      "\x47\x34\x43\x62\x8b\x12\xdd\x7e\x67\x02\x00\x00\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\xe8\x8c\x7f\x03\x00\x00\xff\xff\x5b\x08\xe3\x33",
      1077);
  syz_mount_image(/*fs=*/0x20000440, /*dir=*/0x20000180, /*flags=*/0,
                  /*opts=*/0x20000480, /*chdir=*/0xfd, /*size=*/0x435,
                  /*img=*/0x200008c0);
  memcpy((void*)0x20000040, "./file1\000", 8);
  memcpy((void*)0x20000080, "user.incfs.metadata\000", 20);
  memcpy(
      (void*)0x20001080,
      "\x3b\xca\x3e\x8e\xec\x60\xdb\x6d\xa7\x28\xee\x8e\x29\xfa\x00\xfa\x21\x03"
      "\x9b\x37\x80\xdd\x17\x54\x0b\xcd\xff\x62\xd7\x13\x4b\xa5\x3e\x25\xf5\x9a"
      "\xf9\x2d\xd4\x65\xa8\x39\x65\x8e\xad\x1f\x48\xc4\x42\xf9\x96\xdd\x68\x49"
      "\x56\xb1\x12\x67\xdb\x59\xfd\x65\x72\x81\xe9\x0a\x34\xe6\x3a\x4a\x3f\xeb"
      "\x92\xde\x49\xe5\x7c\xea\x7f\xec\x40\xeb\x97\x08\xf4\xc4\x39\x65\xc6\xae"
      "\x13\x8b\x7e\x6a\xe8\x40\x58\x14\x26\x34\xaf\xb2\x2b\xe1\xc2\x62\xe7\x5b"
      "\x98\x69\x2e\xbb\xd5\x02\x76\x6a\xf2\x62\xe3\x85\x70\x31\xe2\x8c\x9e\x0e"
      "\xa0\xac\xd5\xd9\x95\x31\xbf\x21\xf4\x9f\x9a\xa6\xf5\x44\x58\x2b\x97\x84"
      "\x74\x0e\xb7\xfe\x03\x6c\x44\x52\x06\x6c\x54\xd0\x84\x5e\xdd\x55\xb9\x3b"
      "\x80\x5c\xdc\xbe\x28\x16\x63\xc9\x3f\xbd\xa2\x0a\xed\x20\xa1\x8e\x3f\x1d"
      "\x82\xb1\x9a\xad\x5b\xe2\x1d\xe0\x78\x07\x76\x63\xca\xbf\x60\x7f\x2f\xf2"
      "\x84\x6e\x36\xec\xd7\xb7\xa6\x62\x73\x1a\x5f\x5a\x67\xaa\xaa\x6e\x78\x5e"
      "\xc8\x93\xd6\x0c\x85\xb1\x5c\x0e\x9c\x0a\x87\x52\xbc\xa0\x84\x48\x8a\xed"
      "\xb9\x45\x40\x4c\xe9\x90\x74\x1b\x52\x2e\x99\x15\x4e\x36\x25\xfe\x9d\x3f"
      "\x9c\xe4\xb7\x6b\xb2\x1b\x5c\x92\x69\xb3\xf1\xf8\x87\x99\x67\xc6\xf3\x69"
      "\x92\x95\x05\xaf\xa0\x86\x38\x97\xc6\x95\xab\xa3\xc9\xaa\xce\x09\x08\x52"
      "\x8c\xa6\xc5\xa3\xcf\x56\x28\x5f\xb7\xd4\xf4\xa5\x2b\xe8\x3b\x2d\xfd\x4d"
      "\x2c\xb4\x9a\xde\x14\x12\x34\xc7\x9a\xc1\x77\x38\x04\x8c\x2e\xae\x33\x56"
      "\x3a\x29\x53\x1c\x5b\xe9\xa6\x12\x14\xe1\x14\x60\xef\xce\x6d\xdc\xa4\x36"
      "\xb9\x36\x93\x07\x6b\x8a\x26\x77\x33\x83\x0c\x98\x14\x22\x9b\x5d\x0d\xd4"
      "\xc3\x8f\x6f\x1e\x15\xa0\x21\x11\x45\x30\x16\xdc\x88\x95\x4c\x1b\x53\xff"
      "\x07\x00\x00\xdc\x26\x68\x8e\xb7\x74\xbe\x37\x03\x31\xf7\x44\xea\xd6\x58"
      "\x9b\xd2\x50\x8b\x45\x95\x26\x71\x28\xdf\x30\xcb\xeb\x47\x57\x83\x7b\xcc"
      "\xfb\x15\xa9\x3d\x50\xe7\x29\x36\xbe\xb3\x01\xb4\x6d\x59\x5a\xc1\x87\xae"
      "\x1b\x7c\x0e\x22\x9f\xe6\x19\x49\x53\x7b\xee\xe1\x70\x28\x0b\x22\x83\xcd"
      "\xe3\x3a\x2a\xd0\x25\x91\x76\x66\xcd\x39\xd9\x2d\xab\xfd\x4f\x79\x5a\x03"
      "\x91\x7a\x7c\x78\x58\x2e\x6c\x01\xa0\xa3\xfa\xb4\x1c\xc8\x78\x4d\x38\xda"
      "\xfb\xaa\x00\xad\x88\x3e\x45\x8f\x85\x77\xc8\xd5\x77\x1d\xbe\xa6\x81\x50"
      "\x2e\x1a\x59\x36\x42\x82\x2c\x16\xed\xac\x3d\xeb\x54\x2a\x08\xff\x9b\x2e"
      "\x19\x15\xb3\x88\xba\x37\xec\xb8\x3e\x4e\x80\x6d\xf4\xac\xe9\xd2\x74\xd0"
      "\x9f\x50\x70\x4b\x90\x58\x5f\x76\xe4\x92\x62\x80\xa5\x21\x0d\x25\x52\xb0"
      "\xcd\x30\x07\xa8\x42\x71\xc1\x5b\x1b\xbe\x1d\x26\x3b\x2c\x3a\xf5\x4c\x08"
      "\xb5\x6d\x28\xb1\x77\xe0\xcb\x5b\x3a\x72\x6b\xa2\x29\x55\xcc\x51\xfa\xf8"
      "\xfe\x70\x9d\xce\x54\xfa\x06\xb2\x92\x64\x62\x27\xa2\x61\xf0\xb8\xd0\x5f"
      "\x55\xf2\xbc\xf5\x01\xd1\x96\x57\xea\x01\xe5\xda\xac\xfb\x27\x3d\x1c\x9c"
      "\x58\x2f\xd8\x5f\xc7\xf0\xfc\x2c\xf5\x2d\xdf\x45\x93\xee\xef\xd4\x3a\x8b"
      "\x40\x49\x28\x32\x18\x8d\x42\xcf\xa2\xf4\x46\x22\xdd\x80\xc8\x74\xf1\xdb"
      "\xc7\x3f\xa2\xf2\x84\x62\xf3\x21\xee\x23\xd7\x9a\x61\x1a\xd5\x1f\x2e\x91"
      "\xce\xba\xb7\x8e\xf4\xf3\x22\xcd\xd1\xa8\x23\xff\x9f\x98\xbd\x62\x1e\x94"
      "\x27\x4e\x5d\x2f\x07\x8c\x8d\x87\x26\x72\x5d\x79\xb0\x9d\x94\x9b\x09\xce"
      "\x84\x6b\x29\x7c\xa3\x59\x8c\xfa\x94\xec\xd0\xbf\x9b\xbd\xb2\x1c\x2e\x32"
      "\x06\xf9\x12\x31\x27\x51\x51\xed\x5f\x2b\x23\x94\x42\x94\x5a\x12\x0c\xd1"
      "\x78\xcf\x5f\x6f\xb9\xb7\xab\x8c\xab\x89\xe2\x8b\xe3\x1d\xa5\xfc\x7b\x9b"
      "\xfc\x3a\x8b\xae\x77\x35\xf4\x6a\x98\x8f\xe0\x2d\x38\xe7\xf3\xcc\x4a\xaa"
      "\x8e\xdb\x4c\x9c\xc7\x4d\x35\x16\x19\x0a\x98\xb6\x56\xf6\xd0\xbe\x6b\x49"
      "\x51\xa4\xc3\x24\xbf\xa7\x38\x67\x10\x05\xd8\x58\xac\xc1\xda\x0d\xcc\xb8"
      "\x94\x7b\x27\x8a\x2c\x2a\x49\x15\xd0\xee\xaf\x13\xfb\x07\xb8\xc2\x74\xb3"
      "\x8f\x48\x1f\xc7\xa1\x3b\xb7\x57\xc4\xb3\x8d\x07\x35\x27\xa9\x37\xcc\x4f"
      "\xcc\x95\x06\xfb\x51\x41\x20\x00\x55\x87\x43\xfb\xa1\x7b\xa3\x0d\x59",
      881);
  syscall(__NR_setxattr, /*path=*/0x20000040ul, /*name=*/0x20000080ul,
          /*val=*/0x20001080ul, /*size=*/0x371ul, /*flags=*/0ul);
  memcpy((void*)0x20000000, "./file1\000", 8);
  syscall(__NR_creat, /*file=*/0x20000000ul, /*mode=*/0x108ul);
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
