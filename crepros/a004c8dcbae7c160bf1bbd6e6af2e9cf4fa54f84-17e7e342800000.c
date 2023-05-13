// https://syzkaller.appspot.com/bug?id=1665547180092fef185a46d5a4ba5b382d77848e
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <pthread.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <stdint.h>
#include <string.h>

long r[27];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0xd27000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    r[1] = syscall(__NR_socket, 0xaul, 0x1ul, 0x8010000000000084ul);
    break;
  case 2:
    memcpy((void*)0x203f1000, "\x8f", 1);
    *(uint16_t*)0x202cf000 = (uint16_t)0xa;
    *(uint16_t*)0x202cf002 = (uint16_t)0x204e;
    *(uint32_t*)0x202cf004 = (uint32_t)0x0;
    *(uint8_t*)0x202cf008 = (uint8_t)0xfe;
    *(uint8_t*)0x202cf009 = (uint8_t)0x80;
    *(uint8_t*)0x202cf00a = (uint8_t)0x0;
    *(uint8_t*)0x202cf00b = (uint8_t)0x0;
    *(uint8_t*)0x202cf00c = (uint8_t)0x0;
    *(uint8_t*)0x202cf00d = (uint8_t)0x0;
    *(uint8_t*)0x202cf00e = (uint8_t)0x0;
    *(uint8_t*)0x202cf00f = (uint8_t)0x0;
    *(uint8_t*)0x202cf010 = (uint8_t)0x0;
    *(uint8_t*)0x202cf011 = (uint8_t)0x0;
    *(uint8_t*)0x202cf012 = (uint8_t)0x0;
    *(uint8_t*)0x202cf013 = (uint8_t)0x0;
    *(uint8_t*)0x202cf014 = (uint8_t)0x0;
    *(uint8_t*)0x202cf015 = (uint8_t)0x0;
    *(uint8_t*)0x202cf016 = (uint8_t)0x0;
    *(uint8_t*)0x202cf017 = (uint8_t)0xbb;
    *(uint32_t*)0x202cf018 = (uint32_t)0x2;
    r[23] = syscall(__NR_sendto, r[1], 0x203f1000ul, 0x1ul, 0x0ul,
                    0x202cf000ul, 0x1cul);
    break;
  case 3:
    r[24] = syscall(__NR_listen, r[1], 0x59eul);
    break;
  case 4:
    *(uint32_t*)0x20d24ffc = (uint32_t)0x10;
    r[26] = syscall(__NR_accept, r[1], 0x20b28ff0ul, 0x20d24ffcul);
    break;
  }
  return 0;
}

void loop()
{
  long i;
  pthread_t th[10];

  memset(r, -1, sizeof(r));
  for (i = 0; i < 5; i++) {
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
