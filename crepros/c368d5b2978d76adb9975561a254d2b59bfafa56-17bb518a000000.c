// https://syzkaller.appspot.com/bug?id=c368d5b2978d76adb9975561a254d2b59bfafa56
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

long r[49];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    r[1] = syscall(__NR_socket, 0xful, 0x8000000003ul, 0x2ul);
    break;
  case 2:
    memcpy((void*)0x20e1a000, "\x02\x0b\xaf\x01\x02\x00\x00\x00\x00\x06"
                              "\x7b\xbc\x8e\x1d\x4b\x48",
           16);
    r[3] = syscall(__NR_write, r[1], 0x20e1a000ul, 0x10ul);
    break;
  case 3:
    r[4] = syscall(__NR_socket, 0xaul, 0x2ul, 0x0ul);
    break;
  case 4:
    *(uint64_t*)0x20000000 = (uint64_t)0x20005000;
    *(uint64_t*)0x20000008 = (uint64_t)0x20002fdc;
    *(uint64_t*)0x20000010 = (uint64_t)0x20001ff8;
    *(uint64_t*)0x20000018 = (uint64_t)0x20004ff0;
    *(uint32_t*)0x20000020 = (uint32_t)0x1;
    *(uint32_t*)0x20000024 = (uint32_t)0x0;
    *(uint32_t*)0x20000028 = (uint32_t)0x2;
    *(uint32_t*)0x2000002c = (uint32_t)0x0;
    *(uint32_t*)0x20000030 = (uint32_t)0x0;
    *(uint32_t*)0x20000034 = (uint32_t)0x0;
    *(uint32_t*)0x20000038 = (uint32_t)0x0;
    *(uint32_t*)0x2000003c = (uint32_t)0x0;
    r[17] = syscall(__NR_ioctl, 0xfffffffffffffffful, 0xc04064a0ul,
                    0x20000000ul);
    break;
  case 5:
    r[18] = syscall(__NR_setsockopt, r[4], 0x29ul, 0x23ul, 0x20000000ul,
                    0x264ul);
    break;
  case 6:
    r[19] = syscall(__NR_socket, 0xful, 0x4000000000000003ul, 0x2ul);
    break;
  case 7:
    *(uint64_t*)0x2023dfc8 = (uint64_t)0x20956ff8;
    *(uint32_t*)0x2023dfd0 = (uint32_t)0x8;
    *(uint64_t*)0x2023dfd8 = (uint64_t)0x2097e000;
    *(uint64_t*)0x2023dfe0 = (uint64_t)0x1;
    *(uint64_t*)0x2023dfe8 = (uint64_t)0x206d7000;
    *(uint64_t*)0x2023dff0 = (uint64_t)0x5;
    *(uint32_t*)0x2023dff8 = (uint32_t)0x0;
    *(uint16_t*)0x20956ff8 = (uint16_t)0x0;
    *(uint8_t*)0x20956ffa = (uint8_t)0x0;
    *(uint32_t*)0x20956ffc = (uint32_t)0x4e20;
    *(uint64_t*)0x2097e000 = (uint64_t)0x20254000;
    *(uint64_t*)0x2097e008 = (uint64_t)0x10;
    memcpy((void*)0x20254000, "\x02\x12\x05\x00\x02\x00\x00\x00\x00\x00"
                              "\xf2\x67\x65\x46\xb7\xf5",
           16);
    *(uint64_t*)0x206d7000 = (uint64_t)0x10;
    *(uint32_t*)0x206d7008 = (uint32_t)0x139;
    *(uint32_t*)0x206d700c = (uint32_t)0x2b2b;
    *(uint64_t*)0x206d7010 = (uint64_t)0x10;
    *(uint32_t*)0x206d7018 = (uint32_t)0x111;
    *(uint32_t*)0x206d701c = (uint32_t)0x8;
    *(uint64_t*)0x206d7020 = (uint64_t)0x10;
    *(uint32_t*)0x206d7028 = (uint32_t)0x11b;
    *(uint32_t*)0x206d702c = (uint32_t)0x6;
    *(uint64_t*)0x206d7030 = (uint64_t)0x10;
    *(uint32_t*)0x206d7038 = (uint32_t)0x13f;
    *(uint32_t*)0x206d703c = (uint32_t)0x5;
    *(uint64_t*)0x206d7040 = (uint64_t)0x10;
    *(uint32_t*)0x206d7048 = (uint32_t)0x109;
    *(uint32_t*)0x206d704c = (uint32_t)0x401;
    r[48] = syscall(__NR_sendmsg, r[19], 0x2023dfc8ul, 0x0ul);
    break;
  }
  return 0;
}

void test()
{
  long i;
  pthread_t th[16];

  memset(r, -1, sizeof(r));
  srand(getpid());
  for (i = 0; i < 8; i++) {
    pthread_create(&th[i], 0, thr, (void*)i);
    usleep(rand() % 10000);
  }
  for (i = 0; i < 8; i++) {
    pthread_create(&th[8 + i], 0, thr, (void*)i);
    if (rand() % 2)
      usleep(rand() % 10000);
  }
  usleep(rand() % 100000);
}

int main()
{
  loop();
  return 0;
}
