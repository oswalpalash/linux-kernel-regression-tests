// https://syzkaller.appspot.com/bug?id=1ec500baf19f5376d2162d62557d0394359ce056
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

#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_openat
#define __NR_openat 295
#endif
#ifndef __NR_pipe
#define __NR_pipe 42
#endif
#ifndef __NR_vmsplice
#define __NR_vmsplice 316
#endif
#ifndef __NR_write
#define __NR_write 4
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[1] = {0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  NONFAILING(memcpy((void*)0x20000080, "/dev/uhid\000", 10));
  res = syscall(__NR_openat, 0xffffff9c, 0x20000080, 2, 0);
  if (res != -1)
    r[0] = res;
  NONFAILING(memcpy(
      (void*)0x200000c0,
      "\x0b\x00\x00\x00\x73\x79\x7a\x31\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x8a\x8e\x87\x4b\x73\xe9\x90\xb4"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x2b"
      "\x40\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x01\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\x07\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x73\x79"
      "\x7a\x31\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\xcf\x0a\x00\x00\x00\x00\x00\x00\x00\x00\x0c\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x16\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x3b\x38\xe9\x67\xac\x82\x06\xea"
      "\xae\x86\xb9\x7e\xec\x0b\x2b\xed\x1e\xe2\x33\x64\xb1\x0d\x6a\xad\x51\x02"
      "\x00\x00\x00\xe2\xa5\x0d\xee\x4a\xfc\x66\xd2\x44\x28\x05\x20\x1c\x39\x38"
      "\x9a\x80\x4c\x41\xc2\x99\x3f\xc6\x7e\x8a\x14\x60\x45\xe1\x4a\x8a\x08\x00"
      "\x55\x0e\x6a\x25\xc0\xef\x65\xf6\x00\x00\x00\x00\x00\x00\x00\x03\x18\x7f"
      "\xaf\xa4\xa1\xee\x6e\xce\x53\xc6\x73\x85\xb8\x83\xa3\x6a\xd2\x4a\x04\x00"
      "\x00\x00\x00\x00\x00\x00\x6a\x8a\xb1\x1b\x0a\x0b\x00\xe7\x7e\x6c\x16\x18"
      "\x9c\xd3\x16\xcb\xe0\x1a\x4c\xe4\x11\x37\x8e\xaa\xb7\x37\x2d\xab\x5e\xef"
      "\x84\xc3\x1b\x2d\xad\x86\x8a\x53\xe6\xf5\xe6\x97\x46\xa7\x1e\xc9\x2d\xca"
      "\xa9\xa7\xdf\xab\x39\x42\x86\xe5\xc8\x1e\xae\x45\xe3\x00\x00\xde\xe1\x32"
      "\x5b\xe2\x0b\x88\x56\xe0\x74\x94\x2b\x8d\xa1\x1e\xdb\x57\x8b\x45\x3a\xca"
      "\xc0\x3a\x9d\x34\x48\x00\x00\x00\x00\x83\xd6\xd5\xfe\x4f\x83\x3d\x4d\x4c"
      "\xfb\xee\xf0\xe0\xe6\x2b\xe2\x05\x00\x00\x00\x3c\x32\x98\x4c\x6c\x4b\x2b"
      "\x9c\x33\xd8\xa6\x24\xce\xa9\x5c\x1b\x3b\x3c\x6d\xd8\x73\x56\x9c\xf4\x78"
      "\x6f\xc5\x16\x6b\x03\x00\x00\x00\x00\x00\x1f\xf2\x8d\x3c\xe3\xe3\xb8\xf8"
      "\x1e\x34\xcf\x97\xc9\xc8\x41\xcb\x2e\xf0\x81\x07\xa9\xa9\x65\x49\xe3\xd2"
      "\x59\xdf\x17\xe2\x9e\xd6\x4b\xd6\x12\x08\x13\xf9\xf0\x34\x4e\x13\x95\x06"
      "\x70\x1e\x8f\xde\xdb\x06\x00\x9b\x5e\x4d\x0c\x67\xbd\xa0\xb9\x28\xb7\x32"
      "\xcf\xf7\x82\xb0\x68\x40\x75\xf2\xcb\x78\x51\xef\xdd\x77\x97\xee\x95\xd2"
      "\xac\x28\xa8\xca\xbd\x26\xc1\x56\x82\xaa\x78\xd3\x1a\xec\x95\x6b\xd7\xc2"
      "\x78\x06\x40\x34\x34\xb3\xc3\x0b\x07\x0b\xcc\x82\x66\xe1\x2f\xa6\x66\x02"
      "\x05\x62\x56\xf7\x46\x75\xb7\xcb\x4d\x87\xb2\x70\x8d\x70\xc8\xca\xf8\xfe"
      "\x18\x0c\x4d\xea\x3f\x5b\x7a\x87\x1b\x30\xc7\xa5\x75\x3b\x48\xf7\xf0\x91"
      "\x92\xa3\x4b\x0e\xfa\xab\x02\xdc\xa0\x51\x7e\xee\x50\xff\x30\x20\x6f\x78"
      "\xec\x82\xc7\x2f\x33\xe9\x09\xc1\xae\x1f\xe9\x4f\xe0\x75\x9f\x07\xd1\x64"
      "\x02\x59\x3e\x33\x35\xd4\x20\x00\x88\x8c\x90\x5f\xb4\x1d\xbd\xb4\x20\xea"
      "\x9f\x87\xfc\x86\x51\x26\x85\xeb\x72\x61\xe9\xc2\x5e\xdb\x60\x95\xe9\xec"
      "\x2b\x74\x3b\xcf\x0f\x46\x8b\xad\xa0\xb8\x7a\x02\x60\x65\x30\xc2\x78\xa4"
      "\xc6\x00\x00\x00\xee\x8e\x0e\x4c\x6a\x8d\xb6\x53\xd1\xa2\xf8\x5d\x26\xd0"
      "\xf7\x3a\x91\x2d\x5b\x07\x8e\x27\xc4\x54\xe2\xbb\xed\x02\x07\x3d\x48\xd5"
      "\x5b\xe2\x0a\xbc\x07\x83\x82\x93\xb0\xd1\x83\xf4\x75\x76\xe2\xaa\x51\x3c"
      "\x05\x1e\x73\xfd\xc8\x5b\x2a\x64\x90\xd4\xda\xa1\xcc\x01\x14\x3d\xc9\x06"
      "\xe3\xd7\xa9\xa9\x97\xcc\x78\x6b\x1f\x99\xb0\x81\x3b\x04\x24\x3f\xb8\xac"
      "\x55\x4f\x73\x1b\x79\x7c\x66\xd6\xdb\x9b\x41\x8f\x61\x8a\x9a\x0c\x92\x48"
      "\x90\x77\x7c\xdd\xd8\x6b\xef\x3c\x49\xff\x06\x61\xd4\x01\x46\xb6\xec\x5c"
      "\xdd\x1d\xa1\x51\xf0\x00\x7b\x7c\xef\x4c\xc3\x8d\xfd\x55\xae\xed\xd7\x6e"
      "\xc0\x73\x09\x78\x42\x09\x51\x5d\x75\xc8\x69\x73\x0d\xaf\x71\x8e\x24\x2c"
      "\xf7\x7a\x37\x98\x32\x4a\x5b\xc1\x1a\x3a\xed\x31\x26\xc5\x6c\xfe\x89\xd1"
      "\xb8\x60\x32\xbe\x37\x42\xfd\x28\x6b\xcb\x2b\xda\xed\x2d\x1f\x7e\x0d\xb3"
      "\x11\xcd\x43\xb7\x86\x06\xf7\xd5\xd0\xcb\x6f\x99\x4b\x57\x4d\xfd\x42\x11"
      "\xd1\xd8\x38\x35\x2d\xbf\x86\x84\x80\xfd\xaf\x8c\x97\x34\xd6\x31\xf1\xa0"
      "\x39",
      1009));
  syscall(__NR_write, (intptr_t)r[0], 0x200000c0, 0x12e);
  syscall(__NR_pipe, 0);
  syscall(__NR_vmsplice, -1, 0, 0, 0);
  NONFAILING(*(uint32_t*)0x200016c0 = 8);
  NONFAILING(memcpy(
      (void*)0x200016c4,
      "\x31\xfa\x40\xa4\xe2\x9b\xf9\xaa\xe1\x94\xa4\xea\xcf\x3d\x7a\x64\x96\xb6"
      "\x4b\xb4\x09\x4d\xde\xca\x02\x26\x3a\xc7\xef\x22\x55\xa3\x30\x46\x7c\x84"
      "\x72\xea\x92\xd1\x08\x6e\xe6\xf9\x02\xf3\x52\xe4\x0f\xf2\x4d\xc6\xe9\x7a"
      "\x8c\x11\xa2\xe0\x08\xa3\xfc\x2f\x66\x98\x08\x9b\x7b\xf8\xb2\x19\xaf\xbd"
      "\x4d\x0a\xaf\xbf\x7e\x79\xde\x0e\x0e\x10\x62\xe6\x51\x9b\x29\x95\x62\x90"
      "\x0f\x2d\x87\xa0\x71\x42\x86\xdd\x59\xba\xfb\xc1\x96\x68\x20\x62\x25\x88"
      "\xa8\x72\x33\x9f\x3f\xe1\x83\x3e\x23\xa4\xb1\x6c\xab\x7c\xa6\x26\x57\xdd"
      "\xc0\x9f\x87\x25\xf6\xae\x25\xee\xbb\x82\x9f\x38\x00\x4d\x9b\x3b\x55\xaa"
      "\x03\x03\xf8\x0d\x13\xc9\x8b\xed\xa2\xdc\xd1\xa6\xe9\x43\xf7\x0e\x45\x17"
      "\x76\x30\x7f\x70\x5e\x0b\x28\x31\x42\xfb\xf1\x91\xff\xf6\x77\xa4\x64\x2f"
      "\xbd\x39\x17\x50\xcb\xd1\x6a\xc2\xe9\x7d\x7a\xde\x08\xd8\x8e\xed\xe7\xfa"
      "\xa6\xd0\x81\xaf\x14\xc1\x69\x35\x83\xfb\x46\x02\x1c\x2f\x60\xa0\x0f\xa7"
      "\xf7\x24\x22\x08\x99\x25\xc8\x96\x08\x11\xc0\xc3\xb8\x2f\x8f\x21\xf3\xa9"
      "\x6e\xb4\xfd\xf4\x54\x92\xd3\x8a\xf3\xad\xd2\x90\x86\xda\xc8\x77\xa1\x50"
      "\x68\xb4\xe2\xb5\x6a\xd8\x98\x2a\xfb\xb3\x9d\x29\x0a\x0f\xdb\xdc\x27\x00"
      "\xa2\xd4\x31\xfe\x9e\x42\xfc\xd4\x9b\x1a\xb9\xcb\xda\x13\xf5\x56\x4f\x48"
      "\x24\x7c\x17\xbe\x9c\x45\xb9\x2d\xa2\x31\xaf\x0b\xd4\xf6\x1e\x3a\xd2\x14"
      "\x2e\xc9\x60\xa7\xe2\x39\x3a\xb0\x9f\x1c\x85\xc0\x92\x76\xef\x78\x41\x78"
      "\x5c\x65\xe1\xff\x6d\xfb\x1a\x87\x0b\x20\xf8\x40\x28\x37\xa1\x9b\xdf\x52"
      "\x1d\xc2\x36\xd4\x88\x0c\xfc\xcb\x9d\x7f\x10\xde\xb5\x9e\x10\xea\x19\x20"
      "\x00\xf7\x28\x0c\x4d\x28\x57\x0f\x6f\x63\x5c\xc8\x4a\x54\x41\x2a\xf0\x70"
      "\xb9\xae\x62\x4e\x34\x0c\x80\x14\xb5\x4d\x02\xf4\x8b\x91\x47\xd0\xf4\x0f"
      "\x46\x60\x1f\x70\x69\x51\xc0\xff\xae\x1a\x5e\x9f\x78\xe3\x4c\x88\x22\x88"
      "\x44\x92\xd3\x67\x4e\x39\x41\x06\xaf\xe2\x80\xf9\x3d\x9c\x2a\x4d\x80\xa9"
      "\xfb\xdf\xb1\x00\x4b\x12\x8f\x5f\x75\x09\xbc\x99\xe0\xde\x28\x7b\x3c\xa3"
      "\x0a\xfe\x36\x47\xd4\x02\xe6\x32\xca\xd7\x1a\x72\x98\xa2\x91\xda\x2b\x66"
      "\xaa\xfa\x71\x6b\xbd\x21\x80\x0a\x43\x78\xeb\x1e\x22\x78\xdb\xd0\x9a\xe0"
      "\xd8\xee\x95\x8b\x6c\x2b\x85\x6c\xd4\x91\xaa\x41\x9f\xd7\x7f\x70\x50\xe3"
      "\x6b\xd7\x9a\xe1\x1d\x0a\xee\x09\x0c\xb5\xdd\x2b\x78\xe0\xf6\x8f\x3d\x1f"
      "\x9b\x28\x02\xa2\x2d\xef\xf6\xf7\xce\xa2\xab\x46\x83\x1d\xcd\xba\x55\x95"
      "\xb6\xda\x10\x46\x31\xef\xb1\xe5\xbe\xd4\xa4\x1d\x50\x97\xbf\xe6\xc7\xcb"
      "\xd4\x7a\xce\x83\xbb\xb4\x8c\xee\x69\x26\x5f\x40\xe8\xff\x23\x47\x1f\xd1"
      "\x13\xf5\xe5\x65\xec\x34\x61\x6b\x44\x11\x59\xa5\xcc\x04\xbe\xa7\x8e\xce"
      "\x42\xd4\xaf\x99\x22\xee\xd0\x6c\xb2\x04\xd6\xc8\x8b\x29\x3a\x69\xdc\x83"
      "\x37\xd9\xfc\xfe\x57\xda\xbf\x58\x49\x76\x2f\x34\x35\x5b\x92\xbe\xeb\xef"
      "\xb4\x1c\x47\x3b\xbe\x18\x2f\xa3\xcc\x87\x56\xeb\xe8\xcf\xb3\xdd\x25\x4a"
      "\x98\xd6\x69\xbc\xa2\xb4\x33\x43\xff\xcd\xda\x9e\xc8\x8e\x05\x2e\x71\x4f"
      "\x74\xf0\x3e\xac\xf8\x28\x0b\xe5\x4a\x73\x19\x5e\xd7\xac\xaa\x28\xa2\xca"
      "\x01\x01\x97\x53\xa2\x35\x4e\x9f\xd5\x6a\x9c\x12\x72\x8f\x27\xc5\x3c\x9c"
      "\x90\x06\x87\x54\x9a\xac\xc2\xe1\x7a\xb0\xc1\x9b\x91\x65\x0b\x13\x7d\x67"
      "\x5d\xc0\xe1\x2d\xa2\xe7\xb3\x80\x7e\x8c\x59\x22\xb5\xdb\x11\x1f\xc8\x88"
      "\x89\x50\x54\x66\x7d\x11\xa4\xbe\x6f\x1b\x46\x99\x22\xee\x22\x1a\x85\xef"
      "\x99\x26\xcd\x7a\xca\x66\x31\x56\x6e\x7c\x7a\x1a\xa9\x21\x72\x1e\x48\xde"
      "\x7c\x79\x15\xad\xd3\x8b\x73\x1c\x01\x65\xae\x71\xe4\x9a\x65\x8f\x31\xc1"
      "\x60\xa7\xe2\x84\xa5\x7d\x81\x36\xc7\x85\xe5\xb6\x9c\x52\x17\x05\xcc\xe1"
      "\x7b\xbf\xcb\x3a\xd6\x56\x49\x4e\x1f\x11\x86\xce\x7c\x9c\x41\x72\x9f\x58"
      "\x0b\x90\x62\xdc\x8b\x8f\x46\x74\xe4\x1b\x10\x5d\xcf\xcd\xf5\xe5\xf3\x78"
      "\xda\xf3\x5c\x07\xa3\xb7\xf5\x72\x0e\x29\xe3\xe6\x61\xc2\x59\xaf\xb9\x28"
      "\x80\x27\xf8\x3d\x87\x4c\xc2\x2d\xfd\xab\x48\xda\x04\x79\x9c\xbc\x27\xd9"
      "\xab\xd8\x58\xd2\xf4\xb6\xca\x31\xee\xb4\xae\x98\xbe\x54\xb0\x4c\x19\x2f"
      "\x03\xa4\x1c\x56\x4d\xf7\x46\x92\x0e\x4e\xdd\x9e\x9c\x84\xb5\x48\x76\x95"
      "\x16\x5f\xfa\x38\xf3\xef\x7c\x34\x2b\x22\xbc\x40\x46\x36\x04\xaa\x8d\x73"
      "\x29\x8c\xd6\x80\x11\x36\xed\x21\x5f\xad\x81\x2a\xc5\x36\x21\x0e\x80\x33"
      "\x34\x9e\x95\x9f\x80\xa7\x31\x58\xd4\x5f\x8a\xee\x8b\x9c\x09\x9b\x9c\x74"
      "\xee\x93\xac\xff\x69\xe4\x1c\xe4\x01\x04\x0b\x6b\x21\x7a\x49\xf3\xb4\x3f"
      "\x20\xfe\x7b\x12\x0d\xe4\x22\x2e\x79\xfb\x51\x72\x9b\x9f\x82\xd5\x4b\x7b"
      "\x71\x88\x88\x38\x6b\xb3\x05\xf5\xf4\xf1\x46\x91\xa4\x8a\x2c\xf6\x56\x70"
      "\x53\xc6\xf5\x49\x08\x4d\x74\x08\x03\x5c\xa3\xee\x26\x76\xfa\x5a\x22\xbd"
      "\x30\xd8\x3d\xf2\x82\xd1\x46\xb3\x69\x9b\x98\x3e\x08\xb9\x70\x1b\xe2\x3a"
      "\x91\x17\xf4\x2b\x7f\xfc\xf7\x3e\x1f\x12\x10\xf3\xec\x90\xb0\x0e\xbe\xd4"
      "\x27\x4d\xec\xbf\x37\xb6\x58\xf8\xcf\x24\x1e\xba\xf1\x5e\xbc\x12\x27\xae"
      "\x9e\x25\xc5\xe0\x22\x0c\x1e\x2a\x67\xc5\xe5\x34\xff\x7b\x35\x69\xaf\x12"
      "\xf0\xe2\x82\xe6\xa9\x39\x32\x1b\x98\xec\x02\x4c\x49\x7e\x0e\x63\x64\x87"
      "\x9e\xb0\x20\x4e\xca\xa4\x84\x0b\x69\x90\xe7\x53\x41\xff\xb4\x15\xbf\xcd"
      "\xac\xd8\xbf\x6a\xad\xfd\xf0\x45\x4e\x89\x18\xb6\x7f\x83\x13\x8d\xd8\x6c"
      "\x00\xa6\x1d\x6d\xcb\x7b\x97\x36\xf6\x6a\xda\x2e\x1f\xf0\x94\x81\xea\xa2"
      "\x7d\xd8\x01\x5b\x70\x54\xdd\xc0\xf3\x3b\xdb\xba\xf0\xfe\x15\xf5\xd0\x26"
      "\x98\x5d\xdf\xdb\x90\xad\x48\x9c\x60\x27\x8b\x3b\x31\xb8\x03\xc2\x35\x78"
      "\xe9\x3f\x8c\x7e\x14\xc5\xd3\xb5\x03\xc3\x16\xc0\x4e\x3e\x0a\xd2\xd8\xc8"
      "\xb3\xcb\x18\xba\x1b\x02\xeb\x52\xb3\xd0\xc2\x27\x4e\xa4\x42\x8a\x36\xec"
      "\xa8\xe7\x83\x1f\xcb\x56\xb1\xd5\x72\x28\x5c\xeb\x4b\xc5\x63\xda\x4c\x7a"
      "\xfd\x57\x61\xbb\xfc\xa1\x96\x43\x5f\xb0\x1d\x96\xbb\x48\x93\x3b\xde\xce"
      "\x2b\xa4\x10\x29\x31\x91\x5b\x7a\xdd\x9b\xcd\x40\xf9\x33\xf5\xad\xca\x95"
      "\x07\xe4\x9e\xab\x89\xfe\x8d\x14\x37\x52\xc7\xe7\x02\xf5\xfe\xea\xe7\x12"
      "\x03\x96\xba\x0f\xa4\x4d\x4d\x34\xd7\x09\x5e\xe7\x9b\x99\x43\x40\xf6\x2d"
      "\x1f\x62\x92\x97\x68\x32\x58\x59\xa5\x7a\x12\x54\x13\xb7\xe0\xbf\xd9\x87"
      "\x79\x59\x04\x0d\xa2\x3f\xfb\xcb\x2d\x90\x49\xaf\xa3\x62\xae\x14\x29\x08"
      "\xf5\x22\x2e\xf2\x47\x30\x58\x8d\xd4\x98\xab\xc9\xad\x63\xf3\x2b\x03\xb4"
      "\xe0\xe5\xbd\x65\x35\x06\x04\x66\xf6\x9c\x11\x35\x71\x62\x12\x0c\x39\x8b"
      "\x6c\x09\x1c\xcb\xe9\x45\x38\x2a\x99\x32\xb8\xee\xe2\x23\x46\xe2\x60\x9f"
      "\x1c\x1e\xe0\xe4\xc2\x2d\xcb\x9b\x49\x8c\x1c\xea\xe2\xb4\x22\x7c\xf7\x38"
      "\x39\xe0\x14\x8c\x01\xe4\x41\xd2\x16\x37\x6f\xad\x63\x5c\xe2\xe9\x3c\x9c"
      "\x41\x93\x53\x96\x09\x28\xb6\x95\x26\xe5\x59\x87\x01\x37\x14\x89\x8b\xf8"
      "\x79\xff\xaa\xd7\xf4\x3b\x17\xd3\x1a\x83\x02\x1d\xf2\x1a\x65\x6b\x95\x58"
      "\xc9\x1e\x5a\xc2\x19\xf5\x59\xb7\x87\xa1\x9b\x2a\x65\xd8\x86\x49\xdd\x35"
      "\x79\xb3\x20\xd4\x0a\x3f\x5f\x12\x8c\xb5\x80\xff\x9b\x86\x78\x21\x46\x75"
      "\x11\x39\x9c\x5c\x04\xe2\xe5\xc2\xd8\xb8\x51\xe0\xe3\x51\x35\x51\xe7\x13"
      "\xe2\x0f\x27\x00\x03\x9b\x17\xbc\x2e\x64\x15\x79\x0f\xbb\xd4\xa5\x33\x5f"
      "\x91\xe7\x79\x09\xe8\xaa\x16\xaa\x79\xd7\x08\xeb\x91\xe7\xe5\x7d\x08\x0c"
      "\xce\x09\x41\x7d\x92\x0c\x82\x3f\xed\xcf\x09\x23\x85\xd0\x70\x23\xd1\xaf"
      "\xf4\xfc\x23\x13\x53\xd9\x29\x0c\xbd\x84\x50\xfb\xeb\x66\x1b\x09\x01\xe8"
      "\x0d\x4d\x90\x7c\xb6\xa8\xfc\xc9\xb6\xf6\x26\x50\x4d\x79\x87\xe1\x54\x31"
      "\xe6\x6f\xbb\x5e\xbd\xc7\x30\x50\xe9\xc8\xa9\xb6\xd2\x3d\xe1\xa1\x8e\x73"
      "\x28\xfb\xf2\x00\xf8\xcd\x59\x21\x96\xb5\x75\xb4\x71\x39\x82\x5e\x22\x49"
      "\xc3\x92\x83\x3b\x2d\x3a\x85\xdb\xa4\xf4\x3c\xc1\xe0\x56\x13\xa7\x04\x68"
      "\xee\x7e\xca\x83\xe4\xc6\x19\xc1\xa9\x00\x05\xf8\x9f\x0d\xcc\x64\x9b\xcb"
      "\xb5\x2a\x53\xd2\xcc\x16\xd8\x4b\x63\x50\x36\x7b\xbe\x0d\x5e\x84\xaa\xac"
      "\x8d\xc1\xa8\xab\x23\x3b\xe9\x4a\x27\x6b\xab\xe6\xb2\x06\x60\xc0\xa1\xc1"
      "\x86\x7b\x3d\xc7\x89\xc2\x8d\x73\xbe\x2a\x60\xf2\xf0\x02\xc3\x10\x87\x30"
      "\xe7\x2d\x8e\xc0\x88\x35\xf1\x39\xe6\xed\x38\x72\x43\x18\x02\x66\x34\xf7"
      "\x9d\xbb\xf5\x6d\xef\xfc\x8e\xfb\x3e\x74\x08\x5d\xdc\x18\x5d\x7b\x62\x08"
      "\xce\x8e\x12\x71\x61\x27\x98\x35\x9b\xe9\x84\x0d\x31\x3a\x20\x6f\xcf\x4a"
      "\x17\xbb\xc9\xe5\xd3\x3d\xc0\x52\xd9\x22\x8f\x07\x2d\xc9\x44\x27\x76\xef"
      "\xa8\x1e\xb1\x45\xb7\x3c\x30\x37\x4e\x58\x34\xb0\xc3\xb1\xd2\x15\x07\xb1"
      "\xd3\xb9\xe1\x41\x31\x9e\x68\xdd\xd2\xe5\x1a\x3a\x4a\x07\x7a\xe6\x8b\x6d"
      "\xc5\x68\xf2\x53\xac\x40\xaa\x0f\x21\xbc\x48\x9b\x53\x7b\xa0\xfc\x8f\x4d"
      "\x9b\x5c\x63\xea\xe8\x5e\xbc\xb0\xdb\xf1\x77\x80\x8b\x2e\x34\xeb\x25\x10"
      "\xdf\xe2\x46\x1d\xc2\x83\x46\xbf\xbc\x2f\x3b\xd0\xdb\xac\x4d\x8d\x4c\x85"
      "\xef\xdc\xa8\x00\x04\x51\xe8\x94\x67\x9d\x62\x8c\xe2\x94\x17\x04\xd0\xeb"
      "\x91\x7a\xa3\x69\x0d\x96\x90\x41\xf2\x66\x9a\x7d\x7c\xb5\x1b\xe5\x63\xb6"
      "\xe7\xff\x34\x1c\x0e\x13\x32\x88\x28\xc7\xd6\xd2\x08\xfa\x1a\xce\xff\x14"
      "\x95\xf4\x59\x89\xc2\xfe\x9b\x79\x79\xcc\x76\x67\x04\x17\x61\x7c\x73\x94"
      "\x8a\xae\x13\x84\x5d\xba\xe1\x95\x9e\xfb\x57\x61\xf6\x17\xef\xce\xd1\x4a"
      "\x05\xd9\xeb\x20\xad\xf5\xb6\x6b\xe6\xd4\x55\x1e\xc9\x7c\x11\xd1\xe7\x85"
      "\x3e\x53\xcb\x30\xba\xaa\xab\x6b\xea\x13\xb5\xde\x25\x72\x52\x57\xa4\x04"
      "\x4b\x76\xca\xb4\xf2\x65\x1b\x53\x58\x71\xe4\x7a\x11\x49\xf0\x30\xae\x67"
      "\xa4\x18\x9e\xf5\x73\x6b\xf9\x62\x5e\xac\xbf\x26\xe2\xfe\x0d\xdc\x12\x14"
      "\xaa\xe6\x1d\x31\x3f\xcb\x02\xc6\x58\x25\xbd\x6f\x7b\x59\x25\x88\x8b\x5e"
      "\x1d\xcf\xa6\x30\xbf\x4c\x6f\x1d\x15\xb7\x81\x2c\x8e\xae\x64\xe3\x85\xdd"
      "\x54\x67\xc4\x60\x6f\x62\x41\xcd\x24\xe0\xff\x18\x5b\x2b\x73\xe6\x63\x7f"
      "\xc8\x71\x2f\x9e\x20\x44\x34\x8f\x92\xe0\x61\x09\xd0\x2a\xab\x7a\x7b\xa4"
      "\x97\xa7\x0a\xc9\xc1\x78\x63\xf8\x1a\x90\x55\x0e\x50\x09\x67\x05\x37\x1d"
      "\x96\x3b\xba\x59\xb9\x4e\x08\x53\x6a\x2c\x3d\xc4\x3b\x0a\xda\x1d\x29\xdf"
      "\x99\xcf\xc5\xdd\xab\xb1\x23\x89\x21\x59\xb8\x38\x4c\x56\x89\x06\xa8\x6b"
      "\xdd\xf9\xf8\x4f\xff\xf3\x64\xf8\xfd\xab\x4a\xf3\x4f\xa8\xcb\x0c\x32\xd1"
      "\x8d\x60\x7f\x9d\xf5\x5a\x6c\x5c\x8d\xba\x69\x7d\xf9\xbc\x42\xc4\x58\xd7"
      "\x95\x30\x64\x7f\xbd\x02\x40\x95\x3b\x7d\x1b\x08\x3f\x9e\x62\xf7\x17\x5a"
      "\x4a\xcc\xb6\x20\x63\x88\x39\xce\xf3\x64\xf3\xc0\x44\x1a\xf3\x7a\x52\x1c"
      "\x09\x57\x26\xaa\xce\x28\xe1\x8e\x4c\xc9\x13\xeb\x3d\x44\x9d\x93\x00\xf6"
      "\x97\x00\x47\xe7\x53\xd0\x28\x59\xb7\x77\xd0\x86\xdf\x6a\x09\x79\x44\x1e"
      "\x6e\x7c\x4f\xae\xc7\x00\x2f\xb1\x73\x79\xd7\x02\x1d\x72\x3b\xba\xf5\x13"
      "\x6b\x98\xb1\xcc\x83\x49\xdb\x26\x72\x5a\x23\xff\x6e\x46\xa2\x1f\xc6\x96"
      "\x9f\x9f\x88\x69\x11\x7e\x24\x2f\xce\xc6\xd6\xe0\x55\xa4\x9d\x09\x1a\xae"
      "\xbd\x7d\x97\x04\xbd\xdf\xa2\x03\x65\x72\x83\x49\xaf\x1e\xe7\x83\x1b\x53"
      "\xdf\x14\xd8\x93\xdc\xcc\xbd\x4c\x7a\x9f\x48\x86\xe9\xce\xa7\x6d\x9d\xd5"
      "\x0e\xfb\xb5\x63\x5b\xb4\xac\x52\x7f\x55\xff\x62\x2b\x78\xc4\x99\x10\x7e"
      "\xb8\x3c\xf7\x71\xca\x99\xd0\xee\x42\xa8\x20\x90\x80\xfe\x2d\x02\x4e\x1f"
      "\x41\x67\x55\xe5\x94\xef\x09\x7c\xff\xdb\x69\x72\xdd\x10\x7e\x1d\x4b\x0b"
      "\xbb\x92\xbc\x6c\xb5\xdf\x17\xf6\xb8\x9d\x6f\xad\xc6\x7e\xa1\x3f\xaa\x35"
      "\x47\x53\x7a\xff\xb5\x40\xea\x53\x6f\x05\xf5\x8d\xf8\xe5\x4f\xb2\x92\xb4"
      "\x6a\xef\x82\x68\x51\xf6\x1d\x81\x88\xa1\x47\xb2\xca\x49\x5f\x3f\xda\x04"
      "\x47\x9f\x97\x52\x5f\xa0\x50\x99\xaf\x8e\x22\x88\x02\x0b\xe4\xe7\x9e\x67"
      "\x49\xff\xee\xb1\x6a\x18\xad\x46\x1d\x50\xd2\xc3\x4f\x84\x3a\xe7\x83\x00"
      "\x10\xe2\x5e\xfb\xbd\xc0\x28\x4a\xde\xfd\x73\xaf\x7f\xfa\xeb\x71\x61\xac"
      "\xa7\x32\x7e\xc0\xc8\x1d\xb9\x5c\xcb\x42\x27\xb7\x8c\x65\x7a\x38\xf4\xcb"
      "\x78\xe0\x98\xe8\x48\xc5\x15\xcb\x17\x91\x2d\xe6\x0d\x61\xb1\x31\xf6\x99"
      "\x74\xd6\xa8\x09\x93\x08\x2d\x79\x60\xca\xf5\x43\xa4\x29\xd9\x59\x67\x50"
      "\x15\x7c\x02\x85\x74\xde\x88\xda\xfa\x35\x00\xb5\x4c\x15\x33\x21\xd5\x57"
      "\xb4\xc5\x19\xa8\x91\x4c\xe2\xe3\x51\xf4\x5a\x09\x76\xce\xc0\x79\xd3\xd9"
      "\xe9\x36\xa6\x38\x85\xbd\x0e\xed\x16\xad\x7c\x29\x06\xed\x9e\x88\xc3\x1b"
      "\xd9\x96\x38\x6c\xa8\x74\x8a\x3d\xb2\xb9\x2e\x91\x69\xaa\x49\x48\xaa\x04"
      "\x64\x13\x45\x11\x61\xb4\xe1\x45\xf3\xf6\x76\xce\x06\x3e\x60\x27\x2f\xed"
      "\x4e\xbe\x3c\x56\x1f\xd6\x11\x22\x9d\x6e\x60\x85\xb8\xbe\xd9\xa1\x71\xee"
      "\x7f\x8c\xd8\xdf\x73\xbb\x84\x10\x0b\x8b\xdd\x1c\x1d\x38\x1f\xf5\x1c\xc1"
      "\x84\xdf\xc5\xd9\x33\xdc\x08\x86\x6b\x96\xce\xeb\xaa\x8c\x35\x02\xaf\x11"
      "\x2b\xf6\xac\x84\x80\xd7\x77\xa1\xff\x85\x86\xc0\xbf\xde\xed\x85\xa2\x19"
      "\x8d\xe3\x58\xb2\xcb\xd4\x17\x9b\xaa\x2a\xb6\xd8\x6e\xbc\x57\xc7\xe8\x90"
      "\x99\xc1\xf0\x6a\x59\xa6\xea\xb6\x6f\x37\x4d\xc5\xbb\xaf\xbd\x48\xd4\x3a"
      "\xdb\xe8\xf9\x0e\x8e\xa8\xee\x0d\x69\xf3\x57\x10\xcf\x00\x0f\xff\x98\xe1"
      "\xcc\xcd\x08\x34\xec\xe8\x3e\xc1\xda\xfc\x20\x24\x1a\xbd\xe7\xf0\xe5\x38"
      "\x3b\x35\x60\xb4\x5d\xb3\xf0\x88\x64\x78\xc3\x2f\xeb\x78\xb3\xc3\x82\x15"
      "\xb9\x2f\x67\x68\x3a\x9c\xc9\xd8\x23\x20\xf3\xb3\xe7\xff\x9b\xca\xd3\xa8"
      "\x82\x49\xde\x61\x46\x15\x71\xb1\x23\x25\x6e\xfa\x2b\x98\xb6\x93\xfc\x87"
      "\xf2\xfd\xb1\x6b\x3d\xe9\x2e\x70\xb7\xeb\x5c\x96\x13\xc9\x86\xc5\x7d\x87"
      "\x79\x03\x8b\x5c\xb0\x7d\x3b\xa3\x2f\x49\xa8\x0b\xc1\xe8\xc2\xb7\xb9\x22"
      "\xd2\x9e\x42\x4b\xf0\x5b\x6c\x16\xbb\x2c\x5d\x79\x50\xcd\x31\xf2\x14\x5a"
      "\xc6\xce\x33\x32\xd9\x1c\xf1\xb6\x59\x7e\x4f\x0d\xa4\x34\x4b\x9e\x42\x1e"
      "\x06\xd1\xf0\xb4\x30\xbf\x9f\xce\xf5\xf8\xa6\xb9\xc1\x40\x9a\x91\x51\xb5"
      "\x6e\x2f\x3a\xfa\x2b\x21\x66\x5c\x89\x2c\x7e\xa9\x60\xeb\x18\xd2\x9c\x7b"
      "\xa9\xc8\x13\x03\x34\xc5\x3c\x5e\x54\xcd\x1f\x7b\x15\x86\x27\xa1\xfe\xff"
      "\x9c\x80\x00\x6d\x9e\xcd\x15\x00\xd6\xa5\xa4\x4e\xa4\xc7\x28\x14\xc3\x82"
      "\x74\xac\x96\x8d\xbe\xc1\x24\xa9\xe5\x95\xb8\xf5\x36\x71\x6b\x2b\x28\x0d"
      "\xa7\xff\x87\x52\x5f\x60\x18\x1f\x24\x8b\xf6\x1a\x6c\x94\x19\x36\xaf\x73"
      "\x88\xbe\x8d\x4a\xee\xee\x9c\x87\x8d\x3e\xbf\xfb\xa7\xc1\xff\x51\x69\x61"
      "\xf6\xb5\x62\x55\x6f\x50\xe6\xb7\x31\x9e\x01\xb5\x00\x68\xc3\xb1\x0f\x47"
      "\x91\x35\xd1\xea\xc4\x83\x3d\x5f\xbe\xd5\x6f\x02\xdf\x0d\x2e\xd9\x29\x1d"
      "\xdc\x30\x5b\xd6\x80\x39\x2c\x8a\x21\x04\xc0\xa7\x21\x04\x96\x45\xfa\x59"
      "\x8b\xfb\xf3\x60\x46\x52\x10\xc6\x10\x5c\x23\x84\xd1\x15\x2d\xaa\x58\x7e"
      "\xb1\xb5\xe3\xbd\x40\xd8\x22\xc9\x4f\xa9\xa8\xac\xb7\xd5\x76\xab\xb1\xcf"
      "\x8a\x93\x53\x91\x94\xb9\x43\xbe\xdf\xec\xd4\x25\xe3\x1d\xe1\x69\xf1\x55"
      "\xc0\xe6\x9c\x33\x68\x37\x8f\x95\x84\x45\x70\x22\x7d\x72\xf5\x52\x53\xa6"
      "\xd0\x9c\x35\xc1\x04\x34\xe8\x25\x31\x25\xd8\x2e\xd6\x03\xd3\xbe\xdc\xdc"
      "\xea\x24\x01\x58\xc6\x4b\xbe\xa6\x8e\x60\x47\xc8\xd2\x8f\xef\x68\x78\x1f"
      "\x4e\x5f\xa3\x84\xdd\xb6\x24\x31\x0e\x05\xe5\x4d\x47\x57\x40\x98\x57\xc0"
      "\x47\x27\xb8\x5c\x82\x42\x95\x9d\x6f\x86\xa7\xff\xd2\x84\x8f\x5a\x98\x88"
      "\x55\xdf\x45\x90\x76\x77\x3b\xa5\x0b\x75\x27\x05\x92\xf2\x0d\xe0\x1d\x47"
      "\xea\xd4\xa5\x12\x1a\xf7\x13\x77\x7f\xfd\x4a\x57\x84\x38\x82\x0e\x50\xde"
      "\xe7\xac\x1d\xb2\x8a\x64\x6e\x7c\x99\x38\x15\x40\x4c\x7a\xf9\x32\x6e\x4d"
      "\x12\xb8\xf9\xf4\x55\x5b\x35\x98\x7b\xd1\x8a\x3d\x50\xea\xf1\x54\x46\x7f"
      "\x28\xe1\x03\xf6\x3b\xe8\x15\x2e\x2e\xd4\x3d\xd4\xb7\x8f\x98\x58\x1a\x8f"
      "\x86\x26\xe2\x3b\x60\x87\xc2\x32\x3f\x95\x27\xa7\x66\x09\xc4\xd0\x78\x88"
      "\x80\x13\xd4\x4c\x67\xa8\x9c\x3c\xe9\xe9\xd2\xf5\x3e\x9b\x90\x1a\x13\x24"
      "\xed\xe9\x66\xf4\xee\x4a\x71\x15\x53\x20\xc5\xa8\x3d\xef\x52\xd9\x49\x1a"
      "\x92\x96\x40\x8c\xb2\x04\xe1\x5a\x95\x0a\xba\x34\xe1\xe3\xe2\x18\x89\xd4"
      "\x5e\xa2\x57\xef\x0c\x17\xc0\x7f\x31\xf6\x81\x62\x01\x8e\x55\x4a\x85\x1c"
      "\x06\xbb\x2e\xe3\x07\x8f\xe6\x4f\x36\x2a\x60\x57\xee\xf2\x6b\x5c\xd3\x3c"
      "\x68\xc4\x57\xf3\xc1\xa4\xbc\x2f\xcc\x9e\xb9\x2d\xac\xcb\x2c\x87\x6f\x11"
      "\xa8\x0b\x4d\xa2\x10\x5e\xef\xd9\x14\x44\x6c\x0c\xe7\xc1\xcb\x5d\xec\xa0"
      "\xe5\xb3\xc3\x7d\xa9\x4b\x8f\x9e\x9e\xe6\xe9\xc8\x27\xb7\xeb\x89\xa3\x30"
      "\xf4\xc3\xd7\x02\x53\x15\xd5\xd2\x0f\xc6\x8f\x84\xe0\x6e\xce\x06\xb6\x68"
      "\x76\x89\x3f\xaf\x5e\x95\x9d\x1b\xab\x87\x43\x4e\x94\x2e\xd7\x82\xa7\x3d"
      "\xa0\xbd\x15\xdf\xcc\x4b\xcb\xdc\x37\xad\x09\xd1\xb2\xb8\x86\xbe\x32\xae"
      "\x30\xb7\x3a\x71\xd8\x9c\x5c\xeb\x4f\x87\x4f\x35\xb3\xbe\x09\x60\x7e\x2d"
      "\xf7\xaa\xac\x67\x86\x18\x70\x6a\xe2\x5c\x39\x79\xd1\x26\xbd\x50\x58\x90"
      "\x22\x5b\x80\xff\x86\xa7\x44\xe0\xaf\x3d\x42\xf1\x56\xb2\x81\x4d\x1c\x57"
      "\x1a\x3e\x38\x6b\xbc\x9b\x31\x96\xf8\x2d\x5d\x67\xa2\xce\xa6\xed\x5b\x24"
      "\x31\x90\xa7\xa4\x9e\xca\x66\xd3\x3c\xb5\x73\x98\x5c\x5b\xb4\xcc\xe5\x7e"
      "\x08\xaf\xab\x7b\x27\x26\xf0\x96\x0c\x70\x66\xbd\x82\xbc\x5e\x9f\x44\xc1"
      "\x2a\x30\xd2\xc4\x34\x41\x41\xb2\x6c\xa5\x0c\xa5\x52\x4b\x69\xb2\xce\xa6"
      "\x9b\xc6\x33\x1d\x78\xc7\xf1\xeb\x24\xef\x59\x5b\x14\x5e\x92\x5d\x65\xd6"
      "\x84\xb1\xa7\x27\xc0\xee\x8b\x28\x94\x7c\x94\x31\x84\xec\x53\xe1\x53\xb1"
      "\xae\xe4\x02\x79\x20\x7a\x78\x93\xba\xea\xda\x25\x1d\x6b\xe3\x90\x88\x6c"
      "\x60\x92\xdb\xf7\xb4\xf6\x4f\xc7\x34\x65\x84\xc6\x14\xf7\x9f\x3b\x52\x60"
      "\x17\x2f\x5e\xc5\xa0\x98\xa3\x8c\x68\x71\x08\x90\x3c\xa9\xae\xc3\xea\x53"
      "\xc1\x32\xc6\x79\x09\x48\x25\xd1\x87\xcb\x25\x63\x6c\x60\x20\x16\x30\xd2"
      "\x3a\x9f\x8e\xa8\xae\x17\xde\x34\x98\x71\xd9\x88\x63\x2e\x72\x96\x86\x9d"
      "\xcd\x65\xe4\x2f\x1d\x5c\xa7\x9f\xdb\xbb\x44\xb0\xcd\x57\x31\x55\x3d\x63"
      "\x75\x57\xe0\x3e\xbc\x20\xbe\x20\xf3\x9c\x84\xa7\x48\x96\x5c\x6b\x1b\x8b"
      "\xa3\x13\xcb\x67\x30\x53\x67\x58\x9c\xff\xc4\x7a\xe5\x8f\x59\x17\xb7\x34"
      "\xc7\x94\x36\x2d\xd1\xa8\x04\x18\x9a\x52\xc1\xcc\x85\x2d\xc5\xfc\xc4\x9b"
      "\x45\x30\xcf\x9a\x3c\x83\xc2\x41\xd3\xb3\xa4\x49\x0b\x45\x14\xc0\x81\x31"
      "\x51\xc9\xa1\xcb\xc8\xbd\xe5\xeb\x45\x60\x06\xab\x02\x85\x50\x56\x50\x09"
      "\x27\x1b\x16\x84\x80\x36\xb8\x77\x11\xe2\xeb\x53\xf0\x8f\x9e\x85\xb2\x99"
      "\x87\x5a\xf0\x58\x04\x0f\x99\xec\xd0\xd9\x0b\x87\x34\x92\xcc\x35\x87\xc9"
      "\x39\x90\xd9\xdb\x5b\x5a\xc4\xde\xd0\xb6\xbc\x9c\x77\xda\x6e\xe9\xab\xf9"
      "\x06\x01\x13\x6d\x9b\x79\x1c\xff\x89\x1b\x42\xa7\xa1\x82\x1a\xeb\xe0\x42"
      "\x2c\xc4\xc7\x02\x04\xc8\x21\x5e\x6a\xca\x6d\x03\x04\x6d\xbc\x5e\xf7\x95"
      "\x94\x6a\xc7\xf6\x9c\x95\x2b\x95\xd9\x31",
      4096));
  NONFAILING(*(uint16_t*)0x200026c4 = 0x1000);
  syscall(__NR_write, (intptr_t)r[0], 0x200016c0, 0x1006);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  install_segv_handler();
  loop();
  return 0;
}
