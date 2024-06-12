// https://syzkaller.appspot.com/bug?id=82148e8173e51abf3a46e231a7e233950c815ca7
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
#include <stdarg.h>
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
#define __NR_memfd_create 279
#endif
#ifndef __NR_mmap
#define __NR_mmap 222
#endif
#ifndef __NR_unlinkat
#define __NR_unlinkat 35
#endif

static unsigned long long procid;

static bool write_file(const char* file, const char* what, ...)
{
  char buf[1024];
  va_list args;
  va_start(args, what);
  vsnprintf(buf, sizeof(buf), what, args);
  va_end(args);
  buf[sizeof(buf) - 1] = 0;
  int len = strlen(buf);
  int fd = open(file, O_WRONLY | O_CLOEXEC);
  if (fd == -1)
    return false;
  if (write(fd, buf, len) != len) {
    int err = errno;
    close(fd);
    errno = err;
    return false;
  }
  close(fd);
  return true;
}

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

static void setup_sysctl()
{
  char mypid[32];
  snprintf(mypid, sizeof(mypid), "%d", getpid());
  struct {
    const char* name;
    const char* data;
  } files[] = {
      {"/proc/sys/kernel/hung_task_check_interval_secs", "20"},
      {"/proc/sys/net/core/bpf_jit_kallsyms", "1"},
      {"/proc/sys/net/core/bpf_jit_harden", "0"},
      {"/proc/sys/kernel/kptr_restrict", "0"},
      {"/proc/sys/kernel/softlockup_all_cpu_backtrace", "1"},
      {"/proc/sys/fs/mount-max", "100"},
      {"/proc/sys/vm/oom_dump_tasks", "0"},
      {"/proc/sys/debug/exception-trace", "0"},
      {"/proc/sys/kernel/printk", "7 4 1 3"},
      {"/proc/sys/kernel/keys/gc_delay", "1"},
      {"/proc/sys/vm/oom_kill_allocating_task", "1"},
      {"/proc/sys/kernel/ctrl-alt-del", "0"},
      {"/proc/sys/kernel/cad_pid", mypid},
  };
  for (size_t i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
    if (!write_file(files[i].name, files[i].data))
      printf("write to %s failed: %s\n", files[i].name, strerror(errno));
  }
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
  setup_sysctl();
  intptr_t res = 0;
  memcpy((void*)0x20000000, "hfsplus\000", 8);
  memcpy((void*)0x20000080, "./bus\000", 6);
  memcpy(
      (void*)0x20000a00,
      "\x78\x9c\xec\xdd\xc1\x6f\x1c\x57\x1d\x07\xf0\xef\xac\xd7\x8e\x1d\xaa\xe0"
      "\xb4\x49\x1b\xa1\x22\xa2\x44\x2a\x48\x11\x89\x1d\x2b\x05\x73\x21\x20\x84"
      "\x72\xa8\x50\x55\x0e\x9c\xad\xc4\x49\xac\x6c\x92\xca\x71\x21\xa9\x10\xb8"
      "\x80\xe0\x84\xc4\xa1\x7f\x40\x41\xf2\x8d\x13\x52\xef\x41\xe1\x5c\x6e\xbd"
      "\xfa\x82\x54\x09\x89\x4b\xc4\x21\xe2\xb2\x68\x66\x67\x9c\xcd\xee\x3a\xb6"
      "\x13\x3b\x76\xe0\xf3\xa9\xc6\xef\xcd\xbc\x37\x6f\x7e\xfb\x9b\x37\x33\xde"
      "\x6d\xac\x0d\xf0\x7f\xeb\xd2\x99\xb4\xef\xa7\xc8\xa5\x33\xef\xdc\x2d\xd7"
      "\xd7\xd7\xe6\x3a\xeb\x6b\x73\x87\xea\xe6\x4e\x92\xb2\xde\x4a\xda\xbd\x22"
      "\xc5\xad\xa4\x78\x90\x5c\x2c\xdb\x8b\x24\x27\xea\x32\x7d\xe5\x90\x8f\x97"
      "\xe6\xdf\xfb\xfc\xe1\xfa\x17\xbd\xb5\x76\xbd\x54\xfd\x5b\x4f\xdb\x6f\x84"
      "\x11\x7d\x57\x9b\xca\xc9\x24\x63\x75\x39\x6c\x7c\xbb\x87\x58\x6d\xc6\x2c"
      "\xc7\xb9\x9c\xe4\x95\xa1\x2e\x13\xdb\x1d\xeb\x89\x8e\x65\xd2\x4e\xd7\x25"
      "\xec\xbb\xee\x90\xd5\x9d\xec\xbe\x93\xeb\x16\x38\x60\x9a\xa7\x53\xd1\x7b"
      "\x6e\x0e\x99\x4e\x0e\x27\x99\xac\x7f\x0f\x68\x9e\xb4\xad\x17\x17\xe1\xde"
      "\xd8\xd1\x5d\x0e\x00\x00\x00\x5e\x52\x9f\xdd\xde\xef\x08\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\xe0\xe5\x53\x7f\xff\x7f\x91\xa4\xdb\xed\x76\x5b\x75"
      "\x3d\x27\x53\x34\xdf\xff\x3f\xd1\x6c\xab\xeb\x2f\xb5\xfb\xfb\x1d\x00\x00"
      "\x00\x00\x00\x00\x00\x00\xec\x82\xaf\x3d\xca\xa3\xdc\xcd\x91\x66\xbd\x5b"
      "\xa4\x95\xe4\x54\xb5\x72\x2c\xff\xe9\x26\x5f\xca\x07\xb9\x93\xc5\x2c\xe7"
      "\x6c\xee\x66\x21\x2b\x59\xc9\x72\x66\x93\x4c\xf7\x0d\x34\x71\x77\x61\x65"
      "\x65\x79\x76\x63\xcf\xd2\xe8\x3d\xcf\x67\x6a\xc4\x9e\xe7\x5f\xd4\x2b\x06"
      "\x00\x00\x00\x00\x00\x00\x80\xff\x25\xff\x68\x2a\xbf\xca\xa5\xc7\xff\xff"
      "\x1f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0e\x82\x22\x19\xeb\x15"
      "\xd5\x72\xac\xa9\x4f\xa7\xd5\x4e\x72\xb1\x5e\xcf\x6a\xf2\xf7\x24\x13\xfb"
      "\x1d\xef\x0e\x14\xa3\x36\xde\x7f\xf1\x71\x00\x00\x00\xc0\x73\x99\x7c\x86"
      "\x7d\xbe\xfc\x28\x8f\x72\x37\x47\x9a\xf5\x6e\x51\xbd\xe7\x7f\xbd\x7a\xbf"
      "\x3c\x99\x0f\x72\x2b\x2b\x59\xca\x4a\x3a\x59\xcc\x95\xfa\x3d\x74\xf9\xae"
      "\xbf\xb5\xbe\x36\xd7\x59\x5f\x9b\xbb\x59\x2e\xc3\xe3\x7e\xef\x5f\x3b\x0a"
      "\xa3\x1a\x31\xbd\xcf\x1e\x46\x1f\xf9\x44\xd5\x63\x2a\x57\xb3\x54\x6d\x39"
      "\x9b\xcb\x55\x30\x57\xd2\xaa\xf6\x2c\x9d\x68\xe2\x19\x1d\xd7\x47\x65\x4c"
      "\xc5\x77\x6b\xdb\x8c\xac\x5d\xa7\xb5\x3c\xd8\x1f\x36\xfb\x14\x61\x57\xec"
      "\xf4\xa3\x88\xe9\x32\xb8\x64\x23\x23\x33\x75\x6c\x65\x36\x8e\xf6\x32\x50"
      "\x54\x1f\xd4\x24\x83\x99\xd8\xf2\xec\xb4\x07\x8f\x94\x56\xc6\x37\x8e\x34"
      "\x9b\xd6\xc6\x27\x3f\xc7\xf6\x20\xe7\x87\xeb\xb2\x7c\x3d\xbf\xdd\xd3\x9c"
      "\xef\xd4\x46\x26\x5a\xa9\x32\x71\xbe\x6f\xf6\xbd\xfe\xf4\x4c\x24\x5f\xff"
      "\xf4\xcf\x3f\xb9\xde\xb9\x75\xe3\xfa\xd5\x3b\x67\x0e\xce\x4b\xda\xc2\xd8"
      "\x26\xdb\x07\xe7\xc4\x5c\x5f\x26\xde\x78\xa9\x33\xd1\xde\x61\xff\x99\x2a"
      "\x13\xc7\x37\xd6\x2f\xe5\x87\xf9\x71\xce\xe4\x64\xde\xcd\x72\x96\xf2\xd3"
      "\x2c\x64\x25\x8b\xe9\xd6\xed\x0b\xf5\x7c\x2e\x7f\x4e\x3f\x3d\x53\x17\x9f"
      "\x58\x7b\x77\xab\x48\x26\xea\xf3\xd2\x3b\x67\xdb\x89\xe9\x64\x7e\x50\xd5"
      "\x16\x72\xaa\xda\xf7\x48\x96\x52\xe4\x76\xae\x64\x31\x6f\x57\xff\x9d\xcf"
      "\x6c\xbe\x95\x0b\xb9\x90\xf9\xbe\x33\x7c\x7c\xd3\xb8\xab\xd7\x56\x5d\xf5"
      "\xad\xc1\xab\xbe\x39\xd3\x7f\x1d\x19\xfc\xe9\x6f\xd4\x95\xa9\x24\xbf\xab"
      "\xcb\xa1\x1c\x34\xc6\x7b\xc5\x66\xb3\x73\xb7\xf4\xee\xfd\x65\x5e\x8f\xf6"
      "\xe5\xb5\x37\xeb\x1f\x6e\xf4\x3a\xda\x77\x1d\xcc\xf4\x65\xe9\xd5\x26\x3b"
      "\xe3\x23\x07\x7f\x96\x7b\x63\xfb\x2b\x75\xa5\x3c\xc6\xaf\xeb\xf2\x60\x98"
      "\xae\x33\x51\x5e\x40\xcd\x53\xa2\x89\xee\xb5\x5e\x26\xda\xd5\xb3\x68\x78"
      "\x9e\xff\xb1\xba\x36\xee\x74\x6e\xdd\x58\xbe\xbe\xf0\xfe\x26\xe3\xaf\x0e"
      "\xac\xbf\x55\x97\xe5\xb4\x5a\xfb\xea\x76\xa3\x1c\x7d\x2a\x76\x57\x39\x5f"
      "\x5e\xcd\x64\x7d\x27\x79\x72\x76\x94\x6d\xaf\x95\x9b\x7f\x36\xd8\x36\x5b"
      "\xb5\x1d\xdb\xb8\x03\xb5\x06\xdb\xc6\x8e\x57\x6d\x45\xd1\x5c\xa9\x3f\xda"
      "\xf4\x4a\x9d\xa8\x7f\x87\x1b\x1e\xe9\x7c\xd5\xf6\xc6\xc8\xb6\xb9\xaa\xed"
      "\x44\x5f\xdb\x13\xbf\x6f\xe5\x76\x3a\xb9\xf2\x02\xf2\x07\xc0\x73\x9a\xce"
      "\xe1\x89\xa9\x7f\x4e\x7d\x36\xf5\xc9\xd4\x6f\xa6\xae\x4f\xbd\x33\xf9\xfd"
      "\x43\xdf\x3e\xf4\xe6\x44\xc6\xff\x36\xfe\x9d\xf6\xcc\xd8\x5b\xad\x37\x8b"
      "\xbf\xe4\x93\xfc\xe2\xf1\xfb\x7f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xe0\xd9\xdd\xb9\xf7"
      "\xe1\x8d\x85\x4e\x67\x71\x79\x74\xa5\xb5\x79\xd3\x16\x95\xad\x46\x1e\xa8"
      "\x14\xf5\x17\xfa\x3c\xd3\xb1\x0e\x60\x65\x32\xc9\x13\x5b\xc6\xcb\x0d\x03"
      "\x9d\xe7\xaf\x7d\x7a\xaf\x3c\x09\x7b\x17\xc6\xd4\x60\x18\x43\x95\xee\x2f"
      "\x93\x17\x9e\x9f\xe6\x4b\x04\x47\xf7\xf9\x7d\x59\x69\x67\x3b\x03\x5e\xdc"
      "\xaa\xcf\x47\xfb\x3e\x13\x0e\x7a\x65\x2c\xa3\x27\xc0\xfe\xde\x97\x80\xbd"
      "\x77\x6e\xe5\xe6\xfb\xe7\xee\xdc\xfb\xf0\x9b\x4b\x37\x17\xae\x2d\x5e\x5b"
      "\xbc\x35\x7e\xe1\xc2\xfc\xcc\xfc\x85\xb7\xe7\xce\x5d\x5d\xea\x2c\xce\xf4"
      "\x7e\xee\x77\x94\xc0\x5e\x78\xfc\xd0\xdf\xef\x48\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x80\xed\x1a\xf5\x87\x01\xa7\x5e\xd9\xea\x8f\x46\xb6\xf5\x37"
      "\x1e\xfe\x65\x21\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\x00\x00\xb0\x2b\x2e\x9d\x49\xfb\x7e\x8a\xcc\xce\x9c"
      "\x9d\x29\xd7\xd7\xd7\xe6\x3a\xe5\xd2\xd4\x1f\xf7\x6c\x27\x69\xb5\x92\xe2"
      "\xe7\x49\xf1\x20\xb9\x98\xde\x92\xe9\xbe\xe1\x8a\xfc\xe9\x41\xba\x23\x8e"
      "\xf3\xf1\xd2\xfc\x7b\x9f\x3f\x5c\xff\xa2\x5a\x39\xd4\x8c\x97\xa2\x1c\xb4"
      "\x2e\x9f\xc3\x6a\xbd\xe4\x64\x92\xb1\xba\xdc\xad\xf1\x2e\x3f\xf7\x78\xc5"
      "\xbf\x9b\x57\x58\x26\xec\x74\x93\x38\xd8\x6f\xff\x0d\x00\x00\xff\xff\x77"
      "\x14\xf8\x49",
      1713);
  syz_mount_image(/*fs=*/0x20000000, /*dir=*/0x20000080,
                  /*flags=MS_LAZYTIME|MS_SYNCHRONOUS*/ 0x2000010,
                  /*opts=*/0x20000040, /*chdir=*/0, /*size=*/0x6af,
                  /*img=*/0x20000a00);
  memcpy((void*)0x200010c0, "hfsplus\000", 8);
  memcpy((void*)0x20000080, "./file1\000", 8);
  memcpy(
      (void*)0x20000200,
      "\x00\x00\x00\x11\xd5\x4b\x50\xcb\x52\xe7\x09\xa2\x36\x3c\xad\xde\xd1\x0f"
      "\xa0\xf4\x60\x44\x35\x11\x63\x88\x2d\x5a\x6e\x42\x01\xf5\xa0\x97\xd4\xa7"
      "\x68\x5c\x11\xa0\xa1\x48\xd7\x16\xb7\x42\xb0\xf1\x0c\x1a\x69\x9f\x00",
      53);
  memcpy(
      (void*)0x20000800,
      "\x78\x9c\xec\xdd\x4f\x6c\x1c\x67\xdd\x07\xf0\xef\xac\xed\xb5\x9d\x57\x72"
      "\x9d\x36\x69\xf3\x22\xa4\x5a\x8d\x14\x41\x23\x12\xdb\x4b\x49\x90\x90\x08"
      "\x08\x21\x1f\x2a\x14\x89\x4b\xaf\x56\xe2\x34\x56\x36\x6e\x65\xbb\xc8\x89"
      "\x10\x31\x7f\x0a\x47\x38\xa1\x1e\x7a\x28\x42\xe1\xd0\x13\xe2\x80\x54\xc4"
      "\x01\x51\xce\x48\x48\xdc\x73\x8f\xc4\x3d\x07\x60\xd1\xec\xce\x3a\x6b\xaf"
      "\xbb\xd8\x89\xed\x75\xc2\xe7\x23\xcd\xce\x33\xf3\xcc\xf3\x3c\xbf\xe7\x97"
      "\x99\xd9\xdd\x89\xac\x0d\xf0\x3f\x6b\xe1\xad\x8c\x6d\xa6\xc8\xc2\xf9\x37"
      "\x37\xca\xed\x07\xf7\x1b\xcd\x07\xf7\x1b\xb7\xbb\xe5\x24\xe3\x49\x6a\xc9"
      "\x68\x67\x95\x62\x25\x29\x3e\x4d\xae\xa4\xb3\xe4\xff\xcb\x9d\x55\x77\xc5"
      "\x67\x8d\xf3\xc6\xc3\x4f\x3e\x3c\xf7\xc1\xc7\x8d\xce\xd6\x68\xb5\xb4\x8f"
      "\xaf\x0d\x6a\xd7\x67\xd7\x23\x37\xab\x25\x33\x49\x46\xaa\xf5\x53\xd8\xd6"
      "\xdf\xb5\x5d\xfa\xbb\xb7\xaf\xee\x8a\xad\xb8\xcb\x84\x9d\xed\x26\x0e\x86"
      "\xad\xd5\x67\x73\x3f\xcd\xf7\x7e\xdd\x02\xc7\xd6\xbd\xce\xfb\x66\x9f\xe9"
      "\xe4\x44\x92\x89\xea\x73\x40\xaa\xbb\x43\xed\x68\xa3\x3b\x78\xfb\xba\xcb"
      "\x01\x00\x00\xc0\xf1\xb4\xeb\x77\xf9\x5e\x2f\x3c\xca\xa3\x6c\x64\xea\x68"
      "\xc2\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\xe7\x43\xd1\xf9\xcd\xc0\xa2"
      "\x5a\x6a\xdd\xf2\x4c\x8a\xee\xef\xff\xd7\xab\x7d\xa9\xca\xcf\xb0\xf7\x6f"
      "\x0c\x3b\x02\x00\x00\x00\x00\x00\x00\x00\x38\x08\xaf\x3e\xca\xa3\x6c\x64"
      "\xaa\xbb\xdd\x2a\xda\xff\xe7\xff\x5a\x7b\xe3\x54\xfb\xf5\xff\xf2\x5e\xd6"
      "\xb2\x94\xd5\x5c\xc8\x46\x16\xb3\x9e\xf5\xac\x66\x2e\x19\x9b\xea\xe9\xa8"
      "\xbe\xb1\xb8\xbe\xbe\x3a\xd7\xdf\xf2\x97\x29\x5b\xb6\x5a\xad\x7b\x55\xcb"
      "\xf9\x24\xd3\x7d\x2d\xe7\x8f\x66\xbe\x00\x00\x00\x00\x00\x00\x00\xf0\x9c"
      "\xfa\x51\x16\x32\x35\xec\x20\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\xa0\x57\x91\x8c\x74\x56\xed\xe5\x54\xb7\x3c\x9d\xda\x68\x92\x89\x24\xf5"
      "\xf2\xb8\xcd\xe4\x8f\xdd\xf2\xb3\xec\x4f\xc3\x0e\x00\x00\x00\x00\x0e\xdf"
      "\x64\xb5\x9e\x2a\xfe\xdd\x29\xb4\x8a\xf6\x77\xfe\x97\xdb\xdf\xfb\x27\xf2"
      "\x5e\x56\xb2\x9e\xe5\xac\xa7\x99\xa5\x5c\x6f\x3f\x0b\xe8\x7c\xeb\xaf\xfd"
      "\x6d\xb3\xd1\x7c\x70\xbf\x71\xbb\x5c\xfa\x3b\xfe\xc6\x3f\xf6\x15\x47\xbb"
      "\xc7\x74\x9e\x3d\xec\x3e\xf2\x6c\xfb\x88\xd3\x5b\x2d\x16\xf2\xed\x7c\x37"
      "\xe7\x33\x93\xab\x59\xcd\x72\xbe\x97\xc5\xac\x67\x29\x33\xf9\x56\x39\x89"
      "\x2c\xa6\xc8\x74\xf5\xf4\x62\xba\x1b\xe7\xee\xf1\x5e\xd9\xb6\x75\x75\x67"
      "\x6c\xaf\xee\xd8\x3e\xd3\x8e\x64\x32\x37\xb2\xdc\x8e\xed\x42\xae\xd5\xbb"
      "\xa1\xd7\xaa\xf8\xcf\xf4\x8c\xf6\xfb\x7a\xb2\x63\xc4\x1f\x96\xd9\x29\xbe"
      "\x5e\xd9\x63\x8e\xae\x57\xeb\x72\x46\xbf\xa8\xd6\x95\xd6\x0b\x7b\xec\xe3"
      "\x70\x4c\xb7\x67\x3e\xb6\x95\x91\xd9\x2a\xf7\x65\x36\x4e\x0e\xce\xfd\x3e"
      "\xcf\x93\x9d\x23\xcd\xa5\xb6\xf5\x0c\xea\xd4\xe3\x51\xca\xcd\x9d\x23\x3d"
      "\x51\xce\x4f\x54\xeb\x32\xd7\x3f\xdd\x9e\xf3\x83\xb6\xcf\x47\x69\x3b\x33"
      "\x31\x9f\x5a\x75\xf6\x25\x2f\x0f\xce\x79\xf2\xc5\xbf\xff\xf9\xea\xcd\xda"
      "\xca\xad\x9b\x37\xd6\xce\x1f\xe2\x94\x8e\xc6\xce\x4c\x34\x7a\x32\xf1\xca"
      "\x9e\x32\xd1\x2c\x33\xb1\xb9\xf7\x4c\x8c\xed\xdc\x31\xf1\xb4\x73\x38\x18"
      "\xf5\x2a\x1b\x9d\xbb\xe8\x5e\xef\x96\x65\x69\x31\xaf\xb5\xdb\x4e\xfd\xab"
      "\x95\xef\xe4\x9d\x5c\xcf\x52\x2e\x65\x36\x73\xb9\x9c\xd9\x7c\x35\xf3\x69"
      "\x6c\x3b\xc3\x4e\xf7\xe5\xb5\xd6\x1b\x48\xfb\x5a\xab\xed\xef\x5a\x3b\xfb"
      "\x85\xaa\x50\xbe\x27\xfd\xac\xe7\xbd\x69\xf8\xca\xbc\x9e\xec\xc9\x6b\xef"
      "\x9d\x6e\xba\x5d\x57\xed\xb9\xf2\xf3\xcc\xf6\x64\xe9\xc5\xc1\x67\xdf\x93"
      "\xdc\x91\x46\x3f\x57\x15\xca\x31\x7e\xbc\xf5\x8e\x73\x1c\x6c\xcb\x44\x75"
      "\x6f\xee\x46\xf7\xd2\xe0\x4c\xfc\xaa\x55\xbe\xae\x35\x57\x6e\xad\xde\x5c"
      "\x7c\x77\x8f\xe3\x9d\xab\xd6\xe5\x65\xfb\xfe\xf6\x7b\xf3\xaf\x0f\x62\x3e"
      "\x4f\xae\x3c\x5f\x5e\x2c\xff\xb1\xda\x5b\x27\xb7\x9d\x2f\x65\xdd\x4b\xbb"
      "\xd6\xcd\xb5\xeb\x4e\x6d\xd5\xd5\xfa\xea\x4e\x6f\xd5\x4d\x67\x2a\xcb\x03"
      "\xae\xd4\x7a\xf5\x19\xae\xbf\xa7\x4e\xdd\x2b\xbb\xd6\x35\xda\x75\x67\x7a"
      "\xea\xb6\x7d\xca\xc9\x3b\x69\x6e\x7d\x0a\x01\xe0\x18\x3b\xf1\xfa\x89\xfa"
      "\xe4\xc3\xc9\xbf\x4e\x7e\x34\xf9\x93\xc9\x9b\x93\x6f\x4e\x7c\x73\xfc\xf2"
      "\xf8\xe7\xeb\x19\xfb\xcb\xe8\x1f\x46\x7e\x5b\xfb\x4d\xed\x6b\xc5\xeb\xf9"
      "\x28\x3f\xc8\xd4\xb0\x23\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\xe7\xc1\xda\x9d\xbb\xb7"
      "\x16\x9b\xcd\xa5\xd5\x63\x58\x48\xed\x08\xc7\x2a\xaa\x42\xfd\x78\xcc\xfd"
      "\xd9\x2e\xb4\x46\x92\x5d\xaa\x7e\x97\x64\x40\xab\xfa\x53\x8f\x5e\x1c\xfe"
      "\xf9\x5c\x5e\x34\x87\x36\xc4\x3f\x5b\x1d\x4f\xd5\xcf\xe8\x80\x24\x8c\x67"
      "\x60\xf3\xc9\x3c\xde\x33\xec\x3b\x13\x70\xd8\x2e\xae\xdf\x7e\xf7\xe2\xda"
      "\x9d\xbb\x5f\x5a\xbe\xbd\xf8\xf6\xd2\xdb\x4b\x2b\xf3\x97\x2f\x5d\xbe\xd4"
      "\xf8\xca\xdc\x97\x2f\xde\x58\x6e\x2e\xcd\x76\x5e\x87\x1d\x25\x70\x18\xd6"
      "\xee\xdc\x1d\x19\x76\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00\xc0\xfe\x1c"
      "\xfa\xdf\x4b\xd4\x57\xd7\x86\x3d\x47\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xe0\xd9\xb6\xf0\x56\xc6"
      "\x36\x53\x64\x6e\xf6\xc2\x6c\xb9\xfd\xe0\x7e\xa3\x59\x2e\xdd\xf2\xe3\x23"
      "\x47\x93\xd4\x92\x14\xdf\x4f\x8a\x4f\x93\x2b\xe9\x2c\x99\xee\xe9\xae\xf8"
      "\xac\x71\xde\x78\xf8\xc9\x87\xe7\x3e\xf8\xb8\xf1\xb8\xaf\xd1\xee\xf1\xb5"
      "\x41\xed\xf6\x66\xb3\x5a\x32\x93\x64\xa4\x5a\xff\x77\xe3\xbb\x74\xd3\xdf"
      "\xdf\xb5\x9e\xfe\x36\x9f\x28\xbc\x62\x6b\x86\x65\xc2\xce\x76\x13\x07\xc3"
      "\xf6\x9f\x00\x00\x00\xff\xff\x68\x67\x09\x4e",
      1667);
  res = -1;
  res = syz_mount_image(/*fs=*/0x200010c0, /*dir=*/0x20000080,
                        /*flags=MS_RDONLY|MS_NOATIME|MS_DIRSYNC*/ 0x481,
                        /*opts=*/0x20000200, /*chdir=*/0, /*size=*/0x683,
                        /*img=*/0x20000800);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000000, "fuse\000", 5);
  memcpy((void*)0x20000180, "./file1\000", 8);
  syz_mount_image(/*fs=*/0x20000000, /*dir=*/0x20000180,
                  /*flags=MS_REC|MS_REMOUNT*/ 0x4020, /*opts=*/0x20000180,
                  /*chdir=*/0, /*size=*/0, /*img=*/0);
  memcpy((void*)0x20000100, "./file1\000", 8);
  syscall(__NR_unlinkat, /*fd=*/r[0], /*path=*/0x20000100ul, /*flags=*/0ul);
  return 0;
}
