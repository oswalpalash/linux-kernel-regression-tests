// https://syzkaller.appspot.com/bug?id=d369eafab7836fe0dd2e027d2689da06cad8e1a3
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

const int kFailStatus = 67;
const int kRetryStatus = 69;

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}

__attribute__((noreturn)) static void fail(const char* msg, ...)
{
  int e = errno;
  fflush(stdout);
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit((e == ENOMEM || e == EAGAIN) ? kRetryStatus : kFailStatus);
}

__attribute__((noreturn)) static void exitf(const char* msg, ...)
{
  int e = errno;
  fflush(stdout);
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit(kRetryStatus);
}

#define BITMASK_LEN(type, bf_len) (type)((1ull << (bf_len)) - 1)

#define BITMASK_LEN_OFF(type, bf_off, bf_len)                          \
  (type)(BITMASK_LEN(type, (bf_len)) << (bf_off))

#define STORE_BY_BITMASK(type, addr, val, bf_off, bf_len)              \
  if ((bf_off) == 0 && (bf_len) == 0) {                                \
    *(type*)(addr) = (type)(val);                                      \
  } else {                                                             \
    type new_val = *(type*)(addr);                                     \
    new_val &= ~BITMASK_LEN_OFF(type, (bf_off), (bf_len));             \
    new_val |= ((type)(val)&BITMASK_LEN(type, (bf_len))) << (bf_off);  \
    *(type*)(addr) = new_val;                                          \
  }

struct csum_inet {
  uint32_t acc;
};

static void csum_inet_init(struct csum_inet* csum)
{
  csum->acc = 0;
}

static void csum_inet_update(struct csum_inet* csum,
                             const uint8_t* data, size_t length)
{
  if (length == 0)
    return;

  size_t i;
  for (i = 0; i < length - 1; i += 2)
    csum->acc += *(uint16_t*)&data[i];

  if (length & 1)
    csum->acc += (uint16_t)data[length - 1];

  while (csum->acc > 0xffff)
    csum->acc = (csum->acc & 0xffff) + (csum->acc >> 16);
}

static uint16_t csum_inet_digest(struct csum_inet* csum)
{
  return ~csum->acc;
}

static uint64_t current_time_ms()
{
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    fail("clock_gettime failed");
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void test();

void loop()
{
  int iter;
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      fail("clone failed");
    if (pid == 0) {
      prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
      setpgrp();
      test();
      doexit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      int res = waitpid(-1, &status, __WALL | WNOHANG);
      if (res == pid)
        break;
      usleep(1000);
      if (current_time_ms() - start > 5 * 1000) {
        kill(-pid, SIGKILL);
        kill(pid, SIGKILL);
        while (waitpid(-1, &status, __WALL) != pid) {
        }
        break;
      }
    }
  }
}

long r[248];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    *(uint32_t*)0x20000000 = (uint32_t)0x0;
    *(uint16_t*)0x20000004 = (uint16_t)0x0;
    *(uint16_t*)0x20000006 = (uint16_t)0x0;
    r[4] = syscall(__NR_setsockopt, 0xfffffffffffffffful, 0x84ul,
                   0x17ul, 0x20000000ul, 0x8ul);
    break;
  case 2:
    r[5] = syscall(__NR_socket, 0x2ul, 0x400000005ul, 0xfffful);
    break;
  case 3:
    memcpy((void*)0x2022c000, "\xb9\x0e\x18\x66\x44\xf6", 6);
    *(uint8_t*)0x2022c006 = (uint8_t)0xaa;
    *(uint8_t*)0x2022c007 = (uint8_t)0xaa;
    *(uint8_t*)0x2022c008 = (uint8_t)0xaa;
    *(uint8_t*)0x2022c009 = (uint8_t)0xaa;
    *(uint8_t*)0x2022c00a = (uint8_t)0xaa;
    *(uint8_t*)0x2022c00b = (uint8_t)0x0;
    *(uint16_t*)0x2022c00c = (uint16_t)0x8;
    STORE_BY_BITMASK(uint8_t, 0x2022c00e, 0x10001, 0, 4);
    STORE_BY_BITMASK(uint8_t, 0x2022c00e, 0x6, 4, 4);
    memcpy((void*)0x2022c00f, "\x11\xe8\x74", 3);
    *(uint16_t*)0x2022c012 = (uint16_t)0x1401;
    *(uint8_t*)0x2022c014 = (uint8_t)0x6c;
    *(uint8_t*)0x2022c015 = (uint8_t)0x5;
    *(uint8_t*)0x2022c016 = (uint8_t)0xfe;
    *(uint8_t*)0x2022c017 = (uint8_t)0x80;
    *(uint8_t*)0x2022c018 = (uint8_t)0x0;
    *(uint8_t*)0x2022c019 = (uint8_t)0x0;
    *(uint8_t*)0x2022c01a = (uint8_t)0x0;
    *(uint8_t*)0x2022c01b = (uint8_t)0x0;
    *(uint8_t*)0x2022c01c = (uint8_t)0x0;
    *(uint8_t*)0x2022c01d = (uint8_t)0x0;
    *(uint8_t*)0x2022c01e = (uint8_t)0x0;
    *(uint8_t*)0x2022c01f = (uint8_t)0x0;
    *(uint8_t*)0x2022c020 = (uint8_t)0x0;
    *(uint8_t*)0x2022c021 = (uint8_t)0x0;
    *(uint8_t*)0x2022c022 = (uint8_t)0x0;
    *(uint8_t*)0x2022c023 = (uint8_t)0x0;
    *(uint8_t*)0x2022c024 = (uint8_t)0x0;
    *(uint8_t*)0x2022c025 = (uint8_t)0xbb;
    *(uint8_t*)0x2022c026 = (uint8_t)0xfe;
    *(uint8_t*)0x2022c027 = (uint8_t)0x80;
    *(uint8_t*)0x2022c028 = (uint8_t)0x0;
    *(uint8_t*)0x2022c029 = (uint8_t)0x0;
    *(uint8_t*)0x2022c02a = (uint8_t)0x0;
    *(uint8_t*)0x2022c02b = (uint8_t)0x0;
    *(uint8_t*)0x2022c02c = (uint8_t)0x0;
    *(uint8_t*)0x2022c02d = (uint8_t)0x0;
    *(uint8_t*)0x2022c02e = (uint8_t)0x0;
    *(uint8_t*)0x2022c02f = (uint8_t)0x0;
    *(uint8_t*)0x2022c030 = (uint8_t)0x0;
    *(uint8_t*)0x2022c031 = (uint8_t)0x0;
    *(uint8_t*)0x2022c032 = (uint8_t)0x0;
    *(uint8_t*)0x2022c033 = (uint8_t)0x0;
    *(uint8_t*)0x2022c034 = (uint8_t)0x0;
    *(uint8_t*)0x2022c035 = (uint8_t)0xbb;
    *(uint8_t*)0x2022c036 = (uint8_t)0x0;
    *(uint8_t*)0x2022c037 = (uint8_t)0x0;
    *(uint8_t*)0x2022c038 = (uint8_t)0x0;
    *(uint8_t*)0x2022c039 = (uint8_t)0x0;
    *(uint8_t*)0x2022c03a = (uint8_t)0x0;
    *(uint8_t*)0x2022c03b = (uint8_t)0x0;
    *(uint8_t*)0x2022c03c = (uint8_t)0x0;
    *(uint8_t*)0x2022c03d = (uint8_t)0x0;
    *(uint8_t*)0x2022c03e = (uint8_t)0xc9;
    *(uint8_t*)0x2022c03f = (uint8_t)0x0;
    *(uint8_t*)0x2022c040 = (uint8_t)0xff;
    *(uint8_t*)0x2022c041 = (uint8_t)0x0;
    *(uint8_t*)0x2022c042 = (uint8_t)0xbf;
    *(uint8_t*)0x2022c043 = (uint8_t)0x6;
    *(uint8_t*)0x2022c044 = (uint8_t)0x1;
    STORE_BY_BITMASK(uint8_t, 0x2022c045, 0x2, 0, 1);
    STORE_BY_BITMASK(uint8_t, 0x2022c045, 0x3, 1, 2);
    STORE_BY_BITMASK(uint8_t, 0x2022c045, 0x1f, 3, 5);
    *(uint32_t*)0x2022c046 = (uint32_t)0x66;
    *(uint8_t*)0x2022c04a = (uint8_t)0xff;
    *(uint8_t*)0x2022c04b = (uint8_t)0x101;
    *(uint8_t*)0x2022c04c = (uint8_t)0xfffffffffffffe00;
    STORE_BY_BITMASK(uint8_t, 0x2022c04d, 0x2, 0, 1);
    STORE_BY_BITMASK(uint8_t, 0x2022c04d, 0x401, 1, 2);
    STORE_BY_BITMASK(uint8_t, 0x2022c04d, 0xfffffffffffffff7, 3, 5);
    *(uint32_t*)0x2022c04e = (uint32_t)0x66;
    *(uint8_t*)0x2022c052 = (uint8_t)0x8;
    *(uint8_t*)0x2022c053 = (uint8_t)0x6;
    *(uint8_t*)0x2022c054 = (uint8_t)0x0;
    *(uint8_t*)0x2022c055 = (uint8_t)0x2;
    *(uint32_t*)0x2022c056 = (uint32_t)0x1f000000;
    *(uint8_t*)0x2022c05a = (uint8_t)0xfe;
    *(uint8_t*)0x2022c05b = (uint8_t)0x80;
    *(uint8_t*)0x2022c05c = (uint8_t)0x0;
    *(uint8_t*)0x2022c05d = (uint8_t)0x0;
    *(uint8_t*)0x2022c05e = (uint8_t)0x0;
    *(uint8_t*)0x2022c05f = (uint8_t)0x0;
    *(uint8_t*)0x2022c060 = (uint8_t)0x0;
    *(uint8_t*)0x2022c061 = (uint8_t)0x0;
    *(uint8_t*)0x2022c062 = (uint8_t)0x0;
    *(uint8_t*)0x2022c063 = (uint8_t)0x0;
    *(uint8_t*)0x2022c064 = (uint8_t)0x0;
    *(uint8_t*)0x2022c065 = (uint8_t)0x0;
    *(uint8_t*)0x2022c066 = (uint8_t)0x0;
    *(uint8_t*)0x2022c067 = (uint8_t)0x0;
    *(uint8_t*)0x2022c068 = (uint8_t)0x0;
    *(uint8_t*)0x2022c069 = (uint8_t)0xaa;
    *(uint8_t*)0x2022c06a = (uint8_t)0x0;
    *(uint8_t*)0x2022c06b = (uint8_t)0x0;
    *(uint8_t*)0x2022c06c = (uint8_t)0x0;
    *(uint8_t*)0x2022c06d = (uint8_t)0x0;
    *(uint8_t*)0x2022c06e = (uint8_t)0x0;
    *(uint8_t*)0x2022c06f = (uint8_t)0x0;
    *(uint8_t*)0x2022c070 = (uint8_t)0x0;
    *(uint8_t*)0x2022c071 = (uint8_t)0x0;
    *(uint8_t*)0x2022c072 = (uint8_t)0x0;
    *(uint8_t*)0x2022c073 = (uint8_t)0x0;
    *(uint8_t*)0x2022c074 = (uint8_t)0x0;
    *(uint8_t*)0x2022c075 = (uint8_t)0x0;
    *(uint8_t*)0x2022c076 = (uint8_t)0x0;
    *(uint8_t*)0x2022c077 = (uint8_t)0x0;
    *(uint8_t*)0x2022c078 = (uint8_t)0x0;
    *(uint8_t*)0x2022c079 = (uint8_t)0x0;
    *(uint64_t*)0x2022c07a = (uint64_t)0x0;
    *(uint64_t*)0x2022c082 = (uint64_t)0x100000000000000;
    *(uint8_t*)0x2022c08a = (uint8_t)0xff;
    *(uint8_t*)0x2022c08b = (uint8_t)0xa;
    *(uint8_t*)0x2022c08c = (uint8_t)0x3;
    *(uint8_t*)0x2022c08d = (uint8_t)0x20;
    *(uint32_t*)0x2022c08e = (uint32_t)0x7f000000;
    *(uint8_t*)0x2022c092 = (uint8_t)0xfe;
    *(uint8_t*)0x2022c093 = (uint8_t)0x80;
    *(uint8_t*)0x2022c094 = (uint8_t)0x0;
    *(uint8_t*)0x2022c095 = (uint8_t)0x0;
    *(uint8_t*)0x2022c096 = (uint8_t)0x0;
    *(uint8_t*)0x2022c097 = (uint8_t)0x0;
    *(uint8_t*)0x2022c098 = (uint8_t)0x0;
    *(uint8_t*)0x2022c099 = (uint8_t)0x0;
    *(uint8_t*)0x2022c09a = (uint8_t)0x0;
    *(uint8_t*)0x2022c09b = (uint8_t)0x0;
    *(uint8_t*)0x2022c09c = (uint8_t)0x0;
    *(uint8_t*)0x2022c09d = (uint8_t)0x0;
    *(uint8_t*)0x2022c09e = (uint8_t)0x0;
    *(uint8_t*)0x2022c09f = (uint8_t)0x0;
    *(uint8_t*)0x2022c0a0 = (uint8_t)0x0;
    *(uint8_t*)0x2022c0a1 = (uint8_t)0xbb;
    *(uint8_t*)0x2022c0a2 = (uint8_t)0xfe;
    *(uint8_t*)0x2022c0a3 = (uint8_t)0x80;
    *(uint8_t*)0x2022c0a4 = (uint8_t)0x0;
    *(uint8_t*)0x2022c0a5 = (uint8_t)0x0;
    *(uint8_t*)0x2022c0a6 = (uint8_t)0x0;
    *(uint8_t*)0x2022c0a7 = (uint8_t)0x0;
    *(uint8_t*)0x2022c0a8 = (uint8_t)0x0;
    *(uint8_t*)0x2022c0a9 = (uint8_t)0x0;
    *(uint8_t*)0x2022c0aa = (uint8_t)0x0;
    *(uint8_t*)0x2022c0ab = (uint8_t)0x0;
    *(uint8_t*)0x2022c0ac = (uint8_t)0x0;
    *(uint8_t*)0x2022c0ad = (uint8_t)0x0;
    *(uint8_t*)0x2022c0ae = (uint8_t)0x0;
    *(uint8_t*)0x2022c0af = (uint8_t)0x0;
    *(uint8_t*)0x2022c0b0 = (uint8_t)0x0;
    *(uint8_t*)0x2022c0b1 = (uint8_t)0xbb;
    *(uint64_t*)0x2022c0b2 = (uint64_t)0x0;
    *(uint64_t*)0x2022c0ba = (uint64_t)0x100000000000000;
    *(uint8_t*)0x2022c0c2 = (uint8_t)0x0;
    *(uint8_t*)0x2022c0c3 = (uint8_t)0x0;
    *(uint8_t*)0x2022c0c4 = (uint8_t)0x0;
    *(uint8_t*)0x2022c0c5 = (uint8_t)0x0;
    *(uint8_t*)0x2022c0c6 = (uint8_t)0x0;
    *(uint8_t*)0x2022c0c7 = (uint8_t)0x0;
    *(uint8_t*)0x2022c0c8 = (uint8_t)0x0;
    *(uint8_t*)0x2022c0c9 = (uint8_t)0x0;
    *(uint8_t*)0x2022c0ca = (uint8_t)0x0;
    *(uint8_t*)0x2022c0cb = (uint8_t)0x0;
    *(uint8_t*)0x2022c0cc = (uint8_t)0x0;
    *(uint8_t*)0x2022c0cd = (uint8_t)0x0;
    *(uint8_t*)0x2022c0ce = (uint8_t)0x0;
    *(uint8_t*)0x2022c0cf = (uint8_t)0x0;
    *(uint8_t*)0x2022c0d0 = (uint8_t)0x0;
    *(uint8_t*)0x2022c0d1 = (uint8_t)0x0;
    *(uint64_t*)0x2022c0d2 = (uint64_t)0x0;
    *(uint64_t*)0x2022c0da = (uint64_t)0x100000000000000;
    *(uint8_t*)0x2022c0e2 = (uint8_t)0x7f;
    *(uint8_t*)0x2022c0e3 = (uint8_t)0xa;
    *(uint8_t*)0x2022c0e4 = (uint8_t)0x2;
    *(uint8_t*)0x2022c0e5 = (uint8_t)0x7;
    *(uint32_t*)0x2022c0e6 = (uint32_t)0x100000;
    *(uint8_t*)0x2022c0ea = (uint8_t)0xfe;
    *(uint8_t*)0x2022c0eb = (uint8_t)0x80;
    *(uint8_t*)0x2022c0ec = (uint8_t)0x0;
    *(uint8_t*)0x2022c0ed = (uint8_t)0x0;
    *(uint8_t*)0x2022c0ee = (uint8_t)0x0;
    *(uint8_t*)0x2022c0ef = (uint8_t)0x0;
    *(uint8_t*)0x2022c0f0 = (uint8_t)0x0;
    *(uint8_t*)0x2022c0f1 = (uint8_t)0x0;
    *(uint8_t*)0x2022c0f2 = (uint8_t)0x0;
    *(uint8_t*)0x2022c0f3 = (uint8_t)0x0;
    *(uint8_t*)0x2022c0f4 = (uint8_t)0x0;
    *(uint8_t*)0x2022c0f5 = (uint8_t)0x0;
    *(uint8_t*)0x2022c0f6 = (uint8_t)0x0;
    *(uint8_t*)0x2022c0f7 = (uint8_t)0x0;
    *(uint8_t*)0x2022c0f8 = (uint8_t)0x0;
    *(uint8_t*)0x2022c0f9 = (uint8_t)0xaa;
    *(uint8_t*)0x2022c0fa = (uint8_t)0xfe;
    *(uint8_t*)0x2022c0fb = (uint8_t)0x80;
    *(uint8_t*)0x2022c0fc = (uint8_t)0x0;
    *(uint8_t*)0x2022c0fd = (uint8_t)0x0;
    *(uint8_t*)0x2022c0fe = (uint8_t)0x0;
    *(uint8_t*)0x2022c0ff = (uint8_t)0x0;
    *(uint8_t*)0x2022c100 = (uint8_t)0x0;
    *(uint8_t*)0x2022c101 = (uint8_t)0x0;
    *(uint8_t*)0x2022c102 = (uint8_t)0x0;
    *(uint8_t*)0x2022c103 = (uint8_t)0x0;
    *(uint8_t*)0x2022c104 = (uint8_t)0x0;
    *(uint8_t*)0x2022c105 = (uint8_t)0x0;
    *(uint8_t*)0x2022c106 = (uint8_t)0x0;
    *(uint8_t*)0x2022c107 = (uint8_t)0x0;
    *(uint8_t*)0x2022c108 = (uint8_t)0x0;
    *(uint8_t*)0x2022c109 = (uint8_t)0xaa;
    *(uint64_t*)0x2022c10a = (uint64_t)0x0;
    *(uint64_t*)0x2022c112 = (uint64_t)0x100000000000000;
    *(uint64_t*)0x2022c11a = (uint64_t)0x0;
    *(uint64_t*)0x2022c122 = (uint64_t)0x100000000000000;
    *(uint8_t*)0x2022c12a = (uint8_t)0xfe;
    *(uint8_t*)0x2022c12b = (uint8_t)0x80;
    *(uint8_t*)0x2022c12c = (uint8_t)0x0;
    *(uint8_t*)0x2022c12d = (uint8_t)0x0;
    *(uint8_t*)0x2022c12e = (uint8_t)0x0;
    *(uint8_t*)0x2022c12f = (uint8_t)0x0;
    *(uint8_t*)0x2022c130 = (uint8_t)0x0;
    *(uint8_t*)0x2022c131 = (uint8_t)0x0;
    *(uint8_t*)0x2022c132 = (uint8_t)0x0;
    *(uint8_t*)0x2022c133 = (uint8_t)0x0;
    *(uint8_t*)0x2022c134 = (uint8_t)0x0;
    *(uint8_t*)0x2022c135 = (uint8_t)0x0;
    *(uint8_t*)0x2022c136 = (uint8_t)0x0;
    *(uint8_t*)0x2022c137 = (uint8_t)0x0;
    *(uint8_t*)0x2022c138 = (uint8_t)0x0;
    *(uint8_t*)0x2022c139 = (uint8_t)0xbb;
    *(uint16_t*)0x2022c13a = (uint16_t)0x224e;
    *(uint16_t*)0x2022c13c = (uint16_t)0x224e;
    *(uint8_t*)0x2022c13e = (uint8_t)0x4;
    STORE_BY_BITMASK(uint8_t, 0x2022c13f, 0x1, 0, 4);
    STORE_BY_BITMASK(uint8_t, 0x2022c13f, 0x401, 4, 4);
    *(uint16_t*)0x2022c140 = (uint16_t)0x0;
    STORE_BY_BITMASK(uint8_t, 0x2022c142, 0x0, 0, 1);
    STORE_BY_BITMASK(uint8_t, 0x2022c142, 0x8, 1, 4);
    STORE_BY_BITMASK(uint8_t, 0x2022c142, 0x1002, 5, 3);
    memcpy((void*)0x2022c143, "\xa2\xfd\x76", 3);
    *(uint8_t*)0x2022c146 = (uint8_t)0x0;
    memcpy((void*)0x2022c147, "\x3c\xe0\xca", 3);
    struct csum_inet csum_243;
    csum_inet_init(&csum_243);
    csum_inet_update(&csum_243, (const uint8_t*)0x2022c016, 16);
    csum_inet_update(&csum_243, (const uint8_t*)0x2022c026, 16);
    uint32_t csum_243_chunk_2 = 0x10000000;
    csum_inet_update(&csum_243, (const uint8_t*)&csum_243_chunk_2, 4);
    uint32_t csum_243_chunk_3 = 0x21000000;
    csum_inet_update(&csum_243, (const uint8_t*)&csum_243_chunk_3, 4);
    csum_inet_update(&csum_243, (const uint8_t*)0x2022c13a, 16);
    *(uint16_t*)0x2022c140 = csum_inet_digest(&csum_243);
    break;
  case 4:
    memcpy((void*)0x2002aff7, "\x2f\x64\x65\x76\x2f\x6b\x76\x6d\x00",
           9);
    r[246] = syscall(__NR_openat, 0xffffffffffffff9cul, 0x2002aff7ul,
                     0x0ul, 0x0ul);
    break;
  case 5:
    r[247] = syscall(__NR_ioctl, r[246], 0xae01ul, 0x0ul);
    break;
  }
  return 0;
}

void test()
{
  long i;
  pthread_t th[12];

  memset(r, -1, sizeof(r));
  srand(getpid());
  for (i = 0; i < 6; i++) {
    pthread_create(&th[i], 0, thr, (void*)i);
    usleep(rand() % 10000);
  }
  for (i = 0; i < 6; i++) {
    pthread_create(&th[6 + i], 0, thr, (void*)i);
    if (rand() % 2)
      usleep(rand() % 10000);
  }
  usleep(rand() % 100000);
}

int main()
{
  int i;
  for (i = 0; i < 8; i++) {
    if (fork() == 0) {
      loop();
      return 0;
    }
  }
  sleep(1000000);
  return 0;
}
