// https://syzkaller.appspot.com/bug?id=1665547180092fef185a46d5a4ba5b382d77848e
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

long r[27];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0xaac000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    r[1] = syscall(__NR_socket, 0xaul, 0x1ul, 0x84ul);
    break;
  case 2:
    memcpy((void*)0x20aa7000, "\x22\xc9\xb6\xac\xc9\x4d", 6);
    *(uint16_t*)0x20aa7000 = (uint16_t)0xa;
    *(uint16_t*)0x20aa7002 = (uint16_t)0x234e;
    *(uint32_t*)0x20aa7004 = (uint32_t)0x4;
    *(uint8_t*)0x20aa7008 = (uint8_t)0xfe;
    *(uint8_t*)0x20aa7009 = (uint8_t)0x80;
    *(uint8_t*)0x20aa700a = (uint8_t)0x0;
    *(uint8_t*)0x20aa700b = (uint8_t)0x0;
    *(uint8_t*)0x20aa700c = (uint8_t)0x0;
    *(uint8_t*)0x20aa700d = (uint8_t)0x0;
    *(uint8_t*)0x20aa700e = (uint8_t)0x0;
    *(uint8_t*)0x20aa700f = (uint8_t)0x0;
    *(uint8_t*)0x20aa7010 = (uint8_t)0x0;
    *(uint8_t*)0x20aa7011 = (uint8_t)0x0;
    *(uint8_t*)0x20aa7012 = (uint8_t)0x0;
    *(uint8_t*)0x20aa7013 = (uint8_t)0x0;
    *(uint8_t*)0x20aa7014 = (uint8_t)0x0;
    *(uint8_t*)0x20aa7015 = (uint8_t)0x0;
    *(uint8_t*)0x20aa7016 = (uint8_t)0x0;
    *(uint8_t*)0x20aa7017 = (uint8_t)0xaa;
    *(uint32_t*)0x20aa7018 = (uint32_t)0x2;
    r[23] = syscall(__NR_sendto, r[1], 0x20aa7000ul, 0x6ul, 0x8000ul,
                    0x20aa7000ul, 0x1cul);
    break;
  case 3:
    r[24] = syscall(__NR_listen, r[1], 0xfffffffffffffff9ul);
    break;
  case 4:
    *(uint32_t*)0x20281000 = (uint32_t)0x10;
    r[26] =
        syscall(__NR_accept4, r[1], 0x20aa5ff0ul, 0x20281000ul, 0x0ul);
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
