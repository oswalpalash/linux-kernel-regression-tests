// https://syzkaller.appspot.com/bug?id=9fcc32fe46ce159d6a3e64b64ab3402a7ac2e9b8
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

static void test();

void loop()
{
  while (1) {
    test();
  }
}

long r[77];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    r[1] = syscall(__NR_socket, 0x26ul, 0x5ul, 0x0ul);
    break;
  case 2:
    *(uint16_t*)0x202bf000 = (uint16_t)0x26;
    memcpy((void*)0x202bf002,
           "\x68\x61\x73\x68\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
           14);
    *(uint32_t*)0x202bf010 = (uint32_t)0x21f;
    *(uint32_t*)0x202bf014 = (uint32_t)0x8000;
    memcpy((void*)0x202bf018,
           "\x68\x6d\x61\x63\x28\x73\x68\x61\x32\x35\x36\x29\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00",
           64);
    r[7] = syscall(__NR_bind, r[1], 0x202bf000ul, 0x58ul);
    break;
  case 3:
    *(uint32_t*)0x20a36ffc = (uint32_t)0x4;
    r[9] = syscall(__NR_getsockopt, 0xfffffffffffffffful, 0x0ul, 0x22ul,
                   0x201a1000ul, 0x20a36ffcul);
    break;
  case 4:
    *(uint32_t*)0x2035d000 = (uint32_t)0x0;
    *(uint16_t*)0x2035d008 = (uint16_t)0xa;
    *(uint16_t*)0x2035d00a = (uint16_t)0x234e;
    *(uint32_t*)0x2035d00c = (uint32_t)0x2;
    *(uint8_t*)0x2035d010 = (uint8_t)0x0;
    *(uint8_t*)0x2035d011 = (uint8_t)0x0;
    *(uint8_t*)0x2035d012 = (uint8_t)0x0;
    *(uint8_t*)0x2035d013 = (uint8_t)0x0;
    *(uint8_t*)0x2035d014 = (uint8_t)0x0;
    *(uint8_t*)0x2035d015 = (uint8_t)0x0;
    *(uint8_t*)0x2035d016 = (uint8_t)0x0;
    *(uint8_t*)0x2035d017 = (uint8_t)0x0;
    *(uint8_t*)0x2035d018 = (uint8_t)0x0;
    *(uint8_t*)0x2035d019 = (uint8_t)0x0;
    *(uint8_t*)0x2035d01a = (uint8_t)0x0;
    *(uint8_t*)0x2035d01b = (uint8_t)0x0;
    *(uint8_t*)0x2035d01c = (uint8_t)0x0;
    *(uint8_t*)0x2035d01d = (uint8_t)0x0;
    *(uint8_t*)0x2035d01e = (uint8_t)0x0;
    *(uint8_t*)0x2035d01f = (uint8_t)0x0;
    *(uint32_t*)0x2035d020 = (uint32_t)0xffffffff00000000;
    *(uint64_t*)0x2035d028 = (uint64_t)0x0;
    *(uint64_t*)0x2035d030 = (uint64_t)0x0;
    *(uint64_t*)0x2035d038 = (uint64_t)0x0;
    *(uint64_t*)0x2035d040 = (uint64_t)0x0;
    *(uint64_t*)0x2035d048 = (uint64_t)0x0;
    *(uint64_t*)0x2035d050 = (uint64_t)0x0;
    *(uint64_t*)0x2035d058 = (uint64_t)0x0;
    *(uint64_t*)0x2035d060 = (uint64_t)0x0;
    *(uint64_t*)0x2035d068 = (uint64_t)0x0;
    *(uint64_t*)0x2035d070 = (uint64_t)0x0;
    *(uint64_t*)0x2035d078 = (uint64_t)0x0;
    *(uint64_t*)0x2035d080 = (uint64_t)0x0;
    *(uint64_t*)0x2035d090 = (uint64_t)0x2;
    *(uint64_t*)0x2035d098 = (uint64_t)0xfbd;
    *(uint64_t*)0x2035d0a0 = (uint64_t)0x28e800000000000;
    *(uint64_t*)0x2035d0a8 = (uint64_t)0xc88e1b1;
    *(uint64_t*)0x2035d0b0 = (uint64_t)0x2;
    *(uint64_t*)0x2035d0b8 = (uint64_t)0x6;
    *(uint64_t*)0x2035d0c0 = (uint64_t)0x1;
    *(uint64_t*)0x2035d0c8 = (uint64_t)0x6;
    *(uint64_t*)0x2035d0d0 = (uint64_t)0x8;
    *(uint64_t*)0x2035d0d8 = (uint64_t)0x5;
    *(uint64_t*)0x2035d0e0 = (uint64_t)0x2;
    *(uint64_t*)0x2035d0e8 = (uint64_t)0x7;
    *(uint64_t*)0x2035d0f0 = (uint64_t)0x40;
    *(uint64_t*)0x2035d0f8 = (uint64_t)0x80000001;
    *(uint64_t*)0x2035d100 = (uint64_t)0x8000;
    *(uint32_t*)0x20b9effc = (uint32_t)0x108;
    r[59] = syscall(__NR_getsockopt, 0xfffffffffffffffful, 0x84ul,
                    0x70ul, 0x2035d000ul, 0x20b9effcul);
    break;
  case 5:
    memcpy((void*)0x2063bfe0, "\x69\x72\x6c\x61\x6e\x30\x00\x00\x00\x00"
                              "\x00\x00\x00\x00\x00\x00",
           16);
    *(uint16_t*)0x2063bff0 = (uint16_t)0x2;
    *(uint16_t*)0x2063bff2 = (uint16_t)0x224e;
    *(uint8_t*)0x2063bff4 = (uint8_t)0xac;
    *(uint8_t*)0x2063bff5 = (uint8_t)0x14;
    *(uint8_t*)0x2063bff6 = (uint8_t)0x0;
    *(uint8_t*)0x2063bff7 = (uint8_t)0xaa;
    *(uint8_t*)0x2063bff8 = (uint8_t)0x0;
    *(uint8_t*)0x2063bff9 = (uint8_t)0x0;
    *(uint8_t*)0x2063bffa = (uint8_t)0x0;
    *(uint8_t*)0x2063bffb = (uint8_t)0x0;
    *(uint8_t*)0x2063bffc = (uint8_t)0x0;
    *(uint8_t*)0x2063bffd = (uint8_t)0x0;
    *(uint8_t*)0x2063bffe = (uint8_t)0x0;
    *(uint8_t*)0x2063bfff = (uint8_t)0x0;
    r[75] = syscall(__NR_ioctl, r[1], 0x8918ul, 0x2063bfe0ul);
    break;
  case 6:
    r[76] = syscall(__NR_setsockopt, r[1], 0x117ul, 0x1ul, 0x20490000ul,
                    0x0ul);
    break;
  }
  return 0;
}

void test()
{
  long i;
  pthread_t th[14];

  memset(r, -1, sizeof(r));
  srand(getpid());
  for (i = 0; i < 7; i++) {
    pthread_create(&th[i], 0, thr, (void*)i);
    usleep(rand() % 10000);
  }
  for (i = 0; i < 7; i++) {
    pthread_create(&th[7 + i], 0, thr, (void*)i);
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
