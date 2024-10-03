// https://syzkaller.appspot.com/bug?id=d85346ac6940ddf44c049024b2e4059bfd58dc85
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

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

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

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  *(uint32_t*)0x200009c0 = 0x19;
  *(uint32_t*)0x200009c4 = 4;
  *(uint32_t*)0x200009c8 = 8;
  *(uint32_t*)0x200009cc = 8;
  *(uint32_t*)0x200009d0 = 0;
  *(uint32_t*)0x200009d4 = -1;
  *(uint32_t*)0x200009d8 = 0;
  memset((void*)0x200009dc, 0, 16);
  *(uint32_t*)0x200009ec = 0;
  *(uint32_t*)0x200009f0 = -1;
  *(uint32_t*)0x200009f4 = 0;
  *(uint32_t*)0x200009f8 = 0;
  *(uint32_t*)0x200009fc = 0;
  *(uint64_t*)0x20000a00 = 0;
  res = syscall(__NR_bpf, /*cmd=*/0ul, /*arg=*/0x200009c0ul, /*size=*/0x48ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x200000c0 = 6;
  *(uint32_t*)0x200000c4 = 0x14;
  *(uint64_t*)0x200000c8 = 0x20000540;
  memcpy((void*)0x20000540,
         "\x18\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x18"
         "\x01\x00\x00\x75\x6c\x00\x00\x00\x00\x00\x00\x00\x20\x20\x20\x7b\x1a"
         "\xf8\xff\x00\x00\x00\x00\xbf\xa1\x00\x00\x00\x00\x00\x00\x07\x01\x00"
         "\x00\xf8\xff\xff\xff\xb7\x02\x00\x00\x08\x00\x00\x00\xb7\x03\x00\x00"
         "\x00\x00\x00\x00\x85\x00\x00\x00\x06\x00\x00\x00\x18\x11\x00\x00",
         84);
  *(uint32_t*)0x20000594 = r[0];
  memcpy((void*)0x20000598,
         "\x00\x00\x00\x00\x00\x00\x00\x00\xb7\x08\x00\x00\x00\x00\x00\x00\x7b"
         "\x8a\xf8\xff\x00\x00\x00\x00\xbc\xa2\x00\x00\x00\x00\x00\x00\xa6\x02"
         "\x00\x00\xf8\xff\xff\xff\xb7\x03\x00\x00\x08\x00\x00\x00\xb7\x04\x00"
         "\x00\x00\x00\x00\x00\x85\x00\x00\x00\x33\x00\x00\x00\x95",
         65);
  *(uint64_t*)0x200000d0 = 0x20000180;
  memcpy((void*)0x20000180, "GPL\000", 4);
  *(uint32_t*)0x200000d8 = 0;
  *(uint32_t*)0x200000dc = 0;
  *(uint64_t*)0x200000e0 = 0;
  *(uint32_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000ec = 0;
  memset((void*)0x200000f0, 0, 16);
  *(uint32_t*)0x20000100 = 0;
  *(uint32_t*)0x20000104 = 0;
  *(uint32_t*)0x20000108 = -1;
  *(uint32_t*)0x2000010c = 0;
  *(uint64_t*)0x20000110 = 0;
  *(uint32_t*)0x20000118 = 0;
  *(uint32_t*)0x2000011c = 0;
  *(uint64_t*)0x20000120 = 0;
  *(uint32_t*)0x20000128 = 0;
  *(uint32_t*)0x2000012c = 0;
  *(uint32_t*)0x20000130 = 0;
  *(uint32_t*)0x20000134 = 0;
  *(uint64_t*)0x20000138 = 0;
  *(uint64_t*)0x20000140 = 0;
  *(uint32_t*)0x20000148 = 0;
  *(uint32_t*)0x2000014c = 0;
  res = syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x200000c0ul, /*size=*/0x90ul);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x20000500 = r[1];
  *(uint32_t*)0x20000504 = -1;
  *(uint32_t*)0x20000508 = 0;
  syscall(__NR_bpf, /*cmd=*/0xaul, /*arg=*/0x20000500ul, /*size=*/0xcul);
  *(uint32_t*)0x20002c80 = 3;
  *(uint32_t*)0x20002c84 = 0xc;
  *(uint64_t*)0x20002c88 = 0x20000140;
  *(uint8_t*)0x20000140 = 0x18;
  STORE_BY_BITMASK(uint8_t, , 0x20000141, 2, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000141, 0, 4, 4);
  *(uint16_t*)0x20000142 = 0;
  *(uint32_t*)0x20000144 = 0;
  *(uint8_t*)0x20000148 = 0;
  *(uint8_t*)0x20000149 = 0;
  *(uint16_t*)0x2000014a = 0;
  *(uint32_t*)0x2000014c = 0;
  *(uint8_t*)0x20000150 = 0x85;
  *(uint8_t*)0x20000151 = 0;
  *(uint16_t*)0x20000152 = 0;
  *(uint32_t*)0x20000154 = 0x17;
  *(uint8_t*)0x20000158 = 0x18;
  STORE_BY_BITMASK(uint8_t, , 0x20000159, 1, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000159, 0, 4, 4);
  *(uint16_t*)0x2000015a = 0;
  *(uint32_t*)0x2000015c = 0x256c6c75;
  *(uint8_t*)0x20000160 = 0;
  *(uint8_t*)0x20000161 = 0;
  *(uint16_t*)0x20000162 = 0;
  *(uint32_t*)0x20000164 = 0x20202000;
  STORE_BY_BITMASK(uint8_t, , 0x20000168, 3, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000168, 3, 3, 2);
  STORE_BY_BITMASK(uint8_t, , 0x20000168, 3, 5, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000169, 0xa, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000169, 1, 4, 4);
  *(uint16_t*)0x2000016a = 0xfff8;
  *(uint32_t*)0x2000016c = 0;
  STORE_BY_BITMASK(uint8_t, , 0x20000170, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000170, 1, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000170, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000171, 1, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000171, 0xa, 4, 4);
  *(uint16_t*)0x20000172 = 0;
  *(uint32_t*)0x20000174 = 0;
  STORE_BY_BITMASK(uint8_t, , 0x20000178, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000178, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000178, 0, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000179, 1, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000179, 0, 4, 4);
  *(uint16_t*)0x2000017a = 0;
  *(uint32_t*)0x2000017c = 0xfffffff8;
  STORE_BY_BITMASK(uint8_t, , 0x20000180, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000180, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000180, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000181, 2, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000181, 0, 4, 4);
  *(uint16_t*)0x20000182 = 0;
  *(uint32_t*)0x20000184 = 8;
  STORE_BY_BITMASK(uint8_t, , 0x20000188, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000188, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000188, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000189, 3, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000189, 0, 4, 4);
  *(uint16_t*)0x2000018a = 0;
  *(uint32_t*)0x2000018c = 0;
  *(uint8_t*)0x20000190 = 0x85;
  *(uint8_t*)0x20000191 = 0;
  *(uint16_t*)0x20000192 = 0;
  *(uint32_t*)0x20000194 = 0x7b;
  *(uint8_t*)0x20000198 = 0x95;
  *(uint8_t*)0x20000199 = 0;
  *(uint16_t*)0x2000019a = 0;
  *(uint32_t*)0x2000019c = 0;
  *(uint64_t*)0x20002c90 = 0x20000040;
  memcpy((void*)0x20000040, "GPL\000", 4);
  *(uint32_t*)0x20002c98 = 0;
  *(uint32_t*)0x20002c9c = 0;
  *(uint64_t*)0x20002ca0 = 0;
  *(uint32_t*)0x20002ca8 = 0;
  *(uint32_t*)0x20002cac = 0;
  memset((void*)0x20002cb0, 0, 16);
  *(uint32_t*)0x20002cc0 = 0;
  *(uint32_t*)0x20002cc4 = 0;
  *(uint32_t*)0x20002cc8 = -1;
  *(uint32_t*)0x20002ccc = 8;
  *(uint64_t*)0x20002cd0 = 0;
  *(uint32_t*)0x20002cd8 = 0;
  *(uint32_t*)0x20002cdc = 0x10;
  *(uint64_t*)0x20002ce0 = 0;
  *(uint32_t*)0x20002ce8 = 0;
  *(uint32_t*)0x20002cec = 0;
  *(uint32_t*)0x20002cf0 = 0;
  *(uint32_t*)0x20002cf4 = 0;
  *(uint64_t*)0x20002cf8 = 0;
  *(uint64_t*)0x20002d00 = 0;
  *(uint32_t*)0x20002d08 = 0x10;
  *(uint32_t*)0x20002d0c = 0;
  res = syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x20002c80ul, /*size=*/0x90ul);
  if (res != -1)
    r[2] = res;
  *(uint32_t*)0x200002c0 = r[2];
  *(uint32_t*)0x200002c4 = 0;
  *(uint32_t*)0x200002c8 = 0xe;
  *(uint32_t*)0x200002cc = 0x44;
  *(uint64_t*)0x200002d0 = 0x20000100;
  memcpy((void*)0x20000100,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x51\x22\x9d\xc9", 14);
  *(uint64_t*)0x200002d8 = 0;
  *(uint32_t*)0x200002e0 = 0;
  *(uint32_t*)0x200002e4 = 0;
  *(uint32_t*)0x200002e8 = 0;
  *(uint32_t*)0x200002ec = 0;
  *(uint64_t*)0x200002f0 = 0;
  *(uint64_t*)0x200002f8 = 0;
  *(uint32_t*)0x20000300 = 0;
  *(uint32_t*)0x20000304 = 0;
  *(uint32_t*)0x20000308 = 0;
  syscall(__NR_bpf, /*cmd=*/0xaul, /*arg=*/0x200002c0ul, /*size=*/0x50ul);
  *(uint32_t*)0x20000080 = -1;
  *(uint32_t*)0x20000084 = 0x2a0;
  *(uint32_t*)0x20000088 = 3;
  *(uint32_t*)0x2000008c = 0;
  *(uint64_t*)0x20000090 = 0x20000000;
  memcpy((void*)0x20000000, "\xb9\xff\x06", 3);
  *(uint64_t*)0x20000098 = 0;
  *(uint32_t*)0x200000a0 = 0;
  *(uint32_t*)0x200000a4 = 0x60000000;
  *(uint32_t*)0x200000a8 = 0;
  *(uint32_t*)0x200000ac = 0;
  *(uint64_t*)0x200000b0 = 0;
  *(uint64_t*)0x200000b8 = 0;
  *(uint32_t*)0x200000c0 = 0;
  *(uint32_t*)0x200000c4 = 0;
  *(uint32_t*)0x200000c8 = 0;
  syscall(__NR_bpf, /*cmd=*/0xaul, /*arg=*/0x20000080ul, /*size=*/0x50ul);
  *(uint32_t*)0x200000c0 = 6;
  *(uint32_t*)0x200000c4 = 0xb;
  *(uint64_t*)0x200000c8 = 0x200002c0;
  memcpy((void*)0x200002c0,
         "\x18\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x18"
         "\x01\x00\x00\x20\x20\x70\x25\x00\x00\x00\x00\x00\x20\x20\x20\x7b\x1a"
         "\xf8\xff\x00\x00\x00\x00\xbf\xa1\x00\x00\x00\x00\x00\x00\x07\x01\x00"
         "\x00\xf8\xff\xff\xff\xb7\x02\x00\x00\x08\x00\x00\x00\xb7\x03\x00\x00"
         "\x00\x00\x00\x20\x85\x00\x00\x00\x72\x00\x00\x00\x95",
         81);
  *(uint64_t*)0x200000d0 = 0x20000200;
  memcpy((void*)0x20000200, "GPL\000", 4);
  *(uint32_t*)0x200000d8 = 0;
  *(uint32_t*)0x200000dc = 0;
  *(uint64_t*)0x200000e0 = 0;
  *(uint32_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000ec = 0;
  memset((void*)0x200000f0, 0, 16);
  *(uint32_t*)0x20000100 = 0;
  *(uint32_t*)0x20000104 = 0;
  *(uint32_t*)0x20000108 = -1;
  *(uint32_t*)0x2000010c = 0;
  *(uint64_t*)0x20000110 = 0;
  *(uint32_t*)0x20000118 = 0;
  *(uint32_t*)0x2000011c = 0;
  *(uint64_t*)0x20000120 = 0;
  *(uint32_t*)0x20000128 = 0;
  *(uint32_t*)0x2000012c = 0;
  *(uint32_t*)0x20000130 = 0;
  *(uint32_t*)0x20000134 = 0;
  *(uint64_t*)0x20000138 = 0;
  *(uint64_t*)0x20000140 = 0;
  *(uint32_t*)0x20000148 = 0;
  *(uint32_t*)0x2000014c = 0;
  res = syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x200000c0ul, /*size=*/0x90ul);
  if (res != -1)
    r[3] = res;
  *(uint32_t*)0x20000240 = r[3];
  *(uint32_t*)0x20000244 = 0;
  *(uint32_t*)0x20000248 = 0;
  *(uint32_t*)0x2000024c = 0;
  *(uint64_t*)0x20000250 = 0;
  *(uint64_t*)0x20000258 = 0;
  *(uint32_t*)0x20000260 = 0;
  *(uint32_t*)0x20000264 = 0;
  *(uint32_t*)0x20000268 = 0;
  *(uint32_t*)0x2000026c = 0;
  *(uint64_t*)0x20000270 = 0;
  *(uint64_t*)0x20000278 = 0;
  *(uint32_t*)0x20000280 = 2;
  *(uint32_t*)0x20000284 = 0;
  *(uint32_t*)0x20000288 = 0;
  syscall(__NR_bpf, /*cmd=*/0xaul, /*arg=*/0x20000240ul, /*size=*/0x50ul);
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
  const char* reason;
  (void)reason;
  loop();
  return 0;
}
