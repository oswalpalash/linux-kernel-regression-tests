// https://syzkaller.appspot.com/bug?id=ff3b27c5a26a52454b64fec53964965077f9696c
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

long r[4];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                   r[0], 0x0ul);
    break;
  case 1:
    r[1] = syscall(__NR_socket, 0x10ul, 0x802ul, 0x0ul);
    break;
  case 2:
    memcpy((void*)0x20020000, "\x24\x00\x00\x00\x1a\x00\x25\xf0\x00\x18"
                              "\x00\x04\x00\xed\xfc\x0e\x02\x00\x00\x00"
                              "\x00\x10\xbf\x02\x02\xff\xff\xeb\xcf\x37"
                              "\x00\x00\x47\x18\x6b\xe0\x29",
           37);
    r[3] = syscall(__NR_write, r[1], 0x20020000ul, 0x25ul);
    break;
  }
  return 0;
}

void loop()
{
  long i;
  pthread_t th[6];

  memset(r, -1, sizeof(r));
  for (i = 0; i < 3; i++) {
    pthread_create(&th[i], 0, thr, (void*)i);
    usleep(rand() % 10000);
  }
  usleep(rand() % 100000);
}

int main()
{
  loop();
  return 0;
}
