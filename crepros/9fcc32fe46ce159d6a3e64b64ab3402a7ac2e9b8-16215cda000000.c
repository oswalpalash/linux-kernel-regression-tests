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

long r[10];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0xef4000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    r[1] = syscall(__NR_socket, 0x26ul, 0x5ul, 0x0ul);
    break;
  case 2:
    *(uint16_t*)0x20e04000 = (uint16_t)0x26;
    memcpy((void*)0x20e04002,
           "\x72\x6e\x67\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
           14);
    *(uint32_t*)0x20e04010 = (uint32_t)0x0;
    *(uint32_t*)0x20e04014 = (uint32_t)0x0;
    memcpy((void*)0x20e04018,
           "\x73\x74\x64\x72\x6e\x67\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00",
           64);
    r[7] = syscall(__NR_bind, r[1], 0x20e04000ul, 0x58ul);
    break;
  case 3:
    memcpy((void*)0x2053f000,
           "\x78\x5e\xb9\xf8\xba\xa9\x38\xe3\x32\x43\x15\xc5\xf5\x9e"
           "\x32\x40\xbd\x88\x28\x4c\xc5\xb0\xa5\xd1\xf0\xb1\xbc\x04"
           "\x04\x76\xd0\xe3\xbc\x04\xd8\x05\x3f\x76\x4b\x20\xcc\x57"
           "\x7b\x08\xf7\x29\xee\x66\xd5\x47\x14\xbe\x32\x4e\x82\xb2"
           "\xd3\x0d\xf3\xe3\x33\xe9\x66\x53\x40\xe0\xf9\xd6\xed\xdc"
           "\x53\x79\xb9\xe6\x70\x67\x39\xf7\x48\x0e\xdf\xa6\xdf\xda"
           "\xa3\xc6\x6b\x11\x25\x43\xb6\xad\x37\xcb\xdf\xbf\x3e\x26"
           "\x22\xce\x09\xd1\xab\x56\xa8\xab\xeb\x96\x33\x78\x8f\xc3"
           "\x47\x1b\x3a\x64\xe2\x65\xca\x50\xf4\xfe\x32\x4a\xc4\x80"
           "\x8c\xe0\xf0\xd1\xea\xdd\x08\xb3\x16\x74\x81\x96\xe4\xc9"
           "\xa1",
           141);
    r[9] = syscall(__NR_setsockopt, r[1], 0x117ul, 0x1ul, 0x2053f000ul,
                   0x8dul);
    break;
  }
  return 0;
}

void test()
{
  long i;
  pthread_t th[8];

  memset(r, -1, sizeof(r));
  srand(getpid());
  for (i = 0; i < 4; i++) {
    pthread_create(&th[i], 0, thr, (void*)i);
    usleep(rand() % 10000);
  }
  for (i = 0; i < 4; i++) {
    pthread_create(&th[4 + i], 0, thr, (void*)i);
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
