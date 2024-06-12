// https://syzkaller.appspot.com/bug?id=82148e8173e51abf3a46e231a7e233950c815ca7
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
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

int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul, /*prot=*/7ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);

  memcpy((void*)0x20000180, "hfsplus\000", 8);
  memcpy((void*)0x20000100, "./file1\000", 8);
  *(uint32_t*)0x200000c0 = 0;
  memcpy((void*)0x200000c4,
         "\xbd\x08\xca\x0a\x8d\x08\x98\x17\x83\x00\x5b\x5c\xf5\xcf\xe7\x69\xb9"
         "\x57\xdc\x06\x00\x4b\xb1\x44\x6c\x0c",
         26);
  *(uint32_t*)0x200000de = -1;
  memcpy(
      (void*)0x20001080,
      "\x78\x9c\xec\xdd\x4f\x6c\x1c\x57\x1d\x07\xf0\xef\x6e\xd6\x76\x1c\xa4\xd4"
      "\x6d\x93\x34\xa0\x4a\x44\x8d\x54\x10\x16\x89\xff\xc8\x29\xe6\x42\x40\xa8"
      "\xf2\xa1\xa0\xaa\x1c\x38\x71\xb0\x12\xa7\xb1\xb2\x49\x8b\xed\x22\x27\x42"
      "\x25\xfc\xbd\x70\x80\x43\x2e\xdc\x8a\x84\x6f\x9c\x90\xb8\x47\x2a\x67\xb8"
      "\xf5\xea\x63\x25\x24\x24\xd4\x53\x38\x2d\x9a\xd9\xd9\xf5\xda\xde\x38\xeb"
      "\x38\xf2\x6e\xda\xcf\x27\x7a\xfb\xde\xcc\xdb\x79\xf3\x7b\xbf\x9d\x9d\xd9"
      "\x59\x2b\xda\x00\x5f\x58\x4b\xd3\x69\x3c\x4c\x2d\x4b\xd3\x6f\x6d\x16\xcb"
      "\xdb\x5b\xf3\xcd\xed\xad\xf9\xdb\x9d\x76\x92\x89\x24\xf5\xa4\xd1\xae\x52"
      "\x1b\x4b\x6a\x1f\x27\x57\xd3\x2e\xf9\x72\xb1\xb2\x1a\xae\xf6\xb8\xfd\x3c"
      "\x58\x5d\x7c\xe7\x93\xcf\xb6\x3f\x6d\x2f\x35\xaa\xf2\x66\x9e\xb4\xdd\x3e"
      "\x57\xfa\xad\xbc\x5f\x95\x5c\x48\x72\xa2\xaa\x8f\x60\xd7\x78\xd7\x8e\x3c"
      "\x5e\xad\x3b\xc3\x22\x61\x17\x3b\x89\x83\x61\x1b\x4b\xd2\xda\xe5\xa7\xe7"
      "\x76\x7a\x9e\x68\xf0\xf7\x2d\x30\xb2\x6a\xed\xeb\xe6\x3e\x53\xc9\xa9\x24"
      "\x27\xab\xcf\x01\xed\xab\x62\xfb\x83\xc0\x73\xed\xfe\xb0\x03\x00\x00\x00"
      "\x80\x63\xf0\xc2\xef\xcb\x5b\xf8\xd3\xc3\x8e\x03\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x9e\x27\xd5\xef\xff\xd7\xaa\x52\xef\xb4\x2f\xa4\xd6\xf9\xfd"
      "\xff\xf1\x6a\x5d\xaa\xf6\x73\xed\xe1\xb0\x03\x00\x00\x00\x00\x00\x00\x00"
      "\x80\x67\xe0\xab\x8f\xf2\x28\x9b\x39\xdd\x59\x6e\xd5\xca\xbf\xf9\xbf\x56"
      "\x2e\x9c\x29\x1f\xbf\x94\x0f\xb2\x9e\x95\xac\xe5\x52\x36\xb3\x9c\x8d\x6c"
      "\x64\x2d\xb3\x49\xa6\x7a\x06\x1a\xdf\x5c\xde\xd8\x58\x9b\x1d\x60\xcb\xb9"
      "\xbe\x5b\xce\x0d\x18\xf0\xe4\xd1\xe7\x0c\x00\x00\x00\x00\x00\x00\x00\xa3"
      "\xae\x71\xf8\x4d\x7e\x95\xa5\x9d\xbf\xff\x03\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\xc0\x08\xa8\xfd\x37\x39\x51\xd4\x55\x39\xd3\x69\x4f\xa5\xde"
      "\x48\x72\xb2\x68\x17\x2b\xee\x27\xff\x4a\x32\x3e\xec\x80\x8f\xea\xe1\xb0"
      "\x03\x00\x00\x00\x80\x63\xf0\xc2\xa3\x3c\xca\x66\x4e\x77\x96\x5b\xb5\xf2"
      "\x9e\xff\x5c\x79\xdf\x7f\x32\x1f\xe4\x4e\x36\xb2\x9a\x8d\x34\xb3\x92\xeb"
      "\xe5\x77\x01\xed\xbb\xfe\xfa\xf6\xd6\x7c\x73\x7b\x6b\xfe\x76\x51\xf6\x8f"
      "\xfb\xdd\xff\x1c\x2a\x8c\x72\xc4\xb4\xbf\x7b\xe8\xbf\xe7\xf3\xe5\x33\x26"
      "\x73\x23\xab\xe5\x9a\x4b\xb9\x96\xf7\xd2\xcc\xf5\xd4\x3f\xac\x57\xa3\x9c"
      "\xef\xc4\xd3\x3f\xae\x5f\x16\x31\xd5\xbe\x53\x19\x30\xb2\xeb\x55\x5d\xcc"
      "\xfc\x8f\x55\x3d\x1a\xa6\xca\x8c\x8c\x75\x33\x32\x53\xc5\x56\x64\xe3\xc5"
      "\x83\x33\x51\xbe\x3a\x8d\xa7\xde\xd3\x6c\xea\xdd\x6f\x7e\xce\x3c\xa3\x9c"
      "\x8f\xf5\xb4\xdf\xf8\x4b\xbb\x2e\xe6\xf3\xbb\x91\xce\xf9\x5c\xea\xe9\x1c"
      "\x7d\xe7\x0e\xce\x44\xf2\xb5\xbf\xff\xf5\xc7\x37\x9b\x77\x6e\xdd\xbc\xb1"
      "\x3e\x3d\x3a\x53\x3a\x84\x89\x56\xab\xd5\x69\xef\xcd\xc4\x7c\x4f\x26\x5e"
      "\xf9\xdc\x65\x62\xea\x80\xbe\x99\x32\x13\x67\xbb\xcb\x4b\x79\x33\x3f\xca"
      "\x74\x2e\xe4\xed\xac\x65\x35\x3f\xcb\x72\x36\xb2\x92\x0b\xf9\x7e\x96\x73"
      "\x22\xcb\xd5\xf1\x5c\x3c\x4e\x1d\x9c\xa9\xab\xbb\x96\xde\x7e\x52\x94\xe3"
      "\x65\xa4\x63\xd5\x59\x74\xf0\x98\x36\xb2\x9c\xd7\xca\x6d\x4f\x67\x35\x3f"
      "\xcc\x7b\xb9\x9e\x95\x5c\x29\xff\xcd\x65\x36\x6f\x64\x21\x0b\x59\xec\x79"
      "\x85\xcf\x0e\xf0\xae\xaf\x1f\xee\x4c\x7b\xf1\xeb\x3d\x5f\x26\x3f\xf8\x41"
      "\x32\x39\xd8\x76\xc7\xa0\x08\xec\xc5\xee\xd5\x69\xf7\x39\x77\xaa\xec\xeb"
      "\x5d\xb3\x93\xa5\x97\x9e\xfd\xf5\xa8\xf1\x95\xaa\x51\xec\xe3\xd7\x55\x3d"
      "\x1a\xf6\x66\x62\xb6\x27\x13\x2f\x1f\x9c\x89\x3f\x97\xa7\x95\xf5\xe6\x9d"
      "\x5b\xc9\xf2\xfb\x03\xee\xef\xf5\xaa\x2e\xde\x47\xbf\x1d\x85\xab\x44\x37"
      "\x80\xe2\x78\x79\xa9\x7b\x85\xdd\x7d\x74\x14\x7d\x2f\xf7\xed\x9b\x2d\xfb"
      "\xce\x74\xfb\xea\xfb\xfa\xce\x76\xfb\x9e\xf4\x4e\x1d\xaf\x3e\xc3\xed\x1f"
      "\x69\xae\xec\x7b\xa5\x6f\xdf\x7c\xd9\x77\xbe\xa7\xaf\xdf\xe7\x2d\x00\x46"
      "\xcc\x4f\xf6\xad\x39\xf5\x8d\x53\xe3\x93\xff\x9e\xfc\xe7\xe4\x47\x93\xbf"
      "\x99\xbc\x39\xf9\xd6\xc9\xef\x4d\x7c\x6b\xe2\xd5\xf1\x8c\xfd\x63\xec\xdb"
      "\x8d\x99\x13\xaf\xd7\x5f\xad\xfd\x2d\x1f\xe5\xc3\x9d\xfb\x7f\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\xe0\xe9\xad\xdf\xbd\x77\x6b\xb9\xd9\x5c\x59\xdb\xd3\x68\xb5\x5a"
      "\xbf\xe8\xae\xf9\x43\xff\xe7\x0c\xbb\x31\x96\x64\x38\x7b\x1f\x4f\x72\x94"
      "\x71\x26\x06\x7a\xf2\xf8\x63\x5f\x9d\x83\x1a\x7f\xea\x93\xa8\xc3\x8e\x93"
      "\x1c\x6e\xa7\x13\xd5\xd1\xb4\xa7\xab\xf3\x4b\x83\x77\xef\xdd\xfa\x5f\xab"
      "\xd5\x1a\x91\xc3\x66\x80\x46\xab\x32\x2a\xf1\x0c\xa3\x31\xc4\x93\x12\x70"
      "\x2c\x2e\x6f\xdc\x7e\xff\xf2\xfa\xdd\x7b\xdf\x5c\xbd\xbd\xfc\xee\xca\xbb"
      "\x2b\x77\x16\x17\x16\x16\x67\x16\x17\xae\xcc\x5f\xbe\xb1\xda\x5c\x99\x69"
      "\x3f\x66\xcf\x0f\xb5\x02\x9f\x07\x3b\x17\xfd\x61\x47\x02\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x0c\xea\x38\xfe\x3b\xc1\xb0\xe7\x08\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x3c"
      "\xdf\x96\xa6\x27\xaa\xd6\xa5\x99\xe2\x71\x7b\x6b\xbe\x59\x94\xb2\x3d\xd1"
      "\xae\xdb\x1a\x49\xea\x49\x6a\x3f\x4f\x6a\x1f\x27\x57\xd3\x2e\x99\xea\x19"
      "\xae\xf6\xb8\xfd\x3c\x58\x5d\x7c\xe7\x93\xcf\xb6\x3f\xdd\x19\xab\xd1\x79"
      "\x7e\x7d\xd7\x76\x63\x4f\x33\x8b\xfb\x55\xc9\x85\x24\x27\xaa\xba\xd7\xc9"
      "\x23\x8c\x77\xad\xcf\x78\x87\x53\xeb\xce\xb0\x48\xd8\xc5\x4e\xe2\x60\xd8"
      "\xfe\x1f\x00\x00\xff\xff\x73\x92\x09\x5e",
      1612);
  syz_mount_image(/*fs=*/0x20000180, /*dir=*/0x20000100, /*flags=*/0x50,
                  /*opts=*/0x200000c0, /*chdir=*/0x4a, /*size=*/0x64c,
                  /*img=*/0x20001080);
  memcpy((void*)0x20000080, "./file1\000", 8);
  syscall(__NR_truncate, /*file=*/0x20000080ul, /*len=*/0ul);
  return 0;
}
