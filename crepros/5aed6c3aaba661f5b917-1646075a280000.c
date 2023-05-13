// https://syzkaller.appspot.com/bug?id=11d0e5f6fef53a0ea486bbd07ddd3cba66132150
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
#define __NR_memfd_create 385
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_setxattr
#define __NR_setxattr 226
#endif
#ifndef __NR_unlinkat
#define __NR_unlinkat 328
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

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
      if (current_time_ms() - start < 15000)
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
  memcpy((void*)0x20000040, "ext4\000", 5);
  memcpy((void*)0x20000100, "./file1\000", 8);
  memcpy((void*)0x20000240, "inode_readahead_blks", 20);
  *(uint8_t*)0x20000254 = 0x3d;
  sprintf((char*)0x20000255, "0x%016llx", (long long)0);
  *(uint8_t*)0x20000267 = 0x2c;
  memcpy((void*)0x20000268, "errors=continue", 15);
  *(uint8_t*)0x20000277 = 0x2c;
  memcpy((void*)0x20000278, "debug_want_extra_isize", 22);
  *(uint8_t*)0x2000028e = 0x3d;
  sprintf((char*)0x2000028f, "0x%016llx", (long long)0x40);
  *(uint8_t*)0x200002a1 = 0x2c;
  memcpy((void*)0x200002a2, "dioread_nolock", 14);
  *(uint8_t*)0x200002b0 = 0x2c;
  memcpy((void*)0x200002b1, "max_batch_time", 14);
  *(uint8_t*)0x200002bf = 0x3d;
  sprintf((char*)0x200002c0, "0x%016llx", (long long)8);
  *(uint8_t*)0x200002d2 = 0x2c;
  memcpy((void*)0x200002d3, "nombcache", 9);
  *(uint8_t*)0x200002dc = 0x2c;
  *(uint8_t*)0x200002dd = 0;
  memcpy(
      (void*)0x200004c0,
      "\x78\x9c\xec\xdb\xcb\x6f\x1b\x45\x18\x00\xf0\x6f\xd7\x71\x0b\x7d\x90\x50"
      "\xca\xa3\xa5\x80\xa1\x20\x22\x1e\x49\x93\x16\xe8\x81\x0b\x08\x24\x0e\x20"
      "\x21\xc1\xa1\x1c\x43\x92\x56\xa5\x6e\x83\x9a\x20\xd1\xaa\x82\x82\x50\x39"
      "\xa2\x22\xee\x88\x23\x12\x7f\x01\x27\xb8\x20\xe0\x84\xc4\x15\xee\xa8\x52"
      "\x85\x7a\x69\xe1\x64\xb4\xf6\x6e\xe3\x38\x76\x88\x13\x3b\x2e\xf8\xf7\x93"
      "\xb6\x9e\xf1\x8e\x3b\xf3\x79\x76\x9c\xd9\x19\x3b\x80\xa1\x55\xc9\xfe\x49"
      "\x22\x76\x45\xc4\x6f\x11\x31\xda\xc8\xae\x2c\x50\x69\x3c\xdc\xb8\x76\x61"
      "\xf6\xaf\x6b\x17\x66\x93\xa8\xd5\xde\xf8\x33\xa9\x97\xbb\x7e\xed\xc2\x6c"
      "\x51\xb4\x78\xdd\xce\x3c\x33\x9e\x46\xa4\x9f\x24\x2d\xff\x61\xc3\xe2\xb9"
      "\xf3\xa7\x66\xaa\xd5\xf9\xb3\x79\x7e\x72\xe9\xf4\xbb\x93\x8b\xe7\xce\x3f"
      "\x7d\xf2\xf4\xcc\x89\xf9\x13\xf3\x67\xa6\x8f\x1e\x3d\x72\x78\xea\xb9\x67"
      "\xa7\x9f\xe9\x49\x9c\x59\x5c\xd7\xf7\x7f\xb0\x70\x60\xdf\x2b\x6f\x5d\x7e"
      "\x6d\xf6\xd8\xe5\xb7\x7f\xfa\x26\x6b\xd6\xae\xfc\x7c\x73\x1c\xbd\x52\x89"
      "\x4a\xbb\xd0\xeb\x1e\xeb\x75\x65\x03\xb6\xbb\x29\x9d\x8c\x0c\xb0\x21\x74"
      "\xa5\x14\x11\x59\x77\x95\xeb\xe3\x7f\x34\x4a\xb1\xdc\x79\xa3\xf1\xf2\xc7"
      "\x03\x6d\x1c\xd0\x57\xb5\x5a\xad\xb6\xbd\xf3\xe9\x8b\x35\xe0\x7f\x2c\x89"
      "\x41\xb7\x00\x18\x8c\xe2\x0f\x7d\x76\xff\x5b\x1c\x5b\x34\xf5\xb8\x25\x5c"
      "\x7d\xa1\x71\x03\x94\xc5\x7d\x23\x3f\x1a\x67\x46\x22\xcd\xcb\x94\x5b\xee"
      "\x6f\x7b\xa9\x12\x11\xc7\x2e\xfe\xfd\x65\x76\x44\x9f\xd6\x21\x00\x00\x9a"
      "\x7d\x97\xcd\x7f\x9e\x6a\x37\xff\x4b\xe3\x9e\xa6\x72\x77\xe4\x7b\x28\x63"
      "\x11\x71\x67\x44\xec\x89\x88\xbb\x22\x62\x6f\x44\xdc\x1d\x51\x2f\x7b\x6f"
      "\x44\xdc\xd7\x65\xfd\x95\x96\xfc\xea\xf9\x4f\x7a\x65\x43\x81\xad\x53\x36"
      "\xff\x7b\x3e\xdf\xdb\x5a\x39\xff\x2b\x66\x7f\x31\x56\xca\x73\xbb\xeb\xf1"
      "\x97\x93\xe3\x27\xab\xf3\x87\xf2\xf7\x64\x3c\xca\xdb\xb3\xfc\xd4\x1a\x75"
      "\x7c\xff\xd2\xaf\x9f\x75\x3a\xd7\x3c\xff\xcb\x8e\xac\xfe\x62\x2e\x98\xb7"
      "\xe3\xca\x48\xcb\x02\xdd\xdc\xcc\xd2\xcc\x66\x62\x6e\x76\xf5\xa3\x88\xfd"
      "\x23\xed\xe2\x4f\x6e\xee\x04\x24\x11\xb1\x2f\x22\xf6\x6f\xb0\x8e\x93\x4f"
      "\x7c\x7d\xa0\xd3\xb9\x7f\x8f\x7f\x0d\x3d\xd8\x67\xaa\x7d\x15\xf1\x78\xa3"
      "\xff\x2f\x46\x4b\xfc\x85\x64\xed\xfd\xc9\xc9\xdb\xa2\x3a\x7f\x68\xb2\xb8"
      "\x2a\x56\xfb\xf9\x97\x4b\xaf\x77\xaa\x7f\x53\xf1\xf7\x40\xd6\xff\x3b\xda"
      "\x5e\xff\x37\xe3\x1f\x4b\x9a\xf7\x6b\x17\x23\xe2\xf3\xee\xea\xb8\xf4\xfb"
      "\xa7\x1d\xef\x69\x36\x7a\xfd\x6f\x4b\xde\xac\xa7\xb7\xe5\xcf\xbd\x3f\xb3"
      "\xb4\x74\x76\x2a\x62\x5b\xf2\xea\xea\xe7\xa7\x97\x5f\x5b\xe4\x8b\xf2\x59"
      "\xfc\xe3\x07\xdb\x8f\xff\x3d\xb1\xfc\x4e\xdc\x1f\x11\xd9\x45\xfc\x40\x44"
      "\x3c\x18\x11\x0f\xe5\x6d\x7f\x38\x22\x1e\x89\x88\x83\x6b\xc4\xff\xe3\x8b"
      "\x8f\xbe\xb3\xf1\xf8\xfb\x2b\x8b\x7f\xae\xab\xfe\xef\x3e\x51\x3a\xf5\xc3"
      "\xb7\x9d\xea\x5f\x5f\xff\x1f\xa9\xa7\xc6\xf3\x67\xd6\xf3\xf9\xb7\xde\x06"
      "\x6e\xe6\xbd\x03\x00\x00\x80\xff\x8a\xb4\xfe\x1d\xf8\x24\x9d\xb8\x99\x4e"
      "\xd3\x89\x89\xc6\x77\xf8\xf7\xc6\x8e\xb4\xba\xb0\xb8\xf4\xe4\xf1\x85\xf7"
      "\xce\xcc\x35\xbe\x2b\x3f\x16\xe5\xb4\x58\xe9\x1a\x6d\x5a\x0f\x9d\xca\xd7"
      "\x86\x8b\xfc\x74\x4b\xfe\x70\xbe\x6e\xfc\x45\xe9\xf6\x7a\x7e\x62\x76\xa1"
      "\x3a\x37\xe8\xe0\x61\xc8\xed\xec\x30\xfe\x33\x7f\x94\x06\xdd\x3a\xa0\xef"
      "\xba\xd9\x47\x2b\xf7\xb1\x1d\xc0\xd6\xf3\x7b\x4d\x18\x5e\xc6\x3f\x0c\x2f"
      "\xe3\x1f\x86\x97\xf1\x0f\xc3\xab\xdd\xf8\xff\x70\x00\xed\x00\xb6\xde\xc8"
      "\x8a\x07\x60\x98\x18\xf8\x30\xbc\x8c\x7f\x18\x5e\xc6\x3f\x0c\xa5\xcd\xfc"
      "\xae\x5f\x62\x98\x13\x91\xde\x12\xcd\x90\xe8\x53\x62\xd0\x9f\x4c\x00\x00"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xbd\xf1\x4f\x00\x00\x00\xff\xff"
      "\x14\x2f\xe7\x02",
      1066);
  res = -1;
  res = syz_mount_image(0x20000040, 0x20000100, 0, 0x20000240, 0, 0x42a,
                        0x200004c0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000200, "./file1\000", 8);
  memcpy((void*)0x200003c0, "trusted.overlay.opaque\000", 23);
  syscall(__NR_setxattr, 0x20000200, 0x200003c0, 0, 0, 0);
  memcpy((void*)0x20000340, "./file1\000", 8);
  memcpy((void*)0x20000400, "trusted.overlay.upper\000", 22);
  syscall(__NR_setxattr, 0x20000340, 0x20000400, 0x20001140, 0x1015, 0);
  memcpy((void*)0x20000000, "./file1\000", 8);
  syscall(__NR_unlinkat, (intptr_t)r[0], 0x20000000, 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000, 0x1000, 0, 0x32, -1, 0);
  syscall(__NR_mmap, 0x20000000, 0x1000000, 7, 0x32, -1, 0);
  syscall(__NR_mmap, 0x21000000, 0x1000, 0, 0x32, -1, 0);
  loop();
  return 0;
}
