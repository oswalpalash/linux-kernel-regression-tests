// https://syzkaller.appspot.com/bug?id=1c11a638b7d27e871aa297f3b4d5fd5bc90f0cb4
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <pthread.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <stdint.h>
#include <string.h>

long r[3];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
            0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    r[0] = syscall(__NR_socket, 0x10ul, 0x2ul, 0x6ul);
    break;
  case 2:
    *(uint64_t*)0x20616fc8 = (uint64_t)0x20000000;
    *(uint32_t*)0x20616fd0 = (uint32_t)0x0;
    *(uint64_t*)0x20616fd8 = (uint64_t)0x20664000;
    *(uint64_t*)0x20616fe0 = (uint64_t)0x1;
    *(uint64_t*)0x20616fe8 = (uint64_t)0x2061e000;
    *(uint64_t*)0x20616ff0 = (uint64_t)0x0;
    *(uint32_t*)0x20616ff8 = (uint32_t)0x0;
    *(uint64_t*)0x20664000 = (uint64_t)0x20d9efdf;
    *(uint64_t*)0x20664008 = (uint64_t)0x1f;
    *(uint32_t*)0x20d9efdf = (uint32_t)0x1f;
    *(uint16_t*)0x20d9efe3 = (uint16_t)0x24;
    *(uint16_t*)0x20d9efe5 = (uint16_t)0x1;
    *(uint32_t*)0x20d9efe7 = (uint32_t)0x0;
    *(uint32_t*)0x20d9efeb = (uint32_t)0x0;
    memcpy(
        (void*)0x20d9efef,
        "\x23\x00\xfb\x06\x0b\x00\x03\x00\x00\x00\x00\x00\xff\xff\xff",
        15);
    syscall(__NR_sendmsg, r[0], 0x20616fc8ul, 0x0ul);
    break;
  case 3:
    r[1] = syscall(__NR_socket, 0xaul, 0x2ul, 0x0ul);
    break;
  case 4:
    *(uint8_t*)0x20d51f18 = (uint8_t)0xac;
    *(uint8_t*)0x20d51f19 = (uint8_t)0x14;
    *(uint8_t*)0x20d51f1a = (uint8_t)0x0;
    *(uint8_t*)0x20d51f1b = (uint8_t)0xbb;
    *(uint8_t*)0x20d51f28 = (uint8_t)0xfe;
    *(uint8_t*)0x20d51f29 = (uint8_t)0x80;
    *(uint8_t*)0x20d51f2a = (uint8_t)0x0;
    *(uint8_t*)0x20d51f2b = (uint8_t)0x0;
    *(uint8_t*)0x20d51f2c = (uint8_t)0x0;
    *(uint8_t*)0x20d51f2d = (uint8_t)0x0;
    *(uint8_t*)0x20d51f2e = (uint8_t)0x0;
    *(uint8_t*)0x20d51f2f = (uint8_t)0x0;
    *(uint8_t*)0x20d51f30 = (uint8_t)0x0;
    *(uint8_t*)0x20d51f31 = (uint8_t)0x0;
    *(uint8_t*)0x20d51f32 = (uint8_t)0x0;
    *(uint8_t*)0x20d51f33 = (uint8_t)0x0;
    *(uint8_t*)0x20d51f34 = (uint8_t)0x0;
    *(uint8_t*)0x20d51f35 = (uint8_t)0x0;
    *(uint8_t*)0x20d51f36 = (uint8_t)0x0;
    *(uint8_t*)0x20d51f37 = (uint8_t)0xbb;
    *(uint16_t*)0x20d51f38 = (uint16_t)0x204e;
    *(uint16_t*)0x20d51f3a = (uint16_t)0x0;
    *(uint16_t*)0x20d51f3c = (uint16_t)0x204e;
    *(uint16_t*)0x20d51f3e = (uint16_t)0x0;
    *(uint16_t*)0x20d51f40 = (uint16_t)0x2;
    *(uint8_t*)0x20d51f42 = (uint8_t)0x0;
    *(uint8_t*)0x20d51f43 = (uint8_t)0x0;
    *(uint8_t*)0x20d51f44 = (uint8_t)0x0;
    *(uint32_t*)0x20d51f48 = (uint32_t)0x0;
    *(uint32_t*)0x20d51f4c = (uint32_t)0x0;
    *(uint64_t*)0x20d51f50 = (uint64_t)0x0;
    *(uint64_t*)0x20d51f58 = (uint64_t)0x0;
    *(uint64_t*)0x20d51f60 = (uint64_t)0x0;
    *(uint64_t*)0x20d51f68 = (uint64_t)0x0;
    *(uint64_t*)0x20d51f70 = (uint64_t)0x0;
    *(uint64_t*)0x20d51f78 = (uint64_t)0x0;
    *(uint64_t*)0x20d51f80 = (uint64_t)0x0;
    *(uint64_t*)0x20d51f88 = (uint64_t)0x0;
    *(uint64_t*)0x20d51f90 = (uint64_t)0x0;
    *(uint64_t*)0x20d51f98 = (uint64_t)0x0;
    *(uint64_t*)0x20d51fa0 = (uint64_t)0x0;
    *(uint64_t*)0x20d51fa8 = (uint64_t)0x0;
    *(uint32_t*)0x20d51fb0 = (uint32_t)0x0;
    *(uint32_t*)0x20d51fb4 = (uint32_t)0x0;
    *(uint8_t*)0x20d51fb8 = (uint8_t)0x0;
    *(uint8_t*)0x20d51fb9 = (uint8_t)0x0;
    *(uint8_t*)0x20d51fba = (uint8_t)0x0;
    *(uint8_t*)0x20d51fbb = (uint8_t)0x0;
    *(uint8_t*)0x20d51fc0 = (uint8_t)0xac;
    *(uint8_t*)0x20d51fc1 = (uint8_t)0x14;
    *(uint8_t*)0x20d51fc2 = (uint8_t)0x0;
    *(uint8_t*)0x20d51fc3 = (uint8_t)0xaa;
    *(uint32_t*)0x20d51fd0 = (uint32_t)0x0;
    *(uint8_t*)0x20d51fd4 = (uint8_t)0x0;
    *(uint16_t*)0x20d51fd8 = (uint16_t)0x0;
    *(uint8_t*)0x20d51fdc = (uint8_t)0x0;
    *(uint8_t*)0x20d51fdd = (uint8_t)0x0;
    *(uint8_t*)0x20d51fde = (uint8_t)0x0;
    *(uint8_t*)0x20d51fdf = (uint8_t)0x0;
    *(uint8_t*)0x20d51fe0 = (uint8_t)0x0;
    *(uint8_t*)0x20d51fe1 = (uint8_t)0x0;
    *(uint8_t*)0x20d51fe2 = (uint8_t)0x0;
    *(uint8_t*)0x20d51fe3 = (uint8_t)0x0;
    *(uint8_t*)0x20d51fe4 = (uint8_t)0x0;
    *(uint8_t*)0x20d51fe5 = (uint8_t)0x0;
    *(uint8_t*)0x20d51fe6 = (uint8_t)0x0;
    *(uint8_t*)0x20d51fe7 = (uint8_t)0x0;
    *(uint8_t*)0x20d51fe8 = (uint8_t)0x0;
    *(uint8_t*)0x20d51fe9 = (uint8_t)0x0;
    *(uint8_t*)0x20d51fea = (uint8_t)0x0;
    *(uint8_t*)0x20d51feb = (uint8_t)0x0;
    *(uint32_t*)0x20d51fec = (uint32_t)0x0;
    *(uint8_t*)0x20d51ff0 = (uint8_t)0x0;
    *(uint8_t*)0x20d51ff1 = (uint8_t)0x0;
    *(uint8_t*)0x20d51ff2 = (uint8_t)0x0;
    *(uint32_t*)0x20d51ff4 = (uint32_t)0x0;
    *(uint32_t*)0x20d51ff8 = (uint32_t)0x0;
    *(uint32_t*)0x20d51ffc = (uint32_t)0x0;
    syscall(__NR_setsockopt, r[1], 0x29ul, 0x23ul, 0x20d51f18ul,
            0xe8ul);
    break;
  case 5:
    r[2] = syscall(__NR_socket, 0xful, 0x3ul, 0x2ul);
    break;
  case 6:
    *(uint32_t*)0x20d3affe = (uint32_t)0x0;
    syscall(__NR_setsockopt, r[2], 0x1ul, 0x8ul, 0x20d3affeul, 0x4ul);
    break;
  case 7:
    memcpy((void*)0x20a97ff0, "\x02\x0b\xaf\x01\x02\x00\x00\x00\x00\x06"
                              "\x7b\xbc\x8e\x1d\x4b\x48",
           16);
    syscall(__NR_write, r[2], 0x20a97ff0ul, 0x10ul);
    break;
  case 8:
    memcpy((void*)0x20000ff0, "\x02\x12\xa1\x25\x02\x00\x00\x00\x09\xe5"
                              "\x00\x00\x00\x00\x09\x00",
           16);
    syscall(__NR_write, r[2], 0x20000ff0ul, 0x10ul);
    break;
  }
  return 0;
}

void loop()
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
  loop();
  return 0;
}
