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

  memcpy((void*)0x20000000, "hfsplus\000", 8);
  memcpy((void*)0x20001540,
         "./"
         "file0aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\000",
         251);
  memcpy(
      (void*)0x20000d00,
      "\x78\x9c\xec\xdd\xcd\x6b\x5c\xd7\x19\x07\xe0\xdf\x1d\x8d\x64\xcb\x05\x47"
      "\x49\xec\x24\x94\x40\x45\x0c\x69\xa9\xa8\xad\x0f\x94\x56\xdd\xd4\x2d\xa5"
      "\x68\x11\x4a\x48\x17\x5d\x0b\x5b\x8e\x85\xc7\x4a\x90\x26\x45\x0e\xa5\xa8"
      "\xdf\xdb\x2e\xf2\x07\xa4\x0b\xed\xba\x2a\x74\x6f\x48\xd7\xed\x2e\x5b\x2d"
      "\x03\x85\x6e\xb2\x52\x57\x2a\xf7\xce\x9d\xd1\x48\x1a\x29\xf2\x47\x3c\xa3"
      "\xf8\x79\xcc\x99\x73\xce\x9c\xb9\xe7\xbc\xf7\x9d\x3b\xf7\xce\x8c\x30\x13"
      "\xe0\xb9\xb5\x3c\x93\xe6\xc3\x14\x59\x9e\x79\x7b\xab\xec\xef\xee\x2c\xb4"
      "\x76\x77\x16\x2e\xd4\xc3\xad\x24\x65\xbb\x91\x34\x3b\x55\x8a\xf5\xa4\xf8"
      "\x34\xb9\x99\x4e\xc9\x78\xdf\x74\xc5\x49\xeb\x7c\xbc\xb6\xf4\xee\x67\x5f"
      "\xec\x7e\xde\xe9\x35\xeb\x52\x3d\xbe\x71\x78\x8a\xc7\xb1\x5d\x97\x4c\x27"
      "\x19\xab\xeb\xe3\xce\xbc\xcc\xa1\xf9\x6e\x9d\x38\xdf\x59\x15\xbd\xcc\x94"
      "\x09\xbb\x56\xd6\xdb\x4f\x34\x21\x3c\x1d\xfb\xc7\x3c\xd2\x91\x79\xe2\xeb"
      "\x1d\x38\x3f\x8a\xce\x75\xf3\x98\xa9\xe4\x52\x92\x8b\xf5\xfb\x80\xd4\x67"
      "\x87\xc6\xb3\x8d\xee\xe9\x73\xfd\x05\x00\x00\xe0\x79\xf0\xc2\x5e\xf6\xb2"
      "\x95\xcb\xc3\x8e\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xce\x93\xfa\xf7"
      "\xff\x8b\xba\x34\xba\xed\xe9\x14\xdd\xdf\xff\x9f\xa8\xef\x4b\xdd\x3e\xd7"
      "\x1e\x0e\x3b\x00\x00\x00\x00\x00\x00\x00\x00\x78\x0a\xbe\xb5\x97\xbd\x6c"
      "\xe5\x72\xb7\xbf\x5f\x54\x7f\xf3\x7f\xa3\xea\x5c\xa9\x6e\xbf\x91\x0f\xb3"
      "\x99\xd5\x6c\xe4\x7a\xb6\xb2\x92\x76\xda\xd9\xc8\x5c\x92\xa9\xbe\x89\x26"
      "\xb6\x56\xda\xed\x8d\xb9\x33\x6c\x39\xdf\xdd\x72\xac\x7f\xcb\xf9\x67\xb7"
      "\xcf\x00\x00\x00\x00\x00\x00\x00\xf0\x35\xf4\xbb\x2c\x1f\xfc\xfd\x1f\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x46\x41\x91\x8c\x75\xaa\xaa\x5c"
      "\xe9\xb6\xa7\xd2\x68\x26\xb9\x98\x64\xa2\x7c\xdc\x76\xf2\xef\x6e\xfb\x3c"
      "\x7b\x38\xec\x00\x00\x00\x00\xe0\x19\x78\x61\x2f\x7b\xd9\xca\xe5\x6e\x7f"
      "\xbf\xa8\x3e\xf3\xbf\x52\x7d\xee\xbf\x98\x0f\xb3\x9e\x76\xd6\xd2\x4e\x2b"
      "\xab\xb9\x5d\x7d\x17\xd0\xf9\xd4\xdf\xd8\xdd\x59\x68\xed\xee\x2c\xdc\x2f"
      "\xcb\xf1\x79\x7f\xfc\xdf\x47\x0a\xa3\x9a\x31\x9d\xef\x1e\x06\xaf\xfc\x5a"
      "\xf5\x88\xc9\xdc\xc9\x5a\x75\xcf\xf5\xdc\xca\xfb\x69\xe5\x76\x1a\xe5\x96"
      "\x9d\xc0\xb2\xdd\x89\x67\x70\x5c\xbf\x2d\x63\x2a\x7e\x54\x3b\x63\x64\xb7"
      "\xeb\xba\x5c\xe0\x2f\xe9\x2d\x34\x02\xa6\xaa\x8c\x8c\xf7\x32\x32\x5b\xc7"
      "\x56\xe6\xf1\xc5\xfe\x2c\x1c\xcf\xc4\x23\x3e\x3b\x47\x57\x9a\x4b\xa3\xf7"
      "\xcd\xcf\x95\xd3\x57\x1a\x90\xf3\x01\x47\xcb\x11\x97\xea\xba\xdc\x9f\x3f"
      "\x8d\x74\xce\xe7\x3b\x47\x5f\xf5\x5d\xd8\x2b\xa7\x67\x22\xf9\xf6\x3f\xfe"
      "\xf6\xcb\xbb\xad\xf5\x7b\x77\xef\x6c\xce\x8c\xce\x2e\x3d\xa6\xa3\x99\x58"
      "\xa8\x33\x51\x7a\xf5\xb9\xca\xc4\x6c\x95\x89\xab\xbd\xfe\x72\x7e\x96\x5f"
      "\x64\x26\xd3\x79\x27\xff\xdb\xef\x68\x67\x35\xd3\xf9\x69\x56\xd2\xce\x4a"
      "\x7d\x3c\x97\xb7\x53\xa7\x67\xea\xe6\xa1\xde\x3b\x5f\x16\xc9\x44\xfd\xbc"
      "\x74\xce\xa2\x27\xc5\xb4\x91\xb5\xfc\xaa\x8a\xa4\x3f\xa6\x37\xaa\x6d\x2f"
      "\x67\x2d\x3f\xcf\xfb\xb9\x9d\xd5\xbc\x55\xfd\x9b\xcf\x5c\xbe\x9f\xc5\x2c"
      "\x66\xa9\xef\x19\xbe\x7a\x86\x57\x7d\xe3\xd1\xce\xb4\xd7\xbe\x53\x37\x26"
      "\x93\xfc\xb9\xae\x47\x43\x99\xd7\x17\xfb\xf2\xda\x7f\xce\x9d\xaa\xc6\xfa"
      "\xef\x39\xc8\xd2\x4b\x27\x67\xa9\x78\xcc\xeb\x51\xf3\x9b\x75\xa3\x5c\xe3"
      "\xf7\x75\x3d\x1a\x8e\x66\x62\xae\x2f\x13\x2f\x9f\x7e\xbc\xfc\x75\xbf\xbc"
      "\xdd\x6c\xad\xdf\xdb\xb8\xbb\xf2\xc1\x19\xd7\x7b\xb3\xae\xcb\x54\xfe\x71"
      "\xa4\xae\x12\xe5\xf1\xf2\x52\xf9\x64\x55\xbd\xc3\x47\x47\x39\xf6\xf2\xc0"
      "\xb1\xb9\x6a\xec\x4a\x6f\xac\x71\x6c\xec\x6a\x6f\xec\xcb\x5e\xa9\x13\xf5"
      "\x7b\xb8\xe3\x33\xcd\x57\x63\xaf\x0e\x1c\x5b\xa8\xc6\x5e\xeb\x1b\x1b\xf4"
      "\x7e\x0b\x80\x91\x77\xe9\xbb\x97\x26\x26\xff\x33\xf9\xaf\xc9\x4f\x26\xff"
      "\x30\x79\x77\xf2\xed\x8b\x3f\xb9\xf0\x83\x0b\xaf\x4f\x64\xfc\x9f\xe3\x3f"
      "\x6c\xce\x8e\xbd\xd9\x78\xbd\xf8\x7b\x3e\xc9\x6f\x0e\x3e\xff\x03\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x8f\x6f\xf3\xc1\xfe\xbd\x95\x56\x6b\x75\x63\xf3\xc1\x47\xc3"
      "\x6c\x14\xf5\x0f\xf9\x0c\x35\x8c\xe6\x90\x93\xd0\x6b\x34\x93\x0c\x3b\x8c"
      "\xfa\x00\xd9\x1e\x7e\x36\x34\x86\xd1\x18\xf2\x89\x09\xf8\xca\xdd\x68\xdf"
      "\xff\xe0\xc6\xe6\x83\x8f\xbe\xb7\x76\x7f\xe5\xbd\xd5\xf7\x56\xd7\xc7\x17"
      "\x17\x97\x66\x97\x16\xdf\x5a\xb8\x71\x67\xad\xb5\x3a\xdb\xb9\x1d\x76\x94"
      "\xc0\x57\xe1\xe0\xa2\x3f\xec\x48\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80"
      "\xb3\x7a\x16\xff\x9d\x60\xd8\xfb\x08\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x9c\x6f\xcb\x33\x69\x3e"
      "\x4c\x91\xb9\xd9\xeb\xb3\x65\x7f\x77\x67\xa1\x55\x96\x6e\xfb\xe0\x91\xcd"
      "\x24\x8d\x24\xc5\xaf\x93\xe2\xd3\xe4\x66\x3a\x25\x53\x7d\xd3\x15\x27\xad"
      "\xf3\xf1\xda\xd2\xbb\x9f\x7d\xb1\xfb\xf9\xc1\x5c\xcd\xee\xe3\x1b\xa7\x6d"
      "\x37\xd0\xf8\xd1\x3b\xb6\xeb\x92\xe9\x24\x63\x75\xfd\x04\x0e\xcd\x77\xeb"
      "\x89\xe7\x2b\x7a\x7b\x58\x26\xec\x5a\x37\x71\x30\x6c\xff\x0f\x00\x00\xff"
      "\xff\x97\x03\x0c\x34",
      1553);
  syz_mount_image(/*fs=*/0x20000000, /*dir=*/0x20001540, /*flags=*/0x8000,
                  /*opts=*/0x20000040, /*chdir=*/1, /*size=*/0x611,
                  /*img=*/0x20000d00);
  memcpy((void*)0x20000080,
         "./"
         "file0aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\000",
         251);
  syscall(__NR_mknodat, /*dirfd=*/0xffffff9c, /*file=*/0x20000080ul,
          /*mode=*/0ul, /*dev=*/0x103);
  memcpy((void*)0x20000380,
         "./"
         "file0aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\000",
         251);
  syscall(__NR_open, /*file=*/0x20000380ul, /*flags=*/0ul, /*mode=*/0ul);
  memcpy((void*)0x20000280,
         "./"
         "file0aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\000",
         251);
  syscall(__NR_unlink, /*path=*/0x20000280ul);
  memcpy((void*)0x20004300, "./file1\000", 8);
  syscall(__NR_creat, /*file=*/0x20004300ul, /*mode=*/0xdaul);
  return 0;
}
