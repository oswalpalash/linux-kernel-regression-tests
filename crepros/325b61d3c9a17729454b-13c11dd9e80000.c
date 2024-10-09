// https://syzkaller.appspot.com/bug?id=ff9fe3de46d66c92f26a627abe9e85d0518c9890
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

  memcpy((void*)0x20000600, "hfsplus\000", 8);
  memcpy((void*)0x20000100, "./file1\000", 8);
  memcpy(
      (void*)0x20001500,
      "\x78\x9c\xec\xdd\xcf\x6f\x23\x57\x1d\x00\xf0\xef\x38\x4e\xb2\x0e\x90\xa6"
      "\x6d\xb6\x5d\x50\x25\xa2\xae\x54\x10\x11\xbb\x71\xac\x14\xc2\x85\x05\x21"
      "\x94\x43\x85\xaa\x22\xc4\x39\xda\xf5\x36\xd6\x3a\x69\x49\x5c\x94\x56\x08"
      "\xc2\xef\x2b\x87\xfe\x01\xe5\x90\x1b\x27\x24\xee\x91\xca\x85\x0b\xdc\x7a"
      "\xcd\xb1\x12\x12\x97\x5e\x48\x4f\x46\x33\x1e\x3b\x8e\x63\xa7\xf1\xe6\x87"
      "\x9d\xf2\xf9\xac\xc6\xef\xbd\x79\xf3\xde\x7c\xe7\x6b\x8f\xc7\xf6\x2a\x9a"
      "\x00\xfe\x6f\xad\x2d\x46\xf1\x20\x92\x58\x5b\x7c\x6d\x37\x6d\x1f\xee\x57"
      "\xea\x87\xfb\x95\xcd\x76\x3d\x22\xa6\x23\xa2\x10\x51\x6c\x15\x91\xfc\xb7"
      "\xd9\x6c\x7e\x98\x55\xe3\x41\xfa\xf0\xe5\x74\x65\x3e\x5d\x32\x68\x3f\xef"
      "\xd7\x56\xdf\xf8\xe8\x93\xc3\x8f\x5b\xad\x62\xbe\x64\xdb\x17\xce\x1a\x77"
      "\x3e\x7b\xf9\x12\x0b\x11\x31\x91\x97\x97\x35\xdf\xc3\x0b\xcf\x97\x74\x8e"
      "\x30\x4d\xd8\xdd\x76\xe2\x60\xd4\x26\x23\xa2\x79\xc2\xcf\xfe\xf1\xc5\x4e"
      "\x4f\x97\x52\xbf\xd1\xb7\xae\x25\x46\xe0\x6a\x25\xad\xeb\x66\xa1\x77\xfd"
      "\x5c\xc4\x4c\x7e\xa2\xa7\x9f\x03\x5a\x57\xc5\x3e\xdb\xdd\x34\x7b\xa3\x0e"
      "\x00\x00\x00\x00\xae\xc1\x33\x47\x71\x14\xbb\x31\x3b\xea\x38\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\xe0\x26\x69\xdd\xff\x7f\x3a\xaf\x66\xf7\xf7\x4f"
      "\xcb\x64\x21\x92\xf6\xfd\xff\xa7\xf2\x75\x91\xd7\xc7\xca\xe4\x90\xdb\x1f"
      "\x5c\x51\x1c\x00\x00\x00\x00\x00\x00\x00\x70\x9d\xbe\x7a\x14\x47\xb1\x1b"
      "\xb3\xed\x76\x33\xc9\xfe\xcf\xff\xe5\xac\x31\x9f\x3d\x7e\x21\xde\x89\x9d"
      "\xa8\xc6\x76\xdc\x8b\xdd\x58\x8f\x46\x34\x62\x3b\xca\x11\x31\xd7\x35\xd1"
      "\xd4\xee\x7a\xa3\xb1\x5d\x3e\xc7\xc8\xe5\xbe\x23\x97\x4f\x85\x96\x9c\x68"
      "\x4d\xe7\x65\xe9\xf2\x8e\x1d\x00\x00\x00\x00\x00\x00\x00\x3e\x47\x7e\x13"
      "\x6b\xc7\xff\xff\x0f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xe3\x20"
      "\x89\x98\x68\x15\xd9\x32\xdf\xae\xcf\x45\xa1\x18\x11\xb7\x22\x62\x2a\xdd"
      "\x6e\x2f\xe2\x5f\xed\xfa\x4d\x76\x30\xea\x00\x00\x00\x00\xe0\x1a\x3c\x73"
      "\x14\x47\xb1\x1b\xb3\xed\x76\x33\xc9\xbe\xf3\xbf\x90\x7d\xef\xbf\x15\xef"
      "\xc4\x56\x34\xa2\x16\x8d\xa8\x47\x35\x1e\x65\xbf\x05\xb4\xbe\xf5\x17\x0e"
      "\xf7\x2b\xf5\xc3\xfd\xca\x66\xba\x9c\x9e\xf7\x7b\xff\x19\x2a\x8c\x6c\xc6"
      "\x68\xfd\xf6\xd0\x7f\xcf\x77\xb2\x2d\x4a\xf1\x38\x6a\xd9\x9a\x7b\xf1\x30"
      "\xde\x8a\x7a\x3c\x8a\x42\x36\x32\x75\xa7\x1d\x4f\xff\xb8\x7e\x9d\xc6\x94"
      "\x7c\x37\x77\x9e\xb0\x92\x93\xd5\x3f\x9d\x5c\x75\xd9\x86\xfc\x31\x65\x2e"
      "\xcb\xc8\x64\x27\x23\x4b\x79\x6c\x69\x36\x9e\x3d\x3b\x13\x43\x3e\x3b\xbd"
      "\x7b\x2a\x47\xa1\x13\xec\x7c\xcf\x9e\x7a\x0e\x62\xc8\x9c\xb7\x46\xcf\xe4"
      "\xad\xf4\x78\xfe\x70\xb5\x39\x1f\xa4\x99\xeb\x59\xdd\x9b\x89\xe5\xae\x57"
      "\xdf\x0b\x67\xe7\x3c\xe2\x6b\x7f\xfb\xcb\x4f\x37\xea\x5b\x4f\x36\x1e\xef"
      "\x2c\x8e\xe0\x90\x2e\x64\x22\x2f\x3b\x09\xe9\xcd\x44\xa5\x2b\x13\x2f\x7e"
      "\x9e\x33\x71\xca\x52\x96\x89\xdb\x9d\xf6\x5a\xfc\x30\x7e\x1c\x8b\xb1\x10"
      "\xaf\xc7\x76\xd4\xe2\xe7\xb1\x1e\x8d\xa8\xc6\x42\xfc\x20\xab\xad\xe7\xaf"
      "\xe7\xa4\xeb\x94\x1f\x90\xa9\x07\x27\x5a\xaf\x7f\x56\x24\x53\xf9\xf3\xd2"
      "\x7a\xb2\x86\x8b\xe9\xe5\x6c\xec\x6c\xd4\xe2\x47\xf1\x56\x3c\x8a\x6a\xbc"
      "\x9a\xfd\x5b\x8e\x72\x7c\x2b\x56\x62\x25\x56\xbb\x9e\xe1\xdb\xe7\x78\xa7"
      "\x2d\x0c\x38\xeb\x9b\x5f\xea\x1b\xfc\xdd\xaf\xe7\x95\x52\x44\xfc\x31\x2f"
      "\xc7\x43\x9a\xd7\x67\xbb\xf2\xda\xfd\x9e\x3b\x97\xf5\x75\xaf\x39\xce\xd2"
      "\x73\x97\x7d\x3d\x8a\x28\x7e\x25\xaf\xa4\xfb\xf8\x6d\x5e\x8e\x87\xde\x4c"
      "\x94\xbb\x32\xf1\xfc\xd9\x99\xf8\x73\xf6\xb6\xb2\x53\xdf\x7a\xb2\xbd\xb1"
      "\xfe\xf6\x39\xf7\xf7\x4a\x5e\xa6\xe7\xd1\xef\x47\x73\x95\x18\x20\x7d\xbd"
      "\x3c\x97\x3e\x59\x59\xeb\xe4\xab\x23\xed\x7b\xbe\x6f\x5f\x39\xeb\x9b\xef"
      "\xf4\x15\x4e\xf5\xdd\xee\xf4\xb5\xce\xd4\xbd\x81\x67\xea\x54\xfe\x19\xee"
      "\xf4\x4c\xcb\x59\xdf\x8b\x7d\xfb\x2a\x59\xdf\x9d\xae\xbe\xf4\xf3\x56\xb9"
      "\xe7\xf3\x16\x00\x63\x6f\xe6\x1b\x33\x53\xa5\x7f\x97\xfe\x59\xfa\xa0\xf4"
      "\xbb\xd2\x46\xe9\xb5\x5b\xdf\x9f\xfe\xf6\xf4\x4b\x53\x31\xf9\xf7\xc9\xef"
      "\x14\x97\x26\x5e\x29\xbc\x94\xfc\x35\x3e\x88\x5f\x1e\x7f\xff\x07\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x9e\xde\xce\xbb\xef\x3d\x59\xaf\xd7\xab\xdb\x3d\x95\x66\xb3"
      "\xf9\xab\x01\x5d\x37\xb9\xd2\xbe\x9d\xd9\xb8\xc4\x73\xce\xca\xa7\xe3\x11"
      "\xc6\x05\x2a\x9f\x36\x9b\xcd\x7c\x4d\xf2\xb4\xf3\x24\x71\x5d\x31\xb7\xef"
      "\x02\x38\xd4\xa8\x24\xc6\x20\xcf\x97\x57\x19\xf1\x1b\x13\x70\xe5\xee\x37"
      "\x36\xdf\xbe\xbf\xf3\xee\x7b\xdf\xac\x6d\xae\xbf\x59\x7d\xb3\xba\xb5\xba"
      "\xb2\xb2\xba\xb4\xba\xf2\x6a\xe5\xfe\xe3\x5a\xbd\xba\xd4\x7a\x1c\x75\x94"
      "\xc0\x55\x38\xbe\xe8\x8f\x3a\x12\x00\x00\x00\x00\x00\x00\x00\x00\x00\xe0"
      "\xbc\x86\xf8\xc3\x80\x74\xf3\xa7\xfa\x73\x82\x51\x1f\x23\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x70"
      "\xb3\xad\x2d\x46\xf1\x20\x92\x28\x2f\xdd\x5b\x4a\xdb\x87\xfb\x95\x7a\xba"
      "\xb4\xeb\xc7\x5b\x16\x23\xa2\x10\x11\xc9\x2f\x22\x92\x0f\x23\x1e\x44\x6b"
      "\x89\xb9\xae\xe9\x92\x41\xfb\x79\xbf\xb6\xfa\xc6\x47\x9f\x1c\x7e\x7c\x3c"
      "\x57\xb1\xbd\x7d\xe1\x78\xdc\x4f\x06\x8c\x6e\x7e\xc6\x51\xec\xe5\x4b\x2c"
      "\x44\xc4\x44\x5e\x5e\xc0\x89\xf9\x1e\x5e\x78\xbe\xa4\x73\x84\x69\xc2\xee"
      "\xb6\x13\x07\xa3\xf6\xbf\x00\x00\x00\xff\xff\x2e\x86\x18\x68",
      1617);
  syz_mount_image(/*fs=*/0x20000600, /*dir=*/0x20000100, /*flags=*/0x1010814,
                  /*opts=*/0x20000180, /*chdir=*/0xfd, /*size=*/0x651,
                  /*img=*/0x20001500);
  memcpy((void*)0x20000000, "./file1\000", 8);
  syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0x20000000ul,
          /*flags=*/0x14b340ul, /*mode=*/0ul);
  memcpy((void*)0x20000040, "./file1\000", 8);
  memcpy((void*)0x20000080, "trusted.overlay.opaque\000", 23);
  syscall(__NR_lsetxattr, /*path=*/0x20000040ul, /*name=*/0x20000080ul,
          /*val=*/0ul, /*size=*/0ul, /*flags=*/0ul);
  return 0;
}
