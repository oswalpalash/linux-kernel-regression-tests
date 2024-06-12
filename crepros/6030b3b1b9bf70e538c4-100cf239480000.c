// https://syzkaller.appspot.com/bug?id=82148e8173e51abf3a46e231a7e233950c815ca7
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
  for (call = 0; call < 4; call++) {
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
      event_timedwait(&th->done,
                      50 + (call == 0 ? 4000 : 0) + (call == 2 ? 4000 : 0));
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

void execute_call(int call)
{
  switch (call) {
  case 0:
    memcpy((void*)0x20000080, "hfsplus\000", 8);
    memcpy((void*)0x200006c0, "./file0\000", 8);
    memcpy((void*)0x200000c0, "type", 4);
    *(uint8_t*)0x200000c4 = 0x3d;
    memcpy((void*)0x200000c5, "\xc5\x0c\xb8\xcf", 4);
    *(uint8_t*)0x200000c9 = 0x2c;
    memcpy((void*)0x200000ca, "gid", 3);
    *(uint8_t*)0x200000cd = 0x3d;
    sprintf((char*)0x200000ce, "0x%016llx", (long long)0);
    *(uint8_t*)0x200000e0 = 0x2c;
    memcpy((void*)0x200000e1, "decompose", 9);
    *(uint8_t*)0x200000ea = 0x2c;
    memcpy((void*)0x200000eb, "nls", 3);
    *(uint8_t*)0x200000ee = 0x3d;
    memcpy((void*)0x200000ef, "default", 7);
    *(uint8_t*)0x200000f6 = 0x2c;
    *(uint8_t*)0x200000f7 = 0;
    memcpy(
        (void*)0x20004e00,
        "\x78\x9c\xec\xdd\x4f\x6f\x1c\x67\x1d\x07\xf0\xef\x6e\xe2\xcd\xae\x91"
        "\x12\x27\x4d\xda\x06\x21\x61\x15\xa9\x42\x8d\x48\xfc\x27\x2d\x46\x42"
        "\x02\x4a\x41\x16\x54\x50\x89\x4b\xa5\xaa\x42\x16\xb1\x1b\x2b\x9b\xb4"
        "\xb2\x5d\xe4\xf6\x00\x06\xf1\x0a\x78\x05\x05\x64\x0e\x3d\x71\xe0\x84"
        "\x40\xca\x81\x23\xe2\x2d\x18\xf5\x88\xc4\x29\x07\x7c\x33\x9a\xd9\x59"
        "\x7b\x9d\x6c\x1c\xaf\x6d\xbc\x9b\xf4\xf3\x91\x66\xe7\xf7\xcc\x33\xf3"
        "\xcc\x6f\x7e\x9e\x1d\xcd\xec\xca\xda\x00\x9f\x5b\xf3\x6f\x67\x6c\x23"
        "\xb5\xcc\x5f\x7b\x73\xbd\x68\x6f\x6d\xce\xb6\xb7\x36\x67\xef\x76\xe3"
        "\x24\xe7\x92\xd4\x93\x66\x92\x5a\xb1\xf8\xcf\x49\x3e\x4b\x36\xd2\x99"
        "\x72\xb5\xdb\xd1\x33\x7f\xc4\xbb\xe3\x8d\x07\x9f\xbe\x73\x75\xa9\xd3"
        "\x6a\x56\x53\xb9\x7e\xad\xef\x76\x3f\xbe\x30\xc0\x51\xec\xe6\x32\xd1"
        "\xc9\xb5\x9c\x1f\xc3\xbe\xf1\x66\xaa\x31\x8f\xae\x38\xba\x33\x77\xba"
        "\xad\x4b\x49\x26\x8f\x97\x1f\x9c\x8c\x9d\xae\x7f\xf5\xed\x7e\xec\xfb"
        "\x19\x00\x78\xfa\xd5\x92\x33\xfd\x96\x4f\x24\xe3\xd5\xcd\x7a\xf1\x1c"
        "\xd0\xb9\x2b\x3e\xee\xfd\xf0\x08\xd8\x18\x76\x02\x00\x00\x00\x70\x0a"
        "\x2e\x6c\x67\x3b\xeb\x39\x3f\xec\x3c\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\xe0\x69\x52\xfd\xfe\x7f\xad\x9a\xea\xdd\x78\x32\xb5\x34\x3b"
        "\xbf\xff\xdf\xa8\x96\xa5\x8a\x9f\x6a\xf7\x87\x9d\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x9c\x80\x2f\x6f\x67\x3b\xeb\x39\xdf\x6d\xef\xd4\xca\xef"
        "\xfc\x5f\x2a\x1b\x97\xcb\xd7\x2f\xe4\xc3\xac\x66\x31\x2b\xb9\x9e\xf5"
        "\x2c\x64\x2d\x6b\x59\xc9\x74\x92\x89\x9e\x81\x1a\xeb\x0b\x6b\x6b\x2b"
        "\xd3\x87\xd8\x72\xa6\xef\x96\x33\xa7\x73\xbc\x00\x00\x00\x00\x00\x00"
        "\x00\xf0\x8c\xfa\x75\xe6\xf7\xbe\xff\x07\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x80\x51\x50\x4b\xce\x74\x66\xe5\x74\xb9\x1b\x4f"
        "\xa4\x7e\x36\x49\x33\x49\xa3\x58\x6f\x23\xf9\x6b\x37\x1e\x29\xad\xc1"
        "\x56\xbf\xff\xff\xca\x03\x00\x00\x00\x46\xc8\x85\xed\x6c\x67\x3d\xe7"
        "\xbb\xed\x9d\x5a\xf9\xcc\xff\x7c\xf9\xdc\xdf\xcc\x87\xb9\x97\xb5\x2c"
        "\x67\x2d\xed\x2c\xe6\x56\xf9\x59\x40\xe7\xa9\xbf\xbe\xb5\x39\xdb\xde"
        "\xda\x9c\xbd\x5b\x4c\x8f\x8e\xfb\x9d\xff\x0c\x94\x46\x39\x62\x3a\x9f"
        "\x3d\xf4\xdf\xf3\x54\xb9\xc6\x95\xdd\x2d\xe6\xf3\xfd\xfc\x24\xd7\x32"
        "\x99\xb7\xb2\x92\xe5\xfc\x3c\x0b\x59\xcb\x62\x26\xf3\x46\x19\x2d\xa4"
        "\x96\xdf\x57\x9f\x5e\x4c\x74\xf3\xec\x9f\xef\xb7\xf7\xb5\xde\x4a\xf2"
        "\xcb\x83\x72\x7d\xb1\xcc\xa4\x95\xa5\x2c\x97\xb9\x5d\xcf\xcf\xf2\x7e"
        "\xda\xb9\x95\x7a\x79\x0c\xe5\x3a\x07\xef\xf1\x57\x45\x75\x6a\xdf\xaa"
        "\x1c\xb2\x46\xb7\xaa\x79\x71\x44\x3f\xa8\xe6\xa3\x61\xa2\xac\xc8\xd8"
        "\x6e\x45\xa6\x52\xab\x96\x25\x17\x0f\xae\xc4\x80\xe7\xc9\xc3\x7b\x9a"
        "\x4e\x7d\xf7\x33\xa8\xcb\x8f\xdd\x53\x7d\x37\x1a\xb4\xe6\xe3\xdd\xed"
        "\x92\xbc\x31\xd2\x35\x9f\xe9\x39\xfb\x9e\x3f\xb8\xe6\xc9\xcb\x7f\xf8"
        "\xed\x83\xdb\xed\x7b\x77\x6e\x2f\xad\x5e\x1b\x9d\x43\x3a\xa2\x87\x2b"
        "\x31\xdb\x53\x89\x17\x4e\xa1\x12\xff\x3d\x66\xfe\x27\xa7\x51\x55\xa3"
        "\x73\x15\x1d\xec\x6a\xf9\x52\xb9\xed\xf9\x2c\xe7\x47\x79\x3f\xb7\xb2"
        "\x98\xd7\x72\x33\x73\x99\xc9\xab\xb9\x99\xe9\x7c\x23\xaf\xf6\xd4\xf5"
        "\xca\x21\xae\x6f\xf5\xc1\xde\x6b\x5f\xf9\x6a\x15\x8c\x25\xf9\x5e\x35"
        "\x1f\x0d\x45\x5d\x2f\xf6\xd4\xb5\xf7\x4a\x37\x51\xf6\xf5\x2e\xd9\xab"
        "\xd2\xa5\x27\x54\x69\xe7\xc2\xc0\x57\xa4\xb3\x5f\xac\x82\xe2\x64\x7d"
        "\x7d\xe4\xae\x48\x17\x1f\xba\x36\x77\x2b\xf1\xdc\xc1\x95\xf8\xdd\x4e"
        "\xf1\xba\xda\xbe\x77\x67\xe5\xf6\xc2\x07\x87\xdc\xdf\xcb\xd5\xbc\xa8"
        "\xc0\x77\x47\xaa\x12\xc5\xf9\x72\xa9\xf8\x63\x95\xad\xfd\x67\x47\xd1"
        "\xf7\x5c\xdf\xbe\xe9\xb2\xef\xf2\x6e\x5f\xfd\x91\xbe\x2b\xbb\x7d\x4f"
        "\x7a\xa7\x36\xaa\x7b\xb8\x47\x47\x9a\x29\xfb\x5e\xe8\xdb\x37\x5b\xf6"
        "\xbd\xd8\xd3\xd7\xef\x2e\x67\x9f\x01\xbf\x6e\x01\xe0\x54\x8c\xbf\x32"
        "\xde\x68\xfd\xbb\xf5\xcf\xd6\x27\xad\xdf\xb4\x6e\xb7\xde\x6c\xbe\x7e"
        "\x6e\xee\xdc\x97\x1a\x19\xfb\xfb\xd9\xbf\x9c\xf9\x53\xfd\x8f\xf5\x6f"
        "\xd6\x5e\xc9\x27\xf9\xc5\xde\xf3\x3f\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x70\x74"
        "\xab\x1f\x7d\x7c\x67\xa1\xdd\x5e\x5c\x11\x8c\x76\xf0\x8f\xea\x0f\x36"
        "\x9c\xbd\x37\x92\x8c\x40\x11\x9e\xe5\xe0\xfe\x51\x37\x6f\x56\x67\xc6"
        "\x09\xe7\x33\xc4\x8b\x12\x70\x2a\x6e\xac\xdd\xfd\xe0\xc6\xea\x47\x1f"
        "\x7f\x6d\xf9\xee\xc2\x7b\x8b\xef\x2d\xde\xbb\x39\xf7\xf5\xd9\xd7\x6e"
        "\x4e\xcf\xcd\xdd\x58\x5a\x6e\x2f\x4e\x75\x5e\x87\x9d\x25\x00\x70\x92"
        "\xf6\x6e\xfa\x87\x9d\x09\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\xf0\x38\xa7\xf1\xef\xcd\xc3\x3e\x46\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xe0\xd9\x36\xff\x76\xc6"
        "\x36\x52\xcb\xf4\xd4\xf5\xa9\xa2\xbd\xb5\x39\xdb\x2e\xa6\x6e\xbc\xb7"
        "\x66\x33\x49\xad\x08\xfe\x96\xe4\xb3\x64\x23\x9d\x29\x13\x3d\xc3\xd5"
        "\xf6\xc2\xb1\x7d\xfb\x79\x77\xbc\xf1\xe0\xd3\x77\xae\x2e\xed\x8d\xd5"
        "\xec\xae\x5f\xdb\xbf\xdd\x51\xec\xcb\xa5\xfe\x50\x4e\xc7\x1d\x6f\x66"
        "\x90\xf1\x7e\xda\x13\xff\xb0\x55\x05\x7b\x47\x38\x99\xe4\x52\x35\x87"
        "\xa1\xfb\x5f\x00\x00\x00\xff\xff\x76\xad\xfe\x35",
        1559);
    syz_mount_image(0x20000080, 0x200006c0, 0, 0x200000c0, 1, 0x617,
                    0x20004e00);
    {
      int i;
      for (i = 0; i < 32; i++) {
        syz_mount_image(0x20000080, 0x200006c0, 0, 0x200000c0, 1, 0x617,
                        0x20004e00);
      }
    }
    break;
  case 1:
    syscall(__NR_open, 0ul, 0ul, 0ul);
    break;
  case 2:
    memcpy((void*)0x200000c0, "hfsplus\000", 8);
    memcpy((void*)0x20000080, "./file0\000", 8);
    memcpy((void*)0x20000100, "gid", 3);
    *(uint8_t*)0x20000103 = 0x3d;
    sprintf((char*)0x20000104, "0x%016llx", (long long)0);
    *(uint8_t*)0x20000116 = 0x2c;
    memcpy((void*)0x20000117, "decompose", 9);
    *(uint8_t*)0x20000120 = 0x2c;
    memcpy((void*)0x20000121, "barrier", 7);
    *(uint8_t*)0x20000128 = 0x2c;
    memcpy((void*)0x20000129, "type", 4);
    *(uint8_t*)0x2000012d = 0x3d;
    memcpy((void*)0x2000012e, "\x9b\xe6\xec\xb0", 4);
    *(uint8_t*)0x20000132 = 0x2c;
    memcpy((void*)0x20000133, "barrier", 7);
    *(uint8_t*)0x2000013a = 0x2c;
    memcpy((void*)0x2000013b, "part", 4);
    *(uint8_t*)0x2000013f = 0x3d;
    sprintf((char*)0x20000140, "0x%016llx", (long long)0xafa);
    *(uint8_t*)0x20000152 = 0x2c;
    *(uint8_t*)0x20000153 = 0;
    memcpy(
        (void*)0x20000200,
        "\x78\x9c\xec\xdd\xcd\x6b\x1c\xe7\x1d\x07\xf0\xef\xac\x56\xb2\xe5\x82"
        "\xa3\x24\x76\xe2\x96\x40\x45\x0c\x69\xa9\xa8\xad\x17\x94\x56\xbd\xd4"
        "\x2d\x25\xe8\x10\x4a\x48\x0f\x3d\x0b\x5b\x8e\x17\xaf\x95\x20\x6d\x8a"
        "\x12\x4a\x71\xfa\x42\xff\x81\xd0\x73\x7a\xd0\xad\xa7\x42\xef\x86\xf4"
        "\xdc\xde\x72\xd5\x31\x50\xc8\x25\x87\xa2\x4b\x51\x99\xd9\x59\x69\x63"
        "\xc9\xca\xca\xb6\xb4\xab\xf8\xf3\x11\xcf\x3e\xcf\xb3\xcf\xcc\x33\xbf"
        "\xf9\xed\xcc\xec\x8b\x58\x36\xc0\x33\x6b\x79\x26\xcd\x07\x29\xb2\x3c"
        "\xf3\xe6\x66\xd9\xdf\xde\x5a\x68\x6f\x6f\x2d\x9c\xab\x87\xdb\x49\xca"
        "\x76\x23\x69\x76\xab\x14\x6b\x49\xf1\x69\x72\x23\xdd\x92\x6f\x97\x77"
        "\xd6\xcb\x17\x8f\xda\xce\xc7\xad\xa5\xb7\x3f\xfb\x72\xfb\xf3\x6e\xaf"
        "\x59\x97\x6a\xf9\xc6\x51\xeb\x0d\xe6\x7e\x5d\x32\x9d\x64\xac\xae\x0f"
        "\x1a\x7f\xac\xf9\x6e\x3e\x72\xbe\x41\x15\x7b\x7b\x58\x26\xec\x6a\x2f"
        "\x71\x30\x6c\xbb\x07\xdc\x3f\xce\xea\x4f\x78\xde\x02\xa3\xa0\xe8\x3e"
        "\x6f\x1e\x30\x95\x5c\x48\x72\xbe\x7e\x1d\x90\xfa\xea\xd0\x38\xdd\xe8"
        "\x9e\xbe\x63\x5d\xe5\x00\x00\x00\xe0\x8c\x7a\x6e\x27\x3b\xd9\xcc\xc5"
        "\x61\xc7\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x67\x49\xfd\xfb"
        "\xff\x45\x5d\x1a\xbd\xf6\x74\x8a\xde\xef\xff\x4f\xd4\xf7\xa5\x6e\x9f"
        "\x69\x0f\x86\x1d\x00\x00\x00\x00\x00\x00\x00\x00\x3c\x05\xdf\xdd\xc9"
        "\x4e\x36\x73\xb1\xd7\xdf\x2d\xaa\xff\xf9\xbf\x5a\x75\x2e\x55\xb7\xdf"
        "\xca\xfb\xd9\xc8\x6a\xd6\x73\x2d\x9b\x59\x49\x27\x9d\xac\x67\x2e\xc9"
        "\x54\xdf\x44\x13\x9b\x2b\x9d\xce\xfa\xdc\x00\x6b\xce\x1f\xba\xe6\x7c"
        "\xde\x38\x9d\x3d\x06\x00\x00\x00\x00\x00\x00\x80\x6f\xa4\xdf\x67\x79"
        "\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x30"
        "\x0a\x8a\x64\xac\x5b\x55\xe5\x52\xaf\x3d\x95\x46\x33\xc9\xf9\x24\x13"
        "\xe5\x72\xf7\x93\x7f\xf7\xda\x67\xd9\x83\x61\x07\x00\x00\x00\x00\xa7"
        "\xe0\xb9\x9d\xec\x8c\xf7\xf5\x77\x8b\xea\x3d\xff\x4b\xd5\xfb\xfe\xf3"
        "\x79\x3f\x6b\xe9\xa4\x95\x4e\xda\x59\xcd\xad\xea\xb3\x80\xee\xbb\xfe"
        "\xc6\xf6\xd6\x42\x7b\x7b\x6b\xe1\x5e\x59\x0e\xce\xfb\x97\xff\xee\x56"
        "\x06\x0c\xa3\x9a\x31\xdd\xcf\x1e\x0e\xdf\xf2\x95\x6a\x89\xc9\xdc\x4e"
        "\xab\xba\xe7\x5a\x6e\xe6\xdd\xb4\x73\x2b\x8d\x6a\xcd\xd2\x95\x5e\x3c"
        "\x87\xc7\xf5\xd1\x17\xe5\xdc\x3f\xad\x0d\x18\xd9\xad\xba\x2e\xf7\xfc"
        "\xa3\xba\x1e\x0d\x53\x55\x46\xc6\xf7\x32\x32\x5b\xc7\x56\x66\xe3\xf9"
        "\xa3\x33\xf1\xb3\x2f\x9e\x68\x4b\x73\x69\xec\x7d\xf2\x73\xe9\x04\x72"
        "\x7e\xa1\xae\x8b\x87\xea\xe1\x7b\x38\x13\xf3\x7d\x47\xdf\x4b\x47\x67"
        "\x22\xf9\xde\x3f\xfe\xf6\xeb\x3b\xed\xb5\xbb\x77\x6e\x6f\xcc\x8c\xce"
        "\x2e\x3d\xa6\x87\x33\xb1\xd0\x97\x89\x97\x9f\xa9\x4c\xcc\x56\x99\xb8"
        "\xbc\xd7\x5f\xce\x1b\xf9\x55\x66\x32\x9d\xb7\xb2\x9e\x56\x7e\x93\x95"
        "\x74\xb2\x9a\xe9\xfc\xa2\x6a\xad\xd4\xc7\x73\x79\x3b\x75\x74\xa6\x6e"
        "\x7c\xa5\xf7\xd6\xd7\x45\x32\x51\x3f\x2e\xdd\xab\xe8\xf1\x62\x7a\xb5"
        "\x5a\xf7\x62\x5a\xf9\x65\xde\xcd\xad\xac\xe6\xf5\xea\x6f\x3e\x73\xf9"
        "\x51\x16\xb3\x98\xa5\xbe\x47\xf8\xf2\x00\x67\x7d\xe3\x78\x67\xfd\xd5"
        "\xef\xd7\x8d\xc9\x24\x7f\xae\xeb\xd1\x50\xe6\xf5\xf9\xbe\xbc\xf6\x5f"
        "\x73\xa7\xaa\xb1\xfe\x7b\xf6\xb3\xf4\xc2\x23\xb3\xb4\x3b\xf6\x98\xd7"
        "\xc6\xe6\x77\xea\x46\xb9\x8d\x3f\xd4\xf5\x68\x78\x38\x13\x73\x7d\x99"
        "\x78\xf1\xe8\xe3\xe5\xaf\xd5\xeb\x84\x8d\xf6\xda\xdd\xf5\x3b\x2b\xef"
        "\x0d\xb8\xbd\xd7\xea\xba\x3c\x8f\xfe\x34\x52\xcf\x12\xe5\xf1\xf2\x42"
        "\xf9\x60\x55\xbd\xaf\x1e\x1d\xe5\xd8\x8b\x87\x8e\xcd\x55\x63\x97\xf6"
        "\xc6\x1a\x07\xc6\x2e\xef\x8d\x7d\xdd\x99\x3a\x51\xbf\x86\x3b\x38\xd3"
        "\x7c\x35\xf6\xf2\xa1\x63\x0b\xd5\xd8\x95\xbe\xb1\xc3\x5e\x6f\x01\x30"
        "\xf2\x2e\xfc\xe0\xc2\xc4\xe4\x7f\x26\xff\x35\xf9\xc9\xe4\x1f\x27\xef"
        "\x4c\xbe\x79\xfe\xe7\xe7\x7e\x7c\xee\x95\x89\x8c\xff\x73\xfc\x27\xcd"
        "\xd9\xb1\xd7\x1a\xaf\x14\x7f\xcf\x27\xf9\x5d\x2e\x0e\x3b\x52\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\xf8\x26\xd8\xf8\xe0\xc3\xbb\x2b\xed\xf6\xea\xfa\x90"
        "\x1b\x45\xfd\x43\x3e\xa3\x12\xcf\x71\x1b\xbb\xa3\x11\xc6\x69\x37\xfe"
        "\xb7\x3b\x12\x61\x9c\xec\x91\x59\x8c\xd4\x99\x72\x5a\x8d\x61\x5f\x99"
        "\x80\x93\x76\xbd\x73\xef\xbd\xeb\x1b\x1f\x7c\xf8\xc3\xd6\xbd\x95\x77"
        "\x56\xdf\x59\x5d\x1b\x5f\x5c\x5c\x9a\x5d\x5a\x7c\x7d\xe1\xfa\xed\x56"
        "\x7b\x75\xb6\x7b\x3b\xec\x28\x81\x93\xb0\xff\xa4\x3f\xec\x48\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x80\x41\x1d\xfd\x35\x80\xf1\x7a\xa9\x27"
        "\xfb\x3a\xc1\x90\x77\x11\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x38\xe3\x96\x67\xd2"
        "\x7c\x90\x22\x73\xb3\xd7\x66\xcb\xfe\xf6\xd6\x42\xbb\x2c\xbd\xf6\xfe"
        "\x92\xcd\x24\x8d\x24\xc5\x6f\x93\xe2\xd3\xe4\x46\xba\x25\x53\x7d\xd3"
        "\x15\x8f\xda\xce\xc7\xad\xa5\xb7\x3f\xfb\x72\xfb\xf3\xfd\xb9\x9a\xbd"
        "\xe5\x1b\x47\xad\x37\x98\xfb\x75\xc9\x74\x92\xb1\xba\x7e\x5a\xf3\xdd"
        "\x7c\xe2\xf9\x8a\xbd\x3d\x2c\x13\x76\xb5\x97\x38\x18\xb6\xff\x07\x00"
        "\x00\xff\xff\x69\x08\x0b\x65",
        1537);
    syz_mount_image(0x200000c0, 0x20000080, 0, 0x20000100, 1, 0x601,
                    0x20000200);
    break;
  case 3:
    memcpy((void*)0x20000180, "./file0/file0\000", 14);
    memcpy((void*)0x200001c0, "./file0/file1\000", 14);
    syscall(__NR_rename, 0x20000180ul, 0x200001c0ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  setup_sysctl();
  loop();
  return 0;
}
