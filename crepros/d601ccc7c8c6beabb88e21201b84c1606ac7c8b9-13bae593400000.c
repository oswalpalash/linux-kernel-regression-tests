// https://syzkaller.appspot.com/bug?id=d601ccc7c8c6beabb88e21201b84c1606ac7c8b9
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>
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

#define SYZ_HAVE_SETUP_TEST 1
static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
}

#define SYZ_HAVE_RESET_TEST 1
static void reset_test()
{
  int fd;
  for (fd = 3; fd < 30; fd++)
    close(fd);
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
      reset_test();
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

uint64_t r[1] = {0xffffffffffffffff};

void execute_one(void)
{
  long res = 0;
  res = syscall(__NR_socket, 0xa, 1, 0);
  if (res != -1)
    r[0] = res;
  syscall(__NR_socket, 0x18, 1, 1);
  NONFAILING(memcpy((void*)0x200004c0, "\x62\x72\x6f\x70\x74\x65\x00\x00\x64"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00",
                    32));
  NONFAILING(*(uint32_t*)0x200004e0 = 0x20);
  NONFAILING(*(uint32_t*)0x200004e4 = 1);
  NONFAILING(*(uint32_t*)0x200004e8 = 0x368);
  NONFAILING(*(uint64_t*)0x200004f0 = 0);
  NONFAILING(*(uint64_t*)0x200004f8 = 0);
  NONFAILING(*(uint64_t*)0x20000500 = 0);
  NONFAILING(*(uint64_t*)0x20000508 = 0);
  NONFAILING(*(uint64_t*)0x20000510 = 0);
  NONFAILING(*(uint64_t*)0x20000518 = 0x20000100);
  NONFAILING(*(uint32_t*)0x20000520 = 0x7000000);
  NONFAILING(*(uint64_t*)0x20000528 = 0);
  NONFAILING(*(uint64_t*)0x20000530 = 0x20000100);
  NONFAILING(*(uint32_t*)0x20000100 = 0);
  NONFAILING(memcpy((void*)0x20000104, "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00",
                    32));
  NONFAILING(*(uint32_t*)0x20000124 = 0);
  NONFAILING(*(uint32_t*)0x20000128 = -1);
  NONFAILING(*(uint32_t*)0x2000012c = 1);
  NONFAILING(*(uint32_t*)0x20000130 = 9);
  NONFAILING(*(uint32_t*)0x20000134 = 0);
  NONFAILING(*(uint16_t*)0x20000138 = htobe16(0));
  NONFAILING(memcpy(
      (void*)0x2000013a,
      "\x73\x79\x7a\x5f\x74\x75\x6e\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16));
  NONFAILING(memcpy(
      (void*)0x2000014a,
      "\x62\x6f\x6e\x64\x5f\x73\x6c\x61\x76\x65\x5f\x30\x00\x00\x00\x00", 16));
  NONFAILING(memcpy(
      (void*)0x2000015a,
      "\x76\x65\x74\x68\x30\x5f\x74\x6f\x5f\x74\x65\x61\x6d\x00\x00\x00", 16));
  NONFAILING(memcpy(
      (void*)0x2000016a,
      "\x76\x65\x74\x68\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16));
  NONFAILING(*(uint8_t*)0x2000017a = 1);
  NONFAILING(*(uint8_t*)0x2000017b = 0x80);
  NONFAILING(*(uint8_t*)0x2000017c = 0xc2);
  NONFAILING(*(uint8_t*)0x2000017d = 0);
  NONFAILING(*(uint8_t*)0x2000017e = 0);
  NONFAILING(*(uint8_t*)0x2000017f = 0);
  NONFAILING(*(uint8_t*)0x20000180 = 0);
  NONFAILING(*(uint8_t*)0x20000181 = 0);
  NONFAILING(*(uint8_t*)0x20000182 = 0);
  NONFAILING(*(uint8_t*)0x20000183 = 0);
  NONFAILING(*(uint8_t*)0x20000184 = 0);
  NONFAILING(*(uint8_t*)0x20000185 = 0);
  NONFAILING(*(uint8_t*)0x20000186 = 0);
  NONFAILING(*(uint8_t*)0x20000187 = 0);
  NONFAILING(*(uint8_t*)0x20000188 = 0);
  NONFAILING(*(uint8_t*)0x20000189 = 0);
  NONFAILING(*(uint8_t*)0x2000018a = 0);
  NONFAILING(*(uint8_t*)0x2000018b = 0);
  NONFAILING(*(uint8_t*)0x2000018c = 0);
  NONFAILING(*(uint8_t*)0x2000018d = 0);
  NONFAILING(*(uint8_t*)0x2000018e = 0);
  NONFAILING(*(uint8_t*)0x2000018f = 0);
  NONFAILING(*(uint8_t*)0x20000190 = 0);
  NONFAILING(*(uint8_t*)0x20000191 = 0);
  NONFAILING(*(uint32_t*)0x20000194 = 0x2a8);
  NONFAILING(*(uint32_t*)0x20000198 = 0x2a8);
  NONFAILING(*(uint32_t*)0x2000019c = 0x2d8);
  NONFAILING(memcpy((void*)0x200001a0, "\x62\x70\x66\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x01",
                    32));
  NONFAILING(*(uint32_t*)0x200001c0 = 0x210);
  NONFAILING(*(uint16_t*)0x200001c8 = 0);
  NONFAILING(*(uint16_t*)0x200001cc = 0);
  NONFAILING(*(uint8_t*)0x200001ce = 0);
  NONFAILING(*(uint8_t*)0x200001cf = 0);
  NONFAILING(*(uint32_t*)0x200001d0 = 0);
  NONFAILING(*(uint16_t*)0x200001d4 = 0);
  NONFAILING(*(uint8_t*)0x200001d6 = 0);
  NONFAILING(*(uint8_t*)0x200001d7 = 0);
  NONFAILING(*(uint32_t*)0x200001d8 = 0);
  NONFAILING(*(uint16_t*)0x200001dc = 0);
  NONFAILING(*(uint8_t*)0x200001de = 0);
  NONFAILING(*(uint8_t*)0x200001df = 0);
  NONFAILING(*(uint32_t*)0x200001e0 = 0);
  NONFAILING(*(uint16_t*)0x200001e4 = 0);
  NONFAILING(*(uint8_t*)0x200001e6 = 0);
  NONFAILING(*(uint8_t*)0x200001e7 = 0);
  NONFAILING(*(uint32_t*)0x200001e8 = 0);
  NONFAILING(*(uint16_t*)0x200001ec = 0);
  NONFAILING(*(uint8_t*)0x200001ee = 0);
  NONFAILING(*(uint8_t*)0x200001ef = 0);
  NONFAILING(*(uint32_t*)0x200001f0 = 0);
  NONFAILING(*(uint16_t*)0x200001f4 = 0);
  NONFAILING(*(uint8_t*)0x200001f6 = 0);
  NONFAILING(*(uint8_t*)0x200001f7 = 0);
  NONFAILING(*(uint32_t*)0x200001f8 = 0);
  NONFAILING(*(uint16_t*)0x200001fc = 0);
  NONFAILING(*(uint8_t*)0x200001fe = 0);
  NONFAILING(*(uint8_t*)0x200001ff = 0);
  NONFAILING(*(uint32_t*)0x20000200 = 0);
  NONFAILING(*(uint16_t*)0x20000204 = 0);
  NONFAILING(*(uint8_t*)0x20000206 = 0);
  NONFAILING(*(uint8_t*)0x20000207 = 0);
  NONFAILING(*(uint32_t*)0x20000208 = 0);
  NONFAILING(*(uint16_t*)0x2000020c = 0);
  NONFAILING(*(uint8_t*)0x2000020e = 0);
  NONFAILING(*(uint8_t*)0x2000020f = 0);
  NONFAILING(*(uint32_t*)0x20000210 = 0);
  NONFAILING(*(uint16_t*)0x20000214 = 0);
  NONFAILING(*(uint8_t*)0x20000216 = 0);
  NONFAILING(*(uint8_t*)0x20000217 = 0);
  NONFAILING(*(uint32_t*)0x20000218 = 0);
  NONFAILING(*(uint16_t*)0x2000021c = 0);
  NONFAILING(*(uint8_t*)0x2000021e = 0);
  NONFAILING(*(uint8_t*)0x2000021f = 0);
  NONFAILING(*(uint32_t*)0x20000220 = 0);
  NONFAILING(*(uint16_t*)0x20000224 = 0);
  NONFAILING(*(uint8_t*)0x20000226 = 0);
  NONFAILING(*(uint8_t*)0x20000227 = 0);
  NONFAILING(*(uint32_t*)0x20000228 = 0);
  NONFAILING(*(uint16_t*)0x2000022c = 0);
  NONFAILING(*(uint8_t*)0x2000022e = 0);
  NONFAILING(*(uint8_t*)0x2000022f = 0);
  NONFAILING(*(uint32_t*)0x20000230 = 0);
  NONFAILING(*(uint16_t*)0x20000234 = 0);
  NONFAILING(*(uint8_t*)0x20000236 = 0);
  NONFAILING(*(uint8_t*)0x20000237 = 0);
  NONFAILING(*(uint32_t*)0x20000238 = 0);
  NONFAILING(*(uint16_t*)0x2000023c = 0);
  NONFAILING(*(uint8_t*)0x2000023e = 0);
  NONFAILING(*(uint8_t*)0x2000023f = 0);
  NONFAILING(*(uint32_t*)0x20000240 = 0);
  NONFAILING(*(uint16_t*)0x20000244 = 0);
  NONFAILING(*(uint8_t*)0x20000246 = 0);
  NONFAILING(*(uint8_t*)0x20000247 = 0);
  NONFAILING(*(uint32_t*)0x20000248 = 0);
  NONFAILING(*(uint16_t*)0x2000024c = 0);
  NONFAILING(*(uint8_t*)0x2000024e = 0);
  NONFAILING(*(uint8_t*)0x2000024f = 0);
  NONFAILING(*(uint32_t*)0x20000250 = 0);
  NONFAILING(*(uint16_t*)0x20000254 = 0);
  NONFAILING(*(uint8_t*)0x20000256 = 0);
  NONFAILING(*(uint8_t*)0x20000257 = 0);
  NONFAILING(*(uint32_t*)0x20000258 = 0);
  NONFAILING(*(uint16_t*)0x2000025c = 0);
  NONFAILING(*(uint8_t*)0x2000025e = 0);
  NONFAILING(*(uint8_t*)0x2000025f = 0);
  NONFAILING(*(uint32_t*)0x20000260 = 0);
  NONFAILING(*(uint16_t*)0x20000264 = 0);
  NONFAILING(*(uint8_t*)0x20000266 = 0);
  NONFAILING(*(uint8_t*)0x20000267 = 0);
  NONFAILING(*(uint32_t*)0x20000268 = 0);
  NONFAILING(*(uint16_t*)0x2000026c = 0);
  NONFAILING(*(uint8_t*)0x2000026e = 0);
  NONFAILING(*(uint8_t*)0x2000026f = 0);
  NONFAILING(*(uint32_t*)0x20000270 = 0);
  NONFAILING(*(uint16_t*)0x20000274 = 0);
  NONFAILING(*(uint8_t*)0x20000276 = 0);
  NONFAILING(*(uint8_t*)0x20000277 = 0);
  NONFAILING(*(uint32_t*)0x20000278 = 0);
  NONFAILING(*(uint16_t*)0x2000027c = 0);
  NONFAILING(*(uint8_t*)0x2000027e = 0);
  NONFAILING(*(uint8_t*)0x2000027f = 0);
  NONFAILING(*(uint32_t*)0x20000280 = 0);
  NONFAILING(*(uint16_t*)0x20000284 = 0);
  NONFAILING(*(uint8_t*)0x20000286 = 0);
  NONFAILING(*(uint8_t*)0x20000287 = 0);
  NONFAILING(*(uint32_t*)0x20000288 = 0);
  NONFAILING(*(uint16_t*)0x2000028c = 0);
  NONFAILING(*(uint8_t*)0x2000028e = 0);
  NONFAILING(*(uint8_t*)0x2000028f = 0);
  NONFAILING(*(uint32_t*)0x20000290 = 0);
  NONFAILING(*(uint16_t*)0x20000294 = 0);
  NONFAILING(*(uint8_t*)0x20000296 = 0);
  NONFAILING(*(uint8_t*)0x20000297 = 0);
  NONFAILING(*(uint32_t*)0x20000298 = 0);
  NONFAILING(*(uint16_t*)0x2000029c = 0);
  NONFAILING(*(uint8_t*)0x2000029e = 0);
  NONFAILING(*(uint8_t*)0x2000029f = 0);
  NONFAILING(*(uint32_t*)0x200002a0 = 0);
  NONFAILING(*(uint16_t*)0x200002a4 = 0);
  NONFAILING(*(uint8_t*)0x200002a6 = 0);
  NONFAILING(*(uint8_t*)0x200002a7 = 0);
  NONFAILING(*(uint32_t*)0x200002a8 = 0);
  NONFAILING(*(uint16_t*)0x200002ac = 0);
  NONFAILING(*(uint8_t*)0x200002ae = 0);
  NONFAILING(*(uint8_t*)0x200002af = 0);
  NONFAILING(*(uint32_t*)0x200002b0 = 0);
  NONFAILING(*(uint16_t*)0x200002b4 = 0);
  NONFAILING(*(uint8_t*)0x200002b6 = 0);
  NONFAILING(*(uint8_t*)0x200002b7 = 0);
  NONFAILING(*(uint32_t*)0x200002b8 = 0);
  NONFAILING(*(uint16_t*)0x200002bc = 0);
  NONFAILING(*(uint8_t*)0x200002be = 0);
  NONFAILING(*(uint8_t*)0x200002bf = 0);
  NONFAILING(*(uint32_t*)0x200002c0 = 0);
  NONFAILING(*(uint16_t*)0x200002c4 = 0);
  NONFAILING(*(uint8_t*)0x200002c6 = 0);
  NONFAILING(*(uint8_t*)0x200002c7 = 0);
  NONFAILING(*(uint32_t*)0x200002c8 = 0);
  NONFAILING(*(uint16_t*)0x200002cc = 0);
  NONFAILING(*(uint8_t*)0x200002ce = 0);
  NONFAILING(*(uint8_t*)0x200002cf = 0);
  NONFAILING(*(uint32_t*)0x200002d0 = 0);
  NONFAILING(*(uint16_t*)0x200002d4 = 0);
  NONFAILING(*(uint8_t*)0x200002d6 = 0);
  NONFAILING(*(uint8_t*)0x200002d7 = 0);
  NONFAILING(*(uint32_t*)0x200002d8 = 0);
  NONFAILING(*(uint16_t*)0x200002dc = 0);
  NONFAILING(*(uint8_t*)0x200002de = 0);
  NONFAILING(*(uint8_t*)0x200002df = 0);
  NONFAILING(*(uint32_t*)0x200002e0 = 0);
  NONFAILING(*(uint16_t*)0x200002e4 = 0);
  NONFAILING(*(uint8_t*)0x200002e6 = 0);
  NONFAILING(*(uint8_t*)0x200002e7 = 0);
  NONFAILING(*(uint32_t*)0x200002e8 = 0);
  NONFAILING(*(uint16_t*)0x200002ec = 0);
  NONFAILING(*(uint8_t*)0x200002ee = 0);
  NONFAILING(*(uint8_t*)0x200002ef = 0);
  NONFAILING(*(uint32_t*)0x200002f0 = 0);
  NONFAILING(*(uint16_t*)0x200002f4 = 0);
  NONFAILING(*(uint8_t*)0x200002f6 = 0);
  NONFAILING(*(uint8_t*)0x200002f7 = 0);
  NONFAILING(*(uint32_t*)0x200002f8 = 0);
  NONFAILING(*(uint16_t*)0x200002fc = 0);
  NONFAILING(*(uint8_t*)0x200002fe = 0);
  NONFAILING(*(uint8_t*)0x200002ff = 0);
  NONFAILING(*(uint32_t*)0x20000300 = 0);
  NONFAILING(*(uint16_t*)0x20000304 = 0);
  NONFAILING(*(uint8_t*)0x20000306 = 0);
  NONFAILING(*(uint8_t*)0x20000307 = 0);
  NONFAILING(*(uint32_t*)0x20000308 = 0);
  NONFAILING(*(uint16_t*)0x2000030c = 0);
  NONFAILING(*(uint8_t*)0x2000030e = 0);
  NONFAILING(*(uint8_t*)0x2000030f = 0);
  NONFAILING(*(uint32_t*)0x20000310 = 0);
  NONFAILING(*(uint16_t*)0x20000314 = 0);
  NONFAILING(*(uint8_t*)0x20000316 = 0);
  NONFAILING(*(uint8_t*)0x20000317 = 0);
  NONFAILING(*(uint32_t*)0x20000318 = 0);
  NONFAILING(*(uint16_t*)0x2000031c = 0);
  NONFAILING(*(uint8_t*)0x2000031e = 0);
  NONFAILING(*(uint8_t*)0x2000031f = 0);
  NONFAILING(*(uint32_t*)0x20000320 = 0);
  NONFAILING(*(uint16_t*)0x20000324 = 0);
  NONFAILING(*(uint8_t*)0x20000326 = 0);
  NONFAILING(*(uint8_t*)0x20000327 = 0);
  NONFAILING(*(uint32_t*)0x20000328 = 0);
  NONFAILING(*(uint16_t*)0x2000032c = 0);
  NONFAILING(*(uint8_t*)0x2000032e = 0);
  NONFAILING(*(uint8_t*)0x2000032f = 0);
  NONFAILING(*(uint32_t*)0x20000330 = 0);
  NONFAILING(*(uint16_t*)0x20000334 = 0);
  NONFAILING(*(uint8_t*)0x20000336 = 0);
  NONFAILING(*(uint8_t*)0x20000337 = 0);
  NONFAILING(*(uint32_t*)0x20000338 = 0);
  NONFAILING(*(uint16_t*)0x2000033c = 0);
  NONFAILING(*(uint8_t*)0x2000033e = 0);
  NONFAILING(*(uint8_t*)0x2000033f = 0);
  NONFAILING(*(uint32_t*)0x20000340 = 0);
  NONFAILING(*(uint16_t*)0x20000344 = 0);
  NONFAILING(*(uint8_t*)0x20000346 = 0);
  NONFAILING(*(uint8_t*)0x20000347 = 0);
  NONFAILING(*(uint32_t*)0x20000348 = 0);
  NONFAILING(*(uint16_t*)0x2000034c = 0);
  NONFAILING(*(uint8_t*)0x2000034e = 0);
  NONFAILING(*(uint8_t*)0x2000034f = 0);
  NONFAILING(*(uint32_t*)0x20000350 = 0);
  NONFAILING(*(uint16_t*)0x20000354 = 0);
  NONFAILING(*(uint8_t*)0x20000356 = 0);
  NONFAILING(*(uint8_t*)0x20000357 = 0);
  NONFAILING(*(uint32_t*)0x20000358 = 0);
  NONFAILING(*(uint16_t*)0x2000035c = 0);
  NONFAILING(*(uint8_t*)0x2000035e = 0);
  NONFAILING(*(uint8_t*)0x2000035f = 0);
  NONFAILING(*(uint32_t*)0x20000360 = 0);
  NONFAILING(*(uint16_t*)0x20000364 = 0);
  NONFAILING(*(uint8_t*)0x20000366 = 0);
  NONFAILING(*(uint8_t*)0x20000367 = 0);
  NONFAILING(*(uint32_t*)0x20000368 = 0);
  NONFAILING(*(uint16_t*)0x2000036c = 0);
  NONFAILING(*(uint8_t*)0x2000036e = 0);
  NONFAILING(*(uint8_t*)0x2000036f = 0);
  NONFAILING(*(uint32_t*)0x20000370 = 0);
  NONFAILING(*(uint16_t*)0x20000374 = 0);
  NONFAILING(*(uint8_t*)0x20000376 = 0);
  NONFAILING(*(uint8_t*)0x20000377 = 0);
  NONFAILING(*(uint32_t*)0x20000378 = 0);
  NONFAILING(*(uint16_t*)0x2000037c = 0);
  NONFAILING(*(uint8_t*)0x2000037e = 0);
  NONFAILING(*(uint8_t*)0x2000037f = 0);
  NONFAILING(*(uint32_t*)0x20000380 = 0);
  NONFAILING(*(uint16_t*)0x20000384 = 0);
  NONFAILING(*(uint8_t*)0x20000386 = 0);
  NONFAILING(*(uint8_t*)0x20000387 = 0);
  NONFAILING(*(uint32_t*)0x20000388 = 0);
  NONFAILING(*(uint16_t*)0x2000038c = 0);
  NONFAILING(*(uint8_t*)0x2000038e = 0);
  NONFAILING(*(uint8_t*)0x2000038f = 0);
  NONFAILING(*(uint32_t*)0x20000390 = 0);
  NONFAILING(*(uint16_t*)0x20000394 = 0);
  NONFAILING(*(uint8_t*)0x20000396 = 0);
  NONFAILING(*(uint8_t*)0x20000397 = 0);
  NONFAILING(*(uint32_t*)0x20000398 = 0);
  NONFAILING(*(uint16_t*)0x2000039c = 0);
  NONFAILING(*(uint8_t*)0x2000039e = 0);
  NONFAILING(*(uint8_t*)0x2000039f = 0);
  NONFAILING(*(uint32_t*)0x200003a0 = 0);
  NONFAILING(*(uint16_t*)0x200003a4 = 0);
  NONFAILING(*(uint8_t*)0x200003a6 = 0);
  NONFAILING(*(uint8_t*)0x200003a7 = 0);
  NONFAILING(*(uint32_t*)0x200003a8 = 0);
  NONFAILING(*(uint16_t*)0x200003ac = 0);
  NONFAILING(*(uint8_t*)0x200003ae = 0);
  NONFAILING(*(uint8_t*)0x200003af = 0);
  NONFAILING(*(uint32_t*)0x200003b0 = 0);
  NONFAILING(*(uint16_t*)0x200003b4 = 0);
  NONFAILING(*(uint8_t*)0x200003b6 = 0);
  NONFAILING(*(uint8_t*)0x200003b7 = 0);
  NONFAILING(*(uint32_t*)0x200003b8 = 0);
  NONFAILING(*(uint16_t*)0x200003bc = 0);
  NONFAILING(*(uint8_t*)0x200003be = 0);
  NONFAILING(*(uint8_t*)0x200003bf = 0);
  NONFAILING(*(uint32_t*)0x200003c0 = 0);
  NONFAILING(*(uint16_t*)0x200003c4 = 0);
  NONFAILING(*(uint8_t*)0x200003c6 = 0);
  NONFAILING(*(uint8_t*)0x200003c7 = 0);
  NONFAILING(*(uint32_t*)0x200003c8 = 0);
  NONFAILING(*(uint64_t*)0x200003d0 = 0);
  NONFAILING(memcpy((void*)0x200003d8, "\x43\x4f\x4e\x4e\x53\x45\x43\x4d\x41"
                                       "\x52\x4b\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00",
                    32));
  NONFAILING(*(uint32_t*)0x200003f8 = 8);
  NONFAILING(*(uint8_t*)0x20000400 = 0);
  NONFAILING(*(uint32_t*)0x20000408 = 0);
  NONFAILING(memcpy((void*)0x2000040c, "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00",
                    32));
  NONFAILING(*(uint32_t*)0x2000042c = 1);
  NONFAILING(*(uint32_t*)0x20000430 = -1);
  NONFAILING(*(uint32_t*)0x20000434 = 0);
  NONFAILING(*(uint32_t*)0x20000438 = 0);
  NONFAILING(memcpy((void*)0x2000043c, "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00",
                    32));
  NONFAILING(*(uint32_t*)0x2000045c = 1);
  NONFAILING(*(uint32_t*)0x20000460 = -1);
  NONFAILING(*(uint32_t*)0x20000464 = 0);
  syscall(__NR_setsockopt, r[0], 0, 0x80, 0x200004c0, 0x3e0);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  install_segv_handler();
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
