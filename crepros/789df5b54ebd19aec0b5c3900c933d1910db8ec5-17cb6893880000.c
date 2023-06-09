// https://syzkaller.appspot.com/bug?id=789df5b54ebd19aec0b5c3900c933d1910db8ec5
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

  memcpy((void*)0x20000440, "ext4\000", 5);
  memcpy((void*)0x20000480, "./file0\000", 8);
  memcpy((void*)0x20000080, "inode_readahead_blks", 20);
  *(uint8_t*)0x20000094 = 0x3d;
  sprintf((char*)0x20000095, "0x%016llx", (long long)0);
  *(uint8_t*)0x200000a7 = 0x2c;
  memcpy((void*)0x200000a8, "block_validity", 14);
  *(uint8_t*)0x200000b6 = 0x2c;
  memcpy((void*)0x200000b7, "debug_want_extra_isize", 22);
  *(uint8_t*)0x200000cd = 0x3d;
  sprintf((char*)0x200000ce, "0x%016llx", (long long)0x5c);
  *(uint8_t*)0x200000e0 = 0x2c;
  memcpy((void*)0x200000e1, "noload", 6);
  *(uint8_t*)0x200000e7 = 0x2c;
  memcpy((void*)0x200000e8, "nobh", 4);
  *(uint8_t*)0x200000ec = 0x2c;
  memcpy((void*)0x200000ed, "quota", 5);
  *(uint8_t*)0x200000f2 = 0x2c;
  *(uint8_t*)0x200000f3 = 0;
  memcpy(
      (void*)0x20000900,
      "\x78\x9c\xec\xdb\xcb\x6f\x1b\x45\x18\x00\xf0\x6f\xd7\x49\x0b\x7d\x90\x50"
      "\xca\xa3\x69\x01\x43\x41\x44\x3c\x92\x26\x2d\xd0\x03\x17\x10\x48\x1c\x40"
      "\x42\x82\x43\x39\x86\x24\xad\x42\xdd\x06\x35\x41\xa2\x55\x04\x01\xa1\x72"
      "\x44\x95\xb8\x23\x8e\x48\xfc\x05\x9c\xe0\x82\x80\x13\x12\x57\xb8\xa3\x4a"
      "\x15\xca\xa5\x85\x93\xd1\xda\xbb\x89\x93\xda\x21\x0f\xa7\x2e\xec\xef\x27"
      "\x6d\x32\xe3\x1d\x6b\xe6\xf3\xec\xd8\xb3\x33\x76\x00\xa5\x55\xcd\xfe\x24"
      "\x11\xfb\x22\xe2\xb7\x88\x18\x68\x66\x57\x17\xa8\x36\xff\xdd\x58\x5a\x98"
      "\xfc\x6b\x69\x61\x32\x89\x7a\xfd\xcd\x3f\x93\x46\xb9\xeb\x4b\x0b\x93\x45"
      "\xd1\xe2\x79\x7b\xf3\xcc\x70\x1a\x91\x7e\x9a\xc4\xe1\x36\xf5\xce\x5d\xbc"
      "\x74\x76\xa2\x56\x9b\xbe\x90\xe7\x47\xe7\xcf\xbd\x37\x3a\x77\xf1\xd2\x33"
      "\x33\xe7\x26\xce\x4c\x9f\x99\x3e\x3f\x7e\xf2\xe4\x89\xe3\x63\xcf\x3f\x37"
      "\xfe\x6c\x57\xe2\xcc\xe2\xba\x3e\xf4\xe1\xec\x91\x43\xaf\xbe\x7d\xe5\xf5"
      "\xc9\x53\x57\xde\xf9\xe9\x9b\xac\xbd\xfb\xf2\xf3\xad\x71\x74\x4b\x35\xaa"
      "\xab\x5f\xcb\x16\x8f\x77\xbb\xb2\x1e\xdb\xdf\x92\x4e\xfa\x7a\xd8\x10\x36"
      "\xa5\x12\x11\x59\x77\xf5\x37\xc6\xff\x40\x54\x62\xa5\xf3\x06\xe2\x95\x4f"
      "\x7a\xda\x38\x60\x47\xd5\xeb\xf5\xfa\xee\xce\xa7\x17\xeb\xc0\xff\x58\x12"
      "\xbd\x6e\x01\xd0\x1b\xc5\x07\x7d\x76\xff\x5b\x1c\xb7\x68\xea\x71\x5b\xb8"
      "\xf6\x62\xf3\x06\x28\x8b\xfb\x46\x7e\x34\xcf\xf4\x45\x9a\x97\xe9\x5f\x73"
      "\x7f\xdb\x4d\xd5\x88\x38\xb5\xf8\xf7\x97\xd9\x11\x3b\xb4\x0e\x01\x00\xd0"
      "\xea\xbb\x6c\xfe\xf3\x74\xbb\xf9\x5f\x1a\xf7\xb5\x94\xbb\x2b\xdf\x43\x19"
      "\x8c\x88\xbb\x23\xe2\x40\x44\xdc\x13\x11\x07\x23\xe2\xde\x88\x46\xd9\xfb"
      "\x23\xe2\x81\x4d\xd6\x5f\x5d\x93\xbf\x79\xfe\x93\x5e\xdd\x52\x60\x1b\x94"
      "\xcd\xff\x5e\xc8\xf7\xb6\x56\xcf\xff\x8a\xd9\x5f\x0c\x56\xf2\xdc\xfe\x46"
      "\xfc\xfd\xc9\xe9\x99\xda\xf4\xb1\xfc\x35\x19\x8e\xfe\xdd\x59\x7e\x6c\x9d"
      "\x3a\xbe\x7f\xf9\xd7\xcf\x3b\x9d\x6b\x9d\xff\x65\x47\x56\x7f\x31\x17\xcc"
      "\xdb\x71\xb5\x6f\xcd\x02\xdd\xd4\xc4\xfc\xc4\x76\x62\x6e\x75\xed\xe3\x88"
      "\xa1\xbe\x76\xf1\x27\xcb\x3b\x01\x49\x44\x1c\x8a\x88\xa1\x2d\xd6\x31\xf3"
      "\xe4\xd7\x47\x3a\x9d\xfb\xf7\xf8\xd7\xd1\x85\x7d\xa6\xfa\x57\x11\x4f\x34"
      "\xfb\x7f\x31\xd6\xc4\x5f\x48\xd6\xdf\x9f\x1c\xbd\x23\x6a\xd3\xc7\x46\x8b"
      "\xab\xe2\x66\x3f\xff\x72\xf9\x8d\x4e\xf5\x6f\x2b\xfe\x2e\xc8\xfa\x7f\x4f"
      "\xdb\xeb\x7f\x39\xfe\xc1\xa4\x75\xbf\x76\x6e\xf3\x75\x5c\xfe\xfd\xb3\x8e"
      "\xf7\x34\x5b\xbd\xfe\x77\x25\x6f\x15\x2d\x6c\xfc\xfb\x60\x62\x7e\xfe\xc2"
      "\x58\xc4\xae\xe4\xb5\xc6\x83\xbb\xf2\xb2\x8d\xc7\xc7\x57\x9e\x5b\xe4\x8b"
      "\xf2\x59\xfc\xc3\x47\xdb\x8f\xff\x03\xb1\x52\xc1\xe1\x88\xc8\x2e\xe2\x07"
      "\x23\xe2\xa1\x88\x78\x38\x6f\xfb\x23\x11\xf1\x68\x44\x1c\x5d\x27\xfe\x1f"
      "\x5f\x7a\xec\xdd\xad\xc7\xbf\xb3\xb2\xf8\xa7\x36\xd5\xff\x9b\x4f\x54\xce"
      "\xfe\xf0\x6d\xa7\xfa\x37\xd6\xff\x27\x1a\xa9\xe1\xfc\x91\x8d\xbc\xff\x6d"
      "\xb4\x81\xdb\x79\xed\x00\x00\x00\xe0\xbf\x22\x6d\x7c\x07\x3e\x49\x47\x96"
      "\xd3\x69\x3a\x32\xd2\xfc\x0e\xff\xc1\xd8\x93\xd6\x66\xe7\xe6\x9f\x3a\x3d"
      "\xfb\xfe\xf9\xa9\xe6\x77\xe5\x07\xa3\x3f\x2d\x56\xba\x06\x5a\xd6\x43\xc7"
      "\xf2\xb5\xe1\x22\x3f\xbe\x26\x7f\x3c\x5f\x37\xfe\xa2\x72\x67\x23\x3f\x32"
      "\x39\x5b\x9b\xea\x75\xf0\x50\x72\x7b\x3b\x8c\xff\xcc\x1f\x95\x5e\xb7\x0e"
      "\xd8\x71\x7e\xaf\x05\xe5\x65\xfc\x43\x79\x19\xff\x50\x5e\xc6\x3f\x94\x97"
      "\xf1\x0f\xe5\xd5\x6e\xfc\x7f\xd4\x83\x76\x00\xb7\x9e\xcf\x7f\x28\x2f\xe3"
      "\x1f\xca\xcb\xf8\x87\xf2\x32\xfe\xa1\x94\xb6\xf3\xbb\x7e\x89\x32\x27\x22"
      "\xbd\x2d\x9a\x21\xb1\x43\x89\x5e\xbf\x33\x01\x00\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x74\xc7\x3f\x01\x00\x00\xff\xff\x0e\x86\xe6\x7d",
      1060);
  syz_mount_image(0x20000440, 0x20000480, 0x11, 0x20000080, 0, 0x424,
                  0x20000900);
  return 0;
}
