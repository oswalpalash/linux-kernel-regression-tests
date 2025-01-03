// https://syzkaller.appspot.com/bug?id=2798339fdeb4277326c0891205bb118b755a1906
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

uint64_t r[1] = {0xffffffffffffffff};

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
  intptr_t res = 0;
  memcpy((void*)0x20000000, "udf\000", 4);
  memcpy((void*)0x20000080, "./bus\000", 6);
  *(uint8_t*)0x20000040 = 0;
  memcpy(
      (void*)0x20000180,
      "\x78\x9c\xec\xdd\xcd\x6b\x5d\x69\x19\x00\xf0\xe7\x3d\xcd\x4d\x6f\x3a\xed"
      "\xcc\x9d\xb6\xb6\xd6\x8e\x72\x41\xc1\x32\x62\x49\xd3\x4e\xaa\xa6\x38\xd6"
      "\xc9\x04\x84\xe2\x84\x69\xd3\x85\x2b\x63\x93\x76\xc2\xdc\x26\x25\xc9\x48"
      "\x3a\x0c\xda\x85\xe8\xc6\xff\xc1\xd5\x6c\x14\x64\x40\xdd\x08\x2e\x74\xeb"
      "\x42\x77\x22\x03\xae\xc4\xad\x51\x06\x06\x14\x8b\x9c\x93\x73\xbf\x92\x68"
      "\xa2\x37\x37\xc9\x9d\xfc\x7e\xd0\xde\x73\xcf\x79\xee\x7b\xde\x73\xa0\x85"
      "\xe7\xfd\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x20\xe2"
      "\xab\xaf\xde\x1c\xbd\x92\x0e\xba\x16\x00\x00\x00\x40\x3f\x7d\xfd\xf6\xeb"
      "\xa3\x63\xf2\x7f\x00\x00\x00\xf8\x48\xbb\xa3\xff\x1f\x00\x00\x00\x00\x00"
      "\x00\x00\x06\x5d\x8a\x2c\xbe\x1f\x29\xd6\x4e\xad\xa7\x93\xc5\xf7\x0d\xd5"
      "\x5b\x0b\x8b\x6f\xad\xcd\x4c\x4e\x6d\xff\xb3\x91\x54\xfc\xf2\x58\x11\x9f"
      "\xff\xa9\x5e\x19\xbb\x7a\xed\xa5\xf1\xeb\x5f\x68\x7e\xfe\xf7\xdf\xef\xb5"
      "\x0b\xf1\xda\xed\x3b\x37\xeb\xaf\x2c\x3d\x7c\xb4\x3c\xbf\xb2\x32\x3f\x57"
      "\x9f\x59\x5c\xb8\xb7\x34\x37\xbf\xeb\x12\x7a\xfd\xfd\x66\x2f\x16\x2f\xa0"
      "\xfe\xf0\xcd\xb7\xe6\xee\xdf\x5f\xa9\x8f\x5d\xbe\xda\x75\x79\xad\xf6\x97"
      "\xe3\xcf\x9c\xab\x4d\x7c\xf1\xd5\xb3\xb7\x9a\xb1\x33\x93\x53\x53\xb7\x3b"
      "\x62\x86\x2a\xff\xf7\xdd\xb7\x30\xc2\x03\x00\x00\xe0\x68\x1b\x8e\x2c\xae"
      "\x45\x8a\x3b\x17\x7f\x9a\x4e\x45\x44\x16\xbd\xe7\xc2\x3b\xb4\x1d\xf4\xdb"
      "\x48\xd4\xf2\xfc\xbb\x78\x88\x99\xc9\xa9\xe2\x41\x1a\x0b\xb3\x8b\xab\xf9"
      "\xc5\xe9\x66\x22\x5c\xeb\xce\x89\x87\x9b\x39\xf2\x3e\xe4\xe2\x3d\xa9\x45"
      "\x9c\xce\xeb\x3a\x2c\xa3\x07\x00\x00\x60\xf7\x2a\x91\xc5\xa7\x22\xc5\x85"
      "\xa7\xeb\xe9\xd9\x88\x38\xd6\xcc\x83\x3f\x57\x2c\x0c\xb8\x73\x01\xb5\x7d"
      "\xa8\xe4\x36\x86\x22\xe2\x4c\x44\x5c\x8a\x01\xc8\xd9\x01\x00\x00\xe0\x80"
      "\x1d\x8f\x2c\x5e\x8f\x14\xbf\x6a\xd4\xe2\xb9\x32\xaf\x2e\xf2\xff\xaf\x44"
      "\x4c\x1c\x74\xe5\x00\x00\x00\x80\x3d\x31\x14\x59\x5c\x8f\x14\x1f\x4c\xac"
      "\xa7\x5a\x31\x1e\x20\x22\x5e\x9c\x99\x9c\xaa\xdf\xba\x5b\xff\xda\xe2\xfd"
      "\xa5\x8e\xd8\xe9\x54\xf6\xa8\x0f\xfa\xfc\x80\xfd\x64\x6c\x02\x00\x00\x00"
      "\x87\x40\x35\xb2\x38\x55\xf4\xf8\xaf\xa7\xe7\x0f\xba\x32\x00\x00\x00\x40"
      "\x5f\x8c\x44\x16\xff\x88\x14\x9f\x7d\xf9\xdb\xc5\xba\x72\x51\xac\x4b\xff"
      "\xdc\xc4\x97\x4e\xde\x98\xea\x5c\x61\xee\xfc\x0e\xe5\xe4\xb1\x97\x23\xe2"
      "\xe2\x2e\xe7\xe4\x57\xca\xb5\x06\xa7\xd3\x74\x4a\xd9\x96\xd2\x9e\xec\xc9"
      "\xc3\x01\x00\x00\x00\x85\x6a\xca\xe2\xcf\x91\xe2\xc3\x3f\x56\x8b\xef\x97"
      "\xca\xdc\x3c\x0d\x1d\x74\xcd\x00\x00\x00\x80\x3d\x93\xb2\xf8\x5e\xa4\xf8"
      "\xf2\xf4\x7a\x4a\x9b\xf6\xa5\x3f\xd6\xb1\xbf\x7f\xcb\xa0\xcf\xfd\xef\x6f"
      "\xfd\x47\xaa\xaf\x2c\x3d\x7a\xbc\xbc\xf0\xe0\x8d\xd5\x6d\xaf\x9f\xa8\xde"
      "\xfc\xd6\xca\xea\xf2\xec\xbd\xed\x2f\x6f\xec\x5d\xd8\x35\x1c\x62\xa7\x7d"
      "\x0c\x01\x00\x00\x60\x17\x2a\x29\x8b\xbf\x47\x8a\xdf\x36\xde\x6b\xe5\x9d"
      "\xe5\x1e\x00\xe5\x08\x80\x76\xa2\xf9\xee\x8d\x76\x6e\x5a\x4d\x9b\xae\x16"
      "\xed\x06\xcf\x16\xed\x06\xad\x39\x04\xcf\x8c\x8d\x75\x1e\x6f\x9b\xb2\xfe"
      "\x0f\xeb\xe3\xd5\xca\xfb\x1e\xeb\xfd\xb1\x01\x00\x00\xe0\x48\x49\x29\x8b"
      "\xe1\x48\xf1\x99\xdf\x7c\xbc\xdc\xfb\xff\x44\x6c\xe9\x83\x2e\xe3\x7e\x17"
      "\x29\x6e\x2c\xbd\x50\xc6\x65\xc3\x79\x5c\x73\x9a\x40\xad\xf8\xbb\x7a\x7f"
      "\xa1\x31\x3f\x9a\xc7\x4e\x46\x8a\x9f\x37\x9a\xb1\x51\xc4\x1e\x2f\x63\xcf"
      "\xb4\x63\xaf\xe4\xb1\xbf\xce\xcb\x9d\xed\x8e\xad\x96\xb1\x67\xdb\xb1\x63"
      "\x79\xec\xd3\x48\xf1\xc6\xf2\xf6\xb1\x1f\x6b\xc7\x5e\xcd\x63\x97\x23\xc5"
      "\x4f\x7e\x54\x6f\xc6\x9e\xc8\x63\x4f\x96\xb1\xe7\xda\xb1\x97\xef\x2d\x35"
      "\xe6\xfa\xf6\x82\x01\x00\x00\xe0\x10\xa8\xa4\x2c\x7e\x11\x29\x7e\xf8\xcf"
      "\x7a\x6b\xca\x7f\x77\xff\x7f\xbb\xb7\xfd\xdd\x77\xda\xfd\xfd\x5b\x16\xe8"
      "\xfb\x0f\x7d\xfe\xbd\xf6\xff\xd7\x3a\xce\x3d\x29\xdb\x21\x8e\x97\xed\x15"
      "\x43\x3b\xb4\x57\xbc\x16\x29\x2e\x3c\xff\x42\xf3\x79\x8a\xb6\x82\xe6\xb0"
      "\x82\x8d\xbd\x0e\xda\xed\x15\x7f\x8b\x14\xcb\xdf\xe8\x8e\x1d\x2e\x63\x4f"
      "\xb7\x63\xaf\xec\xfa\xc5\x02\x00\x00\xc0\x21\xd2\x1c\xff\xff\xfb\xbb\xbf"
      "\x6c\x0d\xb9\x2f\x73\xe0\xf2\xeb\xf6\xf9\xff\x27\x36\xaf\x0f\xd8\xa7\xfc"
      "\xbf\x73\x4f\xc2\xfc\x9e\x2b\x8f\xdf\x7e\x73\xb6\xd1\x98\x5f\x1e\xa4\x83"
      "\xef\x46\x44\xd7\x99\x74\x48\x2a\xe6\xe0\xe8\x1e\x94\xff\xa8\x9e\x1c\x96"
      "\xfa\xf4\x7a\xd0\xdb\xff\x83\x00\x00\x70\x14\xe4\xf9\xff\xdd\x48\xb1\xf6"
      "\xc1\xfb\xad\xfe\xee\x32\xff\x2f\x87\xca\xb7\xf3\xff\x0f\xbf\xd3\xce\xff"
      "\x27\x36\x17\xd4\xa7\xfc\xff\x74\xc7\xb9\x89\x72\xbd\x81\xca\x50\x44\x75"
      "\xf5\xe1\xa3\xca\xf9\x88\xea\xca\xe3\xb7\x3f\xbf\xf0\x70\xf6\xc1\xfc\x83"
      "\xf9\xc5\xab\xe3\x2f\x8d\x8f\x8e\x8f\x5f\xbf\x36\x56\x19\x6e\x76\xee\xb7"
      "\x8f\x7a\x7e\x57\x00\x00\x00\x30\xa8\xf2\xfc\x7f\x34\x52\xfc\xf5\x07\x3f"
      "\x6e\xcd\xcf\xdf\x4d\xff\xff\x89\xcd\x05\xf5\x29\xff\x3f\xd3\x71\x2e\xbf"
      "\x67\xbb\xd3\x2f\x3f\xf3\xa7\x5e\x1f\x1f\x00\x00\x00\x8e\x84\x3c\xff\xff"
      "\x59\xa4\xf8\xc3\xc5\xf7\x5a\xeb\xe8\x75\xe7\xff\x1d\xeb\xff\xbf\xd3\x9e"
      "\x67\x7f\xe9\xd3\x1b\xa3\x05\x5a\xad\x03\x7d\xca\xff\xcf\x76\x9c\xab\x15"
      "\xf7\x8d\x18\xd9\xa3\x67\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x80\xc3\xa2\x92\xb2\xf8\x57\xa4\x78\xbf\x3a\x94\xca\x05\xff"
      "\x77\xb5\xfe\xdf\xdc\xe6\x82\xfa\x34\xff\xff\x5c\xc7\xb9\xb9\xd8\x9f\xfd"
      "\xff\x7a\x7e\xa9\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x30\xa0\xb2\xc8\x62\x21\x52\x7c\xf2\xfc\x7a\x7a\x39\x3f"
      "\xf1\xcd\x88\x93\x9d\x9f\x00\x00\x00\xc0\xc0\xfb\x77\x00\x00\x00\xff\xff"
      "\x18\x13\x1b\x15",
      1408);
  syz_mount_image(/*fs=*/0x20000000, /*dir=*/0x20000080,
                  /*flags=MS_STRICTATIME*/ 0x1000000, /*opts=*/0x20000040,
                  /*chdir=*/2, /*size=*/0x580, /*img=*/0x20000180);
  memcpy((void*)0x20000380, "./file1\000", 8);
  res = syscall(
      __NR_openat, /*fd=*/0xffffff9c, /*file=*/0x20000380ul,
      /*flags=O_TRUNC|O_SYNC|O_NONBLOCK|O_CREAT|FASYNC|O_RDWR*/ 0x103a42ul,
      /*mode=*/0ul);
  if (res != -1)
    r[0] = res;
  syscall(__NR_ftruncate, /*fd=*/r[0], /*len=*/0x6000000ul);
  memcpy((void*)0x20000040, "memory.events\000", 14);
  syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0x20000040ul,
          /*flags=*/0x275aul, /*mode=*/0ul);
  *(uint64_t*)0x20000600 = 0;
  syscall(__NR_write, /*fd=*/r[0], /*arg=*/0x20000600ul, /*len=*/0x10ul);
  return 0;
}
