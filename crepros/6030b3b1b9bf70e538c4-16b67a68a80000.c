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
      (void*)0x20001000,
      "\x66\x6f\x72\x63\x65\x00\x00\x69\x61\xe3\x8c\x80\x00\x00\x00\x00\x00\x00"
      "\x00\x30\x30\x30\x30\x30\x30\x07\x3f\x41\x10\x1b\xb0\x30\x31\x39\x2c\x63"
      "\x72\x65\x61\x36\xd9\xbb\x3d\x25\xf4\xea\x83\x66\xa8\x71\x6c\xc2\x4c\xf9"
      "\x13\x02\x00\x6f\x72\x71\x24\x02\xbd\xc8\x02\x3a\x1c\xeb\x6c\xff\x75\x72"
      "\x4a\xcd\xc6\xbf\x74\x17\x01\x00\x53\x45\xd3\x03\x00\x7f\xb7\x82\x97\x50"
      "\x7f\x05\x54\xd6\x18\x50\xe2\x17\x40\x23\x24\xe1\x1e\xca\xf1\xee\x34\x06"
      "\x44\x60\x8b\xa8\x72\x92\xd3\xe1\x82\x1a\x70\x4e\x46\x58\x4f\xb9\x46\xcf"
      "\x3b\x27\x7b\x74\xf3\xd9\x4d\x7f\x3f\x3b\x27\xb1\xb9\x57\xc9\x28\xa6\x3d"
      "\x77\x86\xe2\x3b\x2d\xcf\x98\xf4\xbb\xb4\x90\x3a\x06\xab\x8c\x62\x7b\x7b"
      "\xf4\xb1\xce\x08\xac\x07\xbc\x4a\xb9\x32\x95\xbe\x12\xb8\x2c\x45\x8f\x84"
      "\xc3\xae\x25\xbc\xf2\xd8\x53\xe9\x8b\x87\x3f\xd8\xae\xba\xbb\x35\x96\x57"
      "\x99\x7a\x19\x39\x66\x7f\x5d\x6b\xeb\x1a\xca\x91\xb0\xae\xb7\x9f\x37\xab"
      "\x02\x81\x0b\xde\x52\xe8\xc3\xd1\xe3\xa0\x66\xb8\xf2\x50\x09\xb6\x91\xec"
      "\xa2\x0d\xa6\xe0\x03\x90\x10\xc8\x37\x91\xd1\xd7\x84\x7f\x51\x16\xb5\x44"
      "\xb9\xf3\x72\x66\x39\x13\xff\xa7\x89\xb2\x10\x26\x0b\xe4\x78\x0c\x06\x1f"
      "\xdf\xed\x19\x1d\xf0\x7d\x52\xd6\x8d\xb9\x29\x63\xef\x8f\xbe\x85\x11\xae"
      "\x0c\xe4\xab\xce\xb6\x51\x6b\x98\x73\x78\x78\x83\xad\x79\x74\x47\x00\xe9"
      "\xeb\xbe\x4a\x0f\x56\x46\x75\xca\x9e\x56\xf4\xcc\xcc\x8d\xe7\x8f\x7c\xa8"
      "\x03\xd7\xc0\xf2\x66\xc3\xb5\xcb\x9b\x12\x0d\xd6\xe9\x26\xc5\xb8\x88\x08"
      "\xcc\x43\x53\x31\xa9\xa7\x5b\xb9\xf9\xca\x0f\x4d\x00\x56\xa1\x14\xda\xeb"
      "\x4c\x5e\x31\x71\xde\x79\xf9\x04\xf8\x5d\x1f\x7d\x0e\x91\x27\xd8\xfd\xd8"
      "\x30\x51\xdb\xb6\xfc\xb2\x5b\x8d\x09\x96\x72\x93\x85\x5e\x13\xc0\xd4\xbb"
      "\x8e\xec\x9b\x04\x8e\x3e\x72\x94\x21\xf5\xd7\xac\x24\x2e\xf8\x1b\x53\x75"
      "\xdb\xe9\x0e\xed\x05\xb5\xb6\x0c\xb8\xc9\xf1\x58\xf1\x8d\xed\xe7\x22\x46"
      "\x55\xe9\x01\x0c\x60\x31\x5b\x96\x45\xa7\x87\x67\xa5\xca\x8e\xbf\x40\x00"
      "\x00\x00\x00\x00\x00\x00\x0a\xe8\xd5\x97\xe9\x5a\x5e\x62\x89\x7e\x9a\x38"
      "\x9d\x91\x43\xe6\xeb\x06\x9e\x03\x7c\x29\xb9\x75\x68\xc5\x90\xc6\x8b\x2d"
      "\x65\x13\xbc\xd7\xfe\x78\x80\xf6\x67\x54\x64\xcb\xaa\x3b\x7d\x66\x0c\xe2"
      "\x00\x00\x00\x00\x11\x12\x8b\x18\xdb\xa2\x8d\x01\x86\x14\x83\x48\xc6\x72"
      "\xb6\xfa\x8a\x57\x92\x21\xc6\x77\x6c\xc0\x15\xb2\x6e\x66\xf6\x15\x51\x06"
      "\x9b\x4b\x16\xdf\xdc\x1f\x6e\x72\x5b\xb4\x5a\xa9\x87\x2c\xf6\xde\x46\x8d"
      "\xf0\x03\x75\xc8\x5a\xb6\xd9\x2e\x67\xb0\xa2\xd2\x87\x78\x17\x95\x18\xc6"
      "\xf1\xec\x9a\x1f\xbb\xa7\xb1\x1c\xbe\x38\x58\x59\xd4\x7f\x92\x1e\x42\xbb"
      "\x9e\xc6\x5c\xdb\x36\xd9\x82\x9c\x00\x31\xa1\x50\xc0\xf0\x5d\x0a\xa8\x29"
      "\xaf\x50\x32\xe0\x0b\x76\xa2\x2e\x38\x5e\x5f\x99\x12\xfa\x11\x39\x4a\x7d"
      "\x34\x01\x00\x00\x00\x00\xd4\x03\x28\x43\x33\x06\x7b\x81\xae\x9c\x93\xcf"
      "\x2f\x99\x22\x09\x56\x5d\x76\xaa\xa1\x44\x7c\x34\x3e\x11\x38\x7d\x69\x19"
      "\x03\xf3\x41\x0a\x3c\x31\xe4\xf1\x3f\xca\xf3\xf0\xb9\x57\xf8\xd7\x2b\xea"
      "\x2c\xb3\x8a\x98\x6e\x3d\x47\xd5\x00\x51\x80\x76\x2e\x4f\xf3\xe7\x3f\x25"
      "\x5e\x9b\xac\xa5\xf6\xb7\x6b\x38\xc6\xc8\xe1\xf1\xde\xaa\x0e\x5a\xfc\x20"
      "\x1d\x59\x6f\x66\xf4\xb2\x38\xc7\x9f\x57\xb6\xe3\xb7\xd1\x63\x41\x1c\x5e"
      "\x5d\xb8\x15\xaa\xe6\xf3\x77\xbe\x1f\xd0\xb8\x65\x48\xa8\x38\x36\x3b\x56"
      "\x1c\x08\xe1\xb0\x83\x98\x26\x6c\x8a\x23\x05\x56\x69\xf3\xc4\x07\x5c\x87"
      "\xe9\xd3\xf9\xfa\xb6\xbf\xbd\x59\xb1\x2a\x2e\x0c\x0d\xe4\x4b\x19\xc1\x0e"
      "\xeb\x9d\x96\x0e\xa1\x4e\x4d\x28\x5c\x61\x0f\xdb\x6c\x58\x98\xc0\x7d\x43"
      "\xfe\x27\xaf\xbe\x7b\x2f\x7d\xb4\x8b\xda\xf2\x0c\xeb\xcc\x4b\x3e\xd5\x6b"
      "\x50\x6c\xcb\xd4\xf9\x7d\xb3\xb4\x7d\xf9\x4f\x1f\x45\x26\x9f\x13\x6e\xf2"
      "\x21\x01\x80\x00\x00\x00\x00\x00\x00\x04\x7e\x46\x3a\x0e\x50\x70\x4f\x27"
      "\xa7\xe1\x60\x77\xf1\x80\x9b\x4d\x9d\x5c\xac\x14\xbf\x0b\x80\xe6\x80\x65"
      "\x14\x05\xb6\x18\x91\x3d\x79\x88\x9f\x42\x7f\x86\x7c\x9b\x13\x78\x7a"
      "\xed",
      900);
  memcpy(
      (void*)0x20000700,
      "\x78\x9c\xec\xdd\xcd\x6b\x1c\xe7\x1d\x07\xf0\xef\xac\x57\xb2\xe5\x82\xa3"
      "\x24\x76\x63\x4a\xa0\x22\x86\xb4\x54\xd4\xd6\x0b\x4a\xab\x5e\xea\x96\x52"
      "\x74\x08\x25\xa4\x87\x9e\x85\xbd\x8e\x17\xaf\x95\x20\x6d\x8a\x1c\x4a\x51"
      "\xdf\xaf\x3d\xe4\x0f\x48\x0f\xba\xf5\x54\xe8\xdd\x90\x9e\x9b\x5b\xae\x3a"
      "\x06\x0a\xbd\xe4\xa4\x9e\x54\x66\x76\x56\x5a\x59\x2f\x91\xdf\xb4\xab\xf8"
      "\xf3\x31\xcf\x3e\xcf\xb3\xcf\xcc\x33\xbf\xf9\xed\xcc\xec\xce\x0a\xb3\x01"
      "\x5e\x58\x4b\xd3\x69\x3e\x4c\x91\xa5\xe9\xb7\xd7\xcb\xfe\xd6\xe6\x7c\x67"
      "\x6b\x73\xfe\x7c\x3d\xdc\x49\x52\xb6\x1b\x49\xb3\x57\xa5\x58\x49\x8a\x4f"
      "\x93\x9b\xe9\x95\x8c\x0d\x4c\x57\x1c\xb5\x9d\x8f\xdb\x8b\xef\x7e\xfe\xe5"
      "\xd6\x17\xbd\x5e\xb3\x2e\xd5\xf2\x8d\xfd\x53\x3c\x89\x8d\xba\x64\x2a\xc9"
      "\xb9\xba\x3e\xe8\xc4\x9b\xd9\x37\xdf\xad\x23\xe7\x3b\xa9\x62\x37\x33\x65"
      "\xc2\xae\x95\xf5\xc6\x53\x4d\x08\xcf\xc6\xce\x01\x8f\x75\x64\x1e\x79\xbe"
      "\x03\x67\x47\xd1\x7b\xdf\x3c\x60\x32\xb9\x98\xe4\x42\xfd\x39\x20\xf5\xd5"
      "\xa1\x71\xba\xd1\x3d\x7b\xde\x7f\x01\x00\x00\x78\x11\xbc\xb4\x9d\xed\xac"
      "\xe7\xd2\xb0\xe3\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\xb3\xa4\xfe\xfd"
      "\xff\xa2\x2e\x8d\x7e\x7b\x2a\x45\xff\xf7\xff\xc7\xeb\xe7\x52\xb7\xcf\xb4"
      "\x87\xc3\x0e\x00\x00\x00\x00\x00\x00\x00\x00\x9e\x81\x6f\x6f\x67\x3b\xeb"
      "\xb9\xd4\xef\xef\x14\xd5\xdf\xfc\xdf\xa8\x3a\x97\xab\xc7\x6f\xe4\xc3\xac"
      "\xa5\x95\xd5\x5c\xcf\x7a\x96\xd3\x4d\x37\xab\x99\x4d\x32\x39\x30\xd1\xf8"
      "\xfa\x72\xb7\xbb\x3a\x7b\x82\x35\xe7\x0e\x5d\x73\xee\x74\xf6\x17\x00\x00"
      "\x00\x00\x00\x00\x00\xbe\xa6\x7e\x9f\xa5\xbd\xbf\xff\x03\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\xc0\x28\x28\x92\x73\xbd\xaa\x2a\x97\xfb\xed\xc9"
      "\x34\x9a\x49\x2e\x24\x19\x2f\x97\xdb\x48\x3e\xeb\xb7\xcf\xb2\x87\xc3\x0e"
      "\x00\x00\x00\x00\x4e\xc1\x4b\xdb\xd9\xce\x7a\x2e\xf5\xfb\x3b\x45\x75\xcf"
      "\xff\xcd\xea\xbe\xff\x42\x3e\xcc\x4a\xba\x69\xa7\x9b\x4e\x5a\xb9\x5d\x7d"
      "\x17\xd0\xbb\xeb\x6f\x6c\x6d\xce\x77\xb6\x36\xe7\xef\x97\xe5\xe0\xbc\x3f"
      "\xf9\xef\x63\x85\x51\xcd\x98\xde\x77\x0f\x87\x6f\xf9\x6a\xb5\xc4\x44\xee"
      "\xa4\x5d\x3d\x73\x3d\xb7\xf2\x7e\x3a\xb9\x9d\x46\xb5\x66\xe9\x6a\x3f\x9e"
      "\xc3\xe3\xfa\x5d\x19\x53\xf1\xe3\xda\x09\x23\xbb\x5d\xd7\xe5\x9e\xff\xb5"
      "\xae\x47\xc3\x64\x95\x91\xb1\xdd\x8c\xcc\xd4\xb1\x95\xd9\x78\xf9\xf8\x4c"
      "\x3c\xe6\xab\xf3\xe8\x96\x66\xd3\xd8\xfd\xe6\xe7\xf2\x73\xc8\xf9\xc5\xba"
      "\x2e\xf7\xe7\xcf\x23\x9d\xf3\xb9\xfa\xe8\xfb\xac\x3c\x67\x8e\xcf\x44\xf2"
      "\x9d\x7f\xfe\xfd\x57\x77\x3b\x2b\xf7\xee\xde\x59\x9b\x1e\x9d\x5d\x7a\x42"
      "\x8f\x66\x62\x7e\xe0\x3c\x7c\xed\x85\xca\xc4\x4c\x95\x89\x2b\xbb\xfd\xa5"
      "\xfc\x3c\xbf\xcc\x74\xa6\xf2\x4e\xfe\xb7\xd3\xd3\x4d\x2b\x53\xf9\x59\x96"
      "\xd3\xcd\x72\x7d\x3c\x97\x8f\x93\xc7\x67\xea\xe6\xbe\xde\x3b\x5f\x15\xc9"
      "\x78\xfd\xba\xf4\xae\xa2\x47\xc5\xb4\x9a\x76\x7e\x5d\x45\x32\x18\xd3\x1b"
      "\xd5\xba\x97\xd2\xce\x2f\xf2\x7e\x6e\xa7\x95\xb7\xaa\x7f\x73\x99\xcd\x0f"
      "\xb2\x90\x85\x2c\x0e\xbc\xc2\x57\x4e\x70\xd6\x37\x1e\xef\xac\xbf\xf6\xdd"
      "\xba\x31\x91\xe4\x2f\x75\x3d\x1a\xca\xbc\xbe\x3c\x90\xd7\xc1\x6b\xee\x64"
      "\x35\x36\xf8\xcc\x5e\x96\x5e\x39\x3a\x4b\xc5\x13\x5e\x1b\x9b\xdf\xaa\x1b"
      "\xe5\x36\xfe\x50\xd7\xa3\xe1\xd1\x4c\xcc\x0e\x64\xe2\xd5\xe3\x8f\x97\xbf"
      "\xed\x94\x8f\x6b\x9d\x95\x7b\xab\x77\x97\x3f\x38\xe1\xf6\xde\xac\xeb\x32"
      "\x95\x7f\x1a\xa9\x77\x89\xf2\x78\x79\xa5\x7c\xb1\xaa\xde\xfe\xa3\xa3\x1c"
      "\x7b\xf5\xd0\xb1\xd9\x6a\xec\xf2\xee\x58\xe3\xc0\xd8\x95\xdd\xb1\xaf\x3a"
      "\x53\xc7\xeb\xcf\x70\x07\x67\x9a\xab\xc6\x5e\x3b\x74\x6c\xbe\x1a\xbb\x3a"
      "\x30\x76\xd8\xe7\x2d\x00\x46\xde\xc5\xef\x5d\x1c\x9f\xf8\xcf\xc4\xbf\x27"
      "\x3e\x99\xf8\xe3\xc4\xdd\x89\xb7\x2f\xfc\xf4\xfc\x0f\xcf\xbf\x3e\x9e\xb1"
      "\x7f\x8d\xfd\xa8\x39\x73\xee\xcd\xc6\xeb\xc5\x3f\xf2\x49\x7e\xbb\x77\xff"
      "\x0f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x3c\xb9\xb5\x07\x3b\xf7\x96\x3b\x9d\xd6\xea\xda"
      "\x83\x8f\x86\xd9\x28\xea\x1f\xf2\x19\x6a\x18\xcd\x21\x27\x61\xa4\x1a\xf5"
      "\x01\xb2\x31\x2a\xf1\x68\x9c\x6e\x63\xc8\x17\x26\xe0\xb9\xbb\xd1\xbd\xff"
      "\xc1\x8d\xb5\x07\x1f\x7d\xbf\x7d\x7f\xf9\xbd\xd6\x7b\xad\x95\xb1\x85\x85"
      "\xc5\x99\xc5\x85\xb7\xe6\x6f\xdc\x69\x77\x5a\x33\xbd\xc7\x61\x47\x09\x3c"
      "\x0f\x7b\x6f\xfa\xc3\x8e\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x38\xa9"
      "\xd3\xf8\xef\x04\xc3\xde\x47\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xe0\x6c\x5b\x9a\x4e\xf3\x61\x8a"
      "\xcc\xce\x5c\x9f\x29\xfb\x5b\x9b\xf3\x9d\xb2\xf4\xdb\x7b\x4b\x36\x93\x34"
      "\x92\x14\xbf\x49\x8a\x4f\x93\x9b\xe9\x95\x4c\x0e\x4c\x57\x1c\xb5\x9d\x8f"
      "\xdb\x8b\xef\x7e\xfe\xe5\xd6\x17\x7b\x73\x35\xfb\xcb\x37\x8e\x5b\xef\x50"
      "\x63\x8f\x3e\xb1\x51\x97\x4c\x25\x39\x57\xd7\x4f\x61\xdf\x7c\xb7\x9e\x7a"
      "\xbe\x62\x77\x0f\xcb\x84\x5d\xeb\x27\x0e\x86\xed\xff\x01\x00\x00\xff\xff"
      "\x16\x46\x0e\x9a",
      1534);
  syz_mount_image(/*fs=*/0x20000000, /*dir=*/0x20001540, /*flags=*/0,
                  /*opts=*/0x20001000, /*chdir=*/1, /*size=*/0x5fe,
                  /*img=*/0x20000700);
  memcpy((void*)0x20000080,
         "./"
         "file0aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\000",
         251);
  syscall(__NR_mknodat, /*dirfd=*/0xffffff9c, /*file=*/0x20000080ul,
          /*mode=*/0ul, /*dev=*/0x103);
  memcpy((void*)0x20000180,
         "./"
         "file0aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\000",
         251);
  syscall(__NR_open, /*file=*/0x20000180ul, /*flags=*/0ul, /*mode=*/0ul);
  memcpy((void*)0x20000280,
         "./"
         "file0aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\000",
         251);
  syscall(__NR_unlink, /*path=*/0x20000280ul);
  memcpy((void*)0x20000200, "./file1\000", 8);
  syscall(__NR_creat, /*file=*/0x20000200ul, /*mode=*/0ul);
  return 0;
}
