// https://syzkaller.appspot.com/bug?id=d0d8879a66b36bd60a0d815b7755e558a9afc82f
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

#ifndef __NR_close_range
#define __NR_close_range 436
#endif

uint64_t r[8] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  intptr_t res = 0;
  memcpy((void*)0x20001700, "/sys/kernel/debug/sync/sw_sync\000", 31);
  res = syscall(__NR_openat, /*fd=*/0xffffffffffffff9cul, /*file=*/0x20001700ul,
                /*flags=*/0ul, /*mode=*/0ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x200001c0 = 0;
  memcpy((void*)0x200001c4,
         "\x21\x94\xac\xa6\x8e\xe8\x6e\xba\x2b\x0c\xda\x81\xa3\xaf\xcd\x23\x23"
         "\x9e\x43\xa9\xce\x86\xd4\xa2\x03\x1b\x13\x49\xa1\xe6\xcf\xdd",
         32);
  res = syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0xc0285700,
                /*arg=*/0x200001c0ul);
  if (res != -1)
    r[1] = *(uint32_t*)0x200001e4;
  res = syscall(__NR_fcntl, /*fd=*/r[1], /*cmd=*/0ul, /*arg=*/r[0]);
  if (res != -1)
    r[2] = res;
  *(uint32_t*)0x20000080 = 2;
  memcpy((void*)0x20000084,
         "\xd2\xc4\x92\x4d\x5e\x89\x21\x3d\xc6\xe4\x45\x37\x39\xf8\x2a\x75\xeb"
         "\x31\x8f\xca\x42\x8c\xc2\xff\xbd\xbe\xc7\x72\x02\x0a\xcd\x2c",
         32);
  res = syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0xc0285700,
                /*arg=*/0x20000080ul);
  if (res != -1)
    r[3] = *(uint32_t*)0x200000a4;
  memcpy((void*)0x20000000, "/sys/kernel/debug/sync/sw_sync\000", 31);
  res = syscall(__NR_openat, /*fd=*/0xffffffffffffff9cul, /*file=*/0x20000000ul,
                /*flags=O_LARGEFILE|O_CLOEXEC|FASYNC|O_RDWR*/ 0x8a002ul,
                /*mode=*/0ul);
  if (res != -1)
    r[4] = res;
  *(uint32_t*)0x20000140 = 1;
  memcpy((void*)0x20000144,
         "\x42\x1a\xe3\x75\x37\x85\x25\x92\x49\x15\x4c\x94\x41\x22\xad\x06\x3f"
         "\xf4\x7d\x3b\xd7\xa8\xa4\x5d\x6b\xb4\xc7\x8a\x3a\xb4\xc9\x81",
         32);
  res = syscall(__NR_ioctl, /*fd=*/r[4], /*cmd=*/0xc0285700,
                /*arg=*/0x20000140ul);
  if (res != -1)
    r[5] = *(uint32_t*)0x20000164;
  memcpy((void*)0x200000c0,
         "\xe5\x0d\x1a\xf8\x89\xb4\xea\x07\x00\x00\x00\x00\x00\x00\x00\xf3\xc4"
         "\x9e\x49\x06\xed\xdf\xec\xd8\x36\x34\xe4\xa3\x7e\xf9\x4a\xdd",
         32);
  *(uint32_t*)0x200000e0 = r[5];
  *(uint32_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000ec = 0;
  res = syscall(__NR_ioctl, /*fd=*/r[3], /*cmd=*/0xc0303e03,
                /*arg=*/0x200000c0ul);
  if (res != -1)
    r[6] = *(uint32_t*)0x200000e4;
  *(uint32_t*)0x20000240 = r[6];
  *(uint16_t*)0x20000244 = 0;
  *(uint16_t*)0x20000246 = 0;
  syscall(__NR_poll, /*fds=*/0x20000240ul, /*nfds=*/1ul, /*timeout=*/1);
  syscall(__NR_close_range, /*fd=*/r[2], /*max_fd=*/-1, /*flags=*/0ul);
  memcpy((void*)0x20004b80, "/sys/kernel/debug/sync/info\000", 28);
  res = syscall(__NR_openat, /*fd=*/0xffffffffffffff9cul, /*file=*/0x20004b80ul,
                /*flags=*/0ul, /*mode=*/0ul);
  if (res != -1)
    r[7] = res;
  syscall(__NR_read, /*fd=*/r[7], /*buf=*/0x20002040ul, /*len=*/0x2020ul);
  return 0;
}
