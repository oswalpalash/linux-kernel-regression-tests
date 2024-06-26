// https://syzkaller.appspot.com/bug?id=33e08264ba252fcf3ed89b69fd088ac976363535
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>
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

static __thread int clone_ongoing;
static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* ctx)
{
  if (__atomic_load_n(&clone_ongoing, __ATOMIC_RELAXED) != 0) {
    exit(sig);
  }
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  int skip = __atomic_load_n(&skip_segv, __ATOMIC_RELAXED) != 0;
  int valid = addr < prog_start || addr > prog_end;
  if (skip && valid) {
    _longjmp(segv_env, 1);
  }
  exit(sig);
}

static void install_segv_handler(void)
{
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_IGN;
  syscall(SYS_rt_sigaction, 0x20, &sa, NULL, 8);
  syscall(SYS_rt_sigaction, 0x21, &sa, NULL, 8);
  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = segv_handler;
  sa.sa_flags = SA_NODEFER | SA_SIGINFO;
  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGBUS, &sa, NULL);
}

#define NONFAILING(...)                                                        \
  ({                                                                           \
    int ok = 1;                                                                \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    if (_setjmp(segv_env) == 0) {                                              \
      __VA_ARGS__;                                                             \
    } else                                                                     \
      ok = 0;                                                                  \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    ok;                                                                        \
  })

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

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  install_segv_handler();
  intptr_t res = 0;
  NONFAILING(memcpy((void*)0x200000c0, "udf\000", 4));
  NONFAILING(memcpy((void*)0x20000c40, "./bus\000", 6));
  NONFAILING(memcpy(
      (void*)0x20000c80,
      "\x78\x9c\xec\xdd\x5f\x6c\x5d\xf7\x5d\x00\xf0\xef\xef\xc4\x8e\x9d\x14\x6d"
      "\x97\xae\x49\xbb\x51\x4d\xb7\x45\x74\x21\x63\xc1\x76\xd6\xa6\x95\x27\xd1"
      "\x50\x63\x18\xcb\x5a\x53\xc7\x1b\x50\x1e\x7a\x13\x3b\xe1\x12\xe7\xfa\xca"
      "\x76\xba\xb4\x82\xad\x3c\xf5\x01\x24\xcc\x90\x78\x61\x48\x48\x68\xa8\xe2"
      "\x61\x32\x0f\x7b\x80\xa7\x21\x21\xf1\x88\x85\xf6\x86\x90\xc2\x36\x4a\x11"
      "\x42\xba\x0f\xab\xfa\x82\x6a\x74\xce\xfd\x5d\xfb\xda\x71\x1b\xaf\x8e\xe3"
      "\x24\xfd\x7c\xa4\xe4\x7b\xee\xb9\xdf\xdf\xb9\xbf\x3f\xa7\xe7\x9c\x7b\x7e"
      "\xf7\x34\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x44\xfc"
      "\xf2\xaf\x9c\x1d\x19\x4d\x07\x5d\x0b\x00\xe0\x4e\x7a\x7e\xfa\xc5\x91\x31"
      "\xe7\x7f\x00\xf8\x48\x39\xef\xfb\x3f\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\xdc\xed\x52\x14\xf1\x76\xa4\xf8\xda\xa3\x9d\xf4\x72\xf5\xba\x6b"
      "\xf8\x5c\xb3\x75\xed\xfa\xcc\xc4\xe4\xce\xc5\x8e\xa4\xaa\xe4\xa1\x2a\xbf"
      "\xfc\x33\x3c\x3a\x76\xfa\xf3\x4f\x3e\x75\xe6\xe9\x5e\xfc\xe0\xf2\xb7\xdb"
      "\x27\xe3\x85\xe9\xf3\x67\xeb\xcf\x2d\x5c\x6d\x2f\xce\x2d\x2d\xcd\xcd\xd6"
      "\x67\x5a\xcd\x8b\x0b\xb3\x73\xbb\xde\xc2\x5e\xcb\x6f\x77\xb2\xea\x80\xfa"
      "\xd5\x2b\xd7\x66\x2f\x5d\x5a\xaa\x8f\x9d\x3a\xbd\xe5\xed\xeb\xb5\xb7\x86"
      "\x1e\x38\x5e\x1b\x3f\x33\xfa\xd2\xb1\x5e\xee\xcc\xc4\xe4\xe4\x74\x5f\xce"
      "\xc0\xe0\x87\xfe\xf4\x2d\xfe\x65\x65\x73\x5c\xb7\x3b\x1c\x45\x5c\x8a\x14"
      "\x57\xde\x7c\x3b\x35\x22\xa2\x88\x1d\xfa\xe2\x87\xb5\x9f\xa8\x2f\x6e\xb1"
      "\xef\xec\xb7\x23\x55\x23\x4e\x56\x8d\x98\x99\x98\xac\x1a\x32\xdf\x6c\xb4"
      "\x96\xcb\x37\xa7\x7a\x1d\x51\x44\xd4\xfa\x0a\x3d\xdb\xdb\xf7\xf7\x79\x2c"
      "\xf6\xac\x1e\xf1\x7a\x59\xfd\xb2\xc2\x27\xcb\xe6\x4d\xb7\x1b\x8b\x8d\x0b"
      "\xf3\x73\xf5\xa9\xc6\xe2\x72\x73\xb9\xb9\xd0\x9a\x4a\xdd\xda\xa6\x2a\xbd"
      "\x88\xa7\x53\x44\x3b\x22\x3a\x43\x37\x6f\x6e\x30\x8a\xf8\x7a\xa4\x78\xe3"
      "\x5b\x9d\x74\x21\x22\x0e\xf5\xfa\xe1\xb3\xd5\x83\xc1\xb7\xae\x4f\xb1\x0f"
      "\x6d\xdc\x85\x81\x88\xa8\x0d\x46\xac\x15\xf7\xc0\x98\xdd\xc5\x86\xa2\x88"
      "\xef\x44\x8a\x6f\x7e\x63\x24\x2e\xe6\x7e\xad\xba\xed\x89\x88\x2f\x97\xf1"
      "\x78\xc4\xb5\x32\xde\x88\x58\x29\xe3\xa7\x23\x52\xb9\x83\x3c\x14\xf1\xce"
      "\x0e\xfb\x13\xf7\x96\x81\x28\xe2\x8f\x22\xc5\x8f\xc7\x3b\x69\xb6\x37\xf6"
      "\xd5\x71\xe5\xdc\x57\xea\x5f\x6c\x5d\x5a\xe8\xcb\xed\x1d\x57\xf6\x7a\xae"
      "\x3c\xf0\xf3\xc3\x9d\x74\x97\x1f\x9b\x86\xa3\x88\x46\x75\xc4\xef\xa4\x0f"
      "\x7f\xb1\x03\x00\x00\x00\x00\xc0\xdd\xa7\x88\x3f\x8d\x14\x8f\x7d\xff\x44"
      "\x6a\x47\xff\x9c\x62\xb3\x75\xb9\x7e\xbe\x71\x61\xbe\x7b\x57\xb8\x77\xef"
      "\xbf\x9e\x4b\xad\xaf\xaf\xaf\xd7\x52\x37\x8e\xe4\x38\x95\x63\x3b\xc7\x95"
      "\x1c\x57\x73\x5c\xcb\xb1\x93\x63\xad\xc8\xe5\x73\x9c\xca\xb1\x9d\xe3\x4a"
      "\x8e\xab\x45\xfc\xdc\x93\x65\xf9\xfc\xba\x93\x63\xed\x50\x2e\x9f\xe3\x54"
      "\x8e\xed\x1c\x57\x72\x5c\xcd\x71\x2d\xc7\x4e\x8e\xb5\x81\x5c\x3e\xc7\xa9"
      "\x1c\xdb\x39\xae\xe4\xb8\x9a\xe3\x5a\x8e\x9d\x81\x03\x1c\x2e\x00\x00\x00"
      "\x00\x00\x00\x00\xd8\xc1\x91\x28\xe2\xab\x91\xe2\x89\x5f\x7c\xa5\x7a\xae"
      "\x38\xaa\xe7\xd2\x3f\x3e\x7e\xe6\xc5\x53\xbf\xde\xff\xcc\xf8\x23\xb7\xd8"
      "\x4e\x99\x7b\x2a\x22\x56\x8b\xdd\x3d\x93\x7b\x38\x3f\x3a\x3c\x95\xa6\x52"
      "\x3a\xa0\x67\x88\xe9\x3e\xff\xf7\x07\xf9\xf9\xbf\x3f\x3c\xe8\xca\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x07\xaa\x88\x22\x1e\x8f\x14\xaf\x7c\xb7\x93"
      "\x22\x45\x44\x3d\xe2\xe5\xe8\xc6\x1b\x43\x07\x5d\x3b\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\xe0\x76\x18\x4e\x45\xbc\x13\x29\xfe\xfc\x4b\xc3"
      "\xd5\xeb\xb5\x22\xe2\xb7\x23\xe2\xbd\xf5\xf7\xd6\x23\xe2\xc6\x7b\xeb\xb7"
      "\xdb\x41\xb7\x18\x00\x00\x00\x00\x00\x00\x00\xee\x43\xa9\x88\x57\x23\xc5"
      "\xe3\x2f\x76\x52\x2d\x22\xae\xd7\xde\x1a\x7a\xe0\x78\x6d\xfc\xcc\xe8\x4b"
      "\xc7\x0e\xc5\xa1\x48\x65\x4a\x7f\xfe\x0b\xd3\xe7\xcf\xd6\x9f\x5b\xb8\xda"
      "\x5e\x9c\x5b\x5a\x9a\x9b\xad\xcf\xb4\x9a\x17\x17\x66\xe7\x76\xfb\x71\xc3"
      "\xe7\x9a\xad\x6b\xd7\x67\x26\x26\xf7\xa5\x31\xb7\x74\x64\x9f\xeb\x7f\x64"
      "\xf8\xb9\x85\xf6\xab\x8b\xcd\xcb\xbf\xbb\xbc\xe3\xfb\x47\x87\xcf\x5e\x58"
      "\x5a\x5e\x6c\x5c\xdc\xf9\xed\x38\x12\x45\x44\xbd\x7f\xcd\xc9\xaa\xc2\x33"
      "\x13\x93\x55\xa5\xe7\x9b\x8d\x56\x55\x74\x2a\xed\xb6\xc6\x00\x00\x00\x00"
      "\x00\x00\x00\x7c\x14\x0c\xa6\x22\xde\x8b\x14\x6f\xfc\xcd\x9b\x1b\xf3\xce"
      "\x03\xdd\x39\xff\x81\xed\xb9\xdf\xfe\x42\x44\x91\x97\x27\xf2\xfc\xf3\xc6"
      "\x34\x74\xf5\xbb\x81\x8f\x55\xbf\x1b\xe8\x2e\x7f\x7c\xfc\xcc\x6f\x8c\x7d"
      "\xaa\x7f\x79\xc7\x29\xeb\x93\xd5\x84\x7a\x7d\x66\x62\x72\x72\xba\x6f\xf5"
      "\xc0\xe0\xcd\xa9\xc3\xf9\x73\x47\xf6\xd6\x64\xfa\x94\xe3\xbf\x1c\x29\xfe"
      "\xf8\x2f\xeb\xe9\xb1\xbc\x6e\xeb\xf8\x1f\xda\xc8\xfd\xf6\xef\x6f\x8e\xf7"
      "\xeb\xdb\x37\xf4\x3e\x63\xbe\xd7\xf1\xff\xe9\xbe\x75\xe5\x67\xa6\x54\xc4"
      "\xdf\x45\x8a\x9f\xf9\xcd\x47\xe2\xb1\xaa\x9e\x47\xe3\xa6\xdf\x4c\xe4\xbc"
      "\x2f\x45\x8a\xdf\x5a\x7d\x34\xe7\xc5\xe1\x32\xef\xf1\xfc\xfe\x83\xd5\xdf"
      "\xc3\x97\x9a\xf3\x73\x23\x65\xee\xf5\x48\xf1\x8f\xd7\xb6\xe6\x3e\x91\x73"
      "\x3f\xb1\x99\x3b\xba\xeb\x8e\xbd\x47\x94\xe3\xff\x78\xa4\xf8\x9f\xdf\x59"
      "\xdd\xe8\x9b\x3c\xfe\x79\x04\x36\x47\xad\x7f\xfc\x3f\xb5\x7d\xef\xd8\xa7"
      "\xf1\x7f\xb0\x6f\x5d\x2d\x7f\xee\xcf\xde\x9e\xa6\x13\x11\x4b\xaf\xbe\x76"
      "\xa5\x31\x3f\x3f\xb7\x68\xc1\x82\x05\x0b\x1b\x0b\x07\x7d\x64\xe2\x4e\x28"
      "\xcf\xff\x5f\x8d\x14\xff\xf0\x67\xff\xb6\x71\xbd\x93\xcf\xff\x3f\xd5\x7d"
      "\xb5\x79\xfd\xf7\xee\xd7\x37\xcf\xff\xe3\xdb\x37\xb4\x4f\xe7\xff\x4f\xf4"
      "\xad\x1b\xcf\x57\x23\x83\x03\x11\xc3\xcb\x57\xdb\x83\x0f\x47\x0c\x2f\xbd"
      "\xfa\xda\xe7\x9a\x57\x1b\x97\xe7\x2e\xcf\xb5\x4e\x9f\x79\xe6\xa9\xb1\x91"
      "\x67\x46\x47\x4e\x0f\x1e\xee\x5d\xdc\x6d\x2e\xed\xb9\xaf\xee\x47\xe5\xf8"
      "\xff\x5e\xa4\xf8\xce\x0f\xfe\x36\x3e\x93\xd7\x6d\xbd\xfe\xdb\xf9\xfa\xff"
      "\xe8\xf6\x0d\xed\xd3\xf8\x3f\xd4\xb7\xee\xe8\x96\xeb\x95\x3d\x37\x9d\x3c"
      "\xfe\xff\x17\x29\xfe\x79\xe2\x7b\x71\x22\xaf\xfb\xa0\xeb\xff\xde\xf7\xff"
      "\x13\xf9\x22\x7c\xe3\xfa\x7c\x9f\xc6\xff\x58\xdf\xba\xea\x3b\xde\xc7\x22"
      "\x7e\xbe\x6f\xdd\x89\x63\x11\xf7\xdd\x97\x32\x00\x00\x00\xb8\xcd\x52\x2a"
      "\xe2\x7b\x79\x3e\x75\xe4\x16\xf3\xa9\xff\x14\x29\x5e\xfb\xef\x5f\xc8\x79"
      "\xe9\x78\x99\xf7\x6c\x7e\xbf\x56\xfd\x3d\xfc\xfc\x42\xeb\x73\x67\xe7\xe7"
      "\x17\x2e\x36\x96\x1b\x17\xe6\xe7\xea\xd3\xed\xc6\xc5\xb9\xb2\xec\x8f\x22"
      "\x45\xe7\xaf\x1f\xcd\x65\x8b\x6a\x7e\xb5\x37\xdf\xdc\x9d\xe3\xdd\x9c\x8b"
      "\xfd\xd7\x48\xf1\xcc\xaf\xf5\x72\xbb\x73\xb1\xbd\x7b\x53\x0f\x6d\xe6\x8e"
      "\x96\xb9\xa7\x22\xc5\x9f\x3c\xbf\x35\xb7\x77\x1f\xe3\xd8\x66\xee\x58\x99"
      "\xfb\xef\x91\x62\xf4\xa5\x9d\x73\x8f\x6f\xe6\x9e\x2e\x73\xff\x2b\x52\xbc"
      "\xfb\x17\xf5\x5e\xee\xd1\x32\xf7\x0b\x39\xf7\xe1\xcd\xdc\x53\x17\x17\xe6"
      "\x67\xf7\x61\x58\x00\x00\x00\x00\x00\x60\x4f\x06\x53\x11\x4f\x45\x8a\xbf"
      "\x3f\x3d\x90\x7a\xf7\xb7\x77\xf3\xfb\xcf\x9b\x6e\x7a\xef\xd3\xef\xff\x1e"
      "\xee\x5b\x37\x7b\x87\x9e\x57\xd9\x73\xa7\x02\xc0\x5d\xae\x3c\xff\x9f\x28"
      "\xcf\xea\xbf\xf4\x57\x1b\x73\xf9\x5b\xcf\xff\x9b\xff\x1f\x80\xfe\xf3\xff"
      "\x76\xfd\xff\x6e\xc0\xfb\x2d\x7f\x98\xf3\x7f\xed\xf6\x34\x13\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xe0\x23\x27"
      "\x45\x11\x0b\x91\xe2\x6b\x8f\x76\xd2\x8d\xa1\xf2\x75\xd7\xf0\xb9\x66\xeb"
      "\xda\xf5\x99\x89\xc9\x9d\x8b\x1d\x49\x55\xc9\x43\x55\x7e\xf9\x67\x78\x74"
      "\xec\xf4\xe7\x9f\x7c\xea\xcc\xd3\xbd\xf8\xc1\xe5\x6f\xb7\x4f\xc6\x0b\xd3"
      "\xe7\xcf\xd6\x9f\x5b\xb8\xda\x5e\x9c\x5b\x5a\x9a\x9b\xad\xcf\xb4\x9a\x17"
      "\x17\x66\xe7\x76\xbd\x85\xbd\x96\xdf\xee\x64\xd5\x01\xf5\xab\x57\xae\xcd"
      "\x5e\xba\xb4\x54\x1f\x3b\x75\x7a\xcb\xdb\xd7\x6b\x6f\x0d\x3d\x70\xbc\x36"
      "\x7e\x66\xf4\xa5\x63\xbd\xdc\x99\x89\xc9\xc9\xe9\xbe\x9c\x81\xc1\x0f\xfd"
      "\xe9\x37\x49\xef\xb3\xfe\x70\x14\xf1\xbf\x91\xe2\xca\x9b\x6f\xa7\xff\x18"
      "\x8a\x28\x62\xef\x7d\x71\x8b\x7d\x67\xbf\x1d\xa9\x1a\x71\xb2\x6a\xc4\xcc"
      "\xc4\x64\xd5\x90\xf9\x66\xa3\xb5\x5c\xbe\x39\xd5\xeb\x88\x22\xa2\xd6\x57"
      "\xe8\xd9\x5e\x1f\xdd\x81\xb1\xd8\x93\x7a\xc4\xeb\x65\xf5\xcb\x0a\x9f\x2c"
      "\x9b\x37\xdd\x6e\x2c\x36\x2e\xcc\xcf\xd5\xa7\x1a\x8b\xcb\xcd\xe5\xe6\x42"
      "\x6b\x2a\x75\x6b\x9b\xaa\xf4\x22\x9e\x4e\x11\xed\x88\xe8\x0c\xdd\xbc\xb9"
      "\xc1\x28\x62\x30\x52\xbc\xf1\xad\x4e\xfa\xc1\x50\xc4\xa1\x5e\x3f\x7c\xf6"
      "\xf9\xe9\x17\x47\xc6\x6e\x5d\x9f\x62\x1f\xda\xb8\x0b\x03\x11\x51\x1b\x8c"
      "\x58\x2b\xee\x81\x31\xbb\x8b\x0d\x45\x11\x4f\x46\x8a\x6f\x7e\x63\x24\x7e"
      "\x38\xd4\xed\xd7\xaa\xdb\x9e\x88\xf8\x72\x19\x8f\x47\x5c\x2b\xe3\x8d\x88"
      "\x95\x32\x7e\x3a\x22\x95\x3b\xc8\x43\x11\xef\xec\xb0\x3f\x71\x6f\x19\x88"
      "\x22\x1e\x8c\x14\x3f\x1e\xef\xa4\x1f\x0d\xe5\xb1\xaf\x8e\x2b\xe7\xbe\x52"
      "\xff\x62\xeb\xd2\x42\x5f\x6e\xef\xb8\x72\xcf\x9f\x1f\xee\xa4\xbb\xfc\xd8"
      "\x34\x1c\x45\xbc\x5d\x1d\xf1\x3b\xe9\x3f\xfd\xf7\x0c\x00\x00\x00\x00\x70"
      "\x1f\x29\xe2\x91\x48\xf1\xd8\xf7\x4f\xa4\x6a\x7e\x70\x63\x4e\xb1\xd9\xba"
      "\x5c\x3f\xdf\xb8\x30\xdf\xbd\xad\xdf\xbb\xf7\x5f\xcf\xa5\xd6\xd7\xd7\xd7"
      "\x6b\xa9\x1b\x47\x72\x9c\xca\xb1\x9d\xe2\x57\xcb\xb8\x92\x5f\xaf\xe6\xb8"
      "\x96\x63\x27\xc7\x5a\x91\xcb\xe7\x38\x95\x63\x3b\xc7\x95\x1c\x57\x73\x5c"
      "\xcb\xb1\x93\x63\xed\x50\x2e\x9f\xe3\x54\x8e\xed\x1c\x57\x72\x5c\xcd\x71"
      "\x2d\xc7\x4e\x8e\xb5\x81\x5c\x3e\xc7\xa9\x1c\xdb\x39\xae\xe4\xb8\x9a\xe3"
      "\x5a\x8e\x9d\x81\x83\x1c\x2f\x00\x00\x00\x00\x00\x00\x80\x9d\x15\x51\xc4"
      "\x67\x22\xc5\x2b\xdf\xed\xa4\xf5\xa1\xee\x04\xef\xcb\xd1\x8d\x37\x3c\x0f"
      "\x74\xdf\xfb\xff\x00\x00\x00\xff\xff\x7f\x27\x5c\xab",
      3073));
  NONFAILING(syz_mount_image(0x200000c0, 0x20000c40, 0x100000a, 0x20000380, 8,
                             0xc01, 0x20000c80));
  NONFAILING(memcpy((void*)0x20000180, "./bus\000", 6));
  res = syscall(__NR_creat, 0x20000180ul, 0ul);
  if (res != -1)
    r[0] = res;
  NONFAILING(memcpy((void*)0x200000c0, "./bus\000", 6));
  res = syscall(__NR_open, 0x200000c0ul, 0x14da42ul, 0ul);
  if (res != -1)
    r[1] = res;
  NONFAILING(memcpy((void*)0x20000080, "cgroup.controllers\000", 19));
  syscall(__NR_openat, 0xffffff9c, 0x20000080ul, 0x275aul, 0ul);
  NONFAILING(memcpy((void*)0x20006ac0, "cpuacct.stat\000", 13));
  res = syscall(__NR_openat, 0xffffff9c, 0x20006ac0ul, 0x275aul, 0ul);
  if (res != -1)
    r[2] = res;
  syscall(__NR_mmap, 0x20000000ul, 0x400000ul, 0ul, 0x10012ul, r[2], 0ul);
  NONFAILING(memcpy((void*)0x20002140, "paddr\\n\000", 8));
  syscall(__NR_write, r[1], 0x20002140ul, 8ul);
  syscall(__NR_ftruncate, r[0], 0x7fbul);
  return 0;
}
