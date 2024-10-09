// https://syzkaller.appspot.com/bug?id=ff9fe3de46d66c92f26a627abe9e85d0518c9890
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

#define USLEEP_FORKED_CHILD (3 * 50 * 1000)

static long handle_clone_ret(long ret)
{
  if (ret != 0) {
    return ret;
  }
  usleep(USLEEP_FORKED_CHILD);
  syscall(__NR_exit, 0);
  while (1) {
  }
}

static long syz_clone(volatile long flags, volatile long stack,
                      volatile long stack_len, volatile long ptid,
                      volatile long ctid, volatile long tls)
{
  long sp = (stack + stack_len) & ~15;
  long ret = (long)syscall(__NR_clone, flags & ~CLONE_VM, sp, ptid, ctid, tls);
  return handle_clone_ret(ret);
}

uint64_t r[1] = {0xffffffffffffffff};

void loop(void)
{
  intptr_t res = 0;
  memcpy((void*)0x20000600, "hfsplus\000", 8);
  memcpy((void*)0x20000640, "./file0\000", 8);
  memcpy((void*)0x20000300,
         "\x00\x69\x3c\x50\xc4\xf7\x63\x70\x50\xb2\x33\xc6\x10\xa8\x69\x6b\x99"
         "\xbc\x71\xb5\x05\x13\xe1\x6d\xd5\x67\x41\xc3\x8f\x34\xcb\x29\x06\xf1"
         "\x88\x5c\x41\x02\xd4\xb7\xf0\xf3\x01\x1b\x55\xc9\x83\xe5\x19\xf3\x1a"
         "\x2d\x64\x59\x4b\xf7\x09\x3a\x81\x1f\x44\xde\xde\xa6\x77\x4d\x0a\xd1"
         "\x45\xd9\x4a\xe2\x6c\x01\x01\x5d\xed\x78\x88\xde\x20\x13\x9b\x00\xd6"
         "\xb4\xc5\x83\x80\x87\x08",
         91);
  memcpy(
      (void*)0x200006c0,
      "\x78\x9c\xec\xdd\x4f\x68\x1c\xd7\x1d\x07\xf0\xef\xac\x65\xd9\xeb\x82\xb3"
      "\x49\xec\x24\x2d\x85\x8a\x18\x4c\x1b\x53\x5b\xab\x6d\x6a\x17\x0a\x75\x4b"
      "\x29\x3a\x84\x62\xe8\x25\x57\x61\xaf\x63\xe1\xb5\x12\xa4\x4d\x51\x42\x29"
      "\xea\xff\x6b\x8f\x39\xe4\x90\x1e\xd4\x43\x4e\xbd\xa6\xf4\x10\x9a\x9e\x7b"
      "\xea\x5d\x77\x43\xef\xba\xb9\xcc\xec\xec\x6a\x6d\x29\x8a\x14\xcb\xda\x55"
      "\xf3\xf9\xc0\xdb\xf7\xde\xbe\x99\x37\xef\xfd\x3c\xf3\xb4\x33\x8b\xd9\x00"
      "\x5f\x59\x8b\x6f\xe6\xf4\x46\x8a\x2c\x5e\x79\x63\xbd\xac\x6f\x6d\x76\x7a"
      "\x5b\x9b\x9d\x07\xc3\x72\x92\x33\x49\x1a\xc9\xcc\x20\x4b\xb1\x92\x14\x9f"
      "\x25\x37\x33\x48\xf9\x7a\xf9\x66\xdd\x5d\xf1\x79\xc7\x79\xfd\xe1\x27\x1f"
      "\x5e\xfe\xe0\xe3\xce\xa0\x36\x53\xa7\x6a\xfb\xc6\x7e\xfb\x1d\xcc\x46\x9d"
      "\x32\x97\xe4\x54\x9d\x1f\x55\x7f\xb7\x9f\xba\xbf\x62\x34\xc3\x32\x60\x97"
      "\x86\x81\x83\x49\x7b\xb4\xcb\xc6\x61\x76\x7f\xca\xeb\x16\x98\x06\xc5\xe0"
      "\xef\xe6\x2e\xad\xe4\x5c\x92\xb3\xf5\xe7\x80\xd4\xab\x43\xe3\x78\x47\x77"
      "\xf4\x0e\xb5\xca\x01\x00\x00\xc0\x09\xf5\xdc\x76\xb6\xb3\x9e\xf3\x93\x1e"
      "\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x9c\x24\xf5\xef\xff\x17\x75\x6a"
      "\x0c\xcb\x73\x29\x86\xbf\xff\x3f\x5b\xbf\x97\xba\x7c\xa2\x7d\x3a\xe9\x01"
      "\x00\x00\x00\x00\x00\x00\x00\xc0\x11\xf8\xd6\x76\xb6\xb3\x9e\xf3\xc3\xfa"
      "\xa3\xa2\xfa\xce\xff\xd5\xaa\x72\xa1\x7a\xfd\x5a\xde\xcd\x5a\xba\x59\xcd"
      "\xd5\xac\x67\x29\xfd\xf4\xb3\x9a\x76\x92\xd6\x58\x47\xb3\xeb\x4b\xfd\xfe"
      "\x6a\xfb\x00\x7b\x2e\xec\xb9\xe7\xc2\xf1\xcc\x17\x00\x00\x00\x00\x00\x00"
      "\x00\xfe\x4f\xfd\x36\x8b\x3b\xdf\xff\x03\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\xc0\x34\x28\x92\x53\x83\xac\x4a\x17\x86\xe5\x56\x1a\x33\x49\xce"
      "\x26\x99\x2d\xb7\xdb\x48\xfe\x31\x2c\x9f\x64\x9f\x4e\x7a\x00\x00\x00\x00"
      "\x70\x0c\x9e\xdb\xce\x76\xd6\x73\x7e\x58\x7f\x54\x54\xf7\xfc\x2f\x55\xf7"
      "\xfd\x67\xf3\x6e\x56\xd2\xcf\x72\xfa\xe9\xa5\x9b\x3b\xd5\xb3\x80\xc1\x5d"
      "\x7f\x63\x6b\xb3\xd3\xdb\xda\xec\x3c\x28\xd3\xee\x7e\x7f\xfc\xdf\x43\x0d"
      "\xa3\xea\x31\x83\x67\x0f\x7b\x1f\x79\xbe\xda\xe2\xe2\x68\x8f\xc5\xfc\x2c"
      "\xbf\xc8\x95\xcc\xe5\x56\x56\xb3\x9c\x5f\x66\x29\xfd\x74\x33\x97\x9f\x56"
      "\xa5\xa5\x14\x69\xd5\x4f\x2f\x5a\xc3\x71\xee\x3d\xde\x9b\x8f\xd5\x6e\x7d"
      "\xd1\x58\x5f\xa9\x46\xd2\xcc\xdd\x2c\x57\x63\xbb\x9a\xdb\x79\x3b\xbd\xdc"
      "\x49\xa3\x9a\x43\xb5\xcd\xfe\x47\xfc\x4d\x19\x9d\xe2\x47\xb5\x03\xc6\xe8"
      "\x4e\x9d\x97\x33\xfa\x73\x9d\x4f\x87\x56\x15\x91\xd3\xa3\x88\xcc\xd7\xb1"
      "\x2f\xa3\xf1\xfc\xfe\x91\x38\xe4\x79\xf2\xe4\x91\xda\x69\x8c\x9e\x41\x5d"
      "\x78\x06\x31\x3f\x57\xe7\x65\xac\xff\x38\xd5\x31\x5f\x18\x3b\xfb\x5e\xda"
      "\x3f\x12\xc9\x77\xfe\xf3\xcf\x5b\xf7\x7a\x2b\xf7\xef\xdd\x5d\xbb\x32\x3d"
      "\x53\xfa\x92\x9e\x8c\x44\x67\x2c\x12\x2f\x7f\xa5\x22\x31\x5b\x47\x63\xb0"
      "\x8a\x1e\x6e\xb5\x7c\xb5\xda\xf7\x7c\x96\xf3\xf3\xbc\x9d\x3b\xe9\xe6\x7a"
      "\xe6\xd3\xce\x8d\xcc\xe7\x07\x59\x48\xe7\xb1\x33\xec\xe2\x01\xae\xb5\xc6"
      "\xe1\xae\xb5\x4b\xdf\xae\x0b\xcd\x24\x7f\xaa\xf3\xe9\x50\xc6\xf5\xf9\xb1"
      "\xb8\x8e\xaf\x74\xad\xaa\x6d\xfc\x9d\x9d\x28\xbd\x70\xf4\x2b\xd2\xcc\x37"
      "\xea\x42\x79\x8c\xdf\xd5\xf9\x74\x78\x32\x12\xed\xb1\x48\xbc\xb8\x7f\x24"
      "\xfe\xf2\xa8\x7c\x5d\xeb\xad\xdc\x5f\xbd\xb7\xf4\xce\x01\x8f\x77\xb9\xce"
      "\xcb\xcb\xf6\x0f\x53\xb5\x36\x97\xe7\xcb\x0b\xe5\x3f\x56\x55\x7b\xfc\xec"
      "\x28\xdb\x5e\xdc\xb3\xad\x5d\xb5\x5d\x18\xb5\x35\x76\xb5\x5d\x1c\xb5\x7d"
      "\xd1\x95\x3a\x5b\x7f\x86\xdb\xdd\xd3\xa0\xed\xe5\x3d\xdb\x3a\x55\xdb\x2b"
      "\x63\x6d\x7b\x7d\xca\x01\x60\xea\x9d\x7b\xed\xdc\x6c\xf3\x61\xf3\xdf\xcd"
      "\x8f\x9a\xbf\x6f\xde\x6b\xbe\x71\xf6\x27\x67\x6e\x9c\xf9\xe6\x6c\x4e\xff"
      "\x6b\xe6\xef\xa7\xfe\xd6\xf8\x6b\xe3\x87\xc5\x6b\xf9\x28\xbf\xde\xb9\xff"
      "\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\xbe\xbc\xb5\xf7\xde\xbf\xbf\xd4\xeb\x75\x57\x15"
      "\x14\x14\x14\x46\x85\x49\xaf\x4c\xc0\xb3\x76\xad\xff\xe0\x9d\x6b\x6b\xef"
      "\xbd\xff\xdd\xe5\x07\x4b\x6f\x75\xdf\xea\xae\x2c\xdc\xb8\x7e\xe3\x7a\xe7"
      "\xfb\xed\xef\x5d\xbb\xbb\xdc\xeb\xce\x0f\x5e\x27\x3d\x4a\xe0\x59\xd8\xf9"
      "\xa3\x3f\xe9\x91\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x07\x75\x1c\xff"
      "\x9d\x60\xd2\x73\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\x4e\xb6\xc5\x37\x73\x7a\x23\x45\xda\xf3"
      "\x57\xe7\xcb\xfa\xd6\x66\xa7\x57\xa6\x61\x79\x67\xcb\x99\x24\x8d\x24\xc5"
      "\xaf\x92\xe2\xb3\xe4\x66\x06\x29\xad\xb1\xee\x8a\xcf\x3b\xce\xeb\x0f\x3f"
      "\xf9\xf0\xf2\x07\x1f\x77\x76\xfa\x9a\x19\x6e\xdf\xd8\x6f\xbf\x83\xd9\xa8"
      "\x53\xe6\x92\x9c\xaa\xf3\xa3\xea\xef\xf6\x53\xf7\x57\x8c\x66\x58\x06\xec"
      "\xd2\x30\x70\x30\x69\xff\x0b\x00\x00\xff\xff\xad\x2c\x0e\xd9",
      1491);
  syz_mount_image(/*fs=*/0x20000600, /*dir=*/0x20000640, /*flags=*/0,
                  /*opts=*/0x20000300, /*chdir=*/1, /*size=*/0x5d3,
                  /*img=*/0x200006c0);
  memcpy((void*)0x200005c0, "./file0\000", 8);
  res = syscall(__NR_open, /*file=*/0x200005c0ul, /*flags=*/0ul, /*mode=*/0ul);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000000, "./file0\000", 8);
  syscall(__NR_unlinkat, /*fd=*/r[0], /*path=*/0x20000000ul,
          /*flags=*/0x6900000000000000ul);
  syz_clone(/*flags=CLONE_NEWTIME*/ 0x80, /*stack=*/0x20000040, /*stack_len=*/0,
            /*parentid=*/0, /*childtid=*/0, /*tls=*/0);
  {
    int i;
    for (i = 0; i < 32; i++) {
      syz_clone(/*flags=CLONE_NEWTIME*/ 0x80, /*stack=*/0x20000040,
                /*stack_len=*/0, /*parentid=*/0, /*childtid=*/0, /*tls=*/0);
    }
  }
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
  do_sandbox_none();
  return 0;
}
