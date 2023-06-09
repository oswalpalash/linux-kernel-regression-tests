// https://syzkaller.appspot.com/bug?id=ddb673727990990c6ded3e7cc220e39abfc244ab
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

static uintptr_t syz_open_dev(uintptr_t a0, uintptr_t a1, uintptr_t a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block",
            (uint8_t)a1, (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf));
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

static void test();

void loop()
{
  while (1) {
    test();
  }
}

long r[24];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    memcpy((void*)0x202d2000,
           "\x2f\x64\x65\x76\x2f\x6e\x65\x74\x2f\x74\x75\x6e\x00", 13);
    r[2] = syz_open_dev(0x202d2000ul, 0x0ul, 0x3ul);
    break;
  case 2:
    *(uint32_t*)0x202d1000 = (uint32_t)0xb;
    r[4] = syscall(__NR_ioctl, r[2], 0x400454daul, 0x202d1000ul);
    break;
  case 3:
    memcpy((void*)0x20533000, "\x00\x00\x00\x00\x00\x00\x00\xff\x00\x00"
                              "\x00\x02\x08\x00\x00\x00",
           16);
    *(uint32_t*)0x20533010 = (uint32_t)0x10001;
    *(uint32_t*)0x20533014 = (uint32_t)0x0;
    *(uint64_t*)0x20533018 = (uint64_t)0x20012fe8;
    *(uint32_t*)0x20012fe8 = (uint32_t)0x0;
    *(uint32_t*)0x20012fec = (uint32_t)0x0;
    *(uint32_t*)0x20012ff0 = (uint32_t)0x0;
    *(uint32_t*)0x20012ff4 = (uint32_t)0x0;
    *(uint32_t*)0x20012ff8 = (uint32_t)0x0;
    *(uint16_t*)0x20012ffc = (uint16_t)0x0;
    *(uint16_t*)0x20012ffe = (uint16_t)0x0;
    r[16] = syscall(__NR_ioctl, r[2], 0x400454caul, 0x20533000ul);
    break;
  case 4:
    r[17] = syscall(__NR_setsockopt, 0xfffffffffffffffful, 0x107ul,
                    0x1ul, 0x20001000ul, 0x200ul);
    break;
  case 5:
    r[18] = syscall(__NR_close, r[2]);
    break;
  case 6:
    r[19] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                    0xfffffffffffffffful, 0x0ul);
    break;
  case 7:
    memcpy((void*)0x202d2000,
           "\x2f\x64\x65\x76\x2f\x6e\x65\x74\x2f\x74\x75\x6e\x00", 13);
    r[21] = syz_open_dev(0x202d2000ul, 0x0ul, 0x0ul);
    break;
  case 8:
    *(uint32_t*)0x209a4000 = (uint32_t)0x2;
    r[23] = syscall(__NR_ioctl, r[21], 0x400454cbul, 0x209a4000ul);
    break;
  }
  return 0;
}

void test()
{
  long i;
  pthread_t th[18];

  memset(r, -1, sizeof(r));
  srand(getpid());
  for (i = 0; i < 9; i++) {
    pthread_create(&th[i], 0, thr, (void*)i);
    usleep(rand() % 10000);
  }
  for (i = 0; i < 9; i++) {
    pthread_create(&th[9 + i], 0, thr, (void*)i);
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
