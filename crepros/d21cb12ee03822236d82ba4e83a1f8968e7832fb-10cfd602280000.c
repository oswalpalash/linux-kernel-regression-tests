// https://syzkaller.appspot.com/bug?id=d21cb12ee03822236d82ba4e83a1f8968e7832fb
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
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
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/loop.h>

#ifndef __NR_memfd_create
#define __NR_memfd_create 319
#endif

static unsigned long long procid;

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

static int inject_fault(int nth)
{
  int fd;
  fd = open("/proc/thread-self/fail-nth", O_RDWR);
  if (fd == -1)
    exit(1);
  char buf[16];
  sprintf(buf, "%d", nth);
  if (write(fd, buf, strlen(buf)) != (ssize_t)strlen(buf))
    exit(1);
  return fd;
}

static void setup_fault()
{
  static struct {
    const char* file;
    const char* val;
    bool fatal;
  } files[] = {
      {"/sys/kernel/debug/failslab/ignore-gfp-wait", "N", true},
      {"/sys/kernel/debug/fail_futex/ignore-private", "N", false},
      {"/sys/kernel/debug/fail_page_alloc/ignore-gfp-highmem", "N", false},
      {"/sys/kernel/debug/fail_page_alloc/ignore-gfp-wait", "N", false},
      {"/sys/kernel/debug/fail_page_alloc/min-order", "0", false},
  };
  unsigned i;
  for (i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
    if (!write_file(files[i].file, files[i].val)) {
      if (files[i].fatal)
        exit(1);
    }
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  setup_fault();
  intptr_t res = 0;
  memcpy((void*)0x20000c40, "udf\000", 4);
  memcpy((void*)0x20000c80, "./file0\000", 8);
  memcpy((void*)0x20000200,
         "\x6e\x6f\x76\x72\x73\x2c\x00\xfb\xeb\xeb\x63\x39\x68\x27\x8c\x5b\x11"
         "\x0d\xd2\x86\x37\xf4\x9e\x49\x08\x7d\x66\xcd\x89\xfd\x35\x85\x38\xce"
         "\xb2\x8f\x23\x54\x51\x12\xbf\x67\xb6\xdc\x9f\x6d\x1e\xd1\x20\xc4\x96"
         "\x25\x30\x40\x6b\x8b\xeb\xfd\x9b\x24\xa8\xcd\xb6\xfb\x24\x35\xc7\xaa"
         "\x2e\x20\x9b\x64\x69\xbd\x5b\x44\xcb\x10\x0a\xf4\xdc\xb1\xa6\xeb\x61"
         "\x93\xf9\x2f\xb0\x79\xb7\x1f\x09",
         93);
  memcpy(
      (void*)0x20001940,
      "\x78\x9c\xec\xdd\x5f\x68\x5c\xe9\x79\x3f\xf0\xe7\xd5\x91\xd6\x92\xf7\x97"
      "\x5f\x66\x37\x1b\xe7\x8f\x73\x31\xb0\x81\x6c\xbd\xd9\x45\xb2\xbc\x6b\x15"
      "\x6f\x40\x8e\x15\x91\x05\xe3\x35\x2b\x2b\x17\x0b\x05\x8d\x2d\xd9\x1d\x56"
      "\x9a\x91\x25\xb9\x78\x43\x09\x2e\x24\x94\x90\xb6\xb8\xe4\x22\x97\x35\x6c"
      "\x02\xbd\xab\xaf\x5a\x58\x1a\x70\xaf\xb6\x21\x04\x44\xaf\x4a\x2f\x8a\xdb"
      "\x6e\xcc\xf6\x6e\x12\x9a\xb6\xf4\x22\x2a\x67\xe6\x1d\x69\xa4\xb5\x2d\x75"
      "\x6d\x4b\xb2\xf7\xf3\x31\xf6\xf7\xcc\x99\xe7\xcc\xbc\x67\x56\xcf\xe8\xcc"
      "\xec\xbc\x73\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x88"
      "\xf8\xfa\x37\x4e\x0e\x8f\xa4\xfb\x14\x0c\xec\xe2\x60\x00\x80\x5d\x71\x66"
      "\xea\xcd\xe1\xd1\xfb\xfd\xfe\x07\x00\x9e\x38\xe7\xb6\x7b\xfd\x0f\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x11\x29\x8a\xf8\x6e\xa4\x78\xf7\x07\xad\x34\xd3\xbe\xdc\x31"
      "\x78\xba\xde\xb8\x72\x75\x7a\x62\xf2\xee\x9b\x0d\xa5\x48\xd1\x17\x45\xbb"
      "\xbe\xfc\x3b\x38\x72\x74\xf4\xd8\x2b\xaf\x1e\x1f\xeb\xe6\xfd\xb7\x7f\xd8"
      "\xbe\x10\x6f\x4c\x9d\x3b\x59\x3d\xd5\x5c\x58\x5c\x9a\x5b\x5e\x9e\x9b\xad"
      "\x4e\x37\xea\x17\x9a\xb3\x73\x3b\xbe\x85\x07\xdd\x7e\xab\x23\xed\x07\xa0"
      "\xba\xf0\xf6\x95\xd9\x8b\x17\x97\xab\x47\x5f\x1e\xdd\x74\xf5\xd5\xca\x9d"
      "\x03\x4f\x1f\xaa\x9c\x18\x3b\x3c\xfa\x56\xb7\x76\x7a\x62\x72\x72\xaa\xa7"
      "\xa6\x7f\xe0\x63\xdf\xfb\x47\xa4\x87\x77\x53\x3c\x41\x9e\x8a\x22\xbe\x19"
      "\x29\xde\x7f\xe9\xc3\x54\x8b\x88\xbe\x78\xf0\x5e\xd8\xe6\xb9\xe3\x51\x1b"
      "\x8a\xfe\xb2\xff\xda\x3b\x31\x3d\x31\xd9\xde\x91\xf9\x7a\xad\xb1\x52\x5e"
      "\x99\xfa\x72\x55\x7f\x44\xa5\x67\xa3\xf1\x6e\x8f\xec\x42\x2f\x3e\x90\xf1"
      "\x88\x6b\xe5\x7f\xa7\x72\xc0\x47\xca\xdd\x9b\x5a\xac\x2d\xd5\xce\xcf\xcf"
      "\x55\xcf\xd6\x96\x56\xea\x2b\xf5\x66\x23\xf5\x75\x46\x5b\xee\x4f\x25\xfa"
      "\x62\x2c\x45\x2c\x46\x44\xab\xd8\xeb\xc1\xb3\xdf\x0c\x44\x11\xc7\x22\xc5"
      "\x9d\x5f\xb7\xd2\xf9\x88\x28\xba\x7d\xf0\xe2\x99\xa9\x37\x87\x47\xb7\xbf"
      "\x81\xfe\x5d\x18\xe4\x3d\xee\xb6\x52\x44\xac\xc6\x63\xd0\xb3\xb0\x4f\x1d"
      "\x88\x22\xfe\x3c\x52\xfc\x70\x66\x38\x2e\xe4\xbe\x6a\xb7\xcd\x07\x11\x5f"
      "\x29\xf3\xb5\x88\xcb\x65\xde\x4a\x71\x3d\x5f\x4e\xe5\x13\xc4\x58\xc4\xaf"
      "\xfc\x3e\x81\xc7\x5a\x7f\x14\xf1\x8b\x48\xd1\x4c\xad\x34\xdb\xed\xfd\xf6"
      "\x71\xe5\xe9\x6f\x55\x5f\x6f\x5c\x6c\xf6\xd4\x76\x8f\x2b\x1f\xfb\xd7\x07"
      "\xbb\xc9\xb1\x09\xfb\xd8\x60\x14\x71\xbe\x7d\xc4\xdf\x4a\x1f\xff\xcd\x2e"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x60\x77\x14\xf1\x5e\xa4\xb8\xb9"
      "\xf0\x42\x5a\x8c\xde\x39\xa5\xf5\xc6\xa5\xea\xb9\xda\xf9\xf9\xce\xa7\x82"
      "\xbb\x9f\xfd\xaf\xe6\xad\xd6\xd6\xd6\xd6\x2a\xa9\x93\xd5\x9c\xc3\x39\xc7"
      "\x73\x9e\xcd\x39\x93\x73\x31\xe7\xb5\x9c\xd7\x73\xde\xc8\x79\x33\xe7\xad"
      "\x9c\xab\x39\x6f\xe7\x6c\xe5\x8c\xbe\x7c\xff\x39\xab\x39\x87\x73\x8e\xe7"
      "\x3c\x9b\x73\x26\xe7\x62\xce\x6b\x39\xaf\xe7\xbc\x91\xf3\x66\xce\x5b\x39"
      "\x57\x73\xde\xce\xd9\xca\x19\xe6\x3d\x01\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\xf0\x90\x0d\x45\x11\x93\x91\xe2\xc6\xbb\x7f\xd0"
      "\x3e\xaf\x74\xb4\xcf\x4b\xff\xe9\x13\x63\x67\x26\x9e\xeb\x3d\x67\xfc\xe7"
      "\xb6\xb9\x9d\xb2\xf6\xe5\x88\x78\x2f\x76\x76\x4e\xde\x81\x7c\xae\xf1\xd4"
      "\x57\xfe\x79\xf8\xfb\x05\x6c\x6f\x30\x8a\xf8\x4e\x3e\xff\xdf\x1f\xed\xf5"
      "\x60\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x80\x7d\xa1\x2f\x8a\xf8\x6e\xa4\xf8\xd1\x6f\x5a\x29\x52\x44\x8c\x47\xcc"
      "\x44\x27\x6f\x17\x7b\x3d\x3a\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\xa0\x34\x98\x8a\x38\x15\x29\xfe\xfd\x1b"
      "\x83\xed\xcb\xab\x11\xf1\xc5\x88\xf8\xed\x5a\xf9\x27\xe2\xbf\xd7\xb6\xda"
      "\xeb\x11\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xc0"
      "\x13\x28\x15\x71\x39\x52\xfc\xf8\xfd\x56\xaa\x44\xc4\xd5\xca\x9d\x03\x4f"
      "\x1f\xaa\x9c\x18\x3b\x3c\xfa\x56\x11\x45\xa4\xb2\xa4\xb7\xfe\x8d\xa9\x73"
      "\x27\xab\xa7\x9a\x0b\x8b\x4b\x73\xcb\xcb\x73\xb3\xd5\xe9\x46\xfd\x42\x73"
      "\x76\x6e\xa7\x77\x37\x78\xba\xde\xb8\x72\x75\x7a\x62\xf2\x91\xec\xcc\xb6"
      "\x86\x1e\xf1\xf8\x87\x06\x4f\x35\x17\xdf\x59\xaa\x5f\xfa\xfd\x95\xbb\x5e"
      "\x7f\x70\xf0\xe4\xf9\xe5\x95\xa5\xda\x85\xbb\x5f\x1d\x43\xd1\x1f\x31\xdc"
      "\xbb\xe6\x48\x7b\xc0\xd3\x13\x93\xed\x41\xcf\xd7\x6b\x8d\xf6\xa6\xa9\xef"
      "\x1e\x03\xec\x8f\xa8\xee\x74\x67\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\xd8\x37\x0e\xa6\x22\x26\x22\xc5\xf3\x3f\x3d\x96\xba"
      "\xf3\xc6\xfb\x3b\x73\xfe\x3f\xd5\xb9\x54\xac\xd7\xfe\xe4\x0f\x37\xbe\x0b"
      "\x60\x7e\x4b\x76\xf5\x7e\x7f\xc0\xc6\x72\x77\xb2\xfa\xd6\xf5\xa3\x6f\xa5"
      "\x9d\x0e\xf4\x48\x7b\xe2\x7d\x75\x7a\x62\x72\x72\xaa\x67\x75\xff\xc0\x47"
      "\x4b\xcb\x31\xa5\x54\xc4\x67\x23\xc5\xe1\xbf\xfd\x7c\x7b\x3e\x7c\x8a\x83"
      "\x77\x9d\x1b\x5f\xd6\xfd\x49\xa4\x18\xfb\x9f\x63\xb9\xae\x72\xb8\xac\x1b"
      "\xdf\x54\x35\x78\x64\x7a\x62\xb2\x7a\xa6\xd9\x78\xe9\xe4\xfc\x7c\xf3\x42"
      "\x6d\xa8\x76\x7e\x7e\xae\x3a\xb5\x58\xbb\xb0\xe3\x2f\x0e\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\xfb\x38\x98\x8a\xf8\xd3"
      "\x48\x71\xec\xf5\xd5\xd4\x3d\xef\x7c\x9e\xff\xdf\xdf\xb9\xd4\x33\xff\xff"
      "\xb5\x88\xee\x4c\xfe\xc1\xb4\x39\xd7\xb5\xe7\xf6\xff\xff\xf6\xdc\xfe\xce"
      "\xf2\xa7\x4f\x8c\xbd\x7e\xf4\xf9\x7b\xad\x7f\x14\xf3\xff\xcb\x31\xa5\x54"
      "\xc4\x6f\x23\xc5\x33\x7f\xf1\xf9\xf6\xf9\xf4\xbb\xf3\xff\x87\xb7\xd4\x96"
      "\x75\x3f\x8e\x14\xbf\xf8\xde\x97\x72\x5d\xdf\x53\x65\xdd\x48\x77\x77\x3a"
      "\xb7\x78\xb1\x3e\x3f\x37\x5c\xd6\xbe\x18\x29\xbe\x7f\xb6\x5b\x1b\xed\xda"
      "\x57\x73\xed\x67\x36\x6a\x47\xca\xda\xbf\x8b\x14\xcf\xfe\xde\xe6\xda\xe3"
      "\xb9\xf6\xb9\x8d\xda\xa3\x65\xed\x9d\x48\x31\x79\xe6\xee\xb5\x9f\xdd\xa8"
      "\x1d\x2d\x6b\x87\x22\xc5\x57\xff\xb8\xda\xad\x3d\x58\xd6\x7e\x3d\xd7\x1e"
      "\xda\xa8\x7d\xf9\x42\x73\x7e\x76\xa7\x0f\x2f\x9f\x4c\x65\xff\xff\x73\xa4"
      "\xf8\xf2\xc8\x37\x53\xf7\x67\xfe\x9e\xfd\xdf\xf3\xfd\x1f\xd7\xb6\xe4\xba"
      "\x8f\xf4\xfc\xfd\x97\x1f\x56\xff\x57\x7a\xd6\x5d\xcb\x7d\xbd\x96\xfb\x7f"
      "\x64\x9b\xfe\xbf\x1c\x29\xfe\xec\xfa\x97\x72\x5d\xa7\xf7\x8e\xe6\xeb\x9f"
      "\x69\xff\xbb\xd1\xff\xdf\x8f\x14\xbf\xf3\xa9\xcd\xb5\xaf\xe4\xda\x67\x37"
      "\x6a\x47\x76\xba\x5b\xb0\x97\xca\xfe\xff\x59\xa4\x58\xbd\xfd\x8f\xeb\x3f"
      "\xf3\xb9\xff\x73\x67\x6d\x74\x68\x6f\xff\x7f\xb1\x7f\x73\x76\x8f\x0b\xf6"
      "\xaa\xff\x9f\xe9\x59\x57\xc9\xe3\x1a\xfd\x3f\x3e\x16\xf0\x49\xb3\xfc\xce"
      "\xb7\xdf\xae\xcd\xcf\xcf\x2d\x59\xb0\x60\xc1\xc2\xfa\xc2\x5e\x3f\x33\x01"
      "\x8f\x5a\x79\xfc\xff\x9f\x91\xe2\x6b\x97\x8b\xd4\x7d\x1d\x9b\x8f\xff\xff"
      "\x5f\xe7\xd2\xc6\xeb\xff\xff\xfa\xce\xc6\xf1\xff\x89\x2d\xb9\x6e\x8f\x8e"
      "\xff\x9f\xed\x59\x77\x22\xbf\x6a\x19\xe8\x8f\x18\x5c\x59\x58\x1c\xf8\x5c"
      "\xc4\xe0\xf2\x3b\xdf\x7e\xa9\xbe\x50\xbb\x34\x77\x69\xae\x31\x3a\x3a\x76"
      "\xfc\x77\x8f\x8d\x1c\x3d\x3e\x32\xf0\x54\xf7\xc5\xfd\xc6\xd2\x8e\x1f\x3b"
      "\x78\xdc\x95\xfd\xff\x76\xa4\xf8\xc9\x5f\xfd\xc3\xfa\xfb\xd8\x9b\x5f\xff"
      "\xdf\xfd\xfd\xbf\x83\x5b\x72\xdd\x1e\xf5\xff\x67\x7a\xf7\x69\xd3\xeb\x9a"
      "\x1d\x3f\x14\xf0\x89\x53\xf6\xff\x5f\x46\x8a\x7f\xba\xf1\xe1\xfa\xff\x6f"
      "\xba\xdf\xfb\x7f\xdd\xf7\xf9\x5e\x78\x7e\x73\x0e\x75\x8b\xf6\xa8\xff\x9f"
      "\xeb\x59\x57\xcd\xff\x8c\xf5\xac\x7b\xa1\x88\x38\xb9\xd3\xfb\x02\x00\x00"
      "\x00\x00\x00\x00\x00\x80\xc7\xc4\xc1\x54\xc4\x4f\x23\xc5\x5f\xb7\xfe\x7e"
      "\xfd\x9c\xf7\x9b\x3f\xff\x13\x5f\xee\xd6\xf6\x7e\xfe\xef\x5e\xee\x7e\xfe"
      "\xff\x7b\x2f\x3f\x8a\xf9\xff\x00\xc0\xfd\x95\xbf\xff\xa7\x22\xc5\xcf\x0f"
      "\x7e\x35\x75\xbf\x43\x66\x27\x9f\xff\x9f\xdd\x92\xeb\xf6\xe8\xf3\xbf\x87"
      "\x7a\xd6\xcd\xee\xd2\xbc\xe6\x1d\x3f\xc8\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\xf0\x31\xa5\x28\xe2\x40\xa4\x78\xf7\x07\xad\x74"
      "\xbb\x28\x2f\x77\x0c\x9e\xae\x37\xae\x5c\x9d\x9e\x98\xbc\xfb\x66\xef\x35"
      "\x23\xa2\x2f\x8a\x76\x7d\xf9\x77\x70\xe4\xe8\xe8\xb1\x57\x5e\x3d\x3e\xd6"
      "\xcd\xfb\x6f\xff\xb0\x7d\x21\xde\x98\x3a\x77\xb2\x7a\xaa\xb9\xb0\xb8\x34"
      "\xb7\xbc\x3c\x37\x5b\x9d\x6e\xd4\x2f\x34\x67\xe7\x76\x7c\x0b\x0f\xba\xfd"
      "\x56\x47\xda\x0f\x40\x75\xe1\xed\x2b\xb3\x17\x2f\x2e\x57\x8f\xbe\x3c\xba"
      "\xe9\xea\xab\x95\x3b\x07\x9e\x3e\x54\x39\x31\x76\x78\xf4\xad\x6e\xed\xf4"
      "\xc4\xe4\xe4\x54\x4f\x4d\xff\xc0\xc7\xbe\xf7\x8f\x48\x0f\xef\xa6\x78\x82"
      "\x3c\x15\x45\xfc\x3c\x52\xbc\xff\xd2\x87\xe9\x5f\x8a\xb2\xa7\x1f\xbc\x17"
      "\xb6\x79\xee\x78\xd4\x86\xa2\xbf\xec\xbf\xf6\x4e\x4c\x4f\x4c\xb6\x77\x64"
      "\xbe\x5e\x6b\xac\x94\x57\xa6\xbe\x5c\xd5\x1f\x51\xe9\xd9\x68\xbc\xdb\x23"
      "\xbb\xd0\x8b\x0f\x64\x3c\xe2\x5a\xf9\xdc\x5b\x0e\xf8\x48\xb9\x7b\x53\x8b"
      "\xb5\xa5\xda\xf9\xf9\xb9\xea\xd9\xda\xd2\x4a\x7d\xa5\xde\x6c\xa4\xbe\xce"
      "\x68\xd3\xcf\xfe\x23\x2a\xd1\x17\x63\x29\x62\x31\x22\x5a\xc5\x5e\x0f\x9e"
      "\xfd\x66\x20\x8a\xf8\x9b\x48\x71\xe7\xd7\xad\xf4\xaf\x45\x44\xd1\xed\x83"
      "\x17\xcf\x4c\xbd\x39\x3c\xba\xfd\x0d\xf4\xef\xc2\x20\xef\x71\xb7\x95\x22"
      "\x62\x35\x1e\x83\x9e\x85\x7d\xea\x40\x14\xf1\x5c\xa4\xf8\xe1\xcc\x70\xfc"
      "\x5b\xd1\xe9\xab\x76\xdb\x7c\x10\xf1\x95\x32\x5f\x8b\xb8\x5c\xe6\xad\x14"
      "\xd7\xf3\xe5\x54\x3e\x41\x8c\x45\xfc\xca\xef\x13\x78\xac\xf5\x47\x11\x67"
      "\x23\x45\x33\xb5\xd2\x07\x45\xee\xfd\xf6\x71\xe5\xe9\x6f\x55\x5f\x6f\x5c"
      "\x6c\xf6\xd4\x76\x8f\x2b\x1f\xfb\xd7\x07\xbb\xc9\xb1\x09\xfb\xd8\x60\x14"
      "\xf1\xcb\xf6\x11\x7f\x2b\xfd\xd2\xef\x73\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\xd8\xe7\x8a\xf8\x5a\xa4\xb8\xb9\xf0\x42\x6a\xcf\x0f\x5d\x9f\x53"
      "\x5a\x6f\x5c\xaa\x9e\xab\x9d\x9f\xef\x7c\xac\xbf\xfb\xd9\xff\x6a\xde\x6a"
      "\x6d\x6d\x6d\xad\x92\x3a\x59\xcd\x39\x9c\x73\x3c\xe7\xd9\x9c\x33\x39\x17"
      "\x73\x5e\xcb\x79\x3d\xe7\x8d\x9c\x37\x73\xde\xca\xb9\x9a\xf3\x76\xce\x56"
      "\xce\xe8\xcb\xf7\x9f\xb3\x9a\x73\x38\xe7\x78\xce\xb3\x39\x67\x72\x2e\xe6"
      "\xbc\x96\xf3\x7a\xce\x1b\x39\x6f\xe6\xbc\x95\x73\x35\xe7\xed\x9c\xad\x9c"
      "\xe1\x73\xd2\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x3c"
      "\x22\x7d\x51\xc4\xf7\x22\xc5\x8f\x7e\xd3\x4a\x6b\x45\xe7\xfc\xb2\x33\xd1"
      "\xc9\xdb\xe6\xb9\xc2\x13\xed\x7f\x03\x00\x00\xff\xff\xf3\x71\x47\x5a",
      3131);
  syz_mount_image(0x20000c40, 0x20000c80, 4, 0x20000200, 1, 0xc3b, 0x20001940);
  memcpy((void*)0x20002000, "./bus\000", 6);
  syscall(__NR_open, 0x20002000ul, 0x143142ul, 0ul);
  memcpy((void*)0x20000400, "./bus\000", 6);
  res = syscall(__NR_open, 0x20000400ul, 0x14113eul, 0ul);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000080, "cpu.stat\000", 9);
  res = syscall(__NR_openat, 0xffffff9c, 0x20000080ul, 0x275aul, 0ul);
  if (res != -1)
    r[1] = res;
  syscall(__NR_mmap, 0x20000000ul, 0x3000ul, 4ul, 0x12ul, r[1], 0ul);
  syscall(__NR_ftruncate, r[1], 4ul);
  inject_fault(2);
  syscall(__NR_write, r[0], 0x20000000ul, 0x208e24bul);
  return 0;
}
