// https://syzkaller.appspot.com/bug?id=ed27fa17fde97218578a0f64a9cc69ccadfc8fb8
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <pthread.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <stdint.h>
#include <string.h>

static void test();

void loop()
{
  while (1) {
    test();
  }
}

long r[88];
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
    *(uint16_t*)0x20ff9000 = (uint16_t)0x26;
    memcpy((void*)0x20ff9002,
           "\x61\x65\x61\x64\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
           14);
    *(uint32_t*)0x20ff9010 = (uint32_t)0x0;
    *(uint32_t*)0x20ff9014 = (uint32_t)0x0;
    memcpy((void*)0x20ff9018,
           "\x67\x63\x6d\x28\x74\x77\x6f\x66\x69\x73\x68\x2d\x67\x65"
           "\x6e\x65\x72\x69\x63\x29\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00",
           64);
    r[7] = syscall(__NR_bind, r[1], 0x20ff9000ul, 0x58ul);
    break;
  case 3:
    r[8] = syscall(__NR_accept, r[1], 0x0ul, 0x0ul);
    break;
  case 4:
    *(uint64_t*)0x201dcfc8 = (uint64_t)0x0;
    *(uint32_t*)0x201dcfd0 = (uint32_t)0x0;
    *(uint64_t*)0x201dcfd8 = (uint64_t)0x20012000;
    *(uint64_t*)0x201dcfe0 = (uint64_t)0x1;
    *(uint64_t*)0x201dcfe8 = (uint64_t)0x20719fb8;
    *(uint64_t*)0x201dcff0 = (uint64_t)0x48;
    *(uint32_t*)0x201dcff8 = (uint32_t)0xd0;
    *(uint64_t*)0x20012000 = (uint64_t)0x20049000;
    *(uint64_t*)0x20012008 = (uint64_t)0x0;
    *(uint64_t*)0x20719fb8 = (uint64_t)0x18;
    *(uint32_t*)0x20719fc0 = (uint32_t)0x117;
    *(uint32_t*)0x20719fc4 = (uint32_t)0x3;
    *(uint32_t*)0x20719fc8 = (uint32_t)0x1;
    *(uint64_t*)0x20719fd0 = (uint64_t)0x18;
    *(uint32_t*)0x20719fd8 = (uint32_t)0x117;
    *(uint32_t*)0x20719fdc = (uint32_t)0x3;
    *(uint32_t*)0x20719fe0 = (uint32_t)0x0;
    *(uint64_t*)0x20719fe8 = (uint64_t)0x18;
    *(uint32_t*)0x20719ff0 = (uint32_t)0x117;
    *(uint32_t*)0x20719ff4 = (uint32_t)0x3;
    *(uint32_t*)0x20719ff8 = (uint32_t)0x1;
    r[30] = syscall(__NR_sendmsg, r[8], 0x201dcfc8ul, 0x8010ul);
    break;
  case 5:
    memcpy((void*)0x2023aff0, "\xe5\xfc\x00\x08\x91\xaa\x78\x5d\xeb\xcd"
                              "\xfe\xf1\xdd\xb4\x21\x13",
           16);
    r[32] = syscall(__NR_setsockopt, r[1], 0x117ul, 0x1ul, 0x2023aff0ul,
                    0x10ul);
    break;
  case 6:
    *(uint64_t*)0x20709fc8 = (uint64_t)0x20dab000;
    *(uint32_t*)0x20709fd0 = (uint32_t)0x1c;
    *(uint64_t*)0x20709fd8 = (uint64_t)0x2031efd0;
    *(uint64_t*)0x20709fe0 = (uint64_t)0x3;
    *(uint64_t*)0x20709fe8 = (uint64_t)0x20d40fbc;
    *(uint64_t*)0x20709ff0 = (uint64_t)0x0;
    *(uint32_t*)0x20709ff8 = (uint32_t)0x5;
    *(uint64_t*)0x2031efd0 = (uint64_t)0x2098e000;
    *(uint64_t*)0x2031efd8 = (uint64_t)0x0;
    *(uint64_t*)0x2031efe0 = (uint64_t)0x2010f000;
    *(uint64_t*)0x2031efe8 = (uint64_t)0x1000;
    *(uint64_t*)0x2031eff0 = (uint64_t)0x20e95fd4;
    *(uint64_t*)0x2031eff8 = (uint64_t)0x0;
    r[46] = syscall(__NR_recvmsg, r[8], 0x20709fc8ul, 0x2103ul);
    break;
  case 7:
    *(uint64_t*)0x204bb000 = (uint64_t)0x0;
    *(uint32_t*)0x204bb008 = (uint32_t)0x0;
    *(uint64_t*)0x204bb010 = (uint64_t)0x2016afe0;
    *(uint64_t*)0x204bb018 = (uint64_t)0x2;
    *(uint64_t*)0x204bb020 = (uint64_t)0x204d7000;
    *(uint64_t*)0x204bb028 = (uint64_t)0xa8;
    *(uint32_t*)0x204bb030 = (uint32_t)0x8000;
    *(uint64_t*)0x2016afe0 = (uint64_t)0x20b70f86;
    *(uint64_t*)0x2016afe8 = (uint64_t)0x0;
    *(uint64_t*)0x2016aff0 = (uint64_t)0x20c13000;
    *(uint64_t*)0x2016aff8 = (uint64_t)0xfc;
    memcpy((void*)0x20c13000,
           "\x7a\x70\xcd\x59\x8e\x27\x71\x8f\x86\x55\xc6\x88\x5f\xde"
           "\x5e\x54\x6d\xcf\xa6\x50\x39\xa0\x4f\xed\x2c\xf1\x30\xa1"
           "\x78\xbb\x2d\xa1\xa4\x31\xe0\x43\xdc\x21\x73\x1a\xf1\x55"
           "\x49\x0e\x51\x1e\x17\x92\x40\xb7\xe1\x7c\xe1\xf5\x9b\xca"
           "\x95\xe4\x1d\x52\x2d\x97\xe7\x22\xb7\xd2\xa7\x6f\x21\x69"
           "\xde\xda\x55\xaa\xa8\xa8\x21\xa3\x00\x00\x4d\x85\x0a\xbe"
           "\x89\x02\x08\xe4\xc5\xfd\xd2\xc8\xde\x68\x65\xaa\x6e\x57"
           "\x8c\x2e\x97\x9b\x78\xf9\xcc\xf6\x55\x72\x01\x08\x46\x41"
           "\x75\x47\x8e\x64\x99\x16\x47\x9a\xff\x0d\x86\x20\x75\x75"
           "\x10\x5b\xdd\xb0\x50\xa6\x09\x28\xab\x0b\xe0\x40\xf1\xd6"
           "\xa4\x14\x00\x54\xe7\xa4\x1d\x93\x3e\x13\xf2\xec\x64\xb4"
           "\x86\x04\xfb\x8d\x6f\x46\xa5\x13\x82\x5c\x24\x3f\x4e\xc1"
           "\x29\xbc\xd9\x42\x56\xce\x81\xda\xf8\x5c\x4c\x61\x49\xc2"
           "\x09\x3a\x02\xab\x0e\xf1\xaf\x20\xb7\xd9\xbf\x87\xec\x4a"
           "\xe3\x59\x5d\x17\x2c\x16\x44\x2e\xa2\x4f\x2b\xc7\x2e\xe2"
           "\x19\x6a\xaa\x35\xc6\x5b\x91\x6e\x78\x0c\xf3\xa3\x43\x27"
           "\x45\xb8\x61\x86\xc5\x30\x2a\xb0\x5e\x43\x10\xb3\x75\x9a"
           "\x64\x40\x0d\xf4\x31\x61\x92\x6c\xf4\xba\xf6\xbb\x6d\x54",
           252);
    *(uint64_t*)0x204d7000 = (uint64_t)0x18;
    *(uint32_t*)0x204d7008 = (uint32_t)0x117;
    *(uint32_t*)0x204d700c = (uint32_t)0x3;
    *(uint32_t*)0x204d7010 = (uint32_t)0x1;
    *(uint64_t*)0x204d7018 = (uint64_t)0x18;
    *(uint32_t*)0x204d7020 = (uint32_t)0x117;
    *(uint32_t*)0x204d7024 = (uint32_t)0x4;
    *(uint32_t*)0x204d7028 = (uint32_t)0x3;
    *(uint64_t*)0x204d7030 = (uint64_t)0x18;
    *(uint32_t*)0x204d7038 = (uint32_t)0x117;
    *(uint32_t*)0x204d703c = (uint32_t)0x3;
    *(uint32_t*)0x204d7040 = (uint32_t)0x0;
    *(uint64_t*)0x204d7048 = (uint64_t)0x18;
    *(uint32_t*)0x204d7050 = (uint32_t)0x117;
    *(uint32_t*)0x204d7054 = (uint32_t)0x4;
    *(uint32_t*)0x204d7058 = (uint32_t)0xfffffffffffffeff;
    *(uint64_t*)0x204d7060 = (uint64_t)0x18;
    *(uint32_t*)0x204d7068 = (uint32_t)0x117;
    *(uint32_t*)0x204d706c = (uint32_t)0x4;
    *(uint32_t*)0x204d7070 = (uint32_t)0x0;
    *(uint64_t*)0x204d7078 = (uint64_t)0x18;
    *(uint32_t*)0x204d7080 = (uint32_t)0x117;
    *(uint32_t*)0x204d7084 = (uint32_t)0x4;
    *(uint32_t*)0x204d7088 = (uint32_t)0x3ff;
    *(uint64_t*)0x204d7090 = (uint64_t)0x18;
    *(uint32_t*)0x204d7098 = (uint32_t)0x117;
    *(uint32_t*)0x204d709c = (uint32_t)0x3;
    *(uint32_t*)0x204d70a0 = (uint32_t)0x1;
    r[87] = syscall(__NR_sendmmsg, r[8], 0x204bb000ul, 0x1ul, 0xc080ul);
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
