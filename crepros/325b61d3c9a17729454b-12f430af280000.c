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
  memcpy((void*)0x20000380, "force", 5);
  *(uint8_t*)0x20000385 = 0x2c;
  memcpy((void*)0x20000386, "umask", 5);
  *(uint8_t*)0x2000038b = 0x3d;
  sprintf((char*)0x2000038c, "%023llo", (long long)4);
  *(uint8_t*)0x200003a3 = 0x2c;
  memcpy((void*)0x200003a4, "umask", 5);
  *(uint8_t*)0x200003a9 = 0x3d;
  sprintf((char*)0x200003aa, "%023llo", (long long)7);
  *(uint8_t*)0x200003c1 = 0x2c;
  *(uint8_t*)0x200003c2 = 0;
  memcpy(
      (void*)0x20000c80,
      "\x78\x9c\xec\xdd\xcd\x6b\x1c\xe7\x1d\x07\xf0\xef\xac\x57\xb2\xe5\x82\xa3"
      "\x24\x76\xe2\x96\x40\x45\x0c\x69\xa9\xa8\x2d\x69\x51\x5a\xf5\x52\xb7\x94"
      "\xa2\x43\x28\x21\x3d\xf4\x2c\xec\x75\xbc\x78\xad\x04\x69\x53\x94\x50\x8a"
      "\xfb\x7e\xed\x21\x7f\x40\x7a\xd0\xad\xa7\x42\xef\x86\xf4\xdc\xde\x72\xd5"
      "\x31\x50\xe8\x25\x27\xdd\x5c\x66\x76\x56\xda\x58\xb2\xb2\xf2\x8b\x76\x15"
      "\x7f\x3e\xe2\x99\xe7\x79\xe6\x99\x79\xe6\x99\xdf\xbc\xec\xce\x2e\x62\x03"
      "\x3c\xb7\x56\xe7\xd3\xbc\x9f\x22\xab\xf3\x6f\x6d\x95\xf5\x9d\xed\x56\x77"
      "\x67\xbb\x75\xb6\x6e\xee\x26\x29\xcb\x8d\xa4\xd9\xcf\x52\xac\x27\xc5\xa7"
      "\xc9\xf5\xf4\x53\xbe\x59\xce\xac\x97\x2f\x1e\xb5\x9d\x8f\x3b\x2b\xef\x7c"
      "\xf6\xc5\xce\xe7\xfd\x5a\xb3\x4e\xd5\xf2\x8d\xa3\xd6\x1b\xcd\xbd\x3a\x65"
      "\x2e\xc9\x99\x3a\x3f\x68\xea\xb1\xfa\xbb\xf1\xc8\xfe\x46\x55\xec\xed\x61"
      "\x19\xb0\x2b\x83\xc0\xc1\xb8\x3d\x38\xe0\xde\x71\x56\x7f\xc2\xeb\x16\x98"
      "\x04\x45\xff\x75\xf3\x80\xd9\xe4\x7c\x92\x73\xf5\xfb\x80\xd4\x77\x87\xc6"
      "\xc9\x8e\xee\xe9\x3b\xd6\x5d\x0e\x00\x00\x00\x4e\xa9\x17\x76\xb3\x9b\xad"
      "\x5c\x18\xf7\x38\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xe0\x34\xa9\x7f\xff"
      "\xbf\xa8\x53\x63\x50\x9e\x4b\x31\xf8\xfd\xff\xe9\x7a\x5e\xea\xf2\xa9\x76"
      "\x7f\xdc\x03\x00\x00\x00\x00\x00\x00\x00\x80\xa7\xe0\xdb\xbb\xd9\xcd\x56"
      "\x2e\x0c\xea\x0f\x8a\xea\x3b\xff\xd7\xab\xca\xc5\x6a\xfa\x8d\x7c\x90\xcd"
      "\xb4\xb3\x91\xab\xd9\xca\x5a\x7a\xe9\x65\x23\x8b\x49\x66\x87\x3a\x9a\xde"
      "\x5a\xeb\xf5\x36\x16\x47\x58\x73\xe9\xd0\x35\x97\x4e\x66\x7f\x01\x00\x00"
      "\x00\x00\x00\x00\xe0\x6b\xea\xf7\x59\xdd\xff\xfe\x1f\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x26\x41\x91\x9c\xe9\x67\x55\xba\x38\x28\xcf\xa6"
      "\xd1\x4c\x72\x2e\xc9\x74\xb9\xdc\xbd\xe4\x3f\x83\xf2\x69\x76\x7f\xdc\x03"
      "\x00\x00\x00\x80\x13\xf0\xc2\x6e\x76\xb3\x95\x0b\x83\xfa\x83\xa2\x7a\xe6"
      "\x7f\xa5\x7a\xee\x3f\x97\x0f\xb2\x9e\x5e\x3a\xe9\xa5\x9b\x76\x6e\x56\x9f"
      "\x05\xf4\x9f\xfa\x1b\x3b\xdb\xad\xee\xce\x76\xeb\x6e\x99\x0e\xf6\xfb\x93"
      "\xff\x1d\x6b\x18\x55\x8f\xe9\x7f\xf6\x70\xf8\x96\x2f\x57\x4b\xcc\xe4\x56"
      "\x3a\xd5\x9c\xab\xb9\x91\xf7\xd2\xcd\xcd\x34\xaa\x35\x4b\x97\x07\xe3\x39"
      "\x7c\x5c\xbf\x2b\xc7\x54\xfc\xb8\x36\xe2\xc8\x6e\xd6\x79\xb9\xe7\x7f\xad"
      "\xf3\xc9\x30\x5b\x45\x64\x6a\x2f\x22\x0b\xf5\xd8\xca\x68\xbc\x78\x74\x24"
      "\x8e\x79\x74\x1e\xde\xd2\x62\x1a\x7b\x9f\xfc\x5c\x7c\x06\x31\x3f\x5f\xe7"
      "\xe5\xfe\xfc\x79\xa2\x63\xbe\x34\x74\xf6\xbd\x72\x74\x24\x92\xef\xfc\xf3"
      "\xef\xbf\xba\xdd\x5d\xbf\x73\xfb\xd6\xe6\xfc\xe4\xec\xd2\x63\x7a\x38\x12"
      "\xad\xa1\x48\xbc\xfa\x5c\x45\x62\xa1\x8a\xc4\xa5\xbd\xfa\x6a\x7e\x9e\x5f"
      "\x66\x3e\x73\x79\x3b\x1b\xe9\xe4\xd7\x59\x4b\x2f\xed\xcc\xe5\x67\x55\x69"
      "\xad\x3e\x9f\xcb\xe9\xec\xd1\x91\xba\xfe\xa5\xda\xdb\x5f\x35\x92\xe9\xfa"
      "\xb8\xf4\xef\xa2\xc7\x1b\xd3\xeb\xd5\xba\x17\xd2\xc9\x2f\xf2\x5e\x6e\xa6"
      "\x9d\x37\xab\xbf\xa5\x2c\xe6\x07\x59\xce\x72\x56\x86\x8e\xf0\xa5\x11\xae"
      "\xfa\xc6\xf1\xae\xfa\x2b\xdf\xad\x0b\x33\x49\xfe\x52\xe7\x93\xa1\x8c\xeb"
      "\x8b\x43\x71\x1d\xbe\xe7\xce\x56\x6d\xc3\x73\xf6\xa3\xf4\xd2\xd3\xbf\x37"
      "\x36\xbf\x55\x17\xca\x6d\xfc\xa1\xce\x27\xc3\xc3\x91\x58\x1c\x8a\xc4\xcb"
      "\x47\x47\xe2\x6f\x0f\xca\xe9\x66\x77\xfd\xce\xc6\xed\xb5\xf7\x47\xdc\xde"
      "\x1b\x75\x5e\x5e\x47\x7f\x9a\xa8\x57\x89\xf2\x7c\x79\xa9\x3c\x58\x55\xed"
      "\xcb\x67\x47\xd9\xf6\xf2\xa1\x6d\x8b\x55\xdb\xc5\xbd\xb6\xc6\x81\xb6\x4b"
      "\x7b\x6d\x5f\x75\xa5\x4e\xd7\xef\xe1\x0e\xf6\xb4\x54\xb5\xbd\x7a\x68\x5b"
      "\xab\x6a\xbb\x3c\xd4\x76\xd8\xfb\x2d\x00\x26\xde\xf9\xef\x9d\x9f\x9e\xf9"
      "\xef\xcc\xbf\x67\x3e\x99\xf9\xe3\xcc\xed\x99\xb7\xce\xfd\xf4\xec\x0f\xcf"
      "\xbe\x36\x9d\xa9\x7f\x4d\xfd\xa8\xb9\x70\xe6\x8d\xc6\x6b\xc5\x3f\xf2\x49"
      "\x7e\xbb\xff\xfc\x0f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x3c\xbe\xcd\x0f\x3f\xba\xb3\xd6"
      "\xed\xb6\x37\xc6\x5c\x28\xea\x1f\xf2\x99\x94\xf1\x28\x28\x3c\xdf\x85\x71"
      "\xdf\x99\x80\x67\xed\x5a\xef\xee\xfb\xd7\x36\x3f\xfc\xe8\xfb\x9d\xbb\x6b"
      "\xef\xb6\xdf\x6d\xaf\x4f\x2d\x2f\xaf\x2c\xac\x2c\xbf\xd9\xba\x76\xab\xd3"
      "\x6d\x2f\xf4\xa7\xe3\x1e\x25\xf0\x2c\xec\xbf\xe8\x8f\x7b\x24\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\xc0\xa8\x4e\xe2\xdf\x09\xc6\xbd\x8f\x00\x00\x00"
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
      "\xc0\xe9\xb6\x3a\x9f\xe6\xfd\x14\x59\x5c\xb8\xba\x50\xd6\x77\xb6\x5b\xdd"
      "\x32\x0d\xca\xfb\x4b\x36\x93\x34\x92\x14\xbf\x49\x8a\x4f\x93\xeb\xe9\xa7"
      "\xcc\x0e\x75\x57\x3c\x6a\x3b\x1f\x77\x56\xde\xf9\xec\x8b\x9d\xcf\xf7\xfb"
      "\x6a\x0e\x96\x6f\x1c\xb5\xde\x68\xee\xd5\x29\x73\x49\xce\xd4\xf9\xd3\xea"
      "\xef\xc6\x13\xf7\x57\xec\xed\x61\x19\xb0\x2b\x83\xc0\xc1\xb8\xfd\x3f\x00"
      "\x00\xff\xff\x67\x49\x09\xf4",
      1501);
  syz_mount_image(/*fs=*/0x20000600, /*dir=*/0x20000640, /*flags=*/0xa00010,
                  /*opts=*/0x20000380, /*chdir=*/3, /*size=*/0x5dd,
                  /*img=*/0x20000c80);
  memcpy((void*)0x20021000, "./file0\000", 8);
  res = syscall(__NR_open, /*file=*/0x20021000ul, /*flags=*/0ul, /*mode=*/0ul);
  if (res != -1)
    r[0] = res;
  syscall(__NR_fchdir, /*fd=*/r[0]);
  memcpy((void*)0x200001c0, "./file0\000", 8);
  syscall(__NR_creat, /*file=*/0x200001c0ul, /*mode=*/0xc0ul);
  syz_clone(/*flags=*/0, /*stack=*/0x20000000, /*stack_len=*/0, /*parentid=*/0,
            /*childtid=*/0, /*tls=*/0);
  {
    int i;
    for (i = 0; i < 64; i++) {
      syz_clone(/*flags=*/0, /*stack=*/0x20000000, /*stack_len=*/0,
                /*parentid=*/0, /*childtid=*/0, /*tls=*/0);
    }
  }
}
int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul, /*prot=*/7ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  do_sandbox_none();
  return 0;
}
