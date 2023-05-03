// https://syzkaller.appspot.com/bug?id=e4cd90db60c4517094c0ffcb9468de1bf86809e7
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <stdint.h>
#include <string.h>

static uintptr_t syz_open_procfs(uintptr_t a0, uintptr_t a1)
{

  char buf[128];
  memset(buf, 0, sizeof(buf));
  if (a0 == 0) {
    snprintf(buf, sizeof(buf), "/proc/self/%s", (char*)a1);
  } else if (a0 == (uintptr_t)-1) {
    snprintf(buf, sizeof(buf), "/proc/thread-self/%s", (char*)a1);
  } else {
    snprintf(buf, sizeof(buf), "/proc/self/task/%d/%s", (int)a0,
             (char*)a1);
  }
  int fd = open(buf, O_RDWR);
  if (fd == -1)
    fd = open(buf, O_RDONLY);
  return fd;
}

#ifndef __NR_timer_create
#define __NR_timer_create 259
#endif
#ifndef __NR_preadv
#define __NR_preadv 333
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[20];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  memcpy((void*)0x20337ff2, "\x74\x69\x6d\x65\x72\x73\x00", 7);
  r[2] = syz_open_procfs(0x0ul, 0x20337ff2ul);
  *(uint32_t*)0x20f6afa0 = (uint32_t)0x0;
  *(uint32_t*)0x20f6afa4 = (uint32_t)0x1d;
  *(uint32_t*)0x20f6afa8 = (uint32_t)0x60000001;
  *(uint32_t*)0x20f6afac = (uint32_t)0x20af2000;
  *(uint32_t*)0x20f6afb0 = (uint32_t)0x2093465d;
  *(uint64_t*)0x20f6afb8 = (uint64_t)0x0;
  *(uint64_t*)0x20f6afc0 = (uint64_t)0x0;
  *(uint64_t*)0x20f6afc8 = (uint64_t)0x0;
  *(uint64_t*)0x20f6afd0 = (uint64_t)0x0;
  *(uint64_t*)0x20f6afd8 = (uint64_t)0x0;
  *(uint64_t*)0x20f6afe0 = (uint64_t)0x0;
  *(uint64_t*)0x20f6afe8 = (uint64_t)0x0;
  *(uint64_t*)0x20f6aff0 = (uint64_t)0x0;
  r[16] = syscall(__NR_timer_create, 0x7ul, 0x20f6afa0ul, 0x20ebe000ul);
  *(uint32_t*)0x20b15000 = (uint32_t)0x206e4000;
  *(uint32_t*)0x20b15004 = (uint32_t)0xeb;
  r[19] = syscall(__NR_preadv, r[2], 0x20b15000ul, 0x1ul, 0x0ul);
}

int main()
{
  loop();
  return 0;
}
