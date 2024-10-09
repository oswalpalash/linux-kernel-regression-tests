// https://syzkaller.appspot.com/bug?id=ff9fe3de46d66c92f26a627abe9e85d0518c9890
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

uint64_t r[1] = {0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  memcpy((void*)0x20000000, "hfsplus\000", 8);
  memcpy((void*)0x20000040, "./file1\000", 8);
  memcpy(
      (void*)0x20000c80,
      "\x78\x9c\xec\xdd\x4b\x6c\x5c\x57\x19\x00\xe0\xff\x8e\x67\xc6\x9e\x80\x52"
      "\xa7\x4d\xd2\x80\x2a\x61\x35\x52\x41\x58\x24\x7e\xc8\x29\x66\xd3\x80\x10"
      "\xb2\x44\x85\xaa\xb2\x40\x2c\x47\xc9\xa4\x19\x65\xe2\x56\xf6\x14\x39\x11"
      "\x82\xf0\x5e\xb0\x61\xd1\x3d\x45\xc2\x3b\x36\x20\xb1\x0f\x2a\x6b\x60\xd5"
      "\xad\x97\x95\x90\xd8\x64\x15\x40\x62\xd0\x7d\xcc\x78\xc6\x76\xec\x19\xc7"
      "\xf6\xd8\xea\xf7\x45\x77\xce\x39\xf7\x3c\xee\x39\xff\xdc\x3b\x77\x1e\x8a"
      "\x1c\xc0\xa7\xd6\xca\x6c\x94\x1f\x47\x12\x2b\xb3\x6f\x6e\xa4\xe5\xad\xcd"
      "\xc5\xd6\xd6\xe6\xe2\xfd\x6e\x3e\x22\x26\x23\xa2\x14\x51\xce\x93\x48\x56"
      "\x23\x92\x8f\x22\x6e\x46\xbe\xc5\xe7\xd2\x9d\xc5\x70\xc9\xb3\x8e\xf3\x41"
      "\x73\xf9\xed\x8f\x9f\x6c\x7d\x92\x97\xca\xc5\x96\xb5\x2f\xed\xd7\x6f\x97"
      "\x1b\xa5\x3d\x76\x3e\x2a\xb6\x98\x89\x88\x89\x22\x7d\x0e\x03\xe3\xdd\xda"
      "\x31\x5e\x75\xe4\xe1\x92\xde\x0a\xd3\x80\x5d\xed\x06\x0e\xc6\xad\x12\x11"
      "\x9d\x01\xdf\xbf\xbc\x5d\x73\xa0\xe1\xaf\x5b\xe0\xd4\x4a\xf2\xfb\xe6\xae"
      "\x0b\x7a\x3a\xe2\x5c\x44\x4c\x15\xef\x03\xf2\xbb\x62\x7e\xcf\x3e\xd3\x1e"
      "\x8d\x7b\x02\x00\x00\x00\x70\x02\x5e\xf8\x75\xf6\x11\xfe\xfc\xb8\xe7\x01"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x67\x49\xf1\xf7\xff\x93\x62\x2b\x75"
      "\xf3\x33\x91\x74\xff\xfe\x7f\xb5\xd8\x17\x45\xfe\x4c\x7b\x3c\xee\x09\x00"
      "\x00\x00\x00\x00\x00\x00\xc0\xe8\xbe\xfd\xd9\x1d\x3b\xbe\xf0\x34\x9e\xc6"
      "\x46\x9c\xef\x96\x3b\x49\xf6\x9b\xff\xab\x59\xe1\x62\xf6\xf8\x99\x78\x3f"
      "\xd6\xa3\x11\x6b\x71\x2d\x36\xa2\x1e\xed\x68\xc7\x5a\xcc\x47\xc4\x74\x56"
      "\x5f\xc9\x1e\xab\x1b\xf5\x76\x7b\x6d\x7e\x88\x9e\x0b\xbd\x9e\xd1\xd7\x73"
      "\x61\xc8\x15\xd4\x0e\xbf\x78\x00\x00\x00\x00\x00\x00\x00\x38\x2b\xca\xa3"
      "\x77\xf9\x69\xac\x6c\xff\xfe\x0f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\xa7\x41\x12\x31\x91\x27\xd9\x76\xb1\x9b\x9f\x8e\x52\x39\x22\xa6\x22"
      "\xa2\x9a\xb6\x7b\x14\xf1\xf7\x6e\xfe\x54\xfa\xed\x5f\xfa\x4b\x9d\xff\x76"
      "\x32\xbb\x9a\x3d\x3e\xc9\x39\x01\x00\x00\xc0\x98\xbc\xf0\x34\x9e\xc6\x46"
      "\x9c\xef\x96\x3b\x49\xf6\x99\xff\x72\xf6\xb9\x7f\x2a\xde\x8f\xd5\x68\x47"
      "\x33\xda\xd1\x8a\x46\xdc\xce\xbe\x0b\xc8\x3f\xf5\x97\xb6\x36\x17\x5b\x5b"
      "\x9b\x8b\xf7\xd3\x6d\xf7\xb8\x5f\xff\xd7\x48\xd3\xc8\x46\x8c\xfc\xbb\x87"
      "\xbd\x8f\x7c\x25\x6b\x51\x8b\x3b\xd1\xcc\xf6\x5c\x8b\x5b\xf1\x6e\xb4\xe2"
      "\x76\x94\xb2\x9e\xa9\x2b\xdd\xf9\xec\x3d\xaf\x9f\xa4\x73\x4a\xde\x28\x0c"
      "\x39\xb3\xdb\x45\x9a\xae\xfc\x37\x51\x19\x69\x55\x87\x91\x0c\xdd\x72\x3a"
      "\x8b\x48\x3a\xa3\x3c\x22\x73\x45\xdf\x34\x1a\x17\xf6\x8f\xc4\x88\xcf\x4e"
      "\xf7\x48\xdd\xd8\xcf\x47\xa9\xf7\xcd\xcf\xc5\xa3\x8c\xf9\x46\x9e\xbc\xfe"
      "\xfb\x3c\x4d\xd7\xf3\xcb\x91\x62\x72\xdc\x76\x46\x62\xa1\xef\xec\xbb\xbc"
      "\x7f\x24\x22\xbe\xf8\xe7\x3f\x7c\xef\x6e\x6b\xf5\xde\xe4\x9d\xf5\xd9\xd3"
      "\xb3\xa4\x11\x4c\xf6\x7d\x83\xb6\x33\x12\x8b\x7d\x91\x78\x79\xd8\x48\xdc"
      "\x3d\xab\x91\xe8\x37\x97\x45\xe2\x52\xaf\xbc\x12\xdf\x8a\xef\xc6\x6c\xcc"
      "\xc4\x5b\xb1\x16\xcd\xf8\x41\xd4\xa3\x1d\x8d\x98\x89\x6f\x46\x3d\x26\xa2"
      "\x5e\x9c\xcf\xe9\xe3\xf4\xfe\x91\xba\x39\x50\x7a\xeb\xa0\x99\x54\xb3\xe7"
      "\xa5\x52\xbc\x8a\x0e\x3f\xa7\x76\xd4\xe3\xd5\xac\xef\xf9\x68\xc6\x77\xe2"
      "\xdd\xb8\x1d\x8d\xb8\x91\xfd\x5b\x88\xf9\x78\x3d\x96\x62\x29\x96\xfb\x9e"
      "\xe1\x4b\x43\x5c\xf5\xa5\xd1\x5e\x69\xaf\x7e\xa9\xef\xcb\xe4\x5f\x45\x44"
      "\x6d\xb8\x7e\x27\x20\x9d\xd8\x85\xde\xdd\xa9\xff\xac\x9f\xcb\xae\x83\x0b"
      "\x03\x7b\xb6\xa3\xf4\xe2\xd1\xdf\x8f\xca\x9f\x2f\x32\xe9\x31\x7e\x56\xa4"
      "\xa7\xc3\xce\x48\xcc\xf7\x45\xe2\xa5\xfd\x23\xf1\xbb\xec\x65\x65\xbd\xb5"
      "\x7a\x6f\xed\x6e\xfd\xbd\x21\x8f\xf7\x5a\x91\xa6\xd7\xd1\x2f\x4e\xd5\x5d"
      "\x22\x3d\x5f\x5e\x4c\x9f\xac\xac\x34\x78\x76\xa4\x75\x2f\xed\xac\x9b\xca"
      "\xe3\x55\x2d\x7e\x71\xc9\xeb\x06\xef\xb8\x69\xdd\xa5\x5e\xdd\x41\x57\x6a"
      "\xb5\x78\x0f\xb7\x7b\xa4\x85\xac\xee\xe5\x3d\xeb\x16\xb3\xba\x2b\x7d\x75"
      "\x03\xef\xb7\x6e\xe6\xef\xb7\x00\x38\xf5\xce\x7d\xf9\x5c\xb5\xf6\xcf\xda"
      "\xdf\x6a\x1f\xd6\x7e\x5e\xbb\x5b\x7b\x73\xea\x1b\x93\x5f\x9d\x7c\xa5\x1a"
      "\x95\xbf\x56\xbe\x56\x9e\x9b\x78\xad\xf4\x4a\xf2\xa7\xf8\x30\x7e\xb4\xfd"
      "\xf9\x1f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x38\xbc\xf5\x07\x0f\xef\xd5\x5b\xad\xc6\xda"
      "\x8e\x4c\xa7\xd3\xf9\xf1\x33\xaa\x4e\x28\x13\x71\x50\x9b\x4a\x1c\xdc\xe6"
      "\x78\x32\xd5\x88\xc8\x32\xe5\x6e\x66\xb4\x71\x26\x87\x6a\x5c\xdd\x7e\x76"
      "\xde\xf8\xe3\xf3\xcc\xb9\x32\x6a\xaf\x88\x23\x09\x54\xb9\x38\xc9\x1e\x3c"
      "\xbc\xf7\xef\x4e\xa7\x33\xa6\x13\x69\x20\x53\xd9\xe7\x9c\xdf\xce\x74\x0a"
      "\x63\x9d\xea\x21\x32\xff\xe9\x1c\xdd\x80\x63\x7e\x61\x02\x8e\xdd\xf5\xf6"
      "\xfd\xf7\xae\xaf\x3f\x78\xf8\x95\xe6\xfd\xfa\x3b\x8d\x77\x1a\xab\xcb\x4b"
      "\x4b\xcb\x73\xcb\x4b\x37\xfe\x71\xfd\x4e\xb3\xd5\x98\xcb\x1f\xc7\x3d\x4b"
      "\xe0\x38\xf4\xbf\xdf\x06\x00\x00\x00\x00\x00\x00\x00\x00\x00\xce\x86\x93"
      "\xf9\x9f\x3c\x7b\xfa\xdf\x09\x2f\x15\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x38\xa3\x56\x66\x27\x8b"
      "\xdc\xb5\xb9\xf4\x71\x6b\x73\xb1\x95\x6e\xdd\x7c\xaf\x61\xd6\xac\x14\x11"
      "\xc9\x0f\x23\x92\x8f\x22\x6e\x46\xbe\xc5\x74\xdf\x70\xc9\xb3\x8e\xf3\x41"
      "\x73\xf9\xed\x8f\x9f\x6c\x7d\x92\x97\xca\xc5\x96\xb5\x2f\x0d\xf4\xab\x1c"
      "\x66\x15\x8f\x8a\x2d\x66\x22\x62\xa2\x48\xfb\x4d\x3d\xc7\x78\xb7\x8a\xf4"
      "\x50\x33\xcb\x24\xbd\x15\xa6\x01\xbb\xda\x0d\x1c\x8c\xdb\xff\x03\x00\x00"
      "\xff\xff\xac\xc7\x14\x03",
      1662);
  syz_mount_image(/*fs=*/0x20000000, /*dir=*/0x20000040,
                  /*flags=MS_NOATIME*/ 0x400, /*opts=*/0x20000640,
                  /*chdir=*/0xfe, /*size=*/0x67e, /*img=*/0x20000c80);
  memcpy((void*)0x200000c0, "./file1\000", 8);
  res = syscall(
      __NR_open, /*file=*/0x200000c0ul,
      /*flags=O_SYNC|O_NOCTTY|O_NOATIME|O_DIRECT|O_CREAT|0x2*/ 0x145142ul,
      /*mode=*/0ul);
  if (res != -1)
    r[0] = res;
  syscall(__NR_ftruncate, /*fd=*/r[0], /*len=*/0x2007ffbul);
  memcpy((void*)0x20000480, "./bus\000", 6);
  memcpy((void*)0x20000100, "./bus\000", 6);
  syscall(__NR_symlink, /*old=*/0x20000480ul, /*new=*/0x20000100ul);
  memcpy((void*)0x20000e00,
         "./"
         "file2aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\000",
         252);
  syscall(__NR_creat, /*file=*/0x20000e00ul, /*mode=*/0ul);
  memcpy((void*)0x20000b00, "./bus\000", 6);
  memcpy((void*)0x20000300, "./file2\000", 8);
  syscall(__NR_rename, /*old=*/0x20000b00ul, /*new=*/0x20000300ul);
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
  loop();
  return 0;
}
