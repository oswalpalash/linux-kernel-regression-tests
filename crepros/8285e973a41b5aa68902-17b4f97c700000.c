// https://syzkaller.appspot.com/bug?id=34b2f81e51c2861a7cd7a46c0f7ae96d66fb6d93
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000080 = 0;
  *(uint32_t*)0x20000088 = 0;
  *(uint64_t*)0x20000090 = 0x20000000;
  *(uint64_t*)0x20000000 = 0x200000c0;
  memcpy(
      (void*)0x200000c0,
      "\x23\x00\x00\x00\x42\x00\x81\xae\xe4\x05\xe9\xa4\x00\x00\x00\x00\x00\x00"
      "\xc6\xff\x07\xd8\x00\x40\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
      35);
  *(uint64_t*)0x20000008 = 0x23;
  *(uint64_t*)0x20000098 = 1;
  *(uint64_t*)0x200000a0 = 0;
  *(uint64_t*)0x200000a8 = 0;
  *(uint32_t*)0x200000b0 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000080ul, 0ul);
  *(uint64_t*)0x20001740 = 0;
  *(uint32_t*)0x20001748 = 0;
  *(uint64_t*)0x20001750 = 0x20000140;
  *(uint64_t*)0x20000140 = 0x20000300;
  *(uint64_t*)0x20000148 = 0x1000;
  *(uint64_t*)0x20001758 = 1;
  *(uint64_t*)0x20001760 = 0;
  *(uint64_t*)0x20001768 = 0;
  *(uint32_t*)0x20001770 = 0;
  syscall(__NR_recvmsg, r[0], 0x20001740ul, 0ul, 0);
  memcpy((void*)0x200019c0, "/dev/sequencer\000", 15);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x200019c0ul, 1ul, 0ul);
  if (res != -1)
    r[1] = res;
  *(uint8_t*)0x20000040 = 0;
  *(uint8_t*)0x20000041 = 0;
  *(uint8_t*)0x20000042 = 0;
  *(uint8_t*)0x20000043 = 0;
  *(uint32_t*)0x20000044 = 0;
  *(uint8_t*)0x2000004c = 0;
  *(uint8_t*)0x2000004d = 0;
  *(uint8_t*)0x2000004e = 0;
  *(uint8_t*)0x2000004f = 0;
  *(uint8_t*)0x20000050 = 0;
  *(uint8_t*)0x20000051 = 0;
  *(uint16_t*)0x20000052 = 0;
  *(uint64_t*)0x20000054 = 0;
  syscall(__NR_write, r[1], 0x20000040ul, 0xff33ul);
  memcpy((void*)0x20000000, "/dev/snd/timer\000", 15);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000000ul, 0ul, 0);
  if (res != -1)
    r[2] = res;
  *(uint32_t*)0x20000140 = 1;
  syscall(__NR_ioctl, r[2], 0x40045402, 0x20000140ul);
  syscall(__NR_fcntl, r[2], 4ul, 0x2800ul);
  *(uint32_t*)0x20000100 = 1;
  *(uint32_t*)0x20000104 = 0;
  *(uint32_t*)0x20000108 = 0;
  *(uint32_t*)0x2000010c = 0;
  *(uint32_t*)0x20000110 = 0;
  memset((void*)0x20000114, 0, 32);
  syscall(__NR_ioctl, r[2], 0x40345410, 0x20000100ul);
  *(uint32_t*)0x20000440 = 0;
  *(uint32_t*)0x20000444 = 0x20000006;
  *(uint32_t*)0x20000448 = 0;
  *(uint32_t*)0x2000044c = 0;
  *(uint32_t*)0x20000450 = 0xd;
  memset((void*)0x20000454, 0, 60);
  syscall(__NR_ioctl, r[2], 0x40505412, 0x20000440ul);
  syscall(__NR_ioctl, r[2], 0x54a0, 0);
  res = syscall(__NR_socket, 1ul, 2ul, 0);
  if (res != -1)
    r[3] = res;
  *(uint16_t*)0x200006c0 = 1;
  memcpy(
      (void*)0x200006c2,
      "\351\037q\211Y\036\2223aK\000\000\000\000\000\000\000\000\000\000\000"
      "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
      "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
      "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
      "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
      "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000",
      108);
  syscall(__NR_bind, r[3], 0x200006c0ul, 0x6eul);
  *(uint32_t*)0x200001c0 = -1;
  syscall(__NR_ioctl, r[3], 0x8901, 0x200001c0ul);
  *(uint64_t*)0x20000000 = 2;
  syscall(__NR_ioctl, r[3], 0x5452, 0x20000000ul);
  *(uint16_t*)0x20000080 = 1;
  memcpy(
      (void*)0x20000082,
      "\351\037q\211Y\036\2223aK\000\000\000\000\000\000\000\000\000\000\000"
      "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
      "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
      "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
      "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
      "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000",
      108);
  syscall(__NR_connect, r[3], 0x20000080ul, 0x6eul);
  syscall(__NR_sendmmsg, r[3], 0x20002dc0ul, 0x307017fdb7a66cbul, 0x3ec0ul);
  return 0;
}
