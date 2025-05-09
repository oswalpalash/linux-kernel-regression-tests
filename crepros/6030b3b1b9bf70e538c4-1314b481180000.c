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

  memcpy((void*)0x20000080, "hfsplus\000", 8);
  memcpy((void*)0x20000200, "./file0\000", 8);
  memcpy(
      (void*)0x20000ec0,
      "\x78\x9c\xec\xdd\xcf\x6f\x23\x57\x1d\x00\xf0\xef\x38\xce\x0f\x07\x48\xd3"
      "\x36\xdb\x2e\xa8\x52\xa3\xae\x54\x10\x11\xbb\x49\xac\x14\xc2\xa5\x01\x21"
      "\x94\x43\x85\xaa\x72\xe0\x1c\x6d\xbc\x8d\xb5\x4e\x5a\x12\x17\xa5\x15\x82"
      "\xf0\xfb\xca\xa1\x7f\x40\x39\xe4\xc6\x09\x89\x7b\xa4\x72\xe1\x02\xb7\x5e"
      "\x73\x5c\x09\x89\xcb\x5e\x08\x27\xa3\x19\x8f\x1d\x27\xb1\x9d\xec\x6e\x1c"
      "\x27\xec\xe7\xb3\x7a\x79\x6f\xe6\xcd\xbc\xf9\xce\x77\x66\x3c\x1e\x5b\x2b"
      "\x07\xf0\xdc\x5a\x9d\x8b\xe2\x41\x24\xb1\x3a\xf7\xce\x6e\x3a\x7d\xb8\x5f"
      "\xae\x1d\xee\x97\x37\x5b\xed\x88\x18\x8f\x88\x42\x44\xb1\x59\x45\xf2\x9f"
      "\x46\xa3\xf1\x79\xc4\x4a\x34\x4b\x7c\x35\x9d\x99\x0f\x97\xf4\xda\xce\xa7"
      "\xd5\xe5\xf7\xbe\x78\x7c\xf8\xa8\x39\x55\xcc\x4b\xb6\x7c\xa1\xdf\x7a\x17"
      "\xb3\x97\x97\x98\x8d\x88\x91\xbc\xbe\xac\xf1\xee\x3f\xf3\x78\x49\x7b\x0f"
      "\xd3\x84\xdd\x69\x25\x0e\x86\x6d\x34\x22\x1a\x27\xfc\xf4\xef\x5f\x6e\xf7"
      "\x74\x28\x75\x5b\x7b\xe2\x4a\x62\x04\x06\xeb\xed\xe6\x7d\xf3\x8c\xe9\x88"
      "\xc9\xfc\x42\x4f\xdf\x07\x34\xef\x8a\xcd\x7b\xf6\x8d\xb6\x37\xec\x00\x00"
      "\x00\x00\xe0\x0a\xbc\x70\x14\x47\xb1\x1b\x53\xc3\x8e\x03\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x6e\x92\xa4\xf9\xfb\xff\x49\x5e\x0a\xad\xf6\x6c\x24"
      "\xad\xdf\xff\x1f\xcb\xe7\x45\xde\xbe\x5e\x5e\x7f\xb2\xc5\x0f\x06\x15\x07"
      "\x00\x00\x00\x00\x00\x00\x00\x5c\xa1\xd7\x8f\xe2\x28\x76\x63\xaa\x35\xdd"
      "\x48\xda\xdf\xf9\x47\xcc\x64\x7f\xbf\x14\x1f\xc5\x4e\x54\x62\x3b\xee\xc6"
      "\x6e\xac\x45\x3d\xea\xb1\x1d\x0b\x11\x31\xdd\x31\xd0\xd8\xee\x5a\xbd\xbe"
      "\xbd\x10\x6f\x9c\xbb\xe6\x62\xd7\x35\x17\xcf\x09\x74\x3c\xaf\x4b\x97\xb3"
      "\xdf\x00\x00\x00\x00\x00\x00\x00\xf0\x7f\xe6\xd7\xb1\x7a\xfc\xfd\x3f\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x5c\x07\x49\xc4\x48\xb3\xca\xca"
      "\x4c\xab\x3d\x1d\x85\x62\x44\x4c\xe4\x8b\xad\x44\xc4\x3f\x23\x62\x6c\xc8"
      "\xe1\xf6\xb7\x7d\xfe\x22\x07\x57\x11\x07\x00\x00\x00\x0c\xd9\x0b\x47\x71"
      "\x14\xbb\x31\xd5\x9a\x6e\x24\xd9\x33\xff\x2b\xd9\x73\xff\x44\x7c\x14\x5b"
      "\x51\x8f\x6a\xd4\xa3\x16\x95\x58\xcf\x3e\x0b\x68\x3e\xf5\x17\x0e\xf7\xcb"
      "\xb5\xc3\xfd\xf2\x66\x5a\xce\x8e\xfb\xbd\x7f\x77\xdd\x5c\xa9\x47\x18\xd9"
      "\x88\xd1\xfc\xec\xa1\xfb\x96\x6f\x67\x4b\x94\xe2\x41\x54\xb3\x39\x77\xe3"
      "\x7e\x7c\x10\xb5\x58\x8f\x42\xb6\x66\xea\x76\x2b\x9e\xee\x71\xfd\x2a\x8d"
      "\x29\x79\x3b\x77\xc1\x04\xad\xe7\x75\xba\xe7\x7f\xcc\xeb\x01\x79\xc2\x0f"
      "\x53\xa6\xb3\x8c\x8c\xb6\x33\x32\x9f\xc7\x96\x66\xe3\xc5\xfe\x99\xe8\x71"
      "\x74\x4e\x5a\xe9\xb9\xa5\x85\x28\xb4\x83\x9d\x39\xb5\xa5\x53\x3b\xf1\x54"
      "\x39\x9f\xcc\xeb\x74\x7f\x7e\x7f\x22\xe7\x23\x17\x1c\x61\x50\x4e\x67\x62"
      "\xb1\xe3\xec\x7b\xa5\x4b\xce\x4f\x64\xe3\xeb\x7f\xfd\xf3\x4f\x36\x6a\x5b"
      "\x0f\x37\x1e\xec\xcc\x0d\xf0\x34\x1a\x88\x56\xe2\x1b\xad\x19\xa7\x33\x51"
      "\xee\xc8\xc4\xab\xfd\xcf\xbe\x9b\x9d\x89\x33\xe6\xb3\x4c\xdc\x6a\x4f\xaf"
      "\xc6\x0f\xe3\xc7\x31\x17\xb3\xf1\x6e\x6c\x47\x35\x7e\x16\x6b\x51\x8f\x4a"
      "\xcc\xc6\x0f\xb2\xd6\x5a\x7e\x3e\x27\x1d\xe7\x47\x8f\x4c\xad\x9c\x98\x7a"
      "\xf7\xbc\x48\xc6\xf2\xe3\xd2\x3c\x58\x4f\x16\xd3\x1b\xd9\xba\x53\x51\x8d"
      "\x1f\xc5\x07\xb1\x1e\x95\x78\x2b\xfb\xb7\x18\x0b\xf1\xed\x58\x8a\xa5\x58"
      "\xee\x38\xc2\xb7\x2e\xf0\x4a\x5b\xe8\x71\xd5\x37\xbe\xd2\x35\xf8\x3b\xdf"
      "\xc8\x1b\xe9\x3d\xe2\x0f\x7d\xee\x15\x57\x2f\xcd\xeb\x8b\x1d\x79\xed\x7c"
      "\xcd\x9d\xce\xfa\x3a\xe7\x1c\x67\xe9\xa5\xcb\xbf\x1f\x15\xbf\x96\x37\xd2"
      "\x6d\xfc\x26\xaf\xaf\x87\xd3\x99\x58\xe8\xc8\xc4\xcb\xfd\x33\xf1\xa7\xec"
      "\x65\x65\xa7\xb6\xf5\x70\x7b\x63\xed\xc3\x0b\x6e\xef\xcd\xbc\x4e\xaf\xa3"
      "\xdf\x3d\xcd\x9d\x79\x60\x27\x58\x7a\xbe\xbc\x94\x1e\xac\x6c\xea\xe4\xd9"
      "\x91\xf6\xbd\xdc\xb5\x6f\x21\xeb\x9b\x69\xf7\x15\xce\xf4\xdd\x6a\xf7\x35"
      "\xaf\xd4\xbd\x9e\x57\xea\x58\xfe\x1e\xee\xec\x48\x8b\x59\xdf\xab\x5d\xfb"
      "\xca\x59\xdf\xed\x8e\xbe\x6e\xef\xb7\x00\xb8\xf6\x26\xbf\x39\x39\x56\xfa"
      "\x57\xe9\x1f\xa5\xcf\x4a\xbf\x2d\x6d\x94\xde\x99\xf8\xfe\xf8\x77\xc6\x5f"
      "\x1b\x8b\xd1\xbf\x8d\x7e\xb7\x38\x3f\xf2\x66\xe1\xb5\xe4\x2f\xf1\x59\xfc"
      "\xe2\xf8\xf9\x1f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x78\x7a\x3b\x1f\x7f\xf2\x70\xad\x56"
      "\xab\x6c\x9f\x6a\x34\x1a\x8d\x5f\xf6\xe8\xba\x51\x8d\xd1\x7c\x3f\xf3\x39"
      "\xad\x9f\x33\x1b\x7e\x60\xcf\x59\xe3\xbf\x8d\x46\x23\x9f\x93\x5c\x87\x78"
      "\xfa\x37\x1a\xb9\xeb\x12\xcf\x30\x1a\x43\x7c\x51\x02\xae\xc4\xbd\xfa\xe6"
      "\x87\xf7\x76\x3e\xfe\xe4\x5b\xd5\xcd\xb5\xf7\x2b\xef\x57\xb6\x96\x97\x96"
      "\x96\xe7\x97\x97\xde\x2a\xdf\x7b\x50\xad\x55\xe6\x9b\x7f\x87\x1d\x25\x30"
      "\x08\xc7\x37\xfd\x61\x47\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x5c\xd4"
      "\x55\xfc\x77\x82\x61\xef\x23\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x70\xb3\xad\xce\x45\xf1\x20\x92"
      "\x58\x98\xbf\x3b\x9f\x4e\x1f\xee\x97\x6b\x69\x69\xb5\x8f\x97\x2c\x46\x44"
      "\x21\x22\x92\x9f\x47\x24\x9f\x47\xac\x44\xb3\xc4\x74\xc7\x70\x49\xaf\xed"
      "\x7c\x5a\x5d\x7e\xef\x8b\xc7\x87\x8f\x8e\xc7\x2a\xb6\x96\x2f\xf4\x5b\xef"
      "\x62\xf6\xf2\x12\xb3\x11\x31\x92\xd7\x97\x35\xde\xfd\x67\x1e\x2f\x69\xef"
      "\x61\x9a\xb0\x3b\xad\xc4\xc1\xb0\xfd\x2f\x00\x00\xff\xff\x81\xb3\x10"
      "\x24",
      1602);
  syz_mount_image(
      /*fs=*/0x20000080, /*dir=*/0x20000200,
      /*flags=MS_POSIXACL|MS_SYNCHRONOUS|MS_STRICTATIME|MS_RELATIME|MS_NODIRATIME|0x40*/
      0x1210850, /*opts=*/0x200000c0, /*chdir=*/1, /*size=*/0x642,
      /*img=*/0x20000ec0);
  memcpy((void*)0x20000140, "cgroup.controllers\000", 19);
  syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0x20000140ul,
          /*flags=*/0x275aul, /*mode=*/0ul);
  memcpy((void*)0x20000000, "./file1\000", 8);
  syscall(__NR_truncate, /*file=*/0x20000000ul, /*len=*/0ul);
  return 0;
}
