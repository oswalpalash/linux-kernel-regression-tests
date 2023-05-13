// https://syzkaller.appspot.com/bug?id=4492e9b46eeb9887285ea0a61a5d5d91d12d0148
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
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

uint64_t r[1] = {0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0xaul, 1ul, 0ul);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000000, "raw\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000",
         32);
  *(uint32_t*)0x20000020 = 0x3c1;
  *(uint32_t*)0x20000024 = 3;
  *(uint32_t*)0x20000028 = 0x508;
  *(uint32_t*)0x2000002c = 0x240;
  *(uint32_t*)0x20000030 = 0x240;
  *(uint32_t*)0x20000034 = 0x240;
  *(uint32_t*)0x20000038 = 0;
  *(uint32_t*)0x2000003c = 0x23b;
  *(uint32_t*)0x20000040 = 0x438;
  *(uint32_t*)0x20000044 = 0x438;
  *(uint32_t*)0x20000048 = 0x438;
  *(uint32_t*)0x2000004c = 0x438;
  *(uint32_t*)0x20000050 = 0x438;
  *(uint32_t*)0x20000054 = 3;
  *(uint64_t*)0x20000058 = 0;
  *(uint8_t*)0x20000060 = 0;
  *(uint8_t*)0x20000061 = 0;
  *(uint8_t*)0x20000062 = 0;
  *(uint8_t*)0x20000063 = 0;
  *(uint8_t*)0x20000064 = 0;
  *(uint8_t*)0x20000065 = 0;
  *(uint8_t*)0x20000066 = 0;
  *(uint8_t*)0x20000067 = 0;
  *(uint8_t*)0x20000068 = 0;
  *(uint8_t*)0x20000069 = 0;
  *(uint8_t*)0x2000006a = 0;
  *(uint8_t*)0x2000006b = 0;
  *(uint8_t*)0x2000006c = 0;
  *(uint8_t*)0x2000006d = 0;
  *(uint8_t*)0x2000006e = 0;
  *(uint8_t*)0x2000006f = 0;
  *(uint8_t*)0x20000070 = 0;
  *(uint8_t*)0x20000071 = 0;
  *(uint8_t*)0x20000072 = 0;
  *(uint8_t*)0x20000073 = 0;
  *(uint8_t*)0x20000074 = 0;
  *(uint8_t*)0x20000075 = 0;
  *(uint8_t*)0x20000076 = 0;
  *(uint8_t*)0x20000077 = 0;
  *(uint8_t*)0x20000078 = 0;
  *(uint8_t*)0x20000079 = 0;
  *(uint8_t*)0x2000007a = 0;
  *(uint8_t*)0x2000007b = 0;
  *(uint8_t*)0x2000007c = 0;
  *(uint8_t*)0x2000007d = 0;
  *(uint8_t*)0x2000007e = 0;
  *(uint8_t*)0x2000007f = 0;
  *(uint8_t*)0x20000080 = 0;
  *(uint8_t*)0x20000081 = 0;
  *(uint8_t*)0x20000082 = 0;
  *(uint8_t*)0x20000083 = 0;
  *(uint8_t*)0x20000084 = 0;
  *(uint8_t*)0x20000085 = 0;
  *(uint8_t*)0x20000086 = 0;
  *(uint8_t*)0x20000087 = 0;
  *(uint8_t*)0x20000088 = 0;
  *(uint8_t*)0x20000089 = 0;
  *(uint8_t*)0x2000008a = 0;
  *(uint8_t*)0x2000008b = 0;
  *(uint8_t*)0x2000008c = 0;
  *(uint8_t*)0x2000008d = 0;
  *(uint8_t*)0x2000008e = 0;
  *(uint8_t*)0x2000008f = 0;
  *(uint8_t*)0x20000090 = 0;
  *(uint8_t*)0x20000091 = 0;
  *(uint8_t*)0x20000092 = 0;
  *(uint8_t*)0x20000093 = 0;
  *(uint8_t*)0x20000094 = 0;
  *(uint8_t*)0x20000095 = 0;
  *(uint8_t*)0x20000096 = 0;
  *(uint8_t*)0x20000097 = 0;
  *(uint8_t*)0x20000098 = 0;
  *(uint8_t*)0x20000099 = 0;
  *(uint8_t*)0x2000009a = 0;
  *(uint8_t*)0x2000009b = 0;
  *(uint8_t*)0x2000009c = 0;
  *(uint8_t*)0x2000009d = 0;
  *(uint8_t*)0x2000009e = 0;
  *(uint8_t*)0x2000009f = 0;
  *(uint8_t*)0x200000a0 = 0;
  *(uint8_t*)0x200000a1 = 0;
  *(uint8_t*)0x200000a2 = 0;
  *(uint8_t*)0x200000a3 = 0;
  *(uint8_t*)0x200000a4 = 0;
  *(uint8_t*)0x200000a5 = 0;
  *(uint8_t*)0x200000a6 = 0;
  *(uint8_t*)0x200000a7 = 0;
  *(uint8_t*)0x200000a8 = 0;
  *(uint8_t*)0x200000a9 = 0;
  *(uint8_t*)0x200000aa = 0;
  *(uint8_t*)0x200000ab = 0;
  *(uint8_t*)0x200000ac = 0;
  *(uint8_t*)0x200000ad = 0;
  *(uint8_t*)0x200000ae = 0;
  *(uint8_t*)0x200000af = 0;
  *(uint8_t*)0x200000b0 = 0;
  *(uint8_t*)0x200000b1 = 0;
  *(uint8_t*)0x200000b2 = 0;
  *(uint8_t*)0x200000b3 = 0;
  *(uint8_t*)0x200000b4 = 0;
  *(uint8_t*)0x200000b5 = 0;
  *(uint8_t*)0x200000b6 = 0;
  *(uint8_t*)0x200000b7 = 0;
  *(uint8_t*)0x200000b8 = 0;
  *(uint8_t*)0x200000b9 = 0;
  *(uint8_t*)0x200000ba = 0;
  *(uint8_t*)0x200000bb = 0;
  *(uint8_t*)0x200000bc = 0;
  *(uint8_t*)0x200000bd = 0;
  *(uint8_t*)0x200000be = 0;
  *(uint8_t*)0x200000bf = 0;
  *(uint8_t*)0x200000c0 = 0;
  *(uint8_t*)0x200000c1 = 0;
  *(uint8_t*)0x200000c2 = 0;
  *(uint8_t*)0x200000c3 = 0;
  *(uint8_t*)0x200000c4 = 0;
  *(uint8_t*)0x200000c5 = 0;
  *(uint8_t*)0x200000c6 = 0;
  *(uint8_t*)0x200000c7 = 0;
  *(uint8_t*)0x200000c8 = 0;
  *(uint8_t*)0x200000c9 = 0;
  *(uint8_t*)0x200000ca = 0;
  *(uint8_t*)0x200000cb = 0;
  *(uint8_t*)0x200000cc = 0;
  *(uint8_t*)0x200000cd = 0;
  *(uint8_t*)0x200000ce = 0;
  *(uint8_t*)0x200000cf = 0;
  *(uint8_t*)0x200000d0 = 0;
  *(uint8_t*)0x200000d1 = 0;
  *(uint8_t*)0x200000d2 = 0;
  *(uint8_t*)0x200000d3 = 0;
  *(uint8_t*)0x200000d4 = 0;
  *(uint8_t*)0x200000d5 = 0;
  *(uint8_t*)0x200000d6 = 0;
  *(uint8_t*)0x200000d7 = 0;
  *(uint8_t*)0x200000d8 = 0;
  *(uint8_t*)0x200000d9 = 0;
  *(uint8_t*)0x200000da = 0;
  *(uint8_t*)0x200000db = 0;
  *(uint8_t*)0x200000dc = 0;
  *(uint8_t*)0x200000dd = 0;
  *(uint8_t*)0x200000de = 0;
  *(uint8_t*)0x200000df = 0;
  *(uint8_t*)0x200000e0 = 6;
  *(uint8_t*)0x200000e1 = 0;
  *(uint8_t*)0x200000e2 = 0;
  *(uint8_t*)0x200000e3 = 0;
  *(uint8_t*)0x200000e4 = 0;
  *(uint8_t*)0x200000e5 = 0;
  *(uint8_t*)0x200000e6 = 0;
  *(uint8_t*)0x200000e7 = 0;
  *(uint32_t*)0x200000e8 = 0;
  *(uint16_t*)0x200000ec = 0x220;
  *(uint16_t*)0x200000ee = 0x240;
  *(uint32_t*)0x200000f0 = 0;
  *(uint64_t*)0x200000f8 = 0;
  *(uint64_t*)0x20000100 = 0;
  *(uint16_t*)0x20000108 = 0x150;
  memcpy((void*)0x2000010a, "hashlimit\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000",
         29);
  *(uint8_t*)0x20000127 = 2;
  memcpy((void*)0x20000128,
         "veth1_virt_"
         "wifi\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000",
         255);
  *(uint64_t*)0x20000228 = 0;
  *(uint64_t*)0x20000230 = 7;
  *(uint32_t*)0x20000238 = 0;
  *(uint32_t*)0x2000023c = 0x14030000;
  *(uint32_t*)0x20000240 = 0;
  *(uint32_t*)0x20000244 = 5;
  *(uint32_t*)0x20000248 = 9;
  *(uint8_t*)0x2000024c = 0;
  *(uint8_t*)0x2000024d = 0;
  *(uint64_t*)0x20000250 = 0;
  *(uint16_t*)0x20000258 = 0x28;
  memcpy((void*)0x2000025a, "rpfilter\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000",
         29);
  *(uint8_t*)0x20000277 = 0;
  *(uint8_t*)0x20000278 = 0;
  *(uint16_t*)0x20000280 = 0x20;
  memcpy((void*)0x20000282, "TRACE\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000",
         29);
  *(uint8_t*)0x2000029f = 0;
  *(uint8_t*)0x200002a0 = 0;
  *(uint8_t*)0x200002a1 = 0;
  *(uint8_t*)0x200002a2 = 0;
  *(uint8_t*)0x200002a3 = 0;
  *(uint8_t*)0x200002a4 = 0;
  *(uint8_t*)0x200002a5 = 0;
  *(uint8_t*)0x200002a6 = 0;
  *(uint8_t*)0x200002a7 = 0;
  *(uint8_t*)0x200002a8 = 0;
  *(uint8_t*)0x200002a9 = 0;
  *(uint8_t*)0x200002aa = 0;
  *(uint8_t*)0x200002ab = 0;
  *(uint8_t*)0x200002ac = 0;
  *(uint8_t*)0x200002ad = 0;
  *(uint8_t*)0x200002ae = 0;
  *(uint8_t*)0x200002af = 0;
  *(uint8_t*)0x200002b0 = 0;
  *(uint8_t*)0x200002b1 = 0;
  *(uint8_t*)0x200002b2 = 0;
  *(uint8_t*)0x200002b3 = 0;
  *(uint8_t*)0x200002b4 = 0;
  *(uint8_t*)0x200002b5 = 0;
  *(uint8_t*)0x200002b6 = 0;
  *(uint8_t*)0x200002b7 = 0;
  *(uint8_t*)0x200002b8 = 0;
  *(uint8_t*)0x200002b9 = 0;
  *(uint8_t*)0x200002ba = 0;
  *(uint8_t*)0x200002bb = 0;
  *(uint8_t*)0x200002bc = 0;
  *(uint8_t*)0x200002bd = 0;
  *(uint8_t*)0x200002be = 0;
  *(uint8_t*)0x200002bf = 0;
  *(uint8_t*)0x200002c0 = 0;
  *(uint8_t*)0x200002c1 = 0;
  *(uint8_t*)0x200002c2 = 0;
  *(uint8_t*)0x200002c3 = 0;
  *(uint8_t*)0x200002c4 = 0;
  *(uint8_t*)0x200002c5 = 0;
  *(uint8_t*)0x200002c6 = 0;
  *(uint8_t*)0x200002c7 = 0;
  *(uint8_t*)0x200002c8 = 0;
  *(uint8_t*)0x200002c9 = 0;
  *(uint8_t*)0x200002ca = 0;
  *(uint8_t*)0x200002cb = 0;
  *(uint8_t*)0x200002cc = 0;
  *(uint8_t*)0x200002cd = 0;
  *(uint8_t*)0x200002ce = 0;
  *(uint8_t*)0x200002cf = 0;
  *(uint8_t*)0x200002d0 = 0;
  *(uint8_t*)0x200002d1 = 0;
  *(uint8_t*)0x200002d2 = 0;
  *(uint8_t*)0x200002d3 = 0;
  *(uint8_t*)0x200002d4 = 0;
  *(uint8_t*)0x200002d5 = 0;
  *(uint8_t*)0x200002d6 = 0;
  *(uint8_t*)0x200002d7 = 0;
  *(uint8_t*)0x200002d8 = 0;
  *(uint8_t*)0x200002d9 = 0;
  *(uint8_t*)0x200002da = 0;
  *(uint8_t*)0x200002db = 0;
  *(uint8_t*)0x200002dc = 0;
  *(uint8_t*)0x200002dd = 0;
  *(uint8_t*)0x200002de = 0;
  *(uint8_t*)0x200002df = 0;
  *(uint8_t*)0x200002e0 = 0;
  *(uint8_t*)0x200002e1 = 0;
  *(uint8_t*)0x200002e2 = 0;
  *(uint8_t*)0x200002e3 = 0;
  *(uint8_t*)0x200002e4 = 0;
  *(uint8_t*)0x200002e5 = 0;
  *(uint8_t*)0x200002e6 = 0;
  *(uint8_t*)0x200002e7 = 0;
  *(uint8_t*)0x200002e8 = 0;
  *(uint8_t*)0x200002e9 = 0;
  *(uint8_t*)0x200002ea = 0;
  *(uint8_t*)0x200002eb = 0;
  *(uint8_t*)0x200002ec = 0;
  *(uint8_t*)0x200002ed = 0;
  *(uint8_t*)0x200002ee = 0;
  *(uint8_t*)0x200002ef = 0;
  *(uint8_t*)0x200002f0 = 0;
  *(uint8_t*)0x200002f1 = 0;
  *(uint8_t*)0x200002f2 = 0;
  *(uint8_t*)0x200002f3 = 0;
  *(uint8_t*)0x200002f4 = 0;
  *(uint8_t*)0x200002f5 = 0;
  *(uint8_t*)0x200002f6 = 0;
  *(uint8_t*)0x200002f7 = 0;
  *(uint8_t*)0x200002f8 = 0;
  *(uint8_t*)0x200002f9 = 0;
  *(uint8_t*)0x200002fa = 0;
  *(uint8_t*)0x200002fb = 0;
  *(uint8_t*)0x200002fc = 0;
  *(uint8_t*)0x200002fd = 0;
  *(uint8_t*)0x200002fe = 0;
  *(uint8_t*)0x200002ff = 0;
  *(uint8_t*)0x20000300 = 0;
  *(uint8_t*)0x20000301 = 0;
  *(uint8_t*)0x20000302 = 0;
  *(uint8_t*)0x20000303 = 0;
  *(uint8_t*)0x20000304 = 0;
  *(uint8_t*)0x20000305 = 0;
  *(uint8_t*)0x20000306 = 0;
  *(uint8_t*)0x20000307 = 0;
  *(uint8_t*)0x20000308 = 0;
  *(uint8_t*)0x20000309 = 0;
  *(uint8_t*)0x2000030a = 0;
  *(uint8_t*)0x2000030b = 0;
  *(uint8_t*)0x2000030c = 0;
  *(uint8_t*)0x2000030d = 0;
  *(uint8_t*)0x2000030e = 0;
  *(uint8_t*)0x2000030f = 0;
  *(uint8_t*)0x20000310 = 0;
  *(uint8_t*)0x20000311 = 0;
  *(uint8_t*)0x20000312 = 0;
  *(uint8_t*)0x20000313 = 0;
  *(uint8_t*)0x20000314 = 0;
  *(uint8_t*)0x20000315 = 0;
  *(uint8_t*)0x20000316 = 0;
  *(uint8_t*)0x20000317 = 0;
  *(uint8_t*)0x20000318 = 0;
  *(uint8_t*)0x20000319 = 0;
  *(uint8_t*)0x2000031a = 0;
  *(uint8_t*)0x2000031b = 0;
  *(uint8_t*)0x2000031c = 0;
  *(uint8_t*)0x2000031d = 0;
  *(uint8_t*)0x2000031e = 0;
  *(uint8_t*)0x2000031f = 0;
  *(uint8_t*)0x20000320 = 0;
  *(uint8_t*)0x20000321 = 0;
  *(uint8_t*)0x20000322 = 0;
  *(uint8_t*)0x20000323 = 0;
  *(uint8_t*)0x20000324 = 0;
  *(uint8_t*)0x20000325 = 0;
  *(uint8_t*)0x20000326 = 0;
  *(uint8_t*)0x20000327 = 0;
  *(uint32_t*)0x20000328 = 0;
  *(uint16_t*)0x2000032c = 0xd0;
  *(uint16_t*)0x2000032e = 0x1f8;
  *(uint32_t*)0x20000330 = 0;
  *(uint64_t*)0x20000338 = 0;
  *(uint64_t*)0x20000340 = 0;
  *(uint16_t*)0x20000348 = 0x28;
  memcpy((void*)0x2000034a, "ipv6header\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000",
         29);
  *(uint8_t*)0x20000367 = 0;
  *(uint8_t*)0x20000368 = 0;
  *(uint8_t*)0x20000369 = 0;
  *(uint8_t*)0x2000036a = 0;
  *(uint16_t*)0x20000370 = 0x128;
  memcpy((void*)0x20000372, "SECMARK\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000",
         29);
  *(uint8_t*)0x2000038f = 0;
  *(uint8_t*)0x20000390 = 0;
  *(uint32_t*)0x20000394 = 0;
  memcpy((void*)0x20000398,
         "unconfined_u:system_r:insmod_t:s0-s0:c0."
         "c1023\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000",
         256);
  *(uint8_t*)0x20000498 = 0;
  *(uint8_t*)0x20000499 = 0;
  *(uint8_t*)0x2000049a = 0;
  *(uint8_t*)0x2000049b = 0;
  *(uint8_t*)0x2000049c = 0;
  *(uint8_t*)0x2000049d = 0;
  *(uint8_t*)0x2000049e = 0;
  *(uint8_t*)0x2000049f = 0;
  *(uint8_t*)0x200004a0 = 0;
  *(uint8_t*)0x200004a1 = 0;
  *(uint8_t*)0x200004a2 = 0;
  *(uint8_t*)0x200004a3 = 0;
  *(uint8_t*)0x200004a4 = 0;
  *(uint8_t*)0x200004a5 = 0;
  *(uint8_t*)0x200004a6 = 0;
  *(uint8_t*)0x200004a7 = 0;
  *(uint8_t*)0x200004a8 = 0;
  *(uint8_t*)0x200004a9 = 0;
  *(uint8_t*)0x200004aa = 0;
  *(uint8_t*)0x200004ab = 0;
  *(uint8_t*)0x200004ac = 0;
  *(uint8_t*)0x200004ad = 0;
  *(uint8_t*)0x200004ae = 0;
  *(uint8_t*)0x200004af = 0;
  *(uint8_t*)0x200004b0 = 0;
  *(uint8_t*)0x200004b1 = 0;
  *(uint8_t*)0x200004b2 = 0;
  *(uint8_t*)0x200004b3 = 0;
  *(uint8_t*)0x200004b4 = 0;
  *(uint8_t*)0x200004b5 = 0;
  *(uint8_t*)0x200004b6 = 0;
  *(uint8_t*)0x200004b7 = 0;
  *(uint8_t*)0x200004b8 = 0;
  *(uint8_t*)0x200004b9 = 0;
  *(uint8_t*)0x200004ba = 0;
  *(uint8_t*)0x200004bb = 0;
  *(uint8_t*)0x200004bc = 0;
  *(uint8_t*)0x200004bd = 0;
  *(uint8_t*)0x200004be = 0;
  *(uint8_t*)0x200004bf = 0;
  *(uint8_t*)0x200004c0 = 0;
  *(uint8_t*)0x200004c1 = 0;
  *(uint8_t*)0x200004c2 = 0;
  *(uint8_t*)0x200004c3 = 0;
  *(uint8_t*)0x200004c4 = 0;
  *(uint8_t*)0x200004c5 = 0;
  *(uint8_t*)0x200004c6 = 0;
  *(uint8_t*)0x200004c7 = 0;
  *(uint8_t*)0x200004c8 = 0;
  *(uint8_t*)0x200004c9 = 0;
  *(uint8_t*)0x200004ca = 0;
  *(uint8_t*)0x200004cb = 0;
  *(uint8_t*)0x200004cc = 0;
  *(uint8_t*)0x200004cd = 0;
  *(uint8_t*)0x200004ce = 0;
  *(uint8_t*)0x200004cf = 0;
  *(uint8_t*)0x200004d0 = 0;
  *(uint8_t*)0x200004d1 = 0;
  *(uint8_t*)0x200004d2 = 0;
  *(uint8_t*)0x200004d3 = 0;
  *(uint8_t*)0x200004d4 = 0;
  *(uint8_t*)0x200004d5 = 0;
  *(uint8_t*)0x200004d6 = 0;
  *(uint8_t*)0x200004d7 = 0;
  *(uint8_t*)0x200004d8 = 0;
  *(uint8_t*)0x200004d9 = 0;
  *(uint8_t*)0x200004da = 0;
  *(uint8_t*)0x200004db = 0;
  *(uint8_t*)0x200004dc = 0;
  *(uint8_t*)0x200004dd = 0;
  *(uint8_t*)0x200004de = 0;
  *(uint8_t*)0x200004df = 0;
  *(uint8_t*)0x200004e0 = 0;
  *(uint8_t*)0x200004e1 = 0;
  *(uint8_t*)0x200004e2 = 0;
  *(uint8_t*)0x200004e3 = 0;
  *(uint8_t*)0x200004e4 = 0;
  *(uint8_t*)0x200004e5 = 0;
  *(uint8_t*)0x200004e6 = 0;
  *(uint8_t*)0x200004e7 = 0;
  *(uint8_t*)0x200004e8 = 0;
  *(uint8_t*)0x200004e9 = 0;
  *(uint8_t*)0x200004ea = 0;
  *(uint8_t*)0x200004eb = 0;
  *(uint8_t*)0x200004ec = 0;
  *(uint8_t*)0x200004ed = 0;
  *(uint8_t*)0x200004ee = 0;
  *(uint8_t*)0x200004ef = 0;
  *(uint8_t*)0x200004f0 = 0;
  *(uint8_t*)0x200004f1 = 0;
  *(uint8_t*)0x200004f2 = 0;
  *(uint8_t*)0x200004f3 = 0;
  *(uint8_t*)0x200004f4 = 0;
  *(uint8_t*)0x200004f5 = 0;
  *(uint8_t*)0x200004f6 = 0;
  *(uint8_t*)0x200004f7 = 0;
  *(uint8_t*)0x200004f8 = 0;
  *(uint8_t*)0x200004f9 = 0;
  *(uint8_t*)0x200004fa = 0;
  *(uint8_t*)0x200004fb = 0;
  *(uint8_t*)0x200004fc = 0;
  *(uint8_t*)0x200004fd = 0;
  *(uint8_t*)0x200004fe = 0;
  *(uint8_t*)0x200004ff = 0;
  *(uint8_t*)0x20000500 = 0;
  *(uint8_t*)0x20000501 = 0;
  *(uint8_t*)0x20000502 = 0;
  *(uint8_t*)0x20000503 = 0;
  *(uint8_t*)0x20000504 = 0;
  *(uint8_t*)0x20000505 = 0;
  *(uint8_t*)0x20000506 = 0;
  *(uint8_t*)0x20000507 = 0;
  *(uint8_t*)0x20000508 = 0;
  *(uint8_t*)0x20000509 = 0;
  *(uint8_t*)0x2000050a = 0;
  *(uint8_t*)0x2000050b = 0;
  *(uint8_t*)0x2000050c = 0;
  *(uint8_t*)0x2000050d = 0;
  *(uint8_t*)0x2000050e = 0;
  *(uint8_t*)0x2000050f = 0;
  *(uint8_t*)0x20000510 = 0;
  *(uint8_t*)0x20000511 = 0;
  *(uint8_t*)0x20000512 = 0;
  *(uint8_t*)0x20000513 = 0;
  *(uint8_t*)0x20000514 = 0;
  *(uint8_t*)0x20000515 = 0;
  *(uint8_t*)0x20000516 = 0;
  *(uint8_t*)0x20000517 = 0;
  *(uint8_t*)0x20000518 = 0;
  *(uint8_t*)0x20000519 = 0;
  *(uint8_t*)0x2000051a = 0;
  *(uint8_t*)0x2000051b = 0;
  *(uint8_t*)0x2000051c = 0;
  *(uint8_t*)0x2000051d = 0;
  *(uint8_t*)0x2000051e = 0;
  *(uint8_t*)0x2000051f = 0;
  *(uint32_t*)0x20000520 = 0;
  *(uint16_t*)0x20000524 = 0xa8;
  *(uint16_t*)0x20000526 = 0xd0;
  *(uint32_t*)0x20000528 = 0;
  *(uint64_t*)0x20000530 = 0;
  *(uint64_t*)0x20000538 = 0;
  *(uint16_t*)0x20000540 = 0x28;
  memcpy((void*)0x20000542, "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000",
         29);
  *(uint8_t*)0x2000055f = 0;
  *(uint32_t*)0x20000560 = 0xfffffffe;
  syscall(__NR_setsockopt, r[0], 0x29ul, 0x40ul, 0x20000000ul, 0x568ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
