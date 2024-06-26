// https://syzkaller.appspot.com/bug?id=9bcce2bee542fa9e87b18b57267db95c9e91db0e
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
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
  if (sourcelen < ZLIB_HEADER_WIDTH) {
    errno = EMSGSIZE;
    return -1;
  }
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
    if (strstr(opts, "errors=panic") || strstr(opts, "errors=remount-ro") == 0)
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

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);

  memcpy((void*)0x20000180, "hfsplus\000", 8);
  memcpy((void*)0x20000640, "./file0\000", 8);
  memcpy(
      (void*)0x20000cc0,
      "\x78\x9c\xec\xdd\xc1\x6f\x1c\x57\x1d\x07\xf0\xef\x6c\xe2\xcd\x3a\x48\xe9"
      "\x26\x4d\x9a\x80\x90\xb0\xca\x01\xd4\x88\xd4\xf6\x56\x51\x90\x90\x80\x52"
      "\x90\x85\x2a\x54\x89\x4b\xaf\x56\xb2\xa9\xad\x6c\xd2\xc8\xde\x22\x37\x07"
      "\x14\x10\xe7\xf6\x5f\x28\x07\x73\xe6\xc0\x09\x05\x29\x07\xce\xfc\x0b\x46"
      "\x3d\x22\xb8\xfb\x66\x34\xb3\xb3\xde\x4d\xb2\x75\xed\x26\xcd\xae\xdd\xcf"
      "\x47\x9a\x7d\xef\xcd\x9b\x79\xf3\x9b\x9f\x67\x46\x33\xb3\xb2\x36\xc0\x37"
      "\xd6\xca\xfb\x99\x7b\x9c\x22\x2b\x57\xdf\xdd\x2a\xdb\x3b\xdb\x9d\xde\xce"
      "\x76\xe7\xee\xb0\x9e\xe4\x4c\x92\x46\xd2\x4a\x52\x94\xb3\xff\x96\xe4\xf3"
      "\xe4\x61\x06\x53\xbe\x3d\xec\x18\x2b\x9f\x51\x7c\xba\x72\x73\xed\xd1\x27"
      "\x97\x07\xad\x56\x3d\x55\xcb\x17\x07\xad\x77\x38\xfb\xb1\xb4\x07\xb1\x56"
      "\xe5\x8b\x1a\x6f\xf9\x28\xe3\x35\x27\xcd\x1c\xed\xe1\x42\x92\x0b\x75\x09"
      "\x53\xb7\x37\xf4\xef\x89\xdd\xcf\x79\x5e\x02\x00\xb3\xac\x48\x4e\x4d\x9a"
      "\xdf\x4e\xce\xd6\x37\xeb\xe5\x73\xc0\xe0\xae\x78\x70\x8f\x7d\xac\x3d\x9c"
      "\x76\x00\x00\x00\x00\xf0\x12\xbc\xb2\x9b\xdd\x6c\xe5\xdc\xb4\xe3\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x80\xe3\xa4\xfe\xfd\xff\xa2\x9e\x1a\xc3\xfa"
      "\x42\x8a\xe1\xef\xff\x37\xeb\x79\xa9\xeb\xc7\xda\xe3\x69\x07\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x2f\xc0\xf7\x76\xb3\x9b\xad\x9c\x1b\xb6\xf7\x8a\xea"
      "\x3b\xff\xd7\xab\xc6\xc5\xea\xf3\x5b\xf9\x28\x9b\xe9\x66\x23\xd7\xb2\x95"
      "\xd5\xf4\xd3\xcf\x46\x96\x92\xb4\xc7\x06\x6a\x6e\xad\xf6\xfb\x1b\x4b\x87"
      "\x58\x73\x79\xe2\x9a\xcb\x2f\x67\x7f\x01\x00\x00\x00\x00\x00\x00\xe0\x84"
      "\xfa\x63\x56\x46\xdf\xff\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xc0"
      "\x2c\x28\x92\x53\x83\xa2\x9a\x2e\x0e\xeb\xed\x34\x4e\x27\x69\x25\x69\x96"
      "\xcb\x3d\x4c\x1e\x0d\xeb\xc7\xd9\xe3\x69\x07\x00\x00\x00\x00\x2f\xc1\x2b"
      "\xbb\xd9\xcd\x56\xce\x0d\xdb\x7b\x45\xf5\xcc\xff\x5a\xf5\xdc\xdf\xca\x47"
      "\xb9\x97\x7e\xd6\xd3\x4f\x2f\xdd\xdc\x4a\x63\x7f\xc5\xc6\xce\x76\xa7\xb7"
      "\xb3\xdd\xb9\x5b\x4e\xcf\x8e\xfb\xf3\xff\x1d\x29\x8c\x66\x39\x62\x06\xef"
      "\x1e\x26\x6f\x79\xb1\x5a\xe2\xd2\xfe\x1a\x2b\xf9\x55\x7e\x9b\xab\x59\xc8"
      "\x7b\xd9\xc8\x7a\x7e\x97\xd5\xf4\xd3\xcd\x42\xde\xa9\x6a\xab\x29\xd2\xae"
      "\xdf\x5e\xb4\x87\x71\x4e\x8e\xf7\x67\x4f\xb4\xde\xfb\xb2\x58\xaf\x54\x91"
      "\xcc\xe7\x76\xd6\xab\xd8\xae\xe5\x66\x3e\x4c\xaf\xca\xce\x30\x3f\x57\x0e"
      "\xde\xe2\x1f\xca\xec\x14\x3f\xad\x1d\x32\x47\xb7\xea\xb2\xdc\xa3\x5f\xd7"
      "\xe5\x6c\x68\x57\x19\x99\xdb\xcf\xc8\x62\x9d\xfb\x32\x1b\xe7\x0f\xce\xc4"
      "\x11\x8f\x93\xa7\xb7\xb4\x94\xc6\xfe\x3b\xa8\x8b\x5f\x43\xce\xcf\xd6\x65"
      "\x99\xeb\x77\x66\x3a\xe7\xcb\x63\x47\xdf\x6b\xe5\xde\x17\xd5\x4b\xba\x64"
      "\xd2\xf9\xb9\xb8\xfc\xdf\xfb\x6b\xbd\x7b\x77\xd6\x6e\x6f\x5e\x9d\x9d\x5d"
      "\xfa\x8a\x9e\xce\x44\x67\x2c\x13\x97\x0f\x3e\x26\x4e\x58\x26\x9a\x75\x36"
      "\x06\x57\xd1\xa3\x5d\x2d\x5f\xaf\xd6\x3d\x97\xf5\xfc\x26\x1f\xe6\x56\xba"
      "\xb9\x91\xc5\xdc\xc8\xf5\xbc\x95\x4e\xae\xe7\xc7\xb9\x3e\x96\xd7\x4b\x87"
      "\x38\xd7\x1a\x47\x3b\xd7\xbe\xff\xc3\xba\x32\x97\xe4\x97\x75\x39\x1b\xca"
      "\xbc\x9e\x1f\xcb\xeb\xf8\x95\xae\x5d\xf5\x8d\xcf\x19\x65\xe9\xc2\x20\x3b"
      "\x69\x25\x2f\xea\x8a\x74\xfa\x3b\x75\xa5\x3c\x58\xdf\x9e\xb9\x2b\xd2\xf9"
      "\xa7\xae\xcd\xc3\x4c\xbc\x7a\xf0\xf1\xf2\xe7\xbd\xf2\x73\xb3\x77\xef\xce"
      "\xc6\xda\xea\xfd\x43\x6e\xef\x07\x75\x59\x66\xe0\x17\x33\x95\x89\xf2\x78"
      "\xb9\x50\xfe\xb1\xaa\xd6\x93\x47\x47\xd9\xf7\xea\xc4\xbe\xa5\xaa\xef\xe2"
      "\x7e\x5f\xe3\x99\xbe\x4b\xfb\x7d\x5f\x76\xa6\x36\xeb\x7b\xb8\x67\x47\x5a"
      "\xae\xfa\x2e\x4f\xec\xeb\x54\x7d\x57\xc6\xfa\x26\xdd\xe5\x00\x30\xf3\xce"
      "\xbe\x71\xb6\x39\xff\x9f\xf9\x7f\xcd\x7f\x36\xff\xa7\xf9\xb5\xf9\x77\x5b"
      "\x6f\x9f\xb9\x71\xe6\xbb\xcd\xcc\xfd\xf3\xf4\xdf\x4f\xfd\xb5\xf1\x97\xc6"
      "\x4f\x8a\x37\xf2\x59\x7e\x3f\x7a\xfe\x07\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xbe\xba\xcd"
      "\x8f\x1f\xdc\x59\xed\xf5\xba\x1b\x27\xb9\xb2\x77\x2a\x99\x81\x30\x54\x54"
      "\x8e\x4f\x65\xda\x57\x26\xe0\xeb\xf6\x66\xff\xee\xfd\x37\x37\x3f\x7e\xf0"
      "\xa3\xf5\xbb\xab\x1f\x74\x3f\xe8\xde\x7b\x6b\xe9\xc1\xb0\xeb\xf6\x7a\xaf"
      "\xbb\x38\xf8\x9c\x72\x94\x00\xc0\x8b\x34\xba\xe9\x9f\x76\x24\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\xc0\x17\x79\x19\xff\x4e\x3c\xed\x7d\x04"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x4e"
      "\xb6\x95\xf7\x33\xf7\x38\x45\x96\x16\xaf\x2d\x96\xed\x9d\xed\x4e\xaf\x9c"
      "\x86\xf5\xd1\x92\xad\x24\x45\x59\xf9\x47\x92\xcf\x93\x87\x19\x4c\x69\x8f"
      "\x0d\x57\x7c\xd1\x76\x8a\x4f\x57\x6e\xae\x3d\xfa\xe4\xf2\x68\xac\xd6\x70"
      "\xf9\xe2\xa0\xf5\x0e\xe7\x89\x58\x1a\x4f\xc5\xf4\xbc\xe3\x2d\x3f\xf7\x78"
      "\xa3\x3d\x5c\x48\x72\xa1\x2e\x61\xea\xfe\x1f\x00\x00\xff\xff\x02\x0c\xfb"
      "\x9b",
      1495);
  syz_mount_image(0x20000180, 0x20000640, 0, 0x20000240, 1, 0x5d7, 0x20000cc0);
  return 0;
}
