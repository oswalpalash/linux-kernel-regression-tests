// https://syzkaller.appspot.com/bug?id=deb061ff946b02c2f4821f91683d89a68b2f45f8
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
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
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <linux/capability.h>
#include <linux/loop.h>

#ifndef __NR_memfd_create
#define __NR_memfd_create 319
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

static void setup_common()
{
  if (mount(0, "/sys/fs/fuse/connections", "fusectl", 0, 0)) {
  }
}

static void setup_binderfs()
{
  if (mkdir("/dev/binderfs", 0777)) {
  }
  if (mount("binder", "/dev/binderfs", "binder", 0, NULL)) {
  }
  if (symlink("/dev/binderfs", "./binderfs")) {
  }
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setsid();
  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = (200 << 20);
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 32 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 136 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 128 << 20;
  setrlimit(RLIMIT_CORE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 256;
  setrlimit(RLIMIT_NOFILE, &rlim);
  if (unshare(CLONE_NEWNS)) {
  }
  if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL)) {
  }
  if (unshare(CLONE_NEWIPC)) {
  }
  if (unshare(0x02000000)) {
  }
  if (unshare(CLONE_NEWUTS)) {
  }
  if (unshare(CLONE_SYSVSEM)) {
  }
  typedef struct {
    const char* name;
    const char* value;
  } sysctl_t;
  static const sysctl_t sysctls[] = {
      {"/proc/sys/kernel/shmmax", "16777216"},
      {"/proc/sys/kernel/shmall", "536870912"},
      {"/proc/sys/kernel/shmmni", "1024"},
      {"/proc/sys/kernel/msgmax", "8192"},
      {"/proc/sys/kernel/msgmni", "1024"},
      {"/proc/sys/kernel/msgmnb", "1024"},
      {"/proc/sys/kernel/sem", "1024 1048576 500 1024"},
  };
  unsigned i;
  for (i = 0; i < sizeof(sysctls) / sizeof(sysctls[0]); i++)
    write_file(sysctls[i].name, sysctls[i].value);
}

static int wait_for_loop(int pid)
{
  if (pid < 0)
    exit(1);
  int status = 0;
  while (waitpid(-1, &status, __WALL) != pid) {
  }
  return WEXITSTATUS(status);
}

static void drop_caps(void)
{
  struct __user_cap_header_struct cap_hdr = {};
  struct __user_cap_data_struct cap_data[2] = {};
  cap_hdr.version = _LINUX_CAPABILITY_VERSION_3;
  cap_hdr.pid = getpid();
  if (syscall(SYS_capget, &cap_hdr, &cap_data))
    exit(1);
  const int drop = (1 << CAP_SYS_PTRACE) | (1 << CAP_SYS_NICE);
  cap_data[0].effective &= ~drop;
  cap_data[0].permitted &= ~drop;
  cap_data[0].inheritable &= ~drop;
  if (syscall(SYS_capset, &cap_hdr, &cap_data))
    exit(1);
}

static int do_sandbox_none(void)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid != 0)
    return wait_for_loop(pid);
  setup_common();
  sandbox_common();
  drop_caps();
  if (unshare(CLONE_NEWNET)) {
  }
  write_file("/proc/sys/net/ipv4/ping_group_range", "0 65535");
  setup_binderfs();
  loop();
  exit(1);
}

void loop(void)
{
  memcpy((void*)0x20000100, "hfsplus\000", 8);
  memcpy((void*)0x20000640, "./file0\000", 8);
  memcpy((void*)0x20000080,
         "\x00\x91\xfc\x98\x36\x8c\xa7\xa5\x27\xe3\xac\x06\x2c\x70\xd1\x60\xf4"
         "\x72\x21\x7a\x71\x91\xe1\x85\x7a\x04\x80\xba\xb6\x34\xe9\xa0\x07\x00"
         "\x72\x36\xeb\x9a\x47\xb8\x40\x06\x06\xc7\x46\x3b\x25\x9b\x64\x8e\x95"
         "\x93\xcb\x3f\x84\xdf\xca\x74\xa3\xe8\x94\x80\x52\xc3\x82\x26\xc7\x32"
         "\x54\x90\xcf\x03\xb5\x29\x70\x59\x01\xda\xda\x10\x63\xaa\xfd\x43\x2b"
         "\xc8\x7b\x61\x95\x37\xce\x4b\x95\x0c\xed\x06\xab\xc3\x21\xf3\xdb\x18"
         "\x8f\x6a\x8f\xf9\xb9\x06\xbb\x67\xa1",
         111);
  memcpy(
      (void*)0x20000c80,
      "\x78\x9c\xec\xdd\xcb\x6b\x5c\xd7\x1d\x07\xf0\xef\x1d\x8f\x65\x8f\x0b\x8e"
      "\x92\xd8\x89\x5b\x02\x15\x31\xa4\xa5\xa2\xb6\x1e\x28\xad\xba\x89\x5b\x4a"
      "\xd1\x22\x94\x90\x42\xbb\x16\xb6\x1c\x0f\x1e\x2b\x41\x9a\x14\x25\x94\xd6"
      "\xe9\x73\xdb\x45\xfe\x80\x74\xa1\x5d\xe9\xa2\xd0\xbd\x21\x5d\xb7\xbb\x6c"
      "\xb5\x0c\x14\xba\x68\x56\xda\xa9\xdc\x3b\x77\xe4\x89\x25\x2b\x23\x45\xd6"
      "\x8c\xe2\xcf\xc7\x9c\x39\xe7\xdc\x73\xcf\xb9\xe7\xfe\xee\x63\xe6\x8e\x30"
      "\x13\xe0\xa9\xb5\x34\x9d\xe6\x83\x14\x59\x9a\x7e\x7d\xa3\xac\x6f\x6d\xce"
      "\x77\xb6\x36\xe7\xcf\xd5\xcd\x9d\x24\x65\xb9\x91\x34\x7b\x59\x8a\xd5\xa4"
      "\xf8\x38\xb9\x91\x5e\xca\xd7\xcb\x85\xf5\xfa\xc5\xe3\xb6\xf3\x61\x7b\xf1"
      "\xcd\x4f\x3e\xdb\xfa\xb4\x57\x6b\xd6\xa9\x5a\xbf\x71\x50\xbf\xe1\xdc\xaf"
      "\x53\xa6\x92\x9c\xa9\xf3\xbd\x5e\x3b\xd2\x78\x37\x1f\x3b\xde\xb0\x8a\xdd"
      "\x3d\x2c\x03\x76\xb5\x1f\x38\x18\xb5\x9d\x3d\xee\x1f\xa6\xfb\x97\xbc\x6e"
      "\x81\x71\x50\xf4\xde\x37\xf7\x98\x4c\x2e\x24\x39\x5f\x7f\x0e\x48\x7d\x77"
      "\x68\x9c\xec\xec\x8e\xdf\xa1\xee\x72\x00\x00\x00\x70\x4a\x3d\xb3\x9d\xed"
      "\x6c\xe4\xe2\xa8\xe7\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xa7\x49\xfd"
      "\xfb\xff\x45\x9d\x1a\xfd\xf2\x54\x8a\xfe\xef\xff\x4f\xd4\xcb\x52\x97\x4f"
      "\x87\xbf\xfd\x7c\xdf\xc5\x0f\x4e\x7c\x22\x00\x00\x00\x00\x00\x00\x00\x70"
      "\xfc\xbe\xb9\x9d\xed\x6c\xe4\x62\xbf\xbe\x53\x54\x7f\xf3\x7f\xb9\xaa\x5c"
      "\xaa\x5e\xbf\x96\x77\xb3\x9e\x95\xac\xe5\x5a\x36\xb2\x9c\x6e\xba\x59\xcb"
      "\x6c\x92\xc9\x81\x81\x26\x36\x96\xbb\xdd\xb5\xd9\x21\x7a\xce\xed\xdb\x73"
      "\xee\x64\xf6\x17\x00\x00\x00\x00\x00\x00\x00\xbe\xa2\x7e\x9b\xa5\x87\x7f"
      "\xff\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x71\x50\x24\x67\x7a"
      "\x59\x95\x2e\xf5\xcb\x93\x69\x34\x93\x9c\x4f\x32\x51\xae\x77\x3f\xf9\x77"
      "\xbf\x7c\x9a\x3d\x18\xf5\x04\x00\x00\x00\xe0\x04\x3c\xb3\x9d\xed\x6c\xe4"
      "\x62\xbf\xbe\x53\x54\xcf\xfc\x2f\x54\xcf\xfd\xe7\xf3\x6e\x56\xd3\x4d\x3b"
      "\xdd\x74\xb2\x92\x5b\xd5\x77\x01\xbd\xa7\xfe\xc6\xd6\xe6\x7c\x67\x6b\x73"
      "\xfe\x5e\x99\xf6\x8e\xfb\xc3\xff\x1e\x6a\x1a\xd5\x88\xe9\x7d\xf7\xb0\xff"
      "\x96\xaf\xa4\x48\x23\xad\xdc\x4e\xbb\x5a\x72\x2d\x37\xf3\x76\x3a\xb9\x95"
      "\x46\xd5\xb3\x74\xa5\x3f\x9f\xfd\xe7\xf5\x41\x39\xa7\xe2\xb5\xda\x90\x33"
      "\xbb\x55\xe7\xe5\x9e\xff\xb9\xce\xc7\xc3\x64\x15\xb3\xb3\xbb\x11\x99\xa9"
      "\xe7\x56\x46\xe3\xd9\x83\x23\x71\xc8\xa3\xf3\xe8\x96\x66\xd3\xd8\xfd\xe6"
      "\xe7\xd2\x13\x88\xf9\x85\x3a\x2f\xf7\xe7\x8f\x63\x1d\xf3\xb9\x81\xb3\xef"
      "\x85\x83\x23\x91\x7c\xeb\x1f\x7f\xfd\xc5\x9d\xce\xea\xdd\x3b\xb7\xd7\xa7"
      "\xc7\x67\x97\x8e\xe8\xd1\x48\xcc\x0f\x44\xe2\xc5\xa7\x2a\x12\x33\x55\x24"
      "\x2e\xef\xd6\x97\xf2\x93\xfc\x2c\xd3\x99\xca\x1b\x59\x4b\x3b\xbf\xcc\x72"
      "\xba\x59\xc9\x54\x7e\x5c\x95\x96\xeb\xf3\xb9\x7c\x9d\xac\x22\xf4\xbf\x0f"
      "\x1e\x13\xa9\x1b\x9f\xab\xbd\xf1\x45\x33\x99\xa8\x8f\x4b\xef\x2e\x7a\xb8"
      "\x39\xbd\x5c\xf5\xbd\x98\x76\x7e\x9a\xb7\x73\x2b\x2b\x79\xb5\xfa\x37\x97"
      "\xd9\x7c\x2f\x0b\x59\xc8\xe2\xc0\x11\xbe\x3c\xc4\x55\xdf\x38\xdc\x55\x7f"
      "\xf5\xdb\x75\xa1\x95\xe4\x4f\x75\x3e\x1e\xca\xb8\x3e\x3b\x10\xd7\xc1\x7b"
      "\xee\x64\xd5\x36\xb8\xe4\x61\x94\x9e\x3b\xfe\x7b\x63\xf3\x1b\x75\xa1\xdc"
      "\xc6\xef\xea\x7c\x3c\x3c\x1a\x89\xd9\x81\x48\x3c\x7f\x70\x24\xfe\xb2\x53"
      "\xbe\xae\x77\x56\xef\xae\xdd\x59\x7e\x67\xc8\xed\xbd\x52\xe7\xe5\x75\xf4"
      "\x87\xb1\x7a\x97\x28\xcf\x97\xe7\xca\x83\x55\xd5\x3e\x7f\x76\x94\x6d\xcf"
      "\xef\xdb\x36\x5b\xb5\x5d\xda\x6d\x6b\xec\x69\xbb\xbc\xdb\xf6\x45\x57\xea"
      "\x44\xfd\x19\x6e\xef\x48\x73\x55\xdb\x8b\xfb\xb6\xcd\x57\x6d\x57\x06\xda"
      "\xf6\xfb\xbc\x05\xc0\xd8\xbb\xf0\x9d\x0b\x13\xad\xff\xb4\xfe\xd5\xfa\xa8"
      "\xf5\xfb\xd6\x9d\xd6\xeb\xe7\x7f\x74\xee\xfb\xe7\x5e\x9a\xc8\xd9\x7f\x9e"
      "\xfd\x41\x73\xe6\xcc\x2b\x8d\x97\x8a\xbf\xe7\xa3\xfc\xfa\xe1\xf3\x3f\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x70\x74\xeb\xef\xbd\x7f\x77\xb9\xd3\x59\x59\x3b\x8e\x42"
      "\x51\xff\x1a\xcf\x11\xba\xf7\xbb\x1e\xeb\x7c\x14\x14\x14\x8e\x58\x18\xf5"
      "\x9d\x09\x78\xd2\xae\x77\xef\xbd\x73\x7d\xfd\xbd\xf7\xbf\xdb\xbe\xb7\xfc"
      "\xd6\xca\x5b\x2b\xab\x67\x17\x16\x16\x67\x16\x17\x5e\x9d\xbf\x7e\xbb\xdd"
      "\x59\x99\xe9\xbd\x8e\x7a\x96\xc0\x93\xf0\xf0\x4d\x7f\xd4\x33\x01\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x86\x75\xe4\xff\x21\xd0\xca\xd0\x2b\x8f\x7a"
      "\x1f\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x80\xd3\x6d\x69\x3a\xcd\x07\x29\x32\x3b\x73\x6d\xa6\xac"
      "\x6f\x6d\xce\x77\xca\xd4\x2f\x57\x2b\xed\xfc\x26\x49\x33\x49\x23\x49\xf1"
      "\xab\xa4\xf8\x38\xb9\x91\x5e\xca\xe4\xc0\x70\xc5\xe3\xb6\xf3\x61\x7b\xf1"
      "\xcd\x4f\x3e\xdb\xfa\xb4\x57\x6b\xd6\xa9\x5a\xbf\x71\x50\xbf\xe1\xdc\xaf"
      "\x53\xa6\x92\x9c\xa9\xf3\xe3\x1a\xef\xe6\x97\x1e\xaf\xd8\xdd\xc3\x32\x60"
      "\x57\xfb\x81\x83\x51\xfb\x7f\x00\x00\x00\xff\xff\x6d\x5b\x09\x0a",
      1528);
  syz_mount_image(0x20000100, 0x20000640, 0x10000, 0x20000080, 1, 0x5f8,
                  0x20000c80);
  memcpy((void*)0x20000000, "./file0\000", 8);
  syscall(__NR_open, 0x20000000ul, 0x240ul, 2ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  do_sandbox_none();
  return 0;
}
