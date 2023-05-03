// https://syzkaller.appspot.com/bug?id=1665547180092fef185a46d5a4ba5b382d77848e
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

long r[9];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    r[1] = syscall(__NR_socket, 0xaul, 0x1ul, 0x84ul);
    break;
  case 2:
    memcpy((void*)0x20446000,
           "\x0c\xd3\xff\x16\x6e\x29\xe1\xe5\xba\x67\xee\x45\x08\xa8"
           "\x52\x12\x60\xcc\x92\xde\x7f\xa9\xc4\x83\x2b\xaa\x99\x84"
           "\x91\xcc\x07\x44\x1f\x09\xff\xd8\x02\xaa\x17\x22\xa3\x12"
           "\x3f\x5d\xe8\xab\x89\xd7\xf4\x48\xb9\x95\x98\xb0\x71\xd9"
           "\xc2\x72\xfd\xc9\x47\x3a\x98\x97\x53\x95\x02\x1d\x3b\x4d"
           "\xf1\x58\xc0\xa1\x85\xcc\x27\x5b\xf2\xec\xc2\x20\xd0\x59"
           "\xb7\x33\x0c\x59\xe1\x7d\x11\x1b\xe6\x9a\x48\xf7\x80\x82"
           "\xb3\x1f\xb3\x9b\xd8\x81\xde\x8a\x1b\x0d\x81\x5d\xe6\xd8"
           "\x13\x1e\x64\xc7\x21\x00\xea\xe0\x1a\xdc\xc2\x02\xed\xa1"
           "\xc5\xe6\x37\xf5\xaa\x3c\xd1\xd2\xf8\x14\x8d\xf1\xc9\xe2"
           "\x95\x48\x2e\x84\xe9\xf8\xf2\x6c\x2a\x01\x73\xf3\xbb\x1a"
           "\x01\x5f\x30\x1b\x72\xec\x64\x61\x68\xff\xf3\x81\x17\x29"
           "\xb4\x62\xad\x67\xc1\xa5\xa8\xec\x70\xe4\xed\x64\x2b\x96"
           "\xbb\x6e\x9f\x77\xf3\xc2\x4f\x69\x9f\xca\xee\x64\xfb\xc2"
           "\xa0\x6a\x93\xcf\x8d\xea\xa4\xff\x29\x1e\xdc\x6e\xcc",
           209);
    *(uint16_t*)0x20e88000 = (uint16_t)0x2;
    memcpy((void*)0x20e88002,
           "\xaf\x1d\x8b\x88\x4a\x7e\x2f\x23\x38\x49\x78\x68\x8a\x32"
           "\xa8\xa9\x13\x59\x67\x4c\xfd\x55\x99\xe1\x60\x03\x36\x7b"
           "\xf2\xf0\x1b\x16\x58\xa9\x53\x3a\xa9\xb4\xa3\x59\x97\x08"
           "\xad\x35\x66\x24\x5d\xcd\x6b\x48\x8c\x8e\x13\xf2\x04\x38"
           "\xe4\x23\x29\x64\x02\xad\x5f\xbd\x17\x19\xbf\x37\x6f\x33"
           "\xf9\x4d\x44\x90\x98\x29\x77\x7e\xb6\x9f\x48\x0d\xb5\x0a"
           "\xc8\x75\x7a\x95\xee\x38\xa1\x7e\xe6\xa2\xe8\x34\x8d\x16"
           "\xf4\xe9\x91\x7c\x62\x7c\x1c\x99\x83\xbb\xd6\xa4\x6f\x2a"
           "\x2b\xb3\x55\x78\xf6\x72\x5b\x63\x5f\x71\xb4\xcd\x4e\x62",
           126);
    r[5] = syscall(__NR_sendto, r[1], 0x20446000ul, 0xd1ul, 0x10ul,
                   0x20e88000ul, 0x80ul);
    break;
  case 3:
    r[6] = syscall(__NR_listen, r[1], 0x10000ul);
    break;
  case 4:
    *(uint32_t*)0x208d6ffc = (uint32_t)0x10;
    r[8] = syscall(__NR_accept, r[1], 0x2050b000ul, 0x208d6ffcul);
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
