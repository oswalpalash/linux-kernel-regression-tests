// https://syzkaller.appspot.com/bug?id=1665547180092fef185a46d5a4ba5b382d77848e
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

long r[89];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    r[1] = syscall(__NR_socket, 0x2ul, 0x6ul, 0x0ul);
    break;
  case 2:
    *(uint64_t*)0x20fdf000 = (uint64_t)0x2;
    *(uint64_t*)0x20fdf008 = (uint64_t)0x20e9a000;
    *(uint64_t*)0x20fdf010 = (uint64_t)0x2;
    memcpy((void*)0x20e9a000, "\x25\x64\x66\x43\x27\x84\x5a\x4b\xc7\xec"
                              "\x9a\xfd\x1b\xbf\xe0\x3c",
           16);
    r[6] = syscall(__NR_ioctl, r[1], 0x8940ul, 0x20fdf000ul);
    break;
  case 3:
    r[7] = syscall(__NR_socket, 0xaul, 0x1ul, 0x84ul);
    break;
  case 4:
    memcpy((void*)0x20aa3000,
           "\x03\x56\xef\x5e\x43\xa2\x43\x71\x68\x6c\x70\xaa\xf2\x07"
           "\x20\xa1\x62\x99\x15\xb1\x6d\xf7\xba\x27\x7b\x5a\xd4\xb4"
           "\xdc\xff\x87\x8d\x9d\x77\xf1\xdc\x58\xc9\x36\x48\xfc\x95"
           "\x8b\xf6\xac\x6d\x9f\x35\x7e\x84\x55\xc8\xc3\xba\x3d\xed"
           "\x76\x4b\xe6\x75\xc3\xe3\x88\x85\x1b\x04\x81\xc9\xe1\x7e"
           "\x6e\x56\x58\xd8\xc9\x4e\x94\xa6\xe7\xaa\xc3\xef\xe9\xde"
           "\x09\xda\xbf\xd6\x98\x72\x99\x51\x95\xda\xcd\x00\x28\x3e"
           "\x83\x00",
           100);
    *(uint16_t*)0x20aa4fe4 = (uint16_t)0xa;
    *(uint16_t*)0x20aa4fe6 = (uint16_t)0x234e;
    *(uint32_t*)0x20aa4fe8 = (uint32_t)0xfff;
    *(uint8_t*)0x20aa4fec = (uint8_t)0xfe;
    *(uint8_t*)0x20aa4fed = (uint8_t)0x80;
    *(uint8_t*)0x20aa4fee = (uint8_t)0x0;
    *(uint8_t*)0x20aa4fef = (uint8_t)0x0;
    *(uint8_t*)0x20aa4ff0 = (uint8_t)0x0;
    *(uint8_t*)0x20aa4ff1 = (uint8_t)0x0;
    *(uint8_t*)0x20aa4ff2 = (uint8_t)0x0;
    *(uint8_t*)0x20aa4ff3 = (uint8_t)0x0;
    *(uint8_t*)0x20aa4ff4 = (uint8_t)0x0;
    *(uint8_t*)0x20aa4ff5 = (uint8_t)0x0;
    *(uint8_t*)0x20aa4ff6 = (uint8_t)0x0;
    *(uint8_t*)0x20aa4ff7 = (uint8_t)0x0;
    *(uint8_t*)0x20aa4ff8 = (uint8_t)0x0;
    *(uint8_t*)0x20aa4ff9 = (uint8_t)0x0;
    *(uint8_t*)0x20aa4ffa = (uint8_t)0x0;
    *(uint8_t*)0x20aa4ffb = (uint8_t)0xbb;
    *(uint32_t*)0x20aa4ffc = (uint32_t)0x1a;
    r[29] = syscall(__NR_sendto, r[7], 0x20aa3000ul, 0x64ul, 0x80ul,
                    0x20aa4fe4ul, 0x1cul);
    break;
  case 5:
    r[30] = syscall(__NR_listen, r[7], 0x5ul);
    break;
  case 6:
    *(uint32_t*)0x203ea000 = (uint32_t)0x10;
    r[32] = syscall(__NR_accept4, r[7], 0x20aa4000ul, 0x203ea000ul,
                    0x800ul);
    break;
  case 7:
    *(uint8_t*)0x204ba000 = (uint8_t)0x0;
    *(uint8_t*)0x204ba001 = (uint8_t)0x0;
    *(uint8_t*)0x204ba002 = (uint8_t)0x0;
    *(uint8_t*)0x204ba003 = (uint8_t)0x0;
    *(uint8_t*)0x204ba004 = (uint8_t)0x0;
    *(uint8_t*)0x204ba005 = (uint8_t)0x0;
    *(uint8_t*)0x204ba006 = (uint8_t)0x0;
    *(uint8_t*)0x204ba007 = (uint8_t)0x0;
    *(uint8_t*)0x204ba008 = (uint8_t)0x0;
    *(uint8_t*)0x204ba009 = (uint8_t)0x0;
    *(uint8_t*)0x204ba00a = (uint8_t)0x0;
    *(uint8_t*)0x204ba00b = (uint8_t)0x0;
    *(uint8_t*)0x204ba00c = (uint8_t)0x0;
    *(uint8_t*)0x204ba00d = (uint8_t)0x0;
    *(uint8_t*)0x204ba00e = (uint8_t)0x0;
    *(uint8_t*)0x204ba00f = (uint8_t)0x0;
    *(uint8_t*)0x204ba010 = (uint8_t)0xfe;
    *(uint8_t*)0x204ba011 = (uint8_t)0x80;
    *(uint8_t*)0x204ba012 = (uint8_t)0x0;
    *(uint8_t*)0x204ba013 = (uint8_t)0x0;
    *(uint8_t*)0x204ba014 = (uint8_t)0x0;
    *(uint8_t*)0x204ba015 = (uint8_t)0x0;
    *(uint8_t*)0x204ba016 = (uint8_t)0x0;
    *(uint8_t*)0x204ba017 = (uint8_t)0x0;
    *(uint8_t*)0x204ba018 = (uint8_t)0x0;
    *(uint8_t*)0x204ba019 = (uint8_t)0x0;
    *(uint8_t*)0x204ba01a = (uint8_t)0x0;
    *(uint8_t*)0x204ba01b = (uint8_t)0x0;
    *(uint8_t*)0x204ba01c = (uint8_t)0x0;
    *(uint8_t*)0x204ba01d = (uint8_t)0x0;
    *(uint8_t*)0x204ba01e = (uint8_t)0x0;
    *(uint8_t*)0x204ba01f = (uint8_t)0xaa;
    *(uint8_t*)0x204ba020 = (uint8_t)0x0;
    *(uint8_t*)0x204ba021 = (uint8_t)0x0;
    *(uint8_t*)0x204ba022 = (uint8_t)0x0;
    *(uint8_t*)0x204ba023 = (uint8_t)0x0;
    *(uint8_t*)0x204ba024 = (uint8_t)0x0;
    *(uint8_t*)0x204ba025 = (uint8_t)0x0;
    *(uint8_t*)0x204ba026 = (uint8_t)0x0;
    *(uint8_t*)0x204ba027 = (uint8_t)0x0;
    *(uint8_t*)0x204ba028 = (uint8_t)0x0;
    *(uint8_t*)0x204ba029 = (uint8_t)0x0;
    *(uint8_t*)0x204ba02a = (uint8_t)0x0;
    *(uint8_t*)0x204ba02b = (uint8_t)0x0;
    *(uint8_t*)0x204ba02c = (uint8_t)0x0;
    *(uint8_t*)0x204ba02d = (uint8_t)0x0;
    *(uint8_t*)0x204ba02e = (uint8_t)0x0;
    *(uint8_t*)0x204ba02f = (uint8_t)0x0;
    *(uint32_t*)0x204ba030 = (uint32_t)0x583;
    *(uint16_t*)0x204ba034 = (uint16_t)0x5;
    *(uint16_t*)0x204ba036 = (uint16_t)0x9;
    *(uint32_t*)0x204ba038 = (uint32_t)0x400;
    *(uint64_t*)0x204ba040 = (uint64_t)0x9;
    *(uint32_t*)0x204ba048 = (uint32_t)0x10;
    *(uint32_t*)0x204ba04c = (uint32_t)0x0;
    r[88] = syscall(__NR_ioctl, r[7], 0x890cul, 0x204ba000ul);
    break;
  }
  return 0;
}

void test()
{
  long i;
  pthread_t th[16];

  memset(r, -1, sizeof(r));
  for (i = 0; i < 8; i++) {
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
