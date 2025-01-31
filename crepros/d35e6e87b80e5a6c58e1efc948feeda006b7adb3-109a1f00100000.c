// https://syzkaller.appspot.com/bug?id=d35e6e87b80e5a6c58e1efc948feeda006b7adb3
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

#ifndef __NR_bind
#define __NR_bind 361
#endif
#ifndef __NR_getsockname
#define __NR_getsockname 367
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_sendmsg
#define __NR_sendmsg 370
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[3] = {0xffffffffffffffff, 0x0, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x11, 3, 0);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20000080 = 0x11;
  memcpy((void*)0x20000082,
         "\x00\x00\x01\x00\x00\x00\x00\x00\x08\x00\x44\x94\x4e\xeb\xa7\x1a\x49"
         "\x76\xe2\x52\x92\x2c\xb1\x8f\x6e\x2e\x2a\xba\x00\x00\x00\x01\x2e\x0b"
         "\x38\x36\x00\x54\x04\xb0\xe0\x30\x1a\x4c\xe8\x75\xf2\xe3\xff\x5f\x16"
         "\x3e\xe3\x40\xb7\x67\x95\x00\x80\x00\x00\x00\x00\x00\x00\x01\x01\x01"
         "\x3c\x58\x11\x03\x9e\x15\x77\x50\x27\xec\xce\x66\xfd\x79\x2b\xbf\x0e"
         "\x5b\xf5\xff\x1b\x08\x16\xf3\xf6\xdb\x1c\x00\x01\x00\x00\x00\x00\x00"
         "\x00\x00\x49\x74\x00\x00\x00\x00\x00\x00\x00\x06\xad\x8e\x5e\xcc\x32"
         "\x6d\x3a\x09\xff\xc2\xc6\x54",
         126);
  syscall(__NR_bind, (intptr_t)r[0], 0x20000080, 0x80);
  *(uint32_t*)0x20000000 = 0x14;
  res = syscall(__NR_getsockname, (intptr_t)r[0], 0x200003c0, 0x20000000);
  if (res != -1)
    r[1] = *(uint32_t*)0x200003c4;
  res = syscall(__NR_socket, 0x10, 3, 0);
  if (res != -1)
    r[2] = res;
  *(uint32_t*)0x200007c0 = 0;
  *(uint32_t*)0x200007c4 = 0;
  *(uint32_t*)0x200007c8 = 0x20000780;
  *(uint32_t*)0x20000780 = 0x20000880;
  *(uint32_t*)0x20000880 = 0x2b4;
  *(uint16_t*)0x20000884 = 0x24;
  *(uint16_t*)0x20000886 = 0xf0b;
  *(uint32_t*)0x20000888 = 0;
  *(uint32_t*)0x2000088c = 0;
  *(uint8_t*)0x20000890 = 0;
  *(uint8_t*)0x20000891 = 0;
  *(uint16_t*)0x20000892 = 0;
  *(uint32_t*)0x20000894 = r[1];
  *(uint16_t*)0x20000898 = 0;
  *(uint16_t*)0x2000089a = 0;
  *(uint16_t*)0x2000089c = -1;
  *(uint16_t*)0x2000089e = -1;
  *(uint16_t*)0x200008a0 = 0;
  *(uint16_t*)0x200008a2 = 0;
  *(uint16_t*)0x200008a4 = 0x28;
  STORE_BY_BITMASK(uint16_t, , 0x200008a6, 8, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200008a7, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200008a7, 1, 7, 1);
  *(uint16_t*)0x200008a8 = 0x1c;
  *(uint16_t*)0x200008aa = 1;
  *(uint8_t*)0x200008ac = 0;
  *(uint8_t*)0x200008ad = 0;
  *(uint16_t*)0x200008ae = 0;
  *(uint32_t*)0x200008b0 = 0;
  *(uint32_t*)0x200008b4 = 0;
  *(uint32_t*)0x200008b8 = 0;
  *(uint32_t*)0x200008bc = -1;
  *(uint32_t*)0x200008c0 = 1;
  *(uint16_t*)0x200008c4 = 6;
  *(uint16_t*)0x200008c6 = 2;
  *(uint16_t*)0x200008c8 = 0;
  *(uint16_t*)0x200008cc = 0xa;
  *(uint16_t*)0x200008ce = 1;
  memcpy((void*)0x200008d0, "pfifo\000", 6);
  *(uint16_t*)0x200008d8 = 8;
  *(uint16_t*)0x200008da = 2;
  *(uint32_t*)0x200008dc = 0;
  *(uint16_t*)0x200008e0 = 0xa;
  *(uint16_t*)0x200008e2 = 1;
  memcpy((void*)0x200008e4, "choke\000", 6);
  *(uint16_t*)0x200008ec = 0x248;
  *(uint16_t*)0x200008ee = 2;
  *(uint16_t*)0x200008f0 = 8;
  *(uint16_t*)0x200008f2 = 3;
  *(uint32_t*)0x200008f4 = 8;
  *(uint16_t*)0x200008f8 = 0x104;
  *(uint16_t*)0x200008fa = 2;
  memcpy((void*)0x200008fc,
         "\x47\xa6\x5d\xff\x41\x46\x0f\x2d\x33\x10\xa2\x1d\x7b\x79\x93\x33\xf1"
         "\x1b\x15\x37\x16\x63\xf5\xb4\x3d\x22\x01\xca\xe5\x70\x54\xea\xa0\x9c"
         "\x0b\x37\x61\x2a\x2b\x52\x82\x62\xb8\x7f\x95\x0f\x52\x53\x5a\xec\x6e"
         "\xb1\xf2\xda\x11\xdc\x8a\x62\x7f\xdb\x1c\x9c\x7a\x1b\x39\x09\xe1\x03"
         "\x36\xdc\x12\x50\x9e\x88\x3e\x96\x84\x2c\x05\x1c\x64\x57\x50\xbd\x75"
         "\x83\x9d\x3c\xef\xbf\xab\xf4\xf2\xf7\x56\x55\x21\x2e\x2d\x7f\x9e\xdd"
         "\x59\xb5\xe5\xd0\x38\xd6\x7c\xa9\xb2\xab\xb4\xf1\xc2\xfb\xbc\x15\xe9"
         "\x7e\x4f\xf2\xc8\x13\x53\x0e\xa3\xb5\xab\xb5\x8e\x1d\xb2\x23\xb7\x76"
         "\x0b\x32\x3d\x29\x1b\x12\xea\xe9\xd1\x96\xb4\x48\xce\x86\x94\x3a\x59"
         "\x38\x11\x6c\x01\x73\xa3\x0f\xeb\x7a\x98\xfd\x73\xe0\xe2\x6b\x7a\x0c"
         "\x76\xeb\x23\x2d\xec\xb4\x77\xc7\xc8\x28\x55\x90\xbc\xe8\x66\xc7\xcd"
         "\x27\xf1\x9b\x05\xc4\xb0\x27\x51\xda\xcc\xc7\x67\xc3\x27\x96\xc9\xd3"
         "\x74\x39\xaf\x7c\xc1\x47\x20\xc2\xd5\x9d\xf1\xc0\xe7\x75\x42\xb4\xd4"
         "\x42\x8e\x39\x34\x99\x8a\x8b\xd0\xf5\x6a\x1b\x4f\xed\x4f\xa5\xea\x10"
         "\x69\x0a\xb6\x3e\x96\x58\x19\x7a\xf9\x94\xbe\xd5\x8c\xe7\x3c\x99\x45"
         "\x06",
         256);
  *(uint16_t*)0x200009fc = 8;
  *(uint16_t*)0x200009fe = 3;
  *(uint32_t*)0x20000a00 = 0x538b;
  *(uint16_t*)0x20000a04 = 8;
  *(uint16_t*)0x20000a06 = 3;
  *(uint32_t*)0x20000a08 = 0xff;
  *(uint16_t*)0x20000a0c = 8;
  *(uint16_t*)0x20000a0e = 3;
  *(uint32_t*)0x20000a10 = 8;
  *(uint16_t*)0x20000a14 = 8;
  *(uint16_t*)0x20000a16 = 3;
  *(uint32_t*)0x20000a18 = 9;
  *(uint16_t*)0x20000a1c = 0x104;
  *(uint16_t*)0x20000a1e = 2;
  memcpy((void*)0x20000a20,
         "\xf8\x4b\x96\x6a\xa0\xa5\xeb\x61\x18\x29\x81\x98\x35\x1f\x2f\x79\x02"
         "\xf2\xfc\x69\x11\x0f\x4d\x92\xca\x8b\x57\xe3\x00\x5e\xf0\x59\x85\x84"
         "\x0e\xdf\x64\x60\x13\x74\x02\x0a\xa9\x26\x6c\x52\x7d\xda\xee\x2b\x5e"
         "\x23\xb5\x6b\x64\x61\xfc\xd9\x0a\xea\x87\x7b\x80\xd5\xff\x28\x21\x2f"
         "\x46\xba\xa2\x2b\x20\x02\x69\x68\x95\x24\x58\x7e\x97\x56\xf6\x69\x3d"
         "\x7b\x79\x48\xd0\x99\x4a\xa5\x09\xc2\xd7\x61\x4f\x3f\x86\xe1\x73\x22"
         "\x7c\x1f\xae\x69\x2f\x2b\xd0\x47\x36\x2b\xde\xf1\x2c\x1d\x8f\xf2\x6c"
         "\x79\x78\xec\x9a\xa6\xa7\x41\x0c\x61\x7f\xcc\x11\x83\xb2\x86\x8e\x56"
         "\xf2\xcf\xc1\x9d\x2f\x95\xe2\xae\x8e\xc4\x4b\xf4\x0f\xf3\xcd\x4e\xcb"
         "\x4f\xe7\x55\x57\x9c\xed\x53\x88\x6c\xf5\xb5\xa8\x86\xc3\x1f\xbd\x6c"
         "\x32\x53\x52\x7c\xfc\x19\xe0\x03\x4f\x3a\xb0\x12\x41\x45\x13\x55\xdd"
         "\x9f\xba\x70\xc2\x76\x9a\x1c\xad\x95\xe0\x5d\xd4\x4d\xc1\xd7\xe7\x12"
         "\x14\xf7\x6d\x03\x2c\x76\x42\x92\x7b\xb4\xc2\x07\xda\xe9\xc2\xaa\x35"
         "\xeb\x4f\x3a\x19\x3c\x64\xa6\x01\xd7\x5e\x1d\xee\x31\xf2\x23\x91\x2e"
         "\xf7\xc3\xe5\x39\xdc\x16\x37\x6c\xba\xe5\x4c\x5a\xb9\xd9\xa3\x0d\x97"
         "\xfa",
         256);
  *(uint16_t*)0x20000b20 = 0x14;
  *(uint16_t*)0x20000b22 = 1;
  *(uint32_t*)0x20000b24 = 0;
  *(uint32_t*)0x20000b28 = 0;
  *(uint32_t*)0x20000b2c = 1;
  *(uint8_t*)0x20000b30 = 0x1b;
  *(uint8_t*)0x20000b31 = 0xf;
  *(uint8_t*)0x20000b32 = 0x14;
  *(uint8_t*)0x20000b33 = 1;
  *(uint32_t*)0x20000784 = 0x2b4;
  *(uint32_t*)0x200007cc = 1;
  *(uint32_t*)0x200007d0 = 0;
  *(uint32_t*)0x200007d4 = 0;
  *(uint32_t*)0x200007d8 = 0;
  syscall(__NR_sendmsg, (intptr_t)r[2], 0x200007c0, 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000, 0x1000, 0, 0x32, -1, 0);
  syscall(__NR_mmap, 0x20000000, 0x1000000, 7, 0x32, -1, 0);
  syscall(__NR_mmap, 0x21000000, 0x1000, 0, 0x32, -1, 0);
  loop();
  return 0;
}
