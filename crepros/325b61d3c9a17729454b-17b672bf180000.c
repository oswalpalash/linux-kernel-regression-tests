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

  memcpy((void*)0x20000000, "hfsplus\000", 8);
  memcpy((void*)0x20000c80, "./bus\000", 6);
  memcpy(
      (void*)0x200004c0,
      "\x78\x9c\xec\xdd\x4b\x68\x5c\xd7\x19\x07\xf0\xff\x1d\x8f\x64\x8d\x0b\x8e"
      "\x92\xc8\x89\x5b\x02\x15\x31\xa4\xa5\xa2\xb6\x1e\x28\xad\xba\xa9\x5b\x4a"
      "\xd1\x22\x94\x90\x2e\xba\x16\xb6\x1c\x0b\x8f\x95\x20\x29\x45\x09\xa5\xb8"
      "\x2f\xba\xed\x22\x74\x9d\x2e\xb4\xeb\xaa\xd0\xbd\x21\x5d\xb7\xbb\x6c\xb5"
      "\x0c\x14\xb2\xc9\xa2\x68\xe7\x72\xef\xdc\x91\xc6\x7a\x4c\x24\xcb\xf2\x8c"
      "\xe2\xdf\x4f\x9c\x39\xe7\xcc\xb9\xe7\xdc\x6f\xbe\xb9\x73\xe7\x21\x86\x09"
      "\xf0\xdc\x5a\x9c\x4a\xf3\x61\x8a\x2c\x4e\xbd\xb5\x59\xf6\xb7\xb7\xe6\xda"
      "\xdb\x5b\x73\x17\xeb\xe1\x76\x92\xb2\xdd\x48\x9a\x9d\x2a\xc5\x6a\x52\x7c"
      "\x9a\xdc\x4c\xa7\xe4\x9b\xe5\x95\xf5\xf6\xc5\x51\xfb\xf9\x78\x65\xe1\x9d"
      "\xcf\xbe\xdc\xfe\xbc\xd3\x6b\xd6\xa5\xda\xbe\x91\xb1\xa3\xe7\x1d\xcf\x83"
      "\xba\x64\x32\xc9\x85\xba\x3e\x68\xe4\x89\xd6\xbb\x75\xe4\x7a\xc7\x55\xec"
      "\x66\xa6\x4c\xd8\xb5\x6e\xe2\x60\xd0\x1e\x1d\xf0\xe0\x24\xd3\x4f\xf9\xb8"
      "\x05\x86\x41\xd1\x79\xde\x3c\x60\x3c\xb9\x94\x64\xac\x7e\x1d\x90\xfa\xec"
      "\xd0\x78\xb6\xd1\x3d\x7d\x27\x3a\xcb\x01\x00\x00\xc0\x39\xf5\xc2\x4e\x76"
      "\xb2\x99\xcb\x83\x8e\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xce\x93\xfa"
      "\xf7\xff\x8b\xba\x34\x3a\x75\xb3\x98\x4c\xd1\xfd\xfd\xff\xd1\x7a\x2c\x75"
      "\xfb\x5c\x7b\x38\xe8\x00\x00\x00\x00\x00\x00\x00\x00\xe0\x29\xf8\xf6\x4e"
      "\x76\xb2\x99\xcb\xdd\xfe\xa3\xa2\xfa\x9f\xff\xeb\x55\x67\xa2\xba\xfc\x46"
      "\x3e\xc8\x7a\x96\xb3\x96\xeb\xd9\xcc\x52\x36\xb2\x91\xb5\xcc\x24\x19\xef"
      "\x59\x68\x74\x73\x69\x63\x63\x6d\xe6\x18\x33\x67\x0f\x9d\x39\xfb\x6c\x6e"
      "\x2f\x00\x00\x00\x00\x00\x00\x00\x7c\x4d\xfd\x3e\x8b\x7b\xff\xff\x07\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x61\x50\x24\x17\x3a\x55\x55\x26"
      "\xba\xed\xf1\x34\x9a\x49\xc6\x92\x8c\x96\xdb\x3d\x48\xfe\xd3\x6d\x9f\x67"
      "\x0f\x07\x1d\x00\x00\x00\x00\x3c\x03\x2f\xec\x64\x27\x9b\xb9\xdc\xed\x3f"
      "\x2a\xaa\xf7\xfc\xaf\x54\xef\xfb\xc7\xf2\x41\x56\xb3\x91\x95\x6c\xa4\x9d"
      "\xe5\xdc\xae\x3e\x0b\xe8\xbc\xeb\x6f\x6c\x6f\xcd\xb5\xb7\xb7\xe6\xee\x97"
      "\xe5\xe0\xba\x7f\xfd\xdf\xa3\xca\x31\xc3\xa8\x56\x4c\xe7\xb3\x87\xc3\xf7"
      "\x7c\xb5\xda\xa2\x95\x3b\x59\xa9\xae\xb9\x9e\x5b\x79\x2f\xed\xdc\x4e\xa3"
      "\x9a\x59\xba\xda\x8d\xe7\xf0\xb8\x7e\xf7\x45\xb9\xf6\x8f\x6b\xc7\x8c\xec"
      "\x76\x5d\x97\xb7\xfc\x2f\x75\x3d\x1c\xc6\xab\x8c\x8c\xec\x66\x64\xba\x8e"
      "\xad\xcc\xc6\x8b\xfd\x33\xf1\x93\x2f\x4e\xb5\xa7\x99\x34\x76\x3f\xf9\x99"
      "\x38\x83\x9c\x5f\xaa\xeb\x6e\xae\x2f\x9e\x28\xda\xb3\xb4\x3f\x13\xb3\x3d"
      "\x47\xdf\x2b\xfd\x33\x91\x7c\xe7\x9f\x7f\xff\xd5\xdd\xf6\xea\xbd\xbb\x77"
      "\xd6\xa7\x86\xe7\x30\x7a\x42\xfb\x33\x31\xd7\x93\x89\x57\x9f\xab\x4c\x4c"
      "\x57\x99\xb8\xb2\xdb\x5f\xcc\xcf\xf3\xcb\x4c\x65\x32\x6f\x67\x2d\x2b\xf9"
      "\x75\x96\xb2\x91\xe5\x4c\xe6\x67\x55\x6b\xa9\x3e\xae\xcb\xcb\xf1\xfe\x99"
      "\xba\xf9\x58\xef\xed\xaf\x8a\x64\xb4\xbe\x5f\x3a\x67\xd1\x93\xc5\xf4\x7a"
      "\x39\xb7\xfb\xb1\x6f\x6e\x67\x39\x6f\x56\x7f\xb3\x99\xc9\x0f\x32\x9f\xf9"
      "\x2c\xf4\xdc\xc3\x57\x8e\xf1\xa8\x6f\x9c\xec\x51\x7f\xed\xbb\x75\xa3\x95"
      "\xe4\xcf\x75\x3d\x1c\xca\xbc\xbe\xd8\x93\xd7\xde\x73\xee\x78\x35\xd6\x7b"
      "\xcd\x5e\x96\x5e\x7a\xfa\xe7\xc6\xe6\xb7\xea\x46\xb9\x8f\x3f\xd4\xf5\x70"
      "\xd8\x9f\x89\x99\x9e\x4c\xbc\xdc\x3f\x13\x7f\xab\x5e\x27\xac\xb7\x57\xef"
      "\xad\xdd\x5d\x7a\xff\x98\xfb\x7b\xa3\xae\xcb\x03\xf6\x4f\x43\xf5\xcc\x5c"
      "\x1e\x2f\x2f\x95\x77\x56\xd5\x7b\xfc\xe8\x28\xc7\x5e\x2e\xc7\x5a\xfb\xc7"
      "\x66\xaa\xb1\x89\xdd\x79\x8d\x03\x63\x57\x76\xc7\xc6\x73\x39\x2b\xf9\x45"
      "\xde\x3b\xe2\x91\x3a\x5a\xbf\x86\x3b\xb8\xd2\x6c\x35\xf6\xea\xa1\x63\x73"
      "\xd5\xd8\xd5\x9e\xb1\xc3\x5e\x6f\x01\x30\xf4\x2e\x7d\xef\xd2\x68\xeb\xbf"
      "\xad\x7f\xb7\x3e\x69\xfd\xb1\x75\xb7\xf5\xd6\xd8\x4f\x2f\xfe\xf0\xe2\x6b"
      "\xa3\x19\xf9\xd7\xc8\x8f\x9a\xd3\x17\xde\x68\xbc\x56\xfc\x23\x9f\xe4\xb7"
      "\x7b\xef\xff\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x80\x27\xb7\xfe\xe1\x47\xf7\x96\xda\xed"
      "\xe5\xb5\x33\x6b\x74\x7f\xce\xa9\xff\xc6\xdd\x5f\xa0\x3a\xfb\x78\x86\xbb"
      "\x51\xe6\x60\x08\xc2\xd0\x78\xee\x1b\x03\x3e\x31\x01\x67\xee\xc6\xc6\xfd"
      "\xf7\x6f\xac\x7f\xf8\xd1\xf7\x57\xee\x2f\xbd\xbb\xfc\xee\xf2\xea\xc8\xfc"
      "\xfc\xc2\xf4\xc2\xfc\x9b\x73\x37\xee\xac\xb4\x97\xa7\x3b\x97\x83\x8e\x12"
      "\x38\x0b\x7b\x4f\xfa\xfb\x47\x46\x07\x13\x10\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\xf0\x95\xfa\x7f\x0d\x60\xa4\xde\xea\x74\x5f\x27\x18\xf0\x4d\x04"
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
      "\x00\x00\x00\xce\xb9\xc5\xa9\x34\x1f\xa6\xc8\xcc\xf4\xf5\xe9\xb2\xbf\xbd"
      "\x35\xd7\x2e\x4b\xb7\xbd\xb7\x65\x33\x49\x23\x49\xf1\x9b\xa4\xf8\x34\xb9"
      "\x99\x4e\xc9\x78\xcf\x72\xc5\x51\xfb\xf9\x78\x65\xe1\x9d\xcf\xbe\xdc\xfe"
      "\x7c\x6f\xad\x66\x77\xfb\x46\xbf\x79\xfd\x4d\xd4\xf5\x83\xba\x64\x32\xc9"
      "\x85\xba\x3e\x85\xc7\xd6\xbb\x75\xea\xf5\x8a\xdd\x5b\x58\x26\xec\x5a\x37"
      "\x71\x30\x68\xff\x0f\x00\x00\xff\xff\xb1\xdc\x06\x57",
      1543);
  syz_mount_image(/*fs=*/0x20000000, /*dir=*/0x20000c80,
                  /*flags=MS_RELATIME|MS_NOEXEC*/ 0x200008, /*opts=*/0x20000180,
                  /*chdir=*/6, /*size=*/0x607, /*img=*/0x200004c0);
  memcpy((void*)0x20000100, "./file0/file0\000", 14);
  syscall(
      __NR_creat, /*file=*/0x20000100ul,
      /*mode=S_IXOTH|S_IRGRP|S_IWUSR|0xbc9dc8fbd81cb400*/ 0xbc9dc8fbd81cb4a1ul);
  memcpy((void*)0x20000480, "./bus\000", 6);
  memcpy((void*)0x20000f00,
         "./"
         "file2aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\000",
         252);
  syscall(__NR_symlink, /*old=*/0x20000480ul, /*new=*/0x20000f00ul);
  memcpy((void*)0x20000e00,
         "./"
         "file2aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\000",
         252);
  syscall(__NR_creat, /*file=*/0x20000e00ul, /*mode=*/0ul);
  memcpy((void*)0x20000300, "./bus\000", 6);
  memcpy((void*)0x20001180, "./file2\000", 8);
  syscall(__NR_rename, /*old=*/0x20000300ul, /*new=*/0x20001180ul);
  return 0;
}
