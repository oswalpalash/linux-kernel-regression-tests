// https://syzkaller.appspot.com/bug?id=ba84cc80a9491d65416bc7877e1650c87530fe8a
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>
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
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/loop.h>

#ifndef __NR_getsockopt
#define __NR_getsockopt 209
#endif
#ifndef __NR_memfd_create
#define __NR_memfd_create 279
#endif
#ifndef __NR_mmap
#define __NR_mmap 222
#endif
#ifndef __NR_openat
#define __NR_openat 56
#endif
#ifndef __NR_write
#define __NR_write 64
#endif

static unsigned long long procid;

static void sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
}

static uint64_t current_time_ms(void)
{
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    exit(1);
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

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

static void kill_and_wait(int pid, int* status)
{
  kill(-pid, SIGKILL);
  kill(pid, SIGKILL);
  for (int i = 0; i < 100; i++) {
    if (waitpid(-1, status, WNOHANG | __WALL) == pid)
      return;
    usleep(1000);
  }
  DIR* dir = opendir("/sys/fs/fuse/connections");
  if (dir) {
    for (;;) {
      struct dirent* ent = readdir(dir);
      if (!ent)
        break;
      if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        continue;
      char abort[300];
      snprintf(abort, sizeof(abort), "/sys/fs/fuse/connections/%s/abort",
               ent->d_name);
      int fd = open(abort, O_WRONLY);
      if (fd == -1) {
        continue;
      }
      if (write(fd, abort, 1) < 0) {
      }
      close(fd);
    }
    closedir(dir);
  } else {
  }
  while (waitpid(-1, status, __WALL) != pid) {
  }
}

static void reset_loop()
{
  char buf[64];
  snprintf(buf, sizeof(buf), "/dev/loop%llu", procid);
  int loopfd = open(buf, O_RDWR);
  if (loopfd != -1) {
    ioctl(loopfd, LOOP_CLR_FD, 0);
    close(loopfd);
  }
}

static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  write_file("/proc/self/oom_score_adj", "1000");
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter = 0;
  for (;; iter++) {
    reset_loop();
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      setup_test();
      execute_one();
      exit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      if (waitpid(-1, &status, WNOHANG | WAIT_FLAGS) == pid)
        break;
      sleep_ms(1);
      if (current_time_ms() - start < 5000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[1] = {0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  memcpy((void*)0x20000440, "ext4\000", 5);
  memcpy((void*)0x20000480, "./file0\000", 8);
  memcpy(
      (void*)0x20000900,
      "\x78\x9c\xec\xdb\xcb\x6f\x1b\xc5\x1f\x00\xf0\xef\xae\x93\xf6\xd7\xd7\x2f"
      "\xa1\x94\x47\x1f\x40\xa0\x20\x22\x1e\x49\x93\x16\xe8\x81\x03\x20\x90\x38"
      "\x80\x84\x04\x87\x72\x4c\x93\xb4\x2a\x75\x1b\xd4\x04\x89\x56\x15\x2d\x08"
      "\x95\x1b\xa8\x12\x77\xc4\x11\x89\xbf\x80\x13\x5c\x10\x70\x42\xe2\x0a\x77"
      "\xa8\x54\xa1\x5e\x5a\x38\x19\xad\xbd\x9b\x38\xae\x6d\xe2\xc4\x89\x53\xfc"
      "\xf9\x48\xdb\xce\xec\x4c\x34\xf3\xf5\xec\xc4\xb3\x3b\xd9\x00\xfa\xd6\x48"
      "\xf6\x4f\x12\xb1\x33\xa9\xe5\x87\x6a\xd9\xe5\x15\x46\x6a\xff\xdd\xba\x71"
      "\x71\xfa\xaf\x1b\x17\xa7\x93\xa8\x54\xde\xfc\x33\xa9\xd6\xbb\x79\xe3\xe2"
      "\x74\x51\xb5\xf8\xb9\x1d\x79\x66\x34\x8d\x48\x3f\x4e\x62\x7f\x93\x76\xe7"
      "\xcf\x5f\x38\x3d\x55\x2e\xcf\x9e\xcb\xf3\xe3\x0b\x67\xde\x1d\x9f\x3f\x7f"
      "\xe1\xe9\x53\x67\xa6\x4e\xce\x9e\x9c\x3d\x3b\x79\xf4\xe8\x91\xc3\x13\xcf"
      "\x3d\x3b\xf9\x4c\x57\xe2\xcc\xe2\xba\xb9\xef\x83\xb9\x03\x7b\x5f\x7d\xfb"
      "\xea\xeb\xd3\xc7\xae\x1e\xff\xf1\xeb\xa4\x88\xbf\x21\x8e\x2e\x19\x69\x57"
      "\xf8\x58\x97\x1b\xeb\xb5\x5d\x75\xe9\x64\xa0\x87\x1d\xa1\x23\xa5\x88\xc8"
      "\x86\x6b\x30\x22\x7e\x8d\xa1\x28\xc5\xd2\xe0\x0d\xc5\x2b\x1f\xf5\xb4\x73"
      "\xc0\xba\xaa\x54\x2a\x95\xad\xad\x8b\x2f\x57\x80\xff\xb0\x24\x7a\xdd\x03"
      "\xa0\x37\x8a\x2f\xfa\xec\xfe\xb7\x38\x36\x68\xe9\xb1\x29\x5c\x7f\xb1\x76"
      "\x03\x94\xc5\x7d\x2b\x3f\x6a\x25\x03\x91\xe6\x75\x06\x1b\xee\x6f\xbb\x69"
      "\x24\x22\x8e\x5d\xfe\xfb\x8b\xec\x88\xf5\x79\x0e\x01\x00\xb0\xcc\xb7\xd9"
      "\xfa\xe7\xa9\x66\xeb\xbf\x34\xee\xad\xab\xf7\xff\x7c\x0f\x65\x38\x22\xee"
      "\x8a\x88\xdd\x11\x71\x77\x44\xec\x89\x88\x7b\x22\xaa\x75\xef\x8b\x88\xfb"
      "\x3b\x6c\xbf\x71\x93\xe4\xf6\xf5\x4f\x7a\x6d\x55\x81\xad\x50\xb6\xfe\x7b"
      "\x3e\xdf\xdb\x5a\xbe\xfe\x2b\x56\x7f\x31\x5c\xca\x73\xbb\xaa\xf1\x0f\x26"
      "\x27\x4e\x95\x67\x0f\xe5\x9f\xc9\x68\x0c\x6e\xcd\xf2\x13\x6d\xda\xf8\xee"
      "\xe5\x5f\x3e\x6b\x55\x56\xbf\xfe\xcb\x8e\xac\xfd\x62\x2d\x98\xf7\xe3\xda"
      "\x40\xc3\x03\xba\x99\xa9\x85\xa9\xb5\xc4\x5c\xef\xfa\x87\x11\xfb\x06\x9a"
      "\xc5\x9f\x2c\xee\x04\x24\x11\xb1\x37\x22\xf6\xad\xb2\x8d\x53\x4f\x7c\x75"
      "\xa0\x55\xd9\xbf\xc7\xdf\x46\x17\xf6\x99\x2a\x5f\x46\x3c\x5e\x1b\xff\xcb"
      "\xd1\x10\x7f\x21\x69\xbf\x3f\x39\xfe\xbf\x28\xcf\x1e\x1a\x2f\xae\x8a\xdb"
      "\xfd\xf4\xf3\x95\x37\x5a\xb5\xbf\xa6\xf8\xbb\x20\x1b\xff\xed\x4d\xaf\xff"
      "\xc5\xf8\x87\x93\xfa\xfd\xda\xf9\xce\xdb\xb8\xf2\xdb\x27\x8d\xf7\x34\x97"
      "\x8a\xc4\x6a\xaf\xff\x2d\xc9\x5b\xd5\xf4\x96\xfc\xdc\xfb\x53\x0b\x0b\xe7"
      "\x26\x22\xb6\x24\xaf\xe5\xe7\x5f\x58\x3a\x3f\xb9\xf4\xb3\x45\xbe\xa8\x9f"
      "\xc5\x3f\x7a\xb0\xf9\xfc\xdf\x1d\x4b\x9f\xc4\xfe\x88\xc8\x2e\xe2\x07\x22"
      "\xe2\xc1\x88\x78\x28\xef\xfb\xc3\x11\xf1\x48\x44\x1c\x6c\x13\xff\x0f\x2f"
      "\x3d\xfa\x4e\xab\xb2\xcd\x30\xfe\x33\x1d\x8d\x7f\xe7\x89\xd2\xe9\xef\xbf"
      "\x59\xd6\xe8\x70\x27\xf1\x67\xe3\x7f\xa4\x9a\x1a\xcd\xcf\xac\xe4\xf7\xdf"
      "\x4a\x3b\xb8\xd6\xcf\x0f\x00\x00\x00\xee\x04\x69\x44\xec\x8c\x24\x1d\x5b"
      "\x4c\xa7\xe9\xd8\x58\xed\x6f\xf8\xf7\xc4\xf6\xb4\x3c\x37\xbf\xf0\xe4\x89"
      "\xb9\xf7\xce\xce\x64\x65\xd5\xe7\x9f\x69\xf1\xa4\x6b\xa8\xee\x79\xe8\x44"
      "\x7e\x5b\x5f\xe4\x27\x1b\xf2\x87\xf3\xe7\xc6\x9f\x97\xb6\x55\xf3\x63\xd3"
      "\x73\xe5\x99\x5e\x07\x0f\x7d\x6e\x47\x8b\xf9\x9f\xf9\xbd\xd4\xeb\xde\x01"
      "\xeb\xce\xfb\x5a\xd0\xbf\xda\xce\xff\x64\xe3\xfa\x01\x6c\x3c\xdf\xff\xd0"
      "\xbf\xcc\x7f\xe8\x5f\x4d\xe6\xff\xb6\x5e\xf4\x03\xd8\x78\xcd\xbe\xff\x2f"
      "\xf5\xa0\x1f\xc0\xc6\x1b\xa8\xbe\xe2\x0e\xf4\x23\xf7\xff\xd0\xbf\xcc\x7f"
      "\xe8\x5f\xe6\x3f\xf4\xa5\xb5\xbc\xd7\x2f\xd1\x0f\x89\x3f\x3e\xad\x5d\x29"
      "\x8d\x45\x91\x6e\x96\x1e\x4a\xac\x47\x62\xf9\xef\x89\xe3\xde\x05\x00\x00"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xee\x50\xff\x04\x00\x00\xff\xff"
      "\xb1\x02\xe4\xde",
      1084);
  syz_mount_image(0x20000440, 0x20000480, 0x1e, 0x20000000, 1, 0x43c,
                  0x20000900);
  memcpy((void*)0x200001c0, "memory.current\000", 15);
  res = syscall(__NR_openat, 0xffffff9c, 0x200001c0ul, 0x275aul, 0ul);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000040, "syz1", 4);
  syscall(__NR_write, r[0], 0x20000040ul, 4ul);
  syscall(__NR_mmap, 0x20000000ul, 0xb36000ul, 2ul, 0x13ul, r[0], 0ul);
  *(uint32_t*)0x20000280 = 0;
  syscall(__NR_getsockopt, -1, 1, 0x11, 0ul, 0x20000280ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  loop();
  return 0;
}
