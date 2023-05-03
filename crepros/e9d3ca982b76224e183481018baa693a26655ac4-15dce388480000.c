// https://syzkaller.appspot.com/bug?id=e9d3ca982b76224e183481018baa693a26655ac4
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

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  memcpy((void*)0x20000040, "ext4\000", 5);
  memcpy((void*)0x20000000, "./bus\000", 6);
  memcpy((void*)0x200000c0, "usrjquota=", 10);
  *(uint8_t*)0x200000ca = 0x2c;
  memcpy((void*)0x200000cb, "abort", 5);
  *(uint8_t*)0x200000d0 = 0x2c;
  memcpy((void*)0x200000d1, "debug_want_extra_isize", 22);
  *(uint8_t*)0x200000e7 = 0x3d;
  sprintf((char*)0x200000e8, "0x%016llx", (long long)0x2c);
  *(uint8_t*)0x200000fa = 0x2c;
  memcpy((void*)0x200000fb, "test_dummy_encryption", 21);
  *(uint8_t*)0x20000110 = 0x2c;
  memcpy((void*)0x20000111, "nojournal_checksum", 18);
  *(uint8_t*)0x20000123 = 0x2c;
  memcpy((void*)0x20000124, "noauto_da_alloc", 15);
  *(uint8_t*)0x20000133 = 0x2c;
  *(uint8_t*)0x20000134 = 0;
  memcpy(
      (void*)0x20000300,
      "\x78\x9c\xec\xdc\xcf\x6b\x1c\x55\x1c\x00\xf0\xef\xcc\x26\xa9\xfd\x65\x62"
      "\xad\x3f\xfa\x43\x8d\x56\x31\xf8\x23\x69\xd2\xaa\x3d\x78\x50\x51\x10\x54"
      "\x10\xf4\x50\x8f\x31\x49\x4b\xed\xb6\x91\x26\x82\x2d\x45\xab\x48\x3d\x4a"
      "\xc1\xbb\x78\x14\xfc\x0b\x3c\xe9\x45\xd4\x93\xe0\x55\x4f\x5e\xa4\x50\xa4"
      "\x97\xd6\x9e\x56\x66\x77\x26\xd9\x6c\x76\xd3\xec\xba\xc9\xb6\xee\xe7\x03"
      "\x93\xbc\x37\xf3\x96\xf7\xbe\x3b\xf3\x76\xdf\x9b\x97\x49\x00\x7d\x6b\x34"
      "\xfb\x91\x44\xec\x88\x88\xdf\x23\x62\xb8\x96\x5d\x59\x60\xb4\xf6\xeb\xfa"
      "\xd5\xf3\x33\xff\x5c\x3d\x3f\x93\x44\xa5\xf2\xd6\xdf\x49\xb5\xdc\xb5\xab"
      "\xe7\x67\x8a\xa2\xc5\xeb\xb6\xe7\x99\xb1\x34\x22\xfd\x2c\x89\x7d\x4d\xea"
      "\x5d\x38\x7b\xee\xe4\x74\xb9\x3c\x77\x26\xcf\x4f\x2c\x9e\x7a\x7f\x62\xe1"
      "\xec\xb9\xa7\x4f\x9c\x9a\x3e\x3e\x77\x7c\xee\xf4\xd4\x91\x23\x87\x0f\x4d"
      "\x3e\xf7\xec\xd4\x33\x5d\x89\x33\x8b\xeb\xda\xde\x8f\xe6\xf7\xef\x79\xf5"
      "\x9d\x4b\x6f\xcc\x1c\xbd\xf4\xee\xcf\xdf\x26\x45\xfc\x0d\x71\x74\xc9\xe8"
      "\x5a\x07\x1f\xab\x54\xba\x5c\x5d\x6f\xed\xac\x4b\x27\x03\x3d\x6c\x08\x6d"
      "\x29\x45\x44\x76\xba\x06\xab\xfd\x7f\x38\x4a\xb1\x7c\xf2\x86\xe3\x95\x4f"
      "\x7b\xda\x38\x60\x43\x55\x2a\x95\xca\x96\xd6\x87\x2f\x54\x80\xff\xb1\x24"
      "\x7a\xdd\x02\xa0\x37\x8a\x2f\xfa\x6c\xfe\x5b\x6c\x9b\x34\xf4\xb8\x25\x5c"
      "\x79\xb1\x36\x01\xca\xe2\xbe\x9e\x6f\xb5\x23\x03\x91\xe6\x65\x06\x1b\xe6"
      "\xb7\xdd\x34\x1a\x11\x47\x2f\xdc\xf8\x2a\xdb\x62\x63\xee\x43\x00\x00\xac"
      "\xf0\x7d\x36\xfe\x79\xaa\xd9\xf8\x2f\x8d\x7b\xeb\xca\xdd\x99\xaf\xa1\x8c"
      "\x44\xc4\x5d\x11\xb1\x2b\x22\xee\x8e\x88\xdd\x11\x71\x4f\x44\xb5\xec\x7d"
      "\x11\x71\x7f\x9b\xf5\x37\x2e\x92\xac\x1e\xff\xa4\x97\x3b\x0a\x6c\x9d\xb2"
      "\xf1\xdf\xf3\xf9\xda\xd6\xca\xf1\x5f\x31\xfa\x8b\x91\x52\x9e\xdb\x59\x8d"
      "\x7f\x30\x39\x76\xa2\x3c\x77\x30\x7f\x4f\xc6\x62\x70\x4b\x96\x9f\x5c\xa3"
      "\x8e\x1f\x5e\xfe\xed\x8b\x56\xc7\xea\xc7\x7f\xd9\x96\xd5\x5f\x8c\x05\xf3"
      "\x76\x5c\x1e\x68\xb8\x41\x37\x3b\xbd\x38\xfd\x5f\x62\xae\x77\xe5\x93\x88"
      "\xbd\x03\xcd\xe2\x4f\x96\x56\x02\x92\x88\xd8\x13\x11\x7b\x3b\xac\xe3\xc4"
      "\x13\xdf\xec\x6f\x75\xec\xe6\xf1\xaf\xa1\x0b\xeb\x4c\x95\xaf\x23\x1e\xaf"
      "\x9d\xff\x0b\xd1\x10\x7f\x21\x59\x7b\x7d\x72\xe2\x8e\x28\xcf\x1d\x9c\x28"
      "\xae\x8a\xd5\x7e\xf9\xf5\xe2\x9b\xad\xea\xef\x38\xfe\x1b\x0d\x13\xb8\x0e"
      "\x65\xe7\x7f\x5b\xd3\xeb\x7f\x29\xfe\x91\xa4\x7e\xbd\x76\xa1\xfd\x3a\x2e"
      "\xfe\xf1\x79\xcb\x39\x4d\xa7\xd7\xff\x50\xf2\x76\x35\x3d\x94\xef\xfb\x70"
      "\x7a\x71\xf1\xcc\x64\xc4\x50\xf2\x7a\xbe\xff\x85\xe5\xfd\x53\xcb\xaf\x2d"
      "\xf2\x45\xf9\x2c\xfe\xb1\x03\xcd\xfb\xff\xae\x58\x7e\x27\xf6\x45\x44\x76"
      "\x11\x3f\x10\x11\x0f\x46\xc4\x43\x79\xdb\x1f\x8e\x88\x47\x22\xe2\xc0\x1a"
      "\xf1\xff\xf4\xd2\xa3\xef\x75\x1e\xff\xc6\xca\xe2\x9f\x6d\xeb\xfc\x2f\x27"
      "\x86\xa2\x71\x4f\xf3\x44\xe9\xe4\x8f\xdf\xad\xa8\x74\xa4\x9d\xf8\xb3\xf3"
      "\x7f\xb8\x9a\x1a\xcb\xf7\xac\xe7\xf3\x6f\x3d\xed\xea\xec\x6a\x06\x00\x00"
      "\x80\xdb\x4f\x1a\x11\x3b\x22\x49\xc7\x97\xd2\x69\x3a\x3e\x5e\xfb\x1b\xfe"
      "\xdd\xb1\x2d\x2d\xcf\x2f\x2c\x3e\x79\x6c\xfe\x83\xd3\xb3\xb5\x67\x04\x46"
      "\x62\x30\x2d\xee\x74\x0d\xd7\xdd\x0f\x9d\xcc\xa7\xf5\x45\x7e\xaa\x21\x7f"
      "\x28\xbf\x6f\xfc\x65\x69\x6b\x35\x3f\x3e\x33\x5f\x9e\xed\x75\xf0\xd0\xe7"
      "\xb6\xb7\xe8\xff\x99\xbf\x4a\xbd\x6e\x1d\xb0\xe1\x3c\xaf\x05\xfd\x4b\xff"
      "\x87\x7e\x95\x78\x5e\x1b\xfa\x98\xfe\x0f\xfd\xab\x49\xff\xdf\xda\x8b\x76"
      "\x00\x9b\xaf\xd9\xf7\xff\xc7\x3d\x68\x07\xb0\xf9\x06\xe2\xb5\x3f\xeb\xb2"
      "\x96\xfd\xa0\x8f\x98\xff\x43\xff\xd2\xff\xa1\x7f\xe9\xff\xd0\x97\x16\xb6"
      "\xc6\xcd\x1f\x92\x97\x90\x58\x95\x88\xf4\x96\x68\x46\x4f\x13\xd1\xc6\x7f"
      "\x9a\xb8\xdd\x12\xbd\xfe\x64\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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
      "\x00\xe8\x8e\x7f\x03\x00\x00\xff\xff\x69\x20\xee\x75",
      1093);
  syz_mount_image(0x20000040, 0x20000000, 0xe, 0x200000c0, 6, 0x445,
                  0x20000300);
  memcpy((void*)0x200002c0, "./file0\000", 8);
  res = syscall(__NR_openat, 0xffffff9c, 0x200002c0ul, 0ul, 0ul);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000180, "./bus\000", 6);
  syscall(__NR_mkdirat, r[0], 0x20000180ul, 0ul);
  memcpy((void*)0x20000080, "blkio.bfq.empty_time\000", 21);
  syscall(__NR_openat, r[0], 0x20000080ul, 0x275aul, 0ul);
  return 0;
}
