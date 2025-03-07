// https://syzkaller.appspot.com/bug?id=6e96f5bd56bfecdaa0bfeaf2695a25cc80bfbd9a
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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

unsigned long long procid;

static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* ctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  if (__atomic_load_n(&skip_segv, __ATOMIC_RELAXED) &&
      (addr < prog_start || addr > prog_end)) {
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
  {                                                                            \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    if (_setjmp(segv_env) == 0) {                                              \
      __VA_ARGS__;                                                             \
    }                                                                          \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
  }

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

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

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

static void kill_and_wait(int pid, int* status)
{
  kill(-pid, SIGKILL);
  kill(pid, SIGKILL);
  int i;
  for (i = 0; i < 100; i++) {
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
  int iter;
  for (iter = 0;; iter++) {
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
      if (current_time_ms() - start < 5 * 1000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0xcul);
  if (res != -1)
    r[0] = res;
  NONFAILING(*(uint64_t*)0x20000300 = 0);
  NONFAILING(*(uint32_t*)0x20000308 = 0);
  NONFAILING(*(uint64_t*)0x20000310 = 0x20000000);
  NONFAILING(*(uint64_t*)0x20000000 = 0x20000040);
  NONFAILING(*(uint32_t*)0x20000040 = 0x54);
  NONFAILING(*(uint8_t*)0x20000044 = 2);
  NONFAILING(*(uint8_t*)0x20000045 = 6);
  NONFAILING(*(uint16_t*)0x20000046 = 1);
  NONFAILING(*(uint32_t*)0x20000048 = 0);
  NONFAILING(*(uint32_t*)0x2000004c = 0);
  NONFAILING(*(uint8_t*)0x20000050 = 7);
  NONFAILING(*(uint8_t*)0x20000051 = 0);
  NONFAILING(*(uint16_t*)0x20000052 = htobe16(0));
  NONFAILING(*(uint16_t*)0x20000054 = 0xd);
  NONFAILING(*(uint16_t*)0x20000056 = 3);
  NONFAILING(memcpy((void*)0x20000058, "hash:net\000", 9));
  NONFAILING(*(uint16_t*)0x20000064 = 9);
  NONFAILING(*(uint16_t*)0x20000066 = 2);
  NONFAILING(memcpy((void*)0x20000068, "syz2\000", 5));
  NONFAILING(*(uint16_t*)0x20000070 = 0xc);
  NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x20000072, 7, 0, 14));
  NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x20000073, 0, 6, 1));
  NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x20000073, 1, 7, 1));
  NONFAILING(*(uint16_t*)0x20000074 = 8);
  NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x20000076, 8, 0, 14));
  NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x20000077, 1, 6, 1));
  NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x20000077, 0, 7, 1));
  NONFAILING(*(uint32_t*)0x20000078 = htobe32(0x3b));
  NONFAILING(*(uint16_t*)0x2000007c = 5);
  NONFAILING(*(uint16_t*)0x2000007e = 1);
  NONFAILING(*(uint8_t*)0x20000080 = 7);
  NONFAILING(*(uint16_t*)0x20000084 = 5);
  NONFAILING(*(uint16_t*)0x20000086 = 4);
  NONFAILING(*(uint8_t*)0x20000088 = 0);
  NONFAILING(*(uint16_t*)0x2000008c = 5);
  NONFAILING(*(uint16_t*)0x2000008e = 5);
  NONFAILING(*(uint8_t*)0x20000090 = 2);
  NONFAILING(*(uint64_t*)0x20000008 = 0x54);
  NONFAILING(*(uint64_t*)0x20000318 = 1);
  NONFAILING(*(uint64_t*)0x20000320 = 0);
  NONFAILING(*(uint64_t*)0x20000328 = 0);
  NONFAILING(*(uint32_t*)0x20000330 = 0x20000040);
  syscall(__NR_sendmsg, r[0], 0x20000300ul, 0ul);
  res = syscall(__NR_socket, 0x10ul, 3ul, 0xcul);
  if (res != -1)
    r[1] = res;
  NONFAILING(*(uint64_t*)0x20000d00 = 0);
  NONFAILING(*(uint32_t*)0x20000d08 = 0);
  NONFAILING(*(uint64_t*)0x20000d10 = 0x20000cc0);
  NONFAILING(*(uint64_t*)0x20000cc0 = 0x20000080);
  NONFAILING(*(uint32_t*)0x20000080 = 0x44);
  NONFAILING(*(uint8_t*)0x20000084 = 9);
  NONFAILING(*(uint8_t*)0x20000085 = 6);
  NONFAILING(*(uint16_t*)0x20000086 = 0x801);
  NONFAILING(*(uint32_t*)0x20000088 = 0);
  NONFAILING(*(uint32_t*)0x2000008c = 0);
  NONFAILING(*(uint8_t*)0x20000090 = 0);
  NONFAILING(*(uint8_t*)0x20000091 = 0);
  NONFAILING(*(uint16_t*)0x20000092 = htobe16(0));
  NONFAILING(*(uint16_t*)0x20000094 = 5);
  NONFAILING(*(uint16_t*)0x20000096 = 1);
  NONFAILING(*(uint8_t*)0x20000098 = 7);
  NONFAILING(*(uint16_t*)0x2000009c = 0x1c);
  NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x2000009e, 7, 0, 14));
  NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x2000009f, 0, 6, 1));
  NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x2000009f, 1, 7, 1));
  NONFAILING(*(uint16_t*)0x200000a0 = 0xc);
  NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x200000a2, 2, 0, 14));
  NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x200000a3, 0, 6, 1));
  NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x200000a3, 1, 7, 1));
  NONFAILING(*(uint16_t*)0x200000a4 = 8);
  NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x200000a6, 1, 0, 14));
  NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x200000a7, 1, 6, 1));
  NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x200000a7, 0, 7, 1));
  NONFAILING(*(uint32_t*)0x200000a8 = htobe32(0xc6));
  NONFAILING(*(uint16_t*)0x200000ac = 0xc);
  NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x200000ae, 1, 0, 14));
  NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x200000af, 0, 6, 1));
  NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x200000af, 1, 7, 1));
  NONFAILING(*(uint16_t*)0x200000b0 = 8);
  NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x200000b2, 1, 0, 14));
  NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x200000b3, 1, 6, 1));
  NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x200000b3, 0, 7, 1));
  NONFAILING(*(uint32_t*)0x200000b4 = htobe32(0xe0000001));
  NONFAILING(*(uint16_t*)0x200000b8 = 9);
  NONFAILING(*(uint16_t*)0x200000ba = 2);
  NONFAILING(memcpy((void*)0x200000bc, "syz2\000", 5));
  NONFAILING(*(uint64_t*)0x20000cc8 = 0x44);
  NONFAILING(*(uint64_t*)0x20000d18 = 1);
  NONFAILING(*(uint64_t*)0x20000d20 = 0);
  NONFAILING(*(uint64_t*)0x20000d28 = 0);
  NONFAILING(*(uint32_t*)0x20000d30 = 0);
  syscall(__NR_sendmsg, r[1], 0x20000d00ul, 0ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  install_segv_handler();
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
