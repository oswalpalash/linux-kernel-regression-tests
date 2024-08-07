// https://syzkaller.appspot.com/bug?id=047b4e27025bc84b6c12aaea1d4158b0f111c3ac
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
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

#include <linux/futex.h>
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

static void thread_start(void* (*fn)(void*), void* arg)
{
  pthread_t th;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 128 << 10);
  int i = 0;
  for (; i < 100; i++) {
    if (pthread_create(&th, &attr, fn, arg) == 0) {
      pthread_attr_destroy(&attr);
      return;
    }
    if (errno == EAGAIN) {
      usleep(50);
      continue;
    }
    break;
  }
  exit(1);
}

typedef struct {
  int state;
} event_t;

static void event_init(event_t* ev)
{
  ev->state = 0;
}

static void event_reset(event_t* ev)
{
  ev->state = 0;
}

static void event_set(event_t* ev)
{
  if (ev->state)
    exit(1);
  __atomic_store_n(&ev->state, 1, __ATOMIC_RELEASE);
  syscall(SYS_futex, &ev->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1000000);
}

static void event_wait(event_t* ev)
{
  while (!__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, 0);
}

static int event_isset(event_t* ev)
{
  return __atomic_load_n(&ev->state, __ATOMIC_ACQUIRE);
}

static int event_timedwait(event_t* ev, uint64_t timeout)
{
  uint64_t start = current_time_ms();
  uint64_t now = start;
  for (;;) {
    uint64_t remain = timeout - (now - start);
    struct timespec ts;
    ts.tv_sec = remain / 1000;
    ts.tv_nsec = (remain % 1000) * 1000 * 1000;
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, &ts);
    if (__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
      return 1;
    now = current_time_ms();
    if (now - start > timeout)
      return 0;
  }
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

static void setup_sysctl()
{
  char mypid[32];
  snprintf(mypid, sizeof(mypid), "%d", getpid());
  struct {
    const char* name;
    const char* data;
  } files[] = {
      {"/sys/kernel/debug/x86/nmi_longest_ns", "10000000000"},
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

struct thread_t {
  int created, call;
  event_t ready, done;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    event_wait(&th->ready);
    event_reset(&th->ready);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    event_set(&th->done);
  }
  return 0;
}

static void execute_one(void)
{
  int i, call, thread;
  for (call = 0; call < 6; call++) {
    for (thread = 0; thread < (int)(sizeof(threads) / sizeof(threads[0]));
         thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        event_init(&th->ready);
        event_init(&th->done);
        event_set(&th->done);
        thread_start(thr, th);
      }
      if (!event_isset(&th->done))
        continue;
      event_reset(&th->done);
      th->call = call;
      __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
      event_set(&th->ready);
      if (call == 0)
        break;
      event_timedwait(&th->done, 50 + (call == 0 ? 4000 : 0));
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
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

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    NONFAILING(memcpy((void*)0x20000040, "ext4\000", 5));
    NONFAILING(memcpy((void*)0x20000080, "./file1\000", 8));
    NONFAILING(memcpy(
        (void*)0x20000740,
        "\x78\x9c\xec\xdd\xdf\x6b\x1c\x5b\x1d\x00\xf0\xef\x4c\xb2\x35\x69\x53"
        "\x93\xaa\x0f\xb5\x60\x5b\x6c\x25\x2d\xda\x4d\xd2\xd8\x36\xf8\x50\x2b"
        "\x88\x7d\x2a\xa8\xf5\xbd\xc6\x64\x13\x42\x36\xd9\x90\xdd\xb4\x4d\x28"
        "\x92\xe2\x1f\x20\x88\xa8\xe0\x53\x9f\x7c\xf1\x3f\x10\xa4\x0f\xfe\x01"
        "\x22\x08\xfa\x2e\xfe\x44\xb4\xbd\xf7\xe1\x3e\xdc\x7b\xf7\xb2\xbb\xb3"
        "\xbd\x69\xba\x9b\x0d\xf7\x6e\x32\x25\xf9\x7c\x60\x3a\xe7\x9c\x99\xcd"
        "\xf7\x7b\xb6\xec\xec\xcc\x9c\xc3\x4e\x00\xc7\xd6\xc5\x88\xb8\x13\x11"
        "\x03\x11\x71\x35\x22\x46\xb3\xf6\x34\x5b\xee\x36\x2a\xdb\xad\xfd\x5e"
        "\xbe\x78\x32\xd7\x58\x92\xa8\xd7\xef\xff\x2f\x89\x24\x6b\x6b\xff\xad"
        "\x24\x5b\x9f\x6a\xbd\x24\x86\x22\xe2\xfb\x77\x23\x7e\x94\xbc\x19\xb7"
        "\xba\xb9\xb5\x3c\x5b\x2e\x97\xd6\xb3\xfa\x44\x6d\x65\x6d\xa2\xba\xb9"
        "\x75\x6d\x69\x65\x76\xb1\xb4\x58\x5a\x9d\x9e\x9e\xba\x39\x73\x6b\xe6"
        "\xc6\xcc\x64\x5f\xfa\x39\x16\x11\xb7\xbf\xfd\xcf\x5f\xfc\xf4\x37\xdf"
        "\xb9\xfd\xfb\xaf\x3d\xfa\xdb\x83\xff\x5c\xf9\x71\x23\xad\x91\x6c\xfb"
        "\xce\x7e\xf4\x53\xab\xeb\x85\xe6\x7b\xd1\x36\x18\x11\xeb\x07\x11\x2c"
        "\x07\x03\xd9\xba\xd0\x73\xcf\x3f\x1c\x78\x2e\x00\x00\xf4\xd6\x38\xc7"
        "\xff\x5c\x44\x7c\xb9\x79\xfe\x3f\x1a\x03\xcd\xb3\x53\x00\x00\x00\xe0"
        "\x28\xa9\x7f\x73\x24\xde\x4f\x22\xea\x00\x00\x00\xc0\x91\x95\x36\xe7"
        "\xc0\x26\x69\x31\x9b\x0b\x30\x12\x69\x5a\x2c\xb6\xe6\xf0\x7e\x21\x4e"
        "\xa6\xe5\x4a\xb5\xf6\xd5\x85\xca\xc6\xea\x7c\x6b\xae\xec\x58\x14\xd2"
        "\x85\xa5\x72\x69\x32\x9b\x2b\x3c\x16\x85\xa4\x51\x9f\xca\xe6\xd8\xb6"
        "\xeb\xd7\x77\xd5\xa7\x23\xe2\x4c\x44\xfc\x7c\x74\xb8\x59\x2f\xce\x55"
        "\xca\xf3\x79\xdf\xfc\x00\x00\x00\x80\x63\xe2\xd4\xae\xeb\xff\x77\x47"
        "\x5b\xd7\xff\x00\x00\x00\xc0\x11\x33\x96\x77\x02\x00\x00\x00\xc0\x81"
        "\x7b\xed\xfa\xff\x59\x7e\x79\x00\x00\x00\x00\x07\xc7\xf8\x3f\x00\x00"
        "\x00\x1c\x69\xdf\xbd\x77\xaf\xb1\xd4\xdb\xcf\xbf\x9e\x7f\xb8\xb9\xb1"
        "\x5c\x79\x78\x6d\xbe\x54\x5d\x2e\xae\x6c\xcc\x15\xe7\x2a\xeb\x6b\xc5"
        "\xc5\x4a\x65\xb1\xf9\x9b\x7d\x2b\xbd\xfe\x5e\xb9\x52\x59\xfb\x7a\xac"
        "\x6e\x3c\x9e\xa8\x95\xaa\xb5\x89\xea\xe6\xd6\x83\x95\xca\xc6\x6a\xed"
        "\xc1\xd2\x6b\x8f\xc0\x06\x00\x00\x00\x0e\xd1\x99\x0b\xcf\xff\x9a\x44"
        "\xc4\xf6\x37\x86\x9b\x4b\xc3\x89\xbc\x93\x02\x0e\x45\xd2\x6b\x87\x9d"
        "\x77\xed\xfe\x71\xb0\xb9\x00\x87\x6b\x20\xef\x04\x80\xdc\x0c\xe6\x9d"
        "\x00\x90\x9b\x42\xde\x09\x00\xb9\xeb\x75\x1f\xa0\xeb\xe4\x9d\x3f\xf6"
        "\x3f\x17\x00\x00\xe0\x60\x8c\x7f\xf1\xd5\xf8\xff\x70\xbb\xad\x3d\xfe"
        "\xdf\xfb\xde\x40\xcf\xd1\x43\xe0\x2d\x96\xe6\x9d\x00\x00\x70\xe8\x8c"
        "\xff\xc3\xf1\x55\x30\x03\x10\x8e\xbd\xcf\xee\x6e\x48\x22\xb6\x77\x54"
        "\x3f\xfd\xf8\x7f\xbd\xfe\x49\xf2\x02\x00\x00\xfa\x67\xa4\xb9\x24\x69"
        "\x31\x1b\x0b\x1c\x89\x34\x2d\x16\x23\x4e\x37\x1f\x0b\x50\x48\x16\x96"
        "\xca\xa5\xc9\xec\xfa\xe0\x2f\xa3\x85\xcf\x34\xea\x53\xcd\x57\x26\x46"
        "\xff\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x60\x9f\xea\xf5\x24\xea\x00\x00\x00\xc0\x91\x16\x91\xfe"
        "\x3b\xc9\x9e\xe4\x3f\x3e\x7a\x79\x64\xf7\xfd\x81\x13\xc9\x7b\xa3\xcd"
        "\x75\x44\x3c\xfa\xf5\xfd\x5f\x3e\x9e\xad\xd5\xd6\xa7\x1a\xed\xff\x7f"
        "\xd5\x5e\xfb\x55\xd6\x7e\xdd\xf3\xc4\x01\x00\x00\xe0\x6d\xd0\xbe\x4e"
        "\x6f\x5f\xc7\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x40\x3f\xbd\x7c\xf1\x64\xae\xbd\xbc\xb1"
        "\xf1\xc4\xc1\xc5\xfd\xef\xb7\x22\x62\xac\x53\xfc\xc1\x18\x6a\xae\x87"
        "\xa2\x10\x11\x27\xdf\x49\x62\x70\xc7\xeb\x92\x88\x18\xe8\x43\xfc\xed"
        "\xa7\x11\x71\xb6\x53\xfc\xa4\x91\x56\x8c\x65\x59\x74\x8a\x3f\x9c\x63"
        "\xfc\x34\x22\x4e\xf5\x21\x3e\x1c\x67\xcf\x1b\xc7\x9f\x3b\x9d\x3e\x7f"
        "\x69\x5c\x6c\xae\x3b\x7f\xfe\x1a\xe5\x7f\xf5\x21\x7e\xf7\xe3\x5f\xfa"
        "\xea\xf8\x37\xd0\xe5\xf8\x73\x7a\x9f\x31\xce\x6d\xef\x11\xff\x69\xc4"
        "\xb9\xc1\xce\xc7\x9f\x76\xfc\xa4\x4b\xfc\x4b\xfb\x8c\xff\xc3\x1f\x6c"
        "\x6d\x75\xdb\x56\x7f\x16\x31\xde\xf1\xfb\x27\x79\x2d\xd6\x44\x6d\x65"
        "\x6d\xa2\xba\xb9\x75\x6d\x69\x65\x76\xb1\xb4\x58\x5a\x9d\x9e\x9e\xba"
        "\x39\x73\x6b\xe6\xc6\xcc\xe4\xc4\xc2\x52\xb9\x94\xfd\xdb\x31\xc6\xcf"
        "\xbe\xf4\xbb\x0f\xf7\xea\xff\xc9\x2e\xf1\xc7\x5a\xfd\xbf\xd0\xad\xff"
        "\x97\xf7\xd9\xff\x0f\xfe\xfc\xf8\xc5\xe7\x5b\xc5\x42\xa7\xf8\x57\x2e"
        "\x75\xfe\xfe\x3d\xdb\x8a\xff\xc6\xfb\x9f\x66\xdf\x7d\x5f\xc9\xca\x8d"
        "\xed\xe3\xed\xf2\x76\xab\xbc\xd3\xf9\xdf\xfe\xe9\xfc\x5e\xfd\x9f\xef"
        "\xd2\xff\x5e\xff\xff\x57\xf6\xd9\xff\xab\xdf\xfb\xc9\xdf\xf7\xb9\x2b"
        "\x00\x70\x08\xaa\x9b\x5b\xcb\xb3\xe5\x72\x69\x5d\xa1\x73\xa1\x5e\xf7"
        "\x46\x29\x1c\xc9\x42\x0c\xed\xb5\x4f\xde\x47\x26\x00\x00\xa0\xdf\x3e"
        "\x3e\xe9\xcf\x3b\x13\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x38\xbe\x0e\xe3\x97\xc6\x76\xc7\xdc\xe3\xe7\xa8\x01\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x72\xf3\x51\x00\x00\x00\xff\xff\x2b"
        "\xe2\xd3\xc7",
        1244));
    NONFAILING(syz_mount_image(/*fs=*/0x20000040, /*dir=*/0x20000080,
                               /*flags=*/0x4500, /*opts=*/0x200000c0,
                               /*chdir=*/0xe, /*size=*/0x4dc,
                               /*img=*/0x20000740));
    break;
  case 1:
    NONFAILING(memcpy((void*)0x20000080, "./bus\000", 6));
    syscall(__NR_open, /*file=*/0x20000080ul, /*flags=*/0x1e1142ul,
            /*mode=*/0ul);
    break;
  case 2:
    NONFAILING(memcpy((void*)0x20001280, "/dev/loop", 9));
    NONFAILING(*(uint8_t*)0x20001289 = 0x30);
    NONFAILING(*(uint8_t*)0x2000128a = 0);
    NONFAILING(memcpy((void*)0x20001240, "./bus\000", 6));
    syscall(__NR_mount, /*src=*/0x20001280ul, /*dst=*/0x20001240ul,
            /*type=*/0ul, /*flags=*/0x1000ul, /*data=*/0ul);
    break;
  case 3:
    NONFAILING(memcpy((void*)0x20000400, "./bus\000", 6));
    res = syscall(__NR_open, /*file=*/0x20000400ul, /*flags=*/0x14103eul,
                  /*mode=*/0ul);
    if (res != -1)
      r[0] = res;
    break;
  case 4:
    NONFAILING(memcpy((void*)0x20000100, "1000000\000", 8));
    syscall(__NR_write, /*fd=*/r[0], /*val=*/0x20000100ul, /*len=*/8ul);
    break;
  case 5:
    NONFAILING(memcpy(
        (void*)0x20000c40,
        "\x23\x21\x20\x2e\x2f\x66\x69\x6c\x65\x31\x0a\x82\x6a\x3f\xc6\x7c\x3a"
        "\xab\x7d\x7e\x31\x38\xd5\xcc\x25\x19\xa6\xa8\xf7\xa0\x0b\xff\x3b\xee"
        "\x49\xbb\x93\x22\x1f\x4e\x80\x95\x34\x94\x23\x35\x8b\x24\xc3\x46\x99"
        "\x05\xf2\x81\x89\x7d\xc6\xe1\x7a\x55\x07\xba\x1b\x9e\x33\xcc\x8e\x4f"
        "\x28\xf6\xda\xe3\x4d\x29\x86\xca\x00\x9c\x79\xab\x46\x1e\x69\xfd\x74"
        "\xcc\xe8\xc9\xed\x5b\x91\x09\xf3\xfa\x42\xf4\x3d\x39\x82\xeb\x88\x19"
        "\x60\x25\x7e\x2c\xd3\xd2\x82\x1c\xc7\xf5\x80\xf2\x74\x9a\x70\x11\xd4"
        "\x8d\xa6\x46\xa5\xc0\xf2\xc0\x65\xde\x39\x6a\x55\xb1\xc0\x7d\xb5\x4c"
        "\x9d\xf5\x9d\x0a\x0d\xb7\x18\xa0\x86\xb4\x64\x51\x13\x19\x69\x16\x69"
        "\x1b\x0b\x09\x28\xa7\x00\xff\x9b\x12\xfd\x85\x58\x46\x1f\x93\x09\x96"
        "\x1d\xbb\x6a\x69\x13\x14\x89\x8c\xfd\xb1\xfc\xc5\x1b\x91\x2d\x77\x22"
        "\x8c\x1e\xa4\x43\x4e\x69\x24\x2c\xdd\x53\x59\x03\xa7\xcd\x7a\x57\xd4"
        "\xca\x13\x89\x2c\x04\x05\xb1\xb9\x50\x72\x3d\x18\xd4\x3e\x76\x09\x93"
        "\x99\x8a\xfe\x53\x22\x7a\xc0\xe0\xe8\x6d\x24\x4c\xc5\x51\x92\xce\x11"
        "\x50\x12\xfe\x3f\x36\xef\x52\x9b\x66\xdc\xf4\x62\x54\x4a\xba\xc1\xb2"
        "\x5e\x66\x9d\x6f\x85\x48\xd3\xef\x5e\xd9\xb2\xa3\x3c\xc6\x67\xd1\x03"
        "\x40\x14\x55\xb9\x64\x2b\xb6\xe6\xec\xe1\x37\xcf\x22\xe2\x1c\x49\x43"
        "\xa3\x91\x9c\xe3\x29\xb6\x49\x28\x5b\x74\x6b\xf0\x74\x06\x04\xd7\xe4"
        "\x65\xee\x76\x09\x59\x7a\xc3\x60\xb0\x80\xfa\x16\xb3\x5f\x08\x46\x6e"
        "\xec\xbf\xa3\x5d\x0c\x37\x1b\x0d\x3f\x23\x74\xda\xd8\xfd\x4a\xb7\x77"
        "\x5d\x14\x62\x70\xfb\x31\x4d\xd1\xd4\xde\x55\xda\x46\x88\x41\x33\x28"
        "\xe9\xf2\x13\x12\xcf\x32\x1b\x55\x62\xa7\x55\xe3\x30\x8b\x5b\xc6\x19"
        "\xbd\xf1\x39\xb8\x9e\xf7\xa0\xb8\x2b\x44\x36\xf6\xe7\x5f\xc2\x2b\x61"
        "\x92\x1a\xde\xf4\x12\xcb\x27\xcd\xeb\x68\x3f\x70\xc1\xc6\x10\x21\x7f"
        "\xbe\x12\x77\xdd\x0b\xa0\x25\xcd\x5a\x0d\x5d\x9b\xca\xeb\xd6\xbd\x3f"
        "\xe1\x6d\x72\xe8\xc7\x78\xc4\xb6\x12\xb0\x0c\xbf\xec\x94\x71\x0f\x05"
        "\x20\xf3\xe3\x04\xd4\x4c\x51\xbe\x5e\x33\xc2\x63\x30\x72\x91\xd4\x51"
        "\x34\x84\x74\x0f\x38\x98\x95\x6c\x48\x44\xb6\x05\xda\xeb\x8d\xbc\xb5"
        "\x1e\xdb\x30\xd5\xb9\x13\x8a\x8c\x79\xb3\xa8\xe7\x80\x97\xa0\x73\x77"
        "\x3a\x6f\xfb\xe3\x51\x1d\x2d\xea\xf0\x52\xf8\x58\x89\x17\x64\x11\xbc"
        "\xeb\xf3\xa5\x66\x94\x17\x6e\x4f\x22\x8b\x53\x7e\x27\x1e\x45\x12\xb5"
        "\xbc\xe9\xf2\x29\x27\xd0\xac\x27\x68\xdb\x31\xc5\x9d\xf2\xce\x48\xc7"
        "\x07\x7d\xcf\xc5\xc5\x5f\xe8\xa1\x56\x63\xe4\xcd\x26\x5a\xea\x7a\x3f"
        "\x4f\x4e\x93\x43\xd8\x6e\x86\xbd\xb6\xa9\x00\x31\xfe\xbc\xad\x5b\x8d"
        "\xfd\x62\x81\x46\x41\x64\x67\x1d\x99\xb1\x88\x03\x52\x97\x5e\xb5\xd2"
        "\x1c\xcf\x68\xb5\x7c\xeb\x83\x2b\x2d\x94\x00\x9b\x44\x6f\xe1\x93\x44"
        "\x66\x25\xf7\x95\x68\x95\xb4\xb8\x11\x9a\x5a\x8f\x86\xc3\x06\x72\x6b"
        "\xfe\xb5\x18\x2c\xb8\x77\x29\xbb\x61\x62\xe2\x43\x2d\xec\x1f\x7e\xe6"
        "\x67\x3d\x86\x6d\x4c\xc1\x2c\x2b\xdd\xee\x8a\xcf\xfc\xd9\xe0\x4b\xa8"
        "\x28\x73\x95\x33\xfb\xcc\xe7\xc5\x01\xb8\xf6\x8e\x98\xb7\xa1\xf3\x6e"
        "\xa8\x44\xdc\x77\x79\x84\x4c\x6c\xec\xc3\x0b\xd1\x68\xb2\xb4\xc3\x19"
        "\xd1\xeb\x61\x6d\x2b\xbc\xb5\xfd\x39\xfe\x38\x25\x1f\x11\x12\x9e\xe3"
        "\xd8\xa3\x0f\xaa\xf9\x55\x3e\xc6\x59\xcf\x45\x96\xf0\xd6\xb8\x36\x46"
        "\x66\xa0\x71\xc4\x02\x95\x43\xf4\xea\x52\x48\xa2\xed\x46\x2c\xc6\xfa"
        "\x02\xa3\xd0\x89\x98\xc9\x6d\x36\x7c\xb7\x89\x1e\x64\x9e\xa5\xe2\xc9"
        "\x12\x03\x8c\xa0\xc8\x1b\x77\x3a\x0a\x0d\xe0\xd7\x75\x73\x0a\x84\x5b"
        "\x8e\x76\x45\x81\xce\x25\xd1\x74\x7f\xa2\x18\x91\xdd\x8a\xe0\x4f\x03"
        "\x75\x0a\x45\xc3\x88\x61\x1f\xe0\x16\xd0\x7c\x81\xd3\x54\xf5\xe3\x33"
        "\x06\x8f\xb0\x7c\xfb\x4d\xcc\xb9\x37\x1f\x38\x03\x77\xbc\xdf\xcc\x6c"
        "\x77\x66\xb1\x58\x3d\x9d\xba\x0f\x2f\x7b\xb1\xfd\xbb\x4c\x0f\x48\x77"
        "\x11\x81\x11\xbb\xed\x91\x7c\x50\xfd\xe8\x23\x36\xc7\xbb\x68\x54\x72"
        "\x3b\x61\xb9\x34\xce\x3b\x1a\xef\xbf\xc6\x8e\x2f\x8f\xc0\xf1\x94\x66"
        "\x03\xdc\x58\xc5\x9a\xf7\x6f\x1d\x0b\xe4\x77\x9f\x64\xd7\x9a\x2d\xe4"
        "\x9e\x33\x18\x60\xea\x9c\x22\x47\xdd\x74\xa1\x24\x67\x9e\xc2\x66\x39"
        "\xc6\xa8\x1c\x62\x29\xd7\x61\x2b\x9d\x36\xee\x47\xd8\xe0\x8a\xe6\x35"
        "\xf9\xbc\xbb\x7d\x3e\x58\xdf\x44\x81\xd4\x57\x6f\xa6\xbf\xc2\x4b\x70"
        "\x6e\xec\xad\x1a\x12\xef\xa0\xf6\x15\xb0\x56\xce\x5f\x92\xf4\x2e\xf6"
        "\x33\xba\x5a\xc6\xc6\x92\x4c\xec\x40\xc4\x14\x80\xa0\xe6\x89\x94\xc5"
        "\x51\xe2\x10\x4a\xcb\x3c\xdb\x78\x5b\x3a\xef\x30\x4c\x25\x25\x1b\xfa"
        "\xa6\x28\x0b\xb5\xac\x48\x60\x6b\xc2\xd6\xbb\x87\x12\xae\x0d\x3c\xe7"
        "\x79\xc0\xb5\x47\x66\xa3\x6a\x93\x04\xd7\xc4\xe7\xb9\x2e\x89\x7b\xa0"
        "\x09\xf4\x70\xff\x88\x90\x93\x3b\xf0\x1c\xbd\xed\x96\x61\x22\xfa\x06"
        "\xb3\xf6\x2a\xa3\x27\x46\xf6\xcd\x8d\xc2\x6b\x2d\xaf\x8e\x67\x4b\x9f"
        "\x3f\xb1\xe6\x31\x66\x84\xc4\x40\xad\xc5\x29\x78\x8c\xa2\x4a\x03\xbb"
        "\x6d\x61\x2a\x22\xa3\xb8\x8b\x6b\xc9\xa6\x7f\xd0\x4d\x6a\x74\xd3\x62"
        "\xea\xbc\x58\xdd\x58\xc4\xb1\xea\xbc\xdb\xa5\x71\x4c\x40\x2d\x01\x48"
        "\x75\x9b\x8e\x1d\x59\x3c\x4f\x3a\x9a\x88\x40\xe1\x6f\xbb\x28\x25\x26"
        "\xad\x4d\x6c\x88\x0e\xa6\x01\xd5\x59\xef\xfe\x5c\x47\x77\x54\xac\x61"
        "\x1c\x87\x49\xf9\xff\x7f\x56\xcd\xf7\x92\xd4\x51\x40\x20\x45\x97\xe4"
        "\xff\x08\x03\x3a\x08\x13\x70\xe2\xe0\x15\x03\xbc\x48\xd6\x52\xaa\xf2"
        "\x76\xe3\xae\xbd\xc1\x11\x69\x8c",
        1198));
    syscall(__NR_write, /*fd=*/r[0], /*data=*/0x20000c40ul, /*len=*/0x415ul);
    break;
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
  setup_sysctl();
  install_segv_handler();
  loop();
  return 0;
}
