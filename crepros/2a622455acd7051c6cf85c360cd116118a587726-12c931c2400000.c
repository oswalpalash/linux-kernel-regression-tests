// https://syzkaller.appspot.com/bug?id=2a622455acd7051c6cf85c360cd116118a587726
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

static void execute_one();
extern unsigned long long procid;

void loop()
{
  while (1) {
    execute_one();
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
unsigned long long procid;
void execute_one()
{
  long res = 0;
  memcpy((void*)0x200003c0, "./control", 10);
  syscall(__NR_mkdir, 0x200003c0, 0);
  *(uint64_t*)0x20000200 = 0x20011000;
  *(uint64_t*)0x20000208 = 0x3000;
  *(uint64_t*)0x20000210 = 1;
  *(uint64_t*)0x20000218 = 0;
  syscall(__NR_ioctl, -1, 0xc020aa00, 0x20000200);
  memcpy((void*)0x20000040, "./control/file0", 16);
  res = syscall(__NR_creat, 0x20000040, 0);
  if (res != -1)
    r[0] = res;
  *(uint8_t*)0x20011fd2 = 0;
  *(uint8_t*)0x20011fd3 = 0;
  *(uint8_t*)0x20011fd4 = 0;
  *(uint8_t*)0x20011fd5 = 0;
  *(uint64_t*)0x20011fda = 0;
  *(uint64_t*)0x20011fe2 = 0;
  *(uint8_t*)0x20011fea = 0;
  *(uint8_t*)0x20011feb = 0;
  *(uint8_t*)0x20011fec = 0;
  *(uint8_t*)0x20011fed = 0;
  *(uint64_t*)0x20011ff2 = 0x77359400;
  *(uint64_t*)0x20011ffa = 0;
  syscall(__NR_write, r[0], 0x20011fd2, 0x30);
  memcpy((void*)0x20000100, "./file0", 8);
  syscall(__NR_mkdir, 0x20000100, 0);
  memcpy((void*)0x20000140, "/dev/dsp", 9);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000140, 0, 0);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x20000000, "./file0", 8);
  memcpy((void*)0x200000c0, "9p", 3);
  memcpy((void*)0x200004c0, "trans=fd,", 9);
  memcpy((void*)0x200004c9, "rfdno", 5);
  *(uint8_t*)0x200004ce = 0x3d;
  sprintf((char*)0x200004cf, "0x%016llx", (long long)r[1]);
  *(uint8_t*)0x200004e1 = 0x2c;
  memcpy((void*)0x200004e2, "wfdno", 5);
  *(uint8_t*)0x200004e7 = 0x3d;
  sprintf((char*)0x200004e8, "0x%016llx", (long long)r[0]);
  *(uint8_t*)0x200004fa = 0x2c;
  *(uint8_t*)0x200004fb = 0;
  syscall(__NR_mount, 0, 0x20000000, 0x200000c0, 0, 0x200004c0);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      for (;;) {
        loop();
      }
    }
  }
  sleep(1000000);
  return 0;
}
